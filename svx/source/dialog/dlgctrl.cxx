/*************************************************************************
 *
 *  $RCSfile: dlgctrl.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:18:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#pragma hdrstop

#include "xoutx.hxx"
#include "xtable.hxx"
#include "xpool.hxx"

#include "dialogs.hrc"
#include "accessibility.hrc"
#include "dlgctrl.hxx"
#include "dialmgr.hxx"

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_REGION_HXX
#include <vcl/region.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_HATCH_HXX
#include <vcl/hatch.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include "svxrectctaccessiblecontext.hxx"

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#define OUTPUT_DRAWMODE_COLOR       (DRAWMODE_DEFAULT)
#define OUTPUT_DRAWMODE_CONTRAST    (DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT)

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;


/*************************************************************************
|*
|*  Control zur Darstellung und Auswahl der Eckpunkte (und Mittelpunkt)
|*  eines Objekts
|*
\************************************************************************/

inline Bitmap* SvxRectCtl::GetBitmap( void )
{
    if( !pBitmap )
        InitBitmap();

    return pBitmap;
}

SvxRectCtl::SvxRectCtl( Window* pParent, const ResId& rResId, RECT_POINT eRpt,
                        USHORT nBorder, USHORT nCircle, CTL_STYLE eStyle ) :

    Control( pParent, rResId ),

    pAccContext ( NULL ),
    eDefRP      ( eRpt ),
    nBorderWidth( nBorder ),
    nRadius     ( nCircle),
    m_nState    ( 0 ),
    eCS         ( eStyle ),
    pBitmap     ( NULL ),
    mbCompleteDisable(sal_False)
{
    SetMapMode( MAP_100TH_MM );
    Resize_Impl();
}

// -----------------------------------------------------------------------

SvxRectCtl::~SvxRectCtl()
{
    delete pBitmap;

    if( pAccContext )
        pAccContext->release();
}

// -----------------------------------------------------------------------
void SvxRectCtl::Resize()
{
    Resize_Impl();
    Control::Resize();
}

// -----------------------------------------------------------------------

void SvxRectCtl::Resize_Impl()
{
    aSize = GetOutputSize();

    switch( eCS )
    {
        case CS_RECT:
        case CS_ANGLE:
        case CS_SHADOW:
            aPtLT = Point( 0 + nBorderWidth,  0 + nBorderWidth );
            aPtMT = Point( aSize.Width() / 2, 0 + nBorderWidth );
            aPtRT = Point( aSize.Width() - nBorderWidth, 0 + nBorderWidth );

            aPtLM = Point( 0 + nBorderWidth,  aSize.Height() / 2 );
            aPtMM = Point( aSize.Width() / 2, aSize.Height() / 2 );
            aPtRM = Point( aSize.Width() - nBorderWidth, aSize.Height() / 2 );

            aPtLB = Point( 0 + nBorderWidth,    aSize.Height() - nBorderWidth );
            aPtMB = Point( aSize.Width() / 2,   aSize.Height() - nBorderWidth );
            aPtRB = Point( aSize.Width() - nBorderWidth, aSize.Height() - nBorderWidth );
        break;

        case CS_LINE:
            aPtLT = Point( 0 + 3 * nBorderWidth,  0 + nBorderWidth );
            aPtMT = Point( aSize.Width() / 2, 0 + nBorderWidth );
            aPtRT = Point( aSize.Width() - 3 * nBorderWidth, 0 + nBorderWidth );

            aPtLM = Point( 0 + 3 * nBorderWidth,  aSize.Height() / 2 );
            aPtMM = Point( aSize.Width() / 2, aSize.Height() / 2 );
            aPtRM = Point( aSize.Width() - 3 * nBorderWidth, aSize.Height() / 2 );

            aPtLB = Point( 0 + 3 * nBorderWidth,    aSize.Height() - nBorderWidth );
            aPtMB = Point( aSize.Width() / 2,   aSize.Height() - nBorderWidth );
            aPtRB = Point( aSize.Width() - 3 * nBorderWidth, aSize.Height() - nBorderWidth );
        break;
    }
    Reset();
    InitSettings( TRUE, TRUE );
}
// -----------------------------------------------------------------------

void SvxRectCtl::InitBitmap( void )
{
    if( pBitmap )
        delete pBitmap;

    const StyleSettings&    rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    pBitmap = new Bitmap( SVX_RES( RID_SVXCTRL_RECTBTNS ) );

    // set bitmap-colors
    long    aTempAry1[(7*sizeof(Color))/sizeof(long)];
    long    aTempAry2[(7*sizeof(Color))/sizeof(long)];
    Color*  pColorAry1 = (Color*)aTempAry1;
    Color*  pColorAry2 = (Color*)aTempAry2;
    pColorAry1[0] = Color( 0xC0, 0xC0, 0xC0 );  // light-gray
    pColorAry1[1] = Color( 0xFF, 0xFF, 0x00 );  // yellow
    pColorAry1[2] = Color( 0xFF, 0xFF, 0xFF );  // white
    pColorAry1[3] = Color( 0x80, 0x80, 0x80 );  // dark-gray
    pColorAry1[4] = Color( 0x00, 0x00, 0x00 );  // black
    pColorAry1[5] = Color( 0x00, 0xFF, 0x00 );  // green
    pColorAry1[6] = Color( 0x00, 0x00, 0xFF );  // blue
    pColorAry2[0] = rStyles.GetDialogColor();       // background
    pColorAry2[1] = rStyles.GetWindowColor();
    pColorAry2[2] = rStyles.GetLightColor();
    pColorAry2[3] = rStyles.GetShadowColor();
    pColorAry2[4] = rStyles.GetDarkShadowColor();
    pColorAry2[5] = Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    pColorAry2[6] = rStyles.GetDialogColor();

#ifdef DBG_UTIL
    static BOOL     bModify = FALSE;
    BOOL&           rModify = bModify;
    if( rModify )
    {
        static int      n = 0;
        static UINT8    r = 0xFF;
        static UINT8    g = 0x00;
        static UINT8    b = 0xFF;
        int&            rn = n;
        UINT8&          rr = r;
        UINT8&          rg = g;
        UINT8&          rb = b;
        pColorAry2[ rn ] = Color( rr, rg, rb );
    }
#endif

    pBitmap->Replace( pColorAry1, pColorAry2, 7, NULL );
}

// -----------------------------------------------------------------------

void SvxRectCtl::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }

    delete pBitmap;
    pBitmap = NULL;     // forces new creating of bitmap

    Invalidate();
}

/*************************************************************************
|*
|*  Das angeklickte Rechteck (3 x 3) wird ermittelt und der Parent (Dialog)
|*  wird benachrichtigt, dass der Punkt geaendert wurde
|*
\************************************************************************/

void SvxRectCtl::MouseButtonDown( const MouseEvent& rMEvt )
{
    // #103516# CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(!IsCompletelyDisabled())
    {
        Point aPtLast = aPtNew;

        aPtNew = GetApproxLogPtFromPixPt( rMEvt.GetPosPixel() );

        if( aPtNew == aPtMM && ( eCS == CS_SHADOW || eCS == CS_ANGLE ) )
        {
            aPtNew = aPtLast;
        }
        else
        {
            Invalidate( Rectangle( aPtLast - Point( nRadius, nRadius ),
                                   aPtLast + Point( nRadius, nRadius ) ) );
            Invalidate( Rectangle( aPtNew - Point( nRadius, nRadius ),
                                   aPtNew + Point( nRadius, nRadius ) ) );
            eRP = GetRPFromPoint( aPtNew );

            SetActualRP( eRP );

            if( WINDOW_TABPAGE == GetParent()->GetType() )
                ( (SvxTabPage*) GetParent() )->PointChanged( this, eRP );
        }
    }
}

// -----------------------------------------------------------------------

void SvxRectCtl::KeyInput( const KeyEvent& rKeyEvt )
{
    // #103516# CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(!IsCompletelyDisabled())
    {
        RECT_POINT eNewRP = eRP;
        BOOL bUseMM = (eCS != CS_SHADOW) && (eCS != CS_ANGLE);

        switch( rKeyEvt.GetKeyCode().GetCode() )
        {
            case KEY_DOWN:
            {
                if( !(m_nState & CS_NOVERT) )
                    switch( eNewRP )
                    {
                        case RP_LT: eNewRP = RP_LM; break;
                        case RP_MT: eNewRP = bUseMM ? RP_MM : RP_MB; break;
                        case RP_RT: eNewRP = RP_RM; break;
                        case RP_LM: eNewRP = RP_LB; break;
                        case RP_MM: eNewRP = RP_MB; break;
                        case RP_RM: eNewRP = RP_RB; break;
                    }
            }
            break;
            case KEY_UP:
            {
                if( !(m_nState & CS_NOVERT) )
                    switch( eNewRP )
                    {
                        case RP_LM: eNewRP = RP_LT; break;
                        case RP_MM: eNewRP = RP_MT; break;
                        case RP_RM: eNewRP = RP_RT; break;
                        case RP_LB: eNewRP = RP_LM; break;
                        case RP_MB: eNewRP = bUseMM ? RP_MM : RP_MT; break;
                        case RP_RB: eNewRP = RP_RM; break;
                    }
            }
            break;
            case KEY_LEFT:
            {
                if( !(m_nState & CS_NOHORZ) )
                    switch( eNewRP )
                    {
                        case RP_MT: eNewRP = RP_LT; break;
                        case RP_RT: eNewRP = RP_MT; break;
                        case RP_MM: eNewRP = RP_LM; break;
                        case RP_RM: eNewRP = bUseMM ? RP_MM : RP_LM; break;
                        case RP_MB: eNewRP = RP_LB; break;
                        case RP_RB: eNewRP = RP_MB; break;
                    }
            }
            break;
            case KEY_RIGHT:
            {
                if( !(m_nState & CS_NOHORZ) )
                    switch( eNewRP )
                    {
                        case RP_LT: eNewRP = RP_MT; break;
                        case RP_MT: eNewRP = RP_RT; break;
                        case RP_LM: eNewRP = bUseMM ? RP_MM : RP_RM; break;
                        case RP_MM: eNewRP = RP_RM; break;
                        case RP_LB: eNewRP = RP_MB; break;
                        case RP_MB: eNewRP = RP_RB; break;
                    }
            }
            break;
            default:
                Control::KeyInput( rKeyEvt );
                return;
        }
        if( eNewRP != eRP )
        {
            SetActualRP( eNewRP );

            if( WINDOW_TABPAGE == GetParent()->GetType() )
                ( (SvxTabPage*) GetParent() )->PointChanged( this, eRP );

            SetFocusRect();
        }
    }
}

// -----------------------------------------------------------------------

void SvxRectCtl::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxRectCtl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
    else
        Window::DataChanged( rDCEvt );
}

/*************************************************************************
|*
|*  Zeichnet das Control (Rechteck mit 9 Kreisen)
|*
\************************************************************************/

void SvxRectCtl::Paint( const Rectangle& rRect )
{
    Point   aPtDiff( PixelToLogic( Point( 1, 1 ) ) );

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();

    SetLineColor( rStyles.GetDialogColor() );
    SetFillColor( rStyles.GetDialogColor() );
    DrawRect( Rectangle( Point(0,0), GetOutputSize() ) );

    if( IsEnabled() )
        SetLineColor( rStyles.GetLabelTextColor() );
    else
        SetLineColor( rStyles.GetShadowColor() );

    SetFillColor();

    switch( eCS )
    {

        case CS_RECT:
        case CS_SHADOW:
            if( !IsEnabled() )
            {
                Color aOldCol = GetLineColor();
                SetLineColor( rStyles.GetLightColor() );
                DrawRect( Rectangle( aPtLT + aPtDiff, aPtRB + aPtDiff ) );
                SetLineColor( aOldCol );
            }
            DrawRect( Rectangle( aPtLT, aPtRB ) );
        break;

        case CS_LINE:
            if( !IsEnabled() )
            {
                Color aOldCol = GetLineColor();
                SetLineColor( rStyles.GetLightColor() );
                DrawLine( aPtLM - Point( 2 * nBorderWidth, 0) + aPtDiff,
                          aPtRM + Point( 2 * nBorderWidth, 0 ) + aPtDiff );
                SetLineColor( aOldCol );
            }
            DrawLine( aPtLM - Point( 2 * nBorderWidth, 0),
                      aPtRM + Point( 2 * nBorderWidth, 0 ) );
        break;

        case CS_ANGLE:
            if( !IsEnabled() )
            {
                Color aOldCol = GetLineColor();
                SetLineColor( rStyles.GetLightColor() );
                DrawLine( aPtLT + aPtDiff, aPtRB + aPtDiff );
                DrawLine( aPtLB + aPtDiff, aPtRT + aPtDiff );
                DrawLine( aPtLM + aPtDiff, aPtRM + aPtDiff );
                DrawLine( aPtMT + aPtDiff, aPtMB + aPtDiff );
                SetLineColor( aOldCol );
            }
            DrawLine( aPtLT, aPtRB );
            DrawLine( aPtLB, aPtRT );
            DrawLine( aPtLM, aPtRM );
            DrawLine( aPtMT, aPtMB );
        break;

        default:
            break;
    }
    SetFillColor( GetBackground().GetColor() );

    Size aBtnSize( 11, 11 );
    Size aDstBtnSize(  PixelToLogic( aBtnSize ) );
    Point aToCenter( aDstBtnSize.Width() >> 1, aDstBtnSize.Height() >> 1);
    Point aBtnPnt1( IsEnabled()?0:22,0 );
    Point aBtnPnt2( 11,0 );
    Point aBtnPnt3( 22,0 );

    BOOL bNoHorz = (m_nState & CS_NOHORZ) != 0;
    BOOL bNoVert = (m_nState & CS_NOVERT) != 0;

    Bitmap&         rBitmap = *GetBitmap();

    // #103516# CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(IsCompletelyDisabled())
    {
        DrawBitmap( aPtLT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtMT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtRT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtLM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        if( eCS == CS_RECT || eCS == CS_LINE )
            DrawBitmap( aPtMM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtRM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtLB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtMB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
        DrawBitmap( aPtRB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap );
    }
    else
    {
        DrawBitmap( aPtLT - aToCenter, aDstBtnSize, (bNoHorz | bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );
        DrawBitmap( aPtMT - aToCenter, aDstBtnSize, bNoVert?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );
        DrawBitmap( aPtRT - aToCenter, aDstBtnSize, (bNoHorz | bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );

        DrawBitmap( aPtLM - aToCenter, aDstBtnSize, bNoHorz?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );

        // Mittelpunkt bei Rechteck und Linie
        if( eCS == CS_RECT || eCS == CS_LINE )
            DrawBitmap( aPtMM - aToCenter, aDstBtnSize, aBtnPnt1, aBtnSize, rBitmap );

        DrawBitmap( aPtRM - aToCenter, aDstBtnSize, bNoHorz?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );

        DrawBitmap( aPtLB - aToCenter, aDstBtnSize, (bNoHorz | bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );
        DrawBitmap( aPtMB - aToCenter, aDstBtnSize, bNoVert?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );
        DrawBitmap( aPtRB - aToCenter, aDstBtnSize, (bNoHorz | bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap );
    }

    // draw active button, avoid center pos for angle
    // #103516# CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(!IsCompletelyDisabled())
    {
        if( IsEnabled() && (eCS != CS_ANGLE || aPtNew != aPtMM) )
        {
            Point       aCenterPt( aPtNew );
            aCenterPt -= aToCenter;

            DrawBitmap( aCenterPt, aDstBtnSize, aBtnPnt2, aBtnSize, rBitmap );
        }
    }
}

/*************************************************************************
|*
|*  Konvertiert RECT_POINT in Point
|*
\************************************************************************/

Point SvxRectCtl::GetPointFromRP( RECT_POINT eRP) const
{
    switch( eRP )
    {
        case RP_LT: return aPtLT;
        case RP_MT: return aPtMT;
        case RP_RT: return aPtRT;
        case RP_LM: return aPtLM;
        case RP_MM: return aPtMM;
        case RP_RM: return aPtRM;
        case RP_LB: return aPtLB;
        case RP_MB: return aPtMB;
        case RP_RB: return aPtRB;
    }
    return( aPtMM ); // default
}


void SvxRectCtl::SetFocusRect( const Rectangle* pRect )
{
    HideFocus();

    if( pRect )
        ShowFocus( *pRect );
    else
        ShowFocus( CalculateFocusRectangle() );
}

Point SvxRectCtl::SetActualRPWithoutInvalidate( RECT_POINT eNewRP )
{
    Point aPtLast = aPtNew;
    aPtNew = GetPointFromRP( eNewRP );

    if( (m_nState & CS_NOHORZ) != 0 )
        aPtNew.X() = aPtMM.X();

    if( (m_nState & CS_NOVERT) != 0 )
        aPtNew.Y() = aPtMM.Y();

    eNewRP = GetRPFromPoint( aPtNew );

    eDefRP = eNewRP;
    eRP = eNewRP;

    return aPtLast;
}

void SvxRectCtl::GetFocus()
{
    SetFocusRect();
}


void SvxRectCtl::LoseFocus()
{
    HideFocus();
}


Point SvxRectCtl::GetApproxLogPtFromPixPt( const Point& rPt ) const
{
    Point   aPt = PixelToLogic( rPt );
    long    x;
    long    y;

    if( ( m_nState & CS_NOHORZ ) == 0 )
    {
        if( aPt.X() < aSize.Width() / 3 )
            x = aPtLT.X();
        else if( aPt.X() < aSize.Width() * 2 / 3 )
            x = aPtMM.X();
        else
            x = aPtRB.X();
    }
    else
        x = aPtMM.X();

    if( ( m_nState & CS_NOVERT ) == 0 )
    {
        if( aPt.Y() < aSize.Height() / 3 )
            y = aPtLT.Y();
        else if( aPt.Y() < aSize.Height() * 2 / 3 )
            y = aPtMM.Y();
        else
            y = aPtRB.Y();
    }
    else
            y = aPtMM.Y();

    return Point( x, y );
}


/*************************************************************************
|*
|*  Konvertiert Point in RECT_POINT
|*
\************************************************************************/

RECT_POINT SvxRectCtl::GetRPFromPoint( Point aPt ) const
{
    if     ( aPt == aPtLT) return RP_LT;
    else if( aPt == aPtMT) return RP_MT;
    else if( aPt == aPtRT) return RP_RT;
    else if( aPt == aPtLM) return RP_LM;
    else if( aPt == aPtRM) return RP_RM;
    else if( aPt == aPtLB) return RP_LB;
    else if( aPt == aPtMB) return RP_MB;
    else if( aPt == aPtRB) return RP_RB;

    else
        return RP_MM; // default
}

/*************************************************************************
|*
|*  Bewirkt den Ursprungszustand des Controls
|*
\************************************************************************/

void SvxRectCtl::Reset()
{
    aPtNew = GetPointFromRP( eDefRP );
    eRP = eDefRP;
    Invalidate();
}

/*************************************************************************
|*
|*  Gibt den aktuell ausgewaehlten RECT_POINT zurück
|*
\************************************************************************/

RECT_POINT SvxRectCtl::GetActualRP() const
{
    return( eRP );
}

/*************************************************************************
|*
|*  Gibt den aktuell ausgewaehlten RECT_POINT zurück
|*
\************************************************************************/

void SvxRectCtl::SetActualRP( RECT_POINT eNewRP )
{
    Point aPtLast( SetActualRPWithoutInvalidate( eNewRP ) );

    Invalidate( Rectangle( aPtLast - Point( nRadius, nRadius ), aPtLast + Point( nRadius, nRadius ) ) );
    Invalidate( Rectangle( aPtNew - Point( nRadius, nRadius ), aPtNew + Point( nRadius, nRadius ) ) );

    // notify accessibility object about change
    if( pAccContext )
        pAccContext->selectChild( eNewRP );
}

void SvxRectCtl::SetState( CTL_STATE nState )
{
    m_nState = nState;

    Point aPtLast( GetPointFromRP( eRP ) );
    Point aPtNew( aPtLast );

    if( (m_nState & CS_NOHORZ) != 0 )
        aPtNew.X() = aPtMM.X();

    if( (m_nState & CS_NOVERT) != 0 )
        aPtNew.Y() = aPtMM.Y();

    eRP = GetRPFromPoint( aPtNew );
    Invalidate();

    if( WINDOW_TABPAGE == GetParent()->GetType() )
        ( (SvxTabPage*) GetParent() )->PointChanged( this, eRP );
}

UINT8 SvxRectCtl::GetNumOfChilds( void ) const
{
    return ( eCS == CS_ANGLE )? 8 : 9;
}

Rectangle SvxRectCtl::CalculateFocusRectangle( void ) const
{
    Size        aDstBtnSize( PixelToLogic( Size( 15, 15 ) ) );
    return Rectangle( aPtNew - Point( aDstBtnSize.Width() >> 1, aDstBtnSize.Height() >> 1 ), aDstBtnSize );
}

Rectangle SvxRectCtl::CalculateFocusRectangle( RECT_POINT eRectPoint ) const
{
    Rectangle   aRet;
    RECT_POINT  eOldRectPoint = GetActualRP();

    if( eOldRectPoint == eRectPoint )
        aRet = CalculateFocusRectangle();
    else
    {
        SvxRectCtl* pThis = const_cast< SvxRectCtl* >( this );

        pThis->SetActualRPWithoutInvalidate( eRectPoint );      // no invalidation because it's only temporary!
        aRet = CalculateFocusRectangle();

        pThis->SetActualRPWithoutInvalidate( eOldRectPoint );   // no invalidation because nothing has changed!
    }

    return aRet;
}

Reference< XAccessible > SvxRectCtl::CreateAccessible()
{
    Window*                     pParent = GetAccessibleParentWindow();

    DBG_ASSERT( pParent, "-SvxRectCtl::CreateAccessible(): No Parent!" );

    Reference< XAccessible >    xAccParent  = pParent->GetAccessible();
    if( xAccParent.is() )
    {
        pAccContext = new SvxRectCtlAccessibleContext( xAccParent, *this );
        pAccContext->acquire();

        SetActualRP( GetActualRP() );

        return pAccContext;
    }
    else
        return Reference< XAccessible >();
}

RECT_POINT SvxRectCtl::GetApproxRPFromPixPt( const ::com::sun::star::awt::Point& r ) const
{
    return GetRPFromPoint( GetApproxLogPtFromPixPt( Point( r.X, r.Y ) ) );
}

// #103516# CompletelyDisabled() added to have a disabled state for SvxRectCtl
void SvxRectCtl::DoCompletelyDisable(sal_Bool bNew)
{
    mbCompleteDisable = bNew;
    Invalidate();
}

/*************************************************************************
|*
|* Konstruktor ohne Size-Parameter
|*
\************************************************************************/

SvxAngleCtl::SvxAngleCtl( Window* pParent, const ResId& rResId ) :

    SvxRectCtl( pParent, rResId ),

    aFont( Application::GetSettings().GetStyleSettings().GetAppFont() )
{
    aFontSize = Size( 250, 400 );
    Initialize();
}

/*************************************************************************
|*
|* Konstruktor mit Size-Parameter
|*
\************************************************************************/

SvxAngleCtl::SvxAngleCtl( Window* pParent, const ResId& rResId, Size aSize ) :

    SvxRectCtl( pParent, rResId ),

    aFont( Application::GetSettings().GetStyleSettings().GetAppFont() )
{
    aFontSize = aSize;
    Initialize();
}

/*************************************************************************
|*
|* Initialisierung
|*
\************************************************************************/

void SvxAngleCtl::Initialize()
{
    bPositive = TRUE;

    // aFont.SetName( "Helvetica" );
    aFont.SetSize( aFontSize );
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetTransparent( FALSE );

    SetFont( aFont );
}

/*************************************************************************
|*
|*  Zeichnet das (Mini-)Koordinatensystem
|*
\************************************************************************/

void SvxAngleCtl::Paint( const Rectangle& rRect )
{
    SetLineColor( Color( COL_BLACK ) );     // PEN_DOT ???
    DrawLine( aPtLT - Point( 0, 0), aPtRB + Point( 0, 0 ) );
    DrawLine( aPtLB - Point( 0, 0), aPtRT + Point( 0, 0 ) );

    SetLineColor( Color( COL_BLACK ) );
    DrawLine( aPtLM - Point( 0, 0), aPtRM + Point( 0, 0 ) );
    DrawLine( aPtMT - Point( 0, 0), aPtMB + Point( 0, 0 ) );

    Point aDiff(aFontSize.Width() / 2, aFontSize.Height() / 2);

    DrawText( aPtLT - aDiff, UniString::CreateFromAscii(
                             RTL_CONSTASCII_STRINGPARAM( "135" ) ) );
    DrawText( aPtLM - aDiff, UniString::CreateFromAscii(
                             RTL_CONSTASCII_STRINGPARAM( "180" ) ) );

    if ( bPositive )
        DrawText( aPtLB - aDiff, UniString::CreateFromAscii(
                                 RTL_CONSTASCII_STRINGPARAM( "225" ) ) );
    else
        DrawText( aPtLB - aDiff, UniString::CreateFromAscii(
                                 RTL_CONSTASCII_STRINGPARAM( "-135" ) ) );

    aDiff.X() = aFontSize.Width();
    DrawText( aPtMT - aDiff, UniString::CreateFromAscii(
                             RTL_CONSTASCII_STRINGPARAM( "90" ) ) );
    DrawText( aPtRT - aDiff, UniString::CreateFromAscii(
                             RTL_CONSTASCII_STRINGPARAM( "45" ) ) );
    aDiff.X() = aDiff .X() * 3 / 2;

    if ( bPositive )
        DrawText( aPtMB - aDiff, UniString::CreateFromAscii(
                                 RTL_CONSTASCII_STRINGPARAM( "270" ) ) );
    else
        DrawText( aPtMB - aDiff, UniString::CreateFromAscii(
                                 RTL_CONSTASCII_STRINGPARAM( "-90" ) ) );

    DrawText( aPtRM - Point( 0, aDiff.Y() ), UniString::CreateFromAscii(
                                             RTL_CONSTASCII_STRINGPARAM( "0" ) ) );
    aDiff.X() = aFontSize.Width() * 2;

    if ( bPositive )
        DrawText( aPtRB - aDiff, UniString::CreateFromAscii(
                                 RTL_CONSTASCII_STRINGPARAM( "315" ) ) );
    else
        DrawText( aPtRB - aDiff, UniString::CreateFromAscii(
                                 RTL_CONSTASCII_STRINGPARAM( "-45" ) ) );
}

/*************************************************************************
|*
|*  Control zum Editieren von Bitmaps
|*
\************************************************************************/

SvxPixelCtl::SvxPixelCtl( Window* pParent, const ResId& rResId, USHORT nNumber ) :
                        Control     ( pParent, rResId ),
                        nLines      ( nNumber ),
                        bPaintable  ( TRUE )
{
    // SetMapMode( MAP_100TH_MM );
    aRectSize = GetOutputSize();

    SetPixelColor( Color( COL_BLACK ) );
    SetBackgroundColor( Color( COL_WHITE ) );
    SetLineColor( Application::GetSettings().GetStyleSettings().GetShadowColor() );

    nSquares = nLines * nLines;
    pPixel = new USHORT[ nSquares ];

    // Reset(); <-- wird vom Dialog aufgerufen
}

/*************************************************************************
|*
|*  Destruktor dealociert dyn. Array
|*
\************************************************************************/

SvxPixelCtl::~SvxPixelCtl( )
{
    delete []pPixel;
}

/*************************************************************************
|*
|*  Wechselt die Vordergrund- ,bzw. Hintergrundfarbe
|*
\************************************************************************/

void SvxPixelCtl::ChangePixel( USHORT nPixel )
{
    if( *( pPixel + nPixel) == 0 )
        *( pPixel + nPixel) = 1; // koennte erweitert werden auf mehrere Farben
    else
        *( pPixel + nPixel) = 0;
}

/*************************************************************************
|*
|*  Das angeklickte Rechteck wird ermittelt um die Farbe zu wechseln
|*
\************************************************************************/

void SvxPixelCtl::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPt = PixelToLogic( rMEvt.GetPosPixel() );
    Point aPtTl, aPtBr;
    USHORT  nX, nY;

    nX = (USHORT) ( aPt.X() * nLines / aRectSize.Width() );
    nY = (USHORT) ( aPt.Y() * nLines / aRectSize.Height() );

    ChangePixel( nX + nY * nLines );

    aPtTl.X() = aRectSize.Width() * nX / nLines + 1;
    aPtBr.X() = aRectSize.Width() * (nX + 1) / nLines - 1;
    aPtTl.Y() = aRectSize.Height() * nY / nLines + 1;
    aPtBr.Y() = aRectSize.Height() * (nY + 1) / nLines - 1;

    Invalidate( Rectangle( aPtTl, aPtBr ) );

    if( WINDOW_TABPAGE == GetParent()->GetType() )
        ( (SvxTabPage*) GetParent() )->PointChanged( this, RP_MM ); // RectPoint ist dummy
}

/*************************************************************************
|*
|*  Zeichnet das Control (Rechteck mit 9 Kreisen)
|*
\************************************************************************/

void SvxPixelCtl::Paint( const Rectangle& rRect )
{
    USHORT  i, j, nTmp;
    Point   aPtTl, aPtBr;

    if( bPaintable )
    {
        // Linien Zeichnen
        Control::SetLineColor( aLineColor );
        for( i = 1; i < nLines; i++)
        {
            // horizontal
            nTmp = (USHORT) ( aRectSize.Height() * i / nLines );
            DrawLine( Point( 0, nTmp ), Point( aRectSize.Width(), nTmp ) );
            // vertikal
            nTmp = (USHORT) ( aRectSize.Width() * i / nLines );
            DrawLine( Point( nTmp, 0 ), Point( nTmp, aRectSize.Height() ) );
        }

        // Rechtecke (Quadrate) zeichnen
        Control::SetLineColor();
        USHORT nLastPixel = *pPixel ? 0 : 1;

        for( i = 0; i < nLines; i++)
        {
            aPtTl.Y() = aRectSize.Height() * i / nLines + 1;
            aPtBr.Y() = aRectSize.Height() * (i + 1) / nLines - 1;

            for( j = 0; j < nLines; j++)
            {
                aPtTl.X() = aRectSize.Width() * j / nLines + 1;
                aPtBr.X() = aRectSize.Width() * (j + 1) / nLines - 1;

                if ( *( pPixel + i * nLines + j ) != nLastPixel )
                {
                    nLastPixel = *( pPixel + i * nLines + j );
                    // Farbe wechseln: 0 -> Hintergrundfarbe
                    SetFillColor( nLastPixel ? aPixelColor : aBackgroundColor );
                }
                DrawRect( Rectangle( aPtTl, aPtBr ) );
            }
        }
    } // bPaintable
    else
    {
        SetBackground( Wallpaper( Color( COL_LIGHTGRAY ) ) );
        Control::SetLineColor( Color( COL_LIGHTRED ) );
        DrawLine( Point( 0, 0 ), Point( aRectSize.Width(), aRectSize.Height() ) );
        DrawLine( Point( 0, aRectSize.Height() ), Point( aRectSize.Width(), 0 ) );
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxPixelCtl::SetXBitmap( const XOBitmap& rXBmp )
{
    if( rXBmp.GetBitmapType() == XBITMAP_8X8 )
    {
        aPixelColor = rXBmp.GetPixelColor();
        aBackgroundColor = rXBmp.GetBackgroundColor();

        USHORT* pArray = rXBmp.GetPixelArray();

        for( USHORT i = 0; i < nSquares; i++ )
            *( pPixel + i ) = *( pArray + i );
    }
}

/*************************************************************************
|*
|*  Gibt ein bestimmtes Pixel zurueck
|*
\************************************************************************/

USHORT SvxPixelCtl::GetPixel( const USHORT nPixel )
{
    return( *( pPixel + nPixel ) );
}

/*************************************************************************
|*
|*  Bewirkt den Ursprungszustand des Controls
|*
\************************************************************************/

void SvxPixelCtl::Reset()
{
    // Initialisierung des Arrays
    for( USHORT i = 0; i < nSquares; i++)
        *(pPixel + i) = 0;
    Invalidate();
}

/*************************************************************************
|*
|*  Ctor: BitmapCtl fuer SvxPixelCtl
|*
\************************************************************************/

SvxBitmapCtl::SvxBitmapCtl( Window* pParent, const Size& rSize )
{
    aSize = rSize;
    // aVD.SetOutputSizePixel( aSize );
}

/*************************************************************************
|*
|*  Dtor
|*
\************************************************************************/

SvxBitmapCtl::~SvxBitmapCtl()
{
}

/*************************************************************************
|*
|*  BitmapCtl:  Gibt die Bitmap zurueck
|*
\************************************************************************/

XOBitmap SvxBitmapCtl::GetXBitmap()
{
    XOBitmap aXOBitmap( pBmpArray, aPixelColor, aBackgroundColor );

    return( aXOBitmap );
}

/*************************************************************************
|*
|*  Fuellt die Listbox mit Farben und Strings
|*
\************************************************************************/

void ColorLB::Fill( const XColorTable* pColorTab )
{
    long nCount = pColorTab->Count();
    XColorEntry* pEntry;
    SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pColorTab->Get( i );
        InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }
    SetUpdateMode( TRUE );
}

/************************************************************************/

void ColorLB::Append( XColorEntry* pEntry, Bitmap* pBmp )
{
    InsertEntry( pEntry->GetColor(), pEntry->GetName() );
}

/************************************************************************/

void ColorLB::Modify( XColorEntry* pEntry, USHORT nPos, Bitmap* pBmp )
{
    RemoveEntry( nPos );
    InsertEntry( pEntry->GetColor(), pEntry->GetName(), nPos );
}

/*************************************************************************
|*
|*  Fuellt die Listbox mit Farben und Strings
|*
\************************************************************************/

void FillAttrLB::Fill( const XColorTable* pColorTab )
{
    long nCount = pColorTab->Count();
    XColorEntry* pEntry;
    SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pColorTab->Get( i );
        InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }
    SetUpdateMode( TRUE );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

HatchingLB::HatchingLB( Window* pParent, ResId Id, BOOL bUserDraw /*= TRUE*/ )
: ListBox( pParent, Id ),
  mpList ( NULL ),
  mbUserDraw( bUserDraw )
{
    EnableUserDraw( mbUserDraw );
}

HatchingLB::HatchingLB( Window* pParent, WinBits aWB, BOOL bUserDraw /*= TRUE*/ )
: ListBox( pParent, aWB ),
  mpList ( NULL ),
  mbUserDraw( bUserDraw )
{
    EnableUserDraw( mbUserDraw );
}

void HatchingLB::Fill( const XHatchList* pList )
{
    mpList = (XHatchList*)pList;
    XHatchEntry* pEntry;
    long nCount = pList->Count();

    SetUpdateMode( FALSE );

    if( mbUserDraw )
    {
        for( long i = 0; i < nCount; i++ )
            InsertEntry( pList->Get( i )->GetName() );
    }
    else
    {
        for( long i = 0; i < nCount; i++ )
        {
            pEntry = pList->Get( i );
            Bitmap* pBitmap = pList->GetBitmap( i );
            if( pBitmap )
                InsertEntry( pEntry->GetName(), *pBitmap );
            else
                InsertEntry( pEntry->GetName() );
        }
    }

    SetUpdateMode( TRUE );
}

void HatchingLB::UserDraw( const UserDrawEvent& rUDEvt )
{
    if( mpList != NULL )
    {
        // Draw gradient with borderrectangle
        const Rectangle& rDrawRect = rUDEvt.GetRect();
        Rectangle aRect( rDrawRect.nLeft+1, rDrawRect.nTop+1, rDrawRect.nLeft+33, rDrawRect.nBottom-1 );

        USHORT nId = rUDEvt.GetItemId();
        if( nId >= 0 && nId <= mpList->Count() )
        {
            OutputDevice* pDevice = rUDEvt.GetDevice();

            ULONG nOldDrawMode = pDevice->GetDrawMode();
            pDevice->SetDrawMode( GetDisplayBackground().GetColor().IsDark() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

            XHatch& rXHatch = mpList->Get( rUDEvt.GetItemId() )->GetHatch();
            MapMode aMode( MAP_100TH_MM );
            Hatch aHatch( (HatchStyle) rXHatch.GetHatchStyle(),
                          rXHatch.GetColor(),
                          rUDEvt.GetDevice()->LogicToPixel( Point( rXHatch.GetDistance(), 0 ), aMode ).X(),
                          (sal_uInt16)rXHatch.GetAngle() );
            const Polygon aPolygon( aRect );
            const PolyPolygon aPolypoly( aPolygon );
            pDevice->DrawHatch( aPolypoly, aHatch );

            pDevice->SetLineColor( COL_BLACK );
            pDevice->SetFillColor();
            pDevice->DrawRect( aRect );

            pDevice->SetDrawMode( nOldDrawMode );

            // Draw name
            pDevice->DrawText( Point( aRect.nRight+7, aRect.nTop-1 ), mpList->Get( rUDEvt.GetItemId() )->GetName() );
        }
    }
}

/************************************************************************/

void HatchingLB::Append( XHatchEntry* pEntry, Bitmap* pBmp )
{
    if( pBmp )
        InsertEntry( pEntry->GetName(), *pBmp );
    else
        InsertEntry( pEntry->GetName() );
}

/************************************************************************/

void HatchingLB::Modify( XHatchEntry* pEntry, USHORT nPos, Bitmap* pBmp )
{
    RemoveEntry( nPos );

    if( pBmp )
        InsertEntry( pEntry->GetName(), *pBmp, nPos );
    else
        InsertEntry( pEntry->GetName(), nPos );
}

/************************************************************************/

void HatchingLB::SelectEntryByList( const XHatchList* pList, const String& rStr,
                                    const XHatch& rHatch, USHORT nDist )
{
    long nCount = pList->Count();
    XHatchEntry* pEntry;
    BOOL bFound = FALSE;
    String aStr;

    long i;
    for( i = 0; i < nCount && !bFound; i++ )
    {
        pEntry = pList->Get( i );

        aStr = pEntry->GetName();

        if( rStr == aStr && rHatch == pEntry->GetHatch() )
            bFound = TRUE;
    }
    if( bFound )
        SelectEntryPos( (USHORT) ( i - 1 + nDist ) );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

void FillAttrLB::Fill( const XHatchList* pList )
{
    long nCount = pList->Count();
    XHatchEntry* pEntry;
    ListBox::SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pList->Get( i );
        Bitmap* pBitmap = pList->GetBitmap( i );
        if( pBitmap )
            ListBox::InsertEntry( pEntry->GetName(), *pBitmap );
        else
            InsertEntry( pEntry->GetName() );
    }
    ListBox::SetUpdateMode( TRUE );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

GradientLB::GradientLB( Window* pParent, ResId Id, BOOL bUserDraw /*= TRUE*/ )
: ListBox( pParent, Id ),
  mpList(NULL),
  mbUserDraw( bUserDraw )
{
    EnableUserDraw( mbUserDraw);
}

GradientLB::GradientLB( Window* pParent, WinBits aWB, BOOL bUserDraw /*= TRUE*/ )
: ListBox( pParent, aWB ),
  mpList(NULL),
  mbUserDraw( bUserDraw )
{
    EnableUserDraw( mbUserDraw );
}

void GradientLB::Fill( const XGradientList* pList )
{
    mpList = (XGradientList*)pList;
    XGradientEntry* pEntry;
    long nCount = pList->Count();

    SetUpdateMode( FALSE );

    if( mbUserDraw )
    {
        for( long i = 0; i < nCount; i++ )
            InsertEntry( pList->Get( i )->GetName() );
    }
    else
    {
        for( long i = 0; i < nCount; i++ )
        {
            pEntry = pList->Get( i );
            Bitmap* pBitmap = pList->GetBitmap( i );
            if( pBitmap )
                InsertEntry( pEntry->GetName(), *pBitmap );
            else
                InsertEntry( pEntry->GetName() );
        }
    }

    SetUpdateMode( TRUE );
}

void GradientLB::UserDraw( const UserDrawEvent& rUDEvt )
{
    if( mpList != NULL )
    {
        // Draw gradient with borderrectangle
        const Rectangle& rDrawRect = rUDEvt.GetRect();
        Rectangle aRect( rDrawRect.nLeft+1, rDrawRect.nTop+1, rDrawRect.nLeft+33, rDrawRect.nBottom-1 );

        USHORT nId = rUDEvt.GetItemId();
        if( nId >= 0 && nId <= mpList->Count() )
        {
            OutputDevice* pDevice = rUDEvt.GetDevice();

            XGradient& rXGrad = mpList->Get( rUDEvt.GetItemId() )->GetGradient();
            Gradient aGradient( (GradientStyle) rXGrad.GetGradientStyle(), rXGrad.GetStartColor(), rXGrad.GetEndColor() );
            aGradient.SetAngle( (USHORT)rXGrad.GetAngle() );
            aGradient.SetBorder( rXGrad.GetBorder() );
            aGradient.SetOfsX( rXGrad.GetXOffset() );
            aGradient.SetOfsY( rXGrad.GetYOffset() );
            aGradient.SetStartIntensity( rXGrad.GetStartIntens() );
            aGradient.SetEndIntensity( rXGrad.GetEndIntens() );
            aGradient.SetSteps( 255 );

            pDevice->DrawGradient( aRect, aGradient );

            pDevice->SetLineColor( COL_BLACK );
            pDevice->SetFillColor();
            pDevice->DrawRect( aRect );

            // Draw name
            pDevice->DrawText( Point( aRect.nRight+7, aRect.nTop-1 ), mpList->Get( rUDEvt.GetItemId() )->GetName() );
        }
    }
}

/************************************************************************/

void GradientLB::Append( XGradientEntry* pEntry, Bitmap* pBmp )
{
    if( pBmp )
        InsertEntry( pEntry->GetName(), *pBmp );
    else
        InsertEntry( pEntry->GetName() );
}

/************************************************************************/

void GradientLB::Modify( XGradientEntry* pEntry, USHORT nPos, Bitmap* pBmp )
{
    RemoveEntry( nPos );

    if( pBmp )
        InsertEntry( pEntry->GetName(), *pBmp, nPos );
    else
        InsertEntry( pEntry->GetName(), nPos );
}

/************************************************************************/

void GradientLB::SelectEntryByList( const XGradientList* pList, const String& rStr,
                                const XGradient& rGradient, USHORT nDist )
{
    long nCount = pList->Count();
    XGradientEntry* pEntry;
    BOOL bFound = FALSE;
    String aStr;

    long i;
    for( i = 0; i < nCount && !bFound; i++ )
    {
        pEntry = pList->Get( i );

        aStr = pEntry->GetName();

        if( rStr == aStr && rGradient == pEntry->GetGradient() )
            bFound = TRUE;
    }
    if( bFound )
        SelectEntryPos( (USHORT) ( i - 1 + nDist ) );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

void FillAttrLB::Fill( const XGradientList* pList )
{
    long nCount = pList->Count();
    XGradientEntry* pEntry;
    ListBox::SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pList->Get( i );
        Bitmap* pBitmap = pList->GetBitmap( i );
        if( pBitmap )
            ListBox::InsertEntry( pEntry->GetName(), *pBitmap );
        else
            InsertEntry( pEntry->GetName() );
    }
    ListBox::SetUpdateMode( TRUE );
}

/*************************************************************************
|*
|*  Konstruktor von BitmapLB
|*
\************************************************************************/

BitmapLB::BitmapLB( Window* pParent, ResId Id, BOOL bUserDraw /*= TRUE*/ )
: ListBox( pParent, Id ),
  mpList( NULL ),
  mbUserDraw( bUserDraw )
{
    aVD.SetOutputSizePixel( Size( 32, 16 ) );
    EnableUserDraw( mbUserDraw );
}

/************************************************************************/

void BitmapLB::SetVirtualDevice()
{
    if( aBitmap.GetSizePixel().Width() > 8 ||
        aBitmap.GetSizePixel().Height() > 8 )
    {
        aVD.DrawBitmap( Point( 0, 0 ), Size( 32, 16 ), aBitmap );
    }
    else
    {
        aVD.DrawBitmap( Point( 0, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 8, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 16, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 24, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 0, 8 ), aBitmap );
        aVD.DrawBitmap( Point( 8, 8 ), aBitmap );
        aVD.DrawBitmap( Point( 16, 8 ), aBitmap );
        aVD.DrawBitmap( Point( 24, 8 ), aBitmap );
    }
}

/************************************************************************/

void BitmapLB::Fill( const XBitmapList* pList )
{
    mpList = (XBitmapList*)pList;
    XBitmapEntry* pEntry;
    long nCount = pList->Count();

    SetUpdateMode( FALSE );

    if( mbUserDraw )
    {
        for( long i = 0; i < nCount; i++ )
            InsertEntry( pList->Get( i )->GetName() );
    }
    else
    {
        for( long i = 0; i < nCount; i++ )
        {
            pEntry = pList->Get( i );
            aBitmap = pEntry->GetXBitmap().GetBitmap();

            SetVirtualDevice();

            InsertEntry( pEntry->GetName(), aVD.GetBitmap( Point( 0, 2 ), Size( 32, 12 ) ) );
        }
    }

    SetUpdateMode( TRUE );
}

void BitmapLB::UserDraw( const UserDrawEvent& rUDEvt )
{
    if( mpList != NULL )
    {
        // Draw bitmap
        const Rectangle& rDrawRect = rUDEvt.GetRect();
        Rectangle aRect( rDrawRect.nLeft+1, rDrawRect.nTop+1, rDrawRect.nLeft+33, rDrawRect.nBottom-1 );

        USHORT nId = rUDEvt.GetItemId();
        if( nId >= 0 && nId <= mpList->Count() )
        {
            Rectangle aClipRect( rDrawRect.nLeft+1, rDrawRect.nTop+1, rDrawRect.nRight-1, rDrawRect.nBottom-1 );

            OutputDevice* pDevice = rUDEvt.GetDevice();
            pDevice->SetClipRegion( Region( aClipRect ) );

            aBitmap = mpList->Get( nId )->GetXBitmap().GetBitmap();

            long nPosBaseX = aRect.nLeft;
            long nPosBaseY = aRect.nTop;

            if( aBitmap.GetSizePixel().Width() > 8 ||
                aBitmap.GetSizePixel().Height() > 8 )
            {
                pDevice->DrawBitmap( Point( nPosBaseX, nPosBaseY ), Size( 32, 16 ), aBitmap );
            }
            else
            {
                pDevice->DrawBitmap( Point( nPosBaseX+ 0, nPosBaseY+0 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+ 8, nPosBaseY+0 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+16, nPosBaseY+0 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+24, nPosBaseY+0 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+ 0, nPosBaseY+8 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+ 8, nPosBaseY+8 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+16, nPosBaseY+8 ), aBitmap );
                pDevice->DrawBitmap( Point( nPosBaseX+24, nPosBaseY+8 ), aBitmap );
            }

            pDevice->SetClipRegion();

            // Draw name
            pDevice->DrawText( Point( aRect.nRight+7, aRect.nTop-1 ), mpList->Get( nId )->GetName() );
        }
    }
}

/************************************************************************/

void BitmapLB::Append( XBitmapEntry* pEntry, Bitmap* pBmp )
{
    if( pBmp )
    {
        aBitmap = pEntry->GetXBitmap().GetBitmap();
        SetVirtualDevice();
        InsertEntry( pEntry->GetName(), aVD.GetBitmap( Point( 0, 2 ), Size( 32, 12 ) ) );
    }
    else
        InsertEntry( pEntry->GetName() );
}

/************************************************************************/

void BitmapLB::Modify( XBitmapEntry* pEntry, USHORT nPos, Bitmap* pBmp )
{
    RemoveEntry( nPos );

    if( pBmp )
    {
        aBitmap = pEntry->GetXBitmap().GetBitmap();
        SetVirtualDevice();

        InsertEntry( pEntry->GetName(), aVD.GetBitmap( Point( 0, 2 ), Size( 32, 12 ) ), nPos );
    }
    else
        InsertEntry( pEntry->GetName() );
}

/************************************************************************/

void BitmapLB::SelectEntryByList( const XBitmapList* pList, const String& rStr,
                            const Bitmap& rBmp)
{
    long nCount = pList->Count();
    XBitmapEntry* pEntry;
    BOOL bFound = FALSE;

    long i;
    for( i = 0; i < nCount && !bFound; i++ )
    {
        pEntry = pList->Get( i );

        String aStr = pEntry->GetName();
        // Bitmap aBmp = pEntry->GetBitmap();

        if( rStr == aStr )
        {
            bFound = TRUE;
        }
    }
    if( bFound )
        SelectEntryPos( (USHORT) ( i - 1 ) );
}

/*************************************************************************
|*
|*  Konstruktor von FillAttrLB
|*
\************************************************************************/

FillAttrLB::FillAttrLB( Window* pParent, ResId Id ) :
                    ColorListBox( pParent, Id )
{
    aVD.SetOutputSizePixel( Size( 32, 16 ) );
}

/************************************************************************/

FillAttrLB::FillAttrLB( Window* pParent, WinBits aWB ) :
                    ColorListBox( pParent, aWB )
{
    aVD.SetOutputSizePixel( Size( 32, 16 ) );
}

/************************************************************************/

void FillAttrLB::SetVirtualDevice()
{
    if( aBitmap.GetSizePixel().Width() > 8 ||
        aBitmap.GetSizePixel().Height() > 8 )
    {
        aVD.DrawBitmap( Point( 0, 0 ), Size( 32, 16 ), aBitmap );
    }
    else
    {
        aVD.DrawBitmap( Point( 0, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 8, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 16, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 24, 0 ), aBitmap );
        aVD.DrawBitmap( Point( 0, 8 ), aBitmap );
        aVD.DrawBitmap( Point( 8, 8 ), aBitmap );
        aVD.DrawBitmap( Point( 16, 8 ), aBitmap );
        aVD.DrawBitmap( Point( 24, 8 ), aBitmap );
    }
}

/************************************************************************/

void FillAttrLB::Fill( const XBitmapList* pList )
{
    long nCount = pList->Count();
    XBitmapEntry* pEntry;
    ListBox::SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pList->Get( i );
        aBitmap = pEntry->GetXBitmap().GetBitmap();

        SetVirtualDevice();

        ListBox::InsertEntry( pEntry->GetName(), aVD.GetBitmap( Point( 0, 2 ), Size( 32, 12 ) ) );
    }
    ListBox::SetUpdateMode( TRUE );
}

/************************************************************************/

void FillAttrLB::SelectEntryByList( const XBitmapList* pList, const String& rStr,
                            const Bitmap& rBmp)
{
    long nCount = pList->Count();
    XBitmapEntry* pEntry;
    BOOL bFound = FALSE;

    long i;
    for( i = 0; i < nCount && !bFound; i++ )
    {
        pEntry = pList->Get( i );

        String aStr = pEntry->GetName();
        // Bitmap aBmp = pEntry->GetBitmap();

        if( rStr == aStr )
        {
            bFound = TRUE;
        }
        /*
        if( rStr == aStr && rBmp == aBmp )
            bFound = TRUE; */
    }
    if( bFound )
        SelectEntryPos( (USHORT) ( i - 1 ) );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

void FillTypeLB::Fill()
{
    ResMgr* pMgr = DIALOG_MGR();
    SetUpdateMode( FALSE );
    InsertEntry( String( ResId( RID_SVXSTR_INVISIBLE, pMgr ) ) );
    InsertEntry( String( ResId( RID_SVXSTR_COLOR, pMgr ) ) );
    InsertEntry( String( ResId( RID_SVXSTR_GRADIENT, pMgr ) ) );
    InsertEntry( String( ResId( RID_SVXSTR_HATCH, pMgr ) ) );
    InsertEntry( String( ResId( RID_SVXSTR_BITMAP, pMgr ) ) );
    SetUpdateMode( TRUE );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

void LineLB::Fill( const XDashList* pList )
{
    long nCount = pList->Count();
    XDashEntry* pEntry;
    SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pList->Get( i );
        Bitmap* pBitmap = const_cast<XDashList*>(pList)->CreateBitmapForUI( i );
        if( pBitmap )
        {
            InsertEntry( pEntry->GetName(), *pBitmap );
            delete pBitmap;
        }
        else
            InsertEntry( pEntry->GetName() );
    }
    SetUpdateMode( TRUE );
}

/************************************************************************/

void LineLB::Append( XDashEntry* pEntry, Bitmap* pBmp )
{
    if( pBmp )
        InsertEntry( pEntry->GetName(), *pBmp );
    else
        InsertEntry( pEntry->GetName() );
}

/************************************************************************/

void LineLB::Modify( XDashEntry* pEntry, USHORT nPos, Bitmap* pBmp )
{
    RemoveEntry( nPos );

    if( pBmp )
        InsertEntry( pEntry->GetName(), *pBmp, nPos );
    else
        InsertEntry( pEntry->GetName(), nPos );
}

/************************************************************************/

void LineLB::SelectEntryByList( const XDashList* pList, const String& rStr,
                                const XDash& rDash, USHORT nDist )
{
    long nCount = pList->Count();
    XDashEntry* pEntry;
    BOOL bFound = FALSE;
    String aStr;
    XDash aDash;

    long i;
    for( i = 0; i < nCount && !bFound; i++ )
    {
        pEntry = pList->Get( i );

        aStr = pEntry->GetName();
        aDash = pEntry->GetDash();

        if( rStr == aStr && rDash == aDash )
            bFound = TRUE;
    }
    if( bFound )
        SelectEntryPos( (USHORT) ( i - 1 + nDist ) );
}

/*************************************************************************
|*
|*  Fuellt die Listbox (vorlaeufig) mit Strings
|*
\************************************************************************/

void LineEndLB::Fill( const XLineEndList* pList, BOOL bStart )
{
    long nCount = pList->Count();
    XLineEndEntry* pEntry;
    VirtualDevice aVD;
    SetUpdateMode( FALSE );

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = pList->Get( i );
        Bitmap* pBitmap = const_cast<XLineEndList*>(pList)->CreateBitmapForUI( i );
        if( pBitmap )
        {
            Size aBmpSize( pBitmap->GetSizePixel() );
            aVD.SetOutputSizePixel( aBmpSize, FALSE );
            aVD.DrawBitmap( Point(), *pBitmap );
            InsertEntry( pEntry->GetName(),
                aVD.GetBitmap( bStart ? Point() : Point( aBmpSize.Width() / 2, 0 ),
                    Size( aBmpSize.Width() / 2, aBmpSize.Height() ) ) );

            delete pBitmap;
        }
        else
            InsertEntry( pEntry->GetName() );
    }
    SetUpdateMode( TRUE );
}

/************************************************************************/

void LineEndLB::Append( XLineEndEntry* pEntry, Bitmap* pBmp,
                        BOOL bStart )
{
    if( pBmp )
    {
        VirtualDevice aVD;
        Size aBmpSize( pBmp->GetSizePixel() );

        aVD.SetOutputSizePixel( aBmpSize, FALSE );
        aVD.DrawBitmap( Point(), *pBmp );
        InsertEntry( pEntry->GetName(),
            aVD.GetBitmap( bStart ? Point() : Point( aBmpSize.Width() / 2, 0 ),
                Size( aBmpSize.Width() / 2, aBmpSize.Height() ) ) );
    }
    else
        InsertEntry( pEntry->GetName() );
}

/************************************************************************/

void LineEndLB::Modify( XLineEndEntry* pEntry, USHORT nPos, Bitmap* pBmp,
                        BOOL bStart )
{
    RemoveEntry( nPos );

    if( pBmp )
    {
        VirtualDevice aVD;
        Size aBmpSize( pBmp->GetSizePixel() );

        aVD.SetOutputSizePixel( aBmpSize, FALSE );
        aVD.DrawBitmap( Point(), *pBmp );
        InsertEntry( pEntry->GetName(),
            aVD.GetBitmap( bStart ? Point() : Point( aBmpSize.Width() / 2, 0 ),
                Size( aBmpSize.Width() / 2, aBmpSize.Height() ) ), nPos );
    }
    else
        InsertEntry( pEntry->GetName(), nPos );
}

/*************************************************************************
|*
|*    SvxXLinePreview::SvxXLinePreview()
|*
*************************************************************************/

#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif

#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

SvxXLinePreview::SvxXLinePreview( Window* pParent, const ResId& rResId, XOutputDevice* pXOut )
:   Control( pParent, rResId ),
    mpXOutDev( pXOut ),
    mpLineObjA( 0L ),
    mpLineObjB( 0L ),
    mpLineObjC( 0L ),
    mpModel( 0L ),
    mpGraphic( 0L ),
    mbWithSymbol( sal_False )
{
    SetMapMode( MAP_100TH_MM );
    const Size aOutputSize(GetOutputSize());
    InitSettings( TRUE, TRUE );

    const sal_Int32 nDistance(500L);
    const sal_Int32 nAvailableLength(aOutputSize.Width() - (4 * nDistance));

    // create model
    mpModel = new SdrModel();
    mpModel->GetItemPool().FreezeIdRanges();

    // create DrawObectA
    const sal_Int32 aYPosA(aOutputSize.Height() / 2);
    const Point aPointA1( nDistance,  aYPosA);
    const Point aPointA2( aPointA1.X() + ((nAvailableLength * 14) / 20), aYPosA );
    mpLineObjA = new SdrPathObj(aPointA1, aPointA2);
    mpLineObjA->SetModel(mpModel);

    // create DrawObectB
    const sal_Int32 aYPosB1((aOutputSize.Height() * 3) / 4);
    const sal_Int32 aYPosB2((aOutputSize.Height() * 1) / 4);
    const Point aPointB1( aPointA2.X() + nDistance,  aYPosB1);
    const Point aPointB2( aPointB1.X() + ((nAvailableLength * 2) / 20), aYPosB2 );
    const Point aPointB3( aPointB2.X() + ((nAvailableLength * 2) / 20), aYPosB1 );
    XPolygon aPolygonB(3);
    aPolygonB[0] = aPointB1;
    aPolygonB[1] = aPointB2;
    aPolygonB[2] = aPointB3;
    mpLineObjB = new SdrPathObj(OBJ_PLIN, XPolyPolygon(aPolygonB));
    mpLineObjB->SetModel(mpModel);

    // create DrawObectC
    const Point aPointC1( aPointB3.X() + nDistance,  aYPosB1);
    const Point aPointC2( aPointC1.X() + ((nAvailableLength * 1) / 20), aYPosB2 );
    const Point aPointC3( aPointC2.X() + ((nAvailableLength * 1) / 20), aYPosB1 );
    XPolygon aPolygonC(3);
    aPolygonC[0] = aPointC1;
    aPolygonC[1] = aPointC2;
    aPolygonC[2] = aPointC3;
    mpLineObjC = new SdrPathObj(OBJ_PLIN, XPolyPolygon(aPolygonC));
    mpLineObjC->SetModel(mpModel);

    //  Draw the control's border as a flat thin black line.
    SetBorderStyle (WINDOW_BORDER_MONO);
    SetDrawMode( GetDisplayBackground().GetColor().IsDark() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
}

SvxXLinePreview::~SvxXLinePreview()
{
    delete mpLineObjA;
    delete mpLineObjB;
    delete mpLineObjC;
    delete mpModel;
}

// -----------------------------------------------------------------------

void SvxXLinePreview::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxXLinePreview::SetSymbol(Graphic* p,const Size& s)
{
    mpGraphic = p;
    maSymbolSize = s;
}

// -----------------------------------------------------------------------

void SvxXLinePreview::ResizeSymbol(const Size& s)
{
    if ( s != maSymbolSize )
    {
        maSymbolSize = s;
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void SvxXLinePreview::SetLineAttributes(const SfxItemSet& rItemSet)
{
    // Set ItemSet at objects
    mpLineObjA->SetMergedItemSet(rItemSet);

    // At line joints, do not use arrows
    SfxItemSet aTempSet(rItemSet);
    aTempSet.ClearItem(XATTR_LINESTART);
    aTempSet.ClearItem(XATTR_LINEEND);

    mpLineObjB->SetMergedItemSet(aTempSet);
    mpLineObjC->SetMergedItemSet(aTempSet);
}

// -----------------------------------------------------------------------

void SvxXLinePreview::Paint( const Rectangle& rRect )
{
    const Size aOutputSize(GetOutputSize());

    // paint objects
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(mpLineObjA);
    aObjectVector.push_back(mpLineObjB);
    aObjectVector.push_back(mpLineObjC);

    sdr::contact::ObjectContactOfObjListPainter aPainter(aObjectVector);
    sdr::contact::DisplayInfo aDisplayInfo;
    SdrPaintInfoRec aInfoRec;

    aDisplayInfo.SetExtendedOutputDevice((ExtOutputDevice*)mpXOutDev);
    aDisplayInfo.SetPaintInfoRec(&aInfoRec);
    aDisplayInfo.SetOutputDevice(mpXOutDev->GetOutDev());

    // keep draw hierarchy up-to-date
    aPainter.PreProcessDisplay(aDisplayInfo);

    // do processing
    aPainter.ProcessDisplay(aDisplayInfo);

    // prepare delete
    aPainter.PrepareDelete();

    if ( mbWithSymbol && mpGraphic )
    {
        Point aPos = Point( aOutputSize.Width() / 3, aOutputSize.Height() / 2 );
        aPos.X() -= maSymbolSize.Width() / 2;
        aPos.Y() -= maSymbolSize.Height() / 2;
        mpGraphic->Draw( this, aPos, maSymbolSize );
    }
}

// -----------------------------------------------------------------------

void SvxXLinePreview::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxXLinePreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
    else
        Control::DataChanged( rDCEvt );

   SetDrawMode( GetDisplayBackground().GetColor().IsDark() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
}

/*************************************************************************
|*
|*    SvxXRectPreview::SvxXRectPreview()
|*
*************************************************************************/

SvxXRectPreview::SvxXRectPreview( Window* pParent, const ResId& rResId, XOutputDevice* pXOut ) :

    Control ( pParent, rResId ),

    pXOutDev( pXOut )

{
    SetBorderStyle( WINDOW_BORDER_MONO );
    SetMapMode( MAP_100TH_MM );
    aRect = Rectangle( Point(), GetOutputSize() );
    InitSettings( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void SvxXRectPreview::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxXRectPreview::Paint( const Rectangle& rRect )
{
    pXOutDev->SetLineColor( Color( COL_TRANSPARENT ) );
    pXOutDev->DrawRect( aRect );
}

// -----------------------------------------------------------------------

void SvxXRectPreview::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxXRectPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
    else
        Control::DataChanged( rDCEvt );
}

/*************************************************************************
|*
|*    SvxXShadowPreview::SvxXShadowPreview()
|*
*************************************************************************/

SvxXShadowPreview::SvxXShadowPreview( Window* pParent, const ResId& rResId,
                                      XOutputDevice* pXOut, XOutdevItemPool* pXInPool ) :

    Control     ( pParent, rResId ),

    pXOutDev    ( pXOut ),
    pXPool      ( pXInPool ),
    pRectItem   ( NULL ),
    pShadowItem ( NULL )

{
    SetMapMode( MAP_100TH_MM );
    Size aSize = GetOutputSize();
    aSize.Width() = aSize.Width() / 3;
    aSize.Height() = aSize.Height() / 3;
    aRect = Rectangle( Point( aSize.Width(), aSize.Height() ), aSize );
    aShadow = Rectangle( Point( aSize.Width(), aSize.Height() ), aSize );
    InitSettings( TRUE, TRUE );

    //  Draw the control's border as a flat thin black line.
    SetBorderStyle (WINDOW_BORDER_MONO);
}

// -----------------------------------------------------------------------

SvxXShadowPreview::~SvxXShadowPreview()
{
    delete pRectItem;
}

// -----------------------------------------------------------------------

void SvxXShadowPreview::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxXShadowPreview::Paint( const Rectangle& rRect )
{
    aShadow.SetPos( aShadowPos + Point( aRect.GetWidth(), aRect.GetHeight() ) );
    if ( pShadowItem )
        pXOutDev->SetFillAttr( pShadowItem->GetItemSet() );
    pXOutDev->DrawRect( aShadow );
    if ( pRectItem )
        pXOutDev->SetFillAttr( pRectItem->GetItemSet() );
    pXOutDev->DrawRect( aRect );
}

// -----------------------------------------------------------------------

void SvxXShadowPreview::SetRectAttr( XFillAttrSetItem* pSetItem )
{
    if( pRectItem )
        delete pRectItem;
    pRectItem = (XFillAttrSetItem*) pSetItem->Clone( pXPool );
}

// -----------------------------------------------------------------------

void SvxXShadowPreview::SetShadowAttr( XFillAttrSetItem* pSetItem )
{
    pShadowItem = pSetItem;
}

// -----------------------------------------------------------------------

void SvxXShadowPreview::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxXShadowPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
    else
        Control::DataChanged( rDCEvt );
}


