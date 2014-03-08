/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <ctype.h>
#include <stdio.h>
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>
#include <slot.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_META_FACTORY1( SvMetaSlot, SvMetaAttribute );

SvMetaObject *SvMetaSlot::MakeClone() const
{
        return new SvMetaSlot( *this );
}

SvMetaSlot::SvMetaSlot()
    : aCachable( sal_True, sal_False )
    , aSynchron( sal_True, sal_False )
    , aRecordPerSet( sal_True, sal_False )
    , aRecordAbsolute( sal_False, sal_False )
    , pLinkedSlot(0)
    , pNextSlot(0)
    , pEnumValue(0)
{
}

SvMetaSlot::SvMetaSlot( SvMetaType * pType )
    : SvMetaAttribute( pType )
    , aCachable( sal_True, sal_False )
    , aSynchron( sal_True, sal_False )
    , aRecordPerSet( sal_True, sal_False )
    , aRecordAbsolute( sal_False, sal_False )
    , pLinkedSlot(0)
    , pNextSlot(0)
    , nListPos(0)
    , pEnumValue(0)
{
}

#define TEST_READ
#define TEST_WRITE

void SvMetaSlot::Load( SvPersistStream & rStm )
{
    SvMetaAttribute::Load( rStm );

    sal_uInt16 nMask;
    rStm.ReadUInt16( nMask );

    TEST_READ
    if( nMask & 0x0001 )
    {
        SvMetaAttribute * pMeth;
        rStm >> pMeth;
        aMethod = pMeth;
    }

    TEST_READ
    if( nMask & 0x0002 ) rStm >> aGroupId;
    TEST_READ
    if( nMask & 0x0004 ) rStm >> aHasCoreId;
    TEST_READ
    if( nMask & 0x0008 ) rStm >> aConfigId;
    TEST_READ
    if( nMask & 0x0010 ) rStm >> aExecMethod;
    TEST_READ
    if( nMask & 0x0020 ) rStm >> aStateMethod;
    TEST_READ
    if( nMask & 0x0040 ) rStm >> aDefault;
    TEST_READ
    if( nMask & 0x0080 ) rStm >> aPseudoSlots;
    TEST_READ
    if( nMask & 0x0100 ) rStm >> aGet;
    TEST_READ
    if( nMask & 0x0200 ) rStm >> aSet;
    TEST_READ
    if( nMask & 0x0400 ) rStm >> aCachable;
    TEST_READ
    if( nMask & 0x0800 ) rStm >> aVolatile;
    TEST_READ
    if( nMask & 0x1000 ) rStm >> aToggle;
    TEST_READ
    if( nMask & 0x2000 ) rStm >> aAutoUpdate;
    TEST_READ
    if( nMask & 0x4000 ) rStm >> aSynchron;
    TEST_READ
    if( nMask & 0x8000 ) rStm >> aAsynchron;

    nMask = 0;
    rStm.ReadUInt16( nMask );

    TEST_READ
    if( nMask & 0x0001 ) rStm >> aRecordPerItem;
    TEST_READ
    if( nMask & 0x0002 ) rStm >> aRecordManual;
    TEST_READ
    if( nMask & 0x0004 ) rStm >> aNoRecord;
    TEST_READ
    if( nMask & 0x0008 ) rStm >> aHasDialog;
    TEST_READ
    if( nMask & 0x0010 ) rStm >> aDisableFlags;
    TEST_READ
    if( nMask & 0x0020 ) rStm >> aPseudoPrefix;
    TEST_READ
    if( nMask & 0x0040 ) rStm >> aRecordPerSet;
    TEST_READ
    if( nMask & 0x0080 ) rStm >> aMenuConfig;
    TEST_READ
    if( nMask & 0x0100 ) rStm >> aToolBoxConfig;
    TEST_READ
    if( nMask & 0x0200 ) rStm >> aStatusBarConfig;
    TEST_READ
    if( nMask & 0x0400 ) rStm >> aAccelConfig;
    TEST_READ
    if( nMask & 0x0800 ) rStm >> aFastCall;
    TEST_READ
    if( nMask & 0x1000 ) rStm >> aContainer;
    TEST_READ

    if( nMask & 0x2000 )
    {
        SvMetaType * pT;
        rStm >> pT;
        aSlotType = pT;
    }

    TEST_READ
    if( nMask & 0x4000 ) rStm >> aRecordAbsolute;
    TEST_READ
    if( nMask & 0x8000 ) rStm >> aImageRotation;

    nMask = 0;
    rStm.ReadUInt16( nMask );

    TEST_READ
    if( nMask & 0x0001 ) rStm >> aUnoName;
    if( nMask & 0x0002 ) rStm >> aImageReflection;
}

void SvMetaSlot::Save( SvPersistStream & rStm )
{
    SvMetaAttribute::Save( rStm );

    // create mask
    sal_uInt16 nMask = 0;
    if( aMethod.Is() )          nMask |= 0x0001;
    if( !aGroupId.getString().isEmpty() )       nMask |= 0x0002;
    if( aHasCoreId.IsSet() )    nMask |= 0x0004;
    if( !aConfigId.getString().isEmpty() )      nMask |= 0x0008;
    if( !aExecMethod.getString().isEmpty() )    nMask |= 0x0010;
    if( !aStateMethod.getString().isEmpty() )   nMask |= 0x0020;
    if( !aDefault.getString().isEmpty() )       nMask |= 0x0040;
    if( aPseudoSlots.IsSet() )  nMask |= 0x0080;
    if( aGet.IsSet() )          nMask |= 0x0100;
    if( aSet.IsSet() )          nMask |= 0x0200;
    if( aCachable.IsSet() )     nMask |= 0x0400;
    if( aVolatile.IsSet() )     nMask |= 0x0800;
    if( aToggle.IsSet() )       nMask |= 0x1000;
    if( aAutoUpdate.IsSet() )   nMask |= 0x2000;
    if( aSynchron.IsSet() )     nMask |= 0x4000;
    if( aAsynchron.IsSet() )    nMask |= 0x8000;

    // write data
    rStm.WriteUInt16( nMask );
    TEST_WRITE
    if( nMask & 0x0001 ) WriteSvPersistBase( rStm, aMethod );
    TEST_WRITE
    if( nMask & 0x0002 ) WriteSvIdentifier( rStm, aGroupId );
    TEST_WRITE
    if( nMask & 0x0004 ) rStm.WriteUChar( aHasCoreId );
    TEST_WRITE
    if( nMask & 0x0008 ) WriteSvIdentifier( rStm, aConfigId );
    TEST_WRITE
    if( nMask & 0x0010 ) WriteSvIdentifier( rStm, aExecMethod );
    TEST_WRITE
    if( nMask & 0x0020 ) WriteSvIdentifier( rStm, aStateMethod );
    TEST_WRITE
    if( nMask & 0x0040 ) WriteSvIdentifier( rStm, aDefault );
    TEST_WRITE
    if( nMask & 0x0080 ) rStm.WriteUChar( aPseudoSlots );
    TEST_WRITE
    if( nMask & 0x0100 ) rStm.WriteUChar( aGet );
    TEST_WRITE
    if( nMask & 0x0200 ) rStm.WriteUChar( aSet );
    TEST_WRITE
    if( nMask & 0x0400 ) rStm.WriteUChar( aCachable );
    TEST_WRITE
    if( nMask & 0x0800 ) rStm.WriteUChar( aVolatile );
    TEST_WRITE
    if( nMask & 0x1000 ) rStm.WriteUChar( aToggle );
    TEST_WRITE
    if( nMask & 0x2000 ) rStm.WriteUChar( aAutoUpdate );
    TEST_WRITE
    if( nMask & 0x4000 ) rStm.WriteUChar( aSynchron );
    TEST_WRITE
    if( nMask & 0x8000 ) rStm.WriteUChar( aAsynchron );

    // write next bunch
    // create mask
    nMask = 0;
    if( aRecordPerItem.IsSet() )  nMask |= 0x0001;
    if( aRecordManual.IsSet() )   nMask |= 0x0002;
    if( aNoRecord.IsSet() )       nMask |= 0x0004;
    if( aHasDialog.IsSet() )      nMask |= 0x0008;
    if ( aDisableFlags.IsSet() )      nMask |= 0x0010;
    if( !aPseudoPrefix.getString().isEmpty() )    nMask |= 0x0020;
    if( aRecordPerSet.IsSet() )   nMask |= 0x0040;
    if( aMenuConfig.IsSet() )     nMask |= 0x0080;
    if( aToolBoxConfig.IsSet() )  nMask |= 0x0100;
    if( aStatusBarConfig.IsSet() )nMask |= 0x0200;
    if( aAccelConfig.IsSet() )    nMask |= 0x0400;
    if( aFastCall.IsSet() )       nMask |= 0x0800;
    if( aContainer.IsSet() )      nMask |= 0x1000;
    if( aSlotType.Is() )          nMask |= 0x2000;
    if( aRecordAbsolute.IsSet() ) nMask |= 0x4000;
    if( aImageRotation.IsSet() )       nMask |= 0x8000;

    // write data
    rStm.WriteUInt16( nMask );
    TEST_WRITE
    if( nMask & 0x0001 ) rStm.WriteUChar( aRecordPerItem );
    TEST_WRITE
    if( nMask & 0x0002 ) rStm.WriteUChar( aRecordManual );
    TEST_WRITE
    if( nMask & 0x0004 ) rStm.WriteUChar( aNoRecord );
    TEST_WRITE
    if( nMask & 0x0008 ) rStm.WriteUChar( aHasDialog );
    TEST_WRITE
    if( nMask & 0x0010 ) WriteSvString( rStm, aDisableFlags );
    TEST_WRITE
    if( nMask & 0x0020 ) WriteSvIdentifier( rStm, aPseudoPrefix );
    TEST_WRITE
    if( nMask & 0x0040 ) rStm.WriteUChar( aRecordPerSet );
    TEST_WRITE
    if( nMask & 0x0080 ) rStm.WriteUChar( aMenuConfig );
    TEST_WRITE
    if( nMask & 0x0100 ) rStm.WriteUChar( aToolBoxConfig );
    TEST_WRITE
    if( nMask & 0x0200 ) rStm.WriteUChar( aStatusBarConfig );
    TEST_WRITE
    if( nMask & 0x0400 ) rStm.WriteUChar( aAccelConfig );
    TEST_WRITE
    if( nMask & 0x0800 ) rStm.WriteUChar( aFastCall );
    TEST_WRITE
    if( nMask & 0x1000 ) rStm.WriteUChar( aContainer );
    TEST_WRITE
    if( nMask & 0x2000 ) WriteSvPersistBase( rStm, aSlotType );
    TEST_WRITE
    if( nMask & 0x4000 ) rStm.WriteUChar( aRecordAbsolute );
    TEST_WRITE
    if( nMask & 0x8000 ) rStm.WriteUChar( aImageRotation );

    nMask = 0;
    if( aUnoName.IsSet() )          nMask |= 0x0001;
    if( aImageReflection.IsSet() )  nMask |= 0x0002;
    rStm.WriteUInt16( nMask );
    TEST_WRITE
    if( nMask & 0x0001 ) WriteSvString( rStm, aUnoName );
    TEST_WRITE
    if( nMask & 0x0002 ) rStm.WriteUChar( aImageReflection );
}

sal_Bool SvMetaSlot::IsVariable() const
{
    return SvMetaAttribute::IsVariable();
}

sal_Bool SvMetaSlot::IsMethod() const
{
    bool b = SvMetaAttribute::IsMethod();
    b |= NULL != GetMethod();
    return b;
}

OString SvMetaSlot::GetMangleName( sal_Bool bVariable ) const
{
    if( !bVariable )
    {
        SvMetaAttribute * pMeth = GetMethod();
        if( pMeth )
            return pMeth->GetName().getString();
    }
    return GetName().getString();
}

/*************************************************************************
|*    reference
|*
|*    description       Second FALSE in the SvBOOL-Objects means
|*                      IsSet() provides FALSE (default initialization).
*************************************************************************/
/** reference disbandment **/
SvMetaType * SvMetaSlot::GetSlotType() const
{
    if( aSlotType.Is() || !GetRef() ) return aSlotType;
    return ((SvMetaSlot *)GetRef())->GetSlotType();
}
SvMetaAttribute * SvMetaSlot::GetMethod() const
{
    if( aMethod.Is() || !GetRef() ) return aMethod;
    return ((SvMetaSlot *)GetRef())->GetMethod();
}
sal_Bool SvMetaSlot::GetHasCoreId() const
{
    if( aHasCoreId.IsSet() || !GetRef() ) return aHasCoreId;
    return ((SvMetaSlot *)GetRef())->GetHasCoreId();
}
const OString& SvMetaSlot::GetGroupId() const
{
    if( !aGroupId.getString().isEmpty() || !GetRef() ) return aGroupId.getString();
    return ((SvMetaSlot *)GetRef())->GetGroupId();
}
const OString& SvMetaSlot::GetDisableFlags() const
{
    if( !aDisableFlags.getString().isEmpty() || !GetRef() ) return aDisableFlags.getString();
    return ((SvMetaSlot *)GetRef())->GetDisableFlags();
}
const OString& SvMetaSlot::GetConfigId() const
{
    if( !aConfigId.getString().isEmpty() || !GetRef() ) return aConfigId.getString();
    return ((SvMetaSlot *)GetRef())->GetConfigId();
}
const OString& SvMetaSlot::GetExecMethod() const
{
    if( !aExecMethod.getString().isEmpty() || !GetRef() ) return aExecMethod.getString();
    return ((SvMetaSlot *)GetRef())->GetExecMethod();
}
const OString& SvMetaSlot::GetStateMethod() const
{
    if( !aStateMethod.getString().isEmpty() || !GetRef() ) return aStateMethod.getString();
    return ((SvMetaSlot *)GetRef())->GetStateMethod();
}
const OString& SvMetaSlot::GetDefault() const
{
    if( !aDefault.getString().isEmpty() || !GetRef() ) return aDefault.getString();
    return ((SvMetaSlot *)GetRef())->GetDefault();
}
sal_Bool SvMetaSlot::GetPseudoSlots() const
{
    if( aPseudoSlots.IsSet() || !GetRef() ) return aPseudoSlots;
    return ((SvMetaSlot *)GetRef())->GetPseudoSlots();
}
sal_Bool SvMetaSlot::GetCachable() const
{
    // Cachable and Volatile are exclusive
    if( !GetRef() || aCachable.IsSet() || aVolatile.IsSet() )
         return aCachable;
    return ((SvMetaSlot *)GetRef())->GetCachable();
}
sal_Bool SvMetaSlot::GetVolatile() const
{
    // Cachable and Volatile are exclusive
    if( !GetRef() || aVolatile.IsSet() || aCachable.IsSet() )
        return aVolatile;
    return ((SvMetaSlot *)GetRef())->GetVolatile();
}
sal_Bool SvMetaSlot::GetToggle() const
{
    if( aToggle.IsSet() || !GetRef() ) return aToggle;
    return ((SvMetaSlot *)GetRef())->GetToggle();
}
sal_Bool SvMetaSlot::GetAutoUpdate() const
{
    if( aAutoUpdate.IsSet() || !GetRef() ) return aAutoUpdate;
    return ((SvMetaSlot *)GetRef())->GetAutoUpdate();
}
sal_Bool SvMetaSlot::GetSynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aSynchron.IsSet() || aAsynchron.IsSet() )
        return aSynchron;
    return ((SvMetaSlot *)GetRef())->GetSynchron();
}
sal_Bool SvMetaSlot::GetAsynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aAsynchron.IsSet() || aSynchron.IsSet() )
        return aAsynchron;
    return ((SvMetaSlot *)GetRef())->GetAsynchron();
}
sal_Bool SvMetaSlot::GetRecordPerItem() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerItem;
    return ((SvMetaSlot *)GetRef())->GetRecordPerItem();
}
sal_Bool SvMetaSlot::GetRecordPerSet() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerSet;
    return ((SvMetaSlot *)GetRef())->GetRecordPerSet();
}
sal_Bool SvMetaSlot::GetRecordManual() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordManual;
    return ((SvMetaSlot *)GetRef())->GetRecordManual();
}
sal_Bool SvMetaSlot::GetNoRecord() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aNoRecord;
    return ((SvMetaSlot *)GetRef())->GetNoRecord();
}
sal_Bool SvMetaSlot::GetRecordAbsolute() const
{
    if( !GetRef() || aRecordAbsolute.IsSet() )
        return aRecordAbsolute;
    return ((SvMetaSlot *)GetRef())->GetRecordAbsolute();
}
sal_Bool SvMetaSlot::GetHasDialog() const
{
    if( aHasDialog.IsSet() || !GetRef() ) return aHasDialog;
    return ((SvMetaSlot *)GetRef())->GetHasDialog();
}
const OString& SvMetaSlot::GetPseudoPrefix() const
{
    if( !aPseudoPrefix.getString().isEmpty() || !GetRef() ) return aPseudoPrefix.getString();
    return ((SvMetaSlot *)GetRef())->GetPseudoPrefix();
}
sal_Bool SvMetaSlot::GetMenuConfig() const
{
    if( aMenuConfig.IsSet() || !GetRef() ) return aMenuConfig;
    return ((SvMetaSlot *)GetRef())->GetMenuConfig();
}
sal_Bool SvMetaSlot::GetToolBoxConfig() const
{
    if( aToolBoxConfig.IsSet() || !GetRef() ) return aToolBoxConfig;
    return ((SvMetaSlot *)GetRef())->GetToolBoxConfig();
}
sal_Bool SvMetaSlot::GetStatusBarConfig() const
{
    if( aStatusBarConfig.IsSet() || !GetRef() ) return aStatusBarConfig;
    return ((SvMetaSlot *)GetRef())->GetStatusBarConfig();
}
sal_Bool SvMetaSlot::GetAccelConfig() const
{
    if( aAccelConfig.IsSet() || !GetRef() ) return aAccelConfig;
    return ((SvMetaSlot *)GetRef())->GetAccelConfig();
}
sal_Bool SvMetaSlot::GetFastCall() const
{
    if( aFastCall.IsSet() || !GetRef() ) return aFastCall;
    return ((SvMetaSlot *)GetRef())->GetFastCall();
}
sal_Bool SvMetaSlot::GetContainer() const
{
    if( aContainer.IsSet() || !GetRef() ) return aContainer;
    return ((SvMetaSlot *)GetRef())->GetContainer();
}

sal_Bool SvMetaSlot::GetImageRotation() const
{
    if( aImageRotation.IsSet() || !GetRef() ) return aImageRotation;
    return ((SvMetaSlot *)GetRef())->GetImageRotation();
}

sal_Bool SvMetaSlot::GetImageReflection() const
{
    if( aImageReflection.IsSet() || !GetRef() ) return aImageReflection;
    return ((SvMetaSlot *)GetRef())->GetImageReflection();
}

const OString& SvMetaSlot::GetUnoName() const
{
    if( aUnoName.IsSet() || !GetRef() ) return aUnoName.getString();
    return ((SvMetaSlot *)GetRef())->GetUnoName();
}

void SvMetaSlot::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    SvMetaAttribute::ReadAttributesSvIdl( rBase, rInStm );

    sal_Bool bOk = sal_False;
    bOk |= aDefault.ReadSvIdl( SvHash_Default(), rInStm );
    bOk |= aPseudoSlots.ReadSvIdl( SvHash_PseudoSlots(), rInStm );
    bOk |= aHasCoreId.ReadSvIdl( SvHash_HasCoreId(), rInStm );
    bOk |= aGroupId.ReadSvIdl( SvHash_GroupId(), rInStm );
    bOk |= aExecMethod.ReadSvIdl( SvHash_ExecMethod(), rInStm );
    bOk |= aStateMethod.ReadSvIdl( SvHash_StateMethod(), rInStm );
    bOk |= aDisableFlags.ReadSvIdl( SvHash_DisableFlags(), rInStm );
    if( aGet.ReadSvIdl( SvHash_Get(), rInStm ) )
    {
        rBase.WriteError( "warning", OUStringToOString(rInStm.GetFileName(), RTL_TEXTENCODING_UTF8),
                    "<Get> old style, use Readonly",
                    rInStm.GetToken()->GetLine(),
                    rInStm.GetToken()->GetColumn() );
    }
    if( aSet.ReadSvIdl( SvHash_Set(), rInStm ) )
    {
        rBase.WriteError( "warning", OUStringToOString(rInStm.GetFileName(), RTL_TEXTENCODING_UTF8),
                    "<Set> old style, use method declaration",
                    rInStm.GetToken()->GetLine(),
                    rInStm.GetToken()->GetColumn() );
    }

    if( aCachable.ReadSvIdl( SvHash_Cachable(), rInStm ) )
        SetCachable( aCachable ), bOk = sal_True;
    if( aVolatile.ReadSvIdl( SvHash_Volatile(), rInStm ) )
        SetVolatile( aVolatile ), bOk = sal_True;
    if( aToggle.ReadSvIdl( SvHash_Toggle(), rInStm ) )
        SetToggle( aToggle ), bOk = sal_True;
    if( aAutoUpdate.ReadSvIdl( SvHash_AutoUpdate(), rInStm ) )
        SetAutoUpdate( aAutoUpdate ), bOk = sal_True;

    if( aSynchron.ReadSvIdl( SvHash_Synchron(), rInStm ) )
        SetSynchron( aSynchron ), bOk = sal_True;
    if( aAsynchron.ReadSvIdl( SvHash_Asynchron(), rInStm ) )
        SetAsynchron( aAsynchron ), bOk = sal_True;

    if( aRecordAbsolute.ReadSvIdl( SvHash_RecordAbsolute(), rInStm ) )
        SetRecordAbsolute( aRecordAbsolute), bOk = sal_True;
    if( aRecordPerItem.ReadSvIdl( SvHash_RecordPerItem(), rInStm ) )
        SetRecordPerItem( aRecordPerItem ), bOk = sal_True;
    if( aRecordPerSet.ReadSvIdl( SvHash_RecordPerSet(), rInStm ) )
        SetRecordPerSet( aRecordPerSet ), bOk = sal_True;
    if( aRecordManual.ReadSvIdl( SvHash_RecordManual(), rInStm ) )
        SetRecordManual( aRecordManual ), bOk = sal_True;
    if( aNoRecord.ReadSvIdl( SvHash_NoRecord(), rInStm ) )
        SetNoRecord( aNoRecord ), bOk = sal_True;

    bOk |= aHasDialog.ReadSvIdl( SvHash_HasDialog(), rInStm );
    bOk |= aPseudoPrefix.ReadSvIdl( SvHash_PseudoPrefix(), rInStm );
    bOk |= aMenuConfig.ReadSvIdl( SvHash_MenuConfig(), rInStm );
    bOk |= aToolBoxConfig.ReadSvIdl( SvHash_ToolBoxConfig(), rInStm );
    bOk |= aStatusBarConfig.ReadSvIdl( SvHash_StatusBarConfig(), rInStm );
    bOk |= aAccelConfig.ReadSvIdl( SvHash_AccelConfig(), rInStm );

    SvBOOL aAllConfig;
    if( aAllConfig.ReadSvIdl( SvHash_AllConfig(), rInStm ) )
        SetAllConfig( aAllConfig ), bOk = sal_True;
    bOk |= aFastCall.ReadSvIdl( SvHash_FastCall(), rInStm );
    bOk |= aContainer.ReadSvIdl( SvHash_Container(), rInStm );
    bOk |= aImageRotation.ReadSvIdl( SvHash_ImageRotation(), rInStm );
    bOk |= aImageReflection.ReadSvIdl( SvHash_ImageReflection(), rInStm );
    bOk |= aUnoName.ReadSvIdl( SvHash_UnoName(), rInStm );

    if( !bOk )
    {
        if( !aSlotType.Is() )
        {
            sal_uInt32 nTokPos = rInStm.Tell();
            SvToken * pTok = rInStm.GetToken_Next();
            if( pTok->Is( SvHash_SlotType() ) )
            {
                sal_Bool bBraket = rInStm.Read( '(' );
                if( bBraket || rInStm.Read( '=' ) )
                {
                    aSlotType = rBase.ReadKnownType( rInStm );
                    if( aSlotType.Is() )
                    {
                        if( aSlotType->IsItem() )
                        {
                            if( bBraket )
                            {
                                if( rInStm.Read( ')' ) )
                                    return;
                            }
                            else
                                return;
                        }
                        rBase.SetError( "the SlotType is not a item", rInStm.GetToken() );
                        rBase.WriteError( rInStm );
                    }
                    rBase.SetError( "SlotType with unknown item type", rInStm.GetToken() );
                    rBase.WriteError( rInStm );
                }
            }
            rInStm.Seek( nTokPos );

        }
        if( !aMethod.Is() )
        {
            SvToken * pTok = rInStm.GetToken();
            if( pTok->IsIdentifier() )
            {
                aMethod = new SvMetaSlot();
                sal_uInt32 nTokPos = rInStm.Tell();
                if( aMethod->ReadSvIdl( rBase, rInStm ) )
                {
                    if( aMethod->IsMethod() )
                    {
                        aMethod->SetSlotId( GetSlotId() );
                        if( aMethod->Test( rBase, rInStm ) )
                            return;
                    }
                    rInStm.Seek( nTokPos );
                }
                aMethod.Clear();
            }
        }
    }
}

void SvMetaSlot::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                       SvStream & rOutStm,
                       sal_uInt16 nTab )
{
    SvMetaAttribute::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    if( aSlotType.Is() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( SvHash_SlotType()->GetName().getStr() ).WriteChar( '(' );
        aSlotType->WriteTheType( rBase, rOutStm, nTab, WRITE_IDL );
        rOutStm.WriteCharPtr( ");" ) << endl;
    }
    if( aMethod.Is() )
    {
        WriteTab( rOutStm, nTab );
        aMethod->WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm.WriteChar( ';' ) << endl;
    }
    if( aHasCoreId )
    {
        aHasCoreId.WriteSvIdl( SvHash_HasCoreId(), rOutStm );
        rOutStm.WriteChar( ';' ) << endl;
    }
    if( !aGroupId.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aGroupId.WriteSvIdl( SvHash_GroupId(), rOutStm, nTab +1);
        rOutStm.WriteChar( ';' ) << endl;
    }
    if( !aExecMethod.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aExecMethod.WriteSvIdl( SvHash_ExecMethod(), rOutStm, nTab +1);
        rOutStm.WriteChar( ';' ) << endl;
    }
    if( !aStateMethod.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aStateMethod.WriteSvIdl( SvHash_StateMethod(), rOutStm, nTab +1);
        rOutStm.WriteChar( ';' ) << endl;
    }

    if( !aDisableFlags.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aDisableFlags.WriteSvIdl( SvHash_DisableFlags(), rOutStm, nTab +1);
        rOutStm.WriteChar( ';' ) << endl;
    }

    if( aSet || aGet || aPseudoSlots )
    {
        WriteTab( rOutStm, nTab );
        char const * p = "";
        if( aPseudoSlots )
        {
            aPseudoSlots.WriteSvIdl( SvHash_PseudoSlots(), rOutStm );
            p = ", ";
        }
        if( aGet )
        {
            rOutStm.WriteCharPtr( p );
            aGet.WriteSvIdl( SvHash_Get(), rOutStm );
            p = ", ";
        }
        if( aSet )
        {
            rOutStm.WriteCharPtr( p );
            aSet.WriteSvIdl( SvHash_Set(), rOutStm );
        }
        rOutStm.WriteChar( ';' ) << endl;
    }

    OString aDel(", ");
    OStringBuffer aOut;
    if( aVolatile )
        aOut.append(aVolatile.GetSvIdlString( SvHash_Volatile() ));
    else if( !aCachable )
        // because of Default == TRUE, only when no other is set
        aOut.append(aCachable.GetSvIdlString( SvHash_Cachable() ));
    else
        aDel = OString();

    if( aToggle )
    {
        aOut.append(aDel).append(aToggle.GetSvIdlString( SvHash_Toggle() ));
        aDel = ", ";
    }
    if( aAutoUpdate )
    {
        aOut.append(aDel).append(aAutoUpdate.GetSvIdlString( SvHash_AutoUpdate() ));
        aDel = ", ";
    }

    OString aDel1(", ");
    if( aAsynchron )
        aOut.append(aDel).append(aAsynchron.GetSvIdlString( SvHash_Asynchron() ));
    else if( !aSynchron )
    {
        // because of Default == TRUE, only when no other is set
        aOut.append(aDel).append(aSynchron.GetSvIdlString( SvHash_Synchron() ));
    }
    else
        aDel1 = aDel;

    aDel = ", ";
    if( aRecordManual )
        aOut.append(aDel1).append(aRecordManual.GetSvIdlString( SvHash_RecordManual() ));
    else if( aNoRecord )
        aOut.append(aDel1).append(aNoRecord.GetSvIdlString( SvHash_NoRecord() ));
    else if( !aRecordPerSet )
        // because of Default == TRUE, only when no other is set
        aOut.append(aDel1).append(aRecordPerSet.GetSvIdlString( SvHash_RecordPerSet() ));
    else if( aRecordPerItem )
        aOut.append(aDel1).append(aRecordPerItem.GetSvIdlString( SvHash_RecordPerItem() ));
    else
        aDel = aDel1;

    if( aRecordAbsolute )
    {
        aOut.append(aDel).append(aRecordAbsolute.GetSvIdlString( SvHash_RecordAbsolute() ));
        aDel = ", ";
    }
    if( aHasDialog )
    {
        aOut.append(aDel).append(aHasDialog.GetSvIdlString( SvHash_HasDialog() ));
        aDel = ", ";
    }
    if( aMenuConfig )
    {
        aOut.append(aDel).append(aMenuConfig.GetSvIdlString( SvHash_MenuConfig() ));
        aDel = ", ";
    }
    if( aToolBoxConfig )
    {
        aOut.append(aDel).append(aToolBoxConfig.GetSvIdlString( SvHash_ToolBoxConfig() ));
        aDel = ", ";
    }
    if( aStatusBarConfig )
    {
        aOut.append(aDel).append(aStatusBarConfig.GetSvIdlString( SvHash_StatusBarConfig() ));
        aDel = ", ";
    }
    if( aAccelConfig )
    {
        aOut.append(aDel).append(aAccelConfig.GetSvIdlString( SvHash_AccelConfig() ));
        aDel = ", ";
    }
    if( aFastCall )
    {
        aOut.append(aDel).append(aFastCall.GetSvIdlString( SvHash_FastCall() ));
        aDel = ", ";
    }
    if( aContainer )
    {
        aOut.append(aDel).append(aContainer.GetSvIdlString( SvHash_Container() ));
        aDel = ", ";
    }
    if( aImageRotation )
    {
        aOut.append(aDel).append(aImageRotation.GetSvIdlString( SvHash_ImageRotation() ));
        aDel = ", ";
    }

    if( aImageReflection )
    {
        aOut.append(aDel).append(aImageReflection.GetSvIdlString( SvHash_ImageReflection() ));
        aDel = ", ";
    }

    if( !aOut.isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( aOut.getStr() ) << endl;
    }
}


sal_Bool SvMetaSlot::Test( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_Bool bOk = SvMetaAttribute::Test( rBase, rInStm );
    if( bOk )
    {
        SvMetaType * pType = GetType();
        if( pType->GetType() == TYPE_METHOD )
            pType = pType->GetReturnType();
        if( !pType->IsItem() )
        {
            rBase.SetError( "this attribute is not a slot", rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = sal_False;
        }
    }

    return bOk;
}

sal_Bool SvMetaSlot::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos     = rInStm.Tell();
    sal_Bool bOk = sal_True;

    SvMetaAttribute * pAttr = rBase.ReadKnownAttr( rInStm, GetType() );
    if( pAttr )
    {
        // c
        SvMetaSlot * pKnownSlot = PTR_CAST( SvMetaSlot, pAttr );
        if( pKnownSlot )
        {
            SetRef( pKnownSlot );
            SetName( pKnownSlot->GetName().getString(), &rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        else
        {
            OStringBuffer aStr( "attribute " );
            aStr.append(pAttr->GetName().getString());
            aStr.append(" is method or variable but not a slot");
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = sal_False;
        }
    }
    else
    {
        bOk = SvMetaAttribute::ReadSvIdl( rBase, rInStm );

        SvMetaAttribute *pAttr2 = rBase.SearchKnownAttr( GetSlotId() );
        if( pAttr2 )
        {
            // for testing purposes: reference in case of complete definition
            SvMetaSlot * pKnownSlot = PTR_CAST( SvMetaSlot, pAttr2 );
            if( pKnownSlot )
            {
                SetRef( pKnownSlot );

                  // names may differ, because explicitly given
                if ( pKnownSlot->GetName().getString() != GetName().getString() )
                {
                    OSL_FAIL("Illegal definition!");
                    rInStm.Seek( nTokPos );
                    return sal_False;
                }

                  SetName( pKnownSlot->GetName().getString(), &rBase );
            }
            else
            {
                OStringBuffer aStr("attribute ");
                aStr.append(pAttr2->GetName().getString());
                aStr.append(" is method or variable but not a slot");
                rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
                rBase.WriteError( rInStm );
                bOk = sal_False;
            }
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );

    return bOk;
}

void SvMetaSlot::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab )
{
    SvMetaAttribute::WriteSvIdl( rBase, rOutStm, nTab );
}

void SvMetaSlot::Write( SvIdlDataBase & rBase,
                        SvStream & rOutStm, sal_uInt16 nTab,
                        WriteType nT, WriteAttribute nA )
{
    if ( nT == WRITE_DOCU )
    {
        if ( GetHidden() )
            return;
    }
    else
    {
        // no attribute for Automation
        if( !GetAutomation() || !GetExport() )
            return;
    }

    if( !(nA & WA_VARIABLE) )
    {
        SvMetaAttributeRef xM = GetMethod();
        if( xM.Is() )
        {
            xM->SetSlotId( GetSlotId() );
            xM->SetDescription( GetDescription().getString() );
            xM->Write( rBase, rOutStm, nTab, nT, nA );
            return;
        }
    }

    SvMetaAttribute::Write( rBase, rOutStm, nTab, nT, nA );
}


void SvMetaSlot::Insert( SvSlotElementList& rList, const OString& rPrefix,
                        SvIdlDataBase& rBase)
{
    // get insert position through binary search in slotlist
    sal_uInt16 nId = (sal_uInt16) GetSlotId().GetValue();
    sal_uInt16 nListCount = (sal_uInt16) rList.size();
    sal_uInt16 nPos;
    sal_uLong m;  // for inner "for" loop

    if ( !nListCount )
        nPos = 0;
    else if ( nListCount == 1 )
        nPos = rList[ 0 ]->xSlot->GetSlotId().GetValue() >= nId ? 0 : 1;
    else
    {
        sal_uInt16 nMid = 0, nLow = 0;
        sal_uInt16 nHigh = nListCount - 1;
        sal_Bool bFound = sal_False;
        while ( !bFound && nLow <= nHigh )
        {
            nMid = (nLow + nHigh) >> 1;
            DBG_ASSERT( nMid < nListCount, "bsearch ist buggy" );
            int nDiff = (int) nId - (int) rList[ nMid ]->xSlot->GetSlotId().GetValue();
            if ( nDiff < 0)
            {
                if ( nMid == 0 )
                    break;
                nHigh = nMid - 1;
            }
            else if ( nDiff > 0 )
            {
                nLow = nMid + 1;
                if ( nLow == 0 )
                break;
            }
            else
                bFound = sal_True;
        }

        DBG_ASSERT(!bFound, "Duplicate SlotId!");
        nPos = bFound ? nMid : nLow;
    }

    DBG_ASSERT( nPos <= nListCount,
        "nPos too large" );
    DBG_ASSERT( nPos == nListCount || nId <=
        (sal_uInt16) rList[ nPos ]->xSlot->GetSlotId().GetValue(),
        "Successor has lower SlotId" );
    DBG_ASSERT( nPos == 0 || nId >
        (sal_uInt16) rList[ nPos-1 ]->xSlot->GetSlotId().GetValue(),
        "Predecessor has higher SlotId" );
    DBG_ASSERT( nPos+1 >= nListCount || nId <
        (sal_uInt16) rList[ nPos+1 ]->xSlot->GetSlotId().GetValue(),
        "Successor has lower SlotId" );

    if ( nPos < rList.size() )
    {
        SvSlotElementList::iterator it = rList.begin();
        std::advance( it, nPos );
        rList.insert( it, new SvSlotElement( this, rPrefix ) );
    }
    else
    {
        rList.push_back( new SvSlotElement( this, rPrefix ) );
    }

    // iron out EnumSlots
    SvMetaTypeEnum * pEnum = NULL;
    SvMetaType * pBType = GetType()->GetBaseType();
    pEnum = PTR_CAST( SvMetaTypeEnum, pBType );
    if( GetPseudoSlots() && pEnum && pEnum->Count() )
    {
        // clone the MasterSlot
        SvMetaSlotRef xEnumSlot;
        SvMetaSlot *pFirstEnumSlot = NULL;
        for( sal_uLong n = 0; n < pEnum->Count(); n++ )
        {
            // create SlotId
            SvMetaEnumValue *enumValue = pEnum->GetObject(n);
            OString aValName = enumValue->GetName().getString();
            OStringBuffer aBuf;
            if( !GetPseudoPrefix().isEmpty() )
                aBuf.append(GetPseudoPrefix());
            else
                aBuf.append(GetSlotId().getString());
            aBuf.append('_');
            aBuf.append(aValName.copy(pEnum->GetPrefix().getLength()));

            OString aSId = aBuf.makeStringAndClear();

            xEnumSlot = NULL;
            for( m=0; m<rBase.GetAttrList().size(); m++ )
            {
                SvMetaAttribute * pAttr = rBase.GetAttrList()[m];
                if (aSId.equals(pAttr->GetSlotId().getString()))
                {
                    SvMetaSlot* pSlot = PTR_CAST( SvMetaSlot, pAttr );
                    xEnumSlot = pSlot->Clone();
                    break;
                }
            }

            if ( m == rBase.GetAttrList().size() )
            {
                OSL_FAIL("Invalid EnumSlot!");
                xEnumSlot = Clone();
                sal_uLong nValue;
                if ( rBase.FindId(aSId , &nValue) )
                {
                    SvNumberIdentifier aId;
                    aId.setString(aSId);
                    aId.SetValue(nValue);
                    xEnumSlot->SetSlotId(aId);
                }
            }

            // The slaves are no master!
            xEnumSlot->aPseudoSlots = sal_False;
            xEnumSlot->SetEnumValue(enumValue);

            if ( !pFirstEnumSlot || xEnumSlot->GetSlotId().GetValue() < pFirstEnumSlot->GetSlotId().GetValue() )
                pFirstEnumSlot = xEnumSlot;

            // insert the created slave as well
            xEnumSlot->Insert( rList, rPrefix, rBase);

            // concatenate the EnumSlots with the master
            xEnumSlot->pLinkedSlot = this;
        }

        // master points to the first slave
        pLinkedSlot = pFirstEnumSlot;

        // concatenate slaves among themselves
        xEnumSlot = pFirstEnumSlot;
        size_t i = 0;
        SvSlotElement *pEle;
        do
        {
            pEle = ( ++i < rList.size() ) ? rList[ i ] : NULL;
            if ( pEle && pEle->xSlot->pLinkedSlot == this )
            {
                xEnumSlot->pNextSlot = pEle->xSlot;
                xEnumSlot = pEle->xSlot;
            }
        }
        while ( pEle );
        xEnumSlot->pNextSlot = pFirstEnumSlot;
    }
}


static OString MakeSlotName( SvStringHashEntry * pEntry )
{
    OStringBuffer aName("SFX_SLOT_");
    aName.append(pEntry->GetName());
    return aName.makeStringAndClear().toAsciiUpperCase();
};

void SvMetaSlot::WriteSlotStubs( const OString& rShellName,
                                ByteStringList & rList,
                                SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    OString aMethodName( GetExecMethod() );
    if ( !aMethodName.isEmpty() &&
         aMethodName != "NoExec" )
    {
        sal_Bool bIn = sal_False;
        for( size_t n = 0; n < rList.size(); n++ )
        {
            if (rList[n]->equals(aMethodName))
            {
                bIn=sal_True;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( new OString(aMethodName) );
            rOutStm.WriteCharPtr( "SFX_EXEC_STUB(" )
                   .WriteCharPtr( rShellName.getStr() )
                   .WriteChar( ',' )
                   .WriteCharPtr( aMethodName.getStr() )
                   .WriteChar( ')' ) << endl;
        }
    }

    aMethodName = GetStateMethod();
    if (!aMethodName.isEmpty() &&
        aMethodName != "NoState")
    {
        sal_Bool bIn = sal_False;
        for ( size_t n=0; n < rList.size(); n++ )
        {
            if (rList[n]->equals(aMethodName))
            {
                bIn=sal_True;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( new OString(aMethodName) );
            rOutStm.WriteCharPtr( "SFX_STATE_STUB(" )
                   .WriteCharPtr( rShellName.getStr() )
                   .WriteChar( ',' )
                   .WriteCharPtr( aMethodName.getStr() )
                   .WriteChar( ')' ) << endl;
        }
    }
}

void SvMetaSlot::WriteSlot( const OString& rShellName, sal_uInt16 nCount,
                            const OString& rSlotId,
                            SvSlotElementList& rSlotList,
                            size_t nStart,
                            const OString& rPrefix,
                            SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    sal_Bool bIsEnumSlot = 0 != pEnumValue;

    rOutStm.WriteCharPtr( "// Slot Nr. " )
       .WriteCharPtr( OString::number(nListPos).getStr() )
       .WriteCharPtr( " : " );
    OString aSlotIdValue(OString::number(GetSlotId().GetValue()));
    rOutStm.WriteCharPtr( aSlotIdValue.getStr() ) << endl;
    WriteTab( rOutStm, 1 );
    if( bIsEnumSlot )
        rOutStm.WriteCharPtr( "SFX_NEW_SLOT_ENUM( " );
    else
        rOutStm.WriteCharPtr( "SFX_NEW_SLOT_ARG( " ).WriteCharPtr( rShellName.getStr() ).WriteChar( ',' ) ;

    rOutStm.WriteCharPtr( rSlotId.getStr() ).WriteChar( ',' );
    const SvHelpContext& rHlpCtx = GetHelpContext();
    if( rHlpCtx.IsSet() )
        rOutStm.WriteCharPtr( rHlpCtx.getString().getStr() ).WriteChar( ',' );
    else
        rOutStm.WriteCharPtr( rSlotId.getStr() ).WriteChar( ',' );

    // GroupId
    if( !GetGroupId().isEmpty() )
        rOutStm.WriteCharPtr( GetGroupId().getStr() );
    else
        rOutStm.WriteChar( '0' );
    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 4 );

    if( bIsEnumSlot )
    {
        rOutStm.WriteCharPtr( "&a" ).WriteCharPtr( rShellName.getStr() ).WriteCharPtr( "Slots_Impl[" )
           .WriteCharPtr( OString::number(pLinkedSlot->GetListPos()).getStr() )
           .WriteCharPtr( "] /*Offset Master*/, " ) << endl;
        WriteTab( rOutStm, 4 );
        rOutStm.WriteCharPtr( "&a" ).WriteCharPtr( rShellName.getStr() ).WriteCharPtr( "Slots_Impl[" )
           .WriteCharPtr( OString::number(pNextSlot->GetListPos()).getStr() )
           .WriteCharPtr( "] /*Offset Next*/, " ) << endl;

        WriteTab( rOutStm, 4 );

        // SlotId
        if( !GetSlotId().getString().isEmpty() )
            rOutStm.WriteCharPtr( pLinkedSlot->GetSlotId().getString().getStr() );
        else
            rOutStm.WriteChar( '0' );
        rOutStm.WriteChar( ',' );
        rOutStm.WriteCharPtr( pEnumValue->GetName().getString().getStr() );
    }
    else
    {
        // look for the next slot with the same StateMethod like me
        // the slotlist is set to the current slot
        size_t i = nStart;
        SvSlotElement* pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : NULL;
        pNextSlot = pEle ? &pEle->xSlot : NULL;
        while ( pNextSlot )
        {
            if ( !pNextSlot->pNextSlot &&
                pNextSlot->GetStateMethod() == GetStateMethod()
            ) {
                break;
            }
            pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : NULL;
            pNextSlot = pEle ? &pEle->xSlot : NULL;
        }

        if ( !pNextSlot )
        {
            // There is no slot behind me that has the same ExecMethod.
            // So I search for the first slot with it (could be myself).
            i = 0;
            pEle = rSlotList.empty() ? NULL : rSlotList[ i ];
            pNextSlot = pEle ? &pEle->xSlot : NULL;
            while ( pNextSlot != this )
            {
                if ( !pNextSlot->pEnumValue &&
                    pNextSlot->GetStateMethod() == GetStateMethod() )
                    break;
                pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : NULL;
                pNextSlot = pEle ? &pEle->xSlot : NULL;
            }
        }

        if ( !pLinkedSlot )
        {
            rOutStm.WriteCharPtr( "0 ," );
        }
        else
        {
            rOutStm.WriteCharPtr( "&a" ).WriteCharPtr( rShellName.getStr() ).WriteCharPtr( "Slots_Impl[" )
               .WriteCharPtr( OString::number(pLinkedSlot->GetListPos()).getStr() )
               .WriteCharPtr( "] /*Offset Linked*/, " ) << endl;
            WriteTab( rOutStm, 4 );
        }

        rOutStm.WriteCharPtr( "&a" ).WriteCharPtr( rShellName.getStr() ).WriteCharPtr( "Slots_Impl[" )
           .WriteCharPtr( OString::number(pNextSlot->GetListPos()).getStr() )
           .WriteCharPtr( "] /*Offset Next*/, " ) << endl;

        WriteTab( rOutStm, 4 );

        // write ExecMethod, with standard name if not specified
        if( !GetExecMethod().isEmpty() &&
            GetExecMethod() != "NoExec")
        {
            rOutStm.WriteCharPtr( "SFX_STUB_PTR(" ).WriteCharPtr( rShellName.getStr() ).WriteChar( ',' )
                   .WriteCharPtr( GetExecMethod().getStr() ).WriteChar( ')' );
        }
        else
            rOutStm.WriteCharPtr( "SFX_STUB_PTR_EXEC_NONE" );
        rOutStm.WriteChar( ',' );

        // write StateMethod, with standard name if not specified
        if( !GetStateMethod().isEmpty() &&
            GetStateMethod() != "NoState")
        {
            rOutStm.WriteCharPtr( "SFX_STUB_PTR(" ).WriteCharPtr( rShellName.getStr() ).WriteChar( ',' )
                   .WriteCharPtr( GetStateMethod().getStr() ).WriteChar( ')' );
        }
        else
            rOutStm.WriteCharPtr( "SFX_STUB_PTR_STATE_NONE" );
    }
    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 4 );

    // write flags
    if( GetHasCoreId() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_HasCoreId() ).getStr() ).WriteChar( '|' );
    if( GetCachable() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_Cachable() ).getStr() ).WriteChar( '|' );
    if( GetVolatile() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_Volatile() ).getStr() ).WriteChar( '|' );
    if( GetToggle() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_Toggle() ).getStr() ).WriteChar( '|' );
    if( GetAutoUpdate() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_AutoUpdate() ).getStr() ).WriteChar( '|' );
    if( GetSynchron() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_Synchron() ).getStr() ).WriteChar( '|' );
    if( GetAsynchron() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_Asynchron() ).getStr() ).WriteChar( '|' );
    if( GetRecordPerItem() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_RecordPerItem() ).getStr() ).WriteChar( '|' );
    if( GetRecordPerSet() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_RecordPerSet() ).getStr() ).WriteChar( '|' );
    if( GetRecordManual() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_RecordManual() ).getStr() ).WriteChar( '|' );
    if( GetNoRecord() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_NoRecord() ).getStr() ).WriteChar( '|' );
    if( GetRecordAbsolute() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_RecordAbsolute() ).getStr() ).WriteChar( '|' );
    if( GetHasDialog() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_HasDialog() ).getStr() ).WriteChar( '|' );
    if( GetMenuConfig() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_MenuConfig() ).getStr() ).WriteChar( '|' );
    if( GetToolBoxConfig() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_ToolBoxConfig() ).getStr() ).WriteChar( '|' );
    if( GetStatusBarConfig() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_StatusBarConfig() ).getStr() ).WriteChar( '|' );
    if( GetAccelConfig() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_AccelConfig() ).getStr() ).WriteChar( '|' );
    if( GetFastCall() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_FastCall() ).getStr() ).WriteChar( '|' );
    if( GetContainer() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_Container() ).getStr() ).WriteChar( '|' );
    if ( GetReadOnlyDoc() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_ReadOnlyDoc() ).getStr() ).WriteChar( '|' );
    if( GetImageRotation() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_ImageRotation() ).getStr() ).WriteChar( '|' );
    if( GetImageReflection() )
        rOutStm.WriteCharPtr( MakeSlotName( SvHash_ImageReflection() ).getStr() ).WriteChar( '|' );
    rOutStm.WriteChar( '0' );

    rOutStm.WriteChar( ',' ) << endl;
       WriteTab( rOutStm, 4 );
    if ( GetDisableFlags().isEmpty() )
        rOutStm.WriteCharPtr( "0" );
    else
        rOutStm.WriteCharPtr( GetDisableFlags().getStr() );

    // write attribute type
    if( !bIsEnumSlot )
    {
        rOutStm.WriteChar( ',' ) << endl;
        WriteTab( rOutStm, 4 );

        SvMetaType * pT = GetSlotType();
        if( !pT )
        {
            if( !IsVariable() )
                pT = rBase.FindType( "SfxVoidItem" );
            else
                pT = GetType();
        }
        if( pT )
        {
            rOutStm.WriteCharPtr( pT->GetName().getString().getStr() );
            if( !rBase.FindType( pT, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pT );
        }
        else
            rOutStm.WriteCharPtr( "SfxVoidItem not defined" );
    }
    else
    {
        SvMetaType *pT = rBase.FindType( "SfxBoolItem" );
        if ( pT && !rBase.FindType( pT, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pT );
    }

    if( !bIsEnumSlot )
    {
        rOutStm.WriteChar( ',' ) << endl;
        WriteTab( rOutStm, 4 );
        rOutStm
           .WriteCharPtr( OString::number(nCount).getStr() )
           .WriteCharPtr( "/*Offset*/, " );

        if( IsMethod() )
        {
            SvMetaAttribute * pMethod = GetMethod();
            SvMetaType * pType;
            if( pMethod )
                pType = pMethod->GetType();
            else
                pType = GetType();
            sal_uLong nSCount = pType->GetAttrCount();
            rOutStm
               .WriteCharPtr( OString::number(nSCount).getStr() )
               .WriteCharPtr( "/*Count*/" );
        }
        else
            rOutStm.WriteChar( '0' );

        // name for recording
        if ( GetExport() )
        {
            rOutStm.WriteCharPtr( ",\"" );
            if (!rPrefix.isEmpty())
                rOutStm.WriteCharPtr( rPrefix.getStr() );
            rOutStm.WriteChar( '.' );
            if ( !IsVariable() || !GetType() ||
                 GetType()->GetBaseType()->GetType() != TYPE_STRUCT )
                rOutStm.WriteCharPtr( GetMangleName( sal_False ).getStr() );
            rOutStm.WriteCharPtr( "\"," );
        }
        else
            rOutStm.WriteCharPtr( ", 0, " );

        // Method/Property flags
        if( IsMethod() )
            rOutStm.WriteCharPtr( "SFX_SLOT_METHOD|" );
        if( IsVariable() )
        {
            rOutStm.WriteCharPtr( "SFX_SLOT_PROPGET|" );
            if( !GetReadonly() )
                rOutStm.WriteCharPtr( "SFX_SLOT_PROPSET|" );
        }

        rOutStm.WriteChar( '0' );
    }

    {
        rOutStm.WriteCharPtr( ",\"" );
        rOutStm.WriteCharPtr( GetMangleName( sal_False ).getStr() );
        rOutStm.WriteCharPtr( "\"" );
    }

    rOutStm.WriteCharPtr( " )," ) << endl;
}

sal_uInt16 SvMetaSlot::WriteSlotParamArray( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return 0;

    SvMetaAttribute * pMethod = GetMethod();
    if( IsMethod() )
    {
        SvMetaType * pType;
        if( pMethod )
            pType = pMethod->GetType();
        else
            pType = GetType();

        if( !rBase.FindType( pType, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pType );

        const SvMetaAttributeMemberList & rList =
                    pType->GetAttrList();
        for( sal_uLong n = 0; n < rList.size(); n++ )
        {
            SvMetaAttribute * pPar  = rList[n];
            SvMetaType * pPType     = pPar->GetType();
            WriteTab( rOutStm, 1 );
            rOutStm.WriteCharPtr( "SFX_ARGUMENT(" )
               .WriteCharPtr( pPar->GetSlotId().getString().getStr() ).WriteChar( ',' ) // SlodId
                // parameter name
               .WriteCharPtr( "\"" ).WriteCharPtr( pPar->GetName().getString().getStr() ).WriteCharPtr( "\"," )
                // item name
               .WriteCharPtr( pPType->GetName().getString().getStr() ).WriteCharPtr( ")," ) << endl;
            if( !rBase.FindType( pPType, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pPType );
        }
        return (sal_uInt16)rList.size();
    }
    return 0;
}

sal_uInt16 SvMetaSlot::WriteSlotMap( const OString& rShellName, sal_uInt16 nCount,
                                SvSlotElementList& rSlotList,
                                size_t nStart,
                                const OString& rPrefix,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    // SlotId, if not specified generate from name
    OString slotId = GetSlotId().getString();

    sal_uInt16 nSCount = 0;
    if( IsMethod() )
    {
        SvMetaType * pType;
        SvMetaAttribute * pMethod = GetMethod();
        if( pMethod )
            pType = pMethod->GetType();
        else
            pType = GetType();

        nSCount = (sal_uInt16)pType->GetAttrCount();
    }

    WriteSlot( rShellName, nCount, slotId, rSlotList, nStart, rPrefix, rBase, rOutStm );
    return nSCount;
}

void SvMetaSlot::WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                             HelpIdTable&  rTable )
{
    sal_uLong nSId = GetSlotId().GetValue();
    if( rTable.find( nSId ) == rTable.end() )
    {
        rTable[ nSId ] = this;
        rOutStm.WriteCharPtr( "#define " ).WriteCharPtr( GetSlotId().getString().getStr() ).WriteChar( '\t' )
           .WriteCharPtr( OString::number(nSId).getStr() )
            << endl;
    }

    SvMetaTypeEnum * pEnum = PTR_CAST( SvMetaTypeEnum, GetType() );
    if( GetPseudoSlots() && pEnum )
    {
        for( sal_uLong n = 0; n < pEnum->Count(); ++n )
        {
            OString aValName = pEnum->GetObject( n )->GetName().getString();

            OStringBuffer aBuf;
            if( !GetPseudoPrefix().isEmpty() )
                aBuf.append(GetPseudoPrefix());
            else
                aBuf.append(GetSlotId().getString());
            aBuf.append('_');
            aBuf.append(aValName.copy(pEnum->GetPrefix().getLength()));

            OString aSId = aBuf.makeStringAndClear();

            sal_uLong nSId2;
            sal_Bool bIdOk = sal_False;
            if( rBase.FindId( aSId, &nSId2 ) )
            {
                aSId = OString::number(nSId2);
                bIdOk = sal_True;
            }

            // if id not found, write always
            if( !bIdOk || rTable.find( nSId2 ) == rTable.end() )
            {
                rTable[ nSId2 ] = this;

                rOutStm.WriteCharPtr( "#define " ).WriteCharPtr( aSId.getStr() ).WriteChar( '\t' )
                   .WriteCharPtr( OString::number(
                        nSId2).getStr() )
                    << endl;
            }
        }
    }
}

void WriteBool( sal_Bool bSet, SvStream& rStream )
{
    if ( bSet )
        rStream.WriteCharPtr( "TRUE" ).WriteChar( ',' );
    else
        rStream.WriteCharPtr( "FALSE" ).WriteChar( ',' );
}

void SvMetaSlot::WriteCSV( SvIdlDataBase& rBase, SvStream& rStrm )
{
    rStrm.WriteCharPtr( "PROJECT," );
    rStrm.WriteCharPtr( GetSlotId().getString().getStr() ).WriteChar( ',' );
    rStrm
       .WriteCharPtr( OString::number(
            GetSlotId().GetValue()).getStr() )
       .WriteChar( ',' );

    if ( !GetPseudoPrefix().isEmpty() )
        rStrm.WriteCharPtr( GetPseudoPrefix().getStr() ).WriteChar( ',' );
    else
        rStrm.WriteChar( ',' );

    rStrm.WriteCharPtr( GetGroupId().getStr() ).WriteChar( ',' );

    WriteBool( GetAccelConfig(), rStrm );
    WriteBool( GetMenuConfig(), rStrm );
    WriteBool( GetStatusBarConfig(), rStrm );
    WriteBool( GetToolBoxConfig(), rStrm );

    if ( GetSlotType() )
        rStrm.WriteCharPtr( GetSlotType()->GetName().getString().getStr() ).WriteChar( ',' );
    else
        rStrm.WriteChar( ',' );

    WriteBool( GetAutoUpdate(), rStrm );
    if ( GetCachable() )
        rStrm.WriteCharPtr( "Cachable" ).WriteChar( ',' );
    else
        rStrm.WriteCharPtr( "Volatile" ).WriteChar( ',' );

    WriteBool( GetContainer(), rStrm );
    WriteBool( GetFastCall(), rStrm );
    WriteBool( GetHasCoreId(), rStrm );
    WriteBool( GetHasDialog(), rStrm );
    WriteBool( GetReadOnlyDoc(), rStrm );
    WriteBool( GetImageRotation(), rStrm );
    WriteBool( GetImageReflection(), rStrm );
    rStrm.WriteCharPtr( GetDisableFlags().getStr() ).WriteChar( ',' );

    if( GetSynchron() )
        rStrm.WriteCharPtr( "Synchron" ).WriteChar( ',' );
    else
        rStrm.WriteCharPtr( "Asynchron" ).WriteChar( ',' );

    WriteBool( GetToggle(), rStrm );
    WriteBool( GetReadonly(), rStrm );
    WriteBool( GetExport(), rStrm );
    if( GetRecordPerItem() )
        rStrm.WriteCharPtr( "RecordPerItem" ).WriteChar( ',' );
    else if ( GetNoRecord() )
        rStrm.WriteCharPtr( "NoRecord" ).WriteChar( ',' );
    else if ( GetRecordManual() )
        rStrm.WriteCharPtr( "RecordManual" ).WriteChar( ',' );
    else
        rStrm.WriteCharPtr( "RecordPerSet" ).WriteChar( ',' );

    WriteBool( GetRecordAbsolute(), rStrm );

    if ( GetType()->GetType() != TYPE_METHOD && GetMethod() )
    {
        rStrm.WriteCharPtr( GetMethod()->GetType()->GetReturnType()->GetName().getString().getStr() ).WriteChar( ',' );
        rStrm.WriteCharPtr( GetMethod()->GetName().getString().getStr() ).WriteChar( ',' );
    }
    else
    {
        rStrm.WriteCharPtr( ",," );
    }

    rStrm.WriteCharPtr( GetType()->GetSvName().getStr() ).WriteChar( ',' );
    rStrm.WriteCharPtr( GetName().getString().getStr() ).WriteChar( ',' );

    if ( GetType()->GetType() == TYPE_METHOD || GetMethod() )
    {
        SvMetaAttributeMemberList *pList = &GetType()->GetAttrList();
           if ( GetMethod() )
            pList = &GetMethod()->GetType()->GetAttrList();

        if( pList && !pList->empty() )
        {
            rStrm.WriteCharPtr( "\"(" );
            SvMetaAttributeMemberList::const_iterator it = pList->begin();
            while( it != pList->end() )
            {
                SvMetaAttribute* pAttr = *it;
                pAttr->WriteCSV( rBase, rStrm );
                ++it;
                if( it != pList->end() )
                    rStrm.WriteChar( ',' );
            }
            rStrm.WriteCharPtr( ")\"" );
        }
        else
            rStrm.WriteCharPtr( "()" );
    }

    rStrm << endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
