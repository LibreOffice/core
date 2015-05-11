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

#include <svl/itemiter.hxx>
#include <svtools/imap.hxx>
#include <tools/helpers.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <drawdoc.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtcnct.hxx>
#include <layhelp.hxx>
#include <ndgrf.hxx>
#include <tolayoutanchoredobjectposition.hxx>
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <ndole.hxx>
#include <swtable.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoashp.hxx>
#include "layouter.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "dview.hxx"
#include "frmtool.hxx"
#include "hints.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "sectfrm.hxx"
#include <vcl/svapp.hxx>
#include <calbck.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <textboxhelper.hxx>
#include <txtfly.hxx>

using namespace ::com::sun::star;

TYPEINIT2(SwFlyFrm,SwLayoutFrm,SwAnchoredObject);

SwFlyFrm::SwFlyFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwLayoutFrm( pFmt, pSib ),
    SwAnchoredObject(), // #i26791#
    pPrevLink( 0 ),
    pNextLink( 0 ),
    bInCnt( false ),
    bAtCnt( false ),
    bLayout( false ),
    bAutoPosition( false ),
    bNoShrink( false ),
    bLockDeleteContent( false ),
    m_bValidContentPos( false )
{
    mnFrmType = FRM_FLY;

    bInvalid = bNotifyBack = true;
    bLocked  = bMinHeight =
    bHeightClipped = bWidthClipped = bFormatHeightOnly = false;

    // Size setting: Fixed size is always the width
    const SwFmtFrmSize &rFrmSize = pFmt->GetFrmSize();
    const sal_uInt16 nDir =
        static_cast<const SvxFrameDirectionItem&>(pFmt->GetFmtAttr( RES_FRAMEDIR )).GetValue();
    if( FRMDIR_ENVIRONMENT == nDir )
    {
        mbDerivedVert = true;
        mbDerivedR2L = true;
    }
    else
    {
        mbInvalidVert = false;
        mbDerivedVert = false;
        mbDerivedR2L = false;
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir )
        {
            mbVertLR = false;
            mbVertical = false;
        }
        else
        {
            const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                mbVertLR = false;
                mbVertical = false;
            }
            else
            {
                mbVertical = true;

                if ( FRMDIR_VERT_TOP_LEFT == nDir )
                    mbVertLR = true;
                else
                    mbVertLR = false;
            }
        }

        mbInvalidR2L = false;
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            mbRightToLeft = true;
        else
            mbRightToLeft = false;
    }

    Frm().Width( rFrmSize.GetWidth() );
    Frm().Height( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE ? MINFLY : rFrmSize.GetHeight() );

    // Fixed or variable Height?
    if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        bMinHeight = true;
    else if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        mbFixSize = true;

    // insert columns, if necessary
    InsertColumns();

    // First the Init, then the Content:
    // This is due to the fact that the Content may have Objects/Frames,
    // which are then registered
    InitDrawObj( false );

    Chain( pAnch );

    InsertCnt();

    // Put it somewhere outside so that out document is not formatted unnecessarily often
    Frm().Pos().setX(FAR_AWAY);
    Frm().Pos().setY(FAR_AWAY);
}

void SwFlyFrm::Chain( SwFrm* _pAnch )
{
    // Connect to chain neighboors.
    // No problem, if a neighboor doesn't exist - the construction of the
    // neighboor will make the connection
    const SwFmtChain& rChain = GetFmt()->GetChain();
    if ( rChain.GetPrev() || rChain.GetNext() )
    {
        if ( rChain.GetNext() )
        {
            SwFlyFrm* pFollow = FindChainNeighbour( *rChain.GetNext(), _pAnch );
            if ( pFollow )
            {
                OSL_ENSURE( !pFollow->GetPrevLink(), "wrong chain detected" );
                if ( !pFollow->GetPrevLink() )
                    SwFlyFrm::ChainFrames( this, pFollow );
            }
        }
        if ( rChain.GetPrev() )
        {
            SwFlyFrm *pMaster = FindChainNeighbour( *rChain.GetPrev(), _pAnch );
            if ( pMaster )
            {
                OSL_ENSURE( !pMaster->GetNextLink(), "wrong chain detected" );
                if ( !pMaster->GetNextLink() )
                    SwFlyFrm::ChainFrames( pMaster, this );
            }
        }
    }
}

void SwFlyFrm::InsertCnt()
{
    if ( !GetPrevLink() )
    {
        const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
        OSL_ENSURE( rCntnt.GetCntntIdx(), ":-( no content prepared." );
        sal_uLong nIndex = rCntnt.GetCntntIdx()->GetIndex();
        // Lower() means SwColumnFrm; the Content then needs to be inserted into the (Column)BodyFrm
        ::_InsertCnt( Lower() ? static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(Lower())->Lower()) : (SwLayoutFrm*)this,
                      GetFmt()->GetDoc(), nIndex );

        // NoTxt always have a fixed height.
        if ( Lower() && Lower()->IsNoTxtFrm() )
        {
            mbFixSize = true;
            bMinHeight = false;
        }
    }
}

void SwFlyFrm::InsertColumns()
{
    // #i97379#
    // Check, if column are allowed.
    // Columns are not allowed for fly frames, which represent graphics or embedded objects.
    const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), "<SwFlyFrm::InsertColumns()> - no content prepared." );
    SwNodeIndex aFirstCntnt( *(rCntnt.GetCntntIdx()), 1 );
    if ( aFirstCntnt.GetNode().IsNoTxtNode() )
    {
        return;
    }

    const SwFmtCol &rCol = GetFmt()->GetCol();
    if ( rCol.GetNumCols() > 1 )
    {
        // Start off PrtArea to be as large as Frm, so that we can put in the columns
        // properly. It'll adjust later on.
        Prt().Width( Frm().Width() );
        Prt().Height( Frm().Height() );
        const SwFmtCol aOld; // ChgColumns() also needs an old value passed
        ChgColumns( aOld, rCol );
    }
}

void SwFlyFrm::DestroyImpl()
{
    // Accessible objects for fly frames will be destroyed in this destructor.
    // For frames bound as char or frames that don't have an anchor we have
    // to do that ourselves. For any other frame the call RemoveFly at the
    // anchor will do that.
    if( IsAccessibleFrm() && GetFmt() && (IsFlyInCntFrm() || !GetAnchorFrm()) )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            SwViewShell *pVSh = pRootFrm->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                // Lowers aren't disposed already, so we have to do a recursive
                // dispose
                pVSh->Imp()->DisposeAccessibleFrm( this, true );
            }
        }
    }

    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        // OD 2004-01-19 #110582#
        Unchain();

        // OD 2004-01-19 #110582#
        DeleteCnt();

        if ( GetAnchorFrm() )
            AnchorFrm()->RemoveFly( this );
    }

    FinitDrawObj();

    SwLayoutFrm::DestroyImpl();
}

SwFlyFrm::~SwFlyFrm()
{
}

const IDocumentDrawModelAccess* SwFlyFrm::getIDocumentDrawModelAccess()
{
    return GetFmt()->getIDocumentDrawModelAccess();
}

// OD 2004-01-19 #110582#
void SwFlyFrm::Unchain()
{
    if ( GetPrevLink() )
        UnchainFrames( GetPrevLink(), this );
    if ( GetNextLink() )
        UnchainFrames( this, GetNextLink() );
}

// OD 2004-01-19 #110582#
void SwFlyFrm::DeleteCnt()
{
    // #110582#-2
    if ( IsLockDeleteContent() )
        return;

    SwFrm* pFrm = m_pLower;
    while ( pFrm )
    {
        while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->size() )
        {
            SwAnchoredObject *pAnchoredObj = (*pFrm->GetDrawObjs())[0];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFrm::DestroyFrm(static_cast<SwFlyFrm*>(pAnchoredObj));
            }
            else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
            {
                // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
                SdrObject* pObj = pAnchoredObj->DrawObj();
                if ( pObj->ISA(SwDrawVirtObj) )
                {
                    SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
                    pDrawVirtObj->RemoveFromWriterLayout();
                    pDrawVirtObj->RemoveFromDrawingPage();
                }
                else
                {
                    SwDrawContact* pContact =
                            static_cast<SwDrawContact*>(::GetUserCall( pObj ));
                    if ( pContact )
                    {
                        pContact->DisconnectFromLayout();
                    }
                }
            }
        }

        pFrm->RemoveFromLayout();
        SwFrm::DestroyFrm(pFrm);
        pFrm = m_pLower;
    }

    InvalidatePage();
}

sal_uInt32 SwFlyFrm::_GetOrdNumForNewRef( const SwFlyDrawContact* pContact )
{
    sal_uInt32 nOrdNum( 0L );

    // search for another Writer fly frame registered at same frame format
    SwIterator<SwFlyFrm,SwFmt> aIter( *pContact->GetFmt() );
    const SwFlyFrm* pFlyFrm( 0L );
    for ( pFlyFrm = aIter.First(); pFlyFrm; pFlyFrm = aIter.Next() )
    {
        if ( pFlyFrm != this )
        {
            break;
        }
    }

    if ( pFlyFrm )
    {
        // another Writer fly frame found. Take its order number
        nOrdNum = pFlyFrm->GetVirtDrawObj()->GetOrdNum();
    }
    else
    {
        // no other Writer fly frame found. Take order number of 'master' object
        // #i35748# - use method <GetOrdNumDirect()> instead
        // of method <GetOrdNum()> to avoid a recalculation of the order number,
        // which isn't intended.
        nOrdNum = pContact->GetMaster()->GetOrdNumDirect();
    }

    return nOrdNum;
}

SwVirtFlyDrawObj* SwFlyFrm::CreateNewRef( SwFlyDrawContact *pContact )
{
    SwVirtFlyDrawObj *pDrawObj = new SwVirtFlyDrawObj( *pContact->GetMaster(), this );
    pDrawObj->SetModel( pContact->GetMaster()->GetModel() );
    pDrawObj->SetUserCall( pContact );

    // The Reader creates the Masters and inserts them into the Page in
    // order to transport the z-order.
    // After creating the first Reference the Masters are removed from the
    // List and are not important anymore.
    SdrPage* pPg( 0L );
    if ( 0 != ( pPg = pContact->GetMaster()->GetPage() ) )
    {
        const size_t nOrdNum = pContact->GetMaster()->GetOrdNum();
        pPg->ReplaceObject( pDrawObj, nOrdNum );
    }
    // #i27030# - insert new <SwVirtFlyDrawObj> instance
    // into drawing page with correct order number
    else
    {
        pContact->GetFmt()->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 )->
                        InsertObject( pDrawObj, _GetOrdNumForNewRef( pContact ) );
    }
    // #i38889# - assure, that new <SwVirtFlyDrawObj> instance
    // is in a visible layer.
    pContact->MoveObjToVisibleLayer( pDrawObj );
    return pDrawObj;
}

void SwFlyFrm::InitDrawObj( bool bNotify )
{
    // Find ContactObject from the Format. If there's already one, we just
    // need to create a new Ref, else we create the Contact now.

    IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
    SwFlyDrawContact *pContact = SwIterator<SwFlyDrawContact,SwFmt>( *GetFmt() ).First();
    if ( !pContact )
    {
        // #i52858# - method name changed
        pContact = new SwFlyDrawContact( GetFmt(),
                                          pIDDMA->GetOrCreateDrawModel() );
    }
    OSL_ENSURE( pContact, "InitDrawObj failed" );
    // OD 2004-03-22 #i26791#
    SetDrawObj( *(CreateNewRef( pContact )) );

    // Set the right Layer
    // OD 2004-01-19 #110582#
    SdrLayerID nHeavenId = pIDDMA->GetHeavenId();
    SdrLayerID nHellId = pIDDMA->GetHellId();
    // OD 2004-03-22 #i26791#
    GetVirtDrawObj()->SetLayer( GetFmt()->GetOpaque().GetValue()
                                ? nHeavenId
                                : nHellId );
    if ( bNotify )
        NotifyDrawObj();
}

void SwFlyFrm::FinitDrawObj()
{
    if ( !GetVirtDrawObj() )
        return;

    // Deregister from SdrPageViews if the Objects is still selected there.
    if ( !GetFmt()->GetDoc()->IsInDtor() )
    {
        SwViewShell *p1St = getRootFrm()->GetCurrShell();
        if ( p1St )
        {
            for(SwViewShell& rCurrentShell : p1St->GetRingContainer())
            {   // At the moment the Drawing can do just do an Unmark on everything,
                // as the Object was already removed
                if( rCurrentShell.HasDrawView() )
                    rCurrentShell.Imp()->GetDrawView()->UnmarkAll();
            }
        }
    }

    // Take VirtObject to the grave.
    // If the last VirtObject is destroyed, the DrawObject and the DrawContact
    // also need to be destroyed.
    SwFlyDrawContact *pMyContact = 0;
    if ( GetFmt() )
    {
        bool bContinue = true;
        SwIterator<SwFrm,SwFmt> aFrmIter( *GetFmt() );
        for ( SwFrm* pFrm = aFrmIter.First(); pFrm; pFrm = aFrmIter.Next() )
            if ( pFrm != this )
            {
                // don't delete Contact if there is still a Frm
                bContinue = false;
                break;
            }

        if ( bContinue )
            // no Frm left, find Contact object to destroy
            pMyContact = SwIterator<SwFlyDrawContact,SwFmt>( *GetFmt() ).First();
    }

    // OD, OS 2004-03-31 #116203# - clear user call of Writer fly frame 'master'
    // <SdrObject> to assure, that a <SwXFrame::dispose()> doesn't delete the
    // Writer fly frame again.
    if ( pMyContact )
    {
        pMyContact->GetMaster()->SetUserCall( 0 );
    }
    GetVirtDrawObj()->SetUserCall( 0 ); // Else calls delete of the ContactObj
    delete GetVirtDrawObj();            // Deregisters itself at the Master
    delete pMyContact;                  // Destroys the Master itself
}

void SwFlyFrm::ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    OSL_ENSURE( pMaster && pFollow, "uncomplete chain" );
    OSL_ENSURE( !pMaster->GetNextLink(), "link can not be changed" );
    OSL_ENSURE( !pFollow->GetPrevLink(), "link can not be changed" );

    pMaster->pNextLink = pFollow;
    pFollow->pPrevLink = pMaster;

    if ( pMaster->ContainsCntnt() )
    {
        // To get a text flow we need to invalidate
        SwFrm *pInva = pMaster->FindLastLower();
        SWRECTFN( pMaster )
        const long nBottom = (pMaster->*fnRect->fnGetPrtBottom)();
        while ( pInva )
        {
            if( (pInva->Frm().*fnRect->fnBottomDist)( nBottom ) <= 0 )
            {
                pInva->InvalidateSize();
                pInva->Prepare( PREP_CLEAR );
                pInva = pInva->FindPrev();
            }
            else
                pInva = 0;
        }
    }

    if ( pFollow->ContainsCntnt() )
    {
        // There's only the content from the Masters left; the content from the Follow
        // does not have any Frames left (should always be exactly one empty TxtNode).
        SwFrm *pFrm = pFollow->ContainsCntnt();
        OSL_ENSURE( !pFrm->IsTabFrm() && !pFrm->FindNext(), "follow in chain contains content" );
        pFrm->Cut();
        SwFrm::DestroyFrm(pFrm);
    }

    // invalidate accessible relation set (accessibility wrapper)
    SwViewShell* pSh = pMaster->getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = pMaster->getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

void SwFlyFrm::UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    pMaster->pNextLink = 0;
    pFollow->pPrevLink = 0;

    if ( pFollow->ContainsCntnt() )
    {
        // The Master sucks up the content of the Follow
        SwLayoutFrm *pUpper = pMaster;
        if ( pUpper->Lower()->IsColumnFrm() )
        {
            pUpper = static_cast<SwLayoutFrm*>(pUpper->GetLastLower());
            pUpper = static_cast<SwLayoutFrm*>(pUpper->Lower()); // The (Column)BodyFrm
            OSL_ENSURE( pUpper && pUpper->IsColBodyFrm(), "Missing ColumnBody" );
        }
        SwFlyFrm *pFoll = pFollow;
        while ( pFoll )
        {
            SwFrm *pTmp = ::SaveCntnt( pFoll );
            if ( pTmp )
                ::RestoreCntnt( pTmp, pUpper, pMaster->FindLastLower(), true );
            pFoll->SetCompletePaint();
            pFoll->InvalidateSize();
            pFoll = pFoll->GetNextLink();
        }
    }

    // The Follow needs his own content to be served
    const SwFmtCntnt &rCntnt = pFollow->GetFmt()->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), ":-( No content prepared." );
    sal_uLong nIndex = rCntnt.GetCntntIdx()->GetIndex();
    // Lower() means SwColumnFrm: this one contains another SwBodyFrm
    ::_InsertCnt( pFollow->Lower() ? const_cast<SwLayoutFrm*>(static_cast<const SwLayoutFrm*>(static_cast<const SwLayoutFrm*>(pFollow->Lower())->Lower()))
                                   : static_cast<SwLayoutFrm*>(pFollow),
                  pFollow->GetFmt()->GetDoc(), ++nIndex );

    // invalidate accessible relation set (accessibility wrapper)
    SwViewShell* pSh = pMaster->getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = pMaster->getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

SwFlyFrm *SwFlyFrm::FindChainNeighbour( SwFrmFmt &rChain, SwFrm *pAnch )
{
    // We look for the Fly that's in the same Area.
    // Areas can for now only be Head/Footer or Flys.

    if ( !pAnch ) // If an Anchor was passed along, that one counts (ctor!)
        pAnch = AnchorFrm();

    SwLayoutFrm *pLay;
    if ( pAnch->IsInFly() )
        pLay = pAnch->FindFlyFrm();
    else
    {
        // FindFooterOrHeader is not appropriate here, as we may not have a
        // connection to the Anchor yet.
        pLay = pAnch->GetUpper();
        while ( pLay && !(pLay->GetType() & (FRM_HEADER|FRM_FOOTER)) )
            pLay = pLay->GetUpper();
    }

    SwIterator<SwFlyFrm,SwFmt> aIter( rChain );
    SwFlyFrm *pFly = aIter.First();
    if ( pLay )
    {
        while ( pFly )
        {
            if ( pFly->GetAnchorFrm() )
            {
                if ( pFly->GetAnchorFrm()->IsInFly() )
                {
                    if ( pFly->AnchorFrm()->FindFlyFrm() == pLay )
                        break;
                }
                else if ( pLay == pFly->FindFooterOrHeader() )
                    break;
            }
            pFly = aIter.Next();
        }
    }
    else if ( pFly )
    {
        OSL_ENSURE( !aIter.Next(), "chain with more than one instance" );
    }
    return pFly;
}

SwFrm *SwFlyFrm::FindLastLower()
{
    SwFrm *pRet = ContainsAny();
    if ( pRet && pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    SwFrm *pNxt = pRet;
    while ( pNxt && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNext();
    }
    return pRet;
}

bool SwFlyFrm::FrmSizeChg( const SwFmtFrmSize &rFrmSize )
{
    bool bRet = false;
    SwTwips nDiffHeight = Frm().Height();
    if ( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE )
        mbFixSize = bMinHeight = false;
    else
    {
        if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        {
            mbFixSize = true;
            bMinHeight = false;
        }
        else if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        {
            mbFixSize = false;
            bMinHeight = true;
        }
        nDiffHeight -= rFrmSize.GetHeight();
    }
    // If the Fly contains columns, we already need to set the Fly
    // and the Columns to the required value or else we run into problems.
    if ( Lower() )
    {
        if ( Lower()->IsColumnFrm() )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            const Size   aOldSz( Prt().SSize() );
            const SwTwips nDiffWidth = Frm().Width() - rFrmSize.GetWidth();
            maFrm.Height( maFrm.Height() - nDiffHeight );
            maFrm.Width ( maFrm.Width()  - nDiffWidth  );
            // #i68520#
            InvalidateObjRectWithSpaces();
            maPrt.Height( maPrt.Height() - nDiffHeight );
            maPrt.Width ( maPrt.Width()  - nDiffWidth  );
            ChgLowersProp( aOldSz );
            ::Notify( this, FindPageFrm(), aOld );
            mbValidPos = false;
            bRet = true;
        }
        else if ( Lower()->IsNoTxtFrm() )
        {
            mbFixSize = true;
            bMinHeight = false;
        }
    }
    return bRet;
}

void SwFlyFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if (pNew && pOld && RES_ATTRSET_CHG == pNew->Which())
    {
        SfxItemIter aNIter( *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet() );
        SfxItemIter aOIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *static_cast<const SwAttrSetChg*>(pOld) );
        SwAttrSetChg aNewSet( *static_cast<const SwAttrSetChg*>(pNew) );
        while( true )
        {
            _UpdateAttr( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        _Invalidate();
        if ( nInvFlags & 0x01 )
        {
            _InvalidatePos();
            // #i68520#
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x02 )
        {
            _InvalidateSize();
            // #i68520#
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x04 )
            _InvalidatePrt();
        if ( nInvFlags & 0x08 )
            SetNotifyBack();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
        if ( ( nInvFlags & 0x40 ) && Lower() && Lower()->IsNoTxtFrm() )
            ClrContourCache( GetVirtDrawObj() );
        SwRootFrm *pRoot;
        if ( nInvFlags & 0x20 && 0 != (pRoot = getRootFrm()) )
            pRoot->InvalidateBrowseWidth();
        // #i28701#
        if ( nInvFlags & 0x80 )
        {
            // update sorted object lists, the Writer fly frame is registered at.
            UpdateObjInSortedList();
        }

        // #i87645# - reset flags for the layout process (only if something has been invalidated)
        ResetLayoutProcessBools();
    }
}

void SwFlyFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    switch( nWhich )
    {
        case RES_VERT_ORIENT:
        case RES_HORI_ORIENT:
        // OD 22.09.2003 #i18732# - consider new option 'follow text flow'
        case RES_FOLLOW_TEXT_FLOW:
        {
            // ATTENTION: Always also change Action in ChgRePos()!
            rInvFlags |= 0x09;
        }
        break;
        // OD 2004-07-01 #i28701# - consider new option 'wrap influence on position'
        case RES_WRAP_INFLUENCE_ON_OBJPOS:
        {
            rInvFlags |= 0x89;
        }
        break;
        case RES_SURROUND:
        {
            // OD 2004-05-13 #i28701# - invalidate position on change of
            // wrapping style.
            //rInvFlags |= 0x40;
            rInvFlags |= 0x41;
            // The background needs to messaged and invalidated
            const SwRect aTmp( GetObjRectWithSpaces() );
            NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_ATTR_CHG );

            // By changing the flow of frame-bound Frames, a vertical alignment
            // can be activated/deactivated => MakeFlyPos
            if( FLY_AT_FLY == GetFmt()->GetAnchor().GetAnchorId() )
                rInvFlags |= 0x09;

            // Delete contour in the Node if necessary
            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 !GetFmt()->GetSurround().IsContour() )
            {
                SwNoTxtNode *pNd = static_cast<SwNoTxtNode*>(static_cast<SwCntntFrm*>(Lower())->GetNode());
                if ( pNd->HasContour() )
                    pNd->SetContour( 0 );
            }
            // #i28701# - perform reorder of object lists
            // at anchor frame and at page frame.
            rInvFlags |= 0x80;
        }
        break;

        case RES_PROTECT:
            if (pNew)
            {
                const SvxProtectItem *pP = static_cast<const SvxProtectItem*>(pNew);
                GetVirtDrawObj()->SetMoveProtect( pP->IsPosProtected()   );
                GetVirtDrawObj()->SetResizeProtect( pP->IsSizeProtected() );
                if( pSh )
                {
                    SwRootFrm* pLayout = getRootFrm();
                    if( pLayout && pLayout->IsAnyShellAccessible() )
                    pSh->Imp()->InvalidateAccessibleEditableState( true, this );
                }
            }
            break;
        case RES_COL:
            if (pOld && pNew)
            {
                ChgColumns( *static_cast<const SwFmtCol*>(pOld), *static_cast<const SwFmtCol*>(pNew) );
                const SwFmtFrmSize &rNew = GetFmt()->GetFrmSize();
                if ( FrmSizeChg( rNew ) )
                    NotifyDrawObj();
                rInvFlags |= 0x1A;
                break;
            }

        case RES_FRM_SIZE:
        case RES_FMT_CHG:
        {
            const SwFmtFrmSize &rNew = GetFmt()->GetFrmSize();
            if ( FrmSizeChg( rNew ) )
                NotifyDrawObj();
            rInvFlags |= 0x7F;
            if ( RES_FMT_CHG == nWhich )
            {
                SwRect aNew( GetObjRectWithSpaces() );
                SwRect aOld( maFrm );
                const SvxULSpaceItem &rUL = static_cast<const SwFmtChg*>(pOld)->pChangedFmt->GetULSpace();
                aOld.Top( std::max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                aOld.SSize().Height()+= rUL.GetLower();
                const SvxLRSpaceItem &rLR = static_cast<const SwFmtChg*>(pOld)->pChangedFmt->GetLRSpace();
                aOld.Left  ( std::max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                aOld.SSize().Width() += rLR.GetRight();
                aNew.Union( aOld );
                NotifyBackground( FindPageFrm(), aNew, PREP_CLEAR );

                // Special case:
                // When assigning a template we cannot rely on the old column
                // attribute. As there need to be at least enough for ChgColumns,
                // we need to create a temporary attribute.
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrm *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                ChgColumns( aCol, GetFmt()->GetCol() );
            }

            SwFmtURL aURL( GetFmt()->GetURL() );

            SwFmtFrmSize *pNewFmtFrmSize = NULL;
            SwFmtChg *pOldFmtChg = NULL;
            if (nWhich == RES_FRM_SIZE)
                pNewFmtFrmSize = const_cast<SwFmtFrmSize*>(static_cast<const SwFmtFrmSize*>(pNew));
            else
                pOldFmtChg = const_cast<SwFmtChg*>(static_cast<const SwFmtChg*>(pOld));

            if (aURL.GetMap() && (pNewFmtFrmSize || pOldFmtChg))
            {
                const SwFmtFrmSize &rOld = pNewFmtFrmSize ?
                                *pNewFmtFrmSize :
                                pOldFmtChg->pChangedFmt->GetFrmSize();
                //#35091# Can be "times zero", when loading the template
                if ( rOld.GetWidth() && rOld.GetHeight() )
                {

                    Fraction aScaleX( rOld.GetWidth(), rNew.GetWidth() );
                    Fraction aScaleY( rOld.GetHeight(), rOld.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrmFmt *pFmt = GetFmt();
                    pFmt->LockModify();
                    pFmt->SetFmtAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            const SvxProtectItem &rP = GetFmt()->GetProtect();
            GetVirtDrawObj()->SetMoveProtect( rP.IsPosProtected()    );
            GetVirtDrawObj()->SetResizeProtect( rP.IsSizeProtected() );

            if ( pSh )
                pSh->InvalidateWindows( Frm() );
            const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
            const sal_uInt8 nId = GetFmt()->GetOpaque().GetValue() ?
                             pIDDMA->GetHeavenId() :
                             pIDDMA->GetHellId();
            GetVirtDrawObj()->SetLayer( nId );

            if ( Lower() )
            {
                // Delete contour in the Node if necessary
                if( Lower()->IsNoTxtFrm() &&
                     !GetFmt()->GetSurround().IsContour() )
                {
                    SwNoTxtNode *pNd = static_cast<SwNoTxtNode*>(static_cast<SwCntntFrm*>(Lower())->GetNode());
                    if ( pNd->HasContour() )
                        pNd->SetContour( 0 );
                }
                else if( !Lower()->IsColumnFrm() )
                {
                    SwFrm* pFrm = GetLastLower();
                    if( pFrm->IsTxtFrm() && static_cast<SwTxtFrm*>(pFrm)->IsUndersized() )
                        pFrm->Prepare( PREP_ADJUST_FRM );
                }
            }

            // #i28701# - perform reorder of object lists
            // at anchor frame and at page frame.
            rInvFlags |= 0x80;

            break;
        }
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        {
            rInvFlags |= 0x41;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                getRootFrm()->InvalidateBrowseWidth();
            SwRect aNew( GetObjRectWithSpaces() );
            SwRect aOld( maFrm );
            if (pNew)
            {
                if ( RES_UL_SPACE == nWhich )
                {
                    const SvxULSpaceItem &rUL = *static_cast<const SvxULSpaceItem*>(pNew);
                    aOld.Top( std::max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                    aOld.SSize().Height()+= rUL.GetLower();
                }
                else
                {
                    const SvxLRSpaceItem &rLR = *static_cast<const SvxLRSpaceItem*>(pNew);
                    aOld.Left  ( std::max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                    aOld.SSize().Width() += rLR.GetRight();
                }
            }
            aNew.Union( aOld );
            NotifyBackground( FindPageFrm(), aNew, PREP_CLEAR );
        }
        break;

        case RES_TEXT_VERT_ADJUST:
        {
            InvalidateContentPos();
            rInvFlags |= 0x10;
        }
        break;

        case RES_BOX:
        case RES_SHADOW:
            rInvFlags |= 0x17;
            break;

        case RES_FRAMEDIR :
            SetDerivedVert( false );
            SetDerivedR2L( false );
            CheckDirChange();
            break;

        case RES_OPAQUE:
            {
                if ( pSh )
                    pSh->InvalidateWindows( Frm() );

                const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
                const sal_uInt8 nId = static_cast<const SvxOpaqueItem*>(pNew)->GetValue() ?
                                    pIDDMA->GetHeavenId() :
                                    pIDDMA->GetHellId();
                GetVirtDrawObj()->SetLayer( nId );
                if( pSh )
                {
                    SwRootFrm* pLayout = getRootFrm();
                    if( pLayout && pLayout->IsAnyShellAccessible() )
                {
                    pSh->Imp()->DisposeAccessibleFrm( this );
                    pSh->Imp()->AddAccessibleFrm( this );
                }
                }
                // #i28701# - perform reorder of object lists
                // at anchor frame and at page frame.
                rInvFlags |= 0x80;
            }
            break;

        case RES_URL:
            // The interface changes the frame size when interacting with text frames,
            // the Map, however, needs to be relative to FrmSize().
            if ( (!Lower() || !Lower()->IsNoTxtFrm()) && pNew && pOld &&
                 static_cast<const SwFmtURL*>(pNew)->GetMap() && static_cast<const SwFmtURL*>(pOld)->GetMap() )
            {
                const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
                if ( rSz.GetHeight() != Frm().Height() ||
                     rSz.GetWidth()  != Frm().Width() )
                {
                    SwFmtURL aURL( GetFmt()->GetURL() );
                    Fraction aScaleX( Frm().Width(),  rSz.GetWidth() );
                    Fraction aScaleY( Frm().Height(), rSz.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrmFmt *pFmt = GetFmt();
                    pFmt->LockModify();
                    pFmt->SetFmtAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            // No invalidation necessary
            break;

        case RES_CHAIN:
            if (pNew)
            {
                const SwFmtChain *pChain = static_cast<const SwFmtChain*>(pNew);
                if ( pChain->GetNext() )
                {
                    SwFlyFrm *pFollow = FindChainNeighbour( *pChain->GetNext() );
                    if ( GetNextLink() && pFollow != GetNextLink() )
                        SwFlyFrm::UnchainFrames( this, GetNextLink());
                    if ( pFollow )
                    {
                        if ( pFollow->GetPrevLink() &&
                             pFollow->GetPrevLink() != this )
                            SwFlyFrm::UnchainFrames( pFollow->GetPrevLink(),
                                                     pFollow );
                        if ( !GetNextLink() )
                            SwFlyFrm::ChainFrames( this, pFollow );
                    }
                }
                else if ( GetNextLink() )
                    SwFlyFrm::UnchainFrames( this, GetNextLink() );
                if ( pChain->GetPrev() )
                {
                    SwFlyFrm *pMaster = FindChainNeighbour( *pChain->GetPrev() );
                    if ( GetPrevLink() && pMaster != GetPrevLink() )
                        SwFlyFrm::UnchainFrames( GetPrevLink(), this );
                    if ( pMaster )
                    {
                        if ( pMaster->GetNextLink() &&
                             pMaster->GetNextLink() != this )
                            SwFlyFrm::UnchainFrames( pMaster,
                                                     pMaster->GetNextLink() );
                        if ( !GetPrevLink() )
                            SwFlyFrm::ChainFrames( pMaster, this );
                    }
                }
                else if ( GetPrevLink() )
                    SwFlyFrm::UnchainFrames( GetPrevLink(), this );
            }
            //fall-through
        default:
            bClear = false;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/// Gets information from the Modify
bool SwFlyFrm::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
        return false;   // There's a FlyFrm, so use it
    return true;        // Continue searching
}

void SwFlyFrm::_Invalidate( SwPageFrm *pPage )
{
    InvalidatePage( pPage );
    bNotifyBack = bInvalid = true;

    SwFlyFrm *pFrm;
    if ( GetAnchorFrm() && 0 != (pFrm = AnchorFrm()->FindFlyFrm()) )
    {
        // Very bad case: If the Fly is bound within another Fly which
        // contains columns, the Format should be from that one.
        if ( !pFrm->IsLocked() && !pFrm->IsColLocked() &&
             pFrm->Lower() && pFrm->Lower()->IsColumnFrm() )
            pFrm->InvalidateSize();
    }

    // #i85216#
    // if vertical position is oriented at a layout frame inside a ghost section,
    // assure that the position is invalidated and that the information about
    // the vertical position oriented frame is cleared
    if ( GetVertPosOrientFrm() && GetVertPosOrientFrm()->IsLayoutFrm() )
    {
        const SwSectionFrm* pSectFrm( GetVertPosOrientFrm()->FindSctFrm() );
        if ( pSectFrm && pSectFrm->GetSection() == 0 )
        {
            InvalidatePos();
            ClearVertPosOrientFrm();
        }
    }
}

/** Change the relative position
 *
 * The position will be Fix automatically and the attribute is changed accordingly.
 */
void SwFlyFrm::ChgRelPos( const Point &rNewPos )
{
    if ( GetCurrRelPos() != rNewPos )
    {
        SwFrmFmt *pFmt = GetFmt();
        const bool bVert = GetAnchorFrm()->IsVertical();
        const SwTwips nNewY = bVert ? rNewPos.X() : rNewPos.Y();
        SwTwips nTmpY = nNewY == LONG_MAX ? 0 : nNewY;
        if( bVert )
            nTmpY = -nTmpY;
        SfxItemSet aSet( pFmt->GetDoc()->GetAttrPool(),
                         RES_VERT_ORIENT, RES_HORI_ORIENT);

        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        const SwTxtFrm *pAutoFrm = NULL;
        // #i34948# - handle also at-page and at-fly anchored
        // Writer fly frames
        const RndStdIds eAnchorType = GetFrmFmt().GetAnchor().GetAnchorId();
        if ( eAnchorType == FLY_AT_PAGE )
        {
            aVert.SetVertOrient( text::VertOrientation::NONE );
            aVert.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
        }
        else if ( eAnchorType == FLY_AT_FLY )
        {
            aVert.SetVertOrient( text::VertOrientation::NONE );
            aVert.SetRelationOrient( text::RelOrientation::FRAME );
        }
        else if ( IsFlyAtCntFrm() || text::VertOrientation::NONE != aVert.GetVertOrient() )
        {
            if( text::RelOrientation::CHAR == aVert.GetRelationOrient() && IsAutoPos() )
            {
                if( LONG_MAX != nNewY )
                {
                    aVert.SetVertOrient( text::VertOrientation::NONE );
                    sal_Int32 nOfs =
                        pFmt->GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    OSL_ENSURE( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected" );
                    pAutoFrm = static_cast<const SwTxtFrm*>(GetAnchorFrm());
                    while( pAutoFrm->GetFollow() &&
                           pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                    {
                        if( pAutoFrm == GetAnchorFrm() )
                            nTmpY += pAutoFrm->GetRelPos().Y();
                        nTmpY -= pAutoFrm->GetUpper()->Prt().Height();
                        pAutoFrm = pAutoFrm->GetFollow();
                    }
                    nTmpY = static_cast<SwFlyAtCntFrm*>(this)->GetRelCharY(pAutoFrm)-nTmpY;
                }
                else
                    aVert.SetVertOrient( text::VertOrientation::CHAR_BOTTOM );
            }
            else
            {
                aVert.SetVertOrient( text::VertOrientation::NONE );
                aVert.SetRelationOrient( text::RelOrientation::FRAME );
            }
        }
        aVert.SetPos( nTmpY );
        aSet.Put( aVert );

        // For Flys in the Cnt, the horizontal orientation is of no interest,
        // as it's always 0
        if ( !IsFlyInCntFrm() )
        {
            const SwTwips nNewX = bVert ? rNewPos.Y() : rNewPos.X();
            SwTwips nTmpX = nNewX == LONG_MAX ? 0 : nNewX;
            SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
            // #i34948# - handle also at-page and at-fly anchored
            // Writer fly frames
            if ( eAnchorType == FLY_AT_PAGE )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aHori.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
                aHori.SetPosToggle( false );
            }
            else if ( eAnchorType == FLY_AT_FLY )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aHori.SetRelationOrient( text::RelOrientation::FRAME );
                aHori.SetPosToggle( false );
            }
            else if ( IsFlyAtCntFrm() || text::HoriOrientation::NONE != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                if( text::RelOrientation::CHAR == aHori.GetRelationOrient() && IsAutoPos() )
                {
                    if( LONG_MAX != nNewX )
                    {
                        if( !pAutoFrm )
                        {
                            sal_Int32 nOfs = pFmt->GetAnchor().GetCntntAnchor()
                                          ->nContent.GetIndex();
                            OSL_ENSURE( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected");
                            pAutoFrm = static_cast<const SwTxtFrm*>(GetAnchorFrm());
                            while( pAutoFrm->GetFollow() &&
                                   pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                                pAutoFrm = pAutoFrm->GetFollow();
                        }
                        nTmpX -= static_cast<SwFlyAtCntFrm*>(this)->GetRelCharX(pAutoFrm);
                    }
                }
                else
                    aHori.SetRelationOrient( text::RelOrientation::FRAME );
                aHori.SetPosToggle( false );
            }
            aHori.SetPos( nTmpX );
            aSet.Put( aHori );
        }
        SetCurrRelPos( rNewPos );
        pFmt->GetDoc()->SetAttr( aSet, *pFmt );
    }
}

/** "Formats" the Frame; Frm and PrtArea.
 *
 * The FixSize is not inserted here.
 */
void SwFlyFrm::Format( const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "FlyFrm::Format, pAttrs is 0." );

    ColLock();

    if ( !mbValidSize )
    {
        if ( Frm().Top() == FAR_AWAY && Frm().Left() == FAR_AWAY )
        {
            // Remove safety switch (see SwFrm::CTor)
            Frm().Pos().setX(0);
            Frm().Pos().setY(0);
            // #i68520#
            InvalidateObjRectWithSpaces();
        }

        // Check column width and set it if needed
        if ( Lower() && Lower()->IsColumnFrm() )
            AdjustColumns( 0, false );

        mbValidSize = true;

        const SwTwips nUL = pAttrs->CalcTopLine()  + pAttrs->CalcBottomLine();
        const SwTwips nLR = pAttrs->CalcLeftLine() + pAttrs->CalcRightLine();
        const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
        Size aRelSize( CalcRel( rFrmSz ) );

        OSL_ENSURE( pAttrs->GetSize().Height() != 0 || rFrmSz.GetHeightPercent(), "FrameAttr height is 0." );
        OSL_ENSURE( pAttrs->GetSize().Width()  != 0 || rFrmSz.GetWidthPercent(), "FrameAttr width is 0." );

        SWRECTFN( this )
        if( !HasFixSize() )
        {
            long nMinHeight = 0;
            if( IsMinHeight() )
                nMinHeight = bVert ? aRelSize.Width() : aRelSize.Height();

            SwTwips nRemaining = CalcContentHeight(pAttrs, nMinHeight, nUL);
            if( IsMinHeight() && (nRemaining + nUL) < nMinHeight )
                nRemaining = nMinHeight - nUL;
            // Because the Grow/Shrink of the Flys does not directly
            // set the size - only indirectly by triggering a Format()
            // via Invalidate() - the sizes need to be set here.
            // Notification is running along already.
            // As we already got a lot of zeros per attribute, we block them
            // from now on.

            if ( nRemaining < MINFLY )
                nRemaining = MINFLY;

            (Prt().*fnRect->fnSetHeight)( nRemaining );
            nRemaining -= (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nRemaining + nUL );
            // #i68520#
            if ( nRemaining + nUL != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
            mbValidSize = true;

            std::map<SwFrmFmt*, SwFrmFmt*> aShapes = SwTextBoxHelper::findShapes(GetFmt()->GetDoc());
            if (aShapes.find(GetFmt()) != aShapes.end())
            {
                // This fly is a textbox of a draw shape.
                SdrObject* pShape = aShapes[GetFmt()]->FindSdrObject();
                if (SdrObjCustomShape* pCustomShape = PTR_CAST(SdrObjCustomShape, pShape))
                {
                    // The shape is a customshape: then inform it about the calculated fly size.
                    Size aSize((Frm().*fnRect->fnGetWidth)(), (Frm().*fnRect->fnGetHeight)());
                    pCustomShape->SuggestTextFrameSize(aSize);
                    // Do the calculations normally done after touching editeng text of the shape.
                    pCustomShape->NbcSetOutlinerParaObjectForText(0, 0);
                }
            }
        }
        else
        {
            mbValidSize = true;  // Fixed Frms do not Format itself
            // Flys set their size using the attr
            SwTwips nNewSize = bVert ? aRelSize.Width() : aRelSize.Height();
            nNewSize -= nUL;
            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetHeight)( nNewSize );
            nNewSize += nUL - (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nNewSize );
            // #i68520#
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }

        if ( !bFormatHeightOnly )
        {
            OSL_ENSURE( aRelSize == CalcRel( rFrmSz ), "SwFlyFrm::Format CalcRel problem" );
            SwTwips nNewSize = bVert ? aRelSize.Height() : aRelSize.Width();

            if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
            {
                // #i9046# Autowidth for fly frames
                const SwTwips nAutoWidth = CalcAutoWidth();
                if ( nAutoWidth )
                {
                    if( ATT_MIN_SIZE == rFrmSz.GetWidthSizeType() )
                        nNewSize = std::max( nNewSize - nLR, nAutoWidth );
                    else
                        nNewSize = nAutoWidth;
                }
            }
            else
                nNewSize -= nLR;

            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetWidth)( nNewSize );
            nNewSize += nLR - (Frm().*fnRect->fnGetWidth)();
            (Frm().*fnRect->fnAddRight)( nNewSize );
            // #i68520#
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }
    }
    ColUnlock();
}

// OD 14.03.2003 #i11760# - change parameter <bNoColl>: type <bool>;
//                          default value = false.
// OD 14.03.2003 #i11760# - add new parameter <bNoCalcFollow> with
//                          default value = false.
// OD 11.04.2003 #108824# - new parameter <bNoCalcFollow> was used by method
//                          <FormatWidthCols(..)> to avoid follow formatting
//                          for text frames. But, unformatted follows causes
//                          problems in method <SwCntntFrm::_WouldFit(..)>,
//                          which assumes that the follows are formatted.
//                          Thus, <bNoCalcFollow> no longer used by <FormatWidthCols(..)>.
void CalcCntnt( SwLayoutFrm *pLay,
                bool bNoColl,
                bool bNoCalcFollow )
{
    SwSectionFrm* pSect;
    bool bCollect = false;
    if( pLay->IsSctFrm() )
    {
        pSect = static_cast<SwSectionFrm*>(pLay);
        if( pSect->IsEndnAtEnd() && !bNoColl )
        {
            bCollect = true;
            SwLayouter::CollectEndnotes( pLay->GetFmt()->GetDoc(), pSect );
        }
        pSect->CalcFtnCntnt();
    }
    else
        pSect = NULL;
    SwFrm *pFrm = pLay->ContainsAny();
    if ( !pFrm )
    {
        if( pSect )
        {
            if( pSect->HasFollow() )
                pFrm = pSect->GetFollow()->ContainsAny();
            if( !pFrm )
            {
                if( pSect->IsEndnAtEnd() )
                {
                    if( bCollect )
                        pLay->GetFmt()->GetDoc()->getIDocumentLayoutAccess().GetLayouter()->
                            InsertEndnotes( pSect );
                    bool bLock = pSect->IsFtnLock();
                    pSect->SetFtnLock( true );
                    pSect->CalcFtnCntnt();
                    pSect->CalcFtnCntnt();
                    pSect->SetFtnLock( bLock );
                }
                return;
            }
            pFrm->_InvalidatePos();
        }
        else
            return;
    }
    pFrm->InvalidatePage();

    do
    {
        // local variables to avoid loops caused by anchored object positioning
        SwAnchoredObject* pAgainObj1 = 0;
        SwAnchoredObject* pAgainObj2 = 0;

        // FME 2007-08-30 #i81146# new loop control
        int nLoopControlRuns = 0;
        const int nLoopControlMax = 20;
        const SwFrm* pLoopControlCond = 0;

        SwFrm* pLast;
        do
        {
            pLast = pFrm;
            if( pFrm->IsVertical() ?
                ( pFrm->GetUpper()->Prt().Height() != pFrm->Frm().Height() )
                : ( pFrm->GetUpper()->Prt().Width() != pFrm->Frm().Width() ) )
            {
                pFrm->Prepare( PREP_FIXSIZE_CHG );
                pFrm->_InvalidateSize();
            }

            if ( pFrm->IsTabFrm() )
            {
                static_cast<SwTabFrm*>(pFrm)->m_bCalcLowers = true;
                // OD 26.08.2003 #i18103# - lock move backward of follow table,
                // if no section content is formatted or follow table belongs
                // to the section, which content is formatted.
                if ( static_cast<SwTabFrm*>(pFrm)->IsFollow() &&
                     ( !pSect || pSect == pFrm->FindSctFrm() ) )
                {
                    static_cast<SwTabFrm*>(pFrm)->m_bLockBackMove = true;
                }
            }

            // OD 14.03.2003 #i11760# - forbid format of follow, if requested.
            if ( bNoCalcFollow && pFrm->IsTxtFrm() )
                static_cast<SwTxtFrm*>(pFrm)->ForbidFollowFormat();

            pFrm->Calc();

            // OD 14.03.2003 #i11760# - reset control flag for follow format.
            if ( pFrm->IsTxtFrm() )
            {
                static_cast<SwTxtFrm*>(pFrm)->AllowFollowFormat();
            }

            // #111937# The keep-attribute can cause the position
            // of the prev to be invalid:
            // OD 2004-03-15 #116560# - Do not consider invalid previous frame
            // due to its keep-attribute, if current frame is a follow or is locked.
            // #i44049# - do not consider invalid previous
            // frame due to its keep-attribute, if it can't move forward.
            // #i57765# - do not consider invalid previous
            // frame, if current frame has a column/page break before attribute.
            SwFrm* pTmpPrev = pFrm->FindPrev();
            SwFlowFrm* pTmpPrevFlowFrm = pTmpPrev && pTmpPrev->IsFlowFrm() ? SwFlowFrm::CastFlowFrm(pTmpPrev) : 0;
            SwFlowFrm* pTmpFlowFrm     = pFrm->IsFlowFrm() ? SwFlowFrm::CastFlowFrm(pFrm) : 0;

            bool bPrevInvalid = pTmpPrevFlowFrm && pTmpFlowFrm &&
                               !pTmpFlowFrm->IsFollow() &&
                               !StackHack::IsLocked() && // #i76382#
                               !pTmpFlowFrm->IsJoinLocked() &&
                               !pTmpPrev->GetValidPosFlag() &&
                                pLay->IsAnLower( pTmpPrev ) &&
                                pTmpPrevFlowFrm->IsKeep( *pTmpPrev->GetAttrSet() ) &&
                                pTmpPrevFlowFrm->IsKeepFwdMoveAllowed();

            // format floating screen objects anchored to the frame.
            bool bRestartLayoutProcess = false;
            if ( !bPrevInvalid && pFrm->GetDrawObjs() && pLay->IsAnLower( pFrm ) )
            {
                bool bAgain = false;
                SwPageFrm* pPageFrm = pFrm->FindPageFrm();
                size_t nCnt = pFrm->GetDrawObjs()->size();
                size_t i = 0;
                while ( i < nCnt )
                {
                    // #i28701#
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];

                    // determine if anchored object has to be
                    // formatted and, in case, format it
                    if ( !pAnchoredObj->PositionLocked() && pAnchoredObj->IsFormatPossible() )
                    {
                        // #i43737# - no invalidation of
                        // anchored object needed - causes loops for as-character
                        // anchored objects.
                        //pAnchoredObj->InvalidateObjPos();
                        SwRect aRect( pAnchoredObj->GetObjRect() );
                        if ( !SwObjectFormatter::FormatObj( *pAnchoredObj, pFrm, pPageFrm ) )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }
                        // #i3317# - restart layout process,
                        // if the position of the anchored object is locked now.
                        if ( pAnchoredObj->PositionLocked() )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }

                        if ( aRect != pAnchoredObj->GetObjRect() )
                        {
                            bAgain = true;
                            if ( pAgainObj2 == pAnchoredObj )
                            {
                                OSL_FAIL( "::CalcCntnt(..) - loop detected, perform attribute changes to avoid the loop" );
                                // Prevent oscillation
                                SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                                SwFmtSurround aAttr( rFmt.GetSurround() );
                                if( SURROUND_THROUGHT != aAttr.GetSurround() )
                                {
                                    // When on auto position, we can only set it to
                                    // flow through
                                    if ((rFmt.GetAnchor().GetAnchorId() ==
                                            FLY_AT_CHAR) &&
                                        (SURROUND_PARALLEL ==
                                            aAttr.GetSurround()))
                                    {
                                        aAttr.SetSurround( SURROUND_THROUGHT );
                                    }
                                    else
                                    {
                                        aAttr.SetSurround( SURROUND_PARALLEL );
                                    }
                                    rFmt.LockModify();
                                    rFmt.SetFmtAttr( aAttr );
                                    rFmt.UnlockModify();
                                }
                            }
                            else
                            {
                                if ( pAgainObj1 == pAnchoredObj )
                                    pAgainObj2 = pAnchoredObj;
                                pAgainObj1 = pAnchoredObj;
                            }
                        }

                        if ( !pFrm->GetDrawObjs() )
                            break;
                        if ( pFrm->GetDrawObjs()->size() < nCnt )
                        {
                            --nCnt;
                            // Do not increment index, in this case
                            continue;
                        }
                    }
                    ++i;
                }

                // #i28701# - restart layout process, if
                // requested by floating screen object formatting
                if ( bRestartLayoutProcess )
                {
                    pFrm = pLay->ContainsAny();
                    pAgainObj1 = 0L;
                    pAgainObj2 = 0L;
                    continue;
                }

                // OD 2004-05-17 #i28701# - format anchor frame after its objects
                // are formatted, if the wrapping style influence has to be considered.
                if ( pLay->GetFmt()->getIDocumentSettingAccess()->get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    pFrm->Calc();
                }

                if ( bAgain )
                {
                    pFrm = pLay->ContainsCntnt();
                    if ( pFrm && pFrm->IsInTab() )
                        pFrm = pFrm->FindTabFrm();
                    if( pFrm && pFrm->IsInSct() )
                    {
                        SwSectionFrm* pTmp = pFrm->FindSctFrm();
                        if( pTmp != pLay && pLay->IsAnLower( pTmp ) )
                            pFrm = pTmp;
                    }

                    if ( pFrm == pLoopControlCond )
                        ++nLoopControlRuns;
                    else
                    {
                        nLoopControlRuns = 0;
                        pLoopControlCond = pFrm;
                    }

                    if ( nLoopControlRuns < nLoopControlMax )
                        continue;

                    OSL_FAIL( "LoopControl in CalcCntnt" );
                }
            }
            if ( pFrm->IsTabFrm() )
            {
                if ( static_cast<SwTabFrm*>(pFrm)->IsFollow() )
                    static_cast<SwTabFrm*>(pFrm)->m_bLockBackMove = false;
            }

            pFrm = bPrevInvalid ? pTmpPrev : pFrm->FindNext();
            if( !bPrevInvalid && pFrm && pFrm->IsSctFrm() && pSect )
            {
                // Empty SectionFrms could be present here
                while( pFrm && pFrm->IsSctFrm() && !static_cast<SwSectionFrm*>(pFrm)->GetSection() )
                    pFrm = pFrm->FindNext();

                // If FindNext returns the Follow of the original Area, we want to
                // continue with this content as long as it flows back.
                if( pFrm && pFrm->IsSctFrm() && ( pFrm == pSect->GetFollow() ||
                    static_cast<SwSectionFrm*>(pFrm)->IsAnFollow( pSect ) ) )
                {
                    pFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
                    if( pFrm )
                        pFrm->_InvalidatePos();
                }
            }
          // Stay in the pLay
          // Except for SectionFrms with Follow: the first CntntFrm of the Follow
          // will be formatted, so that it get's a chance to load in the pLay.
          // As long as these Frames are loading in pLay, we continue
        } while ( pFrm &&
                  ( pLay->IsAnLower( pFrm ) ||
                    ( pSect &&
                      ( ( pSect->HasFollow() &&
                          ( pLay->IsAnLower( pLast ) ||
                            ( pLast->IsInSct() &&
                              pLast->FindSctFrm()->IsAnFollow(pSect) ) ) &&
                          pSect->GetFollow()->IsAnLower( pFrm )  ) ||
                        ( pFrm->IsInSct() &&
                          pFrm->FindSctFrm()->IsAnFollow( pSect ) ) ) ) ) );
        if( pSect )
        {
            if( bCollect )
            {
                pLay->GetFmt()->GetDoc()->getIDocumentLayoutAccess().GetLayouter()->InsertEndnotes(pSect);
                pSect->CalcFtnCntnt();
            }
            if( pSect->HasFollow() )
            {
                SwSectionFrm* pNxt = pSect->GetFollow();
                while( pNxt && !pNxt->ContainsCntnt() )
                    pNxt = pNxt->GetFollow();
                if( pNxt )
                    pNxt->CalcFtnCntnt();
            }
            if( bCollect )
            {
                pFrm = pLay->ContainsAny();
                bCollect = false;
                if( pFrm )
                    continue;
            }
        }
        break;
    }
    while( true );
}

// OD 2004-03-23 #i26791#
void SwFlyFrm::MakeObjPos()
{
    if ( !mbValidPos )
    {
        mbValidPos = true;

        // OD 29.10.2003 #113049# - use new class to position object
        GetAnchorFrm()->Calc();
        objectpositioning::SwToLayoutAnchoredObjectPosition
                aObjPositioning( *GetVirtDrawObj() );
        aObjPositioning.CalcPosition();

        // #i58280#
        // update relative position
        SetCurrRelPos( aObjPositioning.GetRelPos() );

        SWRECTFN( GetAnchorFrm() );
        maFrm.Pos( aObjPositioning.GetRelPos() );
        maFrm.Pos() += (GetAnchorFrm()->Frm().*fnRect->fnGetPos)();
        // #i69335#
        InvalidateObjRectWithSpaces();
    }
}

void SwFlyFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{
    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = true;

        // OD 31.07.2003 #110978# - consider vertical layout
        SWRECTFN( this )
        (this->*fnRect->fnSetXMargins)( rAttrs.CalcLeftLine(),
                                        rAttrs.CalcRightLine() );
        (this->*fnRect->fnSetYMargins)( rAttrs.CalcTopLine(),
                                        rAttrs.CalcBottomLine() );
    }
}

void SwFlyFrm::MakeContentPos( const SwBorderAttrs &rAttrs )
{
    if ( !m_bValidContentPos )
    {
        m_bValidContentPos = true;

        const SwTwips nUL = rAttrs.CalcTopLine()  + rAttrs.CalcBottomLine();
        Size aRelSize( CalcRel( GetFmt()->GetFrmSize() ) );

        SWRECTFN( this )
        long nMinHeight = 0;
        if( IsMinHeight() )
            nMinHeight = bVert ? aRelSize.Width() : aRelSize.Height();

        Point aNewContentPos;
        aNewContentPos = Prt().Pos();
        const SdrTextVertAdjust nAdjust = GetFmt()->GetTextVertAdjust().GetValue();

        if( nAdjust != SDRTEXTVERTADJUST_TOP )
        {
            const SwTwips nContentHeight = CalcContentHeight(&rAttrs, nMinHeight, nUL);
            SwTwips nDiff = 0;

            if( nContentHeight != 0)
                nDiff = (Prt().*fnRect->fnGetHeight)() - nContentHeight;

            if( nDiff > 0 )
            {
                if( nAdjust == SDRTEXTVERTADJUST_CENTER )
                {
                    if( bVertL2R )
                        aNewContentPos.setX(aNewContentPos.getX() + nDiff/2);
                    else if( bVert )
                        aNewContentPos.setX(aNewContentPos.getX() - nDiff/2);
                    else
                        aNewContentPos.setY(aNewContentPos.getY() + nDiff/2);
                }
                else if( nAdjust == SDRTEXTVERTADJUST_BOTTOM )
                {
                    if( bVertL2R )
                        aNewContentPos.setX(aNewContentPos.getX() + nDiff);
                    else if( bVert )
                        aNewContentPos.setX(aNewContentPos.getX() - nDiff);
                    else
                        aNewContentPos.setY(aNewContentPos.getY() + nDiff);
                }
            }
        }
        if( aNewContentPos != ContentPos() )
        {
            ContentPos() = aNewContentPos;
            for( SwFrm *pFrm = Lower(); pFrm; pFrm = pFrm->GetNext())
            {
                pFrm->InvalidatePos();
            }
        }
    }
}

void SwFlyFrm::InvalidateContentPos()
{
    m_bValidContentPos = false;
    _Invalidate();
}

SwTwips SwFlyFrm::_Grow( SwTwips nDist, bool bTst )
{
    SWRECTFN( this )
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        SwTwips nSize = (Frm().*fnRect->fnGetHeight)();
        if( nSize > 0 && nDist > ( LONG_MAX - nSize ) )
            nDist = LONG_MAX - nSize;

        if ( nDist <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   // If it's a Column Frame, the Format takes control of the
            // resizing (due to the adjustment).
            if ( !bTst )
            {
                // #i28701# - unlock position of Writer fly frame
                UnlockPosition();
                _InvalidatePos();
                InvalidateSize();
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            _InvalidateSize();
            const bool bOldLock = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
            {
                // #i37068# - no format of position here
                // and prevent move in method <CheckClip(..)>.
                // This is needed to prevent layout loop caused by nested
                // Writer fly frames - inner Writer fly frames format its
                // anchor, which grows/shrinks the outer Writer fly frame.
                // Note: position will be invalidated below.
                mbValidPos = true;
                // #i55416#
                // Suppress format of width for autowidth frame, because the
                // format of the width would call <SwTxtFrm::CalcFitToContent()>
                // for the lower frame, which initiated this grow.
                const bool bOldFormatHeightOnly = bFormatHeightOnly;
                const SwFmtFrmSize& rFrmSz = GetFmt()->GetFrmSize();
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = true;
                }
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( true );
                static_cast<SwFlyFreeFrm*>(this)->SwFlyFreeFrm::MakeAll();
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( false );
                // #i55416#
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll();
            _InvalidateSize();
            InvalidatePos();
            if ( bOldLock )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
                ::Notify( this, FindPageFrm(), aOld );
            return (aNew.*fnRect->fnGetHeight)()-(aOld.*fnRect->fnGetHeight)();
        }
        return nDist;
    }
    return 0L;
}

SwTwips SwFlyFrm::_Shrink( SwTwips nDist, bool bTst )
{
    if( Lower() && !IsColLocked() && !HasFixSize() && !IsNoShrink() )
    {
        SWRECTFN( this )
        SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
        if ( nDist > nHeight )
            nDist = nHeight;

        SwTwips nVal = nDist;
        if ( IsMinHeight() )
        {
            const SwFmtFrmSize& rFmtSize = GetFmt()->GetFrmSize();
            SwTwips nFmtHeight = bVert ? rFmtSize.GetWidth() : rFmtSize.GetHeight();

            nVal = std::min( nDist, nHeight - nFmtHeight );
        }

        if ( nVal <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   // If it's a Column Frame, the Format takes control of the
            // resizing (due to the adjustment).
            if ( !bTst )
            {
                SwRect aOld( GetObjRectWithSpaces() );
                (Frm().*fnRect->fnSetHeight)( nHeight - nVal );
                // #i68520#
                if ( nHeight - nVal != 0 )
                {
                    InvalidateObjRectWithSpaces();
                }
                nHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nHeight - nVal );
                _InvalidatePos();
                InvalidateSize();
                ::Notify( this, FindPageFrm(), aOld );
                NotifyDrawObj();
                if ( GetAnchorFrm()->IsInFly() )
                    AnchorFrm()->FindFlyFrm()->Shrink( nDist, bTst );
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            _InvalidateSize();
            const bool bOldLocked = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
            {
                // #i37068# - no format of position here
                // and prevent move in method <CheckClip(..)>.
                // This is needed to prevent layout loop caused by nested
                // Writer fly frames - inner Writer fly frames format its
                // anchor, which grows/shrinks the outer Writer fly frame.
                // Note: position will be invalidated below.
                mbValidPos = true;
                // #i55416#
                // Suppress format of width for autowidth frame, because the
                // format of the width would call <SwTxtFrm::CalcFitToContent()>
                // for the lower frame, which initiated this shrink.
                const bool bOldFormatHeightOnly = bFormatHeightOnly;
                const SwFmtFrmSize& rFrmSz = GetFmt()->GetFrmSize();
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = true;
                }
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( true );
                static_cast<SwFlyFreeFrm*>(this)->SwFlyFreeFrm::MakeAll();
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( false );
                // #i55416#
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll();
            _InvalidateSize();
            InvalidatePos();
            if ( bOldLocked )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
            {
                ::Notify( this, FindPageFrm(), aOld );
                if ( GetAnchorFrm()->IsInFly() )
                    AnchorFrm()->FindFlyFrm()->Shrink( nDist, bTst );
            }
            return (aOld.*fnRect->fnGetHeight)() -
                   (aNew.*fnRect->fnGetHeight)();
        }
        return nVal;
    }
    return 0L;
}

Size SwFlyFrm::ChgSize( const Size& aNewSize )
{
    // #i53298#
    // If the fly frame anchored at-paragraph or at-character contains an OLE
    // object, assure that the new size fits into the current clipping area
    // of the fly frame
    Size aAdjustedNewSize( aNewSize );
    {
        if ( dynamic_cast<SwFlyAtCntFrm*>(this) &&
             Lower() && dynamic_cast<SwNoTxtFrm*>(Lower()) &&
             static_cast<SwNoTxtFrm*>(Lower())->GetNode()->GetOLENode() )
        {
            SwRect aClipRect;
            ::CalcClipRect( GetVirtDrawObj(), aClipRect, false );
            if ( aAdjustedNewSize.Width() > aClipRect.Width() )
            {
                aAdjustedNewSize.setWidth( aClipRect.Width() );
            }
            if ( aAdjustedNewSize.Height() > aClipRect.Height() )
            {
                aAdjustedNewSize.setWidth( aClipRect.Height() );
            }
        }
    }
    if ( aAdjustedNewSize != Frm().SSize() )
    {
        SwFrmFmt *pFmt = GetFmt();
        SwFmtFrmSize aSz( pFmt->GetFrmSize() );
        aSz.SetWidth( aAdjustedNewSize.Width() );
        aSz.SetHeight( aAdjustedNewSize.Height() );
        // go via the Doc for UNDO
        pFmt->GetDoc()->SetAttr( aSz, *pFmt );
        return aSz.GetSize();
    }
    else
        return Frm().SSize();
}

bool SwFlyFrm::IsLowerOf( const SwLayoutFrm* pUpperFrm ) const
{
    OSL_ENSURE( GetAnchorFrm(), "8-( Fly is lost in Space." );
    const SwFrm* pFrm = GetAnchorFrm();
    do
    {
        if ( pFrm == pUpperFrm )
            return true;
        pFrm = pFrm->IsFlyFrm()
               ? static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm()
               : pFrm->GetUpper();
    } while ( pFrm );
    return false;
}

void SwFlyFrm::Cut()
{
}

void SwFrm::AppendFly( SwFlyFrm *pNew )
{
    if ( !mpDrawObjs )
        mpDrawObjs = new SwSortedObjs();
    mpDrawObjs->Insert( *pNew );
    pNew->ChgAnchorFrm( this );

    // Register at the page
    // If there's none present, register via SwPageFrm::PreparePage
    SwPageFrm* pPage = FindPageFrm();
    if ( pPage != NULL )
    {
        pPage->AppendFlyToPage( pNew );
    }
}

void SwFrm::RemoveFly( SwFlyFrm *pToRemove )
{
    // Deregister from the page
    // Could already have happened, if the page was already destructed
    SwPageFrm *pPage = pToRemove->FindPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
    {
        pPage->RemoveFlyFromPage( pToRemove );
    }
    // #i73201#
    else
    {
        if ( pToRemove->IsAccessibleFrm() &&
             pToRemove->GetFmt() &&
             !pToRemove->IsFlyInCntFrm() )
        {
            SwRootFrm *pRootFrm = getRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
            {
                SwViewShell *pVSh = pRootFrm->GetCurrShell();
                if( pVSh && pVSh->Imp() )
                {
                    pVSh->Imp()->DisposeAccessibleFrm( pToRemove );
                }
            }
        }
    }

    mpDrawObjs->Remove( *pToRemove );
    if ( !mpDrawObjs->size() )
        DELETEZ( mpDrawObjs );

    pToRemove->ChgAnchorFrm( 0 );

    if ( !pToRemove->IsFlyInCntFrm() && GetUpper() && IsInTab() )//MA_FLY_HEIGHT
        GetUpper()->InvalidateSize();
}

void SwFrm::AppendDrawObj( SwAnchoredObject& _rNewObj )
{
    assert(!mpDrawObjs || mpDrawObjs->is_sorted());

    if ( !_rNewObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwFrm::AppendDrawObj(..) - anchored object of unexpected type -> object not appended" );
        return;
    }

    if ( !_rNewObj.GetDrawObj()->ISA(SwDrawVirtObj) &&
         _rNewObj.GetAnchorFrm() && _rNewObj.GetAnchorFrm() != this )
    {
        assert(!mpDrawObjs || mpDrawObjs->is_sorted());
        // perform disconnect from layout, if 'master' drawing object is appended
        // to a new frame.
        static_cast<SwDrawContact*>(::GetUserCall( _rNewObj.GetDrawObj() ))->
                                                DisconnectFromLayout( false );
        assert(!mpDrawObjs || mpDrawObjs->is_sorted());
    }

    if ( _rNewObj.GetAnchorFrm() != this )
    {
        if ( !mpDrawObjs )
            mpDrawObjs = new SwSortedObjs();
        mpDrawObjs->Insert( _rNewObj );
        _rNewObj.ChgAnchorFrm( this );
    }

    // #i113730#
    // Assure the control objects and group objects containing controls are on the control layer
    if ( ::CheckControlLayer( _rNewObj.DrawObj() ) )
    {
        const IDocumentDrawModelAccess* pIDDMA = getIDocumentDrawModelAccess();
        const SdrLayerID aCurrentLayer(_rNewObj.DrawObj()->GetLayer());
        const SdrLayerID aControlLayerID(pIDDMA->GetControlsId());
        const SdrLayerID aInvisibleControlLayerID(pIDDMA->GetInvisibleControlsId());

        if(aCurrentLayer != aControlLayerID && aCurrentLayer != aInvisibleControlLayerID)
        {
            if ( aCurrentLayer == pIDDMA->GetInvisibleHellId() ||
                 aCurrentLayer == pIDDMA->GetInvisibleHeavenId() )
            {
                _rNewObj.DrawObj()->SetLayer(aInvisibleControlLayerID);
            }
            else
            {
                _rNewObj.DrawObj()->SetLayer(aControlLayerID);
            }
            //The layer is part of the key used to sort the obj, so update
            //its position if the layer changed.
            mpDrawObjs->Update(_rNewObj);
        }
    }

    // no direct positioning needed, but invalidate the drawing object position
    _rNewObj.InvalidateObjPos();

    // register at page frame
    SwPageFrm* pPage = FindPageFrm();
    if ( pPage )
    {
        pPage->AppendDrawObjToPage( _rNewObj );
    }

    // Notify accessible layout.
    SwViewShell* pSh = getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->AddAccessibleObj( _rNewObj.GetDrawObj() );
    }

    assert(!mpDrawObjs || mpDrawObjs->is_sorted());
}

void SwFrm::RemoveDrawObj( SwAnchoredObject& _rToRemoveObj )
{
    // Notify accessible layout.
    SwViewShell* pSh = getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->DisposeAccessibleObj( _rToRemoveObj.GetDrawObj() );
    }

    // deregister from page frame
    SwPageFrm* pPage = _rToRemoveObj.GetPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
        pPage->RemoveDrawObjFromPage( _rToRemoveObj );

    mpDrawObjs->Remove( _rToRemoveObj );
    if ( !mpDrawObjs->size() )
        DELETEZ( mpDrawObjs );

    _rToRemoveObj.ChgAnchorFrm( 0 );

    assert(!mpDrawObjs || mpDrawObjs->is_sorted());
}

void SwFrm::InvalidateObjs( const bool _bInvaPosOnly,
                            const bool _bNoInvaOfAsCharAnchoredObjs )
{
    if ( GetDrawObjs() )
    {
        // #i26945# - determine page the frame is on,
        // in order to check, if anchored object is registered at the same
        // page.
        const SwPageFrm* pPageFrm = FindPageFrm();
        // #i28701# - re-factoring
        for ( size_t i = 0; i < GetDrawObjs()->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
            if ( _bNoInvaOfAsCharAnchoredObjs &&
                 (pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                    == FLY_AS_CHAR) )
            {
                continue;
            }
            // #i26945# - no invalidation, if anchored object
            // isn't registered at the same page and instead is registered at
            // the page, where its anchor character text frame is on.
            if ( pAnchoredObj->GetPageFrm() &&
                 pAnchoredObj->GetPageFrm() != pPageFrm )
            {
                SwTxtFrm* pAnchorCharFrm = pAnchoredObj->FindAnchorCharFrm();
                if ( pAnchorCharFrm &&
                     pAnchoredObj->GetPageFrm() == pAnchorCharFrm->FindPageFrm() )
                {
                    continue;
                }
                // #115759# - unlock its position, if anchored
                // object isn't registered at the page, where its anchor
                // character text frame is on, respectively if it has no
                // anchor character text frame.
                else
                {
                    pAnchoredObj->UnlockPosition();
                }
            }
            // #i51474# - reset flag, that anchored object
            // has cleared environment, and unlock its position, if the anchored
            // object is registered at the same page as the anchor frame is on.
            if ( pAnchoredObj->ClearedEnvironment() &&
                 pAnchoredObj->GetPageFrm() &&
                 pAnchoredObj->GetPageFrm() == pPageFrm )
            {
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->SetClearedEnvironment( false );
            }
            // distinguish between writer fly frames and drawing objects
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                pFly->_Invalidate();
                pFly->_InvalidatePos();
                if ( !_bInvaPosOnly )
                    pFly->_InvalidateSize();
            }
            else
            {
                pAnchoredObj->InvalidateObjPos();
            }
        } // end of loop on objects, which are connected to the frame
    }
}

// #i26945# - correct check, if anchored object is a lower
// of the layout frame. E.g., anchor character text frame can be a follow text
// frame.
// #i44016# - add parameter <_bUnlockPosOfObjs> to
// force an unlockposition call for the lower objects.
void SwLayoutFrm::NotifyLowerObjs( const bool _bUnlockPosOfObjs )
{
    // invalidate lower floating screen objects
    SwPageFrm* pPageFrm = FindPageFrm();
    if ( pPageFrm && pPageFrm->GetSortedObjs() )
    {
        SwSortedObjs& rObjs = *(pPageFrm->GetSortedObjs());
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            // #i26945# - check, if anchored object is a lower
            // of the layout frame is changed to check, if its anchor frame
            // is a lower of the layout frame.
            // determine the anchor frame - usually it's the anchor frame,
            // for at-character/as-character anchored objects the anchor character
            // text frame is taken.
            const SwFrm* pAnchorFrm = pObj->GetAnchorFrmContainingAnchPos();
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);

                if ( pFly->Frm().Left() == FAR_AWAY )
                    continue;

                if ( pFly->IsAnLower( this ) )
                    continue;

                // #i26945# - use <pAnchorFrm> to check, if
                // fly frame is lower of layout frame resp. if fly frame is
                // at a different page registered as its anchor frame is on.
                const bool bLow = IsAnLower( pAnchorFrm );
                if ( bLow || pAnchorFrm->FindPageFrm() != pPageFrm )
                {
                    pFly->_Invalidate( pPageFrm );
                    if ( !bLow || pFly->IsFlyAtCntFrm() )
                    {
                        // #i44016#
                        if ( _bUnlockPosOfObjs )
                        {
                            pFly->UnlockPosition();
                        }
                        pFly->_InvalidatePos();
                    }
                    else
                        pFly->_InvalidatePrt();
                }
            }
            else
            {
                OSL_ENSURE( pObj->ISA(SwAnchoredDrawObject),
                        "<SwLayoutFrm::NotifyFlys() - anchored object of unexpected type" );
                // #i26945# - use <pAnchorFrm> to check, if
                // fly frame is lower of layout frame resp. if fly frame is
                // at a different page registered as its anchor frame is on.
                if ( IsAnLower( pAnchorFrm ) ||
                     pAnchorFrm->FindPageFrm() != pPageFrm )
                {
                    // #i44016#
                    if ( _bUnlockPosOfObjs )
                    {
                        pObj->UnlockPosition();
                    }
                    pObj->InvalidateObjPos();
                }
            }
        }
    }
}

void SwFlyFrm::NotifyDrawObj()
{
    SwVirtFlyDrawObj* pObj = GetVirtDrawObj();
    pObj->SetRect();
    pObj->SetRectsDirty();
    pObj->SetChanged();
    pObj->BroadcastObjectChange();
    if ( GetFmt()->GetSurround().IsContour() )
        ClrContourCache( pObj );
}

Size SwFlyFrm::CalcRel( const SwFmtFrmSize &rSz ) const
{
    Size aRet( rSz.GetSize() );

    const SwFrm *pRel = IsFlyLayFrm() ? GetAnchorFrm() : GetAnchorFrm()->GetUpper();
    if( pRel ) // LAYER_IMPL
    {
        long nRelWidth = LONG_MAX, nRelHeight = LONG_MAX;
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        if ( ( pRel->IsBodyFrm() || pRel->IsPageFrm() ) &&
             pSh && pSh->GetViewOptions()->getBrowseMode() &&
             pSh->VisArea().HasArea() )
        {
            nRelWidth  = pSh->GetBrowseWidth();
            nRelHeight = pSh->VisArea().Height();
            Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            long nDiff = nRelWidth - pRel->Prt().Width();
            if ( nDiff > 0 )
                nRelWidth -= nDiff;
            nRelHeight -= 2*aBorder.Height();
            nDiff = nRelHeight - pRel->Prt().Height();
            if ( nDiff > 0 )
                nRelHeight -= nDiff;
        }

        // At the moment only the "== PAGE_FRAME" and "!= PAGE_FRAME" cases are handled.
        // When size is a relative to page size, ignore size of SwBodyFrm.
        if (rSz.GetWidthPercentRelation() != text::RelOrientation::PAGE_FRAME)
            nRelWidth  = std::min( nRelWidth,  pRel->Prt().Width() );
        if (rSz.GetHeightPercentRelation() != text::RelOrientation::PAGE_FRAME)
            nRelHeight = std::min( nRelHeight, pRel->Prt().Height() );
        if( !pRel->IsPageFrm() )
        {
            const SwPageFrm* pPage = FindPageFrm();
            if( pPage )
            {
                if (rSz.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
                    // Ignore margins of pPage.
                    nRelWidth  = std::min( nRelWidth,  pPage->Frm().Width() );
                else
                    nRelWidth  = std::min( nRelWidth,  pPage->Prt().Width() );
                if (rSz.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
                    // Ignore margins of pPage.
                    nRelHeight = std::min( nRelHeight, pPage->Frm().Height() );
                else
                    nRelHeight = std::min( nRelHeight, pPage->Prt().Height() );
            }
        }

        if ( rSz.GetWidthPercent() && rSz.GetWidthPercent() != 0xFF )
            aRet.Width() = nRelWidth * rSz.GetWidthPercent() / 100;
        if ( rSz.GetHeightPercent() && rSz.GetHeightPercent() != 0xFF )
            aRet.Height() = nRelHeight * rSz.GetHeightPercent() / 100;

        if ( rSz.GetWidthPercent() == 0xFF )
        {
            aRet.Width() *= aRet.Height();
            aRet.Width() /= rSz.GetHeight();
        }
        else if ( rSz.GetHeightPercent() == 0xFF )
        {
            aRet.Height() *= aRet.Width();
            aRet.Height() /= rSz.GetWidth();
        }
    }
    return aRet;
}

static SwTwips lcl_CalcAutoWidth( const SwLayoutFrm& rFrm )
{
    SwTwips nRet = 0;
    SwTwips nMin = 0;
    const SwFrm* pFrm = rFrm.Lower();

    // No autowidth defined for columned frames
    if ( !pFrm || pFrm->IsColumnFrm() )
        return nRet;

    while ( pFrm )
    {
        if ( pFrm->IsSctFrm() )
        {
            nMin = lcl_CalcAutoWidth( *static_cast<const SwSectionFrm*>(pFrm) );
        }
        if ( pFrm->IsTxtFrm() )
        {
            nMin = const_cast<SwTxtFrm*>(static_cast<const SwTxtFrm*>(pFrm))->CalcFitToContent();
            const SvxLRSpaceItem &rSpace =
                static_cast<const SwTxtFrm*>(pFrm)->GetTxtNode()->GetSwAttrSet().GetLRSpace();
            if (!static_cast<const SwTxtFrm*>(pFrm)->IsLocked())
                nMin += rSpace.GetRight() + rSpace.GetTxtLeft() + rSpace.GetTxtFirstLineOfst();
        }
        else if ( pFrm->IsTabFrm() )
        {
            const SwFmtFrmSize& rTblFmtSz = static_cast<const SwTabFrm*>(pFrm)->GetTable()->GetFrmFmt()->GetFrmSize();
            if ( USHRT_MAX == rTblFmtSz.GetSize().Width() ||
                 text::HoriOrientation::NONE == static_cast<const SwTabFrm*>(pFrm)->GetFmt()->GetHoriOrient().GetHoriOrient() )
            {
                const SwPageFrm* pPage = rFrm.FindPageFrm();
                // auto width table
                nMin = pFrm->GetUpper()->IsVertical() ?
                    pPage->Prt().Height() :
                    pPage->Prt().Width();
            }
            else
            {
                nMin = rTblFmtSz.GetSize().Width();
            }
        }

        if ( nMin > nRet )
            nRet = nMin;

        pFrm = pFrm->GetNext();
    }

    return nRet;
}

SwTwips SwFlyFrm::CalcAutoWidth() const
{
    return lcl_CalcAutoWidth( *this );
}

/// OD 16.04.2003 #i13147# - If called for paint and the <SwNoTxtFrm> contains
/// a graphic, load of intrinsic graphic has to be avoided.
bool SwFlyFrm::GetContour( tools::PolyPolygon&   rContour,
                           const bool _bForPaint ) const
{
    bool bRet = false;
    if( GetFmt()->GetSurround().IsContour() && Lower() &&
        Lower()->IsNoTxtFrm() )
    {
        SwNoTxtNode *pNd = const_cast<SwNoTxtNode*>(static_cast<const SwNoTxtNode*>(static_cast<const SwCntntFrm*>(Lower())->GetNode()));
        // OD 16.04.2003 #i13147# - determine <GraphicObject> instead of <Graphic>
        // in order to avoid load of graphic, if <SwNoTxtNode> contains a graphic
        // node and method is called for paint.
        const GraphicObject* pGrfObj = NULL;
        bool bGrfObjCreated = false;
        const SwGrfNode* pGrfNd = pNd->GetGrfNode();
        if ( pGrfNd && _bForPaint )
        {
            pGrfObj = &(pGrfNd->GetGrfObj());
        }
        else
        {
            pGrfObj = new GraphicObject( pNd->GetGraphic() );
            bGrfObjCreated = true;
        }
        OSL_ENSURE( pGrfObj, "SwFlyFrm::GetContour() - No Graphic/GraphicObject found at <SwNoTxtNode>." );
        if ( pGrfObj && pGrfObj->GetType() != GRAPHIC_NONE )
        {
            if( !pNd->HasContour() )
            {
                // OD 16.04.2003 #i13147# - no <CreateContour> for a graphic
                // during paint. Thus, return (value of <bRet> should be <false>).
                if ( pGrfNd && _bForPaint )
                {
                    OSL_FAIL( "SwFlyFrm::GetContour() - No Contour found at <SwNoTxtNode> during paint." );
                    return bRet;
                }
                pNd->CreateContour();
            }
            pNd->GetContour( rContour );
            // The Node holds the Polygon matching the original size of the graphic
            // We need to include the scaling here
            SwRect aClip;
            SwRect aOrig;
            Lower()->Calc();
            static_cast<const SwNoTxtFrm*>(Lower())->GetGrfArea( aClip, &aOrig, false );
            // OD 16.04.2003 #i13147# - copy method code <SvxContourDlg::ScaleContour(..)>
            // in order to avoid that graphic has to be loaded for contour scale.
            //SvxContourDlg::ScaleContour( rContour, aGrf, MAP_TWIP, aOrig.SSize() );
            {
                OutputDevice*   pOutDev = Application::GetDefaultDevice();
                const MapMode   aDispMap( MAP_TWIP );
                const MapMode   aGrfMap( pGrfObj->GetPrefMapMode() );
                const Size      aGrfSize( pGrfObj->GetPrefSize() );
                Size            aOrgSize;
                Point           aNewPoint;
                bool            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

                if ( bPixelMap )
                    aOrgSize = pOutDev->PixelToLogic( aGrfSize, aDispMap );
                else
                    aOrgSize = OutputDevice::LogicToLogic( aGrfSize, aGrfMap, aDispMap );

                if ( aOrgSize.Width() && aOrgSize.Height() )
                {
                    double fScaleX = (double) aOrig.Width() / aOrgSize.Width();
                    double fScaleY = (double) aOrig.Height() / aOrgSize.Height();

                    for ( sal_uInt16 j = 0, nPolyCount = rContour.Count(); j < nPolyCount; j++ )
                    {
                        Polygon& rPoly = rContour[ j ];

                        for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
                        {
                            if ( bPixelMap )
                                aNewPoint = pOutDev->PixelToLogic( rPoly[ i ], aDispMap  );
                            else
                                aNewPoint = OutputDevice::LogicToLogic( rPoly[ i ], aGrfMap, aDispMap  );

                            rPoly[ i ] = Point( FRound( aNewPoint.getX() * fScaleX ), FRound( aNewPoint.getY() * fScaleY ) );
                        }
                    }
                }
            }
            // OD 17.04.2003 #i13147# - destroy created <GraphicObject>.
            if ( bGrfObjCreated )
            {
                delete pGrfObj;
            }
            rContour.Move( aOrig.Left(), aOrig.Top() );
            if( !aClip.Width() )
                aClip.Width( 1 );
            if( !aClip.Height() )
                aClip.Height( 1 );
            rContour.Clip( aClip.SVRect() );
            rContour.Optimize(PolyOptimizeFlags::CLOSE);
            bRet = true;
        }
    }
    return bRet;
}

// OD 2004-03-25 #i26791#
const SwVirtFlyDrawObj* SwFlyFrm::GetVirtDrawObj() const
{
    return static_cast<const SwVirtFlyDrawObj*>(GetDrawObj());
}
SwVirtFlyDrawObj* SwFlyFrm::GetVirtDrawObj()
{
    return static_cast<SwVirtFlyDrawObj*>(DrawObj());
}

// OD 2004-03-24 #i26791# - implementation of pure virtual method declared in
// base class <SwAnchoredObject>

void SwFlyFrm::InvalidateObjPos()
{
    InvalidatePos();
    // #i68520#
    InvalidateObjRectWithSpaces();
}

SwFrmFmt& SwFlyFrm::GetFrmFmt()
{
    OSL_ENSURE( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}
const SwFrmFmt& SwFlyFrm::GetFrmFmt() const
{
    OSL_ENSURE( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}

const SwRect SwFlyFrm::GetObjRect() const
{
    return Frm();
}

// #i70122#
// for Writer fly frames the bounding rectangle equals the object rectangles
const SwRect SwFlyFrm::GetObjBoundRect() const
{
    return GetObjRect();
}

// #i68520#
bool SwFlyFrm::_SetObjTop( const SwTwips _nTop )
{
    const bool bChanged( Frm().Pos().getY() != _nTop );

    Frm().Pos().setY(_nTop);

    return bChanged;
}
bool SwFlyFrm::_SetObjLeft( const SwTwips _nLeft )
{
    const bool bChanged( Frm().Pos().getX() != _nLeft );

    Frm().Pos().setX(_nLeft);

    return bChanged;
}

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#
*/
void SwFlyFrm::RegisterAtCorrectPage()
{
    // default behaviour is to do nothing.
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible

    OD 2004-05-11 #i28701#
*/
bool SwFlyFrm::IsFormatPossible() const
{
    return SwAnchoredObject::IsFormatPossible() &&
           !IsLocked() && !IsColLocked();
}

void SwFlyFrm::GetAnchoredObjects( std::list<SwAnchoredObject*>& aList, const SwFmt& rFmt )
{
    SwIterator<SwFlyFrm,SwFmt> aIter( rFmt );
    for( SwFlyFrm* pFlyFrm = aIter.First(); pFlyFrm; pFlyFrm = aIter.Next() )
        aList.push_back( pFlyFrm );
}

const SwFlyFrmFmt * SwFlyFrm::GetFmt() const
{
    return static_cast< const SwFlyFrmFmt * >( GetDep() );
}

SwFlyFrmFmt * SwFlyFrm::GetFmt()
{
    return static_cast< SwFlyFrmFmt * >( GetDep() );
}

void SwFlyFrm::Calc() const
{
    if ( !m_bValidContentPos )
        const_cast<SwFlyFrm*>(this)->PrepareMake();
    else
        SwLayoutFrm::Calc();
}

SwTwips SwFlyFrm::CalcContentHeight(const SwBorderAttrs *pAttrs, const SwTwips nMinHeight, const SwTwips nUL)
{
    SWRECTFN( this )
    SwTwips nHeight = 0;
    if ( Lower() )
    {
        if ( Lower()->IsColumnFrm() )
        {
            FormatWidthCols( *pAttrs, nUL, nMinHeight );
            nHeight = (Lower()->Frm().*fnRect->fnGetHeight)();
        }
        else
        {
            SwFrm *pFrm = Lower();
            while ( pFrm )
            {
                nHeight += (pFrm->Frm().*fnRect->fnGetHeight)();
                if( pFrm->IsTxtFrm() && static_cast<SwTxtFrm*>(pFrm)->IsUndersized() )
                // This TxtFrm would like to be a bit larger
                    nHeight += static_cast<SwTxtFrm*>(pFrm)->GetParHeight()
                            - (pFrm->Prt().*fnRect->fnGetHeight)();
                else if( pFrm->IsSctFrm() && static_cast<SwSectionFrm*>(pFrm)->IsUndersized() )
                    nHeight += static_cast<SwSectionFrm*>(pFrm)->Undersize();
                pFrm = pFrm->GetNext();
            }
        }
        if ( GetDrawObjs() )
        {
            const size_t nCnt = GetDrawObjs()->size();
            SwTwips nTop = (Frm().*fnRect->fnGetTop)();
            SwTwips nBorder = (Frm().*fnRect->fnGetHeight)() -
            (Prt().*fnRect->fnGetHeight)();
            for ( size_t i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    // OD 06.11.2003 #i22305# - consider
                    // only Writer fly frames, which follow the text flow.
                    if ( pFly->IsFlyLayFrm() &&
                        pFly->Frm().Top() != FAR_AWAY &&
                        pFly->GetFmt()->GetFollowTextFlow().GetValue() )
                    {
                        SwTwips nDist = -(pFly->Frm().*fnRect->
                            fnBottomDist)( nTop );
                        if( nDist > nBorder + nHeight )
                            nHeight = nDist - nBorder;
                    }
                }
            }
        }
    }
    return nHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
