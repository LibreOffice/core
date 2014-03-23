/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>

#define _ANIMATION          //animation freischalten

#define _SV_BITMAPEX
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
#include "anmdef.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "View.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "res_bmp.hrc"
#include "ViewShell.hxx"
#include <vcl/svapp.hxx>
// #42894#
#include <EffectMigration.hxx>

#include <string>
#include <algorithm>
#include <svx/svdlegacy.hxx>
#include <svx/svdetc.hxx>

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
    //DrawBitmap( aPt, aBmpSize, *pBitmap );
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
            ? SD_OUTPUT_DRAWMODE_CONTRAST
            : SD_OUTPUT_DRAWMODE_COLOR );
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
        pBitmapEx           ( NULL ),

        bMovie              ( false ),
        bAllObjects         ( false ),

        pBindings           ( pInBindings )
{
    aCtlDisplay.SetAccessibleName(String (SdResId(STR_DISPLAY)));
    FreeResource();

    aBtnGetOneObject.SetModeImage( Image( SdResId( IMG_GET1OBJECT_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnGetAllObjects.SetModeImage( Image( SdResId( IMG_GETALLOBJECT_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnRemoveBitmap.SetModeImage( Image( SdResId( IMG_REMOVEBMP_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnRemoveAll.SetModeImage( Image( SdResId( IMG_REMOVEALLBMP_H ) ), BMP_COLOR_HIGHCONTRAST );

    // neues Dokument mit Seite erzeugen
    pMyDoc = new SdDrawDocument(DOCUMENT_TYPE_IMPRESS, NULL);
    SdPage* pPage = (SdPage*) pMyDoc->AllocPage(false);
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
    sal_uLong i, nCount;

    delete pControllerItem;

    // Bitmapliste bereinigen
    for( i = 0, nCount = aBmpExList.Count(); i < nCount; i++ )
        delete static_cast< BitmapEx* >( aBmpExList.GetObject( i ) );
    aBmpExList.Clear();

    // Timeliste bereinigen
    for( i = 0, nCount = aTimeList.Count(); i < nCount; i++ )
        delete static_cast< Time* >( aTimeList.GetObject( i ) );
    aTimeList.Clear();

    // die Clones loeschen
    delete pMyDoc;
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickFirstHdl, void *, EMPTYARG )
{
    aBmpExList.First();
    pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetCurObject() );
    UpdateControl( aBmpExList.GetCurPos() );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickStopHdl, void *, EMPTYARG )
{
    bMovie = false;
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickPlayHdl, void *, p )
{
    ScopeLockGuard aGuard( maPlayLock );

    bMovie = true;
    bool bDisableCtrls = false;
    sal_uLong nCount = aBmpExList.Count();
    bool bReverse = p == &aBtnReverse;

    // Kann spaeter schwer ermittelt werden
    bool bRbtGroupEnabled = aRbtGroup.IsEnabled();
    bool bBtnGetAllObjectsEnabled = aBtnGetAllObjects.IsEnabled();
    bool bBtnGetOneObjectEnabled = aBtnGetOneObject.IsEnabled();

    // Gesamtzeit ermitteln
    Time aTime( 0 );
    long nFullTime;
    if( aRbtBitmap.IsChecked() )
    {
        for( sal_uLong i = 0; i < nCount; i++ )
            aTime += *static_cast< Time* >( aTimeList.GetObject( i ) );
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
        bDisableCtrls = true;
        aBtnStop.Enable();
        aBtnStop.Update();
        String aStr( RTL_CONSTASCII_USTRINGPARAM( "Animator:" ) ); // Hier sollte man sich noch etwas gescheites ausdenken!
        pProgress = new SfxProgress( NULL, aStr, nFullTime );
    }

    sal_uLong nTmpTime = 0;
    long i = 0;
    bool bCount = i < (long) nCount;
    if( bReverse )
    {
        i = nCount - 1;
        bCount = i >= 0;
    }
    while( bCount && bMovie )
    {
        // Um Konsistenz zwischen Liste und Anzeige zu erwirken
        aBmpExList.Seek( i );
        pBitmapEx = static_cast< BitmapEx* >(  aBmpExList.GetCurObject() );

        UpdateControl( i, bDisableCtrls );

        if( aRbtBitmap.IsChecked() )
        {
            Time* pTime = static_cast< Time* >( aTimeList.GetObject( i ) );
            DBG_ASSERT( pTime, "Keine Zeit gefunden!" );

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
            i--;
            if (i < 0)
            {
                // Terminate loop.
                bCount = false;
                // Move i back into valid range.
                i = 0;
            }
        }
        else
        {
            i++;
            if (i >= (long) nCount)
            {
                // Terminate loop.
                bCount = false;
                // Move i back into valid range.
                i = nCount - 1;
            }
        }
    }

    // Um die Controls wieder zu enablen
    bMovie = false;
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

IMPL_LINK( AnimationWindow, ClickLastHdl, void *, EMPTYARG )
{
    aBmpExList.Last();
    pBitmapEx = static_cast< BitmapEx* >(  aBmpExList.GetCurObject() );
    UpdateControl( aBmpExList.GetCurPos() );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickRbtHdl, void *, p )
{
    if( !pBitmapEx || p == &aRbtGroup || aRbtGroup.IsChecked() )
    {
        aTimeField.SetText( String() );
        aTimeField.Enable( false );
        aLbLoopCount.Enable( false );
    }
    else if( p == &aRbtBitmap || aRbtBitmap.IsChecked() )
    {
        sal_uLong n = static_cast<sal_uLong>(aNumFldBitmap.GetValue());
        if( n > 0 )
        {
            Time* pTime = static_cast< Time* >( aTimeList.GetObject( n - 1 ) );
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
    SfxBoolItem aItem( SID_ANIMATOR_ADD, true );

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
        sal_uLong nPos = aBmpExList.GetCurPos();
        pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetCurObject() );
        if( pBitmapEx )
        {
            delete pBitmapEx;
            aBmpExList.Remove();
            pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetCurObject() );
        }
        Time* pTime = static_cast< Time* >( aTimeList.GetObject( nPos ) );
        if( pTime )
        {
            delete pTime;
            aTimeList.Remove( nPos );
        }

        pObject = pPage->GetObj( nPos );
        // Durch Uebernahme der AnimatedGIFs muessen nicht unbedingt
        // Objekte vorhanden sein.
        if( pObject )
        {
            pObject = pPage->RemoveObjectFromSdrObjList(nPos);
            DBG_ASSERT(pObject, "Clone beim Loeschen nicht gefunden");
            deleteSdrObjectSafeAndClearPointer( pObject );
        }

    }
    else // Alles loeschen
    {
        WarningBox aWarnBox( this, WB_YES_NO, String( SdResId( STR_ASK_DELETE_ALL_PICTURES ) ) );
        short nReturn = aWarnBox.Execute();

        if( nReturn == RET_YES )
        {
            // Bitmapliste bereinigen
            long nCount = aBmpExList.Count();
            long i;

            for( i = nCount - 1; i >= 0; i-- )
            {
                pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetObject( i ) );
                delete pBitmapEx;

                pObject = pPage->GetObj( i );
                if( pObject )
                {
                    pObject = pPage->RemoveObjectFromSdrObjList( i );
                    DBG_ASSERT(pObject, "Clone beim Loeschen nicht gefunden");
                    deleteSdrObjectSafeAndClearPointer( pObject );
                }

            }
            aBmpExList.Clear();

            // Timeliste bereinigen
            nCount = aTimeList.Count();
            for( i = 0; i < nCount; i++ )
            {
                delete static_cast< Time* >( aTimeList.GetObject( i ) );
            }
            aTimeList.Clear();
        }
    }

    // kann noch eine Animationsgruppe erstellt werden?
    if (aBmpExList.Count() == 0)
    {
        aBtnCreateGroup.Disable();
        // Falls vorher durch Uebernahme von AnimatedGIFs disabled:
        //aRbtBitmap.Enable();
        aRbtGroup.Enable();
    }

    // Zoom fuer DisplayWin berechnen und setzen
    Fraction aFrac( GetScale() );
    aCtlDisplay.SetScale( aFrac );

    UpdateControl( aBmpExList.GetCurPos() );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ClickCreateGroupHdl, void *, EMPTYARG )
{
    // Code jetzt in CreatePresObj()
    SfxBoolItem aItem( SID_ANIMATOR_CREATE, true );

    GetBindings().GetDispatcher()->Execute(
        SID_ANIMATOR_CREATE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ModifyBitmapHdl, void *, EMPTYARG )
{
    sal_uLong nBmp = static_cast<sal_uLong>(aNumFldBitmap.GetValue());

    if( nBmp > aBmpExList.Count() )
        nBmp = aBmpExList.Count();

    pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetObject( nBmp - 1 ) );

    // Positionieren in der Liste
    aBmpExList.Seek( nBmp - 1 );

    UpdateControl( nBmp - 1 );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( AnimationWindow, ModifyTimeHdl, void *, EMPTYARG )
{
    sal_uLong nPos = static_cast<sal_uLong>(aNumFldBitmap.GetValue() - 1);

    Time* pTime = static_cast< Time* >( aTimeList.GetObject( nPos ) );
    DBG_ASSERT( pTime, "Zeit nicht gefunden!" );

    *pTime = aTimeField.GetTime();

    return( 0L );
}

// -----------------------------------------------------------------------

void AnimationWindow::UpdateControl( sal_uLong nListPos, bool bDisableCtrls )
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
            Rectangle       aObjRect( sdr::legacy::GetBoundRect(*pObject) );
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
                ? SD_OUTPUT_DRAWMODE_CONTRAST
                : SD_OUTPUT_DRAWMODE_COLOR );
            aVD.Erase();
            pObject->SingleObjectPainter( aVD ); // #110094#-17
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

    aFiCount.SetText( UniString::CreateFromInt32( aBmpExList.Count() ) );

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
        //aFiCount.SetText( String( SdResId( STR_NULL ) ) );

        // Wenn kein Objekt in der Liste ist
        aBtnFirst.Enable( false );
        aBtnReverse.Enable( false );
        aBtnPlay.Enable( false );
        aBtnLast.Enable( false );
        aNumFldBitmap.Enable( false );
        aTimeField.Enable( false );
        aLbLoopCount.Enable( false );
        aBtnRemoveBitmap.Enable( false );
        aBtnRemoveAll.Enable( false );

        //aFtAdjustment.Enable();
        //aLbAdjustment.Enable();
    }

    if( bMovie && bDisableCtrls )
    {
        aBtnGetOneObject.Enable( false );
        aBtnGetAllObjects.Enable( false );
        aRbtGroup.Enable( false );
        aRbtBitmap.Enable( false );
        aBtnCreateGroup.Enable( false );
        aFtAdjustment.Enable( false );
        aLbAdjustment.Enable( false );
    }
    else
    {
        // 'Gruppenobjekt' nur dann enablen, wenn es kein Animated GIF ist
        if (aBmpExList.Count() == 0)
            aRbtGroup.Enable();

        aRbtBitmap.Enable();
        aBtnCreateGroup.Enable(aBmpExList.Count() != 0);
        aFtAdjustment.Enable( true );
        aLbAdjustment.Enable( true );
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
    sal_uLong nPos = aBmpExList.GetCurPos();
    sal_uLong nCount = aBmpExList.Count();
    if( nCount > 0 )
    {
        aBmpSize.Width() = 0;
        aBmpSize.Height() = 0;
        for( sal_uLong i = 0; i < nCount; i++ )
        {
            pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetObject( i ) );
            Size aTempSize( pBitmapEx->GetBitmap().GetSizePixel() );
            aBmpSize.Width() = Max( aBmpSize.Width(), aTempSize.Width() );
            aBmpSize.Height() = Max( aBmpSize.Height(), aTempSize.Height() );
        }

        aBmpSize.Width() += 10;
        aBmpSize.Height() += 10;

        aFrac = Fraction( std::min( (double)aDisplaySize.Width() / (double)aBmpSize.Width(),
                             (double)aDisplaySize.Height() / (double)aBmpSize.Height() ) );
    }
    // Liste wieder auf alten Stand bringen
    pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetObject( nPos ) );
    return( aFrac );
}

// -----------------------------------------------------------------------

void AnimationWindow::Resize()
{
    //if( !IsZoomedIn() )
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
        // aPt.X() = aDiffSize.Width() / 2;
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

        //aFltWinSize = GetSizePixel();
    }
    SfxDockingWindow::Resize();
}

// -----------------------------------------------------------------------

sal_Bool AnimationWindow::Close()
{
    if( maPlayLock.isLocked() )
    {
        return false;
    }
    else
    {
        SfxBoolItem aItem( SID_ANIMATION_OBJECTS, false );

        GetBindings().GetDispatcher()->Execute(
            SID_ANIMATION_OBJECTS, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

        SfxDockingWindow::Close();

        return true;
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
    const SdrObjectVector aSelection(rView.getSelectedSdrObjectVectorFromSdrMarkView());
    SdPage*            pPage       = pMyDoc->GetSdPage(0, PK_STANDARD);

    if (aSelection.size())
    {
        const sal_uInt32 nCloneCount(pPage->GetObjCount());
        // Wenn es sich um EIN Animationsobjekt oder ein Gruppenobjekt
        // handelt, das 'einzeln uebernommen' wurde,
        // werden die einzelnen Objekte eingefuegt
        bool bAnimObj = false;
        if( 1 == aSelection.size() )
        {
            SdrObject* pObject = aSelection[0];
            SdAnimationInfo*    pAnimInfo = rView.GetDoc()->GetAnimationInfo( pObject );
            const sal_uInt32 nInv = pObject->GetObjInventor();
            const sal_uInt16 nId = pObject->GetObjIdentifier();

            // Animated Bitmap (GIF)
            const SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >(pObject);

            if( pGrafObj && pGrafObj->IsAnimated() )
            {
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
                        aBmpExList.Insert( pBitmapEx, aBmpExList.GetCurPos() + 1 );

                        // LoopCount
                        if( i == 0 )
                        {
                            long nLoopCount = aAnimation.GetLoopCount();

                            if( !nLoopCount ) // unendlich
                                aLbLoopCount.SelectEntryPos( aLbLoopCount.GetEntryCount() - 1);
                            else
                                aLbLoopCount.SelectEntry( UniString::CreateFromInt32( nLoopCount ) );
                        }

                        // Time
                        long nTime = rAnimBmp.nWait;
                        Time* pTime = new Time( 0, 0, nTime / 100, nTime % 100 );
                        aTimeList.Insert( pTime, aBmpExList.GetCurPos() + 1 );

                        // Weiterschalten der BitmapListe
                        aBmpExList.Next();
                    }
                    // Nachdem ein animated GIF uebernommen wurde, kann auch nur ein solches erstellt werden
                    aRbtBitmap.Check();
                    aRbtGroup.Enable( false );
                    bAnimObj = true;
                }
            }
            else if( bAllObjects || ( pAnimInfo && pAnimInfo->mbIsMovie ) )
            {
                // Mehrere Objekte
                SdrObjList* pObjList = pObject->getChildrenOfSdrObject();

                if(pObjList)
                {
                    for( sal_uInt32 nObject (0); nObject < pObjList->GetObjCount(); nObject++ )
                    {
                        SdrObject* pSnapShot = (SdrObject*) pObjList->GetObj( nObject );
                        OSL_ENSURE(pSnapShot, "zero SdrObject* in SdrObjList (!)");

                        pBitmapEx = new BitmapEx(GetObjGraphic(*pSnapShot).GetBitmapEx());
                        aBmpExList.Insert( pBitmapEx, aBmpExList.GetCurPos() + 1 );

                        // Time
                        Time* pTime = new Time( aTimeField.GetTime() );
                        aTimeList.Insert( pTime, aBmpExList.GetCurPos() + 1 );

                        // Clone
                        pPage->InsertObjectToSdrObjList(*pSnapShot->CloneSdrObject(), aBmpExList.GetCurPos() + 1);

                        // Weiterschalten der BitmapListe
                        aBmpExList.Next();
                    }
                }
                bAnimObj = true;
            }
        }
        // Auch ein einzelnes animiertes Objekt
        if( !bAnimObj && !( bAllObjects && aSelection.size() > 1 ) )
        {
            pBitmapEx = new BitmapEx( rView.GetAllMarkedGraphic().GetBitmapEx() );
            aBmpExList.Insert( pBitmapEx, aBmpExList.GetCurPos() + 1 );

            // Time
            Time* pTime = new Time( aTimeField.GetTime() );
            aTimeList.Insert( pTime, aBmpExList.GetCurPos() + 1 );

        }

        // ein einzelnes Objekt
        if( 1 == aSelection.size() && !bAnimObj )
        {
            pPage->InsertObjectToSdrObjList(*aSelection[0]->CloneSdrObject(), aBmpExList.GetCurPos() + 1);
        }
        else if (aSelection.size() > 1)
        {
            // mehrere Objekte: die Clones zu einer Gruppe zusammenfassen
            // Objekte einzeln uebernehmen
            if( bAllObjects )
            {
                for( sal_uInt32 nObject(0); nObject < aSelection.size(); nObject++ )
                {
                    // Clone
                    SdrObject* pObject = aSelection[nObject];

                    pBitmapEx = new BitmapEx(GetObjGraphic(*pObject).GetBitmapEx());
                    aBmpExList.Insert( pBitmapEx, aBmpExList.GetCurPos() + 1 );

                    // Time
                    Time* pTime = new Time( aTimeField.GetTime() );
                    aTimeList.Insert( pTime, aBmpExList.GetCurPos() + 1 );

                    pPage->InsertObjectToSdrObjList(*pObject->CloneSdrObject(), aBmpExList.GetCurPos() + 1 );

                    aBmpExList.Next();
                }
                bAnimObj = true; // damit nicht nochmal weitergeschaltet wird
            }
            else
            {
                SdrObjGroup* pCloneGroup = new SdrObjGroup(pPage->getSdrModelFromSdrPage());

                for (sal_uInt32 nObject(0); nObject < aSelection.size(); nObject++)
                {
                    pCloneGroup->InsertObjectToSdrObjList(*aSelection[nObject]->CloneSdrObject());
                }

                pPage->InsertObjectToSdrObjList(*pCloneGroup, aBmpExList.GetCurPos() + 1);
            }
        }

        if( !bAnimObj )
            aBmpExList.Next();

        // wenn vorher nichts im Animator war und jetzt was da ist, kann eine
        // Animationsgruppe erstellt werden
        if (nCloneCount == 0 && aBmpExList.Count() > 0)
            aBtnCreateGroup.Enable();

        // Zoom fuer DisplayWin berechnen und setzen
        Fraction aFrac( GetScale() );
        aCtlDisplay.SetScale( aFrac );

        UpdateControl( aBmpExList.GetCurPos() );
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
    const sal_uLong         nCount = aBmpExList.Count();
    BitmapAdjustment    eBA = (BitmapAdjustment) aLbAdjustment.GetSelectEntryPos();
    sal_uLong               i;

    // Groesste Bitmap ermitteln
    for( i = 0; i < nCount; i++ )
    {
        const BitmapEx& rBmpEx = *static_cast< BitmapEx* >( aBmpExList.GetObject( i ) );
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

    if( aRbtBitmap.IsChecked() )
    {
        // Bitmapgruppe erzeugen (Animated GIF)
        Animation   aAnimation;
        Point       aPt;

        for( i = 0; i < nCount; i++ )
        {
            Time* pTime = static_cast< Time* >( aTimeList.GetObject( i ) );
            long  nTime = pTime->Get100Sec();
            nTime += pTime->GetSec() * 100;

            pBitmapEx = static_cast< BitmapEx* >( aBmpExList.GetObject( i ) );

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
            aAnimBmp.bUserInput = false;

            aAnimation.Insert( aAnimBmp );
            aAnimation.SetDisplaySizePixel( aMaxSizePix );
            aAnimation.SetLoopCount( nLoopCount );
        }

        SdrGrafObj* pGrafObj = new SdrGrafObj(
            rView.getSdrModelFromSdrView(),
            Graphic( aAnimation ) );
        const Point aOrg( aWindowCenter.X() - ( aMaxSizeLog.Width() >> 1 ), aWindowCenter.Y() - ( aMaxSizeLog.Height() >> 1 ) );

        sdr::legacy::SetLogicRect(*pGrafObj, Rectangle( aOrg, aMaxSizeLog ) );
        rView.InsertObjectAtView( *pGrafObj, SDRINSERT_SETDEFLAYER);
    }
    else
    {
        // Offset fuer die gewuenschte Ausrichtung bestimmen
        Size aOffset;
        SdrObject * pClone = NULL;
        SdPage* pPage = pMyDoc->GetSdPage(0, PK_STANDARD);

        for(i = 0; i < nCount; i++)
        {
            pClone = pPage->GetObj(i);
            Rectangle aRect( sdr::legacy::GetSnapRect(*pClone) );

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
            //aRect.SetPos(aWindowCenter + Point(aOffset.Width(), aOffset.Height()));
            //pClone->SetSnapRect( aRect );
            // SetSnapRect ist fuer Ellipsen leider nicht implementiert !!!
            Point aMovePt( aWindowCenter + Point( aOffset.Width(), aOffset.Height() ) - aRect.TopLeft() );
            Size aMoveSize( aMovePt.X(), aMovePt.Y() );
            sdr::legacy::MoveSdrObject(*pClone, aMoveSize );
        }

        // #42894# Caution(!) variable pPage looks right, but it is a page from the local
        // document the dialog is using (!), so get the target page from the target view
        SdPage* pTargetSdPage = dynamic_cast< SdPage* >(rView.GetSdrPageView() ? &rView.GetSdrPageView()->getSdrPageFromSdrPageView() : 0);

        if(pTargetSdPage)
        {
            // Animationsgruppe erzeugen
            SdrObjGroup* pGroup   = new SdrObjGroup(rView.getSdrModelFromSdrView());

            for (i = 0; i < nCount; i++)
            {
                // der Clone verbleibt im Animator; in die Gruppe kommt ein Clone
                // des Clones
                pClone = pPage->GetObj(i);
                SdrObject* pCloneOfClone = pClone->CloneSdrObject();
                pGroup->InsertObjectToSdrObjList(*pCloneOfClone);
            }

            // bis jetzt liegt die linke obere Ecke der Gruppe in der Fenstermitte;
            // jetzt noch um die Haelfte der Groesse nach oben und links korrigieren
            aTemp = aMaxSizeLog;
            aTemp.Height() = - aTemp.Height() / 2;
            aTemp.Width()  = - aTemp.Width() / 2;
            sdr::legacy::MoveSdrObject(*pGroup, aTemp);

            // #42894# create needed SMIL stuff and move child objects to page directly (see
            // comments at EffectMigration::CreateAnimatedGroup why this has to be done).
            EffectMigration::CreateAnimatedGroup(*pGroup, *pTargetSdPage);

            // #42894# if that worked, delete the group again
            if(!pGroup->getChildrenOfSdrObject()->GetObjCount())
            {
                deleteSdrObjectSafeAndClearPointer(pGroup);
            }
        }
    }

    ClickFirstHdl( this );
}

void AnimationWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxDockingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        UpdateControl( aBmpExList.GetCurPos() );
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
        const SfxUInt16Item* pStateItem = dynamic_cast< const SfxUInt16Item* >(pItem );
        DBG_ASSERT( pStateItem, "SfxUInt16Item erwartet");
        sal_uInt16 nState = pStateItem->GetValue();

        pAnimationWin->aBtnGetOneObject.Enable( nState & 1 );
        pAnimationWin->aBtnGetAllObjects.Enable( nState & 2 );
    }
}


} // end of namespace sd
