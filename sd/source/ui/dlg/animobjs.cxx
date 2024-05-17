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

#include <time.h>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/progress.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>

#include <anminfo.hxx>
#include <animobjs.hxx>
#include <app.hrc>
#include <strings.hrc>
#include <sdresid.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

#include <ViewShell.hxx>

#include <vcl/settings.hxx>

#include <EffectMigration.hxx>

#include <algorithm>

using namespace ::com::sun::star;

namespace sd {

/**
 *  SdDisplay - Control
 */
SdDisplay::SdDisplay()
    : aScale(1, 1)
{
}

SdDisplay::~SdDisplay()
{
}

void SdDisplay::SetBitmapEx( BitmapEx const * pBmpEx )
{
    if( pBmpEx )
    {
        aBitmapEx = *pBmpEx;
    }
    else
    {
        const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
        const Color aFillColor = rStyles.GetFieldColor();
        aBitmapEx.Erase(aFillColor);
    }
}

void SdDisplay::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.Push(vcl::PushFlags::MAPMODE);

    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    rRenderContext.SetBackground( Wallpaper( rStyles.GetFieldColor() ) );
    rRenderContext.Erase();

    Point aPt;
    Size aSize = GetOutputSizePixel();

    Size aBmpSize = aBitmapEx.GetBitmap().GetSizePixel();
    aBmpSize.setWidth( static_cast<::tools::Long>( static_cast<double>(aBmpSize.Width()) * static_cast<double>(aScale) ) );
    aBmpSize.setHeight( static_cast<::tools::Long>( static_cast<double>(aBmpSize.Height()) * static_cast<double>(aScale) ) );

    if( aBmpSize.Width() < aSize.Width() )
        aPt.setX( ( aSize.Width() - aBmpSize.Width() ) / 2 );
    if( aBmpSize.Height() < aSize.Height() )
        aPt.setY( ( aSize.Height() - aBmpSize.Height() ) / 2 );

    aBitmapEx.Draw(&rRenderContext, aPt, aBmpSize);

    rRenderContext.Pop();
}

void SdDisplay::SetScale( const Fraction& rFrac )
{
    aScale = rFrac;
}

void SdDisplay::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(147, 87), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);
}

const size_t AnimationWindow::EMPTY_FRAMELIST = std::numeric_limits<size_t>::max();

/**
 *  AnimationWindow - FloatingWindow
 */
AnimationWindow::AnimationWindow(SfxBindings* pInBindings, SfxChildWindow *pCW, vcl::Window* pParent)
    : SfxDockingWindow(pInBindings, pCW, pParent,
        u"DockingAnimation"_ustr, u"modules/simpress/ui/dockinganimation.ui"_ustr)
    , m_xCtlDisplay(new SdDisplay)
    , m_xCtlDisplayWin(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, *m_xCtlDisplay))
    , m_xBtnFirst(m_xBuilder->weld_button(u"first"_ustr))
    , m_xBtnReverse(m_xBuilder->weld_button(u"prev"_ustr))
    , m_xBtnStop(m_xBuilder->weld_button(u"stop"_ustr))
    , m_xBtnPlay(m_xBuilder->weld_button(u"next"_ustr))
    , m_xBtnLast(m_xBuilder->weld_button(u"last"_ustr))
    , m_xNumFldBitmap(m_xBuilder->weld_spin_button(u"numbitmap"_ustr))
    , m_xTimeField(m_xBuilder->weld_formatted_spin_button(u"duration"_ustr))
    , m_xFormatter(new weld::TimeFormatter(*m_xTimeField))
    , m_xLbLoopCount(m_xBuilder->weld_combo_box(u"loopcount"_ustr))
    , m_xBtnGetOneObject(m_xBuilder->weld_button(u"getone"_ustr))
    , m_xBtnGetAllObjects(m_xBuilder->weld_button(u"getall"_ustr))
    , m_xBtnRemoveBitmap(m_xBuilder->weld_button(u"delone"_ustr))
    , m_xBtnRemoveAll(m_xBuilder->weld_button(u"delall"_ustr))
    , m_xFiCount(m_xBuilder->weld_label(u"count"_ustr))
    , m_xRbtGroup(m_xBuilder->weld_radio_button(u"group"_ustr))
    , m_xRbtBitmap(m_xBuilder->weld_radio_button(u"bitmap"_ustr))
    , m_xFtAdjustment(m_xBuilder->weld_label(u"alignmentft"_ustr))
    , m_xLbAdjustment(m_xBuilder->weld_combo_box(u"alignment"_ustr))
    , m_xBtnCreateGroup(m_xBuilder->weld_button(u"create"_ustr))
    , m_xBtnHelp(m_xBuilder->weld_button(u"help"_ustr))
    , m_nCurrentFrame(EMPTY_FRAMELIST)
    , bMovie(false)
    , bAllObjects(false)
{
    SetText(SdResId(STR_ANIMATION_DIALOG_TITLE));

    m_xFormatter->SetDuration(true);
    m_xFormatter->SetTimeFormat(TimeFieldFormat::F_SEC_CS);
    m_xFormatter->EnableEmptyField(false);

    // create new document with page
    pMyDoc.reset( new SdDrawDocument(DocumentType::Impress, nullptr) );
    rtl::Reference<SdPage> pPage = pMyDoc->AllocSdPage(false);
    pMyDoc->InsertPage(pPage.get());

    pControllerItem.reset( new AnimationControllerItem( SID_ANIMATOR_STATE, this, pInBindings ) );

    m_xBtnFirst->connect_clicked( LINK( this, AnimationWindow, ClickFirstHdl ) );
    m_xBtnReverse->connect_clicked( LINK( this, AnimationWindow, ClickPlayHdl ) );
    m_xBtnStop->connect_clicked( LINK( this, AnimationWindow, ClickStopHdl ) );
    m_xBtnPlay->connect_clicked( LINK( this, AnimationWindow, ClickPlayHdl ) );
    m_xBtnLast->connect_clicked( LINK( this, AnimationWindow, ClickLastHdl ) );

    m_xBtnGetOneObject->connect_clicked( LINK( this, AnimationWindow, ClickGetObjectHdl ) );
    m_xBtnGetAllObjects->connect_clicked( LINK( this, AnimationWindow, ClickGetObjectHdl ) );
    m_xBtnRemoveBitmap->connect_clicked( LINK( this, AnimationWindow, ClickRemoveBitmapHdl ) );
    m_xBtnRemoveAll->connect_clicked( LINK( this, AnimationWindow, ClickRemoveBitmapHdl ) );

    m_xRbtGroup->connect_toggled( LINK( this, AnimationWindow, ClickRbtHdl ) );
    m_xRbtBitmap->connect_toggled( LINK( this, AnimationWindow, ClickRbtHdl ) );
    m_xBtnCreateGroup->connect_clicked( LINK( this, AnimationWindow, ClickCreateGroupHdl ) );
    m_xBtnHelp->connect_clicked( LINK( this, AnimationWindow, ClickHelpHdl ) );
    m_xNumFldBitmap->connect_value_changed( LINK( this, AnimationWindow, ModifyBitmapHdl ) );
    m_xTimeField->connect_value_changed( LINK( this, AnimationWindow, ModifyTimeHdl ) );

    SetMinOutputSizePixel(GetOptimalSize());

    ResetAttrs();

    // the animator is empty; no animation group can be created
    m_xBtnCreateGroup->set_sensitive(false);
}

AnimationWindow::~AnimationWindow()
{
    disposeOnce();
}

void AnimationWindow::dispose()
{
    pControllerItem.reset();

    m_FrameList.clear();
    m_nCurrentFrame = EMPTY_FRAMELIST;

    // delete the clones
    pMyDoc.reset();

    m_xCtlDisplayWin.reset();
    m_xCtlDisplay.reset();
    m_xBtnFirst.reset();
    m_xBtnReverse.reset();
    m_xBtnStop.reset();
    m_xBtnPlay.reset();
    m_xBtnLast.reset();
    m_xNumFldBitmap.reset();
    m_xFormatter.reset();
    m_xTimeField.reset();
    m_xLbLoopCount.reset();
    m_xBtnGetOneObject.reset();
    m_xBtnGetAllObjects.reset();
    m_xBtnRemoveBitmap.reset();
    m_xBtnRemoveAll.reset();
    m_xFiCount.reset();
    m_xRbtGroup.reset();
    m_xRbtBitmap.reset();
    m_xFtAdjustment.reset();
    m_xLbAdjustment.reset();
    m_xBtnCreateGroup.reset();
    m_xBtnHelp.reset();
    SfxDockingWindow::dispose();
}

IMPL_LINK_NOARG(AnimationWindow, ClickFirstHdl, weld::Button&, void)
{
    m_nCurrentFrame = (m_FrameList.empty()) ? EMPTY_FRAMELIST : 0;
    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ClickStopHdl, weld::Button&, void)
{
    bMovie = false;
}

IMPL_LINK( AnimationWindow, ClickPlayHdl, weld::Button&, rButton, void )
{
    ScopeLockGuard aGuard( maPlayLock );

    bMovie = true;
    bool bDisableCtrls = false;
    size_t const nCount = m_FrameList.size();
    bool bReverse = &rButton == m_xBtnReverse.get();

    // it is difficult to find it later on
    bool bRbtGroupEnabled = m_xRbtGroup->get_sensitive();
    bool bBtnGetAllObjectsEnabled = m_xBtnGetAllObjects->get_sensitive();
    bool bBtnGetOneObjectEnabled = m_xBtnGetOneObject->get_sensitive();

    // calculate overall time
    ::tools::Time aTime( 0 );
    ::tools::Long nFullTime;
    if( m_xRbtBitmap->get_active() )
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            aTime += m_FrameList[i].second;
        }
        nFullTime  = aTime.GetMSFromTime();
    }
    else
    {
        nFullTime = nCount * 100;
        aTime.MakeTimeFromMS( nFullTime );
    }

    // StatusBarManager from 1 second
    std::unique_ptr<SfxProgress> pProgress;
    if( nFullTime >= 1000 )
    {
        bDisableCtrls = true;
        m_xBtnStop->set_sensitive(true);
        pProgress.reset(new SfxProgress( nullptr, u"Animator:"_ustr, nFullTime )); // "Animator:" here we should think about something smart
    }

    sal_uLong nTmpTime = 0;
    size_t i = 0;
    bool bCount = i < nCount;
    if( bReverse )
    {
        i = nCount - 1;
    }
    while( bCount && bMovie )
    {
        // make list and view consistent
        assert(i < m_FrameList.size());
        m_nCurrentFrame = i;

        UpdateControl(bDisableCtrls);

        if( m_xRbtBitmap->get_active() )
        {
            ::tools::Time const & rTime = m_FrameList[i].second;

            m_xFormatter->SetTime( rTime );
            sal_uLong nTime = rTime.GetMSFromTime();

            WaitInEffect( nTime, nTmpTime, pProgress.get() );
            nTmpTime += nTime;
        }
        else
        {
            WaitInEffect( 100, nTmpTime, pProgress.get() );
            nTmpTime += 100;
        }
        if( bReverse )
        {
            if (i == 0)
            {
                // Terminate loop.
                bCount = false;
            }
            else
            {
                --i;
            }
        }
        else
        {
            i++;
            if (i >= nCount)
            {
                // Terminate loop.
                bCount = false;
                // Move i back into valid range.
                i = nCount - 1;
            }
        }
    }

    // to re-enable the controls
    bMovie = false;
    if (nCount > 0)
    {
        assert(i == m_nCurrentFrame);
        UpdateControl();
    }

    if( pProgress )
    {
        pProgress.reset();
        m_xBtnStop->set_sensitive(false);
    }

    m_xRbtGroup->set_sensitive( bRbtGroupEnabled );
    m_xBtnGetAllObjects->set_sensitive( bBtnGetAllObjectsEnabled );
    m_xBtnGetOneObject->set_sensitive( bBtnGetOneObjectEnabled );
}

IMPL_LINK_NOARG(AnimationWindow, ClickLastHdl, weld::Button&, void)
{
    m_nCurrentFrame =
        (m_FrameList.empty()) ? EMPTY_FRAMELIST : m_FrameList.size() - 1 ;
    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ClickRbtHdl, weld::Toggleable&, void)
{
    if (m_FrameList.empty() || m_xRbtGroup->get_active())
    {
        m_xTimeField->set_text( OUString() );
        m_xTimeField->set_sensitive( false );
        m_xLbLoopCount->set_sensitive( false );
    }
    else if (m_xRbtBitmap->get_active())
    {
        sal_uLong n = m_xNumFldBitmap->get_value();
        if( n > 0 )
        {
            ::tools::Time const & rTime = m_FrameList[n - 1].second;
            m_xFormatter->SetTime( rTime );
            m_xFormatter->ReFormat();
        }
        m_xTimeField->set_sensitive(true);
        m_xLbLoopCount->set_sensitive(true);
    }
}

IMPL_LINK(AnimationWindow, ClickHelpHdl, weld::Button&, rButton, void)
{
    if (Help* pHelp = Application::GetHelp())
        pHelp->Start(m_xContainer->get_help_id(), &rButton);
}

IMPL_LINK( AnimationWindow, ClickGetObjectHdl, weld::Button&, rBtn, void )
{
    bAllObjects = &rBtn == m_xBtnGetAllObjects.get();

    // Code now in AddObj()
    SfxBoolItem aItem( SID_ANIMATOR_ADD, true );

    GetBindings().GetDispatcher()->ExecuteList(
        SID_ANIMATOR_ADD, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK( AnimationWindow, ClickRemoveBitmapHdl, weld::Button&, rBtn, void )
{
    SdPage*     pPage = pMyDoc->GetSdPage(0, PageKind::Standard);
    rtl::Reference<SdrObject> pObject;

    // tdf#95298 check m_nCurrentFrame for EMPTY_FRAMELIST to avoid out-of-bound array access
    if (&rBtn == m_xBtnRemoveBitmap.get() && EMPTY_FRAMELIST  != m_nCurrentFrame)
    {
        m_FrameList.erase(m_FrameList.begin() + m_nCurrentFrame);

        pObject = pPage->GetObj(m_nCurrentFrame);
        // Through acquisition of the AnimatedGIFs, objects does not need to
        // exist.
        if( pObject )
        {
            pObject = pPage->RemoveObject(m_nCurrentFrame);
            DBG_ASSERT(pObject, "Clone not found during deletion");
            pObject.clear();
            pPage->RecalcObjOrdNums();
        }

        if (m_nCurrentFrame >= m_FrameList.size())
        {
            // tdf#95298 last frame was deleted, try to use the one before it or go on empty state
            m_nCurrentFrame = m_FrameList.empty() ? EMPTY_FRAMELIST : m_FrameList.size() - 1;
        }
    }
    else // delete everything
    {
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                   SdResId(STR_ASK_DELETE_ALL_PICTURES)));
        short nReturn = xWarn->run();

        if( nReturn == RET_YES )
        {
            // clear frame list
            for (size_t i = m_FrameList.size(); i > 0; )
            {
                --i;
                pObject = pPage->GetObj( i );
                if( pObject )
                {
                    pObject = pPage->RemoveObject( i );
                    DBG_ASSERT(pObject, "Clone not found during deletion");
                    pObject.clear();
                    //pPage->RecalcObjOrdNums();
                }
            }
            m_FrameList.clear();
            m_nCurrentFrame = EMPTY_FRAMELIST;
        }
    }

    // can we create an animation group
    if (m_FrameList.empty())
    {
        m_xBtnCreateGroup->set_sensitive(false);
        // if previous disabled by acquisition of AnimatedGIFs:
        //m_xRbtBitmap->set_sensitive(true);
        m_xRbtGroup->set_sensitive(true);
    }

    // calculate and set zoom for DisplayWin
    Fraction aFrac(GetScale());
    m_xCtlDisplay->SetScale(aFrac);

    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ClickCreateGroupHdl, weld::Button&, void)
{
    // Code now in CreatePresObj()
    SfxBoolItem aItem( SID_ANIMATOR_CREATE, true );

    GetBindings().GetDispatcher()->ExecuteList(SID_ANIMATOR_CREATE,
            SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(AnimationWindow, ModifyBitmapHdl, weld::SpinButton&, void)
{
    sal_uLong nBmp = m_xNumFldBitmap->get_value();

    if (nBmp > m_FrameList.size())
    {
        nBmp = m_FrameList.size();
    }

    m_nCurrentFrame = nBmp - 1;

    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ModifyTimeHdl, weld::FormattedSpinButton&, void)
{
    sal_uLong nPos = m_xNumFldBitmap->get_value() - 1;

    ::tools::Time & rTime = m_FrameList[nPos].second;

    rTime = m_xFormatter->GetTime();
}

void AnimationWindow::UpdateControl(bool const bDisableCtrls)
{
    // tdf#95298 check m_nCurrentFrame for EMPTY_FRAMELIST to avoid out-of-bound array access
    if (!m_FrameList.empty() && EMPTY_FRAMELIST != m_nCurrentFrame)
    {
        BitmapEx aBmp(m_FrameList[m_nCurrentFrame].first);

        SdPage* pPage = pMyDoc->GetSdPage(0, PageKind::Standard);
        SdrObject *const pObject = pPage->GetObj(m_nCurrentFrame);
        if( pObject )
        {
            ScopedVclPtrInstance< VirtualDevice > pVD;
            ::tools::Rectangle       aObjRect( pObject->GetCurrentBoundRect() );
            Size            aObjSize( aObjRect.GetSize() );
            Point           aOrigin( -aObjRect.Left(), -aObjRect.Top() );
            MapMode         aMap( pVD->GetMapMode() );
            aMap.SetMapUnit( MapUnit::Map100thMM );
            aMap.SetOrigin( aOrigin );
            pVD->SetMapMode( aMap );
            pVD->SetOutputSize( aObjSize );
            const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
            pVD->SetBackground( Wallpaper( rStyles.GetFieldColor() ) );
            pVD->SetDrawMode( rStyles.GetHighContrastMode()
                ? sd::OUTPUT_DRAWMODE_CONTRAST
                : sd::OUTPUT_DRAWMODE_COLOR );
            pVD->Erase();
            pObject->SingleObjectPainter( *pVD );
            aBmp = pVD->GetBitmapEx( aObjRect.TopLeft(), aObjSize );
        }

        m_xCtlDisplay->SetBitmapEx(&aBmp);
    }
    else
    {
        m_xCtlDisplay->SetBitmapEx(nullptr);
    }

    m_xCtlDisplay->Invalidate();

    m_xFiCount->set_label(OUString::number(
                m_FrameList.size()));

    if (!m_FrameList.empty() && !bMovie)
    {
        size_t nIndex = m_nCurrentFrame + 1;
        m_xNumFldBitmap->set_value(nIndex);

        // if there is at least 1 object in the list
        m_xBtnFirst->set_sensitive(true);
        m_xBtnReverse->set_sensitive(true);
        m_xBtnPlay->set_sensitive(true);
        m_xBtnLast->set_sensitive(true);
        m_xNumFldBitmap->set_sensitive(true);
        m_xTimeField->set_sensitive(true);
        m_xLbLoopCount->set_sensitive(true);
        m_xBtnRemoveBitmap->set_sensitive(true);
        m_xBtnRemoveAll->set_sensitive(true);
    }
    else
    {
        // if no object is in the list
        m_xBtnFirst->set_sensitive( false );
        m_xBtnReverse->set_sensitive( false );
        m_xBtnPlay->set_sensitive( false );
        m_xBtnLast->set_sensitive( false );
        m_xNumFldBitmap->set_sensitive( false );
        m_xTimeField->set_sensitive( false );
        m_xLbLoopCount->set_sensitive( false );
        m_xBtnRemoveBitmap->set_sensitive( false );
        m_xBtnRemoveAll->set_sensitive( false );
    }

    if( bMovie && bDisableCtrls )
    {
        m_xBtnGetOneObject->set_sensitive( false );
        m_xBtnGetAllObjects->set_sensitive( false );
        m_xRbtGroup->set_sensitive( false );
        m_xRbtBitmap->set_sensitive( false );
        m_xBtnCreateGroup->set_sensitive( false );
        m_xFtAdjustment->set_sensitive( false );
        m_xLbAdjustment->set_sensitive( false );
    }
    else
    {
        // enable 'group object' only if it is not an Animated GIF
        if (m_FrameList.empty())
        {
            m_xRbtGroup->set_sensitive(true);
        }

        m_xRbtBitmap->set_sensitive(true);
        m_xBtnCreateGroup->set_sensitive(!m_FrameList.empty());
        m_xFtAdjustment->set_sensitive(true);
        m_xLbAdjustment->set_sensitive(true);
    }

    ClickRbtHdl(*m_xRbtGroup);
}

void AnimationWindow::ResetAttrs()
{
    m_xRbtGroup->set_active(true);
    m_xLbAdjustment->set_active( BA_CENTER );
    // LoopCount
    m_xLbLoopCount->set_active( m_xLbLoopCount->get_count() - 1);

    UpdateControl();
}

void AnimationWindow::WaitInEffect( sal_uLong nMilliSeconds, sal_uLong nTime,
                                    SfxProgress* pProgress ) const
{
    sal_uInt64 aEnd = ::tools::Time::GetSystemTicks() + nMilliSeconds;
    sal_uInt64 aCurrent = ::tools::Time::GetSystemTicks();
    while (aCurrent < aEnd)
    {
        aCurrent = ::tools::Time::GetSystemTicks();

        if( pProgress )
            pProgress->SetState( nTime + nMilliSeconds + aCurrent - aEnd );

        Application::Reschedule();

        if( !bMovie )
            return;
    }
}

Fraction AnimationWindow::GetScale()
{
    Fraction aFrac;
    size_t const nCount = m_FrameList.size();
    if (nCount > 0)
    {
        Size aBmpSize(0, 0);
        for (size_t i = 0; i < nCount; i++)
        {
            BitmapEx const & rBitmap = m_FrameList[i].first;
            Size aTempSize( rBitmap.GetBitmap().GetSizePixel() );
            aBmpSize.setWidth( std::max( aBmpSize.Width(), aTempSize.Width() ) );
            aBmpSize.setHeight( std::max( aBmpSize.Height(), aTempSize.Height() ) );
        }

        aBmpSize.AdjustWidth(10 );
        aBmpSize.AdjustHeight(10 );

        Size aDisplaySize(m_xCtlDisplay->GetOutputSizePixel());

        aFrac = Fraction( std::min( static_cast<double>(aDisplaySize.Width()) / static_cast<double>(aBmpSize.Width()),
                             static_cast<double>(aDisplaySize.Height()) / static_cast<double>(aBmpSize.Height()) ) );
    }
    return aFrac;
}

void AnimationWindow::Resize()
{
    SfxDockingWindow::Resize();
    Fraction aFrac(GetScale());
    m_xCtlDisplay->SetScale(aFrac);
}

bool AnimationWindow::Close()
{
    if( maPlayLock.isLocked() )
    {
        return false;
    }
    else
    {
        SfxBoolItem aItem( SID_ANIMATION_OBJECTS, false );

        GetBindings().GetDispatcher()->ExecuteList(
            SID_ANIMATION_OBJECTS, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aItem });

        SfxDockingWindow::Close();

        return true;
    }
}

void AnimationWindow::AddObj (::sd::View& rView )
{
    // finish text entry mode to ensure that bitmap is identical with object
    if( rView.IsTextEdit() )
        rView.SdrEndTextEdit();

    // clone object(s) and insert the clone(s) into the list
    const SdrMarkList& rMarkList   = rView.GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();
    SdPage*            pPage       = pMyDoc->GetSdPage(0, PageKind::Standard);
    const size_t nCloneCount = pPage->GetObjCount();

    if (nMarkCount <= 0)
        return;

    // If it is ONE animation object or one group object, which was
    // 'individually taken', we insert the objects separately
    bool bAnimObj = false;
    if( nMarkCount == 1 )
    {
        SdrMark*            pMark = rMarkList.GetMark(0);
        SdrObject*          pObject = pMark->GetMarkedSdrObj();
        SdAnimationInfo*    pAnimInfo = SdDrawDocument::GetAnimationInfo( pObject );
        SdrInventor         nInv = pObject->GetObjInventor();
        SdrObjKind          nId = pObject->GetObjIdentifier();

        // Animated Bitmap (GIF)
        if( nInv == SdrInventor::Default && nId == SdrObjKind::Graphic && static_cast<SdrGrafObj*>( pObject )->IsAnimated() )
        {
            const SdrGrafObj*   pGrafObj = static_cast<SdrGrafObj*>(pObject);
            Graphic             aGraphic( pGrafObj->GetTransformedGraphic() );
            sal_uInt16              nCount = 0;

            if( aGraphic.IsAnimated() )
                nCount = aGraphic.GetAnimation().Count();

            if( nCount > 0 )
            {
                const Animation aAnimation( aGraphic.GetAnimation() );

                for( sal_uInt16 i = 0; i < nCount; i++ )
                {
                    const AnimationFrame& rAnimationFrame = aAnimation.Get( i );

                    // LoopCount
                    if( i == 0 )
                    {
                        sal_uInt32 nLoopCount = aAnimation.GetLoopCount();

                        if( !nLoopCount ) // endless
                            m_xLbLoopCount->set_active( m_xLbLoopCount->get_count() - 1);
                        else
                            m_xLbLoopCount->set_active_text(OUString::number( nLoopCount ) );
                    }

                    ::tools::Long nTime = rAnimationFrame.mnWait;
                    ::tools::Time aTime( 0, 0, nTime / 100, nTime % 100 );
                    size_t nIndex = m_nCurrentFrame + 1;
                    m_FrameList.insert(
                            m_FrameList.begin() + nIndex,
                            ::std::make_pair(rAnimationFrame.maBitmapEx, aTime));

                    // increment => next one inserted after this one
                    ++m_nCurrentFrame;
                }
                // if an animated GIF is taken, only such one can be created
                m_xRbtBitmap->set_active(true);
                m_xRbtGroup->set_sensitive( false );
                bAnimObj = true;
            }
        }
        else if( bAllObjects || ( pAnimInfo && pAnimInfo->mbIsMovie ) )
        {
            // several objects
            SdrObjList* pObjList = static_cast<SdrObjGroup*>(pObject)->GetSubList();

            for (const rtl::Reference<SdrObject>& pSnapShot : *pObjList)
            {
                BitmapEx aBitmapEx(SdrExchangeView::GetObjGraphic(*pSnapShot).GetBitmapEx());
                size_t nIndex = m_nCurrentFrame + 1;
                m_FrameList.insert(
                        m_FrameList.begin() + nIndex,
                        ::std::make_pair(aBitmapEx, m_xFormatter->GetTime()));

                // increment => next one inserted after this one
                ++m_nCurrentFrame;

                // Clone
                pPage->InsertObject(
                    pSnapShot->CloneSdrObject(pPage->getSdrModelFromSdrPage()).get(),
                    m_nCurrentFrame);
            }
            bAnimObj = true;
        }
    }
    // also one single animated object
    if( !bAnimObj && !( bAllObjects && nMarkCount > 1 ) )
    {
        BitmapEx aBitmapEx(rView.GetAllMarkedGraphic().GetBitmapEx());

        ::tools::Time aTime( m_xFormatter->GetTime() );

        size_t nIndex = m_nCurrentFrame + 1;
        m_FrameList.insert(
                m_FrameList.begin() + nIndex,
                ::std::make_pair(aBitmapEx, aTime));
    }

    // one single object
    if( nMarkCount == 1 && !bAnimObj )
    {
        SdrMark*    pMark   = rMarkList.GetMark(0);
        SdrObject*  pObject = pMark->GetMarkedSdrObj();
        rtl::Reference<SdrObject> pClone(pObject->CloneSdrObject(pPage->getSdrModelFromSdrPage()));
        size_t nIndex = m_nCurrentFrame + 1;
        pPage->InsertObject(pClone.get(), nIndex);
    }
    // several objects: group the clones
    else if (nMarkCount > 1)
    {
        // take objects separately
        if( bAllObjects )
        {
            for( size_t nObject= 0; nObject < nMarkCount; ++nObject )
            {
                // Clone
                SdrObject* pObject(rMarkList.GetMark(nObject)->GetMarkedSdrObj());
                BitmapEx aBitmapEx(SdrExchangeView::GetObjGraphic(*pObject).GetBitmapEx());
                size_t nIndex = m_nCurrentFrame + 1;
                m_FrameList.insert(
                    m_FrameList.begin() + nIndex,
                    ::std::make_pair(aBitmapEx, m_xFormatter->GetTime()));

                // increment => next one inserted after this one
                ++m_nCurrentFrame;

                pPage->InsertObject(
                    pObject->CloneSdrObject(pPage->getSdrModelFromSdrPage()).get(),
                    m_nCurrentFrame);
            }
            bAnimObj = true; // that we don't change again
        }
        else
        {
            rtl::Reference<SdrObjGroup> pCloneGroup = new SdrObjGroup(rView.getSdrModelFromSdrView());
            SdrObjList*  pObjList    = pCloneGroup->GetSubList();

            for (size_t nObject= 0; nObject < nMarkCount; ++nObject)
            {
                pObjList->InsertObject(
                    rMarkList.GetMark(nObject)->GetMarkedSdrObj()->CloneSdrObject(
                        pPage->getSdrModelFromSdrPage()).get());
            }

            size_t nIndex = m_nCurrentFrame + 1;
            pPage->InsertObject(pCloneGroup.get(), nIndex);
        }
    }

    if( !bAnimObj )
    {
        ++m_nCurrentFrame;
    }

    // if there was nothing in the animator before but now is something
    // there, we can create an animation group
    if (nCloneCount == 0 && !m_FrameList.empty())
    {
        m_xBtnCreateGroup->set_sensitive(true);
    }

    // calculate and set zoom for DisplayWin
    Fraction aFrac( GetScale() );
    m_xCtlDisplay->SetScale(aFrac);

    UpdateControl();
}

void AnimationWindow::CreateAnimObj (::sd::View& rView )
{
    vcl::Window* pOutWin = rView.GetFirstOutputDevice()->GetOwnerWindow(); // GetWin( 0 );
    DBG_ASSERT( pOutWin, "Window does not exist!" );

    // find window center
    const MapMode       aMap100( MapUnit::Map100thMM );
    Size                aMaxSizeLog;
    Size                aMaxSizePix;
    Size                aTemp( pOutWin->GetOutputSizePixel() );
    const Point         aWindowCenter( pOutWin->PixelToLogic( Point( aTemp.Width() >> 1, aTemp.Height() >> 1 ) ) );
    const OutputDevice* pDefDev = Application::GetDefaultDevice();
    const size_t nCount = m_FrameList.size();
    BitmapAdjustment    eBA = static_cast<BitmapAdjustment>(m_xLbAdjustment->get_active());

    // find biggest bitmap
    for (size_t i = 0; i < nCount; ++i)
    {
        const BitmapEx& rBmpEx = m_FrameList[i].first;
        const Graphic   aGraphic( rBmpEx );
        Size            aTmpSizeLog;
        const Size      aTmpSizePix( rBmpEx.GetSizePixel() );

        if ( aGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
            aTmpSizeLog = pDefDev->PixelToLogic( aGraphic.GetPrefSize(), aMap100 );
        else
            aTmpSizeLog = OutputDevice::LogicToLogic( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), aMap100 );

        aMaxSizeLog.setWidth( std::max( aMaxSizeLog.Width(), aTmpSizeLog.Width() ) );
        aMaxSizeLog.setHeight( std::max( aMaxSizeLog.Height(), aTmpSizeLog.Height() ) );

        aMaxSizePix.setWidth( std::max( aMaxSizePix.Width(), aTmpSizePix.Width() ) );
        aMaxSizePix.setHeight( std::max( aMaxSizePix.Height(), aTmpSizePix.Height() ) );
    }

    SdrPageView* pPV = rView.GetSdrPageView();

    if( m_xRbtBitmap->get_active() )
    {
        // create bitmap group (Animated GIF)
        Animation   aAnimation;
        Point       aPt;

        for (size_t i = 0; i < nCount; ++i)
        {
            ::tools::Time const & rTime = m_FrameList[i].second;
            ::tools::Long  nTime = rTime.GetNanoSec();
            nTime += rTime.GetSec() * 100;

            BitmapEx const & rBitmapEx = m_FrameList[i].first;

            // calculate offset for the specified direction
            const Size aBitmapSize( rBitmapEx.GetSizePixel() );

            switch( eBA )
            {
                case BA_LEFT_UP:
                break;

                case BA_LEFT:
                    aPt.setY( (aMaxSizePix.Height() - aBitmapSize.Height()) >> 1 );
                break;

                case BA_LEFT_DOWN:
                    aPt.setY( aMaxSizePix.Height() - aBitmapSize.Height() );
                break;

                case BA_UP:
                    aPt.setX( (aMaxSizePix.Width() - aBitmapSize.Width()) >> 1 );
                break;

                case BA_CENTER:
                    aPt.setX( (aMaxSizePix.Width() - aBitmapSize.Width()) >> 1 );
                    aPt.setY( (aMaxSizePix.Height() - aBitmapSize.Height()) >> 1 );
                break;

                case BA_DOWN:
                    aPt.setX( (aMaxSizePix.Width() - aBitmapSize.Width()) >> 1 );
                    aPt.setY( aMaxSizePix.Height() - aBitmapSize.Height() );
                break;

                case BA_RIGHT_UP:
                    aPt.setX( aMaxSizePix.Width() - aBitmapSize.Width() );
                break;

                case BA_RIGHT:
                    aPt.setX( aMaxSizePix.Width() - aBitmapSize.Width() );
                    aPt.setY( (aMaxSizePix.Height() - aBitmapSize.Height()) >> 1 );
                break;

                case BA_RIGHT_DOWN:
                    aPt.setX( aMaxSizePix.Width() - aBitmapSize.Width() );
                    aPt.setY( aMaxSizePix.Height() - aBitmapSize.Height() );
                break;

            }

            // find LoopCount (number of passes)
            AnimationFrame aAnimationFrame;
            sal_uInt32 nLoopCount = 0;
            sal_Int32 nPos = m_xLbLoopCount->get_active();

            if( nPos != -1 && nPos != m_xLbLoopCount->get_count() - 1 ) // endless
                nLoopCount = m_xLbLoopCount->get_active_text().toUInt32();

            aAnimationFrame.maBitmapEx = rBitmapEx;
            aAnimationFrame.maPositionPixel = aPt;
            aAnimationFrame.maSizePixel = aBitmapSize;
            aAnimationFrame.mnWait = nTime;
            aAnimationFrame.meDisposal = Disposal::Back;
            aAnimationFrame.mbUserInput = false;

            aAnimation.Insert( aAnimationFrame );
            aAnimation.SetDisplaySizePixel( aMaxSizePix );
            aAnimation.SetLoopCount( nLoopCount );
        }

        rtl::Reference<SdrGrafObj> pGrafObj = new SdrGrafObj(
            rView.getSdrModelFromSdrView(),
            Graphic(aAnimation));
        const Point aOrg( aWindowCenter.X() - ( aMaxSizeLog.Width() >> 1 ), aWindowCenter.Y() - ( aMaxSizeLog.Height() >> 1 ) );

        pGrafObj->SetLogicRect( ::tools::Rectangle( aOrg, aMaxSizeLog ) );
        rView.InsertObjectAtView( pGrafObj.get(), *pPV, SdrInsertFlags::SETDEFLAYER);
    }
    else
    {
        // calculate offset for the specified direction
        Size aOffset;
        SdrObject * pClone = nullptr;
        SdPage* pPage = pMyDoc->GetSdPage(0, PageKind::Standard);

        for (size_t i = 0; i < nCount; ++i)
        {
            pClone = pPage->GetObj(i);
            ::tools::Rectangle aRect( pClone->GetSnapRect() );

            switch( eBA )
            {
                case BA_LEFT_UP:
                break;

                case BA_LEFT:
                    aOffset.setHeight( (aMaxSizeLog.Height() - aRect.GetHeight()) / 2 );
                break;

                case BA_LEFT_DOWN:
                    aOffset.setHeight( aMaxSizeLog.Height() - aRect.GetHeight() );
                break;

                case BA_UP:
                    aOffset.setWidth( (aMaxSizeLog.Width() - aRect.GetWidth()) / 2 );
                break;

                case BA_CENTER:
                    aOffset.setWidth( (aMaxSizeLog.Width() - aRect.GetWidth()) / 2 );
                    aOffset.setHeight( (aMaxSizeLog.Height() - aRect.GetHeight()) / 2 );
                break;

                case BA_DOWN:
                    aOffset.setWidth( (aMaxSizeLog.Width() - aRect.GetWidth()) / 2 );
                    aOffset.setHeight( aMaxSizeLog.Height() - aRect.GetHeight() );
                break;

                case BA_RIGHT_UP:
                    aOffset.setWidth( aMaxSizeLog.Width() - aRect.GetWidth() );
                break;

                case BA_RIGHT:
                    aOffset.setWidth( aMaxSizeLog.Width() - aRect.GetWidth() );
                    aOffset.setHeight( (aMaxSizeLog.Height() - aRect.GetHeight()) / 2 );
                break;

                case BA_RIGHT_DOWN:
                    aOffset.setWidth( aMaxSizeLog.Width() - aRect.GetWidth() );
                    aOffset.setHeight( aMaxSizeLog.Height() - aRect.GetHeight() );
                break;

            }
            // Unfortunately, SetSnapRect is not implemented for ellipses !!!
            Point aMovePt( aWindowCenter + Point( aOffset.Width(), aOffset.Height() ) - aRect.TopLeft() );
            Size aMoveSize( aMovePt.X(), aMovePt.Y() );
            pClone->NbcMove( aMoveSize );
        }

        // #i42894# Caution(!) variable pPage looks right, but it is a page from the local
        // document the dialog is using (!), so get the target page from the target view
        SdPage* pTargetSdPage = dynamic_cast< SdPage* >(rView.GetSdrPageView() ? rView.GetSdrPageView()->GetPage() : nullptr);

        if(pTargetSdPage)
        {
            // create animation group
            rtl::Reference<SdrObjGroup> pGroup   = new SdrObjGroup(rView.getSdrModelFromSdrView());
            SdrObjList*  pObjList = pGroup->GetSubList();

            for (size_t i = 0; i < nCount; ++i)
            {
                // the clone remains in the animation; we insert a clone of the
                // clone into the group
                pClone = pPage->GetObj(i);
                rtl::Reference<SdrObject> pCloneOfClone(pClone->CloneSdrObject(pTargetSdPage->getSdrModelFromSdrPage()));
                //SdrObject* pCloneOfClone = pPage->GetObj(i)->Clone();
                pObjList->InsertObject(pCloneOfClone.get());
            }

            // until now the top left corner of the group is in the window center;
            // correct the position by half of the size of the group
            aTemp = aMaxSizeLog;
            aTemp.setHeight( - aTemp.Height() / 2 );
            aTemp.setWidth( - aTemp.Width() / 2 );
            pGroup->NbcMove(aTemp);

            // #i42894# create needed SMIL stuff and move child objects to page directly (see
            // comments at EffectMigration::CreateAnimatedGroup why this has to be done).
            EffectMigration::CreateAnimatedGroup(*pGroup, *pTargetSdPage);
        }
    }

    ClickFirstHdl(*m_xBtnFirst);
}

void AnimationWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxDockingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        UpdateControl();
    }
}

/**
 * ControllerItem for Animator
 */
AnimationControllerItem::AnimationControllerItem(
    sal_uInt16 _nId,
    AnimationWindow* pAnimWin,
    SfxBindings*    _pBindings)
    : SfxControllerItem( _nId, *_pBindings ),
      pAnimationWin( pAnimWin )
{
}

void AnimationControllerItem::StateChangedAtToolBoxControl( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SfxItemState::DEFAULT && nSId == SID_ANIMATOR_STATE )
    {
        const SfxUInt16Item* pStateItem = dynamic_cast< const SfxUInt16Item*>( pItem );
        assert(pStateItem); //SfxUInt16Item expected
        if (pStateItem)
        {
            sal_uInt16 nState = pStateItem->GetValue();
            pAnimationWin->m_xBtnGetOneObject->set_sensitive( nState & 1 );
            pAnimationWin->m_xBtnGetAllObjects->set_sensitive( nState & 2 );
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
