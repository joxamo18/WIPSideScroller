// Copyright Epic Games, Inc. All Rights Reserved.

#include "WIPSideScrollerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AWIPSideScrollerCharacter::AWIPSideScrollerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// declare overlap events
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AWIPSideScrollerCharacter::OnOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AWIPSideScrollerCharacter::OnOverlapEnd);


	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f,0.f,75.f);
	CameraBoom->SetRelativeRotation(FRotator(0.f,180.f,0.f));

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	isAgainstWall = false;
}

void AWIPSideScrollerCharacter::BeginPlay() {
	Super::BeginPlay();
	maxJumpTempHolder = JumpMaxCount;
	playerDirection = 1;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWIPSideScrollerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AWIPSideScrollerCharacter::JumpStarted);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AWIPSideScrollerCharacter::DashStarted);
	PlayerInputComponent->BindAction("Dash", IE_Released, this, &AWIPSideScrollerCharacter::DashReleased);

	PlayerInputComponent->BindAxis("MoveRight", this, &AWIPSideScrollerCharacter::MoveRight);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AWIPSideScrollerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AWIPSideScrollerCharacter::TouchStopped);
}

void AWIPSideScrollerCharacter::Tick(float DeltaSeconds)
{
	if (DashTimer > 0.0) {
		DashTimer = DashTimer - DeltaSeconds;
		GetCharacterMovement()->Velocity = FVector(0.0f, (-1 * playerDirection) * DashSpeed, 0.f);
	}

	if (WallJumpTimer > 0.0) {
		WallJumpTimer = WallJumpTimer - DeltaSeconds;
		AddMovementInput(FVector(0.0f, playerDirection, 0.f), 1);
	}
}

void AWIPSideScrollerCharacter::Landed(const FHitResult& Hit) {
	JumpMaxCount = maxJumpTempHolder;
}


void AWIPSideScrollerCharacter::MoveRight(float Value)
{
	// add movement in that direction
	if (WallJumpTimer <= 0.0) {
		AddMovementInput(FVector(0.f, -1.f, 0.f), Value);
	}

	if (Value > 0) {
		playerDirection = 1.0f;
	}
	else if (Value < 0) {
		playerDirection = -1.0f;
	}
	
}

void AWIPSideScrollerCharacter::JumpStarted()
{
	if (isAgainstWall && GetCharacterMovement()->JumpZVelocity && JumpCurrentCount > 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("wall jump"));
		WallJumpTimer = MaxWallJumpDuration;
		JumpMaxCount = JumpMaxCount + 1;
	}
	Jump();
}

void AWIPSideScrollerCharacter::DashStarted()
{
	DashTimer = MaxDashTimeDuration;
}

void AWIPSideScrollerCharacter::DashReleased()
{
	DashTimer = 0.0f;
	GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, 0.f);
}

void AWIPSideScrollerCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch

	Jump();
}

void AWIPSideScrollerCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}

void AWIPSideScrollerCharacter::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		isAgainstWall = true;
	}
}

void AWIPSideScrollerCharacter::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		isAgainstWall = false;
	}
}
