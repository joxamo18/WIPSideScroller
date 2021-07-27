// Copyright Epic Games, Inc. All Rights Reserved.

#include "WIPSideScrollerGameMode.h"
#include "WIPSideScrollerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWIPSideScrollerGameMode::AWIPSideScrollerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
