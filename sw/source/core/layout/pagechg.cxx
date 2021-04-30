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

#include <config_wasm_strip.h>

#include <comphelper/lok.hxx>
#include <ndole.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <svl/itemiter.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtpdsc.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <ftninfo.hxx>
#include <frmtool.hxx>
#include <tgrditem.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <frameformats.hxx>

#include <viewimp.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <dcontact.hxx>
#include <hints.hxx>
#include <FrameControlsManager.hxx>

#include <ftnidx.hxx>
#include <bodyfrm.hxx>
#include <ftnfrm.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <layact.hxx>
#include <flyfrms.hxx>
#include <htmltbl.hxx>
#include <pagedesc.hxx>
#include <editeng/frmdiritem.hxx>
#include <sortedobjs.hxx>
#include <calbck.hxx>
#include <txtfly.hxx>

using namespace ::com::sun::star;

SwBodyFrame::SwBodyFrame( SwFrameFormat *pFormat, SwFrame* pSib ):
    SwLayoutFrame( pFormat, pSib )
{
    mnFrameType = SwFrameType::Body;
}

void SwBodyFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    // Formatting of the body is too simple, thus, it gets its own format method.
    // Borders etc. are not taken into account here.
    // Width is taken from the PrtArea of the Upper. Height is the height of the
    // PrtArea of the Upper minus any neighbors (for robustness).
    // The PrtArea has always the size of the frame.

    if ( !isFrameAreaSizeValid() )
    {
        SwTwips nHeight = GetUpper()->getFramePrintArea().Height();
        SwTwips nWidth = GetUpper()->getFramePrintArea().Width();
        const SwFrame *pFrame = GetUpper()->Lower();
        do
        {
            if ( pFrame != this )
            {
                if( pFrame->IsVertical() )
                    nWidth -= pFrame->getFrameArea().Width();
                else
                    nHeight -= pFrame->getFrameArea().Height();
            }
            pFrame = pFrame->GetNext();
        } while ( pFrame );

        if ( nHeight < 0 )
        {
            nHeight = 0;
        }

        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.Height( nHeight );

        if( IsVertical() && !IsVertLR() && nWidth != aFrm.Width() )
        {
            aFrm.Pos().setX(aFrm.Pos().getX() + aFrm.Width() - nWidth);
        }

        aFrm.Width( nWidth );
    }

    bool bNoGrid = true;
    if( GetUpper()->IsPageFrame() && static_cast<SwPageFrame*>(GetUpper())->HasGrid() )
    {
        SwTextGridItem const*const pGrid(
                GetGridItem(static_cast<SwPageFrame*>(GetUpper())));
        if( pGrid )
        {
            bNoGrid = false;
            tools::Long nSum = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
            SwRectFnSet aRectFnSet(this);
            tools::Long nSize = aRectFnSet.GetWidth(getFrameArea());
            tools::Long nBorder = 0;
            if( GRID_LINES_CHARS == pGrid->GetGridType() )
            {
                //for textgrid refactor
                SwDoc *pDoc = GetFormat()->GetDoc();
                nBorder = nSize % (GetGridWidth(*pGrid, *pDoc));
                nSize -= nBorder;
                nBorder /= 2;
            }

            SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
            aRectFnSet.SetPosX( aPrt, nBorder );
            aRectFnSet.SetWidth( aPrt, nSize );

            // Height of body frame:
            nBorder = aRectFnSet.GetHeight(getFrameArea());

            // Number of possible lines in area of body frame:
            tools::Long nNumberOfLines = nBorder / nSum;
            if( nNumberOfLines > pGrid->GetLines() )
                nNumberOfLines = pGrid->GetLines();

            // Space required for nNumberOfLines lines:
            nSize = nNumberOfLines * nSum;
            nBorder -= nSize;
            nBorder /= 2;

            // #i21774# Footnotes and centering the grid does not work together:
            const bool bAdjust = static_cast<SwPageFrame*>(GetUpper())->GetFormat()->GetDoc()->
                                        GetFootnoteIdxs().empty();

            aRectFnSet.SetPosY( aPrt, bAdjust ? nBorder : 0 );
            aRectFnSet.SetHeight( aPrt, nSize );
        }
    }

    if( bNoGrid )
    {
        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
        aPrt.Pos().setX(0);
        aPrt.Pos().setY(0);
        aPrt.Height( getFrameArea().Height() );
        aPrt.Width( getFrameArea().Width() );
    }

    setFrameAreaSizeValid(true);
    setFramePrintAreaValid(true);
}

SwPageFrame::SwPageFrame( SwFrameFormat *pFormat, SwFrame* pSib, SwPageDesc *pPgDsc ) :
    SwFootnoteBossFrame( pFormat, pSib ),
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
    mnFrameType = SwFrameType::Page;
    m_bInvalidLayout = m_bInvalidContent = m_bInvalidSpelling = m_bInvalidSmartTags = m_bInvalidAutoCmplWrds = m_bInvalidWordCount = true;
    m_bInvalidFlyLayout = m_bInvalidFlyContent = m_bInvalidFlyInCnt = m_bFootnotePage = m_bEndNotePage = false;

    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    vcl::RenderContext* pRenderContext = pSh ? pSh->GetOut() : nullptr;

    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);

        if ( bBrowseMode )
        {
            aFrm.Height( 0 );
            tools::Long nWidth = pSh->VisArea().Width();

            if ( !nWidth )
            {
                nWidth = 5000;     // changes anyway
            }

            aFrm.Width ( nWidth );
        }
        else
        {
            aFrm.SSize( pFormat->GetFrameSize().GetSize() );
        }
    }

    // create and insert body area if it is not a blank page
    SwDoc* pDoc(pFormat->GetDoc());
    m_bEmptyPage = (pFormat == pDoc->GetEmptyPageFormat());

    if(m_bEmptyPage)
    {
        return;
    }

    Calc(pRenderContext); // so that the PrtArea is correct
    SwBodyFrame *pBodyFrame = new SwBodyFrame( pDoc->GetDfltFrameFormat(), this );
    pBodyFrame->ChgSize( getFramePrintArea().SSize() );
    pBodyFrame->Paste( this );
    pBodyFrame->Calc(pRenderContext); // so that the columns can be inserted correctly
    pBodyFrame->InvalidatePos();

    if ( bBrowseMode )
        InvalidateSize_();

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
    if (m_pSortedObjs)
    {
        // Objects can be anchored at pages that are before their anchors (why ever...).
        // In such cases, we would access already freed memory.
        for (SwAnchoredObject* pAnchoredObj : *m_pSortedObjs)
        {
            pAnchoredObj->SetPageFrame( nullptr );
        }
        m_pSortedObjs.reset(); // reset to zero to prevent problems when detaching the Flys
    }

    // prevent access to destroyed pages
    SwDoc *pDoc = GetFormat() ? GetFormat()->GetDoc() : nullptr;
    if( pDoc && !pDoc->IsInDtor() )
    {
        if ( pSh )
        {
            SwViewShellImp *pImp = pSh->Imp();
            pImp->SetFirstVisPageInvalid();
            if ( pImp->IsAction() )
                pImp->GetLayAction().SetAgain(true);
            // #i9719# - retouche area of page
            // including border and shadow area.
            const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
            SwRect aRetoucheRect;
            SwPageFrame::GetBorderAndShadowBoundRect( getFrameArea(), pSh, pSh->GetOut(), aRetoucheRect, IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
            pSh->AddPaintRect( aRetoucheRect );
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
    if( !(bInvalidate || bOld != m_bHasGrid) )
        return;

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

void SwPageFrame::CheckDirection( bool bVert )
{
    SvxFrameDirection nDir = GetFormat()->GetFormatAttr( RES_FRAMEDIR ).GetValue();
    if( bVert )
    {
        if( SvxFrameDirection::Horizontal_LR_TB == nDir || SvxFrameDirection::Horizontal_RL_TB == nDir )
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

                if(SvxFrameDirection::Vertical_RL_TB == nDir)
                    mbVertLR = false;
                else if(SvxFrameDirection::Vertical_LR_TB==nDir)
                    mbVertLR = true;
            }
        }

        mbInvalidVert = false;
    }
    else
    {
        if( SvxFrameDirection::Horizontal_RL_TB == nDir )
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
        const SwFrameType nTypes = SwFrameType::Root | SwFrameType::Page | SwFrameType::Column
                           | SwFrameType::Header | SwFrameType::Footer | SwFrameType::FtnCont
                           | SwFrameType::Ftn | SwFrameType::Body;
        if ( pTmp->GetType() & nTypes )
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
                if (RndStdIds::FLY_AT_PAGE == rAnch.GetAnchorId())
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
                if ( auto pDrawVirtObj = dynamic_cast<SwDrawVirtObj *>( pSdrObj ) )
                {
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
    else if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
        if(auto pSh = getRootFrame()->GetCurrShell())
            pSh->SetFirstVisPageInvalid();

        SwPageFrameInvFlags eInvFlags = SwPageFrameInvFlags::NONE;
        if(pLegacy->m_pNew && RES_ATTRSET_CHG == pLegacy->m_pNew->Which())
        {
            auto& rOldSetChg = *static_cast<const SwAttrSetChg*>(pLegacy->m_pOld);
            auto& rNewSetChg = *static_cast<const SwAttrSetChg*>(pLegacy->m_pNew);
            SfxItemIter aOIter(*rOldSetChg.GetChgSet());
            SfxItemIter aNIter(*rNewSetChg.GetChgSet());
            const SfxPoolItem* pOItem = aOIter.GetCurItem();
            const SfxPoolItem* pNItem = aNIter.GetCurItem();
            SwAttrSetChg aOldSet(rOldSetChg);
            SwAttrSetChg aNewSet(rNewSetChg);
            do
            {
                UpdateAttr_(pOItem, pNItem, eInvFlags, &aOldSet, &aNewSet);
                pOItem = aOIter.NextItem();
                pNItem = aNIter.NextItem();
            } while(pNItem);
            if(aOldSet.Count() || aNewSet.Count())
                SwLayoutFrame::SwClientNotify(rModify, sw::LegacyModifyHint(&aOldSet, &aNewSet));
        }
        else
            UpdateAttr_(pLegacy->m_pOld, pLegacy->m_pNew, eInvFlags);

        if (eInvFlags == SwPageFrameInvFlags::NONE)
            return;

        InvalidatePage( this );
        if(eInvFlags & SwPageFrameInvFlags::InvalidatePrt)
            InvalidatePrt_();
        if(eInvFlags & SwPageFrameInvFlags::SetCompletePaint)
            SetCompletePaint();
        if(eInvFlags & SwPageFrameInvFlags::InvalidateNextPos && GetNext() )
            GetNext()->InvalidatePos();
        if(eInvFlags & SwPageFrameInvFlags::PrepareHeader)
            PrepareHeader();
        if(eInvFlags & SwPageFrameInvFlags::PrepareFooter)
            PrepareFooter();
        if(eInvFlags & SwPageFrameInvFlags::CheckGrid)
            CheckGrid(bool(eInvFlags & SwPageFrameInvFlags::InvalidateGrid));
    } else
        SwFrame::SwClientNotify(rModify, rHint);
}

void SwPageFrame::UpdateAttr_( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                             SwPageFrameInvFlags &rInvFlags,
                             SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_FMT_CHG:
        {
            // state of m_bEmptyPage needs to be determined newly
            const bool bNewState(GetFormat() == GetFormat()->GetDoc()->GetEmptyPageFormat());

            if(m_bEmptyPage != bNewState)
            {
                // copy new state
                m_bEmptyPage = bNewState;

                if(nullptr == GetLower())
                {
                    // if we were an empty page before there is not yet a BodyArea in the
                    // form of a SwBodyFrame, see constructor
                    SwViewShell* pSh(getRootFrame()->GetCurrShell());
                    vcl::RenderContext* pRenderContext(pSh ? pSh->GetOut() : nullptr);
                    Calc(pRenderContext); // so that the PrtArea is correct
                    SwBodyFrame* pBodyFrame = new SwBodyFrame(GetFormat(), this);
                    pBodyFrame->ChgSize(getFramePrintArea().SSize());
                    pBodyFrame->Paste(this);
                    pBodyFrame->InvalidatePos();
                }
            }

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
                rInvFlags |= SwPageFrameInvFlags::CheckGrid;
            }

            // 2. header and footer:
            const SwFormatHeader &rOldH = pOldFormat->GetHeader();
            const SwFormatHeader &rNewH = pNewFormat->GetHeader();
            if( rOldH != rNewH )
                rInvFlags |= SwPageFrameInvFlags::PrepareHeader;

            const SwFormatFooter &rOldF = pOldFormat->GetFooter();
            const SwFormatFooter &rNewF = pNewFormat->GetFooter();
            if( rOldF != rNewF )
                rInvFlags |= SwPageFrameInvFlags::PrepareFooter;
            CheckDirChange();

            [[fallthrough]];
        }
        case RES_FRM_SIZE:
        {
            const SwRect aOldPageFrameRect( getFrameArea() );
            SwViewShell *pSh = getRootFrame()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                setFrameAreaSizeValid(false);
                // OD 28.10.2002 #97265# - Don't call <SwPageFrame::MakeAll()>
                // Calculation of the page is not necessary, because its size is
                // invalidated here and further invalidation is done in the
                // calling method <SwPageFrame::Modify(..)> and probably by calling
                // <SwLayoutFrame::SwClientNotify(..)> at the end.
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

                {
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                    aFrm.Height( std::max( rSz.GetHeight(), tools::Long(MINLAY) ) );
                    aFrm.Width ( std::max( rSz.GetWidth(),  tools::Long(MINLAY) ) );
                }

                if ( GetUpper() )
                {
                    static_cast<SwRootFrame*>(GetUpper())->CheckViewLayout( nullptr, nullptr );
                }
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
            rInvFlags |= SwPageFrameInvFlags::InvalidatePrt | SwPageFrameInvFlags::SetCompletePaint;
            if ( aOldPageFrameRect.Height() != getFrameArea().Height() )
                rInvFlags |= SwPageFrameInvFlags::InvalidateNextPos;
        }
        break;

        case RES_COL:
            assert(pOld && pNew); //COL Missing Format
            if (pOld && pNew)
            {
                SwLayoutFrame *pB = FindBodyCont();
                assert(pB); //page without body
                pB->ChgColumns( *static_cast<const SwFormatCol*>(pOld), *static_cast<const SwFormatCol*>(pNew) );
                rInvFlags |= SwPageFrameInvFlags::SetCompletePaint | SwPageFrameInvFlags::CheckGrid;
            }
        break;

        case RES_HEADER:
            rInvFlags |= SwPageFrameInvFlags::PrepareHeader;
            break;

        case RES_FOOTER:
            rInvFlags |= SwPageFrameInvFlags::PrepareFooter;
            break;
        case RES_TEXTGRID:
            rInvFlags |= SwPageFrameInvFlags::CheckGrid | SwPageFrameInvFlags::InvalidateGrid;
            break;
        case RES_FRAMEDIR :
            CheckDirChange();
            break;

        default:
            bClear = false;
    }
    if ( !bClear )
        return;

    if ( pOldSet || pNewSet )
    {
        if ( pOldSet )
            pOldSet->ClearItem( nWhich );
        if ( pNewSet )
            pNewSet->ClearItem( nWhich );
    }
    else
    {
        SwModify aMod;
        SwLayoutFrame::SwClientNotify(aMod, sw::LegacyModifyHint(pOld, pNew));
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
            pRet = const_cast<SwPageDesc*>(pFlow->GetPageDescItem().GetPageDesc());
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
            pRet = const_cast<SwPageDesc*>(pFlow->GetPageDescItem().GetPageDesc());
    }

    //3. and 3.1
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
                    rSh.Imp()->NotifySizeChg( pRoot->getFrameArea().SSize() );
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

                if ( auto pFly = dynamic_cast<SwFlyAtContentFrame *>( pAnchoredObj ) )
                {
                    SwPageFrame *pAnchPage = pFly->GetAnchorFrame() ?
                                pFly->AnchorFrame()->FindPageFrame() : nullptr;
                    if ( pAnchPage && (pAnchPage != this) )
                    {
                        MoveFly( pFly, pAnchPage );
                        pFly->InvalidateSize();
                        pFly->InvalidatePos_();
                        // Do not increment index, in this case
                        continue;
                    }
                }
                ++i;
            }
        }
        // cleanup Window
        if ( pSh && pSh->GetWin() )
            pSh->InvalidateWindows( getFrameArea() );
    }

    // decrease the root's page number
    static_cast<SwRootFrame*>(GetUpper())->DecrPhyPageNums();
    SwPageFrame *pPg = static_cast<SwPageFrame*>(GetNext());
    if ( pPg )
    {
        while ( pPg )
        {
            --pPg->m_nPhyPageNum;
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
            ++pPg->m_nPhyPageNum;
            pPg->InvalidatePos_();
            pPg->InvalidateLayout();
            pPg = static_cast<SwPageFrame*>(pPg->GetNext());
        }
    }
    else
        ::SetLastPage( this );

    if( getFrameArea().Width() != pParent->getFramePrintArea().Width() )
        InvalidateSize_();

    InvalidatePos();

    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();

    getRootFrame()->CheckViewLayout( nullptr, nullptr );
}

static void lcl_PrepFlyInCntRegister( SwContentFrame *pFrame )
{
    pFrame->Prepare( PrepareHint::Register );
    if( !pFrame->GetDrawObjs() )
        return;

    for(SwAnchoredObject* pAnchoredObj : *pFrame->GetDrawObjs())
    {
        // #i28701#
        if ( auto pFly = dynamic_cast<SwFlyInContentFrame *>( pAnchoredObj ) )
        {
            SwContentFrame *pCnt = pFly->ContainsContent();
            while ( pCnt )
            {
                lcl_PrepFlyInCntRegister( pCnt );
                pCnt = pCnt->GetNextContentFrame();
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
    if( !GetSortedObjs() )
        return;

    for(SwAnchoredObject* pAnchoredObj : *GetSortedObjs())
    {
        // #i28701#
        if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
        {
            pFrame = pFly->ContainsContent();
            while ( pFrame )
            {
                ::lcl_PrepFlyInCntRegister( pFrame );
                pFrame = pFrame->GetNextContentFrame();
            }
        }
    }
}

namespace sw {

/// check if there's content on the page that requires it to exist
bool IsPageFrameEmpty(SwPageFrame const& rPage)
{
    bool bExistEssentialObjs = (nullptr != rPage.GetSortedObjs());
    if (bExistEssentialObjs)
    {
        // Only because the page has Flys does not mean that it is needed. If all Flys are
        // attached to generic content it is also superfluous (checking DocBody should be enough)
        // OD 19.06.2003 - consider that drawing objects in
        // header/footer are supported now.
        bool bOnlySuperfluousObjs = true;
        SwSortedObjs const& rObjs = *rPage.GetSortedObjs();
        for (size_t i = 0; bOnlySuperfluousObjs && i < rObjs.size(); ++i)
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = rObjs[i];
            // do not consider hidden objects
            if ( rPage.GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(
                                pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                 !pAnchoredObj->GetAnchorFrame()->FindFooterOrHeader() )
            {
                bOnlySuperfluousObjs = false;
            }
        }
        bExistEssentialObjs = !bOnlySuperfluousObjs;
    }

    // optimization: check first if essential objects exist.
    const SwLayoutFrame* pBody = nullptr;
    if ( bExistEssentialObjs ||
         rPage.FindFootnoteCont() ||
         (nullptr != (pBody = rPage.FindBodyCont()) &&
            ( pBody->ContainsContent() ||
                // #i47580#
                // Do not delete page if there's an empty tabframe
                // left. I think it might be correct to use ContainsAny()
                // instead of ContainsContent() to cover the empty-table-case,
                // but I'm not fully sure, since ContainsAny() also returns
                // SectionFrames. Therefore I prefer to do it the safe way:
              ( pBody->Lower() && pBody->Lower()->IsTabFrame() ) ) ) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

} // namespace sw

//FIXME: provide missing documentation
/** Check all pages (starting from the given one) if they use the appropriate frame format.
 *
 * If "wrong" pages are found, try to fix this as simple as possible.
 *
 * Also delete pages that don't have content on them.
 *
 * @param pStart        the page from where to start searching
 * @param bNotifyFields
 * @param ppPrev
 */
void SwFrame::CheckPageDescs( SwPageFrame *pStart, bool bNotifyFields, SwPageFrame** ppPrev )
{
    SAL_INFO( "sw.pageframe", "(CheckPageDescs in phy: " << pStart->GetPhyPageNum() );
    assert(pStart && "no starting page.");

    SwViewShell *pSh   = pStart->getRootFrame()->GetCurrShell();
    SwViewShellImp *pImp  = pSh ? pSh->Imp() : nullptr;

    if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
    {
        pImp->GetLayAction().SetCheckPageNum( pStart->GetPhyPageNum() );
        SAL_INFO( "sw.pageframe", "CheckPageDescs out fast - via SetCheckPageNum: "
                  << pStart->GetPhyPageNum() << ")" );
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
        SwPageFrame *pPrevPage = static_cast<SwPageFrame*>(pPage->GetPrev());
        SwPageFrame *pNextPage = static_cast<SwPageFrame*>(pPage->GetNext());

        SwPageDesc *pDesc = pPage->FindPageDesc();
        /// page is intentionally empty page
        bool bIsEmpty = pPage->IsEmptyPage();
        // false for intentionally empty pages, they need additional check
        bool isPageFrameEmpty(!bIsEmpty && sw::IsPageFrameEmpty(*pPage));
        bool bIsOdd = pPage->OnRightPage();
        bool bWantOdd = pPage->WannaRightPage();
        bool bFirst = pPage->OnFirstPage();
        SwFrameFormat *pFormatWish = bWantOdd
            ? pDesc->GetRightFormat(bFirst) : pDesc->GetLeftFormat(bFirst);

        if ( bIsOdd != bWantOdd ||
             pDesc != pPage->GetPageDesc() ||        // wrong Desc
             ( pFormatWish != pPage->GetFormat() &&       // wrong format and
               ( !bIsEmpty || pFormatWish ) // not blank /empty
             )
           )
        {
            // Updating a page might take a while, so check the WaitCursor
            if( pImp )
                pImp->CheckWaitCursor();

            // invalidate the field, starting from here
            if ( nDocPos == LONG_MAX )
                nDocPos = pPrevPage ? pPrevPage->getFrameArea().Top() : pPage->getFrameArea().Top();

            // Cases:
            //  1. Empty page should be "normal" page -> remove empty page and take next one
            //  2. Empty page should have different descriptor -> change
            //  3. Normal page should be empty -> insert empty page if previous page
            //     is not empty, otherwise see (6).
            //  4. Normal page should have different descriptor -> change
            //  5. Normal page should have different format -> change
            //  6. No "wish" format provided -> take the "other" format (left/right) of the PageDesc

            if ( bIsEmpty && ( pFormatWish ||          //1.
                 ( !bWantOdd && !pPrevPage ) ) )
            {
                // Check all cases for the next page, so we don't oscillate empty pages
                // Skip case 1 and 2, as we require a non-empty next page to save the empty page
                // Case 3 is the one we actually want to predict and skip
                // We can skip the empty check of case 3, as we just work on an existing next page
                bool bNextWantOdd;
                SwPageDesc *pNextDesc;
                if ( pNextPage && !pNextPage->IsEmptyPage() &&    //3.
                     pNextPage->OnRightPage() == (bNextWantOdd = pNextPage->WannaRightPage()) &&
                     pNextPage->GetPageDesc() == (pNextDesc = pNextPage->FindPageDesc()) )   //4.
                {
                    bool bNextFirst = pNextPage->OnFirstPage();
                    SwFrameFormat *pNextFormatWish = bNextWantOdd ?   //5.
                        pNextDesc->GetRightFormat(bNextFirst) : pNextDesc->GetLeftFormat(bNextFirst);
                    if ( !pNextFormatWish )    // 6.
                        pNextFormatWish = bNextWantOdd ? pNextDesc->GetLeftFormat() : pNextDesc->GetRightFormat();
                    if ( pNextFormatWish && pNextPage->GetFormat() == pNextFormatWish )
                    {
                        SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                                  << " c: 1+3 - skip next page of p: " << pPage );
                        if (pPrevPage && pPage->GetPageDesc() != pPrevPage->GetPageDesc())
                            pPage->SetPageDesc( pPrevPage->GetPageDesc(), nullptr );
                        // We can skip the next page, as all checks were already done!
                        pPage = static_cast<SwPageFrame*>(pNextPage->GetNext());
                        continue;
                    }
                }

                pPage->Cut();
                bool bUpdatePrev = false;
                if (ppPrev && *ppPrev == pPage)
                    bUpdatePrev = true;
                SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                          << " c: 1 - destroy p: " << pPage );
                SwFrame::DestroyFrame(pPage);
                if ( pStart == pPage )
                    pStart = pNextPage;
                pPage = pNextPage;
                if (bUpdatePrev)
                    *ppPrev = pNextPage;
                continue;
            }
            else if ( bIsEmpty && !pFormatWish &&  //2.
                      pDesc != pPage->GetPageDesc() )
            {
                SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                          << " c: 2 - set desc p: " << pPage << " d: " << pDesc );
                pPage->SetPageDesc( pDesc, nullptr );
            }
            else if ( !bIsEmpty &&      //3.
                      bIsOdd != bWantOdd &&
                      ( ( !pPrevPage && !bWantOdd ) ||
                        ( pPrevPage && !pPrevPage->IsEmptyPage() )
                      )
                    )
            {
                if ( pPrevPage )
                    pDesc = pPrevPage->GetPageDesc();
                SwPageFrame *pTmp = new SwPageFrame( pDoc->GetEmptyPageFormat(), pRoot, pDesc );
                SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                          << " c: 3 - insert empty p: " << pTmp << " d: " << pDesc );
                pTmp->Paste( pRoot, pPage );
                pTmp->PreparePage( false );
                pPage = pTmp;
                isPageFrameEmpty = false; // don't delete it right away!
            }
            else if ( pPage->GetPageDesc() != pDesc )           //4.
            {
                SwPageDesc *pOld = pPage->GetPageDesc();
                pPage->SetPageDesc( pDesc, pFormatWish );
                SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                          << " c: 4 - set desc + format p: " << pPage
                          << " d: " << pDesc << " f: " << pFormatWish );
                if ( bFootnotes )
                {
                    // If specific values of the FootnoteInfo are changed, something has to happen.
                    // We try to limit the damage...
                    // If the page has no FootnoteCont it might be problematic.
                    // Let's hope that invalidation is enough.
                    SwFootnoteContFrame *pCont = pPage->FindFootnoteCont();
                    if ( pCont && !(pOld->GetFootnoteInfo() == pDesc->GetFootnoteInfo()) )
                        pCont->InvalidateAll_();
                }
            }
            else if ( pFormatWish && pPage->GetFormat() != pFormatWish )         //5.
            {
                pPage->SetFrameFormat( pFormatWish );
                SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                          << " c: 5 - set format p: " << pPage << " f: " << pFormatWish );
            }
            else if ( !pFormatWish )                                       //6.
            {
                // get format with inverted logic
                pFormatWish = bWantOdd ? pDesc->GetLeftFormat() : pDesc->GetRightFormat();
                if ( pFormatWish && pPage->GetFormat() != pFormatWish )
                {
                    pPage->SetFrameFormat( pFormatWish );
                    SAL_INFO( "sw.pageframe", "CheckPageDescs phys: " << pPage->GetPhyPageNum()
                              << " c: 6 - set format p: " << pPage << " f: " << pFormatWish );
                }
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "CheckPageDescs, missing solution" );
            }
#endif
        }
        assert(!bIsEmpty || !isPageFrameEmpty);
        const bool bWantRemovePage = bIsEmpty || isPageFrameEmpty;
        if (bWantRemovePage && !pPage->IsDeleteForbidden())
        {
            // It also might be that an empty page is not needed at all.
            // However, the algorithm above cannot determine that. It is not needed if the following
            // page can live without it. Do obtain that information, we need to dig deeper...
            SwPageFrame *pPg = static_cast<SwPageFrame*>(pPage->GetNext());
            if (isPageFrameEmpty || !pPg || pPage->OnRightPage() == pPg->WannaRightPage())
            {
                // The following page can find a FrameFormat or has no successor -> empty page not needed
                SwPageFrame *pTmp = static_cast<SwPageFrame*>(pPage->GetNext());
                if (isPageFrameEmpty && pPage->GetPrev())
                {   // check previous *again* vs. its new next! see "ooo321_stylepagenumber.odt"
                    pTmp = static_cast<SwPageFrame*>(pPage->GetPrev());
                }
                pPage->Cut();
                bool bUpdatePrev = false;
                if (ppPrev && *ppPrev == pPage)
                    bUpdatePrev = true;
                SwFrame::DestroyFrame(pPage);
                SAL_INFO( "sw.pageframe", "CheckPageDescs - handle bIsEmpty - destroy p: " << pPage );
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
    SAL_INFO( "sw.pageframe", "CheckPageDescs out)" );
}

namespace
{
    bool isDeleteForbidden(const SwPageFrame *pDel)
    {
        if (pDel->IsDeleteForbidden())
            return true;
        const SwLayoutFrame* pBody = pDel->FindBodyCont();
        const SwFrame* pBodyContent = pBody ? pBody->Lower() : nullptr;
        return pBodyContent && pBodyContent->IsDeleteForbidden();
    }

    bool doInsertPage( SwRootFrame *pRoot, SwPageFrame **pRefSibling,
                       SwFrameFormat *pFormat, SwPageDesc *pDesc,
                       bool bFootnote, SwPageFrame **pRefPage )
    {
        SwPageFrame *pPage = new SwPageFrame(pFormat, pRoot, pDesc);
        SwPageFrame *pSibling = *pRefSibling;
        if ( pRefPage )
        {
            *pRefPage = pPage;
            SAL_INFO( "sw.pageframe", "doInsertPage p: " << pPage
                                      << " d: " << pDesc << " f: " << pFormat );
        }
        else
            SAL_INFO( "sw.pageframe", "doInsertPage - insert empty p: "
                                      << pPage << " d: " << pDesc );
        pPage->Paste( pRoot, pSibling );

        SwViewShell* pViewShell = pRoot->GetCurrShell();
        if (pViewShell && pViewShell->GetViewOptions()->IsHideWhitespaceMode())
        {
            // Hide-whitespace mode does not shrink the last page, so resize the page that used to
            // be the last one.
            if (SwFrame* pPrevPage = pPage->GetPrev())
            {
                pPrevPage->InvalidateSize();
            }
        }

        pPage->PreparePage( bFootnote );
        // If the sibling has no body text, destroy it as long as it is no footnote page.
        if ( pSibling && !pSibling->IsFootnotePage() &&
             !pSibling->FindFirstBodyContent() &&
             (!pRefPage || !isDeleteForbidden(pSibling)) )
        {
            pRoot->RemovePage( pRefSibling, SwRemoveResult::Next ) ;
            return false;
        }
        return true;
    }
}

SwPageFrame *SwFrame::InsertPage( SwPageFrame *pPrevPage, bool bFootnote )
{
    SwRootFrame *pRoot = static_cast<SwRootFrame*>(pPrevPage->GetUpper());
    SwPageFrame *pSibling = static_cast<SwPageFrame*>(pPrevPage->GetNext());
    SwPageDesc *pDesc = nullptr;

    // insert right (odd) or left (even) page?
    bool bNextRightPage = !pPrevPage->OnRightPage();
    bool bWishedRightPage = bNextRightPage;

    // Which PageDesc is relevant?
    // For ContentFrame take the one from format if provided,
    // otherwise from the Follow of the PrevPage
    if ( IsFlowFrame() && !SwFlowFrame::CastFlowFrame( this )->IsFollow() )
    {
        SwFormatPageDesc &rDesc = const_cast<SwFormatPageDesc&>(GetPageDescItem());
        pDesc = rDesc.GetPageDesc();
        if ( rDesc.GetNumOffset() )
        {
            ::std::optional<sal_uInt16> oNumOffset = rDesc.GetNumOffset();
            bWishedRightPage = sw::IsRightPageByNumber(*pRoot, *oNumOffset);
            // use the opportunity to set the flag at root
            pRoot->SetVirtPageNum( true );
        }
    }
    if ( !pDesc )
        pDesc = pPrevPage->GetPageDesc()->GetFollow();

    assert(pDesc && "Missing PageDesc");
    if( !(bWishedRightPage ? pDesc->GetRightFormat() : pDesc->GetLeftFormat()) )
        bWishedRightPage = !bWishedRightPage;
    bool const bWishedFirst = pDesc != pPrevPage->GetPageDesc();

    SwDoc *pDoc = pPrevPage->GetFormat()->GetDoc();
    bool bCheckPages = false;
    // If there is no FrameFormat for this page, create an empty page.
    if (bWishedRightPage != bNextRightPage)
    {
        if( doInsertPage( pRoot, &pSibling, pDoc->GetEmptyPageFormat(),
                          pPrevPage->GetPageDesc(), bFootnote, nullptr ) )
            bCheckPages = true;
    }
    SwFrameFormat *const pFormat( bWishedRightPage
            ? pDesc->GetRightFormat(bWishedFirst)
            : pDesc->GetLeftFormat(bWishedFirst) );
    assert(pFormat);
    SwPageFrame *pPage = nullptr;
    if( doInsertPage( pRoot, &pSibling, pFormat, pDesc, bFootnote, &pPage ) )
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
                {
                    pImp->GetLayAction().SetCheckPageNumDirect(
                                                    pSibling->GetPhyPageNum() );
                    SAL_INFO( "sw.pageframe", "InsertPage - SetCheckPageNumDirect: "
                              << pSibling->GetPhyPageNum() );
                }
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
        SwDocPosUpdate aMsgHint( pPrevPage->getFrameArea().Top() );
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
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.AddHeight(nDist );
    }

    return nDist;
}

SwTwips SwRootFrame::ShrinkFrame( SwTwips nDist, bool bTst, bool )
{
    OSL_ENSURE( nDist >= 0, "nDist < 0." );
    OSL_ENSURE( nDist <= getFrameArea().Height(), "nDist greater than current size." );

    if ( !bTst )
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.AddHeight( -nDist );
    }

    return nDist;
}

void SwRootFrame::RemovePage( SwPageFrame **pDelRef, SwRemoveResult eResult )
{
    SwPageFrame *pDel = *pDelRef;
    (*pDelRef) = static_cast<SwPageFrame*>(
        eResult == SwRemoveResult::Next ? pDel->GetNext() : pDel->GetPrev() );
    if ( !GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
        RemoveFootnotes( pDel, true );
    pDel->Cut();
    SwFrame::DestroyFrame( pDel );
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
    tools::Long nDocPos = LONG_MAX;

    // Check the corresponding last page if it is empty and stop loop at the last non-empty page.
    do
    {
        if (!sw::IsPageFrameEmpty(*pPage))
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
            SAL_INFO( "sw.pageframe", "RemoveSuperfluous - DestroyFrm p: " << pPage );
            RemovePage( &pPage, SwRemoveResult::Prev );
            nDocPos = pPage ? pPage->getFrameArea().Top() : 0;
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
    // note the needed pages in a set
    sal_uInt16 nMaxPg(0);
    o3tl::sorted_vector< sal_uInt16 > neededPages;
    neededPages.reserve(pTable->size());

    for ( size_t i = 0; i < pTable->size(); ++i )
    {
        const SwFormatAnchor &rAnch = (*pTable)[i]->GetAnchor();
        if(!rAnch.GetContentAnchor())
        {
            const sal_uInt16 nPageNum(rAnch.GetPageNum());

            // calc MaxPage (as before)
            nMaxPg = std::max(nMaxPg, nPageNum);

            // note as needed page
            neededPages.insert(nPageNum);
        }
    }

    // How many pages exist at the moment?
    // And are there EmptyPages that are needed?
    SwPageFrame* pPage(static_cast<SwPageFrame*>(Lower()));
    SwPageFrame* pPrevPage(nullptr);
    SwPageFrame* pFirstRevivedEmptyPage(nullptr);

    while(pPage) // moved two while-conditions to break-statements (see below)
    {
        const sal_uInt16 nPageNum(pPage->GetPhyPageNum());

        if(pPage->IsEmptyPage() &&
            nullptr != pPrevPage &&
            neededPages.find(nPageNum) != neededPages.end())
        {
            // This is an empty page, but it *is* needed since a SwFrame
            // is anchored at it directly. Initially these SwFrames are
            // not fully initialized. Need to change the format of this SwFrame
            // and let the ::Notify mechanism newly evaluate
            // m_bEmptyPage (see SwPageFrame::UpdateAttr_). Code is taken and
            // adapted from ::InsertPage (used below), this needs previous page
            bool bWishedRightPage(!pPrevPage->OnRightPage());
            SwPageDesc* pDesc(pPrevPage->GetPageDesc()->GetFollow());
            assert(pDesc && "Missing PageDesc");

            if (!(bWishedRightPage ? pDesc->GetRightFormat() : pDesc->GetLeftFormat()))
            {
                bWishedRightPage = !bWishedRightPage;
            }

            bool const bWishedFirst(pDesc != pPrevPage->GetPageDesc());
            SwFrameFormat* pFormat(bWishedRightPage ? pDesc->GetRightFormat(bWishedFirst) : pDesc->GetLeftFormat(bWishedFirst));

            // set SwFrameFormat, this will trigger SwPageFrame::UpdateAttr_ and re-evaluate
            // m_bEmptyPage, too
            pPage->SetFrameFormat(pFormat);

            if(nullptr == pFirstRevivedEmptyPage)
            {
                // remember first (lowest) SwPageFrame which needed correction
                pFirstRevivedEmptyPage = pPage;
            }
        }

        // original while-condition II
        if(nullptr == pPage->GetNext())
        {
            break;
        }

        // original while-condition III
        if(static_cast< SwPageFrame* >(pPage->GetNext())->IsFootnotePage())
        {
            break;
        }

        pPrevPage = pPage;
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }

    if ( nMaxPg > pPage->GetPhyPageNum() )
    {
        for ( sal_uInt16 i = pPage->GetPhyPageNum(); i < nMaxPg; ++i )
            pPage = InsertPage( pPage, false );

        // If the endnote pages are now corrupt, destroy them.
        if ( !pDoc->GetFootnoteIdxs().empty() )
        {
            pPage = static_cast<SwPageFrame*>(Lower());
            while ( pPage && !pPage->IsFootnotePage() )
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());

            if ( pPage )
            {
                SwPageDesc *pTmpDesc = pPage->FindPageDesc();
                bool isRightPage = pPage->OnRightPage();
                if ( pPage->GetFormat() !=
                     (isRightPage ? pTmpDesc->GetRightFormat() : pTmpDesc->GetLeftFormat()) )
                    RemoveFootnotes( pPage, false, true );
            }
        }
    }

    // if we corrected SwFrameFormat and changed one (or more) m_bEmptyPage
    // flags, we need to correct evtl. currently wrong positioned SwFrame(s)
    // which did think until now that these Page(s) are empty.
    // After trying to correct myself I found SwRootFrame::AssertPageFlys
    // directly below that already does that, so use it.
    if(nullptr != pFirstRevivedEmptyPage)
    {
        AssertPageFlys(pFirstRevivedEmptyPage);
    }

    //Remove masters that haven't been replaced yet from the list.
    RemoveMasterObjs( mpDrawPage );

#if OSL_DEBUG_LEVEL > 0
    pPage = static_cast<SwPageFrame*>(Lower());
    while ( pPage && pPage->GetNext() &&
            !static_cast<SwPageFrame*>(pPage->GetNext())->IsFootnotePage() )
    {
        SAL_INFO( "sw.pageframe",  "AssertFlyPages p: " << pPage << " d: " << pPage->GetPageDesc()
                   << " f: " << pPage->GetFormat() << " virt: " << pPage->GetVirtPageNum()
                   << " phys: " << pPage->GetPhyPageNum() << " empty: " << pPage->IsEmptyPage() );
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }
    SAL_INFO( "sw.pageframe", "AssertFlyPages p: " << pPage << " d: " << pPage->GetPageDesc()
              << " f: " << pPage->GetFormat() << " virt: " << pPage->GetVirtPageNum()
              << " phys: " << pPage->GetPhyPageNum() << " empty: " << pPage->IsEmptyPage() );
#endif
}

/// Ensure that after the given page all page-bound objects are located on the correct page
void SwRootFrame::AssertPageFlys( SwPageFrame *pPage )
{
    SAL_INFO( "sw.pageframe", "(AssertPageFlys in" );
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
                if ((rAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE) &&
                     nPg != pPage->GetPhyPageNum() )
                {
                    SAL_INFO( "sw.pageframe", nPg << " " << pPage->GetPhyPageNum() );
                    // If on the wrong page, check if previous page is empty
                    if( nPg && !(pPage->GetPhyPageNum()-1 == nPg &&
                        static_cast<SwPageFrame*>(pPage->GetPrev())->IsEmptyPage()) )
                    {
                        // It can move by itself. Just send a modify to its anchor attribute.
#if OSL_DEBUG_LEVEL > 1
                        const size_t nCnt = pPage->GetSortedObjs()->size();
                        rFormat.CallSwClientNotify(sw::LegacyModifyHint(nullptr, &rAnch));
                        OSL_ENSURE( !pPage->GetSortedObjs() ||
                                nCnt != pPage->GetSortedObjs()->size(),
                                "Object couldn't be reattached!" );
#else
                        rFormat.CallSwClientNotify(sw::LegacyModifyHint(nullptr, &rAnch));
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
    SAL_INFO( "sw.pageframe", "AssertPageFlys out)" );
}

Size SwRootFrame::ChgSize( const Size& aNewSize )
{
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.SSize(aNewSize);
    }

    InvalidatePrt_();
    mbFixSize = false;
    return getFrameArea().SSize();
}

void SwRootFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( !isFrameAreaPositionValid() )
    {
        setFrameAreaPositionValid(true);
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aFrm.Pos().setX(DOCUMENTBORDER);
        aFrm.Pos().setY(DOCUMENTBORDER);
    }

    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);
        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
        aPrt.Pos().setX(0);
        aPrt.Pos().setY(0);
        aPrt.SSize( getFrameArea().SSize() );
    }

    if ( !isFrameAreaSizeValid() )
    {
        // SSize is set by the pages (Cut/Paste).
        setFrameAreaSizeValid(true);
    }
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
    mnBrowseWidth = (!comphelper::LibreOfficeKit::isActive() && pSh)? MINLAY + 2 * pSh->GetOut()-> PixelToLogic( pSh->GetBrowseBorder() ).Width(): MIN_BROWSE_WIDTH;

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
            tools::Long nWidth = rAttrs.GetSize().Width();
            if ( nWidth < int(USHRT_MAX)-2000 && //-2k, because USHRT_MAX gets missing while trying to resize!  (and cast to int to avoid -Wsign-compare due to broken USHRT_MAX on Android)
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
                const bool bFly = pAnchoredObj->DynCastFlyFrame() !=  nullptr;
                if ((bFly && (FAR_AWAY == pAnchoredObj->GetObjRect().Width()))
                    || rFormat.GetFrameSize().GetWidthPercent())
                {
                    continue;
                }

                tools::Long nWidth = 0;
                switch ( rFormat.GetAnchor().GetAnchorId() )
                {
                    case RndStdIds::FLY_AS_CHAR:
                        nWidth = bFly ? rFormat.GetFrameSize().GetWidth() :
                                        pAnchoredObj->GetObjRect().Width();
                        break;
                    case RndStdIds::FLY_AT_PARA:
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
                                            nWidth += pFrame->getFramePrintArea().Left();
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
            if ( auto pCursorShell = dynamic_cast<SwCursorShell*>( &rSh) )
                pCursorShell->StartAction();
            else
                rSh.StartAction();
        }
}

void SwRootFrame::EndAllAction( bool bVirDev )
{
    if ( !GetCurrShell() )
        return;

    for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
    {
        const bool bOldEndActionByVirDev = rSh.IsEndActionByVirDev();
        rSh.SetEndActionByVirDev( bVirDev );
        if ( auto pCursorShell = dynamic_cast<SwCursorShell*>( &rSh) )
        {
            pCursorShell->EndAction();
            pCursorShell->CallChgLnk();
            if ( auto pFEShell = dynamic_cast<SwFEShell*>( &rSh) )
                pFEShell->SetChainMarker();
        }
        else
            rSh.EndAction();
        rSh.SetEndActionByVirDev( bOldEndActionByVirDev );
    }
}

void SwRootFrame::UnoRemoveAllActions()
{
    if ( !GetCurrShell() )
        return;

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
    if ( !GetCurrShell() )
        return;

    for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
    {
        sal_uInt16 nActions = rSh.GetRestoreActions();
        while( nActions-- )
        {
            if ( auto pCursorShell = dynamic_cast<SwCursorShell*>( &rSh) )
                pCursorShell->StartAction();
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
    // DO NOT CONVERT THIS TO A C++11 FOR LOOP, IT DID NOT WORK THE LAST 2 TIMES
    for (size_t i = 0; i < pSortedObj->size(); ++i)
    {
        SwAnchoredObject *const pAnchoredObj = (*pSortedObj)[i];
        const SwFrameFormat& rObjFormat = pAnchoredObj->GetFrameFormat();
        const SwFormatAnchor& rAnchor = rObjFormat.GetAnchor();

        // all except from the as character anchored objects are moved
        // when processing the page frame:
        if ( !bPage && (rAnchor.GetAnchorId() != RndStdIds::FLY_AS_CHAR) )
            continue;

        SwObjPositioningInProgress aPosInProgress( *pAnchoredObj );

        if ( auto pFlyFrame = pAnchoredObj->DynCastFlyFrame() )
        {
            lcl_MoveAllLowers( pFlyFrame, rOffset );
            // tdf#138785 update position specific to as-char flys
            if (pFlyFrame->IsFlyInContentFrame())
            {
                static_cast<SwFlyInContentFrame*>(pFlyFrame)->AddRefOfst(rOffset);
            }
            pFlyFrame->NotifyDrawObj();
            // --> let the active embedded object be moved
            SwFrame* pLower = pFlyFrame->Lower();
            if ( pLower && pLower->IsNoTextFrame() )
            {
                SwRootFrame* pRoot = pLower->getRootFrame();
                SwViewShell *pSh = pRoot ? pRoot->GetCurrShell() : nullptr;
                if ( pSh )
                {
                    SwNoTextFrame *const pContentFrame = static_cast<SwNoTextFrame*>(pLower);
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
        else if ( auto pAnchoredDrawObj = dynamic_cast<SwAnchoredDrawObject *>( pAnchoredObj ) )
        {
            // don't touch objects that are not yet positioned:
            if ( pAnchoredDrawObj->NotYetPositioned() )
                continue;

            const Point& aCurrAnchorPos = pAnchoredDrawObj->GetDrawObj()->GetAnchorPos();
            const Point aNewAnchorPos( aCurrAnchorPos + rOffset );
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
    // first move the current frame
    // RotateFlyFrame3: moved to transform_translate instead of
    // direct modification to allow the SwFrame evtl. needed own reactions
    pFrame->transform_translate(rOffset);

    // Don't forget accessibility:
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    if( pFrame->IsAccessibleFrame() )
    {
        SwRootFrame *pRootFrame = pFrame->getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            const SwRect aFrame( pFrame->getFrameArea() );

            pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( pFrame, aFrame );
        }
    }
#endif

    // the move any objects
    lcl_MoveAllLowerObjs( pFrame, rOffset );

    // finally, for layout frames we have to call this function recursively:
    if (pFrame->IsLayoutFrame())
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

    const tools::Long nBorder = getFrameArea().Pos().getX();
    const tools::Long nVisWidth = mnViewWidth - 2 * nBorder;
    const tools::Long nGapBetweenPages = pViewOpt ? pViewOpt->GetGapBetweenPages()
                                           : (pSh ? pSh->GetViewOptions()->GetGapBetweenPages()
                                                  : SwViewOption::defGapBetweenPages);

    // check how many pages fit into the first page layout row:
    SwPageFrame* pPageFrame = static_cast<SwPageFrame*>(Lower());

    // will contain the number of pages per row. 0 means that
    // the page does not fit.
    tools::Long nWidthRemain = nVisWidth;

    // after one row has been processed, these variables contain
    // the width of the row and the maximum of the page heights
    tools::Long nCurrentRowHeight = 0;
    tools::Long nCurrentRowWidth = 0;

    // these variables are used to finally set the size of the
    // root frame
    tools::Long nSumRowHeight = 0;
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
        tools::Long nPageWidth = 0;
        tools::Long nPageHeight = 0;

        if ( mbBookMode )
        {
            const SwFrame& rFormatPage = pPageFrame->GetFormatPage();

            nPageWidth  = rFormatPage.getFrameArea().Width()  + nSidebarWidth + ((bStartOfRow || 1 == (pPageFrame->GetPhyPageNum()%2)) ? 0 : nGapBetweenPages);
            nPageHeight = rFormatPage.getFrameArea().Height() + nGapBetweenPages;
        }
        else
        {
            if ( !pPageFrame->IsEmptyPage() )
            {
                nPageWidth  = pPageFrame->getFrameArea().Width() + nSidebarWidth + (bStartOfRow ? 0 : nGapBetweenPages);
                nPageHeight = pPageFrame->getFrameArea().Height() + nGapBetweenPages;
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
                SwTwips nLastPageWidth = pLastPageInCurrentRow->getFrameArea().Width() + nSidebarWidth;

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
                    pStartOfRow->GetFormatPage().getFrameArea().Width() + nSidebarWidth;
            }

            // center page if possible
            tools::Long nSizeDiff = 0;
            if (nVisWidth > nCurrentRowWidth && !comphelper::LibreOfficeKit::isActive())
                nSizeDiff = ( nVisWidth - nCurrentRowWidth ) / 2;

            // adjust positions of pages in current row
            tools::Long nX = nSizeDiff;

            const tools::Long nRowStart = nBorder + nSizeDiff;
            const tools::Long nRowEnd   = nRowStart + nCurrentRowWidth;

            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nX += pStartOfRow->GetFormatPage().getFrameArea().Width() + nSidebarWidth;
            }

            SwPageFrame* pEndOfRow = pPageFrame;
            SwPageFrame* pPageToAdjust = pStartOfRow;

            do
            {
                const SwPageFrame* pFormatPage = pPageToAdjust;
                if ( mbBookMode )
                    pFormatPage = &pPageToAdjust->GetFormatPage();

                const SwTwips nCurrentPageWidth = pFormatPage->getFrameArea().Width() + (pFormatPage->IsEmptyPage() ? 0 : nSidebarWidth);
                const Point aOldPagePos = pPageToAdjust->getFrameArea().Pos();
                const bool bLeftSidebar = pPageToAdjust->SidebarPosition() == sw::sidebarwindows::SidebarPosition::LEFT;
                const SwTwips nLeftPageAddOffset = bLeftSidebar ?
                                                   nSidebarWidth :
                                                   0;

                Point aNewPagePos( nBorder + nX, nBorder + nSumRowHeight );
                Point aNewPagePosWithLeftOffset( nBorder + nX + nLeftPageAddOffset, nBorder + nSumRowHeight );

                // RTL view layout: Calculate mirrored page position
                if ( bRTL )
                {
                    const tools::Long nXOffsetInRow = aNewPagePos.getX() - nRowStart;
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

                nMinPageLeft  = std::min( nMinPageLeft, SwTwips(aNewPagePos.getX()) );
                nMaxPageRight = std::max( nMaxPageRight, SwTwips(aNewPagePos.getX() + nCurrentPageWidth));

                // border of nGapBetweenPages around the current page:
                SwRect aPageRectWithBorders( aNewPagePos.getX() - nGapBetweenPages,
                                             aNewPagePos.getY(),
                                             pPageToAdjust->getFrameArea().SSize().Width() + nGapBetweenPages + nSidebarWidth,
                                             nCurrentRowHeight );

                static const tools::Long nOuterClickDiff = 1000000;

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
    const Size aOldSize( getFrameArea().SSize() );
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
                pSh->GetDoc()->GetDocShell()->Broadcast(SfxHint(SfxHintId::DocChanged));
            }
        }
    }

    maPagesArea.Pos( getFrameArea().Pos() );
    maPagesArea.SSize( aNewSize );
    if ( TWIPS_MAX != nMinPageLeft )
        maPagesArea.Left_( nMinPageLeft );

    SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

bool SwRootFrame::IsLeftToRightViewLayout() const
{
    // Layout direction determined by layout direction of the first page.
    // #i88036#
    // Only ask a non-empty page frame for its layout direction
    assert(dynamic_cast<const SwPageFrame *>(Lower()) != nullptr);
    const SwPageFrame& rPage = static_cast<const SwPageFrame&>(*Lower()).GetFormatPage();
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
        // But under certain circumstances this assumption is not true -
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
    tools::Long nUpperLimit = 0;
    tools::Long nLowerLimit = 0;
    const SwFrame* pFrame = Lower();
    while ( pFrame )
    {
        if ( pFrame->IsBodyFrame() )
        {
            nUpperLimit = pFrame->getFrameArea().Top();
            nLowerLimit = pFrame->getFrameArea().Bottom();
        }
        else if ( pFrame->IsFootnoteContFrame() )
            nLowerLimit = pFrame->getFrameArea().Bottom();

        pFrame = pFrame->GetNext();
    }

    SwRect aHeaderArea( getFrameArea().TopLeft(),
           Size( getFrameArea().Width(), nUpperLimit - getFrameArea().Top() ) );

    SwViewShell* pViewShell = getRootFrame()->GetCurrShell();
    const bool bHideWhitespaceMode = pViewShell->GetViewOptions()->IsHideWhitespaceMode();
    if ( aHeaderArea.Contains( rPt ) )
    {
        if (!bHideWhitespaceMode || static_cast<const SwFrameFormat*>(GetDep())->GetHeader().IsActive())
        {
            rControl = FrameControlType::Header;
            return true;
        }
    }
    else
    {
        SwRect aFooterArea( Point( getFrameArea().Left(), nLowerLimit ),
                Size( getFrameArea().Width(), getFrameArea().Bottom() - nLowerLimit ) );

        if ( aFooterArea.Contains( rPt ) &&
             (!bHideWhitespaceMode || static_cast<const SwFrameFormat*>(GetDep())->GetFooter().IsActive()) )
        {
            rControl = FrameControlType::Footer;
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
            const SwFrameFormat* pPageFormat = static_cast<const SwFrameFormat*>(GetDep());
            const Size& rPageSize = pPageFormat->GetFrameSize().GetSize();
            tools::Long nWhitespace = rPageSize.getHeight() - getFrameArea().Height();
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

const SwHeaderFrame* SwPageFrame::GetHeaderFrame() const
{
    const SwFrame* pLowerFrame = Lower();
    while (pLowerFrame)
    {
        if (pLowerFrame->IsHeaderFrame())
            return dynamic_cast<const SwHeaderFrame*>(pLowerFrame);
        pLowerFrame = pLowerFrame->GetNext();
    }
    return nullptr;
}

const SwFooterFrame* SwPageFrame::GetFooterFrame() const
{
    const SwFrame* pLowerFrame = Lower();
    while (pLowerFrame)
    {
        if (pLowerFrame->IsFooterFrame())
            return dynamic_cast<const SwFooterFrame*>(pLowerFrame);
        pLowerFrame = pLowerFrame->GetNext();
    }
    return nullptr;
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
