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

#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <ftnfrm.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <hints.hxx>
#include <sectfrm.hxx>
#include <notxtfrm.hxx>
#include <txtfly.hxx>

#include <svx/svdpage.hxx>
#include <editeng/ulspitem.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <ndole.hxx>
#include <tabfrm.hxx>
#include <flyfrms.hxx>
#include <fmtfollowtextflow.hxx>
#include <environmentofanchoredobject.hxx>
#include <sortedobjs.hxx>
#include <viewimp.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <pam.hxx>
#include <ndindex.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

SwFlyFreeFrame::SwFlyFreeFrame( SwFlyFrameFormat *pFormat, SwFrame* pSib, SwFrame *pAnch )
:   SwFlyFrame( pFormat, pSib, pAnch ),
    // #i34753#
    mbNoMakePos( false ),
    // #i37068#
    mbNoMoveOnCheckClip( false )
{
}

void SwFlyFreeFrame::DestroyImpl()
{
    // #i28701# - use new method <GetPageFrame()>
    if( GetPageFrame() )
    {
        if( GetFormat()->GetDoc()->IsInDtor() )
        {
            // #i29879# - remove also to-frame anchored Writer
            // fly frame from page.
            const bool bRemoveFromPage =
                    GetPageFrame()->GetSortedObjs() &&
                    ( IsFlyAtContentFrame() ||
                      ( GetAnchorFrame() && GetAnchorFrame()->IsFlyFrame() ) );
            if ( bRemoveFromPage )
            {
                GetPageFrame()->GetSortedObjs()->Remove( *this );
            }
        }
        else
        {
            SwRect aTmp( GetObjRectWithSpaces() );
            SwFlyFreeFrame::NotifyBackground( GetPageFrame(), aTmp, PrepareHint::FlyFrameLeave );
        }
    }

    SwFlyFrame::DestroyImpl();
}

SwFlyFreeFrame::~SwFlyFreeFrame()
{
#if 0
    // we are possibly in ContourCache, make sure we vanish
    ::ClrContourCache(GetVirtDrawObj());
#endif
}

// #i28701#
/** Notifies the background (all ContentFrames that currently are overlapping).
 *
 * Additionally, the window is also directly invalidated (especially where
 * there are no overlapping ContentFrames).
 * This also takes ContentFrames within other Flys into account.
 */
void SwFlyFreeFrame::NotifyBackground( SwPageFrame *pPageFrame,
                                     const SwRect& rRect, PrepareHint eHint )
{
    ::Notify_Background( GetVirtDrawObj(), pPageFrame, rRect, eHint, true );
}

void SwFlyFreeFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( !GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrame() || IsLocked() || IsColLocked() )
    {
        return;
    }

    // #i28701# - use new method <GetPageFrame()>
    if( !GetPageFrame() && GetAnchorFrame()->IsInFly() )
    {
        SwFlyFrame* pFly = AnchorFrame()->FindFlyFrame();
        SwPageFrame *pPageFrame = pFly ? pFly->FindPageFrame() : nullptr;
        if( pPageFrame )
            pPageFrame->AppendFlyToPage( this );
    }

    if( !GetPageFrame() )
    {
        return;
    }

    Lock(); // The curtain drops

    // takes care of the notification in the dtor
    const SwFlyNotify aNotify( this );

    if ( IsClipped() )
    {
        setFrameAreaSizeValid(false);
        m_bHeightClipped = m_bWidthClipped = false;
        // no invalidation of position,
        // if anchored object is anchored inside a Writer fly frame,
        // its position is already locked, and it follows the text flow.
        // #i34753# - add condition:
        // no invalidation of position, if no direct move is requested in <CheckClip(..)>
        if ( !IsNoMoveOnCheckClip() &&
             !( PositionLocked() &&
                GetAnchorFrame()->IsInFly() &&
                GetFrameFormat().GetFollowTextFlow().GetValue() ) )
        {
            setFrameAreaPositionValid(false);
        }
    }

    // #i81146# new loop control
    int nLoopControlRuns = 0;
    const int nLoopControlMax = 10;

    // RotateFlyFrame3 - outer frame
    const double fRotation(getLocalFrameRotation());
    const bool bRotated(!basegfx::fTools::equalZero(fRotation));

    if(bRotated)
    {
        // Re-layout may be partially (see all isFrameAreaDefinitionValid() flags),
        // so resetting the local SwFrame(s) in the local SwFrameAreaDefinition is
        // needed. Reset to BoundAreas will be done below automatically
        if(isTransformableSwFrame())
        {
            getTransformableSwFrame()->restoreFrameAreas();
        }
    }

    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() || m_bFormatHeightOnly || !m_bValidContentPos )
    {
        SwRectFnSet aRectFnSet(this);
        const SwFormatFrameSize *pSz;
        {   // Additional scope, so aAccess will be destroyed before the check!

            SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrameSize();

            // Only set when the flag is set!
            if ( !isFrameAreaSizeValid() )
            {
                setFramePrintAreaValid(false);
            }

            if ( !isFramePrintAreaValid() )
            {
                MakePrtArea( rAttrs );
                m_bValidContentPos = false;
            }

            if ( !isFrameAreaSizeValid() || m_bFormatHeightOnly )
            {
                setFrameAreaSizeValid(false);
                Format( getRootFrame()->GetCurrShell()->GetOut(), &rAttrs );
                m_bFormatHeightOnly = false;
            }
        }

        if ( !isFrameAreaPositionValid() )
        {
            const Point aOldPos( aRectFnSet.GetPos(getFrameArea()) );
            // #i26791# - use new method <MakeObjPos()>
            // #i34753# - no positioning, if requested.
            if ( IsNoMakePos() )
            {
                setFrameAreaPositionValid(true);
            }
            else
                // #i26791# - use new method <MakeObjPos()>
                MakeObjPos();
            if( aOldPos == aRectFnSet.GetPos(getFrameArea()) )
            {
                if( !isFrameAreaPositionValid() && GetAnchorFrame()->IsInSct() &&
                    !GetAnchorFrame()->FindSctFrame()->isFrameAreaDefinitionValid() )
                {
                    setFrameAreaPositionValid(true);
                }
            }
            else
            {
                setFrameAreaSizeValid(false);
            }
        }

        if ( !m_bValidContentPos )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            MakeContentPos( rAttrs );
        }

        if ( isFrameAreaPositionValid() && isFrameAreaSizeValid() )
        {
            ++nLoopControlRuns;

            OSL_ENSURE( nLoopControlRuns < nLoopControlMax, "LoopControl in SwFlyFreeFrame::MakeAll" );

            if ( nLoopControlRuns < nLoopControlMax )
                CheckClip( *pSz );
        }
        else
            nLoopControlRuns = 0;
    }

    // RotateFlyFrame3 - outer frame
    // Do not refresh transforms/Areas self here, this will be done
    // when inner and outer frame are layouted, in SwNoTextFrame::MakeAll
    if(bRotated)
    {
        // RotateFlyFrame3: Safe changes locally
        // get center from outer frame (layout frame) to be on the safe side
        const Point aCenter(getFrameArea().Center());
        const basegfx::B2DPoint aB2DCenter(aCenter.X(), aCenter.Y());

        if(!mpTransformableSwFrame)
        {
            mpTransformableSwFrame.reset(new TransformableSwFrame(*this));
        }

        getTransformableSwFrame()->createFrameAreaTransformations(
            fRotation,
            aB2DCenter);
        getTransformableSwFrame()->adaptFrameAreasToTransformations();
    }
    else
    {
        // RotateFlyFrame3: Also need to clear ContourCache (if used),
        // usually done in SwFlyFrame::NotifyDrawObj, but there relies on
        // being in transform mode which is already reset then
        if(isTransformableSwFrame())
        {
            ::ClrContourCache(GetVirtDrawObj());
        }

        // reset transformations to show that they are not used
        mpTransformableSwFrame.reset();
    }

    Unlock();

#if OSL_DEBUG_LEVEL > 0
    SwRectFnSet aRectFnSet(this);
    OSL_ENSURE( m_bHeightClipped || ( aRectFnSet.GetHeight(getFrameArea()) > 0 &&
            aRectFnSet.GetHeight(getFramePrintArea()) > 0),
            "SwFlyFreeFrame::Format(), flipping Fly." );

#endif
}

bool SwFlyFreeFrame::supportsAutoContour() const
{
    static bool bOverrideHandleContourToAlwaysOff(true); // loplugin:constvars:ignore

    // RotateFlyFrameFix: For LO6.0 we need to deactivate the AutoContour feature again, it is simply
    // not clear how/if to use and save/load it in ODF. This has to be discussed.
    // The reason not to remove is that this may be used as-is now, using a new switch.
    // Even when not, the detection if it is possible will be needed in any case later.
    if(bOverrideHandleContourToAlwaysOff)
    {
        return false;
    }

    if(!isTransformableSwFrame())
    {
        // support only when transformed, else there is no free space
        return false;
    }

    // Check for Borders. If we have Borders, do (currently) not support,
    // since borders do not transform with the object.
    // (Will need to be enhanced to take into account if we have Borders and if these
    // transform with the object)
    SwBorderAttrAccess aAccess(SwFrame::GetCache(), this);
    const SwBorderAttrs &rAttrs(*aAccess.Get());

    if(rAttrs.IsLine())
    {
        return false;
    }

    // Check for Padding. Do not support when padding is used, this will
    // produce a covered space around the object (filled with fill defines)
    const SfxPoolItem* pItem(nullptr);

    if(GetFormat() && SfxItemState::SET == GetFormat()->GetItemState(RES_BOX, false, &pItem))
    {
        const SvxBoxItem& rBox = *static_cast< const SvxBoxItem* >(pItem);

        if(rBox.HasBorder(/*bTreatPaddingAsBorder*/true))
        {
            return false;
        }
    }

    // check for Fill - if we have fill, it will fill the gaps and we will not
    // support AutoContour
    if(GetFormat() && GetFormat()->supportsFullDrawingLayerFillAttributeSet())
    {
        const drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes(GetFormat()->getSdrAllFillAttributesHelper());

        if(aFillAttributes && aFillAttributes->isUsed())
        {
            return false;
        }
    }
    else
    {
        const std::unique_ptr<SvxBrushItem> aBack(GetFormat()->makeBackgroundBrushItem());

        if(aBack && aBack->isUsed())
        {
            return false;
        }
    }

    // else, support
    return true;
}

// RotateFlyFrame3 - Support for Transformations - outer frame
basegfx::B2DHomMatrix SwFlyFreeFrame::getFrameAreaTransformation() const
{
    if(isTransformableSwFrame())
    {
        // use pre-created transformation
        return getTransformableSwFrame()->getLocalFrameAreaTransformation();
    }

    // call parent
    return SwFlyFrame::getFrameAreaTransformation();
}

basegfx::B2DHomMatrix SwFlyFreeFrame::getFramePrintAreaTransformation() const
{
    if(isTransformableSwFrame())
    {
        // use pre-created transformation
        return getTransformableSwFrame()->getLocalFramePrintAreaTransformation();
    }

    // call parent
    return SwFlyFrame::getFramePrintAreaTransformation();
}

// RotateFlyFrame3 - Support for Transformations
void SwFlyFreeFrame::transform_translate(const Point& rOffset)
{
    // call parent - this will do the basic transform for SwRect(s)
    // in the SwFrameAreaDefinition
    SwFlyFrame::transform_translate(rOffset);

    // check if the Transformations need to be adapted
    if(isTransformableSwFrame())
    {
        const basegfx::B2DHomMatrix aTransform(
            basegfx::utils::createTranslateB2DHomMatrix(
                rOffset.X(), rOffset.Y()));

        // transform using TransformableSwFrame
        getTransformableSwFrame()->transform(aTransform);
    }
}

// RotateFlyFrame3 - outer frame
double getLocalFrameRotation_from_SwNoTextFrame(const SwNoTextFrame& rNoTextFrame)
{
    return rNoTextFrame.getLocalFrameRotation();
}

double SwFlyFreeFrame::getLocalFrameRotation() const
{
    // SwLayoutFrame::Lower() != SwFrame::GetLower(), but SwFrame::GetLower()
    // calls SwLayoutFrame::Lower() when it's a SwLayoutFrame - so use GetLower()
    const SwNoTextFrame* pSwNoTextFrame(dynamic_cast< const SwNoTextFrame* >(GetLower()));

    if(nullptr != pSwNoTextFrame)
    {
        return getLocalFrameRotation_from_SwNoTextFrame(*pSwNoTextFrame);
    }

    // no rotation
    return 0.0;
}

/** determines, if direct environment of fly frame has 'auto' size

    #i17297#
    start with anchor frame and search via <GetUpper()> for a header, footer,
    row or fly frame stopping at page frame.
    return <true>, if such a frame is found and it has 'auto' size.
    otherwise <false> is returned.

    @return boolean indicating, that direct environment has 'auto' size
*/
bool SwFlyFreeFrame::HasEnvironmentAutoSize() const
{
    bool bRetVal = false;

    const SwFrame* pToBeCheckedFrame = GetAnchorFrame();
    while ( pToBeCheckedFrame &&
            !pToBeCheckedFrame->IsPageFrame() )
    {
        if ( pToBeCheckedFrame->IsHeaderFrame() ||
             pToBeCheckedFrame->IsFooterFrame() ||
             pToBeCheckedFrame->IsRowFrame() ||
             pToBeCheckedFrame->IsFlyFrame() )
        {
            bRetVal = SwFrameSize::Fixed !=
                      pToBeCheckedFrame->GetAttrSet()->GetFrameSize().GetHeightSizeType();
            break;
        }
        else
        {
            pToBeCheckedFrame = pToBeCheckedFrame->GetUpper();
        }
    }

    return bRetVal;
}

void SwFlyFreeFrame::CheckClip( const SwFormatFrameSize &rSz )
{
    // It's probably time now to take appropriate measures, if the Fly
    // doesn't fit into its surrounding.
    // First, the Fly gives up its position, then it's formatted.
    // Only if it still doesn't fit after giving up its position, the
    // width or height are given up as well. The frame will be squeezed
    // as much as needed.

    const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
    SwRect aClip, aTmpStretch;
    ::CalcClipRect( pObj, aClip );
    ::CalcClipRect( pObj, aTmpStretch, false );
    aClip.Intersection_( aTmpStretch );

    const tools::Long nBot = getFrameArea().Top() + getFrameArea().Height();
    const tools::Long nRig = getFrameArea().Left() + getFrameArea().Width();
    const tools::Long nClipBot = aClip.Top() + aClip.Height();
    const tools::Long nClipRig = aClip.Left() + aClip.Width();

    const bool bBot = nBot > nClipBot;
    const bool bRig = nRig > nClipRig;
    if (( bBot || bRig ) && !IsDraggingOffPageAllowed(FindFrameFormat(GetDrawObj())))
    {
        bool bAgain = false;
        // #i37068# - no move, if it's requested
        if ( bBot && !IsNoMoveOnCheckClip() &&
             !GetDrawObjs() && !GetAnchorFrame()->IsInTab() )
        {
            SwFrame* pHeader = FindFooterOrHeader();
            // In a header, correction of the position is no good idea.
            // If the fly moves, some paragraphs have to be formatted, this
            // could cause a change of the height of the headerframe,
            // now the flyframe can change its position and so on ...
            if ( !pHeader || !pHeader->IsHeaderFrame() )
            {
                const tools::Long nOld = getFrameArea().Top();

                // tdf#112443 disable positioning if content is completely off page
                bool bDisableOffPagePositioning = GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::DISABLE_OFF_PAGE_POSITIONING);
                if ( !bDisableOffPagePositioning || nOld <= nClipBot)
                {
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                    aFrm.Pos().setY( std::max( aClip.Top(), nClipBot - aFrm.Height() ) );
                }

                if ( getFrameArea().Top() != nOld )
                {
                    bAgain = true;
                }

                m_bHeightClipped = true;
            }
        }
        if ( bRig )
        {
            const tools::Long nOld = getFrameArea().Left();

            // tdf#112443 disable positioning if content is completely off page
            bool bDisableOffPagePositioning = GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::DISABLE_OFF_PAGE_POSITIONING);
            if ( !bDisableOffPagePositioning || nOld <= nClipRig )
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aFrm.Pos().setX( std::max( aClip.Left(), nClipRig - aFrm.Width() ) );
            }

            if ( getFrameArea().Left() != nOld )
            {
                const SwFormatHoriOrient &rH = GetFormat()->GetHoriOrient();
                // Left-aligned ones may not be moved to the left when they
                // are avoiding another one.
                if( rH.GetHoriOrient() == text::HoriOrientation::LEFT )
                {
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                    aFrm.Pos().setX( nOld );
                }
                else
                {
                    bAgain = true;
                }
            }
            m_bWidthClipped = true;
        }
        if ( bAgain )
        {
            setFrameAreaSizeValid(false);
        }
        else
        {
            // If we reach this branch, the Frame protrudes into forbidden
            // areas, and correcting the position is not allowed or not
            // possible or not required.

            // For Flys with OLE objects as lower, we make sure that
            // we always resize proportionally
            Size aOldSize( getFrameArea().SSize() );

            // First, setup the FrameRect, then transfer it to the Frame.
            SwRect aFrameRect( getFrameArea() );

            if ( bBot )
            {
                tools::Long nDiff = nClipBot;
                nDiff -= aFrameRect.Top(); // nDiff represents the available distance
                nDiff = aFrameRect.Height() - nDiff;
                aFrameRect.Height( aFrameRect.Height() - nDiff );
                m_bHeightClipped = true;
            }
            if ( bRig )
            {
                tools::Long nDiff = nClipRig;
                nDiff -= aFrameRect.Left();// nDiff represents the available distance
                nDiff = aFrameRect.Width() - nDiff;
                aFrameRect.Width( aFrameRect.Width() - nDiff );
                m_bWidthClipped = true;
            }

            // #i17297# - no proportional
            // scaling of graphics in environments, which determines its size
            // by its content ('auto' size). Otherwise layout loops can occur and
            // layout sizes of the environment can be incorrect.
            // Such environment are:
            // (1) header and footer frames with 'auto' size
            // (2) table row frames with 'auto' size
            // (3) fly frames with 'auto' size
            // Note: section frames seems to be not critical - didn't found
            //       any critical layout situation so far.
            if ( Lower() && Lower()->IsNoTextFrame() &&
                 (static_cast<SwNoTextFrame*>(Lower())->GetNode()->GetOLENode() ||
                   !HasEnvironmentAutoSize() ) )
            {
                // If width and height got adjusted, then the bigger
                // change is relevant.
                if ( aFrameRect.Width() != aOldSize.Width() &&
                     aFrameRect.Height()!= aOldSize.Height() )
                {
                    if ( (aOldSize.Width() - aFrameRect.Width()) >
                         (aOldSize.Height()- aFrameRect.Height()) )
                        aFrameRect.Height( aOldSize.Height() );
                    else
                        aFrameRect.Width( aOldSize.Width() );
                }

                // Adjusted the width? change height proportionally
                if( aFrameRect.Width() != aOldSize.Width() )
                {
                    aFrameRect.Height( aFrameRect.Width() * aOldSize.Height() /
                                     aOldSize.Width() );
                    m_bHeightClipped = true;
                }
                // Adjusted the height? change width proportionally
                else if( aFrameRect.Height() != aOldSize.Height() )
                {
                    aFrameRect.Width( aFrameRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
                    m_bWidthClipped = true;
                }

                // #i17297# - reactivate change
                // of size attribute for fly frames containing an ole object.

                // Added the aFrameRect.HasArea() hack, because
                // the environment of the ole object does not have to be valid
                // at this moment, or even worse, it does not have to have a
                // reasonable size. In this case we do not want to change to
                // attributes permanently. Maybe one day somebody dares to remove
                // this code.
                if ( aFrameRect.HasArea() &&
                     static_cast<SwNoTextFrame*>(Lower())->GetNode()->GetOLENode() &&
                     ( m_bWidthClipped || m_bHeightClipped ) )
                {
                    SwFlyFrameFormat *pFormat = GetFormat();
                    pFormat->LockModify();
                    SwFormatFrameSize aFrameSize( rSz );
                    aFrameSize.SetWidth( aFrameRect.Width() );
                    aFrameSize.SetHeight( aFrameRect.Height() );
                    pFormat->SetFormatAttr( aFrameSize );
                    pFormat->UnlockModify();
                }
            }

            // Now change the Frame; for columns, we put the new values into the attributes,
            // otherwise we'll end up with unwanted side-effects/oscillations
            const tools::Long nPrtHeightDiff = getFrameArea().Height() - getFramePrintArea().Height();
            const tools::Long nPrtWidthDiff  = getFrameArea().Width()  - getFramePrintArea().Width();
            maUnclippedFrame = getFrameArea();

            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aFrm.Height( aFrameRect.Height() );
                aFrm.Width ( std::max( tools::Long(MINLAY), aFrameRect.Width() ) );
            }

            if ( Lower() && Lower()->IsColumnFrame() )
            {
                ColLock();  //lock grow/shrink
                const Size aTmpOldSize( getFramePrintArea().SSize() );

                {
                    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                    aPrt.Height( getFrameArea().Height() - nPrtHeightDiff );
                    aPrt.Width ( getFrameArea().Width()  - nPrtWidthDiff );
                }

                ChgLowersProp( aTmpOldSize );
                SwFrame *pLow = Lower();
                do
                {
                    pLow->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    // also calculate the (Column)BodyFrame
                    static_cast<SwLayoutFrame*>(pLow)->Lower()->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    pLow = pLow->GetNext();
                } while ( pLow );
                ::CalcContent( this );
                ColUnlock();

                if ( !isFrameAreaSizeValid() && !m_bWidthClipped )
                {
                    setFrameAreaSizeValid(true);
                    m_bFormatHeightOnly = true;
                }
            }
            else
            {
                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                aPrt.Height( getFrameArea().Height() - nPrtHeightDiff );
                aPrt.Width ( getFrameArea().Width()  - nPrtWidthDiff );
            }
        }
    }

    // #i26945#
    OSL_ENSURE( getFrameArea().Height() >= 0,
            "<SwFlyFreeFrame::CheckClip(..)> - fly frame has negative height now." );
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible
    #i43771#
*/
bool SwFlyFreeFrame::IsFormatPossible() const
{
    return SwFlyFrame::IsFormatPossible() &&
           ( GetPageFrame() ||
             ( GetAnchorFrame() && GetAnchorFrame()->IsInFly() ) );
}

SwFlyLayFrame::SwFlyLayFrame( SwFlyFrameFormat *pFormat, SwFrame* pSib, SwFrame *pAnch ) :
    SwFlyFreeFrame( pFormat, pSib, pAnch )
{
    m_bLayout = true;
}

// #i28701#

void SwFlyLayFrame::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy->m_pNew)
        return;
    const auto pAnch = GetAnchorFromPoolItem(*pLegacy->m_pNew);

    if(!pAnch)
    {
        SwFlyFrame::SwClientNotify(rMod, rHint);
        return;
    }
    SAL_WARN_IF(pAnch->GetAnchorId() == GetFormat()->GetAnchor().GetAnchorId(), "sw.core", "Invalid change of anchor type.");

    // Unregister, get hold of the page, attach to the corresponding LayoutFrame.
    SwRect aOld(GetObjRectWithSpaces());
    // #i28701# - use new method <GetPageFrame()>
    SwPageFrame* pOldPage = GetPageFrame();
    AnchorFrame()->RemoveFly(this);

    if(RndStdIds::FLY_AT_PAGE == pAnch->GetAnchorId())
    {
        SwRootFrame* pRoot = getRootFrame();
        SwPageFrame* pTmpPage = static_cast<SwPageFrame*>(pRoot->Lower());
        sal_uInt16 nPagesToFlip = pAnch->GetPageNum()-1;
        while(pTmpPage && nPagesToFlip)
        {
            pTmpPage = static_cast<SwPageFrame*>(pTmpPage->GetNext());
            --nPagesToFlip;
        }
        if(pTmpPage && !nPagesToFlip)
        {
            // #i50432# - adjust synopsis of <PlaceFly(..)>
            pTmpPage->PlaceFly(this, nullptr);
        }
        if(!pTmpPage)
        {
            pRoot->SetAssertFlyPages();
            pRoot->AssertFlyPages();
        }
    }
    else
    {
        SwNodeIndex aIdx(pAnch->GetContentAnchor()->nNode);
        SwContentFrame* pContent = GetFormat()->GetDoc()->GetNodes().GoNext(&aIdx)->
                GetContentNode()->getLayoutFrame(getRootFrame(), nullptr, nullptr);
        if(pContent)
        {
            SwFlyFrame *pTmp = pContent->FindFlyFrame();
            if(pTmp)
                pTmp->AppendFly(this);
        }
    }
    // #i28701# - use new method <GetPageFrame()>
    if ( pOldPage && pOldPage != GetPageFrame() )
        NotifyBackground( pOldPage, aOld, PrepareHint::FlyFrameLeave );
    SetCompletePaint();
    InvalidateAll();
    SetNotifyBack();
}

void SwPageFrame::AppendFlyToPage( SwFlyFrame *pNew )
{
    if ( !pNew->GetVirtDrawObj()->IsInserted() )
        getRootFrame()->GetDrawPage()->InsertObject(
                static_cast<SdrObject*>(pNew->GetVirtDrawObj()),
                pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );

    InvalidateSpelling();
    InvalidateSmartTags();
    InvalidateAutoCompleteWords();
    InvalidateWordCount();

    if ( GetUpper() )
    {
        static_cast<SwRootFrame*>(GetUpper())->SetIdleFlags();
        static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
    }

    SdrObject* pObj = pNew->GetVirtDrawObj();
    OSL_ENSURE( pNew->GetAnchorFrame(), "Fly without Anchor" );
    SwFlyFrame* pFly = const_cast<SwFlyFrame*>(pNew->GetAnchorFrame()->FindFlyFrame());
    if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        //#i119945# set pFly's OrdNum to _rNewObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
        sal_uInt32 nNewNum = pObj->GetOrdNumDirect();
        if ( pObj->getSdrPageFromSdrObject() )
            pObj->getSdrPageFromSdrObject()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFly->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    // Don't look further at Flys that sit inside the Content.
    if ( pNew->IsFlyInContentFrame() )
        InvalidateFlyInCnt();
    else
    {
        InvalidateFlyContent();

        if ( !m_pSortedObjs )
        {
            m_pSortedObjs.reset(new SwSortedObjs());
        }

        const bool bSuccessInserted = m_pSortedObjs->Insert( *pNew );
        OSL_ENSURE( bSuccessInserted, "Fly not inserted in Sorted." );

        // #i87493#
        OSL_ENSURE( pNew->GetPageFrame() == nullptr || pNew->GetPageFrame() == this,
                "<SwPageFrame::AppendFlyToPage(..)> - anchored fly frame seems to be registered at another page frame. Serious defect." );
        // #i28701# - use new method <SetPageFrame(..)>
        pNew->SetPageFrame( this );
        pNew->InvalidatePage( this );
        // #i28701#
        pNew->UnlockPosition();
        // needed to reposition at-page anchored flys moved from different page
        pNew->InvalidateObjPos();

        // Notify accessible layout. That's required at this place for
        // frames only where the anchor is moved. Creation of new frames
        // is additionally handled by the SwFrameNotify class.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
        if( GetUpper() &&
            static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
             static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
        {
            static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                      ->AddAccessibleFrame( pNew );
        }
#endif
    }

    // #i28701# - correction: consider also drawing objects
    if ( !pNew->GetDrawObjs() )
        return;

    SwSortedObjs &rObjs = *pNew->GetDrawObjs();
    for (SwAnchoredObject* pTmpObj : rObjs)
    {
        if ( auto pTmpFly = pTmpObj->DynCastFlyFrame() )
        {
            // #i28701# - use new method <GetPageFrame()>
            if ( pTmpFly->IsFlyFreeFrame() && !pTmpFly->GetPageFrame() )
                AppendFlyToPage( pTmpFly );
        }
        else if ( dynamic_cast<const SwAnchoredDrawObject*>( pTmpObj) !=  nullptr )
        {
            // #i87493#
            if ( pTmpObj->GetPageFrame() != this )
            {
                if ( pTmpObj->GetPageFrame() != nullptr )
                {
                    pTmpObj->GetPageFrame()->RemoveDrawObjFromPage( *pTmpObj );
                }
                AppendDrawObjToPage( *pTmpObj );
            }
        }
    }
}

void SwPageFrame::RemoveFlyFromPage( SwFlyFrame *pToRemove )
{
    const sal_uInt32 nOrdNum = pToRemove->GetVirtDrawObj()->GetOrdNum();
    getRootFrame()->GetDrawPage()->RemoveObject( nOrdNum );
    pToRemove->GetVirtDrawObj()->ReferencedObj().SetOrdNum( nOrdNum );

    if ( GetUpper() )
    {
        if ( !pToRemove->IsFlyInContentFrame() )
            static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
        static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
    }

    // Don't look further at Flys that sit inside the Content.
    if ( pToRemove->IsFlyInContentFrame() )
        return;

    // Don't delete collections just yet. This will happen at the end of the
    // action in the RemoveSuperfluous of the page, kicked off by a method of
    // the same name in the root.
    // The FlyColl might be gone already, because the page's dtor is being
    // executed.
    // Remove it _before_ disposing accessible frames to avoid accesses to
    // the Frame from event handlers.
    if (m_pSortedObjs)
    {
        m_pSortedObjs->Remove(*pToRemove);
        if (!m_pSortedObjs->size())
        {
            m_pSortedObjs.reset();
        }
    }

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrameNotify class.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    if( GetUpper() &&
        static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrame( pToRemove, true );
    }
#endif

    // #i28701# - use new method <SetPageFrame(..)>
    pToRemove->SetPageFrame( nullptr );
}

void SwPageFrame::MoveFly( SwFlyFrame *pToMove, SwPageFrame *pDest )
{
    // Invalidations
    if ( GetUpper() )
    {
        static_cast<SwRootFrame*>(GetUpper())->SetIdleFlags();
        if ( !pToMove->IsFlyInContentFrame() && pDest->GetPhyPageNum() < GetPhyPageNum() )
            static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
    }

    pDest->InvalidateSpelling();
    pDest->InvalidateSmartTags();
    pDest->InvalidateAutoCompleteWords();
    pDest->InvalidateWordCount();

    if ( pToMove->IsFlyInContentFrame() )
    {
        pDest->InvalidateFlyInCnt();
        return;
    }

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrameNotify class.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    if( GetUpper() &&
        static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrame( pToMove, true );
    }
#endif

    // The FlyColl might be gone already, because the page's dtor is being executed.
    if ( m_pSortedObjs )
    {
        m_pSortedObjs->Remove( *pToMove );
        if ( !m_pSortedObjs->size() )
        {
            m_pSortedObjs.reset();
        }
    }

    // Register
    if ( !pDest->GetSortedObjs() )
        pDest->m_pSortedObjs.reset(new SwSortedObjs());

    const bool bSuccessInserted = pDest->GetSortedObjs()->Insert( *pToMove );
    OSL_ENSURE( bSuccessInserted, "Fly not inserted in Sorted." );

    // #i28701# - use new method <SetPageFrame(..)>
    pToMove->SetPageFrame( pDest );
    pToMove->InvalidatePage( pDest );
    pToMove->SetNotifyBack();
    pDest->InvalidateFlyContent();
    // #i28701#
    pToMove->UnlockPosition();

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrameNotify class.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    if( GetUpper() &&
        static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->AddAccessibleFrame( pToMove );
    }
#endif

    // #i28701# - correction: move lowers of Writer fly frame
    if ( !pToMove->GetDrawObjs() )
        return;

    SwSortedObjs &rObjs = *pToMove->GetDrawObjs();
    for (SwAnchoredObject* pObj : rObjs)
    {
        if ( auto pFly = pObj->DynCastFlyFrame() )
        {
            if ( pFly->IsFlyFreeFrame() )
            {
                // #i28701# - use new method <GetPageFrame()>
                SwPageFrame* pPageFrame = pFly->GetPageFrame();
                if ( pPageFrame )
                    pPageFrame->MoveFly( pFly, pDest );
                else
                    pDest->AppendFlyToPage( pFly );
            }
        }
        else if ( dynamic_cast<const SwAnchoredDrawObject*>( pObj) !=  nullptr )
        {
            RemoveDrawObjFromPage( *pObj );
            pDest->AppendDrawObjToPage( *pObj );
        }
    }
}

void SwPageFrame::AppendDrawObjToPage( SwAnchoredObject& _rNewObj )
{
    if ( dynamic_cast<const SwAnchoredDrawObject*>( &_rNewObj) ==  nullptr )
    {
        OSL_FAIL( "SwPageFrame::AppendDrawObjToPage(..) - anchored object of unexpected type -> object not appended" );
        return;
    }

    if ( GetUpper() )
    {
        static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
    }

    assert(_rNewObj.GetAnchorFrame());
    SwFlyFrame* pFlyFrame = const_cast<SwFlyFrame*>(_rNewObj.GetAnchorFrame()->FindFlyFrame());
    if ( pFlyFrame &&
         _rNewObj.GetDrawObj()->GetOrdNum() < pFlyFrame->GetVirtDrawObj()->GetOrdNum() )
    {
        //#i119945# set pFly's OrdNum to _rNewObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
        sal_uInt32 nNewNum = _rNewObj.GetDrawObj()->GetOrdNumDirect();
        if ( _rNewObj.GetDrawObj()->getSdrPageFromSdrObject() )
            _rNewObj.DrawObj()->getSdrPageFromSdrObject()->SetObjectOrdNum( pFlyFrame->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFlyFrame->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    if ( RndStdIds::FLY_AS_CHAR == _rNewObj.GetFrameFormat().GetAnchor().GetAnchorId() )
    {
        return;
    }

    if ( !m_pSortedObjs )
    {
        m_pSortedObjs.reset(new SwSortedObjs());
    }
    if ( !m_pSortedObjs->Insert( _rNewObj ) )
    {
        OSL_ENSURE( m_pSortedObjs->Contains( _rNewObj ),
                "Drawing object not appended into list <pSortedObjs>." );
    }
    // #i87493#
    OSL_ENSURE( _rNewObj.GetPageFrame() == nullptr || _rNewObj.GetPageFrame() == this,
            "<SwPageFrame::AppendDrawObjToPage(..)> - anchored draw object seems to be registered at another page frame. Serious defect." );
    _rNewObj.SetPageFrame( this );

    // invalidate page in order to force a reformat of object layout of the page.
    InvalidateFlyLayout();
}

void SwPageFrame::RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj )
{
    if ( dynamic_cast<const SwAnchoredDrawObject*>( &_rToRemoveObj) ==  nullptr )
    {
        OSL_FAIL( "SwPageFrame::RemoveDrawObjFromPage(..) - anchored object of unexpected type -> object not removed" );
        return;
    }

    if ( m_pSortedObjs )
    {
        m_pSortedObjs->Remove( _rToRemoveObj );
        if ( !m_pSortedObjs->size() )
        {
            m_pSortedObjs.reset();
        }
        if ( GetUpper() )
        {
            if (RndStdIds::FLY_AS_CHAR !=
                    _rToRemoveObj.GetFrameFormat().GetAnchor().GetAnchorId())
            {
                static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
        }
    }
    _rToRemoveObj.SetPageFrame( nullptr );
}

// #i50432# - adjust method description and synopsis.
void SwPageFrame::PlaceFly( SwFlyFrame* pFly, SwFlyFrameFormat* pFormat )
{
    // #i50432# - consider the case that page is an empty page:
    // In this case append the fly frame at the next page
    OSL_ENSURE( !IsEmptyPage() || GetNext(),
            "<SwPageFrame::PlaceFly(..)> - empty page with no next page! -> fly frame appended at empty page" );
    if ( IsEmptyPage() && GetNext() )
    {
        static_cast<SwPageFrame*>(GetNext())->PlaceFly( pFly, pFormat );
    }
    else
    {
        // If we received a Fly, we use that one. Otherwise, create a new
        // one using the Format.
        if ( pFly )
            AppendFly( pFly );
        else
        {
            OSL_ENSURE( pFormat, ":-( No Format given for Fly." );
            pFly = new SwFlyLayFrame( pFormat, this, this );
            AppendFly( pFly );
            ::RegistFlys( this, pFly );
        }
    }
}

// #i18732# - adjustments for following text flow or not
// AND alignment at 'page areas' for to paragraph/to character anchored objects
// #i22305# - adjustment for following text flow for to frame anchored objects
// #i29778# - Because calculating the floating screen object's position
// (Writer fly frame or drawing object) doesn't perform a calculation on its
// upper frames and its anchor frame, a calculation of the upper frames in this
// method is no longer sensible.
// #i28701# - if document compatibility option 'Consider wrapping style influence
// on object positioning' is ON, the clip area corresponds to the one as the
// object doesn't follow the text flow.
bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, bool bMove )
{
    bool bRet = true;
    if ( auto pVirtFlyDrawObj = dynamic_cast<const SwVirtFlyDrawObj*>(pSdrObj) )
    {
        const SwFlyFrame* pFly = pVirtFlyDrawObj->GetFlyFrame();
        const bool bFollowTextFlow = pFly->GetFormat()->GetFollowTextFlow().GetValue();
        // #i28701#
        const bool bConsiderWrapOnObjPos =
                                pFly->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
        const SwFormatVertOrient &rV = pFly->GetFormat()->GetVertOrient();
        if( pFly->IsFlyLayFrame() )
        {
            const SwFrame* pClip;
            // #i22305#
            // #i28701#
            if ( !bFollowTextFlow || bConsiderWrapOnObjPos )
            {
                pClip = pFly->GetAnchorFrame()->FindPageFrame();
            }
            else
            {
                pClip = pFly->GetAnchorFrame();
            }

            rRect = pClip->getFrameArea();
            SwRectFnSet aRectFnSet(pClip);

            // vertical clipping: Top and Bottom, also to PrtArea if necessary
            if( rV.GetVertOrient() != text::VertOrientation::NONE &&
                rV.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
            {
                aRectFnSet.SetTop( rRect, aRectFnSet.GetPrtTop(*pClip) );
                aRectFnSet.SetBottom( rRect, aRectFnSet.GetPrtBottom(*pClip) );
            }
            // horizontal clipping: Top and Bottom, also to PrtArea if necessary
            const SwFormatHoriOrient &rH = pFly->GetFormat()->GetHoriOrient();
            if( rH.GetHoriOrient() != text::HoriOrientation::NONE &&
                rH.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
            {
                aRectFnSet.SetLeft( rRect, aRectFnSet.GetPrtLeft(*pClip) );
                aRectFnSet.SetRight(rRect, aRectFnSet.GetPrtRight(*pClip));
            }
        }
        else if( pFly->IsFlyAtContentFrame() )
        {
            // #i18732# - consider following text flow or not
            // AND alignment at 'page areas'
            const SwFrame* pVertPosOrientFrame = pFly->GetVertPosOrientFrame();
            if ( !pVertPosOrientFrame )
            {
                OSL_FAIL( "::CalcClipRect(..) - frame, vertical position is oriented at, is missing .");
                pVertPosOrientFrame = pFly->GetAnchorFrame();
            }

            if ( !bFollowTextFlow || bConsiderWrapOnObjPos )
            {
                const SwLayoutFrame* pClipFrame = pVertPosOrientFrame->FindPageFrame();
                if (!pClipFrame)
                {
                    OSL_FAIL("!pClipFrame: "
                            "if you can reproduce this please file a bug");
                    return false;
                }
                rRect = bMove ? pClipFrame->GetUpper()->getFrameArea()
                              : pClipFrame->getFrameArea();
                // #i26945# - consider that a table, during
                // its format, can exceed its upper printing area bottom.
                // Thus, enlarge the clip rectangle, if such a case occurred
                if ( pFly->GetAnchorFrame()->IsInTab() )
                {
                    const SwTabFrame* pTabFrame = const_cast<SwFlyFrame*>(pFly)
                                ->GetAnchorFrameContainingAnchPos()->FindTabFrame();
                    SwRect aTmp( pTabFrame->getFramePrintArea() );
                    aTmp += pTabFrame->getFrameArea().Pos();
                    rRect.Union( aTmp );
                    // #i43913# - consider also the cell frame
                    const SwFrame* pCellFrame = const_cast<SwFlyFrame*>(pFly)
                                ->GetAnchorFrameContainingAnchPos()->GetUpper();
                    while ( pCellFrame && !pCellFrame->IsCellFrame() )
                    {
                        pCellFrame = pCellFrame->GetUpper();
                    }
                    if ( pCellFrame )
                    {
                        aTmp = pCellFrame->getFramePrintArea();
                        aTmp += pCellFrame->getFrameArea().Pos();
                        rRect.Union( aTmp );
                    }
                }
            }
            else if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                      rV.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
            {
                // new class <SwEnvironmentOfAnchoredObject>
                objectpositioning::SwEnvironmentOfAnchoredObject
                                                aEnvOfObj( bFollowTextFlow );
                const SwLayoutFrame& rVertClipFrame =
                    aEnvOfObj.GetVertEnvironmentLayoutFrame( *pVertPosOrientFrame );
                if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_FRAME )
                {
                    rRect = rVertClipFrame.getFrameArea();
                }
                else if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( rVertClipFrame.IsPageFrame() )
                    {
                        rRect = static_cast<const SwPageFrame&>(rVertClipFrame).PrtWithoutHeaderAndFooter();
                    }
                    else
                    {
                        rRect = rVertClipFrame.getFrameArea();
                    }
                }
                const SwLayoutFrame* pHoriClipFrame =
                        pFly->GetAnchorFrame()->FindPageFrame()->GetUpper();
                SwRectFnSet aRectFnSet(pFly->GetAnchorFrame());
                aRectFnSet.SetLeft( rRect, aRectFnSet.GetLeft(pHoriClipFrame->getFrameArea()) );
                aRectFnSet.SetRight(rRect, aRectFnSet.GetRight(pHoriClipFrame->getFrameArea()));
            }
            else
            {
                // #i26945#
                const SwFrame *pClip =
                        const_cast<SwFlyFrame*>(pFly)->GetAnchorFrameContainingAnchPos();
                SwRectFnSet aRectFnSet(pClip);
                const SwLayoutFrame *pUp = pClip->GetUpper();
                const SwFrame *pCell = pUp->IsCellFrame() ? pUp : nullptr;
                const SwFrameType nType = bMove
                                     ? SwFrameType::Root   | SwFrameType::Fly | SwFrameType::Header |
                                       SwFrameType::Footer | SwFrameType::Ftn
                                     : SwFrameType::Body   | SwFrameType::Fly | SwFrameType::Header |
                                       SwFrameType::Footer | SwFrameType::Cell| SwFrameType::Ftn;

                while ( !(pUp->GetType() & nType) || pUp->IsColBodyFrame() )
                {
                    pUp = pUp->GetUpper();
                    if ( !pCell && pUp->IsCellFrame() )
                        pCell = pUp;
                }
                if ( bMove && pUp->IsRootFrame() )
                {
                    rRect  = pUp->getFramePrintArea();
                    rRect += pUp->getFrameArea().Pos();
                    pUp = nullptr;
                }
                if ( pUp )
                {
                    if ( pUp->GetType() & SwFrameType::Body )
                    {
                        const SwPageFrame *pPg;
                        if ( pUp->GetUpper() != (pPg = pFly->FindPageFrame()) )
                            pUp = pPg->FindBodyCont();
                        if (pUp)
                        {
                            rRect = pUp->GetUpper()->getFrameArea();
                            aRectFnSet.SetTop( rRect, aRectFnSet.GetPrtTop(*pUp) );
                            aRectFnSet.SetBottom(rRect, aRectFnSet.GetPrtBottom(*pUp));
                        }
                    }
                    else
                    {
                        if( ( pUp->GetType() & (SwFrameType::Fly | SwFrameType::Ftn ) ) &&
                            !pUp->getFrameArea().Contains( pFly->getFrameArea().Pos() ) )
                        {
                            if( pUp->IsFlyFrame() )
                            {
                                const SwFlyFrame *pTmpFly = static_cast<const SwFlyFrame*>(pUp);
                                while( pTmpFly->GetNextLink() )
                                {
                                    pTmpFly = pTmpFly->GetNextLink();
                                    if( pTmpFly->getFrameArea().Contains( pFly->getFrameArea().Pos() ) )
                                        break;
                                }
                                pUp = pTmpFly;
                            }
                            else if( pUp->IsInFootnote() )
                            {
                                const SwFootnoteFrame *pTmp = pUp->FindFootnoteFrame();
                                while( pTmp->GetFollow() )
                                {
                                    pTmp = pTmp->GetFollow();
                                    if( pTmp->getFrameArea().Contains( pFly->getFrameArea().Pos() ) )
                                        break;
                                }
                                pUp = pTmp;
                            }
                        }
                        rRect = pUp->getFramePrintArea();
                        rRect.Pos() += pUp->getFrameArea().Pos();
                        if ( pUp->GetType() & (SwFrameType::Header | SwFrameType::Footer) )
                        {
                            rRect.Left ( pUp->GetUpper()->getFrameArea().Left() );
                            rRect.Width( pUp->GetUpper()->getFrameArea().Width());
                        }
                        else if ( pUp->IsCellFrame() )                //MA_FLY_HEIGHT
                        {
                            const SwFrame *pTab = pUp->FindTabFrame();
                            aRectFnSet.SetBottom( rRect, aRectFnSet.GetPrtBottom(*pTab->GetUpper()) );
                            // expand to left and right cell border
                            rRect.Left ( pUp->getFrameArea().Left() );
                            rRect.Width( pUp->getFrameArea().Width() );
                        }
                    }
                }
                if ( pCell )
                {
                    // CellFrames might also sit in unallowed areas. In this case,
                    // the Fly is allowed to do so as well
                    SwRect aTmp( pCell->getFramePrintArea() );
                    aTmp += pCell->getFrameArea().Pos();
                    rRect.Union( aTmp );
                }
            }
        }
        else
        {
            const SwFrame *pUp = pFly->GetAnchorFrame()->GetUpper();
            SwRectFnSet aRectFnSet(pFly->GetAnchorFrame());
            while( pUp->IsColumnFrame() || pUp->IsSctFrame() || pUp->IsColBodyFrame())
                pUp = pUp->GetUpper();
            rRect = pUp->getFrameArea();
            if( !pUp->IsBodyFrame() )
            {
                rRect += pUp->getFramePrintArea().Pos();
                rRect.SSize( pUp->getFramePrintArea().SSize() );
                if ( pUp->IsCellFrame() )
                {
                    const SwFrame *pTab = pUp->FindTabFrame();
                    aRectFnSet.SetBottom( rRect, aRectFnSet.GetPrtBottom(*pTab->GetUpper()) );
                }
            }
            else if ( pUp->GetUpper()->IsPageFrame() )
            {
                // Objects anchored as character may exceed right margin
                // of body frame:
                aRectFnSet.SetRight( rRect, aRectFnSet.GetRight(pUp->GetUpper()->getFrameArea()) );
            }
            tools::Long nHeight = (9*aRectFnSet.GetHeight(rRect))/10;
            tools::Long nTop;
            const SwFormat *pFormat = GetUserCall(pSdrObj)->GetFormat();
            const SvxULSpaceItem &rUL = pFormat->GetULSpace();
            if( bMove )
            {
                nTop = aRectFnSet.IsVert() ? static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().X() :
                               static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().Y();
                nTop = aRectFnSet.YInc( nTop, -nHeight );
                tools::Long nWidth = aRectFnSet.GetWidth(pFly->getFrameArea());
                aRectFnSet.SetLeftAndWidth( rRect, aRectFnSet.IsVert() ?
                            static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().Y() :
                            static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().X(), nWidth );
                nHeight = 2*nHeight - rUL.GetLower() - rUL.GetUpper();
            }
            else
            {
                nTop = aRectFnSet.YInc( aRectFnSet.GetBottom(pFly->getFrameArea()),
                                           rUL.GetLower() - nHeight );
                nHeight = 2*nHeight - aRectFnSet.GetHeight(pFly->getFrameArea())
                          - rUL.GetLower() - rUL.GetUpper();
            }
            aRectFnSet.SetTopAndHeight( rRect, nTop, nHeight );
        }
    }
    else
    {
        const SwDrawContact *pC = static_cast<const SwDrawContact*>(GetUserCall(pSdrObj));
        const SwFrameFormat  *pFormat = pC->GetFormat();
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( RndStdIds::FLY_AS_CHAR == rAnch.GetAnchorId() )
        {
            const SwFrame* pAnchorFrame = pC->GetAnchorFrame( pSdrObj );
            if( !pAnchorFrame )
            {
                OSL_FAIL( "<::CalcClipRect(..)> - missing anchor frame." );
                const_cast<SwDrawContact*>(pC)->ConnectToLayout();
                pAnchorFrame = pC->GetAnchorFrame();
            }
            const SwFrame* pUp = pAnchorFrame->GetUpper();
            rRect = pUp->getFramePrintArea();
            rRect += pUp->getFrameArea().Pos();
            SwRectFnSet aRectFnSet(pAnchorFrame);
            tools::Long nHeight = (9*aRectFnSet.GetHeight(rRect))/10;
            tools::Long nTop;
            const SvxULSpaceItem &rUL = pFormat->GetULSpace();
            SwRect aSnapRect( pSdrObj->GetSnapRect() );
            tools::Long nTmpH = 0;
            if( bMove )
            {
                nTop = aRectFnSet.YInc( aRectFnSet.IsVert() ? pSdrObj->GetAnchorPos().X() :
                                       pSdrObj->GetAnchorPos().Y(), -nHeight );
                tools::Long nWidth = aRectFnSet.GetWidth(aSnapRect);
                aRectFnSet.SetLeftAndWidth( rRect, aRectFnSet.IsVert() ?
                            pSdrObj->GetAnchorPos().Y() :
                            pSdrObj->GetAnchorPos().X(), nWidth );
            }
            else
            {
                // #i26791# - value of <nTmpH> is needed to
                // calculate value of <nTop>.
                nTmpH = aRectFnSet.IsVert() ? pSdrObj->GetCurrentBoundRect().GetWidth() :
                                       pSdrObj->GetCurrentBoundRect().GetHeight();
                nTop = aRectFnSet.YInc( aRectFnSet.GetTop(aSnapRect),
                                          rUL.GetLower() + nTmpH - nHeight );
            }
            nHeight = 2*nHeight - nTmpH - rUL.GetLower() - rUL.GetUpper();
            aRectFnSet.SetTopAndHeight( rRect, nTop, nHeight );
        }
        else
        {
            // restrict clip rectangle for drawing
            // objects in header/footer to the page frame.
            // #i26791#
            const SwFrame* pAnchorFrame = pC->GetAnchorFrame( pSdrObj );
            if ( pAnchorFrame && pAnchorFrame->FindFooterOrHeader() )
            {
                // clip frame is the page frame the header/footer is on.
                const SwFrame* pClipFrame = pAnchorFrame->FindPageFrame();
                rRect = pClipFrame->getFrameArea();
            }
            else
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
