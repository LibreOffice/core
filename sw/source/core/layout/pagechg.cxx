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

#include <comphelper/lok.hxx>
#include <ndole.hxx>
#include <svl/itemiter.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtpdsc.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <ftninfo.hxx>
#include <tgrditem.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <docary.hxx>

#include "viewimp.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include "dcontact.hxx"
#include "hints.hxx"
#include <FrameControlsManager.hxx>

#include "ftnidx.hxx"
#include "bodyfrm.hxx"
#include "ftnfrm.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "layact.hxx"
#include "flyfrms.hxx"
#include "htmltbl.hxx"
#include "pagedesc.hxx"
#include <editeng/frmdiritem.hxx>
#include <sortedobjs.hxx>
#include <calbck.hxx>
#include <txtfly.hxx>

using namespace ::com::sun::star;

SwBodyFrame::SwBodyFrame( SwFrameFormat *pFormat, SwFrame* pSib ):
    SwLayoutFrame( pFormat, pSib )
{
    mnFrameType = FRM_BODY;
}

void SwBodyFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    // Formatting of the body is too simple, thus, it gets its own format method.
    // Borders etc. are not taken into account here.
    // Width is taken from the PrtArea of the Upper. Height is the height of the
    // PrtArea of the Upper minus any neighbors (for robustness).
    // The PrtArea has always the size of the frame.

    if ( !mbValidSize )
    {
        SwTwips nHeight = GetUpper()->Prt().Height();
        SwTwips nWidth = GetUpper()->Prt().Width();
        const SwFrame *pFrame = GetUpper()->Lower();
        do
        {
            if ( pFrame != this )
            {
                if( pFrame->IsVertical() )
                    nWidth -= pFrame->Frame().Width();
                else
                    nHeight -= pFrame->Frame().Height();
            }
            pFrame = pFrame->GetNext();
        } while ( pFrame );
        if ( nHeight < 0 )
            nHeight = 0;
        Frame().Height( nHeight );

        if( IsVertical() && !IsVertLR() && !IsReverse() && nWidth != Frame().Width() )
            Frame().Pos().setX(Frame().Pos().getX() + Frame().Width() - nWidth);
        Frame().Width( nWidth );
    }

    bool bNoGrid = true;
    if( GetUpper()->IsPageFrame() && static_cast<SwPageFrame*>(GetUpper())->HasGrid() )
    {
        SwTextGridItem const*const pGrid(
                GetGridItem(static_cast<SwPageFrame*>(GetUpper())));
        if( pGrid )
        {
            bNoGrid = false;
            long nSum = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
            SWRECTFN( this )
            long nSize = (Frame().*fnRect->fnGetWidth)();
            long nBorder = 0;
            if( GRID_LINES_CHARS == pGrid->GetGridType() )
            {
                //for textgrid refactor
                SwDoc *pDoc = GetFormat()->GetDoc();
                nBorder = nSize % (GetGridWidth(*pGrid, *pDoc));
                nSize -= nBorder;
                nBorder /= 2;
            }
            (Prt().*fnRect->fnSetPosX)( nBorder );
            (Prt().*fnRect->fnSetWidth)( nSize );

            // Height of body frame:
            nBorder = (Frame().*fnRect->fnGetHeight)();

            // Number of possible lines in area of body frame:
            long nNumberOfLines = nBorder / nSum;
            if( nNumberOfLines > pGrid->GetLines() )
                nNumberOfLines = pGrid->GetLines();

            // Space required for nNumberOfLines lines:
            nSize = nNumberOfLines * nSum;
            nBorder -= nSize;
            nBorder /= 2;

            // #i21774# Footnotes and centering the grid does not work together:
            const bool bAdjust = static_cast<SwPageFrame*>(GetUpper())->GetFormat()->GetDoc()->
                                        GetFootnoteIdxs().empty();

            (Prt().*fnRect->fnSetPosY)( bAdjust ? nBorder : 0 );
            (Prt().*fnRect->fnSetHeight)( nSize );
        }
    }
    if( bNoGrid )
    {
        Prt().Pos().setX(0);
        Prt().Pos().setY(0);
        Prt().Height( Frame().Height() );
        Prt().Width( Frame().Width() );
    }
    mbValidSize = mbValidPrtArea = true;
}

SwPageFrame::SwPageFrame( SwFrameFormat *pFormat, SwFrame* pSib, SwPageDesc *pPgDsc ) :
    SwFootnoteBossFrame( pFormat, pSib ),
    m_pSortedObjs( nullptr ),
    m_pDesc( pPgDsc ),
    m_nPhyPageNum( 0 )
{
    SetDerivedVert( false );
    SetDerivedR2L( false );
    if( m_pDesc )
    {
        m_bHasGrid = true;
        SwTextGridItem const*const pGrid(GetGridItem(this));
        if( !pGrid )
            m_bHasGrid = false;
    }
    else
        m_bHasGrid = false;
    SetMaxFootnoteHeight( pPgDsc->GetFootnoteInfo().GetHeight() ?
                     pPgDsc->GetFootnoteInfo().GetHeight() : LONG_MAX );
    mnFrameType = FRM_PAGE;
    m_bInvalidLayout = m_bInvalidContent = m_bInvalidSpelling = m_bInvalidSmartTags = m_bInvalidAutoCmplWrds = m_bInvalidWordCount = true;
    m_bInvalidFlyLayout = m_bInvalidFlyContent = m_bInvalidFlyInCnt = m_bFootnotePage = m_bEndNotePage = false;

    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    vcl::RenderContext* pRenderContext = pSh ? pSh->GetOut() : nullptr;
    if ( bBrowseMode )
    {
        Frame().Height( 0 );
        long nWidth = pSh->VisArea().Width();
        if ( !nWidth )
            nWidth = 5000L;     //aendert sich sowieso
        Frame().Width ( nWidth );
    }
    else
        Frame().SSize( pFormat->GetFrameSize().GetSize() );

    // create and insert body area if it is not a blank page
    SwDoc *pDoc = pFormat->GetDoc();
    if ( !(m_bEmptyPage = (pFormat == pDoc->GetEmptyPageFormat())) )
    {
        m_bEmptyPage = false;
        Calc(pRenderContext); // so that the PrtArea is correct
        SwBodyFrame *pBodyFrame = new SwBodyFrame( pDoc->GetDfltFrameFormat(), this );
        pBodyFrame->ChgSize( Prt().SSize() );
        pBodyFrame->Paste( this );
        pBodyFrame->Calc(pRenderContext); // so that the columns can be inserted correctly
        pBodyFrame->InvalidatePos();

        if ( bBrowseMode )
            _InvalidateSize();

        // insert header/footer,, but only if active.
        if ( pFormat->GetHeader().IsActive() )
            PrepareHeader();
        if ( pFormat->GetFooter().IsActive() )
            PrepareFooter();

        const SwFormatCol &rCol = pFormat->GetCol();
        if ( rCol.GetNumCols() > 1 )
        {
            const SwFormatCol aOld; //ChgColumns() needs an old value
            pBodyFrame->ChgColumns( aOld, rCol );
        }
    }
}

void SwPageFrame::DestroyImpl()
{
    // Cleanup the header-footer controls in the SwEditWin
    SwViewShell* pSh = getRootFrame()->GetCurrShell();
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( pSh );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
        rEditWin.GetFrameControlsManager( ).RemoveControls( this );
    }

    // empty FlyContainer, deletion of the Flys is done by the anchor (in base class SwFrame)
    if ( m_pSortedObjs )
    {
        // Objects can be anchored at pages that are before their anchors (why ever...).
        // In such cases, we would access already freed memory.
        for ( size_t i = 0; i < m_pSortedObjs->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*m_pSortedObjs)[i];
            pAnchoredObj->SetPageFrame( nullptr );
        }
        delete m_pSortedObjs;
        m_pSortedObjs = nullptr; // reset to zero to prevent problems when detaching the Flys
    }

    if ( !IsEmptyPage() ) //#59184# unnessesary for empty pages
    {
        // prevent access to destroyed pages
        SwDoc *pDoc = GetFormat() ? GetFormat()->GetDoc() : nullptr;
        if( pDoc && !pDoc->IsInDtor() )
        {
            if ( pSh )
            {
                SwViewShellImp *pImp = pSh->Imp();
                pImp->SetFirstVisPageInvalid();
                if ( pImp->IsAction() )
                    pImp->GetLayAction().SetAgain();
                // #i9719# - retouche area of page
                // including border and shadow area.
                const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
                SwRect aRetoucheRect;
                SwPageFrame::GetBorderAndShadowBoundRect( Frame(), pSh, pSh->GetOut(), aRetoucheRect, IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
                pSh->AddPaintRect( aRetoucheRect );
            }
        }
    }

    SwFootnoteBossFrame::DestroyImpl();
}

SwPageFrame::~SwPageFrame()
{
}

void SwPageFrame::CheckGrid( bool bInvalidate )
{
    bool bOld = m_bHasGrid;
    m_bHasGrid = true;
    SwTextGridItem const*const pGrid(GetGridItem(this));
    m_bHasGrid = nullptr != pGrid;
    if( bInvalidate || bOld != m_bHasGrid )
    {
        SwLayoutFrame* pBody = FindBodyCont();
        if( pBody )
        {
            pBody->InvalidatePrt();
            SwContentFrame* pFrame = pBody->ContainsContent();
            while( pBody->IsAnLower( pFrame ) )
            {
                static_cast<SwTextFrame*>(pFrame)->Prepare();
                pFrame = pFrame->GetNextContentFrame();
            }
        }
        SetCompletePaint();
    }
}

void SwPageFrame::CheckDirection( bool bVert )
{
    sal_uInt16 nDir =
            static_cast<const SvxFrameDirectionItem&>(GetFormat()->GetFormatAttr( RES_FRAMEDIR )).GetValue();
    if( bVert )
    {
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir )
        {
            mbVertLR = false;
            mbVertical = false;
        }
        else
        {
            const SwViewShell *pSh = getRootFrame()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                mbVertLR = false;
                mbVertical = false;
            }
            else
            {
                mbVertical = true;

                if(FRMDIR_VERT_TOP_RIGHT == nDir)
                    mbVertLR = false;
                    else if(FRMDIR_VERT_TOP_LEFT==nDir)
                       mbVertLR = true;
            }
        }

        mbReverse = false;
        mbInvalidVert = false;
    }
    else
    {
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            mbRightToLeft = true;
        else
            mbRightToLeft = false;
        mbInvalidR2L = false;
    }
}

/// create specific Flys for this page and format generic content
static void lcl_FormatLay( SwLayoutFrame *pLay )
{
    vcl::RenderContext* pRenderContext = pLay->getRootFrame()->GetCurrShell()->GetOut();
    // format all LayoutFrames - no tables, Flys etc.

    SwFrame *pTmp = pLay->Lower();
    // first the low-level ones
    while ( pTmp )
    {
        if ( pTmp->GetType() & 0x00FF )
            ::lcl_FormatLay( static_cast<SwLayoutFrame*>(pTmp) );
        pTmp = pTmp->GetNext();
    }
    pLay->Calc(pRenderContext);
}

/// Create Flys or register draw objects
static void lcl_MakeObjs( const SwFrameFormats &rTable, SwPageFrame *pPage )
{
    // formats are in the special table of the document

    for ( size_t i = 0; i < rTable.size(); ++i )
    {
        SwFrameFormat *pFormat = rTable[i];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetPageNum() == pPage->GetPhyPageNum() )
        {
            if( rAnch.GetContentAnchor() )
            {
                if (FLY_AT_PAGE == rAnch.GetAnchorId())
                {
                    SwFormatAnchor aAnch( rAnch );
                    aAnch.SetAnchor( nullptr );
                    pFormat->SetFormatAttr( aAnch );
                }
                else
                    continue;
            }

            // is it a border or a SdrObject?
            bool bSdrObj = RES_DRAWFRMFMT == pFormat->Which();
            SdrObject *pSdrObj = nullptr;
            if ( bSdrObj  && nullptr == (pSdrObj = pFormat->FindSdrObject()) )
            {
                OSL_FAIL( "DrawObject not found." );
                pFormat->GetDoc()->DelFrameFormat( pFormat );
                --i;
                continue;
            }
            // The object might be anchored to another page, e.g. when inserting
            // a new page due to a page descriptor change. In such cases, the
            // object needs to be moved.
            // In some cases the object is already anchored to the correct page.
            // This will be handled here and does not need to be coded extra.
            SwPageFrame *pPg = pPage->IsEmptyPage() ? static_cast<SwPageFrame*>(pPage->GetNext()) : pPage;
            if ( bSdrObj )
            {
                // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
                SwDrawContact *pContact =
                            static_cast<SwDrawContact*>(::GetUserCall(pSdrObj));
                if ( dynamic_cast< const SwDrawVirtObj *>( pSdrObj ) !=  nullptr )
                {
                    SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pSdrObj);
                    if ( pContact )
                    {
                        pDrawVirtObj->RemoveFromWriterLayout();
                        pDrawVirtObj->RemoveFromDrawingPage();
                        pPg->AppendDrawObj( *(pContact->GetAnchoredObj( pDrawVirtObj )) );
                    }
                }
                else
                {
                    if ( pContact->GetAnchorFrame() )
                        pContact->DisconnectFromLayout( false );
                    pPg->AppendDrawObj( *(pContact->GetAnchoredObj( pSdrObj )) );
                }
            }
            else
            {
                SwIterator<SwFlyFrame,SwFormat> aIter( *pFormat );
                SwFlyFrame *pFly = aIter.First();
                if ( pFly)
                {
                    if( pFly->GetAnchorFrame() )
                        pFly->AnchorFrame()->RemoveFly( pFly );
                }
                else
                    pFly = new SwFlyLayFrame( static_cast<SwFlyFrameFormat*>(pFormat), pPg, pPg );
                pPg->AppendFly( pFly );
                ::RegistFlys( pPg, pFly );
            }
        }
    }
}

void SwPageFrame::PreparePage( bool bFootnote )
{
    SetFootnotePage( bFootnote );

    // #i82258#
    // Due to made change on OOo 2.0 code line, method <::lcl_FormatLay(..)> has
    // the side effect, that the content of page header and footer are formatted.
    // For this formatting it is needed that the anchored objects are registered
    // at the <SwPageFrame> instance.
    // Thus, first calling <::RegistFlys(..)>, then call <::lcl_FormatLay(..)>
    ::RegistFlys( this, this );

    if ( Lower() )
    {
                ::lcl_FormatLay( this );
    }

    // Flys and draw objects that are still attached to the document.
    // Footnote pages do not have page-bound Flys!
    // There might be Flys or draw objects that want to be placed on
    // empty pages, however, the empty pages ignore that and the following
    // pages take care of them.
    if ( !bFootnote && !IsEmptyPage() )
    {
        SwDoc *pDoc = GetFormat()->GetDoc();

        if ( GetPrev() && static_cast<SwPageFrame*>(GetPrev())->IsEmptyPage() )
            lcl_MakeObjs( *pDoc->GetSpzFrameFormats(), static_cast<SwPageFrame*>(GetPrev()) );
        lcl_MakeObjs( *pDoc->GetSpzFrameFormats(), this );

        // format footer/ header
        SwLayoutFrame *pLow = static_cast<SwLayoutFrame*>(Lower());
        while ( pLow )
        {
            if ( pLow->GetType() & (FRM_HEADER|FRM_FOOTER) )
            {
                SwContentFrame *pContent = pLow->ContainsContent();
                while ( pContent && pLow->IsAnLower( pContent ) )
                {
                    pContent->OptCalc();  // not the predecessors
                    pContent = pContent->GetNextContentFrame();
                }
            }
            pLow = static_cast<SwLayoutFrame*>(pLow->GetNext());
        }
    }
}

void SwPageFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
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
            SwLayoutFrame::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        InvalidatePage( this );
        if ( nInvFlags & 0x01 )
            _InvalidatePrt();
        if ( nInvFlags & 0x02 )
            SetCompletePaint();
        if ( nInvFlags & 0x04 && GetNext() )
            GetNext()->InvalidatePos();
        if ( nInvFlags & 0x08 )
            PrepareHeader();
        if ( nInvFlags & 0x10 )
            PrepareFooter();
        if ( nInvFlags & 0x20 )
            CheckGrid( nInvFlags & 0x40 );
    }
}


void SwPageFrame::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(typeid(sw::PageFootnoteHint) == typeid(rHint))
    {
        // currently the savest way:
        static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
        SetMaxFootnoteHeight(m_pDesc->GetFootnoteInfo().GetHeight());
        if(!GetMaxFootnoteHeight())
            SetMaxFootnoteHeight(LONG_MAX);
        SetColMaxFootnoteHeight();
        // here, the page might be destroyed:
        static_cast<SwRootFrame*>(GetUpper())->RemoveFootnotes(nullptr, false, true);
    }
    else
        SwClient::SwClientNotify(rModify, rHint);
}

void SwPageFrame::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                             sal_uInt8 &rInvFlags,
                             SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_FMT_CHG:
        {
            // If the frame format is changed, several things might also change:
            // 1. columns:
            assert(pOld && pNew); //FMT_CHG Missing Format
            const SwFormat *const pOldFormat = static_cast<const SwFormatChg*>(pOld)->pChangedFormat;
            const SwFormat *const pNewFormat = static_cast<const SwFormatChg*>(pNew)->pChangedFormat;
            assert(pOldFormat && pNewFormat); //FMT_CHG Missing Format
            const SwFormatCol &rOldCol = pOldFormat->GetCol();
            const SwFormatCol &rNewCol = pNewFormat->GetCol();
            if( rOldCol != rNewCol )
            {
                SwLayoutFrame *pB = FindBodyCont();
                assert(pB && "Page without Body.");
                pB->ChgColumns( rOldCol, rNewCol );
                rInvFlags |= 0x20;
            }

            // 2. header and footer:
            const SwFormatHeader &rOldH = pOldFormat->GetHeader();
            const SwFormatHeader &rNewH = pNewFormat->GetHeader();
            if( rOldH != rNewH )
                rInvFlags |= 0x08;

            const SwFormatFooter &rOldF = pOldFormat->GetFooter();
            const SwFormatFooter &rNewF = pNewFormat->GetFooter();
            if( rOldF != rNewF )
                rInvFlags |= 0x10;
            CheckDirChange();
        }
        // no break
        case RES_FRM_SIZE:
        {
            const SwRect aOldPageFrameRect( Frame() );
            SwViewShell *pSh = getRootFrame()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                mbValidSize = false;
                // OD 28.10.2002 #97265# - Don't call <SwPageFrame::MakeAll()>
                // Calculation of the page is not necessary, because its size is
                // invalidated here and further invalidation is done in the
                // calling method <SwPageFrame::Modify(..)> and probably by calling
                // <SwLayoutFrame::Modify(..)> at the end.
                // It can also causes inconsistences, because the lowers are
                // adjusted, but not calculated, and a <SwPageFrame::MakeAll()> of
                // a next page is called. This is performed on the switch to the
                // online layout.
                //MakeAll();
            }
            else if (pNew)
            {
                const SwFormatFrameSize &rSz = nWhich == RES_FMT_CHG ?
                        static_cast<const SwFormatChg*>(pNew)->pChangedFormat->GetFrameSize() :
                        static_cast<const SwFormatFrameSize&>(*pNew);

                Frame().Height( std::max( rSz.GetHeight(), long(MINLAY) ) );
                Frame().Width ( std::max( rSz.GetWidth(),  long(MINLAY) ) );

                if ( GetUpper() )
                    static_cast<SwRootFrame*>(GetUpper())->CheckViewLayout( nullptr, nullptr );
            }
            // cleanup Window
            if( pSh && pSh->GetWin() && aOldPageFrameRect.HasArea() )
            {
                // #i9719# - consider border and shadow of
                // page frame for determine 'old' rectangle - it's used for invalidating.
                const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
                SwRect aOldRectWithBorderAndShadow;
                SwPageFrame::GetBorderAndShadowBoundRect( aOldPageFrameRect, pSh, pSh->GetOut(), aOldRectWithBorderAndShadow,
                    IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
                pSh->InvalidateWindows( aOldRectWithBorderAndShadow );
            }
            rInvFlags |= 0x03;
            if ( aOldPageFrameRect.Height() != Frame().Height() )
                rInvFlags |= 0x04;
        }
        break;

        case RES_COL:
            assert(pOld && pNew); //COL Missing Format
            if (pOld && pNew)
            {
                SwLayoutFrame *pB = FindBodyCont();
                assert(pB); //page without body
                pB->ChgColumns( *static_cast<const SwFormatCol*>(pOld), *static_cast<const SwFormatCol*>(pNew) );
                rInvFlags |= 0x22;
            }
        break;

        case RES_HEADER:
            rInvFlags |= 0x08;
            break;

        case RES_FOOTER:
            rInvFlags |= 0x10;
            break;
        case RES_TEXTGRID:
            rInvFlags |= 0x60;
            break;
        case RES_FRAMEDIR :
            CheckDirChange();
            break;

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

/// get information from Modify
bool SwPageFrame::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
    {
        // a page frame exists, so use this one
        return false;
    }
    return true; // continue searching
}

void  SwPageFrame::SetPageDesc( SwPageDesc *pNew, SwFrameFormat *pFormat )
{
    m_pDesc = pNew;
    if ( pFormat )
        SetFrameFormat( pFormat );
}

/* determine the right PageDesc:
 *  0.  from the document for footnote and endnote pages
 *  1.  from the first BodyContent below a page
 *  2.  from PageDesc of the predecessor page
 *  3.  from PageDesc of the previous page if blank page
 *  3.1 from PageDesc of the next page if no predecessor exists
 *  4.  default PageDesc
 *  5.  In BrowseMode use the first paragraph or default PageDesc.
 */
SwPageDesc *SwPageFrame::FindPageDesc()
{
    // 0.
    if ( IsFootnotePage() )
    {
        SwDoc *pDoc = GetFormat()->GetDoc();
        if ( IsEndNotePage() )
            return pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
        else
            return pDoc->GetFootnoteInfo().GetPageDesc( *pDoc );
    }

    SwPageDesc *pRet = nullptr;

    //5.
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
    {
        SwContentFrame *pFrame = GetUpper()->ContainsContent();
        while (pFrame && !pFrame->IsInDocBody())
            pFrame = pFrame->GetNextContentFrame();
        if (pFrame)
        {
            SwFrame *pFlow = pFrame;
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrame();
            pRet = const_cast<SwPageDesc*>(pFlow->GetAttrSet()->GetPageDesc().GetPageDesc());
        }
        if ( !pRet )
            pRet = &GetFormat()->GetDoc()->GetPageDesc( 0 );
        return pRet;
    }

    SwFrame *pFlow = FindFirstBodyContent();
    if ( pFlow && pFlow->IsInTab() )
        pFlow = pFlow->FindTabFrame();

    //1.
    if ( pFlow )
    {
        SwFlowFrame *pTmp = SwFlowFrame::CastFlowFrame( pFlow );
        if ( !pTmp->IsFollow() )
            pRet = const_cast<SwPageDesc*>(pFlow->GetAttrSet()->GetPageDesc().GetPageDesc());
    }

    //3. und 3.1
    if ( !pRet && IsEmptyPage() )
            // FME 2008-03-03 #i81544# lijian/fme: an empty page should have
            // the same page description as its prev, just like after construction
            // of the empty page.
        pRet = GetPrev() ? static_cast<SwPageFrame*>(GetPrev())->GetPageDesc() :
               GetNext() ? static_cast<SwPageFrame*>(GetNext())->GetPageDesc() : nullptr;

    //2.
    if ( !pRet )
        pRet = GetPrev() ?
                    static_cast<SwPageFrame*>(GetPrev())->GetPageDesc()->GetFollow() : nullptr;

    //4.
    if ( !pRet )
        pRet = &GetFormat()->GetDoc()->GetPageDesc( 0 );

    OSL_ENSURE( pRet, "could not find page descriptor." );
    return pRet;
}

// Notify if the RootFrame changes its size
void AdjustSizeChgNotify( SwRootFrame *pRoot )
{
    const bool bOld = pRoot->IsSuperfluous();
    pRoot->mbCheckSuperfluous = false;
    if ( pRoot->GetCurrShell() )
    {
        for(SwViewShell& rSh : pRoot->GetCurrShell()->GetRingContainer())
        {
            if( pRoot == rSh.GetLayout() )
            {
                rSh.SizeChgNotify();
                if ( rSh.Imp() )
                    rSh.Imp()->NotifySizeChg( pRoot->Frame().SSize() );
            }
        }
    }
    pRoot->mbCheckSuperfluous = bOld;
}

inline void SetLastPage( SwPageFrame *pPage )
{
    static_cast<SwRootFrame*>(pPage->GetUpper())->mpLastPage = pPage;
}

void SwPageFrame::Cut()
{
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( !IsEmptyPage() )
    {
        if ( GetNext() )
            GetNext()->InvalidatePos();

        // move Flys whose anchor is on a different page (draw objects are not relevant here)
        if ( GetSortedObjs() )
        {
            size_t i = 0;
            while ( GetSortedObjs() && i < GetSortedObjs()->size() )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];

                if ( dynamic_cast< const SwFlyAtContentFrame *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrame* pFly = static_cast<SwFlyAtContentFrame*>(pAnchoredObj);
                    SwPageFrame *pAnchPage = pFly->GetAnchorFrame() ?
                                pFly->AnchorFrame()->FindPageFrame() : nullptr;
                    if ( pAnchPage && (pAnchPage != this) )
                    {
                        MoveFly( pFly, pAnchPage );
                        pFly->InvalidateSize();
                        pFly->_InvalidatePos();
                        // Do not increment index, in this case
                        continue;
                    }
                }
                ++i;
            }
        }
        // cleanup Window
        if ( pSh && pSh->GetWin() )
            pSh->InvalidateWindows( Frame() );
    }

    // decrease the root's page number
    static_cast<SwRootFrame*>(GetUpper())->DecrPhyPageNums();
    SwPageFrame *pPg = static_cast<SwPageFrame*>(GetNext());
    if ( pPg )
    {
        while ( pPg )
        {
            pPg->DecrPhyPageNum();  //inline --nPhyPageNum
            pPg = static_cast<SwPageFrame*>(pPg->GetNext());
        }
    }
    else
        ::SetLastPage( static_cast<SwPageFrame*>(GetPrev()) );

    SwFrame* pRootFrame = GetUpper();

    // cut all connections
    RemoveFromLayout();

    if ( pRootFrame )
        static_cast<SwRootFrame*>(pRootFrame)->CheckViewLayout( nullptr, nullptr );
}

void SwPageFrame::Paste( SwFrame* pParent, SwFrame* pSibling )
{
    OSL_ENSURE( pParent->IsRootFrame(), "Parent is no Root." );
    OSL_ENSURE( pParent, "No parent for Paste()." );
    OSL_ENSURE( pParent != this, "I'm my own parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I am still registered somewhere." );

    // insert into tree structure
    InsertBefore( static_cast<SwLayoutFrame*>(pParent), pSibling );

    // increase the root's page number
    static_cast<SwRootFrame*>(GetUpper())->IncrPhyPageNums();
    if( GetPrev() )
        SetPhyPageNum( static_cast<SwPageFrame*>(GetPrev())->GetPhyPageNum() + 1 );
    else
        SetPhyPageNum( 1 );
    SwPageFrame *pPg = static_cast<SwPageFrame*>(GetNext());
    if ( pPg )
    {
        while ( pPg )
        {
            pPg->IncrPhyPageNum();  //inline ++nPhyPageNum
            pPg->_InvalidatePos();
            pPg->InvalidateLayout();
            pPg = static_cast<SwPageFrame*>(pPg->GetNext());
        }
    }
    else
        ::SetLastPage( this );

    if( Frame().Width() != pParent->Prt().Width() )
        _InvalidateSize();

    InvalidatePos();

    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();

    getRootFrame()->CheckViewLayout( nullptr, nullptr );
}

static void lcl_PrepFlyInCntRegister( SwContentFrame *pFrame )
{
    pFrame->Prepare( PREP_REGISTER );
    if( pFrame->GetDrawObjs() )
    {
        for( size_t i = 0; i < pFrame->GetDrawObjs()->size(); ++i )
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[i];
            if ( dynamic_cast< const SwFlyInContentFrame *>( pAnchoredObj ) !=  nullptr )
            {
                SwFlyFrame* pFly = static_cast<SwFlyInContentFrame*>(pAnchoredObj);
                SwContentFrame *pCnt = pFly->ContainsContent();
                while ( pCnt )
                {
                    lcl_PrepFlyInCntRegister( pCnt );
                    pCnt = pCnt->GetNextContentFrame();
                }
            }
        }
    }
}

void SwPageFrame::PrepareRegisterChg()
{
    SwContentFrame *pFrame = FindFirstBodyContent();
    while( pFrame )
    {
        lcl_PrepFlyInCntRegister( pFrame );
        pFrame = pFrame->GetNextContentFrame();
        if( !IsAnLower( pFrame ) )
            break;
    }
    if( GetSortedObjs() )
    {
        for( size_t i = 0; i < GetSortedObjs()->size(); ++i )
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];
            if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
            {
                SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                pFrame = pFly->ContainsContent();
                while ( pFrame )
                {
                    ::lcl_PrepFlyInCntRegister( pFrame );
                    pFrame = pFrame->GetNextContentFrame();
                }
            }
        }
    }
}

//FIXME: provide missing documentation
/** Check all pages (starting from the given one) if they use the right frame format.
 *
 * If "wrong" pages are found, try to fix this as simple as possible.
 *
 * @param pStart        the page from where to start searching
 * @param bNotifyFields
 * @param ppPrev
 */
void SwFrame::CheckPageDescs( SwPageFrame *pStart, bool bNotifyFields, SwPageFrame** ppPrev )
{
    assert(pStart && "no starting page.");

    SwViewShell *pSh   = pStart->getRootFrame()->GetCurrShell();
    SwViewShellImp *pImp  = pSh ? pSh->Imp() : nullptr;

    if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
    {
        pImp->GetLayAction().SetCheckPageNum( pStart->GetPhyPageNum() );
        return;
    }

    // For the update of page numbering fields, nDocPos provides
    // the page position from where invalidation should start.
    SwTwips nDocPos  = LONG_MAX;

    SwRootFrame *pRoot = static_cast<SwRootFrame*>(pStart->GetUpper());
    SwDoc* pDoc      = pStart->GetFormat()->GetDoc();
    const bool bFootnotes = !pDoc->GetFootnoteIdxs().empty();

    SwPageFrame *pPage = pStart;
    if( pPage->GetPrev() && static_cast<SwPageFrame*>(pPage->GetPrev())->IsEmptyPage() )
        pPage = static_cast<SwPageFrame*>(pPage->GetPrev());
    while ( pPage )
    {
        // obtain PageDesc and FrameFormat
        SwPageDesc *pDesc = pPage->FindPageDesc();
        bool bCheckEmpty = pPage->IsEmptyPage();
        bool bActOdd = pPage->OnRightPage();
        bool bOdd = pPage->WannaRightPage();
        bool bFirst = pPage->OnFirstPage();
        SwFrameFormat *pFormatWish = (bOdd)
            ? pDesc->GetRightFormat(bFirst) : pDesc->GetLeftFormat(bFirst);

        if ( bActOdd != bOdd ||
             pDesc != pPage->GetPageDesc() ||        // wrong Desc
             ( pFormatWish != pPage->GetFormat()  &&       // wrong format and
               ( !pPage->IsEmptyPage() || pFormatWish ) // not blank /empty
             )
           )
        {
            // Updating a page might take a while, so check the WaitCursor
            if( pImp )
                pImp->CheckWaitCursor();

            // invalidate the field, starting from here
            if ( nDocPos == LONG_MAX )
                nDocPos = pPage->GetPrev() ?
                            pPage->GetPrev()->Frame().Top() : pPage->Frame().Top();

            // Cases:
            //  1. Empty page should be "normal" page -> remove empty page and take next one
            //  2. Empty page should have different descriptor -> change
            //  3. Normal page should be empty -> insert empty page if previous page
            //     is not empty, otherwise see (6).
            //  4. Normal page should have different descriptor -> change
            //  5. Normal page should have different format -> change
            //  6. No "wish" format provided -> take the "other" format (left/right) of the PageDesc

            if ( pPage->IsEmptyPage() && ( pFormatWish ||          //1.
                 ( !bOdd && !pPage->GetPrev() ) ) )
            {
                SwPageFrame *pTmp = static_cast<SwPageFrame*>(pPage->GetNext());
                pPage->Cut();
                bool bUpdatePrev = false;
                if (ppPrev && *ppPrev == pPage)
                    bUpdatePrev = true;
                SwFrame::DestroyFrame(pPage);
                if ( pStart == pPage )
                    pStart = pTmp;
                pPage = pTmp;
                if (bUpdatePrev)
                    *ppPrev = pTmp;
                continue;
            }
            else if ( pPage->IsEmptyPage() && !pFormatWish &&  //2.
                      pDesc != pPage->GetPageDesc() )
            {
                pPage->SetPageDesc( pDesc, nullptr );
            }
            else if ( !pPage->IsEmptyPage() &&      //3.
                      bActOdd != bOdd &&
                      ( ( !pPage->GetPrev() && !bOdd ) ||
                        ( pPage->GetPrev() &&
                          !static_cast<SwPageFrame*>(pPage->GetPrev())->IsEmptyPage() )
                      )
                    )
            {
                if ( pPage->GetPrev() )
                    pDesc = static_cast<SwPageFrame*>(pPage->GetPrev())->GetPageDesc();
                SwPageFrame *pTmp = new SwPageFrame( pDoc->GetEmptyPageFormat(),pRoot,pDesc);
                pTmp->Paste( pRoot, pPage );
                pTmp->PreparePage( false );
                pPage = pTmp;
            }
            else if ( pPage->GetPageDesc() != pDesc )           //4.
            {
                SwPageDesc *pOld = pPage->GetPageDesc();
                pPage->SetPageDesc( pDesc, pFormatWish );
                if ( bFootnotes )
                {
                    // If specific values of the FootnoteInfo are changed, something has to happen.
                    // We try to limit the damage...
                    // If the page has no FootnoteCont it might be problematic.
                    // Let's hope that invalidation is enough.
                    SwFootnoteContFrame *pCont = pPage->FindFootnoteCont();
                    if ( pCont && !(pOld->GetFootnoteInfo() == pDesc->GetFootnoteInfo()) )
                        pCont->_InvalidateAll();
                }
            }
            else if ( pFormatWish && pPage->GetFormat() != pFormatWish )         //5.
            {
                pPage->SetFrameFormat( pFormatWish );
            }
            else if ( !pFormatWish )                                       //6.
            {
                // get format with inverted logic
                if (!pFormatWish)
                    pFormatWish = bOdd ? pDesc->GetLeftFormat() : pDesc->GetRightFormat();
                if ( pPage->GetFormat() != pFormatWish )
                    pPage->SetFrameFormat( pFormatWish );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "CheckPageDescs, missing solution" );
            }
#endif
        }
        if ( bCheckEmpty )
        {
            // It also might be that an empty page is not needed at all.
            // However, the algorithm above cannot determine that. It is not needed if the following
            // page can live without it. Do obtain that information, we need to dig deeper...
            SwPageFrame *pPg = static_cast<SwPageFrame*>(pPage->GetNext());
            if( !pPg || pPage->OnRightPage() == pPg->WannaRightPage() )
            {
                // The following page can find a FrameFormat or has no successor -> empty page not needed
                SwPageFrame *pTmp = static_cast<SwPageFrame*>(pPage->GetNext());
                pPage->Cut();
                bool bUpdatePrev = false;
                if (ppPrev && *ppPrev == pPage)
                    bUpdatePrev = true;
                SwFrame::DestroyFrame(pPage);
                if ( pStart == pPage )
                    pStart = pTmp;
                pPage = pTmp;
                if (bUpdatePrev)
                    *ppPrev = pTmp;
                continue;
            }
        }
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }

    pRoot->SetAssertFlyPages();
    SwRootFrame::AssertPageFlys( pStart );

    if ( bNotifyFields && (!pImp || !pImp->IsUpdateExpFields()) )
    {
        SwDocPosUpdate aMsgHint( nDocPos );
        pDoc->getIDocumentFieldsAccess().UpdatePageFields( &aMsgHint );
    }

#if OSL_DEBUG_LEVEL > 0
    //1. check if two empty pages are behind one another
    bool bEmpty = false;
    SwPageFrame *pPg = pStart;
    while ( pPg )
    {
        if ( pPg->IsEmptyPage() )
        {
            if ( bEmpty )
            {
                OSL_FAIL( "double empty pages." );
                break;  // once is enough
            }
            bEmpty = true;
        }
        else
            bEmpty = false;

        pPg = static_cast<SwPageFrame*>(pPg->GetNext());
    }
#endif
}

namespace
{
    bool isDeleteForbidden(const SwPageFrame *pDel)
    {
        const SwLayoutFrame* pBody = pDel->FindBodyCont();
        const SwFrame* pBodyContent = pBody ? pBody->Lower() : nullptr;
        return pBodyContent && pBodyContent->IsDeleteForbidden();
    }
}

SwPageFrame *SwFrame::InsertPage( SwPageFrame *pPrevPage, bool bFootnote )
{
    SwRootFrame *pRoot = static_cast<SwRootFrame*>(pPrevPage->GetUpper());
    SwPageFrame *pSibling = static_cast<SwPageFrame*>(pPrevPage->GetNext());
    SwPageDesc *pDesc = nullptr;

    // insert right (odd) or left (even) page?
    bool bNextOdd = !pPrevPage->OnRightPage();
    bool bWishedOdd = bNextOdd;

    // Which PageDesc is relevant?
    // For ContentFrame take the one from format if provided,
    // otherwise from the Follow of the PrevPage
    if ( IsFlowFrame() && !SwFlowFrame::CastFlowFrame( this )->IsFollow() )
    {   SwFormatPageDesc &rDesc = (SwFormatPageDesc&)GetAttrSet()->GetPageDesc();
        pDesc = rDesc.GetPageDesc();
        if ( rDesc.GetNumOffset() )
        {
            ::boost::optional<sal_uInt16> oNumOffset = rDesc.GetNumOffset();
            bWishedOdd = oNumOffset && (oNumOffset.get() % 2) != 0;
            // use the opportunity to set the flag at root
            pRoot->SetVirtPageNum( true );
        }
    }
    if ( !pDesc )
        pDesc = pPrevPage->GetPageDesc()->GetFollow();

    assert(pDesc && "Missing PageDesc");
    if( !(bWishedOdd ? pDesc->GetRightFormat() : pDesc->GetLeftFormat()) )
        bWishedOdd = !bWishedOdd;
    bool const bWishedFirst = pDesc != pPrevPage->GetPageDesc();

    SwDoc *pDoc = pPrevPage->GetFormat()->GetDoc();
    bool bCheckPages = false;
    // If there is no FrameFormat for this page, create an empty page.
    if( bWishedOdd != bNextOdd )
    {
        SwFrameFormat *const pEmptyFormat = pDoc->GetEmptyPageFormat();
        SwPageDesc *pTmpDesc = pPrevPage->GetPageDesc();
        SwPageFrame *pPage = new SwPageFrame(pEmptyFormat, pRoot, pTmpDesc);
        pPage->Paste( pRoot, pSibling );
        pPage->PreparePage( bFootnote );
        // If the sibling has no body text, destroy it as long as it is no footnote page.
        if ( pSibling && !pSibling->IsFootnotePage() &&
             !pSibling->FindFirstBodyContent() )
        {
            SwPageFrame *pDel = pSibling;
            pSibling = static_cast<SwPageFrame*>(pSibling->GetNext());
            if ( !pDoc->GetFootnoteIdxs().empty() )
                pRoot->RemoveFootnotes( pDel, true );
            pDel->Cut();
            SwFrame::DestroyFrame(pDel);
        }
        else
            bCheckPages = true;
    }
    SwFrameFormat *const pFormat( (bWishedOdd)
            ? pDesc->GetRightFormat(bWishedFirst)
            : pDesc->GetLeftFormat(bWishedFirst) );
    assert(pFormat);
    SwPageFrame *pPage = new SwPageFrame( pFormat, pRoot, pDesc );
    pPage->Paste( pRoot, pSibling );
    pPage->PreparePage( bFootnote );
    // If the sibling has no body text, destroy it as long as it is no footnote page.
    if ( pSibling && !pSibling->IsFootnotePage() &&
         !pSibling->FindFirstBodyContent() && !isDeleteForbidden(pSibling) )
    {
        SwPageFrame *pDel = pSibling;
        pSibling = static_cast<SwPageFrame*>(pSibling->GetNext());
        if ( !pDoc->GetFootnoteIdxs().empty() )
            pRoot->RemoveFootnotes( pDel, true );
        pDel->Cut();
        SwFrame::DestroyFrame(pDel);
    }
    else
        bCheckPages = true;

    if ( pSibling )
    {
        if ( bCheckPages )
        {
            CheckPageDescs( pSibling, false );
            SwViewShell *pSh = getRootFrame()->GetCurrShell();
            SwViewShellImp *pImp = pSh ? pSh->Imp() : nullptr;
            if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
            {
                const sal_uInt16 nNum = pImp->GetLayAction().GetCheckPageNum();
                if ( nNum == pPrevPage->GetPhyPageNum() + 1 )
                    pImp->GetLayAction().SetCheckPageNumDirect(
                                                    pSibling->GetPhyPageNum() );
                return pPage;
            }
        }
        else
            SwRootFrame::AssertPageFlys( pSibling );
    }

    // For the update of page numbering fields, nDocPos provides
    // the page position from where invalidation should start.
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( !pSh || !pSh->Imp()->IsUpdateExpFields() )
    {
        SwDocPosUpdate aMsgHint( pPrevPage->Frame().Top() );
        pDoc->getIDocumentFieldsAccess().UpdatePageFields( &aMsgHint );
    }
    return pPage;
}

sw::sidebarwindows::SidebarPosition SwPageFrame::SidebarPosition() const
{
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if( !pSh || pSh->GetViewOptions()->getBrowseMode() )
    {
        return sw::sidebarwindows::SidebarPosition::RIGHT;
    }
    else
    {
        const bool bLTR = getRootFrame()->IsLeftToRightViewLayout();
        const bool bBookMode = pSh->GetViewOptions()->IsViewLayoutBookMode();
        const bool bRightSidebar = bLTR ? (!bBookMode || OnRightPage()) : (bBookMode && !OnRightPage());

        return bRightSidebar
               ? sw::sidebarwindows::SidebarPosition::RIGHT
               : sw::sidebarwindows::SidebarPosition::LEFT;
    }
}

SwTwips SwRootFrame::GrowFrame( SwTwips nDist, bool bTst, bool )
{
    if ( !bTst )
        Frame().SSize().Height() += nDist;
    return nDist;
}

SwTwips SwRootFrame::ShrinkFrame( SwTwips nDist, bool bTst, bool )
{
    OSL_ENSURE( nDist >= 0, "nDist < 0." );
    OSL_ENSURE( nDist <= Frame().Height(), "nDist > als aktuelle Groesse." );

    if ( !bTst )
        Frame().SSize().Height() -= nDist;
    return nDist;
}

/// remove pages that are not needed at all
void SwRootFrame::RemoveSuperfluous()
{
    // A page is empty if the body text area has no ContentFrame, but not if there
    // is at least one Fly or one footnote attached to the page. Two runs are
    // needed: one for endnote pages and one for the pages of the body text.

    if ( !IsSuperfluous() )
        return;
    mbCheckSuperfluous = false;

    SwPageFrame *pPage = GetLastPage();
    long nDocPos = LONG_MAX;

    // Check the corresponding last page if it is empty and stop loop at the last non-empty page.
    do
    {
        bool bExistEssentialObjs = ( nullptr != pPage->GetSortedObjs() );
        if ( bExistEssentialObjs )
        {
            // Only because the page has Flys does not mean that it is needed. If all Flys are
            // attached to generic content it is also superfluous (checking DocBody should be enough)
            // OD 19.06.2003 #108784# - consider that drawing objects in
            // header/footer are supported now.
            bool bOnlySuperfluosObjs = true;
            SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for ( size_t i = 0; bOnlySuperfluosObjs && i < rObjs.size(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                // OD 2004-01-19 #110582# - do not consider hidden objects
                if ( pPage->GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                     !pAnchoredObj->GetAnchorFrame()->FindFooterOrHeader() )
                {
                    bOnlySuperfluosObjs = false;
                }
            }
            bExistEssentialObjs = !bOnlySuperfluosObjs;
        }

        // OD 19.06.2003 #108784# - optimization: check first, if essential objects
        // exists.
        const SwLayoutFrame* pBody = nullptr;
        if ( bExistEssentialObjs ||
             pPage->FindFootnoteCont() ||
             ( nullptr != ( pBody = pPage->FindBodyCont() ) &&
                ( pBody->ContainsContent() ||
                    // #i47580#
                    // Do not delete page if there's an empty tabframe
                    // left. I think it might be correct to use ContainsAny()
                    // instead of ContainsContent() to cover the empty-table-case,
                    // but I'm not fully sure, since ContainsAny() also returns
                    // SectionFrames. Therefore I prefer to do it the safe way:
                  ( pBody->Lower() && pBody->Lower()->IsTabFrame() ) ) ) )
        {
            if ( pPage->IsFootnotePage() )
            {
                while ( pPage->IsFootnotePage() )
                {
                    pPage = static_cast<SwPageFrame*>(pPage->GetPrev());
                    OSL_ENSURE( pPage, "only endnote pages remain." );
                }
                continue;
            }
            else
                pPage = nullptr;
        }

        if ( pPage )
        {
            SwPageFrame *pEmpty = pPage;
            pPage = static_cast<SwPageFrame*>(pPage->GetPrev());
            if ( !GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
                RemoveFootnotes( pEmpty, true );
            pEmpty->Cut();
            SwFrame::DestroyFrame(pEmpty);
            nDocPos = pPage ? pPage->Frame().Top() : 0;
        }
    } while ( pPage );

    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( nDocPos != LONG_MAX &&
         (!pSh || !pSh->Imp()->IsUpdateExpFields()) )
    {
        SwDocPosUpdate aMsgHint( nDocPos );
        GetFormat()->GetDoc()->getIDocumentFieldsAccess().UpdatePageFields( &aMsgHint );
    }
}

/// Ensures that enough pages exist, so that all page bound frames and draw objects can be placed
void SwRootFrame::AssertFlyPages()
{
    if ( !IsAssertFlyPages() )
        return;
    mbAssertFlyPages = false;

    SwDoc *pDoc = GetFormat()->GetDoc();
    const SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();

    // what page targets the "last" Fly?
    sal_uInt16 nMaxPg = 0;

    for ( size_t i = 0; i < pTable->size(); ++i )
    {
        const SwFormatAnchor &rAnch = (*pTable)[i]->GetAnchor();
        if ( !rAnch.GetContentAnchor() && nMaxPg < rAnch.GetPageNum() )
            nMaxPg = rAnch.GetPageNum();
    }
    // How many pages exist at the moment?
    SwPageFrame *pPage = static_cast<SwPageFrame*>(Lower());
    while ( pPage && pPage->GetNext() &&
            !static_cast<SwPageFrame*>(pPage->GetNext())->IsFootnotePage() )
    {
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }

    if ( nMaxPg > pPage->GetPhyPageNum() )
    {
        // Continue pages based on the rules of the PageDesc after the last page.
        bool bOdd = (pPage->GetPhyPageNum() % 2) != 0;
        SwPageDesc *pDesc = pPage->GetPageDesc();
        SwFrame *pSibling = pPage->GetNext();
        for ( sal_uInt16 i = pPage->GetPhyPageNum(); i < nMaxPg; ++i  )
        {
            if ( !(bOdd ? pDesc->GetRightFormat() : pDesc->GetLeftFormat()) )
            {
                // Insert empty page (but Flys will be stored in the next page)
                pPage = new SwPageFrame( pDoc->GetEmptyPageFormat(), this, pDesc );
                pPage->Paste( this, pSibling );
                pPage->PreparePage( false );
                bOdd = !bOdd;
                ++i;
            }
            pPage = new
                    SwPageFrame( (bOdd ? pDesc->GetRightFormat() :
                                       pDesc->GetLeftFormat()), this, pDesc );
            pPage->Paste( this, pSibling );
            pPage->PreparePage( false );
            bOdd = !bOdd;
            pDesc = pDesc->GetFollow();
        }
        // If the endnote pages are now corrupt, destroy them.
        if ( !pDoc->GetFootnoteIdxs().empty() )
        {
            pPage = static_cast<SwPageFrame*>(Lower());
            while ( pPage && !pPage->IsFootnotePage() )
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());

            if ( pPage )
            {
                SwPageDesc *pTmpDesc = pPage->FindPageDesc();
                bOdd = pPage->OnRightPage();
                if ( pPage->GetFormat() !=
                     (bOdd ? pTmpDesc->GetRightFormat() : pTmpDesc->GetLeftFormat()) )
                    RemoveFootnotes( pPage, false, true );
            }
        }
    }
}

/// Ensure that after the given page all page-bound objects are located on the correct page
void SwRootFrame::AssertPageFlys( SwPageFrame *pPage )
{
    while ( pPage )
    {
        if (pPage->GetSortedObjs())
        {
            size_t i = 0;
            while ( pPage->GetSortedObjs() && i< pPage->GetSortedObjs()->size() )
            {
                // #i28701#
                SwFrameFormat& rFormat = (*pPage->GetSortedObjs())[i]->GetFrameFormat();
                const SwFormatAnchor &rAnch = rFormat.GetAnchor();
                const sal_uInt16 nPg = rAnch.GetPageNum();
                if ((rAnch.GetAnchorId() == FLY_AT_PAGE) &&
                     nPg != pPage->GetPhyPageNum() )
                {
                    // If on the wrong page, check if previous page is empty
                    if( nPg && !(pPage->GetPhyPageNum()-1 == nPg &&
                        static_cast<SwPageFrame*>(pPage->GetPrev())->IsEmptyPage()) )
                    {
                        // It can move by itself. Just send a modify to its anchor attribute.
#if OSL_DEBUG_LEVEL > 1
                        const size_t nCnt = pPage->GetSortedObjs()->size();
                        rFormat.NotifyClients( 0, (SwFormatAnchor*)&rAnch );
                        OSL_ENSURE( !pPage->GetSortedObjs() ||
                                nCnt != pPage->GetSortedObjs()->size(),
                                "Object couldn't be reattached!" );
#else
                        rFormat.NotifyClients( nullptr, &rAnch );
#endif
                        // Do not increment index, in this case
                        continue;
                    }
                }
                ++i;
            }
        }
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }
}

Size SwRootFrame::ChgSize( const Size& aNewSize )
{
    Frame().SSize() = aNewSize;
    _InvalidatePrt();
    mbFixSize = false;
    return Frame().SSize();
}

void SwRootFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( !mbValidPos )
    {   mbValidPos = true;
        maFrame.Pos().setX(DOCUMENTBORDER);
        maFrame.Pos().setY(DOCUMENTBORDER);
    }
    if ( !mbValidPrtArea )
    {   mbValidPrtArea = true;
        maPrt.Pos().setX(0);
        maPrt.Pos().setY(0);
        maPrt.SSize( maFrame.SSize() );
    }
    if ( !mbValidSize )
        // SSize is set by the pages (Cut/Paste).
        mbValidSize = true;
}

void SwRootFrame::ImplInvalidateBrowseWidth()
{
    mbBrowseWidthValid = false;
    SwFrame *pPg = Lower();
    while ( pPg )
    {
        pPg->InvalidateSize();
        pPg = pPg->GetNext();
    }
}

void SwRootFrame::ImplCalcBrowseWidth()
{
    OSL_ENSURE( GetCurrShell() && GetCurrShell()->GetViewOptions()->getBrowseMode(),
            "CalcBrowseWidth and not in BrowseView" );

    // The (minimal) with is determined from borders, tables and paint objects.
    // It is calculated based on the attributes. Thus, it is not relevant how wide they are
    // currently but only how wide they want to be.
    // Frames and paint objects inside other objects (frames, tables) do not count.
    // Borders and columns are not taken into account.

    SwFrame *pFrame = ContainsContent();
    while ( pFrame && !pFrame->IsInDocBody() )
        pFrame = static_cast<SwContentFrame*>(pFrame)->GetNextContentFrame();
    if ( !pFrame )
        return;

    mbBrowseWidthValid = true;
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    mnBrowseWidth = pSh
                    ? MINLAY + 2 * pSh->GetOut()->
                                PixelToLogic( pSh->GetBrowseBorder() ).Width()
                    : 5000;
    do
    {
        if ( pFrame->IsInTab() )
            pFrame = pFrame->FindTabFrame();

        if ( pFrame->IsTabFrame() &&
             !static_cast<SwLayoutFrame*>(pFrame)->GetFormat()->GetFrameSize().GetWidthPercent() )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFrame );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            const SwFormatHoriOrient &rHori = rAttrs.GetAttrSet().GetHoriOrient();
            long nWidth = rAttrs.GetSize().Width();
            if ( nWidth < USHRT_MAX-2000 && //-2k, because USHRT_MAX gets missing while trying to resize!
                 text::HoriOrientation::FULL != rHori.GetHoriOrient() )
            {
                const SwHTMLTableLayout *pLayoutInfo =
                    static_cast<const SwTabFrame *>(pFrame)->GetTable()
                                            ->GetHTMLTableLayout();
                if ( pLayoutInfo )
                    nWidth = std::min( nWidth, pLayoutInfo->GetBrowseWidthMin() );

                switch ( rHori.GetHoriOrient() )
                {
                    case text::HoriOrientation::NONE:
                        // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                        nWidth += rAttrs.CalcLeft( pFrame ) + rAttrs.CalcRight( pFrame );
                        break;
                    case text::HoriOrientation::LEFT_AND_WIDTH:
                        nWidth += rAttrs.CalcLeft( pFrame );
                        break;
                    default:
                        break;
                }
                mnBrowseWidth = std::max( mnBrowseWidth, nWidth );
            }
        }
        else if ( pFrame->GetDrawObjs() )
        {
            for ( size_t i = 0; i < pFrame->GetDrawObjs()->size(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[i];
                const SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
                const bool bFly = dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr;
                if ((bFly && (FAR_AWAY == pAnchoredObj->GetObjRect().Width()))
                    || rFormat.GetFrameSize().GetWidthPercent())
                {
                    continue;
                }

                long nWidth = 0;
                switch ( rFormat.GetAnchor().GetAnchorId() )
                {
                    case FLY_AS_CHAR:
                        nWidth = bFly ? rFormat.GetFrameSize().GetWidth() :
                                        pAnchoredObj->GetObjRect().Width();
                        break;
                    case FLY_AT_PARA:
                        {
                            // #i33170#
                            // Reactivated old code because
                            // nWidth = pAnchoredObj->GetObjRect().Right()
                            // gives wrong results for objects that are still
                            // at position FAR_AWAY.
                            if ( bFly )
                            {
                                nWidth = rFormat.GetFrameSize().GetWidth();
                                const SwFormatHoriOrient &rHori = rFormat.GetHoriOrient();
                                switch ( rHori.GetHoriOrient() )
                                {
                                    case text::HoriOrientation::NONE:
                                        nWidth += rHori.GetPos();
                                        break;
                                    case text::HoriOrientation::INSIDE:
                                    case text::HoriOrientation::LEFT:
                                        if ( text::RelOrientation::PRINT_AREA == rHori.GetRelationOrient() )
                                            nWidth += pFrame->Prt().Left();
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                                // Paint objects to not have attributes and
                                // are defined by their current size
                                nWidth = pAnchoredObj->GetObjRect().Right() -
                                         pAnchoredObj->GetDrawObj()->GetAnchorPos().X();
                        }
                        break;
                    default:    /* do nothing */;
                }
                mnBrowseWidth = std::max( mnBrowseWidth, nWidth );
            }
        }
        pFrame = pFrame->FindNextCnt();
    } while ( pFrame );
}

void SwRootFrame::StartAllAction()
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            if ( dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr )
                static_cast<SwCursorShell*>(&rSh)->StartAction();
            else
                rSh.StartAction();
        }
}

void SwRootFrame::EndAllAction( bool bVirDev )
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            const bool bOldEndActionByVirDev = rSh.IsEndActionByVirDev();
            rSh.SetEndActionByVirDev( bVirDev );
            if ( dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr )
            {
                static_cast<SwCursorShell*>(&rSh)->EndAction();
                static_cast<SwCursorShell*>(&rSh)->CallChgLnk();
                if ( dynamic_cast<const SwFEShell*>( &rSh) !=  nullptr )
                    static_cast<SwFEShell*>(&rSh)->SetChainMarker();
            }
            else
                rSh.EndAction();
            rSh.SetEndActionByVirDev( bOldEndActionByVirDev );
        }
}

void SwRootFrame::UnoRemoveAllActions()
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            // #i84729#
            // No end action, if <SwViewShell> instance is currently in its end action.
            // Recursives calls to <::EndAction()> are not allowed.
            if ( !rSh.IsInEndAction() )
            {
                OSL_ENSURE(!rSh.GetRestoreActions(), "Restore action count is already set!");
                bool bCursor = dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr;
                bool bFE = dynamic_cast<const SwFEShell*>( &rSh) !=  nullptr;
                sal_uInt16 nRestore = 0;
                while( rSh.ActionCount() )
                {
                    if( bCursor )
                    {
                        static_cast<SwCursorShell*>(&rSh)->EndAction();
                        static_cast<SwCursorShell*>(&rSh)->CallChgLnk();
                        if ( bFE )
                            static_cast<SwFEShell*>(&rSh)->SetChainMarker();
                    }
                    else
                        rSh.EndAction();
                    nRestore++;
                }
                rSh.SetRestoreActions(nRestore);
            }
            rSh.LockView(true);
        }
}

void SwRootFrame::UnoRestoreAllActions()
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            sal_uInt16 nActions = rSh.GetRestoreActions();
            while( nActions-- )
            {
                if ( dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr )
                    static_cast<SwCursorShell*>(&rSh)->StartAction();
                else
                    rSh.StartAction();
            }
            rSh.SetRestoreActions(0);
            rSh.LockView(false);
        }
}

// Helper functions for SwRootFrame::CheckViewLayout
static void lcl_MoveAllLowers( SwFrame* pFrame, const Point& rOffset );

static void lcl_MoveAllLowerObjs( SwFrame* pFrame, const Point& rOffset )
{
    const bool bPage = pFrame->IsPageFrame();
    const SwSortedObjs* pSortedObj = bPage
                        ? static_cast<SwPageFrame*>(pFrame)->GetSortedObjs()
                        : pFrame->GetDrawObjs();
    if (pSortedObj == nullptr)
        return;

    // note: pSortedObj elements may be removed and inserted from
    // MoveObjectIfActive(), invalidating iterators
    for (size_t i = 0; i < pSortedObj->size(); ++i)
    {
        SwAnchoredObject *const pAnchoredObj = (*pSortedObj)[i];
        const SwFrameFormat& rObjFormat = pAnchoredObj->GetFrameFormat();
        const SwFormatAnchor& rAnchor = rObjFormat.GetAnchor();

        // all except from the as character anchored objects are moved
        // when processing the page frame:
        if ( !bPage && (rAnchor.GetAnchorId() != FLY_AS_CHAR) )
            continue;

        SwObjPositioningInProgress aPosInProgress( *pAnchoredObj );

        if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) != nullptr )
        {
            SwFlyFrame* pFlyFrame( static_cast<SwFlyFrame*>(pAnchoredObj) );
            lcl_MoveAllLowers( pFlyFrame, rOffset );
            pFlyFrame->NotifyDrawObj();
            // --> let the active embedded object be moved
            SwFrame* pLower = pFlyFrame->Lower();
            if ( pLower )
            {
                if ( pLower->IsNoTextFrame() )
                {
                    SwRootFrame* pRoot = pLower->getRootFrame();
                    SwViewShell *pSh = pRoot ? pRoot->GetCurrShell() : nullptr;
                    if ( pSh )
                    {
                        SwContentFrame* pContentFrame = static_cast<SwContentFrame*>(pLower);
                        SwOLENode* pNode = pContentFrame->GetNode()->GetOLENode();
                        if ( pNode )
                        {
                            svt::EmbeddedObjectRef& xObj = pNode->GetOLEObj().GetObject();
                            if ( xObj.is() )
                            {
                                for(SwViewShell& rSh : pSh->GetRingContainer())
                                {
                                    SwFEShell* pFEShell = dynamic_cast< SwFEShell* >( &rSh );
                                    if ( pFEShell )
                                        pFEShell->MoveObjectIfActive( xObj, rOffset );
                                }
                            }
                        }
                    }
                }
            }
        }
        else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchoredObj ) !=  nullptr )
        {
            SwAnchoredDrawObject* pAnchoredDrawObj( static_cast<SwAnchoredDrawObject*>(pAnchoredObj) );

            // don't touch objects that are not yet positioned:
            if ( pAnchoredDrawObj->NotYetPositioned() )
                continue;

            const Point& aCurrAnchorPos = pAnchoredDrawObj->GetDrawObj()->GetAnchorPos();
            const Point aNewAnchorPos( ( aCurrAnchorPos + rOffset ) );
            pAnchoredDrawObj->DrawObj()->SetAnchorPos( aNewAnchorPos );
            pAnchoredDrawObj->SetLastObjRect( pAnchoredDrawObj->GetObjRect().SVRect() );

            // clear contour cache
            if ( pAnchoredDrawObj->GetFrameFormat().GetSurround().IsContour() )
                ClrContourCache( pAnchoredDrawObj->GetDrawObj() );
        }
        // #i92511#
        // cache for object rectangle inclusive spaces has to be invalidated.
        pAnchoredObj->InvalidateObjRectWithSpaces();
    }
}

static void lcl_MoveAllLowers( SwFrame* pFrame, const Point& rOffset )
{
    const SwRect aFrame( pFrame->Frame() );

    // first move the current frame
    Point &rPoint = pFrame->Frame().Pos();
    if (rPoint.X() != FAR_AWAY)
        rPoint.X() += rOffset.X();
    if (rPoint.Y() != FAR_AWAY)
        rPoint.Y() += rOffset.Y();

    // Don't forget accessibility:
    if( pFrame->IsAccessibleFrame() )
    {
        SwRootFrame *pRootFrame = pFrame->getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( pFrame, aFrame );
        }
    }

    // the move any objects
    lcl_MoveAllLowerObjs( pFrame, rOffset );

    // finally, for layout frames we have to call this function recursively:
    if ( dynamic_cast< const SwLayoutFrame *>( pFrame ) !=  nullptr )
    {
        SwFrame* pLowerFrame = pFrame->GetLower();
        while ( pLowerFrame )
        {
            lcl_MoveAllLowers( pLowerFrame, rOffset );
            pLowerFrame = pLowerFrame->GetNext();
        }
    }
}

// Calculate how the pages have to be positioned
void SwRootFrame::CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea )
{
    SwViewShell* pSh = GetCurrShell();
    vcl::RenderContext* pRenderContext = pSh ? pSh->GetOut() : nullptr;
    // #i91432#
    // No calculation of page positions, if only an empty page is present.
    // This situation occurs when <SwRootFrame> instance is in construction
    // and the document contains only left pages.
    if ( Lower()->GetNext() == nullptr &&
         static_cast<SwPageFrame*>(Lower())->IsEmptyPage() )
    {
        return;
    }

    if ( !pVisArea )
    {
        // no early return for bNewPage
        if ( mnViewWidth < 0 )
            mnViewWidth = 0;
    }
    else
    {
        assert(pViewOpt && "CheckViewLayout required ViewOptions");

        const sal_uInt16 nColumns = pViewOpt->GetViewLayoutColumns();
        const bool bBookMode = pViewOpt->IsViewLayoutBookMode();

        if ( nColumns == mnColumns && bBookMode == mbBookMode && pVisArea->Width() == mnViewWidth && !mbSidebarChanged )
            return;

        mnColumns = nColumns;
        mbBookMode = bBookMode;
        mnViewWidth = pVisArea->Width();
        mbSidebarChanged = false;
    }

    if( GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE ) )
    {
        mnColumns = 1;
        mbBookMode = false;
    }

    Calc(pRenderContext);

    const bool bOldCallbackActionEnabled = IsCallbackActionEnabled();
    SetCallbackActionEnabled( false );

    maPageRects.clear();

    const long nBorder = Frame().Pos().getX();
    const long nVisWidth = mnViewWidth - 2 * nBorder;
    const long nGapBetweenPages = pViewOpt ? pViewOpt->GetGapBetweenPages()
                                           : (pSh ? pSh->GetViewOptions()->GetGapBetweenPages()
                                                  : SwViewOption::GetDefGapBetweenPages());

    // check how many pages fit into the first page layout row:
    SwPageFrame* pPageFrame = static_cast<SwPageFrame*>(Lower());

    // will contain the number of pages per row. 0 means that
    // the page does not fit.
    long nWidthRemain = nVisWidth;

    // after one row has been processed, these variables contain
    // the width of the row and the maxium of the page heights
    long nCurrentRowHeight = 0;
    long nCurrentRowWidth = 0;

    // these variables are used to finally set the size of the
    // root frame
    long nSumRowHeight = 0;
    SwTwips nMinPageLeft = TWIPS_MAX;
    SwTwips nMaxPageRight = 0;
    SwPageFrame* pStartOfRow = pPageFrame;
    sal_uInt16 nNumberOfPagesInRow = mbBookMode ? 1 : 0; // in book view, start with right page
    bool bFirstRow = true;

    bool bPageChanged = false;
    const bool bRTL = !IsLeftToRightViewLayout();
    const SwTwips nSidebarWidth = SwPageFrame::GetSidebarBorderWidth( pSh );

    while ( pPageFrame )
    {
        // we consider the current page to be "start of row" if
        // 1. it is the first page in the current row or
        // 2. it is the second page in the row and the first page is an empty page in non-book view:
        const bool bStartOfRow = pPageFrame == pStartOfRow ||
                                             ( pStartOfRow->IsEmptyPage() && pPageFrame == pStartOfRow->GetNext() && !mbBookMode );

        const bool bEmptyPage = pPageFrame->IsEmptyPage() && !mbBookMode;

        // no half doc border space for first page in each row and
        long nPageWidth = 0;
        long nPageHeight = 0;

        if ( mbBookMode )
        {
            const SwFrame& rFormatPage = pPageFrame->GetFormatPage();

            nPageWidth  = rFormatPage.Frame().Width()  + nSidebarWidth + ((bStartOfRow || 1 == (pPageFrame->GetPhyPageNum()%2)) ? 0 : nGapBetweenPages);
            nPageHeight = rFormatPage.Frame().Height() + nGapBetweenPages;
        }
        else
        {
            if ( !pPageFrame->IsEmptyPage() )
            {
                nPageWidth  = pPageFrame->Frame().Width() + nSidebarWidth + (bStartOfRow ? 0 : nGapBetweenPages);
                nPageHeight = pPageFrame->Frame().Height() + nGapBetweenPages;
            }
        }

        if ( !bEmptyPage )
            ++nNumberOfPagesInRow;

        // finish current row if
        // 1. in dynamic mode the current page does not fit anymore or
        // 2. the current page exceeds the maximum number of columns
        bool bRowFinished = (0 == mnColumns && nWidthRemain < nPageWidth ) ||
                            (0 != mnColumns && mnColumns < nNumberOfPagesInRow);

        // make sure that at least one page goes to the current row:
        if ( !bRowFinished || bStartOfRow )
        {
            // current page is allowed to be in current row
            nWidthRemain = nWidthRemain - nPageWidth;

            nCurrentRowWidth = nCurrentRowWidth + nPageWidth;
            nCurrentRowHeight = std::max( nCurrentRowHeight, nPageHeight );

            pPageFrame = static_cast<SwPageFrame*>(pPageFrame->GetNext());

            if ( !pPageFrame )
                bRowFinished = true;
        }

        if ( bRowFinished )
        {
            // pPageFrame now points to the first page in the new row or null
            // pStartOfRow points to the first page in the current row

            // special centering for last row. pretend to fill the last row with virtual copies of the last page before centering:
            if ( !pPageFrame && nWidthRemain > 0 )
            {
                // find last page in current row:
                const SwPageFrame* pLastPageInCurrentRow = pStartOfRow;
                while( pLastPageInCurrentRow->GetNext() )
                    pLastPageInCurrentRow = static_cast<const SwPageFrame*>(pLastPageInCurrentRow->GetNext());

                if ( pLastPageInCurrentRow->IsEmptyPage() )
                    pLastPageInCurrentRow = static_cast<const SwPageFrame*>(pLastPageInCurrentRow->GetPrev());

                // check how many times the last page would still fit into the remaining space:
                sal_uInt16 nNumberOfVirtualPages = 0;
                const sal_uInt16 nMaxNumberOfVirtualPages = mnColumns > 0 ? mnColumns - nNumberOfPagesInRow : USHRT_MAX;
                SwTwips nRemain = nWidthRemain;
                SwTwips nVirtualPagesWidth = 0;
                SwTwips nLastPageWidth = pLastPageInCurrentRow->Frame().Width() + nSidebarWidth;

                while ( ( mnColumns > 0 || nRemain > 0 ) && nNumberOfVirtualPages < nMaxNumberOfVirtualPages )
                {
                    SwTwips nLastPageWidthWithGap = nLastPageWidth;
                    if ( !mbBookMode || ( 0 == (nNumberOfVirtualPages + nNumberOfPagesInRow) %2) )
                        nLastPageWidthWithGap += nGapBetweenPages;

                    if ( mnColumns > 0 || nLastPageWidthWithGap < nRemain )
                    {
                        ++nNumberOfVirtualPages;
                        nVirtualPagesWidth += nLastPageWidthWithGap;
                    }
                    nRemain = nRemain - nLastPageWidthWithGap;
                }

                nCurrentRowWidth = nCurrentRowWidth + nVirtualPagesWidth;
            }

            // first page in book mode is always special:
            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nCurrentRowWidth +=
                    pStartOfRow->GetFormatPage().Frame().Width() + nSidebarWidth;
            }

            // center page if possible
            long nSizeDiff = 0;
            if (nVisWidth > nCurrentRowWidth && !comphelper::LibreOfficeKit::isActive())
                nSizeDiff = ( nVisWidth - nCurrentRowWidth ) / 2;

            // adjust positions of pages in current row
            long nX = nSizeDiff;

            const long nRowStart = nBorder + nSizeDiff;
            const long nRowEnd   = nRowStart + nCurrentRowWidth;

            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nX += pStartOfRow->GetFormatPage().Frame().Width() + nSidebarWidth;
            }

            SwPageFrame* pEndOfRow = pPageFrame;
            SwPageFrame* pPageToAdjust = pStartOfRow;

            do
            {
                const SwPageFrame* pFormatPage = pPageToAdjust;
                if ( mbBookMode )
                    pFormatPage = &pPageToAdjust->GetFormatPage();

                const SwTwips nCurrentPageWidth = pFormatPage->Frame().Width() + (pFormatPage->IsEmptyPage() ? 0 : nSidebarWidth);
                const Point aOldPagePos = pPageToAdjust->Frame().Pos();
                const bool bLeftSidebar = pPageToAdjust->SidebarPosition() == sw::sidebarwindows::SidebarPosition::LEFT;
                const SwTwips nLeftPageAddOffset = bLeftSidebar ?
                                                   nSidebarWidth :
                                                   0;

                Point aNewPagePos( nBorder + nX, nBorder + nSumRowHeight );
                Point aNewPagePosWithLeftOffset( nBorder + nX + nLeftPageAddOffset, nBorder + nSumRowHeight );

                // RTL view layout: Calculate mirrored page position
                if ( bRTL )
                {
                    const long nXOffsetInRow = aNewPagePos.getX() - nRowStart;
                    aNewPagePos.setX(nRowEnd - nXOffsetInRow - nCurrentPageWidth);
                    aNewPagePosWithLeftOffset = aNewPagePos;
                    aNewPagePosWithLeftOffset.setX(aNewPagePosWithLeftOffset.getX() + nLeftPageAddOffset);
                }

                if ( aNewPagePosWithLeftOffset != aOldPagePos )
                {
                    lcl_MoveAllLowers( pPageToAdjust, aNewPagePosWithLeftOffset - aOldPagePos );
                    pPageToAdjust->SetCompletePaint();
                    bPageChanged = true;
                }

                // calculate area covered by the current page and store to
                // maPageRects. This is used e.g., for cursor setting
                const bool bFirstColumn = pPageToAdjust == pStartOfRow;
                const bool bLastColumn = pPageToAdjust->GetNext() == pEndOfRow;
                const bool bLastRow = !pEndOfRow;

                nMinPageLeft  = std::min( nMinPageLeft, aNewPagePos.getX() );
                nMaxPageRight = std::max( nMaxPageRight, aNewPagePos.getX() + nCurrentPageWidth);

                // border of nGapBetweenPages around the current page:
                SwRect aPageRectWithBorders( aNewPagePos.getX() - nGapBetweenPages,
                                             aNewPagePos.getY(),
                                             pPageToAdjust->Frame().SSize().Width() + nGapBetweenPages + nSidebarWidth,
                                             nCurrentRowHeight );

                static const long nOuterClickDiff = 1000000;

                // adjust borders for these special cases:
                if ( (bFirstColumn && !bRTL) || (bLastColumn && bRTL) )
                    aPageRectWithBorders.SubLeft( nOuterClickDiff );
                if ( (bLastColumn && !bRTL) || (bFirstColumn && bRTL) )
                    aPageRectWithBorders.AddRight( nOuterClickDiff );
                if ( bFirstRow )
                    aPageRectWithBorders.SubTop( nOuterClickDiff );
                if ( bLastRow )
                    aPageRectWithBorders.AddBottom( nOuterClickDiff );

                maPageRects.push_back( aPageRectWithBorders );

                nX = nX + nCurrentPageWidth;
                pPageToAdjust = static_cast<SwPageFrame*>(pPageToAdjust->GetNext());

                // distance to next page
                if ( pPageToAdjust && pPageToAdjust != pEndOfRow )
                {
                    // in book view, we add the x gap before left (even) pages:
                    if ( mbBookMode )
                    {
                        if ( 0 == (pPageToAdjust->GetPhyPageNum()%2) )
                            nX = nX + nGapBetweenPages;
                    }
                    else
                    {
                        // in non-book view, don't add x gap before
                        // 1. the last empty page in a row
                        // 2. after an empty page
                        const bool bDontAddGap = ( pPageToAdjust->IsEmptyPage() && pPageToAdjust->GetNext() == pEndOfRow ) ||
                                                 ( static_cast<SwPageFrame*>(pPageToAdjust->GetPrev())->IsEmptyPage() );

                        if  ( !bDontAddGap )
                            nX = nX + nGapBetweenPages;
                    }
                }
            }
            while (pPageToAdjust && pPageToAdjust != pEndOfRow);

            // adjust values for root frame size
            nSumRowHeight = nSumRowHeight + nCurrentRowHeight;

            // start new row:
            nCurrentRowHeight = 0;
            nCurrentRowWidth = 0;
            pStartOfRow = pEndOfRow;
            nWidthRemain = nVisWidth;
            nNumberOfPagesInRow = 0;
            bFirstRow = false;
        } // end row finished
    } // end while

    // set size of root frame:
    const Size aOldSize( Frame().SSize() );
    const Size aNewSize( nMaxPageRight - nBorder, nSumRowHeight - nGapBetweenPages );

    if ( bPageChanged || aNewSize != aOldSize )
    {
        ChgSize( aNewSize );
        ::AdjustSizeChgNotify( this );
        Calc(pRenderContext);

        if ( pSh && pSh->GetDoc()->GetDocShell() )
        {
            pSh->SetFirstVisPageInvalid();
            if (bOldCallbackActionEnabled)
            {
                pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
                pSh->GetDoc()->GetDocShell()->Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
            }
        }
    }

    maPagesArea.Pos( Frame().Pos() );
    maPagesArea.SSize( aNewSize );
    if ( TWIPS_MAX != nMinPageLeft )
        maPagesArea._Left( nMinPageLeft );

    SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

bool SwRootFrame::IsLeftToRightViewLayout() const
{
    // Layout direction determined by layout direction of the first page.
    // #i88036#
    // Only ask a non-empty page frame for its layout direction
    const SwPageFrame& rPage =
                    dynamic_cast<const SwPageFrame&>(*Lower()).GetFormatPage();
    return !rPage.IsRightToLeft() && !rPage.IsVertical();
}

const SwPageFrame& SwPageFrame::GetFormatPage() const
{
    const SwPageFrame* pRet = this;
    if ( IsEmptyPage() )
    {
        pRet = static_cast<const SwPageFrame*>( OnRightPage() ? GetNext() : GetPrev() );
        // #i88035#
        // Typically a right empty page frame has a next non-empty page frame and
        // a left empty page frame has a previous non-empty page frame.
        // But under certain cirsumstances this assumption is not true -
        // e.g. during insertion of a left page at the end of the document right
        // after a left page in an intermediate state a right empty page does not
        // have a next page frame.
        if ( pRet == nullptr )
        {
            if ( OnRightPage() )
            {
                pRet = static_cast<const SwPageFrame*>( GetPrev() );
            }
            else
            {
                pRet = static_cast<const SwPageFrame*>( GetNext() );
            }
        }
        assert(pRet &&
                "<SwPageFrame::GetFormatPage()> - inconsistent layout: empty page without previous and next page frame --> crash.");
    }
    return *pRet;
}

bool SwPageFrame::IsOverHeaderFooterArea( const Point& rPt, FrameControlType &rControl ) const
{
    long nUpperLimit = 0;
    long nLowerLimit = 0;
    const SwFrame* pFrame = Lower();
    while ( pFrame )
    {
        if ( pFrame->IsBodyFrame() )
        {
            nUpperLimit = pFrame->Frame().Top();
            nLowerLimit = pFrame->Frame().Bottom();
        }
        else if ( pFrame->IsFootnoteContFrame() )
            nLowerLimit = pFrame->Frame().Bottom();

        pFrame = pFrame->GetNext();
    }

    SwRect aHeaderArea( Frame().TopLeft(),
           Size( Frame().Width(), nUpperLimit - Frame().Top() ) );

    SwViewShell* pViewShell = getRootFrame()->GetCurrShell();
    const bool bHideWhitespaceMode = pViewShell->GetViewOptions()->IsHideWhitespaceMode();
    if ( aHeaderArea.IsInside( rPt ) )
    {
        if (!bHideWhitespaceMode || static_cast<const SwFrameFormat*>(GetRegisteredIn())->GetHeader().IsActive())
        {
            rControl = Header;
            return true;
        }
    }
    else
    {
        SwRect aFooterArea( Point( Frame().Left(), nLowerLimit ),
                Size( Frame().Width(), Frame().Bottom() - nLowerLimit ) );

        if ( aFooterArea.IsInside( rPt ) &&
             (!bHideWhitespaceMode || static_cast<const SwFrameFormat*>(GetRegisteredIn())->GetFooter().IsActive()) )
        {
            rControl = Footer;
            return true;
        }
    }

    return false;
}

bool SwPageFrame::CheckPageHeightValidForHideWhitespace(SwTwips nDiff)
{
    SwViewShell* pShell = getRootFrame()->GetCurrShell();
    if (pShell && pShell->GetViewOptions()->IsWhitespaceHidden())
    {
        // When whitespace is hidden, the page frame has two heights: the
        // nominal (defined by the frame format), and the actual (which is
        // at most the nominal height, but can be smaller in case there is
        // no content for the whole page).
        // The layout size is the actual one, but we want to move the
        // content frame to a new page only in case it doesn't fit the
        // nominal size.
        if (nDiff < 0)
        {
            // Content frame doesn't fit the actual size, check if it fits the nominal one.
            const SwFrameFormat* pPageFormat = static_cast<const SwFrameFormat*>(GetRegisteredIn());
            const Size& rPageSize = pPageFormat->GetFrameSize().GetSize();
            long nWhitespace = rPageSize.getHeight() - Frame().Height();
            if (nWhitespace > -nDiff)
            {
                // It does: don't move it and invalidate our page frame so
                // that it gets a larger height.
                return false;
            }
        }
    }

    return true;
}

SwTextGridItem const* GetGridItem(SwPageFrame const*const pPage)
{
    if (pPage && pPage->HasGrid())
    {
        SwTextGridItem const& rGridItem(
                pPage->GetPageDesc()->GetMaster().GetTextGrid());
        if (GRID_NONE != rGridItem.GetGridType())
        {
            return &rGridItem;
        }
    }
    return nullptr;
}

sal_uInt16 GetGridWidth(SwTextGridItem const& rG, SwDoc const& rDoc)
{
    return (rDoc.IsSquaredPageMode()) ? rG.GetBaseHeight() : rG.GetBaseWidth();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
