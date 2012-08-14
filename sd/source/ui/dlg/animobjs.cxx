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

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <svx/xoutbmp.hxx>
#include <time.h>
#include <svl/eitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/progress.hxx>
#include <vcl/msgbox.hxx>
#include "anminfo.hxx"
#include "animobjs.hxx"
#include "animobjs.hrc"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "View.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "res_bmp.hrc"
#include "ViewShell.hxx"

#include <vcl/svapp.hxx>

#include <string>
#include <algorithm>

using namespace ::com::sun::star;

namespace sd {

/*************************************************************************
|*  SdDisplay - Control
\************************************************************************/

SdDisplay::SdDisplay( Window* pWin, SdResId Id ) :
        Control( pWin, Id ),
        aScale( 1, 1 )
{
    SetMapMode( MAP_PIXEL );
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( Color( rStyles.GetFieldColor() ) ) );
}

// -----------------------------------------------------------------------

SdDisplay::~SdDisplay()
{
}

// -----------------------------------------------------------------------

void SdDisplay::SetBitmapEx( BitmapEx* pBmpEx )
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

// -----------------------------------------------------------------------

void SdDisplay::Paint( const Rectangle& )
{
    Point aPt;
    Size aSize = GetOutputSize();
    Size aBmpSize = aBitmapEx.GetBitmap().GetSizePixel();
    aBmpSize.Width() = (long) ( (double) aBmpSize.Width() * (double) aScale );
    aBmpSize.Height() = (long) ( (double) aBmpSize.Height() * (double) aScale );

    if( aBmpSize.Width() < aSize.Width() )
        aPt.X() = ( aSize.Width() - aBmpSize.Width() ) / 2;
    if( aBmpSize.Height() < aSize.Height() )
        aPt.Y() = ( aSize.Height() - aBmpSize.Height() ) / 2;

    aBitmapEx.Draw( this, aPt, aBmpSize );
}

// -----------------------------------------------------------------------

void SdDisplay::SetScale( const Fraction& rFrac )
{
    aScale = rFrac;
}

void SdDisplay::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
        SetBackground( Wallpaper( Color( rStyles.GetFieldColor() ) ) );
        SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode()
            ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
            : ViewShell::OUTPUT_DRAWMODE_COLOR );
    }
}

/*************************************************************************
|*  AnimationWindow - FloatingWindow
\************************************************************************/

AnimationWindow::AnimationWindow( SfxBindings* pInBindings,
                SfxChildWindow *pCW, Window* pParent, const SdResId& rSdResId ) :
        SfxDockingWindow    ( pInBindings, pCW, pParent, rSdResId ),
        aCtlDisplay         ( this, SdResId( CTL_DISPLAY ) ),
        aBtnFirst           ( this, SdResId( BTN_FIRST ) ),
        aBtnReverse         ( this, SdResId( BTN_REVERSE ) ),
        aBtnStop            ( this, SdResId( BTN_STOP ) ),
        aBtnPlay            ( this, SdResId( BTN_PLAY ) ),
        aBtnLast            ( this, SdResId( BTN_LAST ) ),
        aNumFldBitmap       ( this, SdResId( NUM_FLD_BITMAP ) ),
        aTimeField          ( this, SdResId( TIME_FIELD ) ),
        aLbLoopCount        ( this, SdResId( LB_LOOP_COUNT ) ),
        aGrpBitmap          ( this, SdResId( GRP_BITMAP ) ),
        aBtnGetOneObject    ( this, SdResId( BTN_GET_ONE_OBJECT ) ),
        aBtnGetAllObjects   ( this, SdResId( BTN_GET_ALL_OBJECTS ) ),
        aBtnRemoveBitmap    ( this, SdResId( BTN_REMOVE_BITMAP ) ),
        aBtnRemoveAll       ( this, SdResId( BTN_REMOVE_ALL ) ),
        aFtCount            ( this, SdResId( FT_COUNT ) ),
        aFiCount            ( this, SdResId( FI_COUNT ) ),
        aGrpAnimation       ( this, SdResId( GRP_ANIMATION_GROUP ) ),
        aRbtGroup           ( this, SdResId( RBT_GROUP ) ),
        aRbtBitmap          ( this, SdResId( RBT_BITMAP ) ),
        aFtAdjustment       ( this, SdResId( FT_ADJUSTMENT ) ),
        aLbAdjustment       ( this, SdResId( LB_ADJUSTMENT ) ),
        aBtnCreateGroup     ( this, SdResId( BTN_CREATE_GROUP ) ),

        pWin                ( pParent ),
        m_nCurrentFrame     ( EMPTY_FRAMELIST ),
        pBitmapEx           ( NULL ),

        bMovie              ( sal_False ),
        bAllObjects         ( sal_False ),

        pBindings           ( pInBindings )
{
    aCtlDisplay.SetAccessibleName(String (SdResId(STR_DISPLAY)));
    FreeResource();

    // neues Dokument mit Seite erzeugen
    pMyDoc = new SdDrawDocument(DOCUMENT_TYPE_IMPRESS, NULL);
    SdPage* pPage = (SdPage*) pMyDoc->AllocPage(sal_False);
    pMyDoc->InsertPage(pPage);

    pControllerItem = new AnimationControllerItem( SID_ANIMATOR_STATE, this, pBindings );

    // Solange noch nicht in der Resource
    aTimeField.SetFormat( TIMEF_SEC_CS );

    aBtnFirst.SetClickHdl( LINK( this, AnimationWindow, ClickFirstHdl ) );
    aBtnReverse.SetClickHdl( LINK( this, AnimationWindow, ClickPlayHdl ) );
    aBtnStop.SetClickHdl( LINK( this, AnimationWindow, ClickStopHdl ) );
    aBtnPlay.SetClickHdl( LINK( this, AnimationWindow, ClickPlayHdl ) );
    aBtnLast.SetClickHdl( LINK( this, AnimationWindow, ClickLastHdl ) );

    aBtnGetOneObject.SetClickHdl( LINK( this, AnimationWindow, ClickGetObjectHdl ) );
    aBtnGetAllObjects.SetClickHdl( LINK( this, AnimationWindow, ClickGetObjectHdl ) );
    aBtnRemoveBitmap.SetClickHdl( LINK( this, AnimationWindow, ClickRemoveBitmapHdl ) );
    aBtnRemoveAll.SetClickHdl( LINK( this, AnimationWindow, ClickRemoveBitmapHdl ) );

    aRbtGroup.SetClickHdl( LINK( this, AnimationWindow, ClickRbtHdl ) );
    aRbtBitmap.SetClickHdl( LINK( this, AnimationWindow, ClickRbtHdl ) );
    aBtnCreateGroup.SetClickHdl( LINK( this, AnimationWindow, ClickCreateGroupHdl ) );
    aNumFldBitmap.SetModifyHdl( LINK( this, AnimationWindow, ModifyBitmapHdl ) );
    aTimeField.SetModifyHdl( LINK( this, AnimationWindow, ModifyTimeHdl ) );

    // disable 3D border
    aCtlDisplay.SetBorderStyle(WINDOW_BORDER_MONO);
    aDisplaySize = aCtlDisplay.GetOutputSize();

    aSize = GetOutputSizePixel();
    SetMinOutputSizePixel( aSize );

    ResetAttrs();

    // der Animator ist leer; es kann keine Animationsgruppe erstellt werden
    aBtnCreateGroup.Disable();

    aBtnGetOneObject.SetAccessibleRelationMemberOf( &aGrpBitmap );
    aBtnGetAllObjects.SetAccessibleRelationMemberOf( &aGrpBitmap );
    aBtnRemoveBitmap.SetAccessibleRelationMemberOf( &aGrpBitmap );
    aBtnRemoveAll.SetAccessibleRelationMemberOf( &aGrpBitmap );
}

// -----------------------------------------------------------------------

AnimationWindow::~AnimationWindow()
{
    delete pControllerItem;

    for (size_t i = 0; i < m_FrameList.size(); ++i)
    {
        delete m_FrameList[i].first;
        delete m_FrameList[i].second;
    }
    m_FrameList.clear();
    m_nCurrentFrame = EMPTY_FRAMELIST;

    // die Clones loeschen
    delete pMyDoc;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(AnimationWindow, ClickFirstHdl)
{
    pBitmapEx = (m_FrameList.empty()) ? 0 : m_FrameList.front().first;
    m_nCurrentFrame = (pBitmapEx) ? 0 : EMPTY_FRAMELIST;
    UpdateControl(m_nCurrentFrame);

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(AnimationWindow, ClickStopHdl)
{
    bMovie = sal_False;
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickPlayHdl, void *, p )
{
    ScopeLockGuard aGuard( maPlayLock );

    bMovie = sal_True;
    sal_Bool bDisableCtrls = sal_False;
    size_t const nCount = m_FrameList.size();
    sal_Bool bReverse = p == &aBtnReverse;

    // Kann spaeter schwer ermittelt werden
    sal_Bool bRbtGroupEnabled = aRbtGroup.IsEnabled();
    sal_Bool bBtnGetAllObjectsEnabled = aBtnGetAllObjects.IsEnabled();
    sal_Bool bBtnGetOneObjectEnabled = aBtnGetOneObject.IsEnabled();

    // Gesamtzeit ermitteln
    Time aTime( 0 );
    long nFullTime;
    if( aRbtBitmap.IsChecked() )
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            aTime += *m_FrameList[i].second;
        }
        nFullTime  = aTime.GetMSFromTime();
    }
    else
    {
        nFullTime = nCount * 100;
        aTime.MakeTimeFromMS( nFullTime );
    }

    // StatusBarManager ab 1 Sekunde
    SfxProgress* pProgress = NULL;
    if( nFullTime >= 1000 )
    {
        bDisableCtrls = sal_True;
        aBtnStop.Enable();
        aBtnStop.Update();
        rtl::OUString aStr("Animator:"); // Hier sollte man sich noch etwas gescheites ausdenken!
        pProgress = new SfxProgress( NULL, aStr, nFullTime );
    }

    sal_uLong nTmpTime = 0;
    size_t i = 0;
    sal_Bool bCount = i < nCount;
    if( bReverse )
    {
        i = nCount - 1;
    }
    while( bCount && bMovie )
    {
        // make list and view consistent
        assert(0 < i && i < m_FrameList.size());
        m_nCurrentFrame = i;
        pBitmapEx = m_FrameList[i].first;

        UpdateControl( i, bDisableCtrls );

        if( aRbtBitmap.IsChecked() )
        {
            Time *const pTime = m_FrameList[i].second;
            assert(pTime);

            aTimeField.SetTime( *pTime );
            sal_uLong nTime = pTime->GetMSFromTime();

            WaitInEffect( nTime, nTmpTime, pProgress );
            nTmpTime += nTime;
        }
        else
        {
            WaitInEffect( 100, nTmpTime, pProgress );
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

    // Um die Controls wieder zu enablen
    bMovie = sal_False;
    if (nCount > 0)
        UpdateControl(i);

    if( pProgress )
    {
        delete pProgress;
        aBtnStop.Disable();
    }

    aRbtGroup.Enable( bRbtGroupEnabled );
    aBtnGetAllObjects.Enable( bBtnGetAllObjectsEnabled );
    aBtnGetOneObject.Enable( bBtnGetOneObjectEnabled );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(AnimationWindow, ClickLastHdl)
{
    pBitmapEx = (m_FrameList.empty()) ? 0 : m_FrameList.back().first;
    m_nCurrentFrame = (pBitmapEx) ? m_FrameList.size() - 1 : EMPTY_FRAMELIST;
    UpdateControl(m_nCurrentFrame);

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickRbtHdl, void *, p )
{
    if( !pBitmapEx || p == &aRbtGroup || aRbtGroup.IsChecked() )
    {
        aTimeField.SetText( String() );
        aTimeField.Enable( sal_False );
        aLbLoopCount.Enable( sal_False );
    }
    else if( p == &aRbtBitmap || aRbtBitmap.IsChecked() )
    {
        sal_uLong n = static_cast<sal_uLong>(aNumFldBitmap.GetValue());
        if( n > 0 )
        {
            Time *const pTime = m_FrameList[n - 1].second;
            if( pTime )
                aTimeField.SetTime( *pTime );
        }
        aTimeField.Enable();
        aLbLoopCount.Enable();
    }

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickGetObjectHdl, void *, pBtn )
{
    bAllObjects = pBtn == &aBtnGetAllObjects;

    // Code jetzt in AddObj()
    SfxBoolItem aItem( SID_ANIMATOR_ADD, sal_True );

    GetBindings().GetDispatcher()->Execute(
        SID_ANIMATOR_ADD, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickRemoveBitmapHdl, void *, pBtn )
{
    SdPage*     pPage = pMyDoc->GetSdPage(0, PK_STANDARD);
    SdrObject*  pObject;

    if( pBtn == &aBtnRemoveBitmap )
    {
        pBitmapEx = m_FrameList[m_nCurrentFrame].first;
        delete pBitmapEx;
        Time *const pTime = m_FrameList[m_nCurrentFrame].second;
        delete pTime;
        m_FrameList.erase(m_FrameList.begin() + m_nCurrentFrame);

        pObject = pPage->GetObj(m_nCurrentFrame);
        // Durch Uebernahme der AnimatedGIFs muessen nicht unbedingt
        // Objekte vorhanden sein.
        if( pObject )
        {
            pObject = pPage->RemoveObject(m_nCurrentFrame);
            DBG_ASSERT(pObject, "Clone beim Loeschen nicht gefunden");
            SdrObject::Free( pObject );
            pPage->RecalcObjOrdNums();
        }

        if (m_nCurrentFrame >= m_FrameList.size())
        {
            assert(m_FrameList.empty());
            m_nCurrentFrame = EMPTY_FRAMELIST;
        }
    }
    else // Alles loeschen
    {
        WarningBox aWarnBox( this, WB_YES_NO, String( SdResId( STR_ASK_DELETE_ALL_PICTURES ) ) );
        short nReturn = aWarnBox.Execute();

        if( nReturn == RET_YES )
        {
            // clear frame list
            for (size_t i = m_FrameList.size(); i > 0; )
            {
                --i;
                pBitmapEx = m_FrameList[i].first;
                delete pBitmapEx;

                pObject = pPage->GetObj( i );
                if( pObject )
                {
                    pObject = pPage->RemoveObject( i );
                    DBG_ASSERT(pObject, "Clone beim Loeschen nicht gefunden");
                    SdrObject::Free( pObject );
                    //pPage->RecalcObjOrdNums();
                }

                delete m_FrameList[i].second;
            }
            m_FrameList.clear();
            m_nCurrentFrame = EMPTY_FRAMELIST;
        }
    }

    // kann noch eine Animationsgruppe erstellt werden?
    if (m_FrameList.empty())
    {
        aBtnCreateGroup.Disable();
        // Falls vorher durch Uebernahme von AnimatedGIFs disabled:
        //aRbtBitmap.Enable();
        aRbtGroup.Enable();
    }

    // Zoom fuer DisplayWin berechnen und setzen
    Fraction aFrac( GetScale() );
    aCtlDisplay.SetScale( aFrac );

    UpdateControl(m_nCurrentFrame);

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(AnimationWindow, ClickCreateGroupHdl)
{
    // Code jetzt in CreatePresObj()
    SfxBoolItem aItem( SID_ANIMATOR_CREATE, sal_True );

    GetBindings().GetDispatcher()->Execute(
        SID_ANIMATOR_CREATE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(AnimationWindow, ModifyBitmapHdl)
{
    sal_uLong nBmp = static_cast<sal_uLong>(aNumFldBitmap.GetValue());

    if (nBmp > m_FrameList.size())
    {
        nBmp = m_FrameList.size();
    }

    m_nCurrentFrame = nBmp - 1;
    pBitmapEx = m_FrameList[m_nCurrentFrame].first;

    UpdateControl(m_nCurrentFrame);

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(AnimationWindow, ModifyTimeHdl)
{
    sal_uLong nPos = static_cast<sal_uLong>(aNumFldBitmap.GetValue() - 1);

    Time *const pTime = m_FrameList[nPos].second;

    *pTime = aTimeField.GetTime();

    return( 0L );
}

// -----------------------------------------------------------------------

void AnimationWindow::UpdateControl( sal_uLong nListPos, sal_Bool bDisableCtrls )
{
    String aString;

    if( pBitmapEx )
    {
        BitmapEx aBmp( *pBitmapEx );

        SdPage* pPage = pMyDoc->GetSdPage(0, PK_STANDARD);
        SdrObject* pObject = (SdrObject*) pPage->GetObj( (sal_uLong) nListPos );
        if( pObject )
        {
            VirtualDevice   aVD;
            Rectangle       aObjRect( pObject->GetCurrentBoundRect() );
            Size            aObjSize( aObjRect.GetSize() );
            Point           aOrigin( Point( -aObjRect.Left(), -aObjRect.Top() ) );
            MapMode         aMap( aVD.GetMapMode() );
            aMap.SetMapUnit( MAP_100TH_MM );
            aMap.SetOrigin( aOrigin );
            aVD.SetMapMode( aMap );
            aVD.SetOutputSize( aObjSize );
            const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
            aVD.SetBackground( Wallpaper( rStyles.GetFieldColor() ) );
            aVD.SetDrawMode( rStyles.GetHighContrastMode()
                ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
                : ViewShell::OUTPUT_DRAWMODE_COLOR );
            aVD.Erase();
            pObject->SingleObjectPainter( aVD );
            aBmp = BitmapEx( aVD.GetBitmap( aObjRect.TopLeft(), aObjSize ) );
        }


        aCtlDisplay.SetBitmapEx( &aBmp );
    }
    else
    {
        aCtlDisplay.SetBitmapEx( pBitmapEx );
    }
    aCtlDisplay.Invalidate();
    aCtlDisplay.Update();

    aFiCount.SetText(rtl::OUString::valueOf(
                static_cast<sal_Int64>(m_FrameList.size())));

    if( pBitmapEx && !bMovie )
    {
        aNumFldBitmap.SetValue( nListPos + 1 );

        // Wenn mind. 1 Objekt in der Liste ist
        aBtnFirst.Enable();
        aBtnReverse.Enable();
        aBtnPlay.Enable();
        aBtnLast.Enable();
        aNumFldBitmap.Enable();
        aTimeField.Enable();
        aLbLoopCount.Enable();
        aBtnRemoveBitmap.Enable();
        aBtnRemoveAll.Enable();
    }
    else
    {
        // Wenn kein Objekt in der Liste ist
        aBtnFirst.Enable( sal_False );
        aBtnReverse.Enable( sal_False );
        aBtnPlay.Enable( sal_False );
        aBtnLast.Enable( sal_False );
        aNumFldBitmap.Enable( sal_False );
        aTimeField.Enable( sal_False );
        aLbLoopCount.Enable( sal_False );
        aBtnRemoveBitmap.Enable( sal_False );
        aBtnRemoveAll.Enable( sal_False );

        //aFtAdjustment.Enable();
        //aLbAdjustment.Enable();
    }

    if( bMovie && bDisableCtrls )
    {
        aBtnGetOneObject.Enable( sal_False );
        aBtnGetAllObjects.Enable( sal_False );
        aRbtGroup.Enable( sal_False );
        aRbtBitmap.Enable( sal_False );
        aBtnCreateGroup.Enable( sal_False );
        aFtAdjustment.Enable( sal_False );
        aLbAdjustment.Enable( sal_False );
    }
    else
    {
        // 'Gruppenobjekt' nur dann enablen, wenn es kein Animated GIF ist
        if (m_FrameList.empty())
        {
            aRbtGroup.Enable();
        }

        aRbtBitmap.Enable();
        aBtnCreateGroup.Enable(!m_FrameList.empty());
        aFtAdjustment.Enable( sal_True );
        aLbAdjustment.Enable( sal_True );
    }

    ClickRbtHdl( NULL );
}

// -----------------------------------------------------------------------

void AnimationWindow::ResetAttrs()
{
    aRbtGroup.Check();
    aLbAdjustment.SelectEntryPos( BA_CENTER );
    // LoopCount
    aLbLoopCount.SelectEntryPos( aLbLoopCount.GetEntryCount() - 1);

    UpdateControl( 0 );
}

// -----------------------------------------------------------------------

void AnimationWindow::WaitInEffect( sal_uLong nMilliSeconds, sal_uLong nTime,
                                    SfxProgress* pProgress ) const
{
    clock_t aEnd = Time::GetSystemTicks() + nMilliSeconds;
    clock_t aCurrent = Time::GetSystemTicks();
    while (aCurrent < aEnd)
    {
        aCurrent = Time::GetSystemTicks();

        if( pProgress )
            pProgress->SetState( nTime + nMilliSeconds + aCurrent - aEnd );

        Application::Reschedule();

        if( !bMovie )
            return;
    }
}

// -----------------------------------------------------------------------

Fraction AnimationWindow::GetScale()
{
    Fraction aFrac;
    size_t const nCount = m_FrameList.size();
    if (nCount > 0)
    {
        aBmpSize.Width() = 0;
        aBmpSize.Height() = 0;
        for (size_t i = 0; i < nCount; i++)
        {
            BitmapEx *const pBitmap = m_FrameList[i].first;
            Size aTempSize( pBitmap->GetBitmap().GetSizePixel() );
            aBmpSize.Width() = Max( aBmpSize.Width(), aTempSize.Width() );
            aBmpSize.Height() = Max( aBmpSize.Height(), aTempSize.Height() );
        }

        aBmpSize.Width() += 10;
        aBmpSize.Height() += 10;

        aFrac = Fraction( std::min( (double)aDisplaySize.Width() / (double)aBmpSize.Width(),
                             (double)aDisplaySize.Height() / (double)aBmpSize.Height() ) );
    }
    return( aFrac );
}

// -----------------------------------------------------------------------

void AnimationWindow::Resize()
{
    if ( !IsFloatingMode() ||
         !GetFloatingWindow()->IsRollUp() )
    {
        Size aWinSize( GetOutputSizePixel() ); // vorher rSize im Resizing()

        Size aDiffSize;
        aDiffSize.Width() = aWinSize.Width() - aSize.Width();
        aDiffSize.Height() = aWinSize.Height() - aSize.Height();

        // Umgroessern des Display-Controls
        aDisplaySize.Width() += aDiffSize.Width();
        aDisplaySize.Height() += aDiffSize.Height();
        aCtlDisplay.SetOutputSizePixel( aDisplaySize );

        Point aPt;
        aPt.Y() = aDiffSize.Height();

        // Verschieben der anderen Controls
        aBtnFirst.Hide();
        aBtnReverse.Hide();
        aBtnStop.Hide();
        aBtnPlay.Hide();
        aBtnLast.Hide();
        aTimeField.Hide();
        aLbLoopCount.Hide();
        aNumFldBitmap.Hide();
        aFtCount.Hide();
        aFiCount.Hide();
        aBtnGetOneObject.Hide();
        aBtnGetAllObjects.Hide();
        aBtnRemoveBitmap.Hide();
        aBtnRemoveAll.Hide();
        aGrpBitmap.Hide();
        aRbtGroup.Hide();
        aRbtBitmap.Hide();
        aFtAdjustment.Hide();
        aLbAdjustment.Hide();
        aBtnCreateGroup.Hide();
        aGrpAnimation.Hide();


        aBtnFirst.SetPosPixel( aBtnFirst.GetPosPixel() + aPt );
        aBtnReverse.SetPosPixel( aBtnReverse.GetPosPixel() + aPt );
        aBtnStop.SetPosPixel( aBtnStop.GetPosPixel() + aPt );
        aBtnPlay.SetPosPixel( aBtnPlay.GetPosPixel() + aPt );
        aBtnLast.SetPosPixel( aBtnLast.GetPosPixel() + aPt );
        aNumFldBitmap.SetPosPixel( aNumFldBitmap.GetPosPixel() + aPt );
        aTimeField.SetPosPixel( aTimeField.GetPosPixel() + aPt );
        aLbLoopCount.SetPosPixel( aLbLoopCount.GetPosPixel() + aPt );
        aFtCount.SetPosPixel( aFtCount.GetPosPixel() + aPt );
        aFiCount.SetPosPixel( aFiCount.GetPosPixel() + aPt );
        aRbtGroup.SetPosPixel( aRbtGroup.GetPosPixel() + aPt );
        aRbtBitmap.SetPosPixel( aRbtBitmap.GetPosPixel() + aPt );
        aFtAdjustment.SetPosPixel( aFtAdjustment.GetPosPixel() + aPt );
        aLbAdjustment.SetPosPixel( aLbAdjustment.GetPosPixel() + aPt );
        aBtnGetOneObject.SetPosPixel( aBtnGetOneObject.GetPosPixel() + aPt );
        aBtnGetAllObjects.SetPosPixel( aBtnGetAllObjects.GetPosPixel() + aPt );
        aBtnRemoveBitmap.SetPosPixel( aBtnRemoveBitmap.GetPosPixel() + aPt );
        aBtnRemoveAll.SetPosPixel( aBtnRemoveAll.GetPosPixel() + aPt );
        aBtnCreateGroup.SetPosPixel( aBtnCreateGroup.GetPosPixel() + aPt );
        aGrpBitmap.SetPosPixel( aGrpBitmap.GetPosPixel() + aPt );
        aGrpAnimation.SetPosPixel( aGrpAnimation.GetPosPixel() + aPt );

        // Zoom fuer DisplayWin berechnen und setzen
        Fraction aFrac( GetScale() );
        aCtlDisplay.SetScale( aFrac );

        aBtnFirst.Show();
        aBtnReverse.Show();
        aBtnStop.Show();
        aBtnPlay.Show();
        aBtnLast.Show();
        aNumFldBitmap.Show();
        aTimeField.Show();
        aLbLoopCount.Show();
        aFtCount.Show();
        aFiCount.Show();
        aFtAdjustment.Show();
        aLbAdjustment.Show();
        aBtnGetOneObject.Show();
        aBtnGetAllObjects.Show();
        aBtnRemoveBitmap.Show();
        aBtnRemoveAll.Show();
        aGrpBitmap.Show();
        aRbtGroup.Show();
        aRbtBitmap.Show();
        aFtAdjustment.Show();
        aLbAdjustment.Show();
        aBtnCreateGroup.Show();
        aGrpAnimation.Show();

        aSize = aWinSize;
    }
    SfxDockingWindow::Resize();
}

// -----------------------------------------------------------------------

sal_Bool AnimationWindow::Close()
{
    if( maPlayLock.isLocked() )
    {
        return sal_False;
    }
    else
    {
        SfxBoolItem aItem( SID_ANIMATION_OBJECTS, sal_False );

        GetBindings().GetDispatcher()->Execute(
            SID_ANIMATION_OBJECTS, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

        SfxDockingWindow::Close();

        return sal_True;
    }
}

// -----------------------------------------------------------------------

void AnimationWindow::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo ) ;
}

// -----------------------------------------------------------------------

void AnimationWindow::AddObj (::sd::View& rView )
{
    // Texteingabemodus beenden, damit Bitmap mit
    // Objekt identisch ist.
    if( rView.IsTextEdit() )
        rView.SdrEndTextEdit();

    // Objekt(e) clonen und den/die Clone(s) in die Liste stellen
    const SdrMarkList& rMarkList   = rView.GetMarkedObjectList();
    sal_uLong              nMarkCount  = rMarkList.GetMarkCount();
    SdPage*            pPage       = pMyDoc->GetSdPage(0, PK_STANDARD);
    sal_uLong              nCloneCount = pPage->GetObjCount();

    if (nMarkCount > 0)
    {
        // Wenn es sich um EIN Animationsobjekt oder ein Gruppenobjekt
        // handelt, das 'einzeln uebernommen' wurde,
        // werden die einzelnen Objekte eingefuegt
        sal_Bool bAnimObj = sal_False;
        if( nMarkCount == 1 )
        {
            SdrMark*            pMark = rMarkList.GetMark(0);
            SdrObject*          pObject = pMark->GetMarkedSdrObj();
            SdAnimationInfo*    pAnimInfo = rView.GetDoc().GetAnimationInfo( pObject );
            sal_uInt32              nInv = pObject->GetObjInventor();
            sal_uInt16              nId = pObject->GetObjIdentifier();

            // Animated Bitmap (GIF)
            if( nInv == SdrInventor && nId == OBJ_GRAF && ( (SdrGrafObj*) pObject )->IsAnimated() )
            {
                const SdrGrafObj*   pGrafObj = (SdrGrafObj*) pObject;
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

                        pBitmapEx = new BitmapEx( rAnimBmp.aBmpEx );

                        // LoopCount
                        if( i == 0 )
                        {
                            long nLoopCount = aAnimation.GetLoopCount();

                            if( !nLoopCount ) // unendlich
                                aLbLoopCount.SelectEntryPos( aLbLoopCount.GetEntryCount() - 1);
                            else
                                aLbLoopCount.SelectEntry(rtl::OUString::valueOf( nLoopCount ) );
                        }

                        long nTime = rAnimBmp.nWait;
                        Time* pTime = new Time( 0, 0, nTime / 100, nTime % 100 );
                        m_FrameList.insert(
                                m_FrameList.begin() + m_nCurrentFrame + 1,
                                ::std::make_pair(pBitmapEx, pTime));

                        // increment => next one inserted after this one
                        ++m_nCurrentFrame;
                    }
                    // Nachdem ein animated GIF uebernommen wurde, kann auch nur ein solches erstellt werden
                    aRbtBitmap.Check();
                    aRbtGroup.Enable( sal_False );
                    bAnimObj = sal_True;
                }
            }
            else if( bAllObjects || ( pAnimInfo && pAnimInfo->mbIsMovie ) )
            {
                // Mehrere Objekte
                SdrObjList* pObjList = ((SdrObjGroup*)pObject)->GetSubList();

                for( sal_uInt16 nObject = 0; nObject < pObjList->GetObjCount(); nObject++ )
                {
                    SdrObject* pSnapShot = (SdrObject*) pObjList->GetObj( (sal_uLong) nObject );

                    pBitmapEx = new BitmapEx( SdrExchangeView::GetObjGraphic( pSnapShot->GetModel(), pSnapShot ).GetBitmapEx() );

                    Time* pTime = new Time( aTimeField.GetTime() );

                    m_FrameList.insert(
                            m_FrameList.begin() + m_nCurrentFrame + 1,
                            ::std::make_pair(pBitmapEx, pTime));

                    // increment => next one inserted after this one
                    ++m_nCurrentFrame;

                    // Clone
                    pPage->InsertObject(pSnapShot->Clone(), m_nCurrentFrame);
                }
                bAnimObj = sal_True;
            }
        }
        // Auch ein einzelnes animiertes Objekt
        if( !bAnimObj && !( bAllObjects && nMarkCount > 1 ) )
        {
            pBitmapEx = new BitmapEx( rView.GetAllMarkedGraphic().GetBitmapEx() );

            Time* pTime = new Time( aTimeField.GetTime() );

            m_FrameList.insert(
                    m_FrameList.begin() + m_nCurrentFrame + 1,
                    ::std::make_pair(pBitmapEx, pTime));
        }

        // ein einzelnes Objekt
        if( nMarkCount == 1 && !bAnimObj )
        {
            SdrMark*    pMark   = rMarkList.GetMark(0);
            SdrObject*  pObject = pMark->GetMarkedSdrObj();
            SdrObject*  pClone  = pObject->Clone();
            pPage->InsertObject(pClone, m_nCurrentFrame + 1);
        }
        // mehrere Objekte: die Clones zu einer Gruppe zusammenfassen
        else if (nMarkCount > 1)
        {
            // Objekte einzeln uebernehmen
            if( bAllObjects )
            {
                for( sal_uLong nObject= 0; nObject < nMarkCount; nObject++ )
                {
                    // Clone
                    SdrObject* pObject = rMarkList.GetMark( nObject )->GetMarkedSdrObj();

                    pBitmapEx = new BitmapEx( SdrExchangeView::GetObjGraphic( pObject->GetModel(), pObject ).GetBitmapEx() );

                    Time* pTime = new Time( aTimeField.GetTime() );

                    m_FrameList.insert(
                        m_FrameList.begin() + m_nCurrentFrame + 1,
                        ::std::make_pair(pBitmapEx, pTime));

                    // increment => next one inserted after this one
                    ++m_nCurrentFrame;

                    pPage->InsertObject(pObject->Clone(), m_nCurrentFrame);
                }
                bAnimObj = sal_True; // damit nicht nochmal weitergeschaltet wird
            }
            else
            {
                SdrObjGroup* pCloneGroup = new SdrObjGroup;
                SdrObjList*  pObjList    = pCloneGroup->GetSubList();

                for (sal_uLong nObject= 0; nObject < nMarkCount; nObject++)
                    pObjList->InsertObject(rMarkList.GetMark(nObject)->GetMarkedSdrObj()->Clone(), LIST_APPEND);

                pPage->InsertObject(pCloneGroup, m_nCurrentFrame + 1);
            }
        }

        if( !bAnimObj )
        {
            ++m_nCurrentFrame;
        }

        // wenn vorher nichts im Animator war und jetzt was da ist, kann eine
        // Animationsgruppe erstellt werden
        if (nCloneCount == 0 && !m_FrameList.empty())
        {
            aBtnCreateGroup.Enable();
        }

        // Zoom fuer DisplayWin berechnen und setzen
        Fraction aFrac( GetScale() );
        aCtlDisplay.SetScale( aFrac );

        UpdateControl(m_nCurrentFrame);
    }
}

// -----------------------------------------------------------------------

void AnimationWindow::CreateAnimObj (::sd::View& rView )
{
    ::Window* pOutWin = static_cast< ::Window*>(rView.GetFirstOutputDevice()); // GetWin( 0 );
    DBG_ASSERT( pOutWin, "Window ist nicht vorhanden!" );

    // die Fentermitte ermitteln
    const MapMode       aMap100( MAP_100TH_MM );
    Size                aMaxSizeLog;
    Size                aMaxSizePix;
    Size                aTemp( pOutWin->GetOutputSizePixel() );
    const Point         aWindowCenter( pOutWin->PixelToLogic( Point( aTemp.Width() >> 1, aTemp.Height() >> 1 ) ) );
    const OutputDevice* pDefDev = Application::GetDefaultDevice();
    const size_t nCount = m_FrameList.size();
    BitmapAdjustment    eBA = (BitmapAdjustment) aLbAdjustment.GetSelectEntryPos();

    // Groesste Bitmap ermitteln
    for (size_t i = 0; i < nCount; ++i)
    {
        const BitmapEx& rBmpEx = *m_FrameList[i].first;
        const Graphic   aGraphic( rBmpEx );
        Size            aTmpSizeLog;
        const Size      aTmpSizePix( rBmpEx.GetSizePixel() );

        if ( aGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
            aTmpSizeLog = pDefDev->PixelToLogic( aGraphic.GetPrefSize(), aMap100 );
        else
            aTmpSizeLog = pDefDev->LogicToLogic( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), aMap100 );

        aMaxSizeLog.Width() = Max( aMaxSizeLog.Width(), aTmpSizeLog.Width() );
        aMaxSizeLog.Height() = Max( aMaxSizeLog.Height(), aTmpSizeLog.Height() );

        aMaxSizePix.Width() = Max( aMaxSizePix.Width(), aTmpSizePix.Width() );
        aMaxSizePix.Height() = Max( aMaxSizePix.Height(), aTmpSizePix.Height() );
    }

    SdrPageView* pPV = rView.GetSdrPageView();

    if( aRbtBitmap.IsChecked() )
    {
        // Bitmapgruppe erzeugen (Animated GIF)
        Animation   aAnimation;
        Point       aPt;

        for (size_t i = 0; i < nCount; ++i)
        {
            Time *const pTime = m_FrameList[i].second;
            long  nTime = pTime->Get100Sec();
            nTime += pTime->GetSec() * 100;

            pBitmapEx = m_FrameList[i].first;

            // Offset fuer die gewuenschte Ausrichtung bestimmen
            const Size aBitmapSize( pBitmapEx->GetSizePixel() );

            switch( eBA )
            {
                case BA_LEFT_UP:
                break;

                case BA_LEFT:
                    aPt.Y() = (aMaxSizePix.Height() - aBitmapSize.Height()) >> 1;
                break;

                case BA_LEFT_DOWN:
                    aPt.Y() = aMaxSizePix.Height() - aBitmapSize.Height();
                break;

                case BA_UP:
                    aPt.X() = (aMaxSizePix.Width() - aBitmapSize.Width()) >> 1;
                break;

                case BA_CENTER:
                    aPt.X()  = (aMaxSizePix.Width() - aBitmapSize.Width()) >> 1;
                    aPt.Y() = (aMaxSizePix.Height() - aBitmapSize.Height()) >> 1;
                break;

                case BA_DOWN:
                    aPt.X()  = (aMaxSizePix.Width() - aBitmapSize.Width()) >> 1;
                    aPt.Y() = aMaxSizePix.Height() - aBitmapSize.Height();
                break;

                case BA_RIGHT_UP:
                    aPt.X() = aMaxSizePix.Width() - aBitmapSize.Width();
                break;

                case BA_RIGHT:
                    aPt.X()  = aMaxSizePix.Width() - aBitmapSize.Width();
                    aPt.Y() = (aMaxSizePix.Height() - aBitmapSize.Height()) >> 1;
                break;

                case BA_RIGHT_DOWN:
                    aPt.X()  = aMaxSizePix.Width() - aBitmapSize.Width();
                    aPt.Y() = aMaxSizePix.Height() - aBitmapSize.Height();
                break;

            }

            // LoopCount (Anzahl der Durchlaeufe) ermitteln
            AnimationBitmap aAnimBmp;
            long            nLoopCount = 0L;
            sal_uInt16          nPos = aLbLoopCount.GetSelectEntryPos();

            if( nPos != LISTBOX_ENTRY_NOTFOUND && nPos != aLbLoopCount.GetEntryCount() - 1 ) // unendlich
                nLoopCount = (long) aLbLoopCount.GetSelectEntry().ToInt32();

            aAnimBmp.aBmpEx = *pBitmapEx;
            aAnimBmp.aPosPix = aPt;
            aAnimBmp.aSizePix = aBitmapSize;
            aAnimBmp.nWait = nTime;
            aAnimBmp.eDisposal = DISPOSE_BACK;
            aAnimBmp.bUserInput = sal_False;

            aAnimation.Insert( aAnimBmp );
            aAnimation.SetDisplaySizePixel( aMaxSizePix );
            aAnimation.SetLoopCount( nLoopCount );
        }

        SdrGrafObj* pGrafObj = new SdrGrafObj( Graphic( aAnimation ) );
        const Point aOrg( aWindowCenter.X() - ( aMaxSizeLog.Width() >> 1 ), aWindowCenter.Y() - ( aMaxSizeLog.Height() >> 1 ) );

        pGrafObj->SetLogicRect( Rectangle( aOrg, aMaxSizeLog ) );
        rView.InsertObjectAtView( pGrafObj, *pPV, SDRINSERT_SETDEFLAYER);
    }
    else
    {
        // Offset fuer die gewuenschte Ausrichtung bestimmen
        Size aOffset;
        SdrObject * pClone = NULL;
        SdPage* pPage = pMyDoc->GetSdPage(0, PK_STANDARD);

        for (size_t i = 0; i < nCount; ++i)
        {
            pClone = pPage->GetObj(i);
            Rectangle aRect( pClone->GetSnapRect() );

            switch( eBA )
            {
                case BA_LEFT_UP:
                break;

                case BA_LEFT:
                    aOffset.Height() = (aMaxSizeLog.Height() - aRect.GetHeight()) / 2;
                break;

                case BA_LEFT_DOWN:
                    aOffset.Height() = aMaxSizeLog.Height() - aRect.GetHeight();
                break;

                case BA_UP:
                    aOffset.Width() = (aMaxSizeLog.Width() - aRect.GetWidth()) / 2;
                break;

                case BA_CENTER:
                    aOffset.Width()  = (aMaxSizeLog.Width() - aRect.GetWidth()) / 2;
                    aOffset.Height() = (aMaxSizeLog.Height() - aRect.GetHeight()) / 2;
                break;

                case BA_DOWN:
                    aOffset.Width()  = (aMaxSizeLog.Width() - aRect.GetWidth()) / 2;
                    aOffset.Height() = aMaxSizeLog.Height() - aRect.GetHeight();
                break;

                case BA_RIGHT_UP:
                    aOffset.Width() = aMaxSizeLog.Width() - aRect.GetWidth();
                break;

                case BA_RIGHT:
                    aOffset.Width()  = aMaxSizeLog.Width() - aRect.GetWidth();
                    aOffset.Height() = (aMaxSizeLog.Height() - aRect.GetHeight()) / 2;
                break;

                case BA_RIGHT_DOWN:
                    aOffset.Width()  = aMaxSizeLog.Width() - aRect.GetWidth();
                    aOffset.Height() = aMaxSizeLog.Height() - aRect.GetHeight();
                break;

            }
            // SetSnapRect ist fuer Ellipsen leider nicht implementiert !!!
            Point aMovePt( aWindowCenter + Point( aOffset.Width(), aOffset.Height() ) - aRect.TopLeft() );
            Size aMoveSize( aMovePt.X(), aMovePt.Y() );
            pClone->NbcMove( aMoveSize );
        }

        // Animationsgruppe erzeugen
        SdrObjGroup* pGroup   = new SdrObjGroup;
        SdrObjList*  pObjList = pGroup->GetSubList();

        for (size_t i = 0; i < nCount; i++)
        {
            // der Clone verbleibt im Animator; in die Gruppe kommt ein Clone
            // des Clones
            pClone = pPage->GetObj(i);
            SdrObject* pCloneOfClone = pClone->Clone();
            //SdrObject* pCloneOfClone = pPage->GetObj(i)->Clone();
            pObjList->InsertObject(pCloneOfClone, LIST_APPEND);
        }

        // bis jetzt liegt die linke obere Ecke der Gruppe in der Fenstermitte;
        // jetzt noch um die Haelfte der Groesse nach oben und links korrigieren
        aTemp = aMaxSizeLog;
        aTemp.Height() = - aTemp.Height() / 2;
        aTemp.Width()  = - aTemp.Width() / 2;
        pGroup->NbcMove(aTemp);

        // Animationsinformation erzeugen
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pGroup,true);
        pInfo->meEffect = presentation::AnimationEffect_NONE;
        pInfo->meSpeed = presentation::AnimationSpeed_MEDIUM;
        pInfo->mbActive = sal_True;
        pInfo->mbIsMovie = sal_True;
        pInfo->maBlueScreen = COL_WHITE;

        rView.InsertObjectAtView( pGroup, *pPV, SDRINSERT_SETDEFLAYER);
    }

    ClickFirstHdl( this );
}

void AnimationWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxDockingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        UpdateControl(m_nCurrentFrame);
    }
}

/*************************************************************************
|*
|* ControllerItem fuer Animator
|*
\************************************************************************/

AnimationControllerItem::AnimationControllerItem(
    sal_uInt16 _nId,
    AnimationWindow* pAnimWin,
    SfxBindings*    _pBindings)
    : SfxControllerItem( _nId, *_pBindings ),
      pAnimationWin( pAnimWin )
{
}

// -----------------------------------------------------------------------

void AnimationControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_ANIMATOR_STATE )
    {
        const SfxUInt16Item* pStateItem = PTR_CAST( SfxUInt16Item, pItem );
        DBG_ASSERT( pStateItem, "SfxUInt16Item erwartet");
        sal_uInt16 nState = pStateItem->GetValue();

        pAnimationWin->aBtnGetOneObject.Enable( nState & 1 );
        pAnimationWin->aBtnGetAllObjects.Enable( nState & 2 );
    }
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
