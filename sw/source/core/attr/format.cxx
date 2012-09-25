/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <doc.hxx>
#include <fmtcolfunc.hxx>
#include <frame.hxx>
#include <format.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <paratr.hxx>
#include <rtl/strbuf.hxx>
#include <swcache.hxx>

TYPEINIT1( SwFmt, SwClient );

SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
              const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm,
              sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    aSet( rPool, pWhichRanges ),
    nWhichId( nFmtWhich ),
    nFmtId( 0 ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    aFmtName.AssignAscii( pFmtNm );
    bWritten = bFmtInDTOR = bAutoUpdateFmt = sal_False; // LAYER_IMPL
    bAutoFmt = sal_True;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}

SwFmt::SwFmt( SwAttrPool& rPool, const String& rFmtNm,
              const sal_uInt16* pWhichRanges, SwFmt* pDrvdFrm,
              sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    aFmtName( rFmtNm ),
    aSet( rPool, pWhichRanges ),
    nWhichId( nFmtWhich ),
    nFmtId( 0 ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    bWritten = bFmtInDTOR = bAutoUpdateFmt = sal_False; // LAYER_IMPL
    bAutoFmt = sal_True;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}

SwFmt::SwFmt( const SwFmt& rFmt )
    : SwModify( rFmt.DerivedFrom() ),
    aFmtName( rFmt.aFmtName ),
    aSet( rFmt.aSet ),
    nWhichId( rFmt.nWhichId ),
    nFmtId( 0 ),
    nPoolFmtId( rFmt.GetPoolFmtId() ),
    nPoolHelpId( rFmt.GetPoolHelpId() ),
    nPoolHlpFileId( rFmt.GetPoolHlpFileId() )
{
    bWritten = bFmtInDTOR = sal_False; // LAYER_IMPL
    bAutoFmt = rFmt.bAutoFmt;
    bAutoUpdateFmt = rFmt.bAutoUpdateFmt;

    if( rFmt.DerivedFrom() )
        aSet.SetParent( &rFmt.DerivedFrom()->aSet );
    // a few special treatments for attributes
    aSet.SetModifyAtAttr( this );
}

SwFmt &SwFmt::operator=(const SwFmt& rFmt)
{
    nWhichId = rFmt.nWhichId;
    nPoolFmtId = rFmt.GetPoolFmtId();
    nPoolHelpId = rFmt.GetPoolHelpId();
    nPoolHlpFileId = rFmt.GetPoolHlpFileId();

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // copy only array with attributes delta
    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    aSet.Intersect_BC( rFmt.aSet, &aOld, &aNew );
    aSet.Put_BC( rFmt.aSet, &aOld, &aNew );

    // a few special treatments for attributes
    aSet.SetModifyAtAttr( this );

    // create PoolItem attribute for Modify
    if( aOld.Count() )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }

    if( GetRegisteredIn() != rFmt.GetRegisteredIn() )
    {
        if( GetRegisteredIn() )
            GetRegisteredInNonConst()->Remove(this);
        if( rFmt.GetRegisteredIn() )
        {
            const_cast<SwFmt&>(rFmt).GetRegisteredInNonConst()->Add(this);
            aSet.SetParent( &rFmt.aSet );
        }
        else
        {
            aSet.SetParent( 0 );
        }
    }
    bAutoFmt = rFmt.bAutoFmt;
    bAutoUpdateFmt = rFmt.bAutoUpdateFmt;
    return *this;
}

void SwFmt::SetName( const String& rNewName, sal_Bool bBroadcast )
{
    OSL_ENSURE( !IsDefault(), "SetName: Defaultformat" );
    if( bBroadcast )
    {
        SwStringMsgPoolItem aOld( RES_NAME_CHANGED, aFmtName );
        SwStringMsgPoolItem aNew( RES_NAME_CHANGED, rNewName );
        aFmtName = rNewName;
        ModifyNotification( &aOld, &aNew );
    }
    else
    {
        aFmtName = rNewName;
    }
}

/** Copy attributes

    This function is called in every Copy-Ctor for copying the attributes.
    The latter can be only copied as soon as the derived class exists since
    for setting them the Which() function is called and that has the default
    value of 0 in the base class and is then overridden by the derived class.

    If we copy over multiple documents then the new document has to be provided
    in which <this> is defined. Currently this is important for DropCaps
    because that contains data that needs to be copied deeply.
*/
void SwFmt::CopyAttrs( const SwFmt& rFmt, sal_Bool bReplace )
{
    // copy only array with attributes delta
    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // special treatments for some attributes
    SwAttrSet* pChgSet = (SwAttrSet*)&rFmt.aSet;

    if( !bReplace )     // refresh only those that are not set?
    {
        if( pChgSet == (SwAttrSet*)&rFmt.aSet )
            pChgSet = new SwAttrSet( rFmt.aSet );
        pChgSet->Differentiate( aSet );
    }

    // copy only array with attributes delta
    if( pChgSet->GetPool() != aSet.GetPool() )
        pChgSet->CopyToModify( *this );
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );

        if ( 0 != aSet.Put_BC( *pChgSet, &aOld, &aNew ) )
        {
            // a few special treatments for attributes
            aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
        }
    }

    if( pChgSet != (SwAttrSet*)&rFmt.aSet ) // was a Set created?
        delete pChgSet;
}

SwFmt::~SwFmt()
{
    // This happens at a ObjectDying message. Thus put all dependent
    // ones on DerivedFrom.
    if( GetDepends() )
    {
        OSL_ENSURE( DerivedFrom(), "SwFmt::~SwFmt: Def dependents!" );

        bFmtInDTOR = sal_True;

        SwFmt* pParentFmt = DerivedFrom();
        if( !pParentFmt )
        {
            OSL_FAIL(rtl::OStringBuffer(
                    RTL_CONSTASCII_STRINGPARAM("~SwFmt: parent format missing from: ")).
                append(rtl::OUStringToOString(GetName(), osl_getThreadTextEncoding())).getStr());
        }
        else
        {
            while( GetDepends() )
            {
                SwFmtChg aOldFmt( this );
                SwFmtChg aNewFmt( pParentFmt );
                SwClient* pDepend = (SwClient*)GetDepends();
                pParentFmt->Add( pDepend );
                pDepend->ModifyNotification( &aOldFmt, &aNewFmt );
            }
        }
    }
}

void SwFmt::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    bool bContinue = true; // true = pass on to dependent ones

    sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                    pNewValue ? pNewValue->Which() : 0 ;
    switch( nWhich )
    {
    case 0:     break;          // Which-Id of 0?

    case RES_OBJECTDYING :
        {
            // If the dying object is the parent format of this format so
            // attach this to the parent of the parent
            SwFmt* pFmt = (SwFmt*) ((SwPtrMsgPoolItem*)pNewValue)->pObject;

            // do not move if this is the topmost format
            if( GetRegisteredIn() && GetRegisteredIn() == pFmt )
            {
                if( pFmt->GetRegisteredIn() )
                {
                    // if parent so register in new parent
                    pFmt->DerivedFrom()->Add( this );
                    aSet.SetParent( &DerivedFrom()->aSet );
                }
                else
                {
                    // otherwise de-register at least from dying one
                    DerivedFrom()->Remove( this );
                    aSet.SetParent( 0 );
                }
            }
        }
        break;
    case RES_ATTRSET_CHG:
        if( ((SwAttrSetChg*)pOldValue)->GetTheChgdSet() != &aSet )
        {
            // pass only those that are not set
            SwAttrSetChg aOld( *(SwAttrSetChg*)pOldValue );
            SwAttrSetChg aNew( *(SwAttrSetChg*)pNewValue );

            aOld.GetChgSet()->Differentiate( aSet );
            aNew.GetChgSet()->Differentiate( aSet );

            if( aNew.Count() )
                NotifyClients( &aOld, &aNew );
            bContinue = false;
        }
        break;
    case RES_FMT_CHG:
        // if the format parent will be moved so register my attribute set at
        // the new one

        // skip my own Modify
        if( ((SwFmtChg*)pOldValue)->pChangedFmt != this &&
            ((SwFmtChg*)pNewValue)->pChangedFmt == DerivedFrom() )
        {
            // attach Set to new parent
            aSet.SetParent( DerivedFrom() ? &DerivedFrom()->aSet : 0 );
        }
        break;
    case RES_RESET_FMTWRITTEN:
        {
            ResetWritten();

            // mba: here we don't use the additional stuff from NotifyClients().
            // should we?!
            // mba: move the code that ignores this event to the clients

            // pass Hint only to dependent formats (no Frames)
            ModifyBroadcast( pOldValue, pNewValue, TYPE(SwFmt) );
            bContinue = false;
        }
        break;
    default:
        {
            // attribute is defined in this format
            if( SFX_ITEM_SET == aSet.GetItemState( nWhich, sal_False ))
            {
                // DropCaps might come into this block
                OSL_ENSURE( RES_PARATR_DROP == nWhich, "Modify was sent without sender" );
                bContinue = false;
            }
        }
    }

    if( bContinue )
    {
        // walk over all dependent formats
        NotifyClients( pOldValue, pNewValue );
    }
}

sal_Bool SwFmt::SetDerivedFrom(SwFmt *pDerFrom)
{
    if ( pDerFrom )
    {
        const SwFmt* pFmt = pDerFrom;
        while ( pFmt != 0 )
        {
            if ( pFmt == this )
                return sal_False;

            pFmt=pFmt->DerivedFrom();
        }
    }
    else
    {
        // nothing provided, search for Dflt format
        pDerFrom = this;
        while ( pDerFrom->DerivedFrom() )
            pDerFrom = pDerFrom->DerivedFrom();
    }
    if ( (pDerFrom == DerivedFrom()) || (pDerFrom == this) )
        return sal_False;

    OSL_ENSURE( Which()==pDerFrom->Which()
            || ( Which()==RES_CONDTXTFMTCOLL && pDerFrom->Which()==RES_TXTFMTCOLL)
            || ( Which()==RES_TXTFMTCOLL && pDerFrom->Which()==RES_CONDTXTFMTCOLL)
            || ( Which()==RES_FLYFRMFMT && pDerFrom->Which()==RES_FRMFMT ),
            "SetDerivedFrom: derive apples from oranges?");

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    pDerFrom->Add( this );
    aSet.SetParent( &pDerFrom->aSet );

    SwFmtChg aOldFmt( this );
    SwFmtChg aNewFmt( this );
    ModifyNotification( &aOldFmt, &aNewFmt );

    return sal_True;
}

sal_Bool SwFmt::SetFmtAttr( const SfxPoolItem& rAttr )
{
    if ( IsInCache() || IsInSwFntCache() )
    {
        const sal_uInt16 nWhich = rAttr.Which();
        CheckCaching( nWhich );
    }

    // if Modify is locked then no modifications will be sent;
    // but call Modify always for FrmFmts
    sal_Bool bRet = sal_False;
    const sal_uInt16 nFmtWhich = Which();
    if( IsModifyLocked() ||
        ( !GetDepends() &&
          (RES_GRFFMTCOLL == nFmtWhich  ||
           RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( 0 != ( bRet = (0 != aSet.Put( rAttr ))) )
            aSet.SetModifyAtAttr( this );
        // #i71574#
        if ( nFmtWhich == RES_TXTFMTCOLL && rAttr.Which() == RES_PARATR_NUMRULE )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
    }
    else
    {
        // copy only array with attributes delta
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );

        bRet = 0 != aSet.Put_BC( rAttr, &aOld, &aNew );
        if( bRet )
        {
            // some special treatments for attributes
            aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
        }
    }
    return bRet;
}

sal_Bool SwFmt::SetFmtAttr( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return sal_False;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // if Modify is locked then no modifications will be sent;
    // but call Modify always for FrmFmts
    sal_Bool bRet = sal_False;
    const sal_uInt16 nFmtWhich = Which();
    if ( IsModifyLocked() ||
         ( !GetDepends() &&
           ( RES_GRFFMTCOLL == nFmtWhich ||
             RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( 0 != ( bRet = (0 != aSet.Put( rSet ))) )
            aSet.SetModifyAtAttr( this );
        // #i71574#
        if ( nFmtWhich == RES_TXTFMTCOLL )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
    }
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );
        bRet = 0 != aSet.Put_BC( rSet, &aOld, &aNew );
        if( bRet )
        {
            // some special treatments for attributes
            aSet.SetModifyAtAttr( this );
            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
        }
    }
    return bRet;
}

// remove Hint using nWhich from array with delta
sal_Bool SwFmt::ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !aSet.Count() )
        return sal_False;

    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1; // then set to 1st ID, only this item

    if ( IsInCache() || IsInSwFntCache() )
    {
        for( sal_uInt16 n = nWhich1; n < nWhich2; ++n )
            CheckCaching( n );
    }

    // if Modify is locked then no modifications will be sent
    if( IsModifyLocked() )
        return 0 != (( nWhich2 == nWhich1 )
                     ? aSet.ClearItem( nWhich1 )
                     : aSet.ClearItem_BC( nWhich1, nWhich2 ));

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    sal_Bool bRet = 0 != aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }
    return bRet;
}

// #i73790#
sal_uInt16 SwFmt::ResetAllFmtAttr()
{
    if( !aSet.Count() )
        return 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // if Modify is locked then no modifications will be sent
    if( IsModifyLocked() )
        return aSet.ClearItem( 0 );

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    bool bRet = 0 != aSet.ClearItem_BC( 0, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }
    return aNew.Count();
}

sal_Bool SwFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    sal_Bool bRet = SwModify::GetInfo( rInfo );
    return bRet;
}

void SwFmt::DelDiffs( const SfxItemSet& rSet )
{
    if( !aSet.Count() )
        return;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    // if Modify is locked then no modifications will be sent
    if( IsModifyLocked() )
    {
        aSet.Intersect( rSet );
        return;
    }

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    bool bRet = 0 != aSet.Intersect_BC( rSet, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }
}

/** SwFmt::IsBackgroundTransparent

    Virtual method to determine, if background of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent background.

    @return false, default implementation
*/
sal_Bool SwFmt::IsBackgroundTransparent() const
{
    return sal_False;
}

/** SwFmt::IsShadowTransparent

    Virtual method to determine, if shadow of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent shadow.

    @return false, default implementation
*/
sal_Bool SwFmt::IsShadowTransparent() const
{
    return sal_False;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwFmt::getIDocumentSettingAccess() const { return GetDoc(); }
const IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() const { return GetDoc(); }
IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() { return GetDoc(); }
const IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() const { return GetDoc(); }
IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() { return GetDoc(); }
IDocumentTimerAccess* SwFmt::getIDocumentTimerAccess() { return GetDoc(); }
IDocumentFieldsAccess* SwFmt::getIDocumentFieldsAccess() { return GetDoc(); }
IDocumentChartDataProviderAccess* SwFmt::getIDocumentChartDataProviderAccess() { return GetDoc(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
