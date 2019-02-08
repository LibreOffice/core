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

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <slot.hxx>
#include <globals.hxx>
#include <database.hxx>


SvMetaSlot::SvMetaSlot()
    : aRecordPerSet( true )
    , aRecordAbsolute( false )
    , pNextSlot(nullptr)
    , nListPos(0)
    , aReadOnlyDoc ( true )
    , aExport( true )
{
}

SvMetaSlot::SvMetaSlot( SvMetaType * pType )
    : SvMetaAttribute( pType )
    , aRecordPerSet( true )
    , aRecordAbsolute( false )
    , pNextSlot(nullptr)
    , nListPos(0)
    , aReadOnlyDoc ( true )
    , aExport( true )
{
}

bool SvMetaSlot::GetReadOnlyDoc() const
{
    if( aReadOnlyDoc.IsSet() || !GetRef() ) return aReadOnlyDoc;
    return static_cast<SvMetaSlot *>(GetRef())->GetReadOnlyDoc();
}

bool SvMetaSlot::GetExport() const
{
    if( aExport.IsSet() || !GetRef() ) return aExport;
    return static_cast<SvMetaSlot *>(GetRef())->GetExport();
}

bool SvMetaSlot::GetHidden() const
{
    // when export is set, but hidden is not the default is used
    if ( aExport.IsSet() )
        return !aExport;
    else if( !GetRef() )
        return false;
    else
        return static_cast<SvMetaSlot *>(GetRef())->GetHidden();
}

bool SvMetaSlot::IsVariable() const
{
    SvMetaType * pType = GetType();
    return pType->GetMetaTypeType() != MetaTypeType::Method;
}

bool SvMetaSlot::IsMethod() const
{
    SvMetaType * pType = GetType();
    return pType->GetMetaTypeType() == MetaTypeType::Method;
}

/*************************************************************************
|*    reference
|*
|*    description       Second FALSE in the SvBOOL-Objects means
|*                      IsSet() provides FALSE (default initialization).
*************************************************************************/
/** reference disbandment **/
const OString& SvMetaSlot::GetGroupId() const
{
    if( !aGroupId.getString().isEmpty() || !GetRef() ) return aGroupId.getString();
    return static_cast<SvMetaSlot *>(GetRef())->GetGroupId();
}
const OString& SvMetaSlot::GetDisableFlags() const
{
    if( !aDisableFlags.isEmpty() || !GetRef() ) return aDisableFlags;
    return static_cast<SvMetaSlot *>(GetRef())->GetDisableFlags();
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
bool SvMetaSlot::GetAsynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aAsynchron.IsSet() )
        return aAsynchron;
    return static_cast<SvMetaSlot *>(GetRef())->GetAsynchron();
}
bool SvMetaSlot::GetRecordPerItem() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() )
        return aRecordPerItem;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordPerItem();
}
bool SvMetaSlot::GetRecordPerSet() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() )
        return aRecordPerSet;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordPerSet();
}
bool SvMetaSlot::GetNoRecord() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet()  )
        return aNoRecord;
    return static_cast<SvMetaSlot *>(GetRef())->GetNoRecord();
}
bool SvMetaSlot::GetRecordAbsolute() const
{
    if( !GetRef() || aRecordAbsolute.IsSet() )
        return aRecordAbsolute;
    return static_cast<SvMetaSlot *>(GetRef())->GetRecordAbsolute();
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

void SvMetaSlot::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    SvMetaAttribute::ReadAttributesSvIdl( rBase, rInStm );

    aGroupId.ReadSvIdl( SvHash_GroupId(), rInStm );
    aExecMethod.ReadSvIdl( SvHash_ExecMethod(), rInStm );
    aStateMethod.ReadSvIdl( SvHash_StateMethod(), rInStm );
    ReadStringSvIdl( SvHash_DisableFlags(), rInStm, aDisableFlags );
    aReadOnlyDoc.ReadSvIdl( SvHash_ReadOnlyDoc(), rInStm );
    aExport.ReadSvIdl( SvHash_Export(), rInStm );
    aToggle.ReadSvIdl( SvHash_Toggle(), rInStm );
    aAutoUpdate.ReadSvIdl( SvHash_AutoUpdate(), rInStm );
    aAsynchron.ReadSvIdl( SvHash_Asynchron(), rInStm );
    aRecordAbsolute.ReadSvIdl( SvHash_RecordAbsolute(), rInStm );

    if( aRecordPerItem.ReadSvIdl( SvHash_RecordPerItem(), rInStm ) )
    {
        SetRecordPerItem( aRecordPerItem );
    }
    if( aRecordPerSet.ReadSvIdl( SvHash_RecordPerSet(), rInStm ) )
    {
        SetRecordPerSet( aRecordPerSet );
    }
    if( aNoRecord.ReadSvIdl( SvHash_NoRecord(), rInStm ) )
    {
        SetNoRecord( aNoRecord );
    }

    aMenuConfig.ReadSvIdl( SvHash_MenuConfig(), rInStm );
    aToolBoxConfig.ReadSvIdl( SvHash_ToolBoxConfig(), rInStm );
    aAccelConfig.ReadSvIdl( SvHash_AccelConfig(), rInStm );

    aFastCall.ReadSvIdl( SvHash_FastCall(), rInStm );
    aContainer.ReadSvIdl( SvHash_Container(), rInStm );
}

bool SvMetaSlot::Test( SvTokenStream & rInStm )
{
    bool bOk = SvMetaAttribute::Test( rInStm );
    if( bOk )
    {
        SvMetaType * pType = GetType();
        if( pType->GetMetaTypeType() == MetaTypeType::Method )
            pType = pType->GetReturnType();
        if( !pType->IsItem() )
        {
            throw SvParseException( rInStm, "this attribute is not a slot" );
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
        if( !pKnownSlot )
            throw SvParseException( rInStm, "attribute " + pAttr->GetName() + " is method or variable but not a slot" );
        SetRef( pKnownSlot );
        SetName( pKnownSlot->GetName() );
        bOk = SvMetaObject::ReadSvIdl( rBase, rInStm );
    }
    else
    {
        bOk = SvMetaAttribute::ReadSvIdl( rBase, rInStm );

        SvMetaAttribute *pAttr2 = rBase.FindKnownAttr( GetSlotId() );
        if( pAttr2 )
        {
            // for testing purposes: reference in case of complete definition
            SvMetaSlot * pKnownSlot = dynamic_cast<SvMetaSlot*>( pAttr2  );
            if( !pKnownSlot )
                throw SvParseException( rInStm, "attribute " + pAttr2->GetName() + " is method or variable but not a slot" );
            SetRef( pKnownSlot );

              // names may differ, because explicitly given
            if ( pKnownSlot->GetName() != GetName() )
            {
                OSL_FAIL("Illegal definition!");
                rInStm.Seek( nTokPos );
                return false;
            }

            SetName( pKnownSlot->GetName() );
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );

    return bOk;
}

void SvMetaSlot::Insert( SvSlotElementList& rList)
{
    // get insert position through binary search in slotlist
    sal_uInt16 nId = static_cast<sal_uInt16>(GetSlotId().GetValue());
    sal_uInt16 nListCount = static_cast<sal_uInt16>(rList.size());
    sal_uInt16 nPos;

    if ( !nListCount )
        nPos = 0;
    else if ( nListCount == 1 )
        nPos = rList[ 0 ]->GetSlotId().GetValue() >= nId ? 0 : 1;
    else
    {
        sal_uInt16 nMid = 0, nLow = 0;
        sal_uInt16 nHigh = nListCount - 1;
        bool bFound = false;
        while ( !bFound && nLow <= nHigh )
        {
            nMid = (nLow + nHigh) >> 1;
            DBG_ASSERT( nMid < nListCount, "bsearch is buggy" );
            int nDiff = static_cast<int>(nId) - static_cast<int>(rList[ nMid ]->GetSlotId().GetValue());
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
        static_cast<sal_uInt16>(rList[ nPos ]->GetSlotId().GetValue()),
        "Successor has lower SlotId" );
    DBG_ASSERT( nPos == 0 || nId >
        static_cast<sal_uInt16>(rList[ nPos-1 ]->GetSlotId().GetValue()),
        "Predecessor has higher SlotId" );
    DBG_ASSERT( nPos+1 >= nListCount || nId <
        static_cast<sal_uInt16>(rList[ nPos+1 ]->GetSlotId().GetValue()),
        "Successor has lower SlotId" );

    if ( nPos < rList.size() )
    {
        SvSlotElementList::iterator it = rList.begin();
        std::advance( it, nPos );
        rList.insert( it, this );
    }
    else
    {
        rList.push_back( this );
    }
}


static OString MakeSlotName( SvStringHashEntry const * pEntry )
{
    return "SfxSlotMode::" + pEntry->GetName().toAsciiUpperCase();
};

void SvMetaSlot::WriteSlotStubs( const OString& rShellName,
                                std::vector<OString> & rList,
                                SvStream & rOutStm ) const
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
            if (rList[n] == aMethodName)
            {
                bIn = true;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( aMethodName );
            rOutStm.WriteCharPtr( "SFX_EXEC_STUB(" )
                   .WriteOString( rShellName )
                   .WriteChar( ',' )
                   .WriteOString( aMethodName )
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
            if (rList[n] == aMethodName)
            {
                bIn = true;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( aMethodName );
            rOutStm.WriteCharPtr( "SFX_STATE_STUB(" )
                   .WriteOString( rShellName )
                   .WriteChar( ',' )
                   .WriteOString( aMethodName )
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

    rOutStm.WriteCharPtr( "// Slot Nr. " )
       .WriteOString( OString::number(nListPos) )
       .WriteCharPtr( " : " );
    OString aSlotIdValue(OString::number(GetSlotId().GetValue()));
    rOutStm.WriteOString( aSlotIdValue ) << endl;
    WriteTab( rOutStm, 1 );
    rOutStm.WriteCharPtr( "SFX_NEW_SLOT_ARG( " ).WriteOString( rShellName ).WriteChar( ',' ) ;

    rOutStm.WriteOString( rSlotId ).WriteChar( ',' );

    // GroupId
    if( !GetGroupId().isEmpty() )
        rOutStm.WriteOString( GetGroupId() );
    else
        rOutStm.WriteCharPtr( "SfxGroupId::NONE" );
    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 4 );

    // look for the next slot with the same StateMethod like me
    // the slotlist is set to the current slot
    size_t i = nStart;
    SvMetaSlot* pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : nullptr;
    pNextSlot = pEle;
    while ( pNextSlot )
    {
        if ( !pNextSlot->pNextSlot &&
            pNextSlot->GetStateMethod() == GetStateMethod()
        ) {
            break;
        }
        pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : nullptr;
        pNextSlot = pEle;
    }

    if ( !pNextSlot )
    {
        // There is no slot behind me that has the same ExecMethod.
        // So I search for the first slot with it (could be myself).
        i = 0;
        pEle = rSlotList.empty() ? nullptr : rSlotList[ i ];
        pNextSlot = pEle;
        while (pNextSlot && pNextSlot != this)
        {
            if ( pNextSlot->GetStateMethod() == GetStateMethod() )
                break;
            pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : nullptr;
            pNextSlot = pEle;
        }
    }

    assert(pNextSlot);

    rOutStm.WriteCharPtr( "&a" ).WriteOString( rShellName ).WriteCharPtr( "Slots_Impl[" )
       .WriteOString( OString::number(pNextSlot->GetListPos()) )
       .WriteCharPtr( "] /*Offset Next*/, " ) << endl;

    WriteTab( rOutStm, 4 );

    // write ExecMethod, with standard name if not specified
    if( !GetExecMethod().isEmpty() &&
        GetExecMethod() != "NoExec")
    {
        rOutStm.WriteCharPtr( "SFX_STUB_PTR(" ).WriteOString( rShellName ).WriteChar( ',' )
               .WriteOString( GetExecMethod() ).WriteChar( ')' );
    }
    else
        rOutStm.WriteCharPtr( "SFX_STUB_PTR_EXEC_NONE" );
    rOutStm.WriteChar( ',' );

    // write StateMethod, with standard name if not specified
    if( !GetStateMethod().isEmpty() &&
        GetStateMethod() != "NoState")
    {
        rOutStm.WriteCharPtr( "SFX_STUB_PTR(" ).WriteOString( rShellName ).WriteChar( ',' )
               .WriteOString( GetStateMethod() ).WriteChar( ')' );
    }
    else
        rOutStm.WriteCharPtr( "SFX_STUB_PTR_STATE_NONE" );

    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 4 );

    // write flags
    if( GetToggle() )
        rOutStm.WriteOString( MakeSlotName( SvHash_Toggle() ) ).WriteChar( '|' );
    if( GetAutoUpdate() )
        rOutStm.WriteOString( MakeSlotName( SvHash_AutoUpdate() ) ).WriteChar( '|' );
    if( GetAsynchron() )
        rOutStm.WriteOString( MakeSlotName( SvHash_Asynchron() ) ).WriteChar( '|' );
    if( GetRecordPerItem() )
        rOutStm.WriteOString( MakeSlotName( SvHash_RecordPerItem() ) ).WriteChar( '|' );
    if( GetRecordPerSet() )
        rOutStm.WriteOString( MakeSlotName( SvHash_RecordPerSet() ) ).WriteChar( '|' );
    if( GetNoRecord() )
        rOutStm.WriteOString( MakeSlotName( SvHash_NoRecord() ) ).WriteChar( '|' );
    if( GetRecordAbsolute() )
        rOutStm.WriteOString( MakeSlotName( SvHash_RecordAbsolute() ) ).WriteChar( '|' );
    if( GetMenuConfig() )
        rOutStm.WriteOString( MakeSlotName( SvHash_MenuConfig() ) ).WriteChar( '|' );
    if( GetToolBoxConfig() )
        rOutStm.WriteOString( MakeSlotName( SvHash_ToolBoxConfig() ) ).WriteChar( '|' );
    if( GetAccelConfig() )
        rOutStm.WriteOString( MakeSlotName( SvHash_AccelConfig() ) ).WriteChar( '|' );
    if( GetFastCall() )
        rOutStm.WriteOString( MakeSlotName( SvHash_FastCall() ) ).WriteChar( '|' );
    if( GetContainer() )
        rOutStm.WriteOString( MakeSlotName( SvHash_Container() ) ).WriteChar( '|' );
    if ( GetReadOnlyDoc() )
        rOutStm.WriteOString( MakeSlotName( SvHash_ReadOnlyDoc() ) ).WriteChar( '|' );
    rOutStm.WriteCharPtr( "SfxSlotMode::NONE" );

    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 4 );
    if ( GetDisableFlags().isEmpty() )
        rOutStm.WriteCharPtr( "SfxDisableFlags::NONE" );
    else
        rOutStm.WriteOString( GetDisableFlags() );

    // write attribute type
    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 4 );

    SvMetaType * pT = GetType();
    if( !IsVariable() )
    {
        SvMetaType * pRT = GetType()->GetReturnType();
        pT =  pRT ? pRT : rBase.FindType( "SfxVoidItem" );
    }

    if( pT )
    {
        assert(pT->IsItem());
        rOutStm.WriteOString( pT->GetName() );
        if( !SvIdlDataBase::FindType( pT, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pT );
    }
    else
        rOutStm.WriteCharPtr( "SfxVoidItem not defined" );

    {
        rOutStm.WriteChar( ',' ) << endl;
        WriteTab( rOutStm, 4 );
        rOutStm
           .WriteOString( OString::number(nCount) )
           .WriteCharPtr( "/*Offset*/, " );

        if( IsMethod() )
        {
            SvMetaType * pType = GetType();
            sal_uLong nSCount = pType->GetAttrCount();
            rOutStm
               .WriteOString( OString::number(nSCount) )
               .WriteCharPtr( "/*Count*/," );
        }
        else
            rOutStm.WriteCharPtr( "0," );

        rOutStm.WriteCharPtr( " " );

        // Method/Property flags
        if( IsMethod() )
            rOutStm.WriteCharPtr( "SfxSlotMode::METHOD|" );

        rOutStm.WriteCharPtr( "SfxSlotMode::NONE" );
    }

    {
        rOutStm.WriteCharPtr( ",\"" );
        rOutStm.WriteOString( GetName() );
        rOutStm.WriteCharPtr( "\"" );
    }

    rOutStm.WriteCharPtr( " )," ) << endl;
}

sal_uInt16 SvMetaSlot::WriteSlotParamArray( SvIdlDataBase & rBase, SvStream & rOutStm ) const
{
    if ( !GetExport() && !GetHidden() )
        return 0;

    if( IsMethod() )
    {
        SvMetaType * pType = GetType();

        if( !SvIdlDataBase::FindType( pType, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pType );

        const SvRefMemberList<SvMetaAttribute *>& rList =
                    pType->GetAttrList();
        for( sal_uLong n = 0; n < rList.size(); n++ )
        {
            SvMetaAttribute * pPar  = rList[n];
            SvMetaType * pPType     = pPar->GetType();
            WriteTab( rOutStm, 1 );
            rOutStm.WriteCharPtr("{ (const SfxType*) &a")
                // item type
               .WriteOString(pPType->GetName()).WriteCharPtr("_Impl, ")
                // parameter name
               .WriteCharPtr("\"").WriteOString(pPar->GetName()).WriteCharPtr("\", ")
                // slot id
               .WriteOString(pPar->GetSlotId().getString()).WriteCharPtr(" },") << endl;
            if( !SvIdlDataBase::FindType( pPType, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pPType );
        }
        return static_cast<sal_uInt16>(rList.size());
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
        SvMetaType * pType = GetType();
        nSCount = static_cast<sal_uInt16>(pType->GetAttrCount());
    }

    WriteSlot( rShellName, nCount, slotId, rSlotList, nStart, rBase, rOutStm );
    return nSCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
