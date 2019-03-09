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
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/progress.hxx>
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
SdDisplay::SdDisplay(vcl::Window* pWin)
    : Control(pWin, 0)
    , aScale(1, 1)
{
    SetMapMode(MapMode(MapUnit::MapPixel));
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyles.GetFieldColor() ) );
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

void SdDisplay::Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& )
{
    Point aPt;
    Size aSize = GetOutputSize();
    Size aBmpSize = aBitmapEx.GetBitmap().GetSizePixel();
    aBmpSize.setWidth( static_cast<long>( static_cast<double>(aBmpSize.Width()) * static_cast<double>(aScale) ) );
    aBmpSize.setHeight( static_cast<long>( static_cast<double>(aBmpSize.Height()) * static_cast<double>(aScale) ) );

    if( aBmpSize.Width() < aSize.Width() )
        aPt.setX( ( aSize.Width() - aBmpSize.Width() ) / 2 );
    if( aBmpSize.Height() < aSize.Height() )
        aPt.setY( ( aSize.Height() - aBmpSize.Height() ) / 2 );

    aBitmapEx.Draw( this, aPt, aBmpSize );
}

void SdDisplay::SetScale( const Fraction& rFrac )
{
    aScale = rFrac;
}

Size SdDisplay::GetOptimalSize() const
{
    return LogicToPixel(Size(147, 87), MapMode(MapUnit::MapAppFont));
}

void SdDisplay::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
        SetBackground( Wallpaper( rStyles.GetFieldColor() ) );
        SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode()
            ? sd::OUTPUT_DRAWMODE_CONTRAST
            : sd::OUTPUT_DRAWMODE_COLOR );
    }
}

const size_t AnimationWindow::EMPTY_FRAMELIST = std::numeric_limits<size_t>::max();

/**
 *  AnimationWindow - FloatingWindow
 */
AnimationWindow::AnimationWindow(SfxBindings* pInBindings, SfxChildWindow *pCW, vcl::Window* pParent)
    : SfxDockingWindow(pInBindings, pCW, pParent,
        "DockingAnimation", "modules/simpress/ui/dockinganimation.ui")
    , m_nCurrentFrame(EMPTY_FRAMELIST)
    , bMovie(false)
    , bAllObjects(false)
{
    get(m_pBtnFirst, "first");
    get(m_pBtnReverse, "prev");
    get(m_pBtnStop, "stop");
    get(m_pBtnPlay, "next");
    get(m_pBtnLast, "last");
    get(m_pNumFldBitmap, "numbitmap");
    get(m_pTimeField, "duration");
    m_pTimeField->SetDuration(true);
    m_pTimeField->EnforceValidValue(true);
    m_pTimeField->SetMax(tools::Time(0, 0, 59, 99*tools::Time::nanoPerCenti));
    get(m_pLbLoopCount, "loopcount");
    get(m_pBtnGetOneObject, "getone");
    get(m_pBtnGetAllObjects, "getall");
    get(m_pBtnRemoveBitmap, "delone");
    get(m_pBtnRemoveAll, "delall");
    get(m_pFiCount, "count");
    get(m_pRbtGroup, "group");
    get(m_pRbtBitmap, "bitmap");
    get(m_pFtAdjustment, "alignmentft");
    get(m_pLbAdjustment, "alignment");
    get(m_pBtnCreateGroup, "create");

    m_pCtlDisplay = VclPtr<SdDisplay>::Create(get<Window>("box"));
    m_pCtlDisplay->set_hexpand(true);
    m_pCtlDisplay->set_vexpand(true);
    m_pCtlDisplay->Show();

    // create new document with page
    pMyDoc.reset( new SdDrawDocument(DocumentType::Impress, nullptr) );
    SdPage* pPage = pMyDoc->AllocSdPage(false);
    pMyDoc->InsertPage(pPage);

    pControllerItem.reset( new AnimationControllerItem( SID_ANIMATOR_STATE, this, pInBindings ) );

    // as long as not in the resource
    m_pTimeField->SetFormat( TimeFieldFormat::F_SEC_CS );

    m_pBtnFirst->SetClickHdl( LINK( this, AnimationWindow, ClickFirstHdl ) );
    m_pBtnReverse->SetClickHdl( LINK( this, AnimationWindow, ClickPlayHdl ) );
    m_pBtnStop->SetClickHdl( LINK( this, AnimationWindow, ClickStopHdl ) );
    m_pBtnPlay->SetClickHdl( LINK( this, AnimationWindow, ClickPlayHdl ) );
    m_pBtnLast->SetClickHdl( LINK( this, AnimationWindow, ClickLastHdl ) );

    m_pBtnGetOneObject->SetClickHdl( LINK( this, AnimationWindow, ClickGetObjectHdl ) );
    m_pBtnGetAllObjects->SetClickHdl( LINK( this, AnimationWindow, ClickGetObjectHdl ) );
    m_pBtnRemoveBitmap->SetClickHdl( LINK( this, AnimationWindow, ClickRemoveBitmapHdl ) );
    m_pBtnRemoveAll->SetClickHdl( LINK( this, AnimationWindow, ClickRemoveBitmapHdl ) );

    m_pRbtGroup->SetClickHdl( LINK( this, AnimationWindow, ClickRbtHdl ) );
    m_pRbtBitmap->SetClickHdl( LINK( this, AnimationWindow, ClickRbtHdl ) );
    m_pBtnCreateGroup->SetClickHdl( LINK( this, AnimationWindow, ClickCreateGroupHdl ) );
    m_pNumFldBitmap->SetModifyHdl( LINK( this, AnimationWindow, ModifyBitmapHdl ) );
    m_pTimeField->SetModifyHdl( LINK( this, AnimationWindow, ModifyTimeHdl ) );

    // disable 3D border
    m_pCtlDisplay->SetBorderStyle(WindowBorderStyle::MONO);

    SetMinOutputSizePixel(GetOptimalSize());

    ResetAttrs();

    // the animator is empty; no animation group can be created
    m_pBtnCreateGroup->Disable();
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

    m_pCtlDisplay.disposeAndClear();
    m_pBtnFirst.clear();
    m_pBtnReverse.clear();
    m_pBtnStop.clear();
    m_pBtnPlay.clear();
    m_pBtnLast.clear();
    m_pNumFldBitmap.clear();
    m_pTimeField.clear();
    m_pLbLoopCount.clear();
    m_pBtnGetOneObject.clear();
    m_pBtnGetAllObjects.clear();
    m_pBtnRemoveBitmap.clear();
    m_pBtnRemoveAll.clear();
    m_pFiCount.clear();
    m_pRbtGroup.clear();
    m_pRbtBitmap.clear();
    m_pFtAdjustment.clear();
    m_pLbAdjustment.clear();
    m_pBtnCreateGroup.clear();
    SfxDockingWindow::dispose();
}

IMPL_LINK_NOARG(AnimationWindow, ClickFirstHdl, Button*, void)
{
    m_nCurrentFrame = (m_FrameList.empty()) ? EMPTY_FRAMELIST : 0;
    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ClickStopHdl, Button*, void)
{
    bMovie = false;
}

IMPL_LINK( AnimationWindow, ClickPlayHdl, Button *, p, void )
{
    ScopeLockGuard aGuard( maPlayLock );

    bMovie = true;
    bool bDisableCtrls = false;
    size_t const nCount = m_FrameList.size();
    bool bReverse = p == m_pBtnReverse;

    // it is difficult to find it later on
    bool bRbtGroupEnabled = m_pRbtGroup->IsEnabled();
    bool bBtnGetAllObjectsEnabled = m_pBtnGetAllObjects->IsEnabled();
    bool bBtnGetOneObjectEnabled = m_pBtnGetOneObject->IsEnabled();

    // calculate overall time
    tools::Time aTime( 0 );
    long nFullTime;
    if( m_pRbtBitmap->IsChecked() )
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
        m_pBtnStop->Enable();
        m_pBtnStop->Update();
        OUString const aStr("Animator:"); // here we should think about something smart
        pProgress.reset(new SfxProgress( nullptr, aStr, nFullTime ));
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

        if( m_pRbtBitmap->IsChecked() )
        {
            tools::Time const & rTime = m_FrameList[i].second;

            m_pTimeField->SetTime( rTime );
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
        m_pBtnStop->Disable();
    }

    m_pRbtGroup->Enable( bRbtGroupEnabled );
    m_pBtnGetAllObjects->Enable( bBtnGetAllObjectsEnabled );
    m_pBtnGetOneObject->Enable( bBtnGetOneObjectEnabled );
}

IMPL_LINK_NOARG(AnimationWindow, ClickLastHdl, Button*, void)
{
    m_nCurrentFrame =
        (m_FrameList.empty()) ? EMPTY_FRAMELIST : m_FrameList.size() - 1 ;
    UpdateControl();
}

IMPL_LINK( AnimationWindow, ClickRbtHdl, Button*, p, void )
{
    if (m_FrameList.empty() || p == m_pRbtGroup || m_pRbtGroup->IsChecked())
    {
        m_pTimeField->SetText( OUString() );
        m_pTimeField->Enable( false );
        m_pLbLoopCount->Enable( false );
    }
    else if( p == m_pRbtBitmap || m_pRbtBitmap->IsChecked() )
    {
        sal_uLong n = static_cast<sal_uLong>(m_pNumFldBitmap->GetValue());
        if( n > 0 )
        {
            tools::Time const & rTime = m_FrameList[n - 1].second;
            m_pTimeField->SetTime( rTime );
        }
        m_pTimeField->Enable();
        m_pLbLoopCount->Enable();
    }
}

IMPL_LINK( AnimationWindow, ClickGetObjectHdl, Button*, pBtn, void )
{
    bAllObjects = pBtn == m_pBtnGetAllObjects;

    // Code now in AddObj()
    SfxBoolItem aItem( SID_ANIMATOR_ADD, true );

    GetBindings().GetDispatcher()->ExecuteList(
        SID_ANIMATOR_ADD, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK( AnimationWindow, ClickRemoveBitmapHdl, Button*, pBtn, void )
{
    SdPage*     pPage = pMyDoc->GetSdPage(0, PageKind::Standard);
    SdrObject*  pObject;

    // tdf#95298 check m_nCurrentFrame for EMPTY_FRAMELIST to avoid out-of-bound array access
    if (pBtn == m_pBtnRemoveBitmap && EMPTY_FRAMELIST  != m_nCurrentFrame)
    {
        m_FrameList.erase(m_FrameList.begin() + m_nCurrentFrame);

        pObject = pPage->GetObj(m_nCurrentFrame);
        // Through acquisition of the AnimatedGIFs, objects does not need to
        // exist.
        if( pObject )
        {
            pObject = pPage->RemoveObject(m_nCurrentFrame);
            DBG_ASSERT(pObject, "Clone not found during deletion");
            SdrObject::Free( pObject );
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
                    SdrObject::Free( pObject );
                    //pPage->RecalcObjOrdNums();
                }
            }
            m_FrameList.clear();
            m_nCurrentFrame = EMPTY_FRAMELIST;
        }
    }

    // can we create a animation group
    if (m_FrameList.empty())
    {
        m_pBtnCreateGroup->Disable();
        // if previous disabled by acquisition of AnimatedGIFs:
        //m_pRbtBitmap->Enable();
        m_pRbtGroup->Enable();
    }

    // calculate and set zoom for DisplayWin
    Fraction aFrac(GetScale());
    m_pCtlDisplay->SetScale(aFrac);

    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ClickCreateGroupHdl, Button*, void)
{
    // Code now in CreatePresObj()
    SfxBoolItem aItem( SID_ANIMATOR_CREATE, true );

    GetBindings().GetDispatcher()->ExecuteList(SID_ANIMATOR_CREATE,
            SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(AnimationWindow, ModifyBitmapHdl, Edit&, void)
{
    sal_uLong nBmp = static_cast<sal_uLong>(m_pNumFldBitmap->GetValue());

    if (nBmp > m_FrameList.size())
    {
        nBmp = m_FrameList.size();
    }

    m_nCurrentFrame = nBmp - 1;

    UpdateControl();
}

IMPL_LINK_NOARG(AnimationWindow, ModifyTimeHdl, Edit&, void)
{
    sal_uLong nPos = static_cast<sal_uLong>(m_pNumFldBitmap->GetValue() - 1);

    tools::Time & rTime = m_FrameList[nPos].second;

    rTime = m_pTimeField->GetTime();
}

void AnimationWindow::UpdateControl(bool const bDisableCtrls)
{
    // tdf#95298 check m_nCurrentFrame for EMPTY_FRAMELIST to avoid out-of-bound array access
    if (!m_FrameList.empty() && EMPTY_FRAMELIST != m_nCurrentFrame)
    {
        BitmapEx & rBmp(m_FrameList[m_nCurrentFrame].first);

        SdPage* pPage = pMyDoc->GetSdPage(0, PageKind::Standard);
        SdrObject *const pObject = pPage->GetObj(m_nCurrentFrame);
        if( pObject )
        {
            ScopedVclPtrInstance< VirtualDevice > pVD;
            ::tools::Rectangle       aObjRect( pObject->GetCurrentBoundRect() );
            Size            aObjSize( aObjRect.GetSize() );
            Point           aOrigin( Point( -aObjRect.Left(), -aObjRect.Top() ) );
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
            rBmp = pVD->GetBitmapEx( aObjRect.TopLeft(), aObjSize );
        }

        m_pCtlDisplay->SetBitmapEx(&rBmp);
    }
    else
    {
        m_pCtlDisplay->SetBitmapEx(nullptr);
    }
    m_pCtlDisplay->Invalidate();
    m_pCtlDisplay->Update();

    m_pFiCount->SetText(OUString::number(
                m_FrameList.size()));

    if (!m_FrameList.empty() && !bMovie)
    {
        size_t nIndex = m_nCurrentFrame + 1;
        m_pNumFldBitmap->SetValue(nIndex);

        // if there is at least 1 object in the list
        m_pBtnFirst->Enable();
        m_pBtnReverse->Enable();
        m_pBtnPlay->Enable();
        m_pBtnLast->Enable();
        m_pNumFldBitmap->Enable();
        m_pTimeField->Enable();
        m_pLbLoopCount->Enable();
        m_pBtnRemoveBitmap->Enable();
        m_pBtnRemoveAll->Enable();
    }
    else
    {
        // if no object is in the list
        m_pBtnFirst->Enable( false );
        m_pBtnReverse->Enable( false );
        m_pBtnPlay->Enable( false );
        m_pBtnLast->Enable( false );
        m_pNumFldBitmap->Enable( false );
        m_pTimeField->Enable( false );
        m_pLbLoopCount->Enable( false );
        m_pBtnRemoveBitmap->Enable( false );
        m_pBtnRemoveAll->Enable( false );
    }

    if( bMovie && bDisableCtrls )
    {
        m_pBtnGetOneObject->Enable( false );
        m_pBtnGetAllObjects->Enable( false );
        m_pRbtGroup->Enable( false );
        m_pRbtBitmap->Enable( false );
        m_pBtnCreateGroup->Enable( false );
        m_pFtAdjustment->Enable( false );
        m_pLbAdjustment->Enable( false );
    }
    else
    {
        // enable 'group object' only if it is not a Animated GIF
        if (m_FrameList.empty())
        {
            m_pRbtGroup->Enable();
        }

        m_pRbtBitmap->Enable();
        m_pBtnCreateGroup->Enable(!m_FrameList.empty());
        m_pFtAdjustment->Enable();
        m_pLbAdjustment->Enable();
    }

    ClickRbtHdl( nullptr );
}

void AnimationWindow::ResetAttrs()
{
    m_pRbtGroup->Check();
    m_pLbAdjustment->SelectEntryPos( BA_CENTER );
    // LoopCount
    m_pLbLoopCount->SelectEntryPos( m_pLbLoopCount->GetEntryCount() - 1);

    UpdateControl();
}

void AnimationWindow::WaitInEffect( sal_uLong nMilliSeconds, sal_uLong nTime,
                                    SfxProgress* pProgress ) const
{
    sal_uInt64 aEnd = tools::Time::GetSystemTicks() + nMilliSeconds;
    sal_uInt64 aCurrent = tools::Time::GetSystemTicks();
    while (aCurrent < aEnd)
    {
        aCurrent = tools::Time::GetSystemTicks();

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

        Size aDisplaySize(m_pCtlDisplay->GetOutputSize());

        aFrac = Fraction( std::min( static_cast<double>(aDisplaySize.Width()) / static_cast<double>(aBmpSize.Width()),
                             static_cast<double>(aDisplaySize.Height()) / static_cast<double>(aBmpSize.Height()) ) );
    }
    return aFrac;
}

void AnimationWindow::Resize()
{
    SfxDockingWindow::Resize();
    Fraction aFrac(GetScale());
    m_pCtlDisplay->SetScale(aFrac);
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
        sal_uInt16          nId = pObject->GetObjIdentifier();

        // Animated Bitmap (GIF)
        if( nInv == SdrInventor::Default && nId == OBJ_GRAF && static_cast<SdrGrafObj*>( pObject )->IsAnimated() )
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
                    const AnimationBitmap& rAnimBmp = aAnimation.Get( i );

                    // LoopCount
                    if( i == 0 )
                    {
                        sal_uInt32 nLoopCount = aAnimation.GetLoopCount();

                        if( !nLoopCount ) // endless
                            m_pLbLoopCount->SelectEntryPos( m_pLbLoopCount->GetEntryCount() - 1);
                        else
                            m_pLbLoopCount->SelectEntry(OUString::number( nLoopCount ) );
                    }

                    long nTime = rAnimBmp.nWait;
                    ::tools::Time aTime( 0, 0, nTime / 100, nTime % 100 );
                    size_t nIndex = m_nCurrentFrame + 1;
                    m_FrameList.insert(
                            m_FrameList.begin() + nIndex,
                            ::std::make_pair(rAnimBmp.aBmpEx, aTime));

                    // increment => next one inserted after this one
                    ++m_nCurrentFrame;
                }
                // if a animated GIF is taken, only such one can be created
                m_pRbtBitmap->Check();
                m_pRbtGroup->Enable( false );
                bAnimObj = true;
            }
        }
        else if( bAllObjects || ( pAnimInfo && pAnimInfo->mbIsMovie ) )
        {
            // several objects
            SdrObjList* pObjList = static_cast<SdrObjGroup*>(pObject)->GetSubList();

            for( size_t nObject = 0; nObject < pObjList->GetObjCount(); ++nObject )
            {
                SdrObject* pSnapShot(pObjList->GetObj(nObject));
                BitmapEx aBitmapEx(SdrExchangeView::GetObjGraphic(*pSnapShot).GetBitmapEx());
                size_t nIndex = m_nCurrentFrame + 1;
                m_FrameList.insert(
                        m_FrameList.begin() + nIndex,
                        ::std::make_pair(aBitmapEx, m_pTimeField->GetTime()));

                // increment => next one inserted after this one
                ++m_nCurrentFrame;

                // Clone
                pPage->InsertObject(
                    pSnapShot->CloneSdrObject(pPage->getSdrModelFromSdrPage()),
                    m_nCurrentFrame);
            }
            bAnimObj = true;
        }
    }
    // also one single animated object
    if( !bAnimObj && !( bAllObjects && nMarkCount > 1 ) )
    {
        BitmapEx aBitmapEx(rView.GetAllMarkedGraphic().GetBitmapEx());

        ::tools::Time aTime( m_pTimeField->GetTime() );

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
        SdrObject* pClone(pObject->CloneSdrObject(pPage->getSdrModelFromSdrPage()));
        size_t nIndex = m_nCurrentFrame + 1;
        pPage->InsertObject(pClone, nIndex);
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
                    ::std::make_pair(aBitmapEx, m_pTimeField->GetTime()));

                // increment => next one inserted after this one
                ++m_nCurrentFrame;

                pPage->InsertObject(
                    pObject->CloneSdrObject(pPage->getSdrModelFromSdrPage()),
                    m_nCurrentFrame);
            }
            bAnimObj = true; // that we don't change again
        }
        else
        {
            SdrObjGroup* pCloneGroup = new SdrObjGroup(rView.getSdrModelFromSdrView());
            SdrObjList*  pObjList    = pCloneGroup->GetSubList();

            for (size_t nObject= 0; nObject < nMarkCount; ++nObject)
            {
                pObjList->InsertObject(
                    rMarkList.GetMark(nObject)->GetMarkedSdrObj()->CloneSdrObject(
                        pPage->getSdrModelFromSdrPage()));
            }

            size_t nIndex = m_nCurrentFrame + 1;
            pPage->InsertObject(pCloneGroup, nIndex);
        }
    }

    if( !bAnimObj )
    {
        ++m_nCurrentFrame;
    }

    // if there was nothing in the animator before but now is something
    // there, we can create a animation group
    if (nCloneCount == 0 && !m_FrameList.empty())
    {
        m_pBtnCreateGroup->Enable();
    }

    // calculate and set zoom for DisplayWin
    Fraction aFrac( GetScale() );
    m_pCtlDisplay->SetScale(aFrac);

    UpdateControl();
}

void AnimationWindow::CreateAnimObj (::sd::View& rView )
{
    vcl::Window* pOutWin = static_cast< vcl::Window*>(rView.GetFirstOutputDevice()); // GetWin( 0 );
    DBG_ASSERT( pOutWin, "Window does not exist!" );

    // find window center
    const MapMode       aMap100( MapUnit::Map100thMM );
    Size                aMaxSizeLog;
    Size                aMaxSizePix;
    Size                aTemp( pOutWin->GetOutputSizePixel() );
    const Point         aWindowCenter( pOutWin->PixelToLogic( Point( aTemp.Width() >> 1, aTemp.Height() >> 1 ) ) );
    const OutputDevice* pDefDev = Application::GetDefaultDevice();
    const size_t nCount = m_FrameList.size();
    BitmapAdjustment    eBA = static_cast<BitmapAdjustment>(m_pLbAdjustment->GetSelectedEntryPos());

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

    if( m_pRbtBitmap->IsChecked() )
    {
        // create bitmap group (Animated GIF)
        Animation   aAnimation;
        Point       aPt;

        for (size_t i = 0; i < nCount; ++i)
        {
            tools::Time const & rTime = m_FrameList[i].second;
            long  nTime = rTime.GetNanoSec();
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
            AnimationBitmap aAnimBmp;
            sal_uInt32 nLoopCount = 0;
            sal_Int32 nPos = m_pLbLoopCount->GetSelectedEntryPos();

            if( nPos != LISTBOX_ENTRY_NOTFOUND && nPos != m_pLbLoopCount->GetEntryCount() - 1 ) // endless
                nLoopCount = m_pLbLoopCount->GetSelectedEntry().toUInt32();

            aAnimBmp.aBmpEx = rBitmapEx;
            aAnimBmp.aPosPix = aPt;
            aAnimBmp.aSizePix = aBitmapSize;
            aAnimBmp.nWait = nTime;
            aAnimBmp.eDisposal = Disposal::Back;
            aAnimBmp.bUserInput = false;

            aAnimation.Insert( aAnimBmp );
            aAnimation.SetDisplaySizePixel( aMaxSizePix );
            aAnimation.SetLoopCount( nLoopCount );
        }

        SdrGrafObj* pGrafObj = new SdrGrafObj(
            rView.getSdrModelFromSdrView(),
            Graphic(aAnimation));
        const Point aOrg( aWindowCenter.X() - ( aMaxSizeLog.Width() >> 1 ), aWindowCenter.Y() - ( aMaxSizeLog.Height() >> 1 ) );

        pGrafObj->SetLogicRect( ::tools::Rectangle( aOrg, aMaxSizeLog ) );
        rView.InsertObjectAtView( pGrafObj, *pPV, SdrInsertFlags::SETDEFLAYER);
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
            SdrObjGroup* pGroup   = new SdrObjGroup(rView.getSdrModelFromSdrView());
            SdrObjList*  pObjList = pGroup->GetSubList();

            for (size_t i = 0; i < nCount; ++i)
            {
                // the clone remains in the animation; we insert a clone of the
                // clone into the group
                pClone = pPage->GetObj(i);
                SdrObject* pCloneOfClone(pClone->CloneSdrObject(pPage->getSdrModelFromSdrPage()));
                //SdrObject* pCloneOfClone = pPage->GetObj(i)->Clone();
                pObjList->InsertObject(pCloneOfClone);
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

            // #i42894# if that worked, delete the group again
            if(!pGroup->GetSubList()->GetObjCount())
            {
                // always use SdrObject::Free(...) for SdrObjects (!)
                SdrObject* pTemp(pGroup);
                SdrObject::Free(pTemp);
            }
        }
    }

    ClickFirstHdl( nullptr );
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

void AnimationControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SfxItemState::DEFAULT && nSId == SID_ANIMATOR_STATE )
    {
        const SfxUInt16Item* pStateItem = dynamic_cast< const SfxUInt16Item*>( pItem );
        assert(pStateItem); //SfxUInt16Item expected
        if (pStateItem)
        {
            sal_uInt16 nState = pStateItem->GetValue();
            pAnimationWin->m_pBtnGetOneObject->Enable( nState & 1 );
            pAnimationWin->m_pBtnGetAllObjects->Enable( nState & 2 );
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
