/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <doc.hxx>
#include <fmtcolfunc.hxx>
#include <frame.hxx>
#include <format.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <paratr.hxx>
#include <swcache.hxx>
#include <svl/grabbagitem.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>

using namespace com::sun::star;

TYPEINIT1( SwFmt, SwClient );

SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
              const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm,
              sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    aFmtName( OUString::createFromAscii(pFmtNm) ),
    aSet( rPool, pWhichRanges ),
    nWhichId( nFmtWhich ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    bAutoUpdateFmt = sal_False; 
    bAutoFmt = true;
    bWritten = bFmtInDTOR = bHidden = false;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}

SwFmt::SwFmt( SwAttrPool& rPool, const OUString& rFmtNm,
              const sal_uInt16* pWhichRanges, SwFmt* pDrvdFrm,
              sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    aFmtName( rFmtNm ),
    aSet( rPool, pWhichRanges ),
    nWhichId( nFmtWhich ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    bAutoUpdateFmt = sal_False; 
    bAutoFmt = true;
    bWritten = bFmtInDTOR = bHidden = false;

    if( pDrvdFrm )
        aSet.SetParent( &pDrvdFrm->aSet );
}

SwFmt::SwFmt( const SwFmt& rFmt )
    : SwModify( rFmt.DerivedFrom() ),
    aFmtName( rFmt.aFmtName ),
    aSet( rFmt.aSet ),
    nWhichId( rFmt.nWhichId ),
    nPoolFmtId( rFmt.GetPoolFmtId() ),
    nPoolHelpId( rFmt.GetPoolHelpId() ),
    nPoolHlpFileId( rFmt.GetPoolHlpFileId() )
{
    bWritten = bFmtInDTOR = false; 
    bAutoFmt = rFmt.bAutoFmt;
    bHidden = rFmt.bHidden;
    bAutoUpdateFmt = rFmt.bAutoUpdateFmt;

    if( rFmt.DerivedFrom() )
        aSet.SetParent( &rFmt.DerivedFrom()->aSet );
    
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

    
    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    aSet.Intersect_BC( rFmt.aSet, &aOld, &aNew );
    aSet.Put_BC( rFmt.aSet, &aOld, &aNew );

    
    aSet.SetModifyAtAttr( this );

    
    if( aOld.Count() )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); 
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
    bHidden = rFmt.bHidden;
    bAutoUpdateFmt = rFmt.bAutoUpdateFmt;
    return *this;
}

void SwFmt::SetName( const OUString& rNewName, sal_Bool bBroadcast )
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
    
    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    
    SwAttrSet* pChgSet = (SwAttrSet*)&rFmt.aSet;

    if( !bReplace )     
    {
        if( pChgSet == (SwAttrSet*)&rFmt.aSet )
            pChgSet = new SwAttrSet( rFmt.aSet );
        pChgSet->Differentiate( aSet );
    }

    
    if( pChgSet->GetPool() != aSet.GetPool() )
        pChgSet->CopyToModify( *this );
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );

        if ( aSet.Put_BC( *pChgSet, &aOld, &aNew ) )
        {
            
            aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); 
        }
    }

    if( pChgSet != (SwAttrSet*)&rFmt.aSet ) 
        delete pChgSet;
}

SwFmt::~SwFmt()
{
    
    
    if( GetDepends() )
    {
        OSL_ENSURE( DerivedFrom(), "SwFmt::~SwFmt: Def dependents!" );

        bFmtInDTOR = true;

        SwFmt* pParentFmt = DerivedFrom();
        if( !pParentFmt )
        {
            SAL_WARN(
                "sw.core",
                "~SwFmt: parent format missing from: " << GetName() );
        }
        else
        {
            while( GetDepends() && pParentFmt)
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
    bool bContinue = true; 

    sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                    pNewValue ? pNewValue->Which() : 0 ;
    switch( nWhich )
    {
    case 0:     break;          

    case RES_OBJECTDYING :
        {
            
            
            SwFmt* pFmt = (SwFmt*) ((SwPtrMsgPoolItem*)pNewValue)->pObject;

            
            if( GetRegisteredIn() && GetRegisteredIn() == pFmt )
            {
                if( pFmt->GetRegisteredIn() )
                {
                    
                    pFmt->DerivedFrom()->Add( this );
                    aSet.SetParent( &DerivedFrom()->aSet );
                }
                else
                {
                    
                    DerivedFrom()->Remove( this );
                    aSet.SetParent( 0 );
                }
            }
        }
        break;
    case RES_ATTRSET_CHG:
        if( ((SwAttrSetChg*)pOldValue)->GetTheChgdSet() != &aSet )
        {
            
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
        
        

        
        if( ((SwFmtChg*)pOldValue)->pChangedFmt != this &&
            ((SwFmtChg*)pNewValue)->pChangedFmt == DerivedFrom() )
        {
            
            aSet.SetParent( DerivedFrom() ? &DerivedFrom()->aSet : 0 );
        }
        break;
    case RES_RESET_FMTWRITTEN:
        {
            ResetWritten();

            
            
            

            
            ModifyBroadcast( pOldValue, pNewValue, TYPE(SwFmt) );
            bContinue = false;
        }
        break;
    default:
        {
            
            if( SFX_ITEM_SET == aSet.GetItemState( nWhich, false ))
            {
                
                OSL_ENSURE( RES_PARATR_DROP == nWhich, "Modify was sent without sender" );
                bContinue = false;
            }
        }
    }

    if( bContinue )
    {
        
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

bool SwFmt::SetFmtAttr( const SfxPoolItem& rAttr )
{
    if ( IsInCache() || IsInSwFntCache() )
    {
        const sal_uInt16 nWhich = rAttr.Which();
        CheckCaching( nWhich );
    }

    
    
    bool bRet = false;
    const sal_uInt16 nFmtWhich = Which();
    if( IsModifyLocked() ||
        ( !GetDepends() &&
          (RES_GRFFMTCOLL == nFmtWhich  ||
           RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( ( bRet = (0 != aSet.Put( rAttr ))) )
            aSet.SetModifyAtAttr( this );
        
        if ( nFmtWhich == RES_TXTFMTCOLL && rAttr.Which() == RES_PARATR_NUMRULE )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
    }
    else
    {
        
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );

        bRet = aSet.Put_BC( rAttr, &aOld, &aNew );
        if( bRet )
        {
            
            aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); 
        }
    }
    return bRet;
}

bool SwFmt::SetFmtAttr( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return false;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    
    
    bool bRet = false;
    const sal_uInt16 nFmtWhich = Which();
    if ( IsModifyLocked() ||
         ( !GetDepends() &&
           ( RES_GRFFMTCOLL == nFmtWhich ||
             RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( ( bRet = aSet.Put( rSet )) )
            aSet.SetModifyAtAttr( this );
        
        if ( nFmtWhich == RES_TXTFMTCOLL )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
    }
    else
    {
        SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
                  aNew( *aSet.GetPool(), aSet.GetRanges() );
        bRet = aSet.Put_BC( rSet, &aOld, &aNew );
        if( bRet )
        {
            
            aSet.SetModifyAtAttr( this );
            SwAttrSetChg aChgOld( aSet, aOld );
            SwAttrSetChg aChgNew( aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); 
        }
    }
    return bRet;
}


bool SwFmt::ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !aSet.Count() )
        return false;

    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1; 

    if ( IsInCache() || IsInSwFntCache() )
    {
        for( sal_uInt16 n = nWhich1; n < nWhich2; ++n )
            CheckCaching( n );
    }

    
    if( IsModifyLocked() )
        return 0 != (( nWhich2 == nWhich1 )
                     ? aSet.ClearItem( nWhich1 )
                     : aSet.ClearItem_BC( nWhich1, nWhich2 ));

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    bool bRet = 0 != aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); 
    }
    return bRet;
}


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

    
    if( IsModifyLocked() )
        return aSet.ClearItem( 0 );

    SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
              aNew( *aSet.GetPool(), aSet.GetRanges() );
    bool bRet = 0 != aSet.ClearItem_BC( 0, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( aSet, aOld );
        SwAttrSetChg aChgNew( aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); 
    }
    return aNew.Count();
}

bool SwFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    return SwModify::GetInfo( rInfo );
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
        ModifyNotification( &aChgOld, &aChgNew ); 
    }
}

/** SwFmt::IsBackgroundTransparent

    Virtual method to determine, if background of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent background.

    @return false, default implementation
*/
bool SwFmt::IsBackgroundTransparent() const
{
    return false;
}

/** SwFmt::IsShadowTransparent

    Virtual method to determine, if shadow of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent shadow.

    @return false, default implementation
*/
bool SwFmt::IsShadowTransparent() const
{
    return false;
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

void SwFmt::GetGrabBagItem(uno::Any& rVal) const
{
    if (m_pGrabBagItem.get())
        m_pGrabBagItem->QueryValue(rVal);
    else
    {
        uno::Sequence<beans::PropertyValue> aValue(0);
        rVal = uno::makeAny(aValue);
    }
}

void SwFmt::SetGrabBagItem(const uno::Any& rVal)
{
    if (!m_pGrabBagItem.get())
        m_pGrabBagItem.reset(new SfxGrabBagItem);

    m_pGrabBagItem->PutValue(rVal);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
