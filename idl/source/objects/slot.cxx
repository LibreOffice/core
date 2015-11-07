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
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <slot.hxx>
#include <globals.hxx>
#include <database.hxx>


SvMetaObject *SvMetaSlot::MakeClone() const
{
        return new SvMetaSlot( *this );
}

SvMetaSlot::SvMetaSlot()
    : aCachable( true, false )
    , aSynchron( true, false )
    , aRecordPerSet( true, false )
    , aRecordAbsolute( false, false )
    , pLinkedSlot(nullptr)
    , pNextSlot(nullptr)
    , nListPos(0)
    , pEnumValue(nullptr)
{
}

SvMetaSlot::SvMetaSlot( SvMetaType * pType )
    : SvMetaAttribute( pType )
    , aCachable( true, false )
    , aSynchron( true, false )
    , aRecordPerSet( true, false )
    , aRecordAbsolute( false, false )
    , pLinkedSlot(nullptr)
    , pNextSlot(nullptr)
    , nListPos(0)
    , pEnumValue(nullptr)
{
}

bool SvMetaSlot::IsVariable() const
{
    return SvMetaAttribute::IsVariable();
}

bool SvMetaSlot::IsMethod() const
{
    bool b = SvMetaAttribute::IsMethod();
    b |= nullptr != GetMethod();
    return b;
}

OString SvMetaSlot::GetMangleName( bool bVariable ) const
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
    return static_cast<SvMetaSlot *>(GetRef())->GetSlotType();
}
SvMetaAttribute * SvMetaSlot::GetMethod() const
{
    if( aMethod.Is() || !GetRef() ) return aMethod;
    return static_cast<SvMetaSlot *>(GetRef())->GetMethod();
}
bool SvMetaSlot::GetHasCoreId() const
{
    if( aHasCoreId.IsSet() || !GetRef() ) return aHasCoreId;
    return static_cast<SvMetaSlot *>(GetRef())->GetHasCoreId();
}
const OString& SvMetaSlot::GetGroupId() const
{
    if( !aGroupId.getString().isEmpty() || !GetRef() ) return aGroupId.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetGroupId();
}
const OString& SvMetaSlot::GetDisableFlags() const
{
    if( !aDisableFlags.getString().isEmpty() || !GetRef() ) return aDisableFlags.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetDisableFlags();
}
const OString& SvMetaSlot::GetConfigId() const
{
    if( !aConfigId.getString().isEmpty() || !GetRef() ) return aConfigId.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetConfigId();
}
const OString& SvMetaSlot::GetExecMethod() const
{
    if( !aExecMethod.getString().isEmpty() || !GetRef() ) return aExecMethod.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetExecMethod();
}
const OString& SvMetaSlot::GetStateMethod() const
{
    if( !aStateMethod.getString().isEmpty() || !GetRef() ) return aStateMethod.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetStateMethod();
}
const OString& SvMetaSlot::GetDefault() const
{
    if( !aDefault.getString().isEmpty() || !GetRef() ) return aDefault.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetDefault();
}
bool SvMetaSlot::GetPseudoSlots() const
{
    if( aPseudoSlots.IsSet() || !GetRef() ) return aPseudoSlots;
    return static_cast<SvMetaSlot *>(GetRef())->GetPseudoSlots();
}
bool SvMetaSlot::GetCachable() const
{
    // Cachable and Volatile are exclusive
    if( !GetRef() || aCachable.IsSet() || aVolatile.IsSet() )
         return aCachable;
    return static_cast<SvMetaSlot *>(GetRef())->GetCachable();
}
bool SvMetaSlot::GetVolatile() const
{
    // Cachable and Volatile are exclusive
    if( !GetRef() || aVolatile.IsSet() || aCachable.IsSet() )
        return aVolatile;
    return static_cast<SvMetaSlot *>(GetRef())->GetVolatile();
}
bool SvMetaSlot::GetToggle() const
{
    if( aToggle.IsSet() || !GetRef() ) return aToggle;
    return static_cast<SvMetaSlot *>(GetRef())->GetToggle();
}
bool SvMetaSlot::GetAutoUpdate() const
{
    if( aAutoUpdate.IsSet() || !GetRef() ) return aAutoUpdate;
    return static_cast<SvMetaSlot *>(GetRef())->GetAutoUpdate();
}
bool SvMetaSlot::GetSynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aSynchron.IsSet() || aAsynchron.IsSet() )
        return aSynchron;
    return static_cast<SvMetaSlot *>(GetRef())->GetSynchron();
}
bool SvMetaSlot::GetAsynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aAsynchron.IsSet() || aSynchron.IsSet() )
        return aAsynchron;
    return static_cast<SvMetaSlot *>(GetRef())->GetAsynchron();
}
bool SvMetaSlot::GetRecordPerItem() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerItem;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordPerItem();
}
bool SvMetaSlot::GetRecordPerSet() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerSet;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordPerSet();
}
bool SvMetaSlot::GetRecordManual() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordManual;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordManual();
}
bool SvMetaSlot::GetNoRecord() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aNoRecord;
    return static_cast<SvMetaSlot *>(GetRef())->GetNoRecord();
}
bool SvMetaSlot::GetRecordAbsolute() const
{
    if( !GetRef() || aRecordAbsolute.IsSet() )
        return aRecordAbsolute;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordAbsolute();
}
bool SvMetaSlot::GetHasDialog() const
{
    if( aHasDialog.IsSet() || !GetRef() ) return aHasDialog;
    return static_cast<SvMetaSlot *>(GetRef())->GetHasDialog();
}
const OString& SvMetaSlot::GetPseudoPrefix() const
{
    if( !aPseudoPrefix.getString().isEmpty() || !GetRef() ) return aPseudoPrefix.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetPseudoPrefix();
}
bool SvMetaSlot::GetMenuConfig() const
{
    if( aMenuConfig.IsSet() || !GetRef() ) return aMenuConfig;
    return static_cast<SvMetaSlot *>(GetRef())->GetMenuConfig();
}
bool SvMetaSlot::GetToolBoxConfig() const
{
    if( aToolBoxConfig.IsSet() || !GetRef() ) return aToolBoxConfig;
    return static_cast<SvMetaSlot *>(GetRef())->GetToolBoxConfig();
}
bool SvMetaSlot::GetStatusBarConfig() const
{
    if( aStatusBarConfig.IsSet() || !GetRef() ) return aStatusBarConfig;
    return static_cast<SvMetaSlot *>(GetRef())->GetStatusBarConfig();
}
bool SvMetaSlot::GetAccelConfig() const
{
    if( aAccelConfig.IsSet() || !GetRef() ) return aAccelConfig;
    return static_cast<SvMetaSlot *>(GetRef())->GetAccelConfig();
}
bool SvMetaSlot::GetFastCall() const
{
    if( aFastCall.IsSet() || !GetRef() ) return aFastCall;
    return static_cast<SvMetaSlot *>(GetRef())->GetFastCall();
}
bool SvMetaSlot::GetContainer() const
{
    if( aContainer.IsSet() || !GetRef() ) return aContainer;
    return static_cast<SvMetaSlot *>(GetRef())->GetContainer();
}

bool SvMetaSlot::GetImageRotation() const
{
    if( aImageRotation.IsSet() || !GetRef() ) return aImageRotation;
    return static_cast<SvMetaSlot *>(GetRef())->GetImageRotation();
}

bool SvMetaSlot::GetImageReflection() const
{
    if( aImageReflection.IsSet() || !GetRef() ) return aImageReflection;
    return static_cast<SvMetaSlot *>(GetRef())->GetImageReflection();
}

const OString& SvMetaSlot::GetUnoName() const
{
    if( aUnoName.IsSet() || !GetRef() ) return aUnoName.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetUnoName();
}

void SvMetaSlot::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    SvMetaAttribute::ReadAttributesSvIdl( rBase, rInStm );

    bool bOk = false;
    bOk |= aDefault.ReadSvIdl( SvHash_Default(), rInStm );
    bOk |= aPseudoSlots.ReadSvIdl( SvHash_PseudoSlots(), rInStm );
    bOk |= aHasCoreId.ReadSvIdl( SvHash_HasCoreId(), rInStm );
    bOk |= aGroupId.ReadSvIdl( SvHash_GroupId(), rInStm );
    bOk |= aExecMethod.ReadSvIdl( SvHash_ExecMethod(), rInStm );
    bOk |= aStateMethod.ReadSvIdl( SvHash_StateMethod(), rInStm );
    bOk |= aDisableFlags.ReadSvIdl( SvHash_DisableFlags(), rInStm );
    if( aGet.ReadSvIdl( SvHash_Get(), rInStm ) )
    {
        SvIdlDataBase::WriteError( "warning", OUStringToOString(rInStm.GetFileName(), RTL_TEXTENCODING_UTF8),
                    "<Get> old style, use Readonly",
                    rInStm.GetToken().GetLine(),
                    rInStm.GetToken().GetColumn() );
    }
    if( aSet.ReadSvIdl( SvHash_Set(), rInStm ) )
    {
        SvIdlDataBase::WriteError( "warning", OUStringToOString(rInStm.GetFileName(), RTL_TEXTENCODING_UTF8),
                    "<Set> old style, use method declaration",
                    rInStm.GetToken().GetLine(),
                    rInStm.GetToken().GetColumn() );
    }

    if( aCachable.ReadSvIdl( SvHash_Cachable(), rInStm ) )
        SetCachable( aCachable ), bOk = true;
    if( aVolatile.ReadSvIdl( SvHash_Volatile(), rInStm ) )
        SetVolatile( aVolatile ), bOk = true;
    if( aToggle.ReadSvIdl( SvHash_Toggle(), rInStm ) )
        SetToggle( aToggle ), bOk = true;
    if( aAutoUpdate.ReadSvIdl( SvHash_AutoUpdate(), rInStm ) )
        SetAutoUpdate( aAutoUpdate ), bOk = true;

    if( aSynchron.ReadSvIdl( SvHash_Synchron(), rInStm ) )
        SetSynchron( aSynchron ), bOk = true;
    if( aAsynchron.ReadSvIdl( SvHash_Asynchron(), rInStm ) )
        SetAsynchron( aAsynchron ), bOk = true;

    if( aRecordAbsolute.ReadSvIdl( SvHash_RecordAbsolute(), rInStm ) )
        SetRecordAbsolute( aRecordAbsolute), bOk = true;
    if( aRecordPerItem.ReadSvIdl( SvHash_RecordPerItem(), rInStm ) )
        SetRecordPerItem( aRecordPerItem ), bOk = true;
    if( aRecordPerSet.ReadSvIdl( SvHash_RecordPerSet(), rInStm ) )
        SetRecordPerSet( aRecordPerSet ), bOk = true;
    if( aRecordManual.ReadSvIdl( SvHash_RecordManual(), rInStm ) )
        SetRecordManual( aRecordManual ), bOk = true;
    if( aNoRecord.ReadSvIdl( SvHash_NoRecord(), rInStm ) )
        SetNoRecord( aNoRecord ), bOk = true;

    bOk |= aHasDialog.ReadSvIdl( SvHash_HasDialog(), rInStm );
    bOk |= aPseudoPrefix.ReadSvIdl( SvHash_PseudoPrefix(), rInStm );
    bOk |= aMenuConfig.ReadSvIdl( SvHash_MenuConfig(), rInStm );
    bOk |= aToolBoxConfig.ReadSvIdl( SvHash_ToolBoxConfig(), rInStm );
    bOk |= aStatusBarConfig.ReadSvIdl( SvHash_StatusBarConfig(), rInStm );
    bOk |= aAccelConfig.ReadSvIdl( SvHash_AccelConfig(), rInStm );

    SvBOOL aAllConfig;
    if( aAllConfig.ReadSvIdl( SvHash_AllConfig(), rInStm ) )
        SetAllConfig( aAllConfig ), bOk = true;
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
                bool bBraket = rInStm.Read( '(' );
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
            SvToken& rTok = rInStm.GetToken();
            if( rTok.IsIdentifier() )
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

bool SvMetaSlot::Test( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    bool bOk = SvMetaAttribute::Test( rBase, rInStm );
    if( bOk )
    {
        SvMetaType * pType = GetType();
        if( pType->GetType() == TYPE_METHOD )
            pType = pType->GetReturnType();
        if( !pType->IsItem() )
        {
            rBase.SetError( "this attribute is not a slot", rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = false;
        }
    }

    return bOk;
}

bool SvMetaSlot::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos     = rInStm.Tell();
    bool bOk = true;

    SvMetaAttribute * pAttr = rBase.ReadKnownAttr( rInStm, GetType() );
    if( pAttr )
    {
        // c
        SvMetaSlot * pKnownSlot = dynamic_cast<SvMetaSlot*>( pAttr  );
        if( pKnownSlot )
        {
            SetRef( pKnownSlot );
            SetName( pKnownSlot->GetName().getString(), &rBase );
            bOk = SvMetaObject::ReadSvIdl( rBase, rInStm );
        }
        else
        {
            OStringBuffer aStr( "attribute " );
            aStr.append(pAttr->GetName().getString());
            aStr.append(" is method or variable but not a slot");
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = false;
        }
    }
    else
    {
        bOk = SvMetaAttribute::ReadSvIdl( rBase, rInStm );

        SvMetaAttribute *pAttr2 = rBase.SearchKnownAttr( GetSlotId() );
        if( pAttr2 )
        {
            // for testing purposes: reference in case of complete definition
            SvMetaSlot * pKnownSlot = dynamic_cast<SvMetaSlot*>( pAttr2  );
            if( pKnownSlot )
            {
                SetRef( pKnownSlot );

                  // names may differ, because explicitly given
                if ( pKnownSlot->GetName().getString() != GetName().getString() )
                {
                    OSL_FAIL("Illegal definition!");
                    rInStm.Seek( nTokPos );
                    return false;
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
                bOk = false;
            }
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );

    return bOk;
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
        bool bFound = false;
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
                bFound = true;
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
    SvMetaTypeEnum * pEnum = nullptr;
    SvMetaType * pBType = GetType()->GetBaseType();
    pEnum = dynamic_cast<SvMetaTypeEnum*>( pBType  );
    if( GetPseudoSlots() && pEnum && pEnum->Count() )
    {
        // clone the MasterSlot
        SvMetaSlotRef xEnumSlot;
        SvMetaSlot *pFirstEnumSlot = nullptr;
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

            xEnumSlot = nullptr;
            for( m=0; m<rBase.GetAttrList().size(); m++ )
            {
                SvMetaAttribute * pAttr = rBase.GetAttrList()[m];
                if (aSId.equals(pAttr->GetSlotId().getString()))
                {
                    SvMetaSlot& rSlot = dynamic_cast<SvMetaSlot&>(*pAttr);
                    xEnumSlot = rSlot.Clone();
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
                    SvIdentifier aId;
                    aId.setString(aSId);
                    aId.SetValue(nValue);
                    xEnumSlot->SetSlotId(aId);
                }
            }

            // The slaves are no master!
            xEnumSlot->aPseudoSlots = false;
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
            pEle = ( ++i < rList.size() ) ? rList[ i ] : nullptr;
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
    OStringBuffer aName("SfxSlotMode::");
    aName.append(pEntry->GetName().toAsciiUpperCase());
    return aName.makeStringAndClear();
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
        bool bIn = false;
        for( size_t n = 0; n < rList.size(); n++ )
        {
            if (rList[n]->equals(aMethodName))
            {
                bIn = true;
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
        bool bIn = false;
        for ( size_t n=0; n < rList.size(); n++ )
        {
            if (rList[n]->equals(aMethodName))
            {
                bIn = true;
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
                            SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    bool bIsEnumSlot = nullptr != pEnumValue;

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
        SvSlotElement* pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : nullptr;
        pNextSlot = pEle ? &pEle->xSlot : nullptr;
        while ( pNextSlot )
        {
            if ( !pNextSlot->pNextSlot &&
                pNextSlot->GetStateMethod() == GetStateMethod()
            ) {
                break;
            }
            pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : nullptr;
            pNextSlot = pEle ? &pEle->xSlot : nullptr;
        }

        if ( !pNextSlot )
        {
            // There is no slot behind me that has the same ExecMethod.
            // So I search for the first slot with it (could be myself).
            i = 0;
            pEle = rSlotList.empty() ? nullptr : rSlotList[ i ];
            pNextSlot = pEle ? &pEle->xSlot : nullptr;
            while ( pNextSlot != this )
            {
                if ( !pNextSlot->pEnumValue &&
                    pNextSlot->GetStateMethod() == GetStateMethod() )
                    break;
                pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : nullptr;
                pNextSlot = pEle ? &pEle->xSlot : nullptr;
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
    rOutStm.WriteCharPtr( "SfxSlotMode::NONE" );

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
            if( !SvIdlDataBase::FindType( pT, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pT );
        }
        else
            rOutStm.WriteCharPtr( "SfxVoidItem not defined" );
    }
    else
    {
        SvMetaType *pT = rBase.FindType( "SfxBoolItem" );
        if ( pT && !SvIdlDataBase::FindType( pT, rBase.aUsedTypes ) )
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
               .WriteCharPtr( "/*Count*/," );
        }
        else
            rOutStm.WriteCharPtr( "0," );

        rOutStm.WriteCharPtr( " " );

        // Method/Property flags
        if( IsMethod() )
            rOutStm.WriteCharPtr( "SfxSlotMode::METHOD|" );
        if( IsVariable() )
        {
            rOutStm.WriteCharPtr( "SfxSlotMode::PROPGET|" );
            if( !GetReadonly() )
                rOutStm.WriteCharPtr( "SfxSlotMode::PROPSET|" );
        }

        rOutStm.WriteCharPtr( "SfxSlotMode::NONE" );
    }

    {
        rOutStm.WriteCharPtr( ",\"" );
        rOutStm.WriteCharPtr( GetMangleName( false ).getStr() );
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

        if( !SvIdlDataBase::FindType( pType, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pType );

        const SvMetaAttributeMemberList & rList =
                    pType->GetAttrList();
        for( sal_uLong n = 0; n < rList.size(); n++ )
        {
            SvMetaAttribute * pPar  = rList[n];
            SvMetaType * pPType     = pPar->GetType();
            WriteTab( rOutStm, 1 );
            rOutStm.WriteCharPtr("{ (const SfxType*) &a")
                // item type
               .WriteCharPtr(pPType->GetName().getString().getStr()).WriteCharPtr("_Impl, ")
                // parameter name
               .WriteCharPtr("\"").WriteCharPtr(pPar->GetName().getString().getStr()).WriteCharPtr("\", ")
                // slot id
               .WriteCharPtr(pPar->GetSlotId().getString().getStr()).WriteCharPtr(" },") << endl;
            if( !SvIdlDataBase::FindType( pPType, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pPType );
        }
        return (sal_uInt16)rList.size();
    }
    return 0;
}

sal_uInt16 SvMetaSlot::WriteSlotMap( const OString& rShellName, sal_uInt16 nCount,
                                SvSlotElementList& rSlotList,
                                size_t nStart,
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

    WriteSlot( rShellName, nCount, slotId, rSlotList, nStart, rBase, rOutStm );
    return nSCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
