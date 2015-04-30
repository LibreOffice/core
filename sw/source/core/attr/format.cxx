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

#include <doc.hxx>
#include <DocumentSettingManager.hxx> //For SwFmt::getIDocumentSettingAccess()
#include <IDocumentTimerAccess.hxx>
#include <fmtcolfunc.hxx>
#include <frame.hxx>
#include <format.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <paratr.hxx>
#include <swcache.hxx>
#include <swtblfmt.hxx>
#include <svl/grabbagitem.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>

//UUUU
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <svx/xdef.hxx>

using namespace com::sun::star;

TYPEINIT1( SwFmt, SwClient );

SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
              const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm,
              sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    m_aFmtName( OUString::createFromAscii(pFmtNm) ),
    m_aSet( rPool, pWhichRanges ),
    m_nWhichId( nFmtWhich ),
    m_nPoolFmtId( USHRT_MAX ),
    m_nPoolHelpId( USHRT_MAX ),
    m_nPoolHlpFileId( UCHAR_MAX )
{
    m_bAutoUpdateFmt = false; // LAYER_IMPL
    m_bAutoFmt = true;
    m_bWritten = m_bFmtInDTOR = m_bHidden = false;

    if( pDrvdFrm )
        m_aSet.SetParent( &pDrvdFrm->m_aSet );
}

SwFmt::SwFmt( SwAttrPool& rPool, const OUString& rFmtNm,
              const sal_uInt16* pWhichRanges, SwFmt* pDrvdFrm,
              sal_uInt16 nFmtWhich )
    : SwModify( pDrvdFrm ),
    m_aFmtName( rFmtNm ),
    m_aSet( rPool, pWhichRanges ),
    m_nWhichId( nFmtWhich ),
    m_nPoolFmtId( USHRT_MAX ),
    m_nPoolHelpId( USHRT_MAX ),
    m_nPoolHlpFileId( UCHAR_MAX )
{
    m_bAutoUpdateFmt = false; // LAYER_IMPL
    m_bAutoFmt = true;
    m_bWritten = m_bFmtInDTOR = m_bHidden = false;

    if( pDrvdFrm )
        m_aSet.SetParent( &pDrvdFrm->m_aSet );
}

SwFmt::SwFmt( const SwFmt& rFmt )
    : SwModify( rFmt.DerivedFrom() ),
    m_aFmtName( rFmt.m_aFmtName ),
    m_aSet( rFmt.m_aSet ),
    m_nWhichId( rFmt.m_nWhichId ),
    m_nPoolFmtId( rFmt.GetPoolFmtId() ),
    m_nPoolHelpId( rFmt.GetPoolHelpId() ),
    m_nPoolHlpFileId( rFmt.GetPoolHlpFileId() )
{
    m_bWritten = m_bFmtInDTOR = false; // LAYER_IMPL
    m_bAutoFmt = rFmt.m_bAutoFmt;
    m_bHidden = rFmt.m_bHidden;
    m_bAutoUpdateFmt = rFmt.m_bAutoUpdateFmt;

    if( rFmt.DerivedFrom() )
        m_aSet.SetParent( &rFmt.DerivedFrom()->m_aSet );
    // a few special treatments for attributes
    m_aSet.SetModifyAtAttr( this );
}

SwFmt &SwFmt::operator=(const SwFmt& rFmt)
{
    m_nWhichId = rFmt.m_nWhichId;
    m_nPoolFmtId = rFmt.GetPoolFmtId();
    m_nPoolHelpId = rFmt.GetPoolHelpId();
    m_nPoolHlpFileId = rFmt.GetPoolHlpFileId();

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( false );
    }
    SetInSwFntCache( false );

    // copy only array with attributes delta
    SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
              aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );
    m_aSet.Intersect_BC( rFmt.m_aSet, &aOld, &aNew );
    (void)m_aSet.Put_BC( rFmt.m_aSet, &aOld, &aNew );

    // a few special treatments for attributes
    m_aSet.SetModifyAtAttr( this );

    // create PoolItem attribute for Modify
    if( aOld.Count() )
    {
        SwAttrSetChg aChgOld( m_aSet, aOld );
        SwAttrSetChg aChgNew( m_aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }

    if( GetRegisteredIn() != rFmt.GetRegisteredIn() )
    {
        if( GetRegisteredIn() )
            GetRegisteredInNonConst()->Remove(this);
        if( rFmt.GetRegisteredIn() )
        {
            const_cast<SwFmt&>(rFmt).GetRegisteredInNonConst()->Add(this);
            m_aSet.SetParent( &rFmt.m_aSet );
        }
        else
        {
            m_aSet.SetParent( 0 );
        }
    }
    m_bAutoFmt = rFmt.m_bAutoFmt;
    m_bHidden = rFmt.m_bHidden;
    m_bAutoUpdateFmt = rFmt.m_bAutoUpdateFmt;
    return *this;
}

void SwFmt::SetName( const OUString& rNewName, bool bBroadcast )
{
    OSL_ENSURE( !IsDefault(), "SetName: Defaultformat" );
    if( bBroadcast )
    {
        SwStringMsgPoolItem aOld( RES_NAME_CHANGED, m_aFmtName );
        SwStringMsgPoolItem aNew( RES_NAME_CHANGED, rNewName );
        m_aFmtName = rNewName;
        ModifyNotification( &aOld, &aNew );
    }
    else
    {
        m_aFmtName = rNewName;
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
void SwFmt::CopyAttrs( const SwFmt& rFmt, bool bReplace )
{
    // copy only array with attributes delta
    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( false );
    }
    SetInSwFntCache( false );

    // special treatments for some attributes
    SwAttrSet* pChgSet = const_cast<SwAttrSet*>(&rFmt.m_aSet);

    if( !bReplace )     // refresh only those that are not set?
    {
        if( pChgSet == &rFmt.m_aSet )
            pChgSet = new SwAttrSet( rFmt.m_aSet );
        pChgSet->Differentiate( m_aSet );
    }

    // copy only array with attributes delta
    if( pChgSet->GetPool() != m_aSet.GetPool() )
        pChgSet->CopyToModify( *this );
    else
    {
        SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
                  aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );

        if ( m_aSet.Put_BC( *pChgSet, &aOld, &aNew ) )
        {
            // a few special treatments for attributes
            m_aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( m_aSet, aOld );
            SwAttrSetChg aChgNew( m_aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
        }
    }

    if( pChgSet != &rFmt.m_aSet ) // was a Set created?
        delete pChgSet;
}

SwFmt::~SwFmt()
{
    // This happens at a ObjectDying message. Thus put all dependent
    // ones on DerivedFrom.
    if( HasWriterListeners() )
    {
        OSL_ENSURE( DerivedFrom(), "SwFmt::~SwFmt: Def dependents!" );

        m_bFmtInDTOR = true;

        SwFmt* pParentFmt = DerivedFrom();
        if( !pParentFmt )
        {
            SAL_WARN(
                "sw.core",
                "~SwFmt: parent format missing from: " << GetName() );
        }
        else
        {
            SwFmtChg aOldFmt( this );
            SwFmtChg aNewFmt( pParentFmt );
            SwIterator<SwClient,SwFmt> aIter(*this);
            for(SwClient* pClient = aIter.First(); pClient && pParentFmt; pClient = aIter.Next())
            {
                pParentFmt->Add( pClient );
                pClient->ModifyNotification( &aOldFmt, &aNewFmt );
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

    case RES_OBJECTDYING:
        if (pNewValue)
        {
            // If the dying object is the parent format of this format so
            // attach this to the parent of the parent
            SwFmt* pFmt = static_cast<SwFmt*>(static_cast<const SwPtrMsgPoolItem*>(pNewValue)->pObject);

            // do not move if this is the topmost format
            if( GetRegisteredIn() && GetRegisteredIn() == pFmt )
            {
                if( pFmt->GetRegisteredIn() )
                {
                    // if parent so register in new parent
                    pFmt->DerivedFrom()->Add( this );
                    m_aSet.SetParent( &DerivedFrom()->m_aSet );
                }
                else
                {
                    // otherwise de-register at least from dying one
                    DerivedFrom()->Remove( this );
                    m_aSet.SetParent( 0 );
                }
            }
        }
        break;
    case RES_ATTRSET_CHG:
        if (pOldValue && pNewValue && static_cast<const SwAttrSetChg*>(pOldValue)->GetTheChgdSet() != &m_aSet)
        {
            // pass only those that are not set
            SwAttrSetChg aOld( *static_cast<const SwAttrSetChg*>(pOldValue) );
            SwAttrSetChg aNew( *static_cast<const SwAttrSetChg*>(pNewValue) );

            aOld.GetChgSet()->Differentiate( m_aSet );
            aNew.GetChgSet()->Differentiate( m_aSet );

            if( aNew.Count() )
                NotifyClients( &aOld, &aNew );
            bContinue = false;
        }
        break;
    case RES_FMT_CHG:
        // if the format parent will be moved so register my attribute set at
        // the new one

        // skip my own Modify
        if ( pOldValue && pNewValue &&
            static_cast<const SwFmtChg*>(pOldValue)->pChangedFmt != this &&
            static_cast<const SwFmtChg*>(pNewValue)->pChangedFmt == GetRegisteredIn() )
        {
            // attach Set to new parent
            m_aSet.SetParent( DerivedFrom() ? &DerivedFrom()->m_aSet : 0 );
        }
        break;
    case RES_RESET_FMTWRITTEN:
        {
            ResetWritten();

            // mba: here we don't use the additional stuff from NotifyClients().
            // should we?!
            // mba: move the code that ignores this event to the clients

            // pass Hint only to dependent formats (no Frames)
            //ModifyBroadcast( pOldValue, pNewValue, TYPE(SwFmt) );
            //bContinue = false;
        }
        break;
    default:
        {
            // attribute is defined in this format
            if( SfxItemState::SET == m_aSet.GetItemState( nWhich, false ))
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

bool SwFmt::SetDerivedFrom(SwFmt *pDerFrom)
{
    if ( pDerFrom )
    {
        const SwFmt* pFmt = pDerFrom;
        while ( pFmt != 0 )
        {
            if ( pFmt == this )
                return false;

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
        return false;

    OSL_ENSURE( Which()==pDerFrom->Which()
            || ( Which()==RES_CONDTXTFMTCOLL && pDerFrom->Which()==RES_TXTFMTCOLL)
            || ( Which()==RES_TXTFMTCOLL && pDerFrom->Which()==RES_CONDTXTFMTCOLL)
            || ( Which()==RES_FLYFRMFMT && pDerFrom->Which()==RES_FRMFMT ),
            "SetDerivedFrom: derive apples from oranges?");

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( false );
    }
    SetInSwFntCache( false );

    pDerFrom->Add( this );
    m_aSet.SetParent( &pDerFrom->m_aSet );

    SwFmtChg aOldFmt( this );
    SwFmtChg aNewFmt( this );
    ModifyNotification( &aOldFmt, &aNewFmt );

    return true;
}

bool SwFmt::supportsFullDrawingLayerFillAttributeSet() const
{
    return false;
}

const SfxPoolItem& SwFmt::GetFmtAttr( sal_uInt16 nWhich, bool bInParents ) const
{
    if (RES_BACKGROUND == nWhich && supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        SAL_INFO("sw.core", "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes or makeBackgroundBrushItem (simple fallback is in place and used)");
        static SvxBrushItem aSvxBrushItem(RES_BACKGROUND);

        // fill the local static SvxBrushItem from the current ItemSet so that
        // the fill attributes [XATTR_FILL_FIRST .. XATTR_FILL_LAST] are used
        // as good as possible to create a fallback representation and return that
        aSvxBrushItem = getSvxBrushItemFromSourceSet(m_aSet, RES_BACKGROUND, bInParents);

        return aSvxBrushItem;
    }

    return m_aSet.Get( nWhich, bInParents );
}

SfxItemState SwFmt::GetItemState( sal_uInt16 nWhich, bool bSrchInParent, const SfxPoolItem **ppItem ) const
{
    if (RES_BACKGROUND == nWhich && supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        SAL_INFO("sw.core", "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes or SwFmt::GetBackgroundStat (simple fallback is in place and used)");
        const drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFill = getSdrAllFillAttributesHelper();

        // check if the new fill attributes are used
        if(aFill.get() && aFill->isUsed())
        {
            // if yes, fill the local SvxBrushItem using the new fill attributes
            // as good as possible to have an instance for the pointer to point
            // to and return as state that it is set

            static SvxBrushItem aSvxBrushItem(RES_BACKGROUND);

            aSvxBrushItem = getSvxBrushItemFromSourceSet(m_aSet, RES_BACKGROUND, bSrchInParent);
            if( ppItem )
                *ppItem = &aSvxBrushItem;

            return SfxItemState::SET;
        }

        // if not, reset pointer and return SfxItemState::DEFAULT to signal that
        // the item is not set
        if( ppItem )
            *ppItem = NULL;

        return SfxItemState::DEFAULT;
    }

    return m_aSet.GetItemState( nWhich, bSrchInParent, ppItem );
}

SfxItemState SwFmt::GetBackgroundState(SvxBrushItem &rItem, bool bSrchInParent) const
{
    if (supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        const drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFill = getSdrAllFillAttributesHelper();

        // check if the new fill attributes are used
        if(aFill.get() && aFill->isUsed())
        {
            // if yes, fill the local SvxBrushItem using the new fill attributes
            // as good as possible to have an instance for the pointer to point
            // to and return as state that it is set
            rItem = getSvxBrushItemFromSourceSet(m_aSet, RES_BACKGROUND, bSrchInParent);
            return SfxItemState::SET;
        }

        // if not return SfxItemState::DEFAULT to signal that the item is not set
        return SfxItemState::DEFAULT;
    }

    const SfxPoolItem* pItem = 0;
    SfxItemState eRet = m_aSet.GetItemState(RES_BACKGROUND, bSrchInParent, &pItem);
    if (pItem)
        rItem = *static_cast<const SvxBrushItem*>(pItem);
    return eRet;
}

bool SwFmt::SetFmtAttr( const SfxPoolItem& rAttr )
{
    if ( IsInCache() || IsInSwFntCache() )
    {
        const sal_uInt16 nWhich = rAttr.Which();
        CheckCaching( nWhich );
    }

    bool bRet = false;

    //UUUU
    if (RES_BACKGROUND == rAttr.Which() && supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        SAL_INFO("sw.core", "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");
        SfxItemSet aTempSet(*m_aSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0, 0);
        const SvxBrushItem& rSource = static_cast< const SvxBrushItem& >(rAttr);

        // fill a local ItemSet with the attributes corresponding as good as possible
        // to the new fill properties [XATTR_FILL_FIRST .. XATTR_FILL_LAST] and set these
        // as ItemSet
        setSvxBrushItemAsFillAttributesToTargetSet(rSource, aTempSet);

        if(IsModifyLocked())
        {
            if( ( bRet = m_aSet.Put( aTempSet ) ) )
            {
                m_aSet.SetModifyAtAttr( this );
            }
        }
        else
        {
            SwAttrSet aOld(*m_aSet.GetPool(), m_aSet.GetRanges()), aNew(*m_aSet.GetPool(), m_aSet.GetRanges());

            bRet = m_aSet.Put_BC(aTempSet, &aOld, &aNew);

            if(bRet)
            {
                m_aSet.SetModifyAtAttr(this);

                SwAttrSetChg aChgOld(m_aSet, aOld);
                SwAttrSetChg aChgNew(m_aSet, aNew);

                ModifyNotification(&aChgOld, &aChgNew);
            }
        }

        return bRet;
    }

    // if Modify is locked then no modifications will be sent;
    // but call Modify always for FrmFmts
    const sal_uInt16 nFmtWhich = Which();
    if( IsModifyLocked() ||
        ( !HasWriterListeners() &&
          (RES_GRFFMTCOLL == nFmtWhich  ||
           RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( ( bRet = (0 != m_aSet.Put( rAttr ))) )
            m_aSet.SetModifyAtAttr( this );
        // #i71574#
        if ( nFmtWhich == RES_TXTFMTCOLL && rAttr.Which() == RES_PARATR_NUMRULE )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
    }
    else
    {
        // copy only array with attributes delta
        SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
                  aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );

        bRet = m_aSet.Put_BC( rAttr, &aOld, &aNew );
        if( bRet )
        {
            // some special treatments for attributes
            m_aSet.SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( m_aSet, aOld );
            SwAttrSetChg aChgNew( m_aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
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
        SetInCache( false );
    }
    SetInSwFntCache( false );

    bool bRet = false;

    //UUUU Use local copy to be able to apply needed changes, e.g. call
    // CheckForUniqueItemForLineFillNameOrIndex which is needed for NameOrIndex stuff
    SfxItemSet aTempSet(rSet);

    //UUUU Need to check for unique item for DrawingLayer items of type NameOrIndex
    // and evtl. correct that item to ensure unique names for that type. This call may
    // modify/correct entries inside of the given SfxItemSet
    if(GetDoc())
    {
        GetDoc()->CheckForUniqueItemForLineFillNameOrIndex(aTempSet);
    }

    if (supportsFullDrawingLayerFillAttributeSet())
    {
        const SfxPoolItem* pSource = 0;

        if(SfxItemState::SET == aTempSet.GetItemState(RES_BACKGROUND, false, &pSource))
        {
            //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
            SAL_INFO("sw.core", "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");

            // copy all items to be set anyways to a local ItemSet with is also prepared for the new
            // fill attribute ranges [XATTR_FILL_FIRST .. XATTR_FILL_LAST]. Add the attributes
            // corresponding as good as possible to the new fill properties and set the whole ItemSet
            const SvxBrushItem& rSource(static_cast< const SvxBrushItem& >(*pSource));
            setSvxBrushItemAsFillAttributesToTargetSet(rSource, aTempSet);

            if(IsModifyLocked())
            {
                if( ( bRet = m_aSet.Put( aTempSet ) ) )
                {
                    m_aSet.SetModifyAtAttr( this );
                }
            }
            else
            {
                SwAttrSet aOld(*m_aSet.GetPool(), m_aSet.GetRanges()), aNew(*m_aSet.GetPool(), m_aSet.GetRanges());

                bRet = m_aSet.Put_BC(aTempSet, &aOld, &aNew);

                if(bRet)
                {
                    m_aSet.SetModifyAtAttr(this);

                    SwAttrSetChg aChgOld(m_aSet, aOld);
                    SwAttrSetChg aChgNew(m_aSet, aNew);

                    ModifyNotification(&aChgOld, &aChgNew);
                }
            }

            return bRet;
        }
    }

    // if Modify is locked then no modifications will be sent;
    // but call Modify always for FrmFmts
    const sal_uInt16 nFmtWhich = Which();
    if ( IsModifyLocked() ||
         ( !HasWriterListeners() &&
           ( RES_GRFFMTCOLL == nFmtWhich ||
             RES_TXTFMTCOLL == nFmtWhich ) ) )
    {
        if( ( bRet = m_aSet.Put( aTempSet )) )
            m_aSet.SetModifyAtAttr( this );
        // #i71574#
        if ( nFmtWhich == RES_TXTFMTCOLL )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
    }
    else
    {
        SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
                  aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );
        bRet = m_aSet.Put_BC( aTempSet, &aOld, &aNew );
        if( bRet )
        {
            // some special treatments for attributes
            m_aSet.SetModifyAtAttr( this );
            SwAttrSetChg aChgOld( m_aSet, aOld );
            SwAttrSetChg aChgNew( m_aSet, aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
        }
    }
    return bRet;
}

// remove Hint using nWhich from array with delta
bool SwFmt::ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !m_aSet.Count() )
        return false;

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
                     ? m_aSet.ClearItem( nWhich1 )
                     : m_aSet.ClearItem_BC( nWhich1, nWhich2 ));

    SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
              aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );
    bool bRet = 0 != m_aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( m_aSet, aOld );
        SwAttrSetChg aChgNew( m_aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }
    return bRet;
}

// #i73790#
sal_uInt16 SwFmt::ResetAllFmtAttr()
{
    if( !m_aSet.Count() )
        return 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( false );
    }
    SetInSwFntCache( false );

    // if Modify is locked then no modifications will be sent
    if( IsModifyLocked() )
        return m_aSet.ClearItem( 0 );

    SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
              aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );
    bool bRet = 0 != m_aSet.ClearItem_BC( 0, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( m_aSet, aOld );
        SwAttrSetChg aChgNew( m_aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }
    return aNew.Count();
}

bool SwFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    return SwModify::GetInfo( rInfo );
}

void SwFmt::DelDiffs( const SfxItemSet& rSet )
{
    if( !m_aSet.Count() )
        return;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( false );
    }
    SetInSwFntCache( false );

    // if Modify is locked then no modifications will be sent
    if( IsModifyLocked() )
    {
        m_aSet.Intersect( rSet );
        return;
    }

    SwAttrSet aOld( *m_aSet.GetPool(), m_aSet.GetRanges() ),
              aNew( *m_aSet.GetPool(), m_aSet.GetRanges() );
    bool bRet = 0 != m_aSet.Intersect_BC( rSet, &aOld, &aNew );
    if( bRet )
    {
        SwAttrSetChg aChgOld( m_aSet, aOld );
        SwAttrSetChg aChgNew( m_aSet, aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // send all modified ones
    }
}

/** SwFmt::IsBackgroundTransparent

    Virtual method to determine, if background of format is transparent.
    Default implementation returns false. Thus, subclasses have to override
    method, if the specific subclass can have a transparent background.

    @return false, default implementation
*/
bool SwFmt::IsBackgroundTransparent() const
{
    return false;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwFmt::getIDocumentSettingAccess() const { return & GetDoc()->GetDocumentSettingManager(); }
const IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() const { return & GetDoc()->getIDocumentDrawModelAccess(); }
IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() { return & GetDoc()->getIDocumentDrawModelAccess(); }
const IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() const { return &GetDoc()->getIDocumentLayoutAccess(); }
IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() { return &GetDoc()->getIDocumentLayoutAccess(); }
IDocumentTimerAccess* SwFmt::getIDocumentTimerAccess() { return & GetDoc()->getIDocumentTimerAccess(); }
IDocumentFieldsAccess* SwFmt::getIDocumentFieldsAccess() { return &GetDoc()->getIDocumentFieldsAccess(); }
IDocumentChartDataProviderAccess* SwFmt::getIDocumentChartDataProviderAccess() { return & GetDoc()->getIDocumentChartDataProviderAccess(); }

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

//UUUU
SvxBrushItem SwFmt::makeBackgroundBrushItem(bool bInP) const
{
    if (supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        SAL_INFO("sw.core", "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");

        // fill the local static SvxBrushItem from the current ItemSet so that
        // the fill attributes [XATTR_FILL_FIRST .. XATTR_FILL_LAST] are used
        // as good as possible to create a fallback representation and return that
        return getSvxBrushItemFromSourceSet(m_aSet, RES_BACKGROUND, bInP);
    }

    return m_aSet.GetBackground(bInP);
}

//UUUU
drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFmt::getSdrAllFillAttributesHelper() const
{
    return drawinglayer::attribute::SdrAllFillAttributesHelperPtr();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
