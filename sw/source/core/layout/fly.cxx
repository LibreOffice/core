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

static SwTwips lcl_CalcAutoWidth( const SwLayoutFrame& rFrame );

SwFlyFrame::SwFlyFrame( SwFlyFrameFormat *pFormat, SwFrame* pSib, SwFrame *pAnch ) :
    SwLayoutFrame( pFormat, pSib ),
    SwAnchoredObject(), // #i26791#
    m_pPrevLink( nullptr ),
    m_pNextLink( nullptr ),
    m_bInCnt( false ),
    m_bAtCnt( false ),
    m_bLayout( false ),
    m_bAutoPosition( false ),
    m_bValidContentPos( false )
{
    mnFrameType = SwFrameType::Fly;

    m_bInvalid = m_bNotifyBack = true;
    m_bLocked  = m_bMinHeight =
    m_bHeightClipped = m_bWidthClipped = m_bFormatHeightOnly = false;

    // Size setting: Fixed size is always the width
    const SwFormatFrameSize &rFrameSize = pFormat->GetFrameSize();
    const sal_uInt16 nDir =
        static_cast<const SvxFrameDirectionItem&>(pFormat->GetFormatAttr( RES_FRAMEDIR )).GetValue();
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
            const SwViewShell *pSh = getRootFrame() ? getRootFrame()->GetCurrShell() : nullptr;
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

    Frame().Width( rFrameSize.GetWidth() );
    Frame().Height( rFrameSize.GetHeightSizeType() == ATT_VAR_SIZE ? MINFLY : rFrameSize.GetHeight() );

    // Fixed or variable Height?
    if ( rFrameSize.GetHeightSizeType() == ATT_MIN_SIZE )
        m_bMinHeight = true;
    else if ( rFrameSize.GetHeightSizeType() == ATT_FIX_SIZE )
        mbFixSize = true;

    // insert columns, if necessary
    InsertColumns();

    // First the Init, then the Content:
    // This is due to the fact that the Content may have Objects/Frames,
    // which are then registered
    InitDrawObj();

    Chain( pAnch );

    InsertCnt();

    // Put it somewhere outside so that out document is not formatted unnecessarily often
    Frame().Pos().setX(FAR_AWAY);
    Frame().Pos().setY(FAR_AWAY);
}

void SwFlyFrame::Chain( SwFrame* _pAnch )
{
    // Connect to chain neighbours.
    // No problem, if a neighboor doesn't exist - the construction of the
    // neighboor will make the connection
    const SwFormatChain& rChain = GetFormat()->GetChain();
    if ( rChain.GetPrev() || rChain.GetNext() )
    {
        if ( rChain.GetNext() )
        {
            SwFlyFrame* pFollow = FindChainNeighbour( *rChain.GetNext(), _pAnch );
            if ( pFollow )
            {
                OSL_ENSURE( !pFollow->GetPrevLink(), "wrong chain detected" );
                if ( !pFollow->GetPrevLink() )
                    SwFlyFrame::ChainFrames( this, pFollow );
            }
        }
        if ( rChain.GetPrev() )
        {
            SwFlyFrame *pMaster = FindChainNeighbour( *rChain.GetPrev(), _pAnch );
            if ( pMaster )
            {
                OSL_ENSURE( !pMaster->GetNextLink(), "wrong chain detected" );
                if ( !pMaster->GetNextLink() )
                    SwFlyFrame::ChainFrames( pMaster, this );
            }
        }
    }
}

void SwFlyFrame::InsertCnt()
{
    if ( !GetPrevLink() )
    {
        const SwFormatContent& rContent = GetFormat()->GetContent();
        OSL_ENSURE( rContent.GetContentIdx(), ":-( no content prepared." );
        sal_uLong nIndex = rContent.GetContentIdx()->GetIndex();
        // Lower() means SwColumnFrame; the Content then needs to be inserted into the (Column)BodyFrame
        ::InsertCnt_( Lower() ? static_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame*>(Lower())->Lower()) : static_cast<SwLayoutFrame*>(this),
                      GetFormat()->GetDoc(), nIndex );

        // NoText always have a fixed height.
        if ( Lower() && Lower()->IsNoTextFrame() )
        {
            mbFixSize = true;
            m_bMinHeight = false;
        }
    }
}

void SwFlyFrame::InsertColumns()
{
    // #i97379#
    // Check, if column are allowed.
    // Columns are not allowed for fly frames, which represent graphics or embedded objects.
    const SwFormatContent& rContent = GetFormat()->GetContent();
    OSL_ENSURE( rContent.GetContentIdx(), "<SwFlyFrame::InsertColumns()> - no content prepared." );
    SwNodeIndex aFirstContent( *(rContent.GetContentIdx()), 1 );
    if ( aFirstContent.GetNode().IsNoTextNode() )
    {
        return;
    }

    const SwFormatCol &rCol = GetFormat()->GetCol();
    if ( rCol.GetNumCols() > 1 )
    {
        // Start off PrtArea to be as large as Frame, so that we can put in the columns
        // properly. It'll adjust later on.
        Prt().Width( Frame().Width() );
        Prt().Height( Frame().Height() );
        const SwFormatCol aOld; // ChgColumns() also needs an old value passed
        ChgColumns( aOld, rCol );
    }
}

void SwFlyFrame::DestroyImpl()
{
    // Accessible objects for fly frames will be destroyed in this destructor.
    // For frames bound as char or frames that don't have an anchor we have
    // to do that ourselves. For any other frame the call RemoveFly at the
    // anchor will do that.
    if( IsAccessibleFrame() && GetFormat() && (IsFlyInContentFrame() || !GetAnchorFrame()) )
    {
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() )
        {
            SwViewShell *pVSh = pRootFrame->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                // Lowers aren't disposed already, so we have to do a recursive
                // dispose
                pVSh->Imp()->DisposeAccessibleFrame( this, true );
            }
        }
    }

    if( GetFormat() && !GetFormat()->GetDoc()->IsInDtor() )
    {
        // OD 2004-01-19 #110582#
        Unchain();

        // OD 2004-01-19 #110582#
        DeleteCnt();

        if ( GetAnchorFrame() )
            AnchorFrame()->RemoveFly( this );
    }

    FinitDrawObj();

    SwLayoutFrame::DestroyImpl();
}

SwFlyFrame::~SwFlyFrame()
{
}

const IDocumentDrawModelAccess& SwFlyFrame::getIDocumentDrawModelAccess()
{
    return GetFormat()->getIDocumentDrawModelAccess();
}

// OD 2004-01-19 #110582#
void SwFlyFrame::Unchain()
{
    if ( GetPrevLink() )
        UnchainFrames( GetPrevLink(), this );
    if ( GetNextLink() )
        UnchainFrames( this, GetNextLink() );
}

// OD 2004-01-19 #110582#
void SwFlyFrame::DeleteCnt()
{
    SwFrame* pFrame = m_pLower;
    while ( pFrame )
    {
        while ( pFrame->GetDrawObjs() && pFrame->GetDrawObjs()->size() )
        {
            SwAnchoredObject *pAnchoredObj = (*pFrame->GetDrawObjs())[0];
            if ( dynamic_cast<const SwFlyFrame*>( pAnchoredObj) !=  nullptr )
            {
                SwFrame::DestroyFrame(static_cast<SwFlyFrame*>(pAnchoredObj));
            }
            else if ( dynamic_cast<const SwAnchoredDrawObject*>( pAnchoredObj) !=  nullptr )
            {
                // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
                SdrObject* pObj = pAnchoredObj->DrawObj();
                if ( dynamic_cast<const SwDrawVirtObj*>( pObj) !=  nullptr )
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

        pFrame->RemoveFromLayout();
        SwFrame::DestroyFrame(pFrame);
        pFrame = m_pLower;
    }

    InvalidatePage();
}

sal_uInt32 SwFlyFrame::GetOrdNumForNewRef( const SwFlyDrawContact* pContact )
{
    sal_uInt32 nOrdNum( 0L );

    // search for another Writer fly frame registered at same frame format
    SwIterator<SwFlyFrame,SwFormat> aIter( *pContact->GetFormat() );
    const SwFlyFrame* pFlyFrame( nullptr );
    for ( pFlyFrame = aIter.First(); pFlyFrame; pFlyFrame = aIter.Next() )
    {
        if ( pFlyFrame != this )
        {
            break;
        }
    }

    if ( pFlyFrame )
    {
        // another Writer fly frame found. Take its order number
        nOrdNum = pFlyFrame->GetVirtDrawObj()->GetOrdNum();
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

SwVirtFlyDrawObj* SwFlyFrame::CreateNewRef( SwFlyDrawContact *pContact )
{
    SwVirtFlyDrawObj *pDrawObj = new SwVirtFlyDrawObj( *pContact->GetMaster(), this );
    pDrawObj->SetModel( pContact->GetMaster()->GetModel() );
    pDrawObj->SetUserCall( pContact );

    // The Reader creates the Masters and inserts them into the Page in
    // order to transport the z-order.
    // After creating the first Reference the Masters are removed from the
    // List and are not important anymore.
    SdrPage* pPg( nullptr );
    if ( nullptr != ( pPg = pContact->GetMaster()->GetPage() ) )
    {
        const size_t nOrdNum = pContact->GetMaster()->GetOrdNum();
        pPg->ReplaceObject( pDrawObj, nOrdNum );
    }
    // #i27030# - insert new <SwVirtFlyDrawObj> instance
    // into drawing page with correct order number
    else
    {
        pContact->GetFormat()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )->
                        InsertObject( pDrawObj, GetOrdNumForNewRef( pContact ) );
    }
    // #i38889# - assure, that new <SwVirtFlyDrawObj> instance
    // is in a visible layer.
    pContact->MoveObjToVisibleLayer( pDrawObj );
    return pDrawObj;
}

void SwFlyFrame::InitDrawObj()
{
    // Find ContactObject from the Format. If there's already one, we just
    // need to create a new Ref, else we create the Contact now.

    IDocumentDrawModelAccess& rIDDMA = GetFormat()->getIDocumentDrawModelAccess();
    if(!GetFormat()->GetContact())
    {
        // #i52858# - method name changed
        GetFormat()->InitContact(rIDDMA.GetOrCreateDrawModel());
    }
    OSL_ENSURE(GetFormat()->GetContact(), "InitDrawObj failed");
    // OD 2004-03-22 #i26791#
    SetDrawObj( *(CreateNewRef( GetFormat()->GetContact() )) );

    // Set the right Layer
    // OD 2004-01-19 #110582#
    // OD 2004-03-22 #i26791#
    GetVirtDrawObj()->SetLayer(GetFormat()->GetOpaque().GetValue()
        ? rIDDMA.GetHeavenId()
        : rIDDMA.GetHellId());
}

void SwFlyFrame::FinitDrawObj()
{
    if(!GetVirtDrawObj() )
        return;
    SwFormat* pFormat = GetFormat();
    // Deregister from SdrPageViews if the Objects is still selected there.
    if(!pFormat->GetDoc()->IsInDtor())
    {
        SwViewShell* p1St = getRootFrame()->GetCurrShell();
        if(p1St)
        {
            for(SwViewShell& rCurrentShell : p1St->GetRingContainer())
            {   // At the moment the Drawing can do just do an Unmark on everything,
                // as the Object was already removed
                if(rCurrentShell.HasDrawView() )
                    rCurrentShell.Imp()->GetDrawView()->UnmarkAll();
            }
        }
    }
    if(!pFormat)
    {
        GetVirtDrawObj()->SetUserCall(nullptr);
        delete GetVirtDrawObj();
        return;
    }
    bool bOtherFramesAround(false);
    SwFlyDrawContact* pContact(nullptr);
    pFormat->CallSwClientNotify(sw::KillDrawHint(this, bOtherFramesAround, pContact));

    // Take VirtObject to the grave.
    // If the last VirtObject is destroyed, the DrawObject and the DrawContact
    // also need to be destroyed.
    // OD, OS 2004-03-31 #116203# - clear user call of Writer fly frame 'master'
    // <SdrObject> to assure, that a <SwXFrame::dispose()> doesn't delete the
    // Writer fly frame again.
    if(bOtherFramesAround)
        pContact = nullptr;
    if(pContact)
        pContact->GetMaster()->SetUserCall(nullptr);
    GetVirtDrawObj()->SetUserCall(nullptr); // Else calls delete of the ContactObj
    delete GetVirtDrawObj();            // Deregisters itself at the Master
}

void SwFlyFrame::ChainFrames( SwFlyFrame *pMaster, SwFlyFrame *pFollow )
{
    OSL_ENSURE( pMaster && pFollow, "uncomplete chain" );
    OSL_ENSURE( !pMaster->GetNextLink(), "link can not be changed" );
    OSL_ENSURE( !pFollow->GetPrevLink(), "link can not be changed" );

    pMaster->m_pNextLink = pFollow;
    pFollow->m_pPrevLink = pMaster;

    if ( pMaster->ContainsContent() )
    {
        // To get a text flow we need to invalidate
        SwFrame *pInva = pMaster->FindLastLower();
        SwRectFnSet aRectFnSet(pMaster);
        const long nBottom = aRectFnSet.GetPrtBottom(*pMaster);
        while ( pInva )
        {
            if( aRectFnSet.BottomDist( pInva->Frame(), nBottom ) <= 0 )
            {
                pInva->InvalidateSize();
                pInva->Prepare();
                pInva = pInva->FindPrev();
            }
            else
                pInva = nullptr;
        }
    }

    if ( pFollow->ContainsContent() )
    {
        // There's only the content from the Masters left; the content from the Follow
        // does not have any Frames left (should always be exactly one empty TextNode).
        SwFrame *pFrame = pFollow->ContainsContent();
        OSL_ENSURE( !pFrame->IsTabFrame() && !pFrame->FindNext(), "follow in chain contains content" );
        pFrame->Cut();
        SwFrame::DestroyFrame(pFrame);
    }

    // invalidate accessible relation set (accessibility wrapper)
    SwViewShell* pSh = pMaster->getRootFrame()->GetCurrShell();
    if( pSh )
    {
        SwRootFrame* pLayout = pMaster->getRootFrame();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

void SwFlyFrame::UnchainFrames( SwFlyFrame *pMaster, SwFlyFrame *pFollow )
{
    pMaster->m_pNextLink = nullptr;
    pFollow->m_pPrevLink = nullptr;

    if ( pFollow->ContainsContent() )
    {
        // The Master sucks up the content of the Follow
        SwLayoutFrame *pUpper = pMaster;
        if ( pUpper->Lower()->IsColumnFrame() )
        {
            pUpper = static_cast<SwLayoutFrame*>(pUpper->GetLastLower());
            pUpper = static_cast<SwLayoutFrame*>(pUpper->Lower()); // The (Column)BodyFrame
            OSL_ENSURE( pUpper && pUpper->IsColBodyFrame(), "Missing ColumnBody" );
        }
        SwFlyFrame *pFoll = pFollow;
        while ( pFoll )
        {
            SwFrame *pTmp = ::SaveContent( pFoll );
            if ( pTmp )
                ::RestoreContent( pTmp, pUpper, pMaster->FindLastLower() );
            pFoll->SetCompletePaint();
            pFoll->InvalidateSize();
            pFoll = pFoll->GetNextLink();
        }
    }

    // The Follow needs his own content to be served
    const SwFormatContent &rContent = pFollow->GetFormat()->GetContent();
    OSL_ENSURE( rContent.GetContentIdx(), ":-( No content prepared." );
    sal_uLong nIndex = rContent.GetContentIdx()->GetIndex();
    // Lower() means SwColumnFrame: this one contains another SwBodyFrame
    ::InsertCnt_( pFollow->Lower() ? const_cast<SwLayoutFrame*>(static_cast<const SwLayoutFrame*>(static_cast<const SwLayoutFrame*>(pFollow->Lower())->Lower()))
                                   : static_cast<SwLayoutFrame*>(pFollow),
                  pFollow->GetFormat()->GetDoc(), ++nIndex );

    // invalidate accessible relation set (accessibility wrapper)
    SwViewShell* pSh = pMaster->getRootFrame()->GetCurrShell();
    if( pSh )
    {
        SwRootFrame* pLayout = pMaster->getRootFrame();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

SwFlyFrame *SwFlyFrame::FindChainNeighbour( SwFrameFormat &rChain, SwFrame *pAnch )
{
    // We look for the Fly that's in the same Area.
    // Areas can for now only be Head/Footer or Flys.

    if ( !pAnch ) // If an Anchor was passed along, that one counts (ctor!)
        pAnch = AnchorFrame();

    SwLayoutFrame *pLay;
    if ( pAnch->IsInFly() )
        pLay = pAnch->FindFlyFrame();
    else
    {
        // FindFooterOrHeader is not appropriate here, as we may not have a
        // connection to the Anchor yet.
        pLay = pAnch->GetUpper();
        while ( pLay && !(pLay->GetType() & (SwFrameType::Header|SwFrameType::Footer)) )
            pLay = pLay->GetUpper();
    }

    SwIterator<SwFlyFrame,SwFormat> aIter( rChain );
    SwFlyFrame *pFly = aIter.First();
    if ( pLay )
    {
        while ( pFly )
        {
            if ( pFly->GetAnchorFrame() )
            {
                if ( pFly->GetAnchorFrame()->IsInFly() )
                {
                    if ( pFly->AnchorFrame()->FindFlyFrame() == pLay )
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

SwFrame *SwFlyFrame::FindLastLower()
{
    SwFrame *pRet = ContainsAny();
    if ( pRet && pRet->IsInTab() )
        pRet = pRet->FindTabFrame();
    SwFrame *pNxt = pRet;
    while ( pNxt && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNext();
    }
    return pRet;
}

bool SwFlyFrame::FrameSizeChg( const SwFormatFrameSize &rFrameSize )
{
    bool bRet = false;
    SwTwips nDiffHeight = Frame().Height();
    if ( rFrameSize.GetHeightSizeType() == ATT_VAR_SIZE )
        mbFixSize = m_bMinHeight = false;
    else
    {
        if ( rFrameSize.GetHeightSizeType() == ATT_FIX_SIZE )
        {
            mbFixSize = true;
            m_bMinHeight = false;
        }
        else if ( rFrameSize.GetHeightSizeType() == ATT_MIN_SIZE )
        {
            mbFixSize = false;
            m_bMinHeight = true;
        }
        nDiffHeight -= rFrameSize.GetHeight();
    }
    // If the Fly contains columns, we already need to set the Fly
    // and the Columns to the required value or else we run into problems.
    if ( Lower() )
    {
        if ( Lower()->IsColumnFrame() )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            const Size   aOldSz( Prt().SSize() );
            const SwTwips nDiffWidth = Frame().Width() - rFrameSize.GetWidth();
            maFrame.Height( maFrame.Height() - nDiffHeight );
            maFrame.Width ( maFrame.Width()  - nDiffWidth  );
            // #i68520#
            InvalidateObjRectWithSpaces();
            maPrt.Height( maPrt.Height() - nDiffHeight );
            maPrt.Width ( maPrt.Width()  - nDiffWidth  );
            ChgLowersProp( aOldSz );
            ::Notify( this, FindPageFrame(), aOld );
            mbValidPos = false;
            bRet = true;
        }
        else if ( Lower()->IsNoTextFrame() )
        {
            mbFixSize = true;
            m_bMinHeight = false;
        }
    }
    return bRet;
}

void SwFlyFrame::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    SwFrame::SwClientNotify(rMod, rHint);
    if(auto pGetZOrdnerHint = dynamic_cast<const sw::GetZOrderHint*>(&rHint))
    {
        auto pFormat(dynamic_cast<const SwFrameFormat*>(&rMod));
        if(pFormat->Which() == RES_FLYFRMFMT && pFormat->getIDocumentLayoutAccess().GetCurrentViewShell()) // #i11176#
            pGetZOrdnerHint->m_rnZOrder = GetVirtDrawObj()->GetOrdNum();
    }
    else if(auto pConnectedHint = dynamic_cast<const sw::GetObjectConnectedHint*>(&rHint))
    {
        auto pFormat(dynamic_cast<const SwFrameFormat*>(&rMod));
        if(!pConnectedHint->m_risConnected && pFormat->Which() == RES_FLYFRMFMT && (!pConnectedHint->m_pRoot || pConnectedHint->m_pRoot == getRootFrame()))
            pConnectedHint->m_risConnected = true;
    }
};

void SwFlyFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
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
            UpdateAttr_( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrame::Modify( &aOldSet, &aNewSet );
    }
    else
        UpdateAttr_( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        Invalidate_();
        if ( nInvFlags & 0x01 )
        {
            InvalidatePos_();
            // #i68520#
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x02 )
        {
            InvalidateSize_();
            // #i68520#
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x04 )
            InvalidatePrt_();
        if ( nInvFlags & 0x08 )
            SetNotifyBack();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
        if ( ( nInvFlags & 0x40 ) && Lower() && Lower()->IsNoTextFrame() )
            ClrContourCache( GetVirtDrawObj() );
        SwRootFrame *pRoot;
        if ( nInvFlags & 0x20 && nullptr != (pRoot = getRootFrame()) )
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

void SwFlyFrame::UpdateAttr_( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
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
            // The background needs to be messaged and invalidated
            const SwRect aTmp( GetObjRectWithSpaces() );
            NotifyBackground( FindPageFrame(), aTmp, PREP_FLY_ATTR_CHG );

            // By changing the flow of frame-bound Frames, a vertical alignment
            // can be activated/deactivated => MakeFlyPos
            if( FLY_AT_FLY == GetFormat()->GetAnchor().GetAnchorId() )
                rInvFlags |= 0x09;

            // Delete contour in the Node if necessary
            if ( Lower() && Lower()->IsNoTextFrame() &&
                 !GetFormat()->GetSurround().IsContour() )
            {
                SwNoTextNode *pNd = static_cast<SwNoTextNode*>(static_cast<SwContentFrame*>(Lower())->GetNode());
                if ( pNd->HasContour() )
                    pNd->SetContour( nullptr );
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
                    SwRootFrame* pLayout = getRootFrame();
                    if( pLayout && pLayout->IsAnyShellAccessible() )
                    pSh->Imp()->InvalidateAccessibleEditableState( true, this );
                }
            }
            break;
        case RES_COL:
            if (pOld && pNew)
            {
                ChgColumns( *static_cast<const SwFormatCol*>(pOld), *static_cast<const SwFormatCol*>(pNew) );
                const SwFormatFrameSize &rNew = GetFormat()->GetFrameSize();
                if ( FrameSizeChg( rNew ) )
                    NotifyDrawObj();
                rInvFlags |= 0x1A;
            }
            break;

        case RES_FRM_SIZE:
        case RES_FMT_CHG:
        {
            const SwFormatFrameSize &rNew = GetFormat()->GetFrameSize();
            if ( FrameSizeChg( rNew ) )
                NotifyDrawObj();
            rInvFlags |= 0x7F;
            if ( RES_FMT_CHG == nWhich )
            {
                SwRect aNew( GetObjRectWithSpaces() );
                SwRect aOld( maFrame );
                const SvxULSpaceItem &rUL = static_cast<const SwFormatChg*>(pOld)->pChangedFormat->GetULSpace();
                aOld.Top( std::max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                aOld.SSize().Height()+= rUL.GetLower();
                const SvxLRSpaceItem &rLR = static_cast<const SwFormatChg*>(pOld)->pChangedFormat->GetLRSpace();
                aOld.Left  ( std::max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                aOld.SSize().Width() += rLR.GetRight();
                aNew.Union( aOld );
                NotifyBackground( FindPageFrame(), aNew, PREP_CLEAR );

                // Special case:
                // When assigning a template we cannot rely on the old column
                // attribute. As there need to be at least enough for ChgColumns,
                // we need to create a temporary attribute.
                SwFormatCol aCol;
                if ( Lower() && Lower()->IsColumnFrame() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrame *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                ChgColumns( aCol, GetFormat()->GetCol() );
            }

            SwFormatURL aURL( GetFormat()->GetURL() );

            SwFormatFrameSize *pNewFormatFrameSize = nullptr;
            SwFormatChg *pOldFormatChg = nullptr;
            if (nWhich == RES_FRM_SIZE)
                pNewFormatFrameSize = const_cast<SwFormatFrameSize*>(static_cast<const SwFormatFrameSize*>(pNew));
            else
                pOldFormatChg = const_cast<SwFormatChg*>(static_cast<const SwFormatChg*>(pOld));

            if (aURL.GetMap() && (pNewFormatFrameSize || pOldFormatChg))
            {
                const SwFormatFrameSize &rOld = pNewFormatFrameSize ?
                                *pNewFormatFrameSize :
                                pOldFormatChg->pChangedFormat->GetFrameSize();
                //#35091# Can be "times zero", when loading the template
                if ( rOld.GetWidth() && rOld.GetHeight() )
                {

                    Fraction aScaleX( rOld.GetWidth(), rNew.GetWidth() );
                    Fraction aScaleY( rOld.GetHeight(), rOld.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrameFormat *pFormat = GetFormat();
                    pFormat->LockModify();
                    pFormat->SetFormatAttr( aURL );
                    pFormat->UnlockModify();
                }
            }
            const SvxProtectItem &rP = GetFormat()->GetProtect();
            GetVirtDrawObj()->SetMoveProtect( rP.IsPosProtected()    );
            GetVirtDrawObj()->SetResizeProtect( rP.IsSizeProtected() );

            if ( pSh )
                pSh->InvalidateWindows( Frame() );
            const IDocumentDrawModelAccess& rIDDMA = GetFormat()->getIDocumentDrawModelAccess();
            const sal_uInt8 nId = GetFormat()->GetOpaque().GetValue() ?
                             rIDDMA.GetHeavenId() :
                             rIDDMA.GetHellId();
            GetVirtDrawObj()->SetLayer( nId );

            if ( Lower() )
            {
                // Delete contour in the Node if necessary
                if( Lower()->IsNoTextFrame() &&
                     !GetFormat()->GetSurround().IsContour() )
                {
                    SwNoTextNode *pNd = static_cast<SwNoTextNode*>(static_cast<SwContentFrame*>(Lower())->GetNode());
                    if ( pNd->HasContour() )
                        pNd->SetContour( nullptr );
                }
                else if( !Lower()->IsColumnFrame() )
                {
                    SwFrame* pFrame = GetLastLower();
                    if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsUndersized() )
                        pFrame->Prepare( PREP_ADJUST_FRM );
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
                getRootFrame()->InvalidateBrowseWidth();
            SwRect aNew( GetObjRectWithSpaces() );
            SwRect aOld( maFrame );
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
            NotifyBackground( FindPageFrame(), aNew, PREP_CLEAR );
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
                    pSh->InvalidateWindows( Frame() );

                const IDocumentDrawModelAccess& rIDDMA = GetFormat()->getIDocumentDrawModelAccess();
                const sal_uInt8 nId = static_cast<const SvxOpaqueItem*>(pNew)->GetValue() ?
                                    rIDDMA.GetHeavenId() :
                                    rIDDMA.GetHellId();
                GetVirtDrawObj()->SetLayer( nId );
                if( pSh )
                {
                    SwRootFrame* pLayout = getRootFrame();
                    if( pLayout && pLayout->IsAnyShellAccessible() )
                {
                    pSh->Imp()->DisposeAccessibleFrame( this );
                    pSh->Imp()->AddAccessibleFrame( this );
                }
                }
                // #i28701# - perform reorder of object lists
                // at anchor frame and at page frame.
                rInvFlags |= 0x80;
            }
            break;

        case RES_URL:
            // The interface changes the frame size when interacting with text frames,
            // the Map, however, needs to be relative to FrameSize().
            if ( (!Lower() || !Lower()->IsNoTextFrame()) && pNew && pOld &&
                 static_cast<const SwFormatURL*>(pNew)->GetMap() && static_cast<const SwFormatURL*>(pOld)->GetMap() )
            {
                const SwFormatFrameSize &rSz = GetFormat()->GetFrameSize();
                if ( rSz.GetHeight() != Frame().Height() ||
                     rSz.GetWidth()  != Frame().Width() )
                {
                    SwFormatURL aURL( GetFormat()->GetURL() );
                    Fraction aScaleX( Frame().Width(),  rSz.GetWidth() );
                    Fraction aScaleY( Frame().Height(), rSz.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrameFormat *pFormat = GetFormat();
                    pFormat->LockModify();
                    pFormat->SetFormatAttr( aURL );
                    pFormat->UnlockModify();
                }
            }
            // No invalidation necessary
            break;

        case RES_CHAIN:
            if (pNew)
            {
                const SwFormatChain *pChain = static_cast<const SwFormatChain*>(pNew);
                if ( pChain->GetNext() )
                {
                    SwFlyFrame *pFollow = FindChainNeighbour( *pChain->GetNext() );
                    if ( GetNextLink() && pFollow != GetNextLink() )
                        SwFlyFrame::UnchainFrames( this, GetNextLink());
                    if ( pFollow )
                    {
                        if ( pFollow->GetPrevLink() &&
                             pFollow->GetPrevLink() != this )
                            SwFlyFrame::UnchainFrames( pFollow->GetPrevLink(),
                                                     pFollow );
                        if ( !GetNextLink() )
                            SwFlyFrame::ChainFrames( this, pFollow );
                    }
                }
                else if ( GetNextLink() )
                    SwFlyFrame::UnchainFrames( this, GetNextLink() );
                if ( pChain->GetPrev() )
                {
                    SwFlyFrame *pMaster = FindChainNeighbour( *pChain->GetPrev() );
                    if ( GetPrevLink() && pMaster != GetPrevLink() )
                        SwFlyFrame::UnchainFrames( GetPrevLink(), this );
                    if ( pMaster )
                    {
                        if ( pMaster->GetNextLink() &&
                             pMaster->GetNextLink() != this )
                            SwFlyFrame::UnchainFrames( pMaster,
                                                     pMaster->GetNextLink() );
                        if ( !GetPrevLink() )
                            SwFlyFrame::ChainFrames( pMaster, this );
                    }
                }
                else if ( GetPrevLink() )
                    SwFlyFrame::UnchainFrames( GetPrevLink(), this );
            }
            SAL_FALLTHROUGH;
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
            SwLayoutFrame::Modify( pOld, pNew );
    }
}

/// Gets information from the Modify
bool SwFlyFrame::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
        return false;   // There's a FlyFrame, so use it
    return true;        // Continue searching
}

void SwFlyFrame::Invalidate_( SwPageFrame *pPage )
{
    InvalidatePage( pPage );
    m_bNotifyBack = m_bInvalid = true;

    SwFlyFrame *pFrame;
    if ( GetAnchorFrame() && nullptr != (pFrame = AnchorFrame()->FindFlyFrame()) )
    {
        // Very bad case: If the Fly is bound within another Fly which
        // contains columns, the Format should be from that one.
        if ( !pFrame->IsLocked() && !pFrame->IsColLocked() &&
             pFrame->Lower() && pFrame->Lower()->IsColumnFrame() )
            pFrame->InvalidateSize();
    }

    // #i85216#
    // if vertical position is oriented at a layout frame inside a ghost section,
    // assure that the position is invalidated and that the information about
    // the vertical position oriented frame is cleared
    if ( GetVertPosOrientFrame() && GetVertPosOrientFrame()->IsLayoutFrame() )
    {
        const SwSectionFrame* pSectFrame( GetVertPosOrientFrame()->FindSctFrame() );
        if ( pSectFrame && pSectFrame->GetSection() == nullptr )
        {
            InvalidatePos();
            ClearVertPosOrientFrame();
        }
    }
}

/** Change the relative position
 *
 * The position will be Fix automatically and the attribute is changed accordingly.
 */
void SwFlyFrame::ChgRelPos( const Point &rNewPos )
{
    if ( GetCurrRelPos() != rNewPos )
    {
        SwFrameFormat *pFormat = GetFormat();
        const bool bVert = GetAnchorFrame()->IsVertical();
        const SwTwips nNewY = bVert ? rNewPos.X() : rNewPos.Y();
        SwTwips nTmpY = nNewY == LONG_MAX ? 0 : nNewY;
        if( bVert )
            nTmpY = -nTmpY;
        SfxItemSet aSet( pFormat->GetDoc()->GetAttrPool(),
                         RES_VERT_ORIENT, RES_HORI_ORIENT);

        SwFormatVertOrient aVert( pFormat->GetVertOrient() );
        const SwTextFrame *pAutoFrame = nullptr;
        // #i34948# - handle also at-page and at-fly anchored
        // Writer fly frames
        const RndStdIds eAnchorType = GetFrameFormat().GetAnchor().GetAnchorId();
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
        else if ( IsFlyAtContentFrame() || text::VertOrientation::NONE != aVert.GetVertOrient() )
        {
            if( text::RelOrientation::CHAR == aVert.GetRelationOrient() && IsAutoPos() )
            {
                if( LONG_MAX != nNewY )
                {
                    aVert.SetVertOrient( text::VertOrientation::NONE );
                    sal_Int32 nOfs =
                        pFormat->GetAnchor().GetContentAnchor()->nContent.GetIndex();
                    OSL_ENSURE( GetAnchorFrame()->IsTextFrame(), "TextFrame expected" );
                    pAutoFrame = static_cast<const SwTextFrame*>(GetAnchorFrame());
                    while( pAutoFrame->GetFollow() &&
                           pAutoFrame->GetFollow()->GetOfst() <= nOfs )
                    {
                        if( pAutoFrame == GetAnchorFrame() )
                            nTmpY += pAutoFrame->GetRelPos().Y();
                        nTmpY -= pAutoFrame->GetUpper()->Prt().Height();
                        pAutoFrame = pAutoFrame->GetFollow();
                    }
                    nTmpY = static_cast<SwFlyAtContentFrame*>(this)->GetRelCharY(pAutoFrame)-nTmpY;
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
        if ( !IsFlyInContentFrame() )
        {
            const SwTwips nNewX = bVert ? rNewPos.Y() : rNewPos.X();
            SwTwips nTmpX = nNewX == LONG_MAX ? 0 : nNewX;
            SwFormatHoriOrient aHori( pFormat->GetHoriOrient() );
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
            else if ( IsFlyAtContentFrame() || text::HoriOrientation::NONE != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                if( text::RelOrientation::CHAR == aHori.GetRelationOrient() && IsAutoPos() )
                {
                    if( LONG_MAX != nNewX )
                    {
                        if( !pAutoFrame )
                        {
                            sal_Int32 nOfs = pFormat->GetAnchor().GetContentAnchor()
                                          ->nContent.GetIndex();
                            OSL_ENSURE( GetAnchorFrame()->IsTextFrame(), "TextFrame expected");
                            pAutoFrame = static_cast<const SwTextFrame*>(GetAnchorFrame());
                            while( pAutoFrame->GetFollow() &&
                                   pAutoFrame->GetFollow()->GetOfst() <= nOfs )
                                pAutoFrame = pAutoFrame->GetFollow();
                        }
                        nTmpX -= static_cast<SwFlyAtContentFrame*>(this)->GetRelCharX(pAutoFrame);
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
        pFormat->GetDoc()->SetAttr( aSet, *pFormat );
    }
}

/** "Formats" the Frame; Frame and PrtArea.
 *
 * The FixSize is not inserted here.
 */
void SwFlyFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "FlyFrame::Format, pAttrs is 0." );

    ColLock();

    if ( !mbValidSize )
    {
        if ( Frame().Top() == FAR_AWAY && Frame().Left() == FAR_AWAY )
        {
            // Remove safety switch (see SwFrame::CTor)
            Frame().Pos().setX(0);
            Frame().Pos().setY(0);
            // #i68520#
            InvalidateObjRectWithSpaces();
        }

        // Check column width and set it if needed
        if ( Lower() && Lower()->IsColumnFrame() )
            AdjustColumns( nullptr, false );

        mbValidSize = true;

        const SwTwips nUL = pAttrs->CalcTopLine()  + pAttrs->CalcBottomLine();
        const SwTwips nLR = pAttrs->CalcLeftLine() + pAttrs->CalcRightLine();
        const SwFormatFrameSize &rFrameSz = GetFormat()->GetFrameSize();
        Size aRelSize( CalcRel( rFrameSz ) );

        OSL_ENSURE( pAttrs->GetSize().Height() != 0 || rFrameSz.GetHeightPercent(), "FrameAttr height is 0." );
        OSL_ENSURE( pAttrs->GetSize().Width()  != 0 || rFrameSz.GetWidthPercent(), "FrameAttr width is 0." );

        SwRectFnSet aRectFnSet(this);
        if( !HasFixSize() )
        {
            long nMinHeight = 0;
            if( IsMinHeight() )
                nMinHeight = aRectFnSet.IsVert() ? aRelSize.Width() : aRelSize.Height();

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

            aRectFnSet.SetHeight( Prt(), nRemaining );
            nRemaining -= aRectFnSet.GetHeight(Frame());
            aRectFnSet.AddBottom( Frame(), nRemaining + nUL );
            // #i68520#
            if ( nRemaining + nUL != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
            mbValidSize = true;

            if (SwFrameFormat* pShapeFormat = SwTextBoxHelper::getOtherTextBoxFormat(GetFormat(), RES_FLYFRMFMT))
            {
                // This fly is a textbox of a draw shape.
                SdrObject* pShape = pShapeFormat->FindSdrObject();
                if (SdrObjCustomShape* pCustomShape = dynamic_cast<SdrObjCustomShape*>( pShape) )
                {
                    // The shape is a customshape: then inform it about the calculated fly size.
                    Size aSize(aRectFnSet.GetWidth(Frame()), aRectFnSet.GetHeight(Frame()));
                    pCustomShape->SuggestTextFrameSize(aSize);
                    // Do the calculations normally done after touching editeng text of the shape.
                    pCustomShape->NbcSetOutlinerParaObjectForText(nullptr, nullptr);
                }
            }
        }
        else
        {
            mbValidSize = true;  // Fixed Frames do not Format itself
            // Flys set their size using the attr
            SwTwips nNewSize = aRectFnSet.IsVert() ? aRelSize.Width() : aRelSize.Height();
            nNewSize -= nUL;
            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            aRectFnSet.SetHeight( Prt(), nNewSize );
            nNewSize += nUL - aRectFnSet.GetHeight(Frame());
            aRectFnSet.AddBottom( Frame(), nNewSize );
            // #i68520#
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }

        if ( !m_bFormatHeightOnly )
        {
            OSL_ENSURE( aRelSize == CalcRel( rFrameSz ), "SwFlyFrame::Format CalcRel problem" );
            SwTwips nNewSize = aRectFnSet.IsVert() ? aRelSize.Height() : aRelSize.Width();

            if ( rFrameSz.GetWidthSizeType() != ATT_FIX_SIZE )
            {
                // #i9046# Autowidth for fly frames
                const SwTwips nAutoWidth = lcl_CalcAutoWidth( *this );
                if ( nAutoWidth )
                {
                    if( ATT_MIN_SIZE == rFrameSz.GetWidthSizeType() )
                        nNewSize = std::max( nNewSize - nLR, nAutoWidth );
                    else
                        nNewSize = nAutoWidth;
                }
            }
            else
                nNewSize -= nLR;

            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            aRectFnSet.SetWidth( Prt(), nNewSize );
            nNewSize += nLR - aRectFnSet.GetWidth(Frame());
            aRectFnSet.AddRight( Frame(), nNewSize );
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
//                          problems in method <SwContentFrame::WouldFit_(..)>,
//                          which assumes that the follows are formatted.
//                          Thus, <bNoCalcFollow> no longer used by <FormatWidthCols(..)>.
void CalcContent( SwLayoutFrame *pLay, bool bNoColl )
{
    vcl::RenderContext* pRenderContext = pLay->getRootFrame()->GetCurrShell()->GetOut();
    SwSectionFrame* pSect;
    bool bCollect = false;
    if( pLay->IsSctFrame() )
    {
        pSect = static_cast<SwSectionFrame*>(pLay);
        if( pSect->IsEndnAtEnd() && !bNoColl )
        {
            bCollect = true;
            SwLayouter::CollectEndnotes( pLay->GetFormat()->GetDoc(), pSect );
        }
        pSect->CalcFootnoteContent();
    }
    else
        pSect = nullptr;
    SwFrame *pFrame = pLay->ContainsAny();
    if ( !pFrame )
    {
        if( pSect )
        {
            if( pSect->HasFollow() )
                pFrame = pSect->GetFollow()->ContainsAny();
            if( !pFrame )
            {
                if( pSect->IsEndnAtEnd() )
                {
                    if( bCollect )
                        pLay->GetFormat()->GetDoc()->getIDocumentLayoutAccess().GetLayouter()->
                            InsertEndnotes( pSect );
                    bool bLock = pSect->IsFootnoteLock();
                    pSect->SetFootnoteLock( true );
                    pSect->CalcFootnoteContent();
                    pSect->CalcFootnoteContent();
                    pSect->SetFootnoteLock( bLock );
                }
                return;
            }
            pFrame->InvalidatePos_();
        }
        else
            return;
    }
    pFrame->InvalidatePage();

    do
    {
        // local variables to avoid loops caused by anchored object positioning
        SwAnchoredObject* pAgainObj1 = nullptr;
        SwAnchoredObject* pAgainObj2 = nullptr;

        // FME 2007-08-30 #i81146# new loop control
        int nLoopControlRuns = 0;
        const int nLoopControlMax = 20;
        const SwFrame* pLoopControlCond = nullptr;

        SwFrame* pLast;
        do
        {
            pLast = pFrame;
            if( pFrame->IsVertical() ?
                ( pFrame->GetUpper()->Prt().Height() != pFrame->Frame().Height() )
                : ( pFrame->GetUpper()->Prt().Width() != pFrame->Frame().Width() ) )
            {
                pFrame->Prepare( PREP_FIXSIZE_CHG );
                pFrame->InvalidateSize_();
            }

            if ( pFrame->IsTabFrame() )
            {
                static_cast<SwTabFrame*>(pFrame)->m_bCalcLowers = true;
                // OD 26.08.2003 #i18103# - lock move backward of follow table,
                // if no section content is formatted or follow table belongs
                // to the section, which content is formatted.
                if ( static_cast<SwTabFrame*>(pFrame)->IsFollow() &&
                     ( !pSect || pSect == pFrame->FindSctFrame() ) )
                {
                    static_cast<SwTabFrame*>(pFrame)->m_bLockBackMove = true;
                }
            }

            {
                SwFrameDeleteGuard aDeleteGuard(pSect);
                pFrame->Calc(pRenderContext);
            }

            // OD 14.03.2003 #i11760# - reset control flag for follow format.
            if ( pFrame->IsTextFrame() )
            {
                static_cast<SwTextFrame*>(pFrame)->AllowFollowFormat();
            }

            // #111937# The keep-attribute can cause the position
            // of the prev to be invalid:
            // OD 2004-03-15 #116560# - Do not consider invalid previous frame
            // due to its keep-attribute, if current frame is a follow or is locked.
            // #i44049# - do not consider invalid previous
            // frame due to its keep-attribute, if it can't move forward.
            // #i57765# - do not consider invalid previous
            // frame, if current frame has a column/page break before attribute.
            SwFrame* pTmpPrev = pFrame->FindPrev();
            SwFlowFrame* pTmpPrevFlowFrame = pTmpPrev && pTmpPrev->IsFlowFrame() ? SwFlowFrame::CastFlowFrame(pTmpPrev) : nullptr;
            SwFlowFrame* pTmpFlowFrame     = pFrame->IsFlowFrame() ? SwFlowFrame::CastFlowFrame(pFrame) : nullptr;

            bool bPrevInvalid = pTmpPrevFlowFrame && pTmpFlowFrame &&
                               !pTmpFlowFrame->IsFollow() &&
                               !StackHack::IsLocked() && // #i76382#
                               !pTmpFlowFrame->IsJoinLocked() &&
                               !pTmpPrev->GetValidPosFlag() &&
                                pLay->IsAnLower( pTmpPrev ) &&
                                pTmpPrevFlowFrame->IsKeep( *pTmpPrev->GetAttrSet() ) &&
                                pTmpPrevFlowFrame->IsKeepFwdMoveAllowed();

            // format floating screen objects anchored to the frame.
            if ( !bPrevInvalid && pFrame->GetDrawObjs() && pLay->IsAnLower( pFrame ) )
            {
                bool bAgain = false;
                bool bRestartLayoutProcess = false;
                SwPageFrame* pPageFrame = pFrame->FindPageFrame();
                size_t nCnt = pFrame->GetDrawObjs()->size();
                size_t i = 0;
                while ( i < nCnt )
                {
                    // #i28701#
                    SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[i];

                    // determine if anchored object has to be
                    // formatted and, in case, format it
                    if ( !pAnchoredObj->PositionLocked() && pAnchoredObj->IsFormatPossible() )
                    {
                        // #i43737# - no invalidation of
                        // anchored object needed - causes loops for as-character
                        // anchored objects.
                        //pAnchoredObj->InvalidateObjPos();
                        SwRect aRect( pAnchoredObj->GetObjRect() );
                        if ( !SwObjectFormatter::FormatObj( *pAnchoredObj, pFrame, pPageFrame ) )
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
                                OSL_FAIL( "::CalcContent(..) - loop detected, perform attribute changes to avoid the loop" );
                                // Prevent oscillation
                                SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
                                SwFormatSurround aAttr( rFormat.GetSurround() );
                                if( SURROUND_THROUGHT != aAttr.GetSurround() )
                                {
                                    // When on auto position, we can only set it to
                                    // flow through
                                    if ((rFormat.GetAnchor().GetAnchorId() ==
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
                                    rFormat.LockModify();
                                    rFormat.SetFormatAttr( aAttr );
                                    rFormat.UnlockModify();
                                }
                            }
                            else
                            {
                                if ( pAgainObj1 == pAnchoredObj )
                                    pAgainObj2 = pAnchoredObj;
                                pAgainObj1 = pAnchoredObj;
                            }
                        }

                        if ( !pFrame->GetDrawObjs() )
                            break;
                        if ( pFrame->GetDrawObjs()->size() < nCnt )
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
                    pFrame = pLay->ContainsAny();
                    pAgainObj1 = nullptr;
                    pAgainObj2 = nullptr;
                    continue;
                }

                // OD 2004-05-17 #i28701# - format anchor frame after its objects
                // are formatted, if the wrapping style influence has to be considered.
                if ( pLay->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    pFrame->Calc(pRenderContext);
                }

                if ( bAgain )
                {
                    pFrame = pLay->ContainsContent();
                    if ( pFrame && pFrame->IsInTab() )
                        pFrame = pFrame->FindTabFrame();
                    if( pFrame && pFrame->IsInSct() )
                    {
                        SwSectionFrame* pTmp = pFrame->FindSctFrame();
                        if( pTmp != pLay && pLay->IsAnLower( pTmp ) )
                            pFrame = pTmp;
                    }

                    if ( pFrame == pLoopControlCond )
                        ++nLoopControlRuns;
                    else
                    {
                        nLoopControlRuns = 0;
                        pLoopControlCond = pFrame;
                    }

                    if ( nLoopControlRuns < nLoopControlMax )
                        continue;

                    OSL_FAIL( "LoopControl in CalcContent" );
                }
            }
            if ( pFrame->IsTabFrame() )
            {
                if ( static_cast<SwTabFrame*>(pFrame)->IsFollow() )
                    static_cast<SwTabFrame*>(pFrame)->m_bLockBackMove = false;
            }

            pFrame = bPrevInvalid ? pTmpPrev : pFrame->FindNext();
            if( !bPrevInvalid && pFrame && pFrame->IsSctFrame() && pSect )
            {
                // Empty SectionFrames could be present here
                while( pFrame && pFrame->IsSctFrame() && !static_cast<SwSectionFrame*>(pFrame)->GetSection() )
                    pFrame = pFrame->FindNext();

                // If FindNext returns the Follow of the original Area, we want to
                // continue with this content as long as it flows back.
                if( pFrame && pFrame->IsSctFrame() && ( pFrame == pSect->GetFollow() ||
                    static_cast<SwSectionFrame*>(pFrame)->IsAnFollow( pSect ) ) )
                {
                    pFrame = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                    if( pFrame )
                        pFrame->InvalidatePos_();
                }
            }
          // Stay in the pLay
          // Except for SectionFrames with Follow: the first ContentFrame of the Follow
          // will be formatted, so that it get's a chance to load in the pLay.
          // As long as these Frames are loading in pLay, we continue
        } while ( pFrame &&
                  ( pLay->IsAnLower( pFrame ) ||
                    ( pSect &&
                      ( ( pSect->HasFollow() &&
                          ( pLay->IsAnLower( pLast ) ||
                            ( pLast->IsInSct() &&
                              pLast->FindSctFrame()->IsAnFollow(pSect) ) ) &&
                          pSect->GetFollow()->IsAnLower( pFrame )  ) ||
                        ( pFrame->IsInSct() &&
                          pFrame->FindSctFrame()->IsAnFollow( pSect ) ) ) ) ) );
        if( pSect )
        {
            if( bCollect )
            {
                pLay->GetFormat()->GetDoc()->getIDocumentLayoutAccess().GetLayouter()->InsertEndnotes(pSect);
                pSect->CalcFootnoteContent();
            }
            if( pSect->HasFollow() )
            {
                SwSectionFrame* pNxt = pSect->GetFollow();
                while( pNxt && !pNxt->ContainsContent() )
                    pNxt = pNxt->GetFollow();
                if( pNxt )
                    pNxt->CalcFootnoteContent();
            }
            if( bCollect )
            {
                pFrame = pLay->ContainsAny();
                bCollect = false;
                if( pFrame )
                    continue;
            }
        }
        break;
    }
    while( true );
}

// OD 2004-03-23 #i26791#
void SwFlyFrame::MakeObjPos()
{
    if ( !mbValidPos )
    {
        vcl::RenderContext* pRenderContext = getRootFrame()->GetCurrShell()->GetOut();
        mbValidPos = true;

        // OD 29.10.2003 #113049# - use new class to position object
        GetAnchorFrame()->Calc(pRenderContext);
        objectpositioning::SwToLayoutAnchoredObjectPosition
                aObjPositioning( *GetVirtDrawObj() );
        aObjPositioning.CalcPosition();

        // #i58280#
        // update relative position
        SetCurrRelPos( aObjPositioning.GetRelPos() );

        SwRectFnSet aRectFnSet(GetAnchorFrame());
        maFrame.Pos( aObjPositioning.GetRelPos() );
        maFrame.Pos() += aRectFnSet.GetPos(GetAnchorFrame()->Frame());
        // #i69335#
        InvalidateObjRectWithSpaces();
    }
}

void SwFlyFrame::MakePrtArea( const SwBorderAttrs &rAttrs )
{
    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = true;

        // OD 31.07.2003 #110978# - consider vertical layout
        SwRectFnSet aRectFnSet(this);
        aRectFnSet.SetXMargins( *this, rAttrs.CalcLeftLine(),
                                        rAttrs.CalcRightLine() );
        aRectFnSet.SetYMargins( *this, rAttrs.CalcTopLine(),
                                        rAttrs.CalcBottomLine() );
    }
}

void SwFlyFrame::MakeContentPos( const SwBorderAttrs &rAttrs )
{
    if ( !m_bValidContentPos )
    {
        m_bValidContentPos = true;

        const SwTwips nUL = rAttrs.CalcTopLine()  + rAttrs.CalcBottomLine();
        Size aRelSize( CalcRel( GetFormat()->GetFrameSize() ) );

        SwRectFnSet aRectFnSet(this);
        long nMinHeight = 0;
        if( IsMinHeight() )
            nMinHeight = aRectFnSet.IsVert() ? aRelSize.Width() : aRelSize.Height();

        Point aNewContentPos;
        aNewContentPos = Prt().Pos();
        const SdrTextVertAdjust nAdjust = GetFormat()->GetTextVertAdjust().GetValue();

        if( nAdjust != SDRTEXTVERTADJUST_TOP )
        {
            const SwTwips nContentHeight = CalcContentHeight(&rAttrs, nMinHeight, nUL);
            SwTwips nDiff = 0;

            if( nContentHeight != 0)
                nDiff = aRectFnSet.GetHeight(Prt()) - nContentHeight;

            if( nDiff > 0 )
            {
                if( nAdjust == SDRTEXTVERTADJUST_CENTER )
                {
                    if( aRectFnSet.IsVertL2R() )
                        aNewContentPos.setX(aNewContentPos.getX() + nDiff/2);
                    else if( aRectFnSet.IsVert() )
                        aNewContentPos.setX(aNewContentPos.getX() - nDiff/2);
                    else
                        aNewContentPos.setY(aNewContentPos.getY() + nDiff/2);
                }
                else if( nAdjust == SDRTEXTVERTADJUST_BOTTOM )
                {
                    if( aRectFnSet.IsVertL2R() )
                        aNewContentPos.setX(aNewContentPos.getX() + nDiff);
                    else if( aRectFnSet.IsVert() )
                        aNewContentPos.setX(aNewContentPos.getX() - nDiff);
                    else
                        aNewContentPos.setY(aNewContentPos.getY() + nDiff);
                }
            }
        }
        if( aNewContentPos != ContentPos() )
        {
            ContentPos() = aNewContentPos;
            for( SwFrame *pFrame = Lower(); pFrame; pFrame = pFrame->GetNext())
            {
                pFrame->InvalidatePos();
            }
        }
    }
}

void SwFlyFrame::InvalidateContentPos()
{
    m_bValidContentPos = false;
    Invalidate_();
}

SwTwips SwFlyFrame::Grow_( SwTwips nDist, bool bTst )
{
    SwRectFnSet aRectFnSet(this);
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        SwTwips nSize = aRectFnSet.GetHeight(Frame());
        if( nSize > 0 && nDist > ( LONG_MAX - nSize ) )
            nDist = LONG_MAX - nSize;

        if ( nDist <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrame() )
        {   // If it's a Column Frame, the Format takes control of the
            // resizing (due to the adjustment).
            if ( !bTst )
            {
                // #i28701# - unlock position of Writer fly frame
                UnlockPosition();
                InvalidatePos_();
                InvalidateSize();
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            InvalidateSize_();
            const bool bOldLock = m_bLocked;
            Unlock();
            if ( IsFlyFreeFrame() )
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
                // format of the width would call <SwTextFrame::CalcFitToContent()>
                // for the lower frame, which initiated this grow.
                const bool bOldFormatHeightOnly = m_bFormatHeightOnly;
                const SwFormatFrameSize& rFrameSz = GetFormat()->GetFrameSize();
                if ( rFrameSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    m_bFormatHeightOnly = true;
                }
                static_cast<SwFlyFreeFrame*>(this)->SetNoMoveOnCheckClip( true );
                static_cast<SwFlyFreeFrame*>(this)->SwFlyFreeFrame::MakeAll(getRootFrame()->GetCurrShell()->GetOut());
                static_cast<SwFlyFreeFrame*>(this)->SetNoMoveOnCheckClip( false );
                // #i55416#
                if ( rFrameSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    m_bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll(getRootFrame()->GetCurrShell()->GetOut());
            InvalidateSize_();
            InvalidatePos();
            if ( bOldLock )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
                ::Notify( this, FindPageFrame(), aOld );
            return aRectFnSet.GetHeight(aNew)-aRectFnSet.GetHeight(aOld);
        }
        return nDist;
    }
    return 0L;
}

SwTwips SwFlyFrame::Shrink_( SwTwips nDist, bool bTst )
{
    if( Lower() && !IsColLocked() && !HasFixSize() )
    {
        SwRectFnSet aRectFnSet(this);
        SwTwips nHeight = aRectFnSet.GetHeight(Frame());
        if ( nDist > nHeight )
            nDist = nHeight;

        SwTwips nVal = nDist;
        if ( IsMinHeight() )
        {
            const SwFormatFrameSize& rFormatSize = GetFormat()->GetFrameSize();
            SwTwips nFormatHeight = aRectFnSet.IsVert() ? rFormatSize.GetWidth() : rFormatSize.GetHeight();

            nVal = std::min( nDist, nHeight - nFormatHeight );
        }

        if ( nVal <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrame() )
        {   // If it's a Column Frame, the Format takes control of the
            // resizing (due to the adjustment).
            if ( !bTst )
            {
                SwRect aOld( GetObjRectWithSpaces() );
                aRectFnSet.SetHeight( Frame(), nHeight - nVal );
                // #i68520#
                if ( nHeight - nVal != 0 )
                {
                    InvalidateObjRectWithSpaces();
                }
                nHeight = aRectFnSet.GetHeight(Prt());
                aRectFnSet.SetHeight( Prt(), nHeight - nVal );
                InvalidatePos_();
                InvalidateSize();
                ::Notify( this, FindPageFrame(), aOld );
                NotifyDrawObj();
                if ( GetAnchorFrame()->IsInFly() )
                    AnchorFrame()->FindFlyFrame()->Shrink( nDist, bTst );
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            InvalidateSize_();
            const bool bOldLocked = m_bLocked;
            Unlock();
            if ( IsFlyFreeFrame() )
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
                // format of the width would call <SwTextFrame::CalcFitToContent()>
                // for the lower frame, which initiated this shrink.
                const bool bOldFormatHeightOnly = m_bFormatHeightOnly;
                const SwFormatFrameSize& rFrameSz = GetFormat()->GetFrameSize();
                if ( rFrameSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    m_bFormatHeightOnly = true;
                }
                static_cast<SwFlyFreeFrame*>(this)->SetNoMoveOnCheckClip( true );
                static_cast<SwFlyFreeFrame*>(this)->SwFlyFreeFrame::MakeAll(getRootFrame()->GetCurrShell()->GetOut());
                static_cast<SwFlyFreeFrame*>(this)->SetNoMoveOnCheckClip( false );
                // #i55416#
                if ( rFrameSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    m_bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll(getRootFrame()->GetCurrShell()->GetOut());
            InvalidateSize_();
            InvalidatePos();
            if ( bOldLocked )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
            {
                ::Notify( this, FindPageFrame(), aOld );
                if ( GetAnchorFrame()->IsInFly() )
                    AnchorFrame()->FindFlyFrame()->Shrink( nDist, bTst );
            }
            return aRectFnSet.GetHeight(aOld) -
                   aRectFnSet.GetHeight(aNew);
        }
        return nVal;
    }
    return 0L;
}

Size SwFlyFrame::ChgSize( const Size& aNewSize )
{
    // #i53298#
    // If the fly frame anchored at-paragraph or at-character contains an OLE
    // object, assure that the new size fits into the current clipping area
    // of the fly frame
    Size aAdjustedNewSize( aNewSize );
    {
        if ( dynamic_cast<SwFlyAtContentFrame*>(this) &&
             Lower() && dynamic_cast<SwNoTextFrame*>(Lower()) &&
             static_cast<SwNoTextFrame*>(Lower())->GetNode()->GetOLENode() )
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
    if ( aAdjustedNewSize != Frame().SSize() )
    {
        SwFrameFormat *pFormat = GetFormat();
        SwFormatFrameSize aSz( pFormat->GetFrameSize() );
        aSz.SetWidth( aAdjustedNewSize.Width() );
        aSz.SetHeight( aAdjustedNewSize.Height() );
        // go via the Doc for UNDO
        pFormat->GetDoc()->SetAttr( aSz, *pFormat );
        return aSz.GetSize();
    }
    else
        return Frame().SSize();
}

bool SwFlyFrame::IsLowerOf( const SwLayoutFrame* pUpperFrame ) const
{
    OSL_ENSURE( GetAnchorFrame(), "8-( Fly is lost in Space." );
    const SwFrame* pFrame = GetAnchorFrame();
    do
    {
        if ( pFrame == pUpperFrame )
            return true;
        pFrame = pFrame->IsFlyFrame()
               ? static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame()
               : pFrame->GetUpper();
    } while ( pFrame );
    return false;
}

void SwFlyFrame::Cut()
{
}

void SwFrame::AppendFly( SwFlyFrame *pNew )
{
    if ( !mpDrawObjs )
        mpDrawObjs = new SwSortedObjs();
    mpDrawObjs->Insert( *pNew );
    pNew->ChgAnchorFrame( this );

    // Register at the page
    // If there's none present, register via SwPageFrame::PreparePage
    SwPageFrame* pPage = FindPageFrame();
    if ( pPage != nullptr )
    {
        pPage->AppendFlyToPage( pNew );
    }
}

void SwFrame::RemoveFly( SwFlyFrame *pToRemove )
{
    // Deregister from the page
    // Could already have happened, if the page was already destructed
    SwPageFrame *pPage = pToRemove->FindPageFrame();
    if ( pPage && pPage->GetSortedObjs() )
    {
        pPage->RemoveFlyFromPage( pToRemove );
    }
    // #i73201#
    else
    {
        if ( pToRemove->IsAccessibleFrame() &&
             pToRemove->GetFormat() &&
             !pToRemove->IsFlyInContentFrame() )
        {
            SwRootFrame *pRootFrame = getRootFrame();
            if( pRootFrame && pRootFrame->IsAnyShellAccessible() )
            {
                SwViewShell *pVSh = pRootFrame->GetCurrShell();
                if( pVSh && pVSh->Imp() )
                {
                    pVSh->Imp()->DisposeAccessibleFrame( pToRemove );
                }
            }
        }
    }

    mpDrawObjs->Remove( *pToRemove );
    if ( !mpDrawObjs->size() )
        DELETEZ( mpDrawObjs );

    pToRemove->ChgAnchorFrame( nullptr );

    if ( !pToRemove->IsFlyInContentFrame() && GetUpper() && IsInTab() )//MA_FLY_HEIGHT
        GetUpper()->InvalidateSize();
}

void SwFrame::AppendDrawObj( SwAnchoredObject& _rNewObj )
{
    assert(!mpDrawObjs || mpDrawObjs->is_sorted());

    if ( dynamic_cast<const SwAnchoredDrawObject*>( &_rNewObj) ==  nullptr )
    {
        OSL_FAIL( "SwFrame::AppendDrawObj(..) - anchored object of unexpected type -> object not appended" );
        return;
    }

    if ( dynamic_cast<const SwDrawVirtObj*>(_rNewObj.GetDrawObj()) ==  nullptr &&
         _rNewObj.GetAnchorFrame() && _rNewObj.GetAnchorFrame() != this )
    {
        assert(!mpDrawObjs || mpDrawObjs->is_sorted());
        // perform disconnect from layout, if 'master' drawing object is appended
        // to a new frame.
        static_cast<SwDrawContact*>(::GetUserCall( _rNewObj.GetDrawObj() ))->
                                                DisconnectFromLayout( false );
        assert(!mpDrawObjs || mpDrawObjs->is_sorted());
    }

    if ( _rNewObj.GetAnchorFrame() != this )
    {
        if ( !mpDrawObjs )
            mpDrawObjs = new SwSortedObjs();
        mpDrawObjs->Insert( _rNewObj );
        _rNewObj.ChgAnchorFrame( this );
    }

    // #i113730#
    // Assure the control objects and group objects containing controls are on the control layer
    if ( ::CheckControlLayer( _rNewObj.DrawObj() ) )
    {
        const IDocumentDrawModelAccess& rIDDMA = getIDocumentDrawModelAccess();
        const SdrLayerID aCurrentLayer(_rNewObj.DrawObj()->GetLayer());
        const SdrLayerID aControlLayerID(rIDDMA.GetControlsId());
        const SdrLayerID aInvisibleControlLayerID(rIDDMA.GetInvisibleControlsId());

        if(aCurrentLayer != aControlLayerID && aCurrentLayer != aInvisibleControlLayerID)
        {
            if ( aCurrentLayer == rIDDMA.GetInvisibleHellId() ||
                 aCurrentLayer == rIDDMA.GetInvisibleHeavenId() )
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
    SwPageFrame* pPage = FindPageFrame();
    if ( pPage )
    {
        pPage->AppendDrawObjToPage( _rNewObj );
    }

    // Notify accessible layout.
    SwViewShell* pSh = getRootFrame()->GetCurrShell();
    if( pSh )
    {
        SwRootFrame* pLayout = getRootFrame();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        {
            pSh->Imp()->AddAccessibleObj( _rNewObj.GetDrawObj() );
        }
    }

    assert(!mpDrawObjs || mpDrawObjs->is_sorted());
}

void SwFrame::RemoveDrawObj( SwAnchoredObject& _rToRemoveObj )
{
    // Notify accessible layout.
    SwViewShell* pSh = getRootFrame()->GetCurrShell();
    if( pSh )
    {
        SwRootFrame* pLayout = getRootFrame();
        if (pLayout && pLayout->IsAnyShellAccessible())
            pSh->Imp()->DisposeAccessibleObj(_rToRemoveObj.GetDrawObj(), false);
    }

    // deregister from page frame
    SwPageFrame* pPage = _rToRemoveObj.GetPageFrame();
    if ( pPage && pPage->GetSortedObjs() )
        pPage->RemoveDrawObjFromPage( _rToRemoveObj );

    mpDrawObjs->Remove( _rToRemoveObj );
    if ( !mpDrawObjs->size() )
        DELETEZ( mpDrawObjs );

    _rToRemoveObj.ChgAnchorFrame( nullptr );

    assert(!mpDrawObjs || mpDrawObjs->is_sorted());
}

void SwFrame::InvalidateObjs( const bool _bNoInvaOfAsCharAnchoredObjs )
{
    if ( GetDrawObjs() )
    {
        // #i26945# - determine page the frame is on,
        // in order to check, if anchored object is registered at the same
        // page.
        const SwPageFrame* pPageFrame = FindPageFrame();
        // #i28701# - re-factoring
        for (SwAnchoredObject* pAnchoredObj : *GetDrawObjs())
        {
            if ( _bNoInvaOfAsCharAnchoredObjs &&
                 (pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                    == FLY_AS_CHAR) )
            {
                continue;
            }
            // #i26945# - no invalidation, if anchored object
            // isn't registered at the same page and instead is registered at
            // the page, where its anchor character text frame is on.
            if ( pAnchoredObj->GetPageFrame() &&
                 pAnchoredObj->GetPageFrame() != pPageFrame )
            {
                SwTextFrame* pAnchorCharFrame = pAnchoredObj->FindAnchorCharFrame();
                if ( pAnchorCharFrame &&
                     pAnchoredObj->GetPageFrame() == pAnchorCharFrame->FindPageFrame() )
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
                 pAnchoredObj->GetPageFrame() &&
                 pAnchoredObj->GetPageFrame() == pPageFrame )
            {
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->SetClearedEnvironment( false );
            }
            // distinguish between writer fly frames and drawing objects
            if ( dynamic_cast<const SwFlyFrame*>( pAnchoredObj) !=  nullptr )
            {
                SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                pFly->Invalidate_();
                pFly->InvalidatePos_();
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
void SwLayoutFrame::NotifyLowerObjs( const bool _bUnlockPosOfObjs )
{
    // invalidate lower floating screen objects
    SwPageFrame* pPageFrame = FindPageFrame();
    if ( pPageFrame && pPageFrame->GetSortedObjs() )
    {
        SwSortedObjs& rObjs = *(pPageFrame->GetSortedObjs());
        for (SwAnchoredObject* pObj : rObjs)
        {
            // #i26945# - check, if anchored object is a lower
            // of the layout frame is changed to check, if its anchor frame
            // is a lower of the layout frame.
            // determine the anchor frame - usually it's the anchor frame,
            // for at-character/as-character anchored objects the anchor character
            // text frame is taken.
            const SwFrame* pAnchorFrame = pObj->GetAnchorFrameContainingAnchPos();
            if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
            {
                SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pObj);

                if ( pFly->Frame().Left() == FAR_AWAY )
                    continue;

                if ( pFly->IsAnLower( this ) )
                    continue;

                // #i26945# - use <pAnchorFrame> to check, if
                // fly frame is lower of layout frame resp. if fly frame is
                // at a different page registered as its anchor frame is on.
                const bool bLow = IsAnLower( pAnchorFrame );
                if ( bLow || pAnchorFrame->FindPageFrame() != pPageFrame )
                {
                    pFly->Invalidate_( pPageFrame );
                    if ( !bLow || pFly->IsFlyAtContentFrame() )
                    {
                        // #i44016#
                        if ( _bUnlockPosOfObjs )
                        {
                            pFly->UnlockPosition();
                        }
                        pFly->InvalidatePos_();
                    }
                    else
                        pFly->InvalidatePrt_();
                }
            }
            else
            {
                OSL_ENSURE( dynamic_cast<const SwAnchoredDrawObject*>( pObj) !=  nullptr,
                        "<SwLayoutFrame::NotifyFlys() - anchored object of unexpected type" );
                // #i26945# - use <pAnchorFrame> to check, if
                // fly frame is lower of layout frame resp. if fly frame is
                // at a different page registered as its anchor frame is on.
                if ( IsAnLower( pAnchorFrame ) ||
                     pAnchorFrame->FindPageFrame() != pPageFrame )
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

void SwFlyFrame::NotifyDrawObj()
{
    SwVirtFlyDrawObj* pObj = GetVirtDrawObj();
    pObj->SetRect();
    pObj->SetRectsDirty();
    pObj->SetChanged();
    pObj->BroadcastObjectChange();
    if ( GetFormat()->GetSurround().IsContour() )
        ClrContourCache( pObj );
}

Size SwFlyFrame::CalcRel( const SwFormatFrameSize &rSz ) const
{
    Size aRet( rSz.GetSize() );

    const SwFrame *pRel = IsFlyLayFrame() ? GetAnchorFrame() : GetAnchorFrame()->GetUpper();
    if( pRel ) // LAYER_IMPL
    {
        long nRelWidth = LONG_MAX, nRelHeight = LONG_MAX;
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        if ( ( pRel->IsBodyFrame() || pRel->IsPageFrame() ) &&
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
        // When size is a relative to page size, ignore size of SwBodyFrame.
        if (rSz.GetWidthPercentRelation() != text::RelOrientation::PAGE_FRAME)
            nRelWidth  = std::min( nRelWidth,  pRel->Prt().Width() );
        if (rSz.GetHeightPercentRelation() != text::RelOrientation::PAGE_FRAME)
            nRelHeight = std::min( nRelHeight, pRel->Prt().Height() );
        if( !pRel->IsPageFrame() )
        {
            const SwPageFrame* pPage = FindPageFrame();
            if( pPage )
            {
                if (rSz.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
                    // Ignore margins of pPage.
                    nRelWidth  = std::min( nRelWidth,  pPage->Frame().Width() );
                else
                    nRelWidth  = std::min( nRelWidth,  pPage->Prt().Width() );
                if (rSz.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
                    // Ignore margins of pPage.
                    nRelHeight = std::min( nRelHeight, pPage->Frame().Height() );
                else
                    nRelHeight = std::min( nRelHeight, pPage->Prt().Height() );
            }
        }

        if ( rSz.GetWidthPercent() && rSz.GetWidthPercent() != SwFormatFrameSize::SYNCED )
            aRet.Width() = nRelWidth * rSz.GetWidthPercent() / 100;
        if ( rSz.GetHeightPercent() && rSz.GetHeightPercent() != SwFormatFrameSize::SYNCED )
            aRet.Height() = nRelHeight * rSz.GetHeightPercent() / 100;

        if ( rSz.GetWidthPercent() == SwFormatFrameSize::SYNCED )
        {
            aRet.Width() *= aRet.Height();
            aRet.Width() /= rSz.GetHeight();
        }
        else if ( rSz.GetHeightPercent() == SwFormatFrameSize::SYNCED )
        {
            aRet.Height() *= aRet.Width();
            aRet.Height() /= rSz.GetWidth();
        }
    }
    return aRet;
}

static SwTwips lcl_CalcAutoWidth( const SwLayoutFrame& rFrame )
{
    SwTwips nRet = 0;
    SwTwips nMin = 0;
    const SwFrame* pFrame = rFrame.Lower();

    // No autowidth defined for columned frames
    if ( !pFrame || pFrame->IsColumnFrame() )
        return nRet;

    while ( pFrame )
    {
        if ( pFrame->IsSctFrame() )
        {
            nMin = lcl_CalcAutoWidth( *static_cast<const SwSectionFrame*>(pFrame) );
        }
        if ( pFrame->IsTextFrame() )
        {
            nMin = const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pFrame))->CalcFitToContent();
            const SvxLRSpaceItem &rSpace =
                static_cast<const SwTextFrame*>(pFrame)->GetTextNode()->GetSwAttrSet().GetLRSpace();
            if (!static_cast<const SwTextFrame*>(pFrame)->IsLocked())
                nMin += rSpace.GetRight() + rSpace.GetTextLeft() + rSpace.GetTextFirstLineOfst();
        }
        else if ( pFrame->IsTabFrame() )
        {
            const SwFormatFrameSize& rTableFormatSz = static_cast<const SwTabFrame*>(pFrame)->GetTable()->GetFrameFormat()->GetFrameSize();
            if ( USHRT_MAX == rTableFormatSz.GetSize().Width() ||
                 text::HoriOrientation::NONE == static_cast<const SwTabFrame*>(pFrame)->GetFormat()->GetHoriOrient().GetHoriOrient() )
            {
                const SwPageFrame* pPage = rFrame.FindPageFrame();
                // auto width table
                nMin = pFrame->GetUpper()->IsVertical() ?
                    pPage->Prt().Height() :
                    pPage->Prt().Width();
            }
            else
            {
                nMin = rTableFormatSz.GetSize().Width();
            }
        }

        if ( nMin > nRet )
            nRet = nMin;

        pFrame = pFrame->GetNext();
    }

    return nRet;
}

/// OD 16.04.2003 #i13147# - If called for paint and the <SwNoTextFrame> contains
/// a graphic, load of intrinsic graphic has to be avoided.
bool SwFlyFrame::GetContour( tools::PolyPolygon&   rContour,
                           const bool _bForPaint ) const
{
    vcl::RenderContext* pRenderContext = getRootFrame()->GetCurrShell()->GetOut();
    bool bRet = false;
    if( GetFormat()->GetSurround().IsContour() && Lower() &&
        Lower()->IsNoTextFrame() )
    {
        SwNoTextNode *pNd = const_cast<SwNoTextNode*>(static_cast<const SwNoTextNode*>(static_cast<const SwContentFrame*>(Lower())->GetNode()));
        // OD 16.04.2003 #i13147# - determine <GraphicObject> instead of <Graphic>
        // in order to avoid load of graphic, if <SwNoTextNode> contains a graphic
        // node and method is called for paint.
        const GraphicObject* pGrfObj = nullptr;
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
        OSL_ENSURE( pGrfObj, "SwFlyFrame::GetContour() - No Graphic/GraphicObject found at <SwNoTextNode>." );
        if ( pGrfObj && pGrfObj->GetType() != GraphicType::NONE )
        {
            if( !pNd->HasContour() )
            {
                // OD 16.04.2003 #i13147# - no <CreateContour> for a graphic
                // during paint. Thus, return (value of <bRet> should be <false>).
                if ( pGrfNd && _bForPaint )
                {
                    OSL_FAIL( "SwFlyFrame::GetContour() - No Contour found at <SwNoTextNode> during paint." );
                    return bRet;
                }
                pNd->CreateContour();
            }
            pNd->GetContour( rContour );
            // The Node holds the Polygon matching the original size of the graphic
            // We need to include the scaling here
            SwRect aClip;
            SwRect aOrig;
            Lower()->Calc(pRenderContext);
            static_cast<const SwNoTextFrame*>(Lower())->GetGrfArea( aClip, &aOrig );
            // OD 16.04.2003 #i13147# - copy method code <SvxContourDlg::ScaleContour(..)>
            // in order to avoid that graphic has to be loaded for contour scale.
            //SvxContourDlg::ScaleContour( rContour, aGrf, MapUnit::MapTwip, aOrig.SSize() );
            {
                OutputDevice*   pOutDev = Application::GetDefaultDevice();
                const MapMode   aDispMap( MapUnit::MapTwip );
                const MapMode   aGrfMap( pGrfObj->GetPrefMapMode() );
                const Size      aGrfSize( pGrfObj->GetPrefSize() );
                Size            aOrgSize;
                Point           aNewPoint;
                bool            bPixelMap = aGrfMap.GetMapUnit() == MapUnit::MapPixel;

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
                        tools::Polygon& rPoly = rContour[ j ];

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
const SwVirtFlyDrawObj* SwFlyFrame::GetVirtDrawObj() const
{
    return static_cast<const SwVirtFlyDrawObj*>(GetDrawObj());
}
SwVirtFlyDrawObj* SwFlyFrame::GetVirtDrawObj()
{
    return static_cast<SwVirtFlyDrawObj*>(DrawObj());
}

// OD 2004-03-24 #i26791# - implementation of pure virtual method declared in
// base class <SwAnchoredObject>

void SwFlyFrame::InvalidateObjPos()
{
    InvalidatePos();
    // #i68520#
    InvalidateObjRectWithSpaces();
}

SwFrameFormat& SwFlyFrame::GetFrameFormat()
{
    OSL_ENSURE( GetFormat(),
            "<SwFlyFrame::GetFrameFormat()> - missing frame format -> crash." );
    return *GetFormat();
}
const SwFrameFormat& SwFlyFrame::GetFrameFormat() const
{
    OSL_ENSURE( GetFormat(),
            "<SwFlyFrame::GetFrameFormat()> - missing frame format -> crash." );
    return *GetFormat();
}

const SwRect SwFlyFrame::GetObjRect() const
{
    return Frame();
}

// #i70122#
// for Writer fly frames the bounding rectangle equals the object rectangles
const SwRect SwFlyFrame::GetObjBoundRect() const
{
    return GetObjRect();
}

// #i68520#
bool SwFlyFrame::SetObjTop_( const SwTwips _nTop )
{
    const bool bChanged( Frame().Pos().getY() != _nTop );

    Frame().Pos().setY(_nTop);

    return bChanged;
}
bool SwFlyFrame::SetObjLeft_( const SwTwips _nLeft )
{
    const bool bChanged( Frame().Pos().getX() != _nLeft );

    Frame().Pos().setX(_nLeft);

    return bChanged;
}

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#
*/
void SwFlyFrame::RegisterAtCorrectPage()
{
    // default behaviour is to do nothing.
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible

    OD 2004-05-11 #i28701#
*/
bool SwFlyFrame::IsFormatPossible() const
{
    return SwAnchoredObject::IsFormatPossible() &&
           !IsLocked() && !IsColLocked();
}

void SwFlyFrame::GetAnchoredObjects( std::list<SwAnchoredObject*>& aList, const SwFormat& rFormat )
{
    SwIterator<SwFlyFrame,SwFormat> aIter( rFormat );
    for( SwFlyFrame* pFlyFrame = aIter.First(); pFlyFrame; pFlyFrame = aIter.Next() )
        aList.push_back( pFlyFrame );
}

const SwFlyFrameFormat * SwFlyFrame::GetFormat() const
{
    return static_cast< const SwFlyFrameFormat * >( GetDep() );
}

SwFlyFrameFormat * SwFlyFrame::GetFormat()
{
    return static_cast< SwFlyFrameFormat * >( GetDep() );
}

void SwFlyFrame::Calc(vcl::RenderContext* pRenderContext) const
{
    if ( !m_bValidContentPos )
        const_cast<SwFlyFrame*>(this)->PrepareMake(pRenderContext);
    else
        SwLayoutFrame::Calc(pRenderContext);
}

SwTwips SwFlyFrame::CalcContentHeight(const SwBorderAttrs *pAttrs, const SwTwips nMinHeight, const SwTwips nUL)
{
    SwRectFnSet aRectFnSet(this);
    SwTwips nHeight = 0;
    if ( Lower() )
    {
        if ( Lower()->IsColumnFrame() )
        {
            FormatWidthCols( *pAttrs, nUL, nMinHeight );
            nHeight = aRectFnSet.GetHeight(Lower()->Frame());
        }
        else
        {
            SwFrame *pFrame = Lower();
            while ( pFrame )
            {
                nHeight += aRectFnSet.GetHeight(pFrame->Frame());
                if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsUndersized() )
                // This TextFrame would like to be a bit larger
                    nHeight += static_cast<SwTextFrame*>(pFrame)->GetParHeight()
                            - aRectFnSet.GetHeight(pFrame->Prt());
                else if( pFrame->IsSctFrame() && static_cast<SwSectionFrame*>(pFrame)->IsUndersized() )
                    nHeight += static_cast<SwSectionFrame*>(pFrame)->Undersize();
                pFrame = pFrame->GetNext();
            }
        }
        if ( GetDrawObjs() )
        {
            const size_t nCnt = GetDrawObjs()->size();
            SwTwips nTop = aRectFnSet.GetTop(Frame());
            SwTwips nBorder = aRectFnSet.GetHeight(Frame()) -
            aRectFnSet.GetHeight(Prt());
            for ( size_t i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                if ( dynamic_cast<const SwFlyFrame*>( pAnchoredObj) !=  nullptr )
                {
                    SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                    // OD 06.11.2003 #i22305# - consider
                    // only Writer fly frames, which follow the text flow.
                    if ( pFly->IsFlyLayFrame() &&
                        pFly->Frame().Top() != FAR_AWAY &&
                        pFly->GetFormat()->GetFollowTextFlow().GetValue() )
                    {
                        SwTwips nDist = -aRectFnSet.BottomDist( pFly->Frame(), nTop );
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
