/*************************************************************************
 *
 *  $RCSfile: fntcache.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: fme $ $Date: 2001-08-31 06:22:48 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VIEWSH_HXX
#include <viewsh.hxx>       // Bildschirmabgleich
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>      // Bildschirmabgleich abschalten, ViewOption
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>       // CH_BLANK + CH_BULLET
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx>     // SwDrawTextInfo
#endif

#ifdef VERTICAL_LAYOUT
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // SwTxtFrm
#endif
#endif



// Enable this to use the helpclass SwRVPMark
#ifdef DEBUG
#ifndef _RVP_MARK_HXX
#include <rvp_mark.hxx>
#endif
#endif

// globale Variablen, werden in FntCache.Hxx bekanntgegeben
// Der FontCache wird in TxtInit.Cxx _TXTINIT erzeugt und in _TXTEXIT geloescht
SwFntCache *pFntCache = NULL;
// Letzter Font, der durch ChgFntCache eingestellt wurde.
SwFntObj *pLastFont = NULL;
// Die "MagicNumber", die den Fonts zur Identifizierung verpasst wird
BYTE* pMagicNo = NULL;

Color *pSpellCol = 0;
Color *pWaveCol = 0;

long SwFntObj::nPixWidth;
MapMode* SwFntObj::pPixMap = NULL;
OutputDevice* SwFntObj::pPixOut = NULL;

#ifdef _RVP_MARK_HXX

void SwRVPMarker::Mark( const OutputDevice* pOut )
{
    if( pOut )
    {
        Color aOldCol = pOut->GetLineColor();
        Color aBlack = Color( COL_BLACK );
        if( aOldCol != aBlack )
        {
            ((OutputDevice*)pOut)->SetLineColor( aBlack );
            ((OutputDevice*)pOut)->DrawChord( Rectangle(0,1,0,1),
                                              Point(), Point() );
            ((OutputDevice*)pOut)->SetLineColor( aOldCol );
        }
        else
            ((OutputDevice*)pOut)->DrawChord( Rectangle(0,1,0,1),
                                              Point(), Point() );
    }
}

#endif

/*************************************************************************
|*
|*  SwFntCache::Flush()
|*
|*  Ersterstellung      AMA 16. Dez. 94
|*  Letzte Aenderung    AMA 16. Dez. 94
|*
|*************************************************************************/

void SwFntCache::Flush( )
{
    if ( pLastFont )
    {
        pLastFont->Unlock();
        pLastFont = NULL;
    }
    SwCache::Flush( );
}

/*************************************************************************
|*
|*  SwFntObj::SwFntObj(), ~SwFntObj()
|*
|*  Ersterstellung      AMA 7. Nov. 94
|*  Letzte Aenderung    AMA 7. Nov. 94
|*
|*************************************************************************/

SwFntObj::SwFntObj( const SwSubFont &rFont, const void *pOwner, ViewShell *pSh ) :
    SwCacheObj( (void*)pOwner ),
    aFont( rFont ),
    pScrFont( NULL ),
    pPrtFont( &aFont ),
    pPrinter( NULL ),
    nPropWidth( rFont.GetPropWidth() )
{
    nZoom = pSh ? pSh->GetViewOptions()->GetZoom() : USHRT_MAX;
    nLeading = USHRT_MAX;
    nPrtAscent = USHRT_MAX;
    nPrtHeight = USHRT_MAX;
    bPaintBlank = ( UNDERLINE_NONE != aFont.GetUnderline()
                  || STRIKEOUT_NONE != aFont.GetStrikeout() )
                  && !aFont.IsWordLineMode();
}

SwFntObj::~SwFntObj()
{
    if ( pScrFont != pPrtFont )
        delete pScrFont;
    if ( pPrtFont != &aFont )
        delete pPrtFont;
}

void SwFntObj::InitPrtFont( Printer *pPrt )
{
    if( pPrt )
    {
        if( pScrFont != pPrtFont )
            delete pScrFont;
        if( pPrtFont != &aFont )
            delete pPrtFont;
        _InitPrtFont( pPrt );
    }
}

void SwFntObj::_InitPrtFont( OutputDevice *pOut )
{
    const Font aOldFnt( pOut->GetFont() );
    pOut->SetFont( aFont );
    const FontMetric aWinMet( pOut->GetFontMetric() );
    pOut->SetFont( aOldFnt );
    long nWidth = ( aWinMet.GetSize().Width() * nPropWidth ) / 100;
    if( !nWidth )
        ++nWidth;
    pPrtFont = new Font( aFont );
    pPrtFont->SetSize( Size( nWidth, aFont.GetSize().Height() ) );
    pScrFont = NULL;
}

/*************************************************************************
 *
 *  USHORT SwFntObj::GetAscent( const OutputDevice *pOut )
 *
 *  Ersterstellung      AMA 7. Nov. 94
 *  Letzte Aenderung    AMA 7. Nov. 94
 *
 *  Beschreibung: liefern den Ascent des Fonts auf dem
 *  gewuenschten Outputdevice zurueck, ggf. muss der Bildschirmfont erst
 *  erzeugt werden. Dies wird in CheckScrFont ueberprueft und in
 *  CreateScrFont erledigt.
 *
 *************************************************************************/

USHORT SwFntObj::GetAscent( ViewShell *pSh, const OutputDevice *pOut )
{
    if( OUTDEV_PRINTER == pOut->GetOutDevType() )
    {
        if ( nPrtAscent == USHRT_MAX ) // DruckerAscent noch nicht bekannt?
        {
            CheckPrtFont( (Printer*)pOut );
            const Font aOldFnt( pOut->GetFont() );
            ( (OutputDevice *)pOut )->SetFont( *pPrtFont );
            const FontMetric aWinMet( pOut->GetFontMetric() );
            nPrtAscent = (USHORT) aWinMet.GetAscent();
            ( (OutputDevice *)pOut )->SetFont( aOldFnt );
        }
        return nPrtAscent + nLeading;
    }
    CheckScrFont( pSh, pOut );  // eventuell Bildschirmanpassung durchfuehren
    return nScrAscent;
}

USHORT SwFntObj::GetHeight( ViewShell *pSh, const OutputDevice *pOut )
{
    if( OUTDEV_PRINTER == pOut->GetOutDevType() )
    {
        if ( nPrtHeight == USHRT_MAX ) // PrinterHeight noch nicht bekannt?
        {
            CheckPrtFont( (Printer*)pOut );
            const Font aOldFnt( pOut->GetFont() );
            ( (OutputDevice *)pOut )->SetFont( *pPrtFont );
            nPrtHeight = (USHORT) pOut->GetTextHeight();
            ((OutputDevice *)pOut)->SetFont( aOldFnt );
        }
        return nPrtHeight + nLeading;
    }
    CheckScrFont( pSh, pOut );  // eventuell Bildschirmanpassung durchfuehren
    if ( nScrHeight == USHRT_MAX ) // ScreenHeight noch nicht bekannt?
    {
        const Font aOldFnt( pOut->GetFont() );
        ((OutputDevice *)pOut)->SetFont( *pPrtFont );
        nScrHeight = (USHORT) pOut->GetTextHeight();
        ((OutputDevice *)pOut)->SetFont( aOldFnt );
    }
    return nScrHeight;
}

/*************************************************************************
 *
 *  void SwFntObj::CreateScrFont( const OutputDevice *pOut ),
 *  void SwFntObj::ChooseFont( const OutputDevice *pOut )
 *
 *  Ersterstellung      AMA 7. Nov. 94
 *  Letzte Aenderung    AMA 7. Nov. 94
 *
 *  Beschreibung: CreateScrFont ermittelt mittels ChooseFont den fuer die
 *  Bildschirmdarstellung optimalen Font und haelt Ascent, Leading und die
 *  Buchstabenbreiten fest.
 *
 *************************************************************************/

// Es wird jetzt der im OutputDevice eingestellte Font mit dem ueber Drucker-
// abgleich einstellbaren verglichen. Derjenige, der dichter am Druckerfont
// liegt, wird eingestellt, wobei ein zu schmaler Font gegenueber einem zu
// breiten bevorzugt wird.

void SwFntObj::ChooseFont( ViewShell *pSh, OutputDevice *pOut )
{
static sal_Char __READONLY_DATA sStandardString[] = "Dies ist der Teststring";

    nScrHeight = USHRT_MAX;

    Printer *pPrt;
    // "No screen adj"
    if( pSh && ( !pSh->GetDoc()->IsBrowseMode() ||
                  pSh->GetViewOptions()->IsPrtFormat() ) &&
        ( 0 != ( pPrt = (Printer *)(pSh->GetDoc()->GetPrt() ) ) ) &&
        pPrt->IsValid() )
#ifdef MAC
    {
        CheckPrtFont( pPrt );
        pPrinter = pPrt;
    }
    else if( !pPrinter )
        CheckScrPrtFont( pOut );
    bSymbol = CHARSET_SYMBOL == pPrtFont->GetCharSet();
    nLeading = 0;
    pScrFont = pPrtFont;
#else
    {
        CheckPrtFont( pPrt );
        pPrinter = pPrt;
        Font aOldFnt( pPrt->GetFont() );
        pPrt->SetFont( *pPrtFont );
        FontMetric aMet = pPrt->GetFontMetric( );
        pScrFont = pPrtFont;
        bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();
        if ( nLeading == USHRT_MAX )
        {
            long nTmpLeading = (long)aMet.GetLeading();
            if ( nTmpLeading < 5 )
            {
                GetAscent( pSh, pPrt );
                GuessLeading( pSh, aMet );
            }
            else
                nLeading = 0;
        }
#ifdef DEBUG
        const XubString aDbgTxt1( pPrtFont->GetName() );
        const XubString aDbgTxt2( aMet.GetName() );
#endif
        if ( aMet.IsDeviceFont( ) )
        {
            if ( (RTL_TEXTENCODING_DONTKNOW == pPrtFont->GetCharSet() ||
                  FAMILY_DONTKNOW  == pPrtFont->GetFamily()  ||
                  PITCH_DONTKNOW   == pPrtFont->GetPitch()     ) &&
                 (RTL_TEXTENCODING_DONTKNOW == aMet.GetCharSet()  ||
                  FAMILY_DONTKNOW  == aMet.GetFamily()   ||
                  PITCH_DONTKNOW   == aMet.GetPitch()      )    )
            {
                // Das folgende ist teuer, aber selten: ein unbekannter Font
                // kann vom Drucker nicht vernuenftig zugeordnet werden. Dann
                // nehmen wir eben das Mapping des Bildschirms in Anspruch und
                // setzen den Familyname, Charset und Pitch wie dort. Dieser
                // Font wird nun nochmals auf dem Drucker eingestellt.
                Font aFnt1 = pOut->GetFontMetric();
                Font aFnt2( *pPrtFont );
                aFnt2.SetCharSet( aFnt1.GetCharSet() );
                aFnt2.SetFamily( aFnt1.GetFamily() );
                aFnt2.SetPitch( aFnt1.GetPitch() );
                pPrt->SetFont( aFnt2 );
                aMet = pPrt->GetFontMetric( );
            }
            const XubString aStandardStr( sStandardString,
                RTL_TEXTENCODING_MS_1252 );
            long nOWidth = pPrt->GetTextWidth( aStandardStr );
            long nSWidth = nOWidth - pOut->GetTextWidth( aStandardStr );
            nScrHeight = (USHORT) pOut->GetTextHeight();
            // Um Aerger mit dem Generic Printer aus dem Wege zu gehen.
            if( aMet.GetSize().Height() )
            {
                BOOL bScrSymbol;
                CharSet ePrtChSet = aMet.GetCharSet();
                // NoSymbol bedeutet, dass der Drucker sich fuer einen
                // Nicht-Symbol-Font entschieden hat.
                BOOL bNoSymbol = ( RTL_TEXTENCODING_DONTKNOW != ePrtChSet &&
                                   RTL_TEXTENCODING_SYMBOL != ePrtChSet );
                if ( bNoSymbol )
                    bScrSymbol = RTL_TEXTENCODING_SYMBOL ==
                                 pOut->GetFontMetric().GetCharSet();
                pOut->SetFont( aMet );  // Druckerabgleich
                if( bNoSymbol && ( bScrSymbol != ( RTL_TEXTENCODING_SYMBOL ==
                                        pOut->GetFontMetric().GetCharSet() ) ) )
                {
                    // Hier landen wir, wenn der Drucker keinen Symbolfont waehlt,
                    // aber genau einer der beiden Screenfonts ein Symbolfont ist.
                    // Wir nehmen dann eben den anderen.
                    if ( bScrSymbol )
                        pScrFont = new Font( aMet ); // mit Abgleich
                    else
                        pOut->SetFont( *pPrtFont ); // ohne Abgleich
                }
                else
                {
                    long nPWidth =
                        nOWidth - pOut->GetTextWidth( aStandardStr );
                    // lieber schmaler als breiter
                    BYTE nNeg = 0;
                    if ( nSWidth<0 ) { nSWidth *= -2; nNeg = 1; }
                    if ( nPWidth<0 ) { nPWidth *= -2; nNeg |= 2; }
                    if ( nSWidth <= nPWidth )
                    {
                        pOut->SetFont( *pPrtFont ); // ohne Abgleich
                        nPWidth = nSWidth;
                        nNeg &= 1;
                    }
                    else
                    {
                        pScrFont = new Font( aMet ); // mit Abgleich
                        nSWidth = nPWidth;
                        nNeg &= 2;
                    }
                    if( nNeg && nOWidth )
                    {
                        nPWidth *= 100;
                        nPWidth /= nOWidth;
                        // if the screen font is too wide, we try to reduce
                        // the font height and get a smaller one
                        if( nPWidth > 15 )
                        {
                            if( nPWidth > 80 )
                                nPWidth = 80;
                            nPWidth = 100 - nPWidth/4;
                            Size aTmp = pScrFont->GetSize();
                            aTmp.Height() *= nPWidth;
                            aTmp.Height() /= 100;
                            if( aTmp.Width() )
                            {
                                aTmp.Width() *= nPWidth;
                                aTmp.Width() /= 100;
                            }
                            Font *pNew = new Font( *pScrFont );
                            pNew->SetSize( aTmp );
                            pOut->SetFont( *pNew );
                            nPWidth = nOWidth -
                                      pOut->GetTextWidth( aStandardStr );
                            if( nPWidth < 0 ) { nPWidth *= -2; }
                            if( nPWidth < nSWidth )
                            {
                                if( pScrFont != pPrtFont )
                                    delete pScrFont;
                                pScrFont = pNew;
                            }
                            else
                            {
                                delete pNew;
                                pOut->SetFont( *pScrFont );
                            }
                        }
                    }
                }
            }
        }
        pPrt->SetFont( aOldFnt );
    }
    else
    {
        bSymbol = RTL_TEXTENCODING_SYMBOL == aFont.GetCharSet();
        if ( nLeading == USHRT_MAX )
            nLeading = 0;
        if( !pPrinter )
            CheckScrPrtFont( pOut );
        pScrFont = pPrtFont;
    }
#endif
    // Zoomfaktor ueberpruefen, z.B. wg. PrtOle2 beim Speichern
    {
        // Sollte der Zoomfaktor des OutputDevices nicht mit dem der View-
        // Options uebereinstimmen, so darf dieser Font nicht gecacht
        // werden, deshalb wird der Zoomfaktor auf einen "ungueltigen" Wert
        // gesetzt.
        long nTmp;
        if( pOut->GetMapMode().GetScaleX().IsValid() &&
            pOut->GetMapMode().GetScaleY().IsValid() &&
            pOut->GetMapMode().GetScaleX() == pOut->GetMapMode().GetScaleY() )
        {
            nTmp = ( 100 * pOut->GetMapMode().GetScaleX().GetNumerator() ) /
                     pOut->GetMapMode().GetScaleX().GetDenominator();
        }
        else
            nTmp = 0;
        if( nTmp != nZoom )
            nZoom = USHRT_MAX - 1;
    }
    nScrAscent = (USHORT)pOut->GetFontMetric().GetAscent();
}

void SwFntObj::CreateScrFont( ViewShell *pSh, const OutputDevice *pOut )
{
    Font aOldFnt( pOut->GetFont() );
    ((OutputDevice *)pOut)->SetFont( *pPrtFont );
    // Jetzt wird der "bessere" Font eingestellt.
    ChooseFont( pSh, (OutputDevice *)pOut );
    ((OutputDevice *)pOut)->SetFont( aOldFnt );
}

void SwFntObj::GuessLeading( ViewShell *pSh, const FontMetric& rMet )
{
//  Wie waere es mit 50% des Descents (StarMath??):
//  nLeading = USHORT( aMet.GetDescent() / 2 );

#if defined(WNT) || defined(WIN) || defined(PM2)
    OutputDevice *pWin = ( pSh && pSh->GetWin() ) ? pSh->GetWin() :
                         GetpApp()->GetDefaultDevice();
    if ( pWin )
    {
        MapMode aTmpMap( MAP_TWIP );
        MapMode aOldMap = pWin->GetMapMode( );
        pWin->SetMapMode( aTmpMap );
        const Font aOldFnt( pWin->GetFont() );
        pWin->SetFont( *pPrtFont );
        const FontMetric aWinMet( pWin->GetFontMetric() );
        const USHORT nWinHeight = USHORT( aWinMet.GetSize().Height() );
        if( pPrtFont->GetName().Search( aWinMet.GetName() ) < USHRT_MAX )
        {
            // Wenn das Leading auf dem Window auch 0 ist, dann
            // muss es auch so bleiben (vgl. StarMath!).
            long nTmpLeading = (long)aWinMet.GetLeading();
             // einen Versuch haben wir noch wg. 31003:
            if( nTmpLeading <= 0 )
            {
                pWin->SetFont( rMet );
                nTmpLeading = (long)pWin->GetFontMetric().GetLeading();
                if( nTmpLeading < 0 )
                    nLeading = 0;
                else
                    nLeading = USHORT(nTmpLeading);
            }
            else
            {
                nLeading = USHORT(nTmpLeading);
                // Manta-Hack #50153#:
                // Wer beim Leading luegt, luegt moeglicherweise auch beim
                // Ascent/Descent, deshalb wird hier ggf. der Font ein wenig
                // tiefergelegt, ohne dabei seine Hoehe zu aendern.
                long nDiff = Min( rMet.GetDescent() - aWinMet.GetDescent(),
                    aWinMet.GetAscent() - rMet.GetAscent() - nTmpLeading );
                if( nDiff > 0 )
                {
                    ASSERT( nPrtAscent < USHRT_MAX, "GuessLeading: PrtAscent-Fault" );
                    nPrtAscent += ( 2 * nDiff ) / 5;
                }
            }
        }
        else
        {
            // Wenn alle Stricke reissen, nehmen wir 15% der
            // Hoehe, ein von CL empirisch ermittelter Wert.
            nLeading = (nWinHeight * 15) / 100;
        }
        pWin->SetFont( aOldFnt );
        pWin->SetMapMode( aOldMap );
    }
    else
#endif
#ifdef MAC
        nLeading = (pPrtFont->GetSize().Height() * 15) / 100;
#else
        nLeading = 0;
#endif
}

/*************************************************************************
 *
 *  void SwFntObj::SetDeviceFont( const OutputDevice *pOut ),
 *
 *  Ersterstellung      AMA 7. Nov. 94
 *  Letzte Aenderung    AMA 7. Nov. 94
 *
 *  Beschreibung: stellt den Font am gewuenschten OutputDevice ein,
 *  am Bildschirm muss eventuell erst den Abgleich durchgefuehrt werden.
 *
 *************************************************************************/

void SwFntObj::SetDevFont( ViewShell *pSh, OutputDevice *pOut )
{
    if( OUTDEV_PRINTER != pOut->GetOutDevType() )
    {
        CheckScrFont( pSh, pOut );        // Bildschirm/Druckerabgleich
        if( !GetScrFont()->IsSameInstance( pOut->GetFont() ) )
            pOut->SetFont( *pScrFont );
        if( pPrinter && ( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) ) )
            pPrinter->SetFont( *pPrtFont );
    }
    else
    {
        CheckPrtFont( (Printer*)pOut );
        if( !pPrtFont->IsSameInstance( pOut->GetFont() ) )
            pOut->SetFont( *pPrtFont );
        if ( nLeading == USHRT_MAX )
        {
            FontMetric aMet( pOut->GetFontMetric() );
            bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();
            long nTmpLead = (long)aMet.GetLeading();
            if ( nTmpLead < 5 )
            {
                GetAscent( pSh, pOut );
                GuessLeading( pSh, aMet );
            }
            else
                nLeading = 0;
        }
    }
}

#define WRONG_SHOW_MIN 5
#define WRONG_SHOW_SMALL 11
#define WRONG_SHOW_MEDIUM 15

/*************************************************************************
 *
 * void SwFntObj::DrawText( ... )
 *
 *  Ersterstellung      AMA 16. Dez. 94
 *  Letzte Aenderung    AMA 16. Dez. 94
 *
 *  Beschreibung: Textausgabe
 *                  auf dem Bildschirm          => DrawTextArray
 *                  auf dem Drucker, !Kerning   => DrawText
 *                  auf dem Drucker + Kerning   => DrawStretchText
 *
 *************************************************************************/

// ER 09.07.95 20:34
// mit -Ox Optimierung stuerzt's unter win95 ab
// JP 12.07.95: unter WNT auch (i386);       Alpha ??
// global optimization off
#if defined( WNT ) && defined( MSC ) //&& defined( W40 )
#pragma optimize("g",off)
#endif

void SwFntObj::DrawText( SwDrawTextInfo &rInf )
{

static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
    BOOL bPrt = OUTDEV_PRINTER == rInf.GetOut().GetOutDevType();
    Font* pTmpFont = bPrt ? pPrtFont : GetScrFont();
    // HACK: UNDERLINE_WAVE darf nicht mehr missbraucht werden, daher
    // wird die graue Wellenlinie des ExtendedAttributSets zunaechst
    // in der Fontfarbe erscheinen.

    BOOL bCompress = rInf.GetKanaComp() && rInf.GetLen();
    ASSERT( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

    Point aPos( rInf.GetPos() );
    if( !bPrt )
    {
        if( rInf.GetpOut() != pPixOut || rInf.GetOut().GetMapMode() != *pPixMap )
        {
            *pPixMap = rInf.GetOut().GetMapMode();
            pPixOut = rInf.GetpOut();
            Size aTmp( 1, 1 );
            nPixWidth = rInf.GetOut().PixelToLogic( aTmp ).Width();
        }
        aPos.X() += nPixWidth;
    }
    BOOL bChgColor = FALSE;
    ColorData nNewColor;
    if( bPrt && rInf.GetShell()->GetViewOptions()->IsBlackFont() )
    {
        if( COL_BLACK != pTmpFont->GetColor().GetColor() )
        {
            nNewColor = COL_BLACK;
            bChgColor = TRUE;
        }
    }
    else if( COL_AUTO == pTmpFont->GetColor().GetColor() )
    {
        ViewShell* pSh = rInf.GetShell();
        if( pSh && pSh->GetWin() )
        {
            const StyleSettings& rS = pSh->GetWin()->GetSettings().GetStyleSettings();
            nNewColor = rInf.GetDarkBack()
                ? COL_WHITE : rS.GetWindowTextColor().GetColor();
        }
        else
            nNewColor = rInf.GetDarkBack() ? COL_WHITE : COL_BLACK;

        bChgColor = TRUE;
    }
    if( bChgColor )
    {
        Color aOldColor( pTmpFont->GetColor() );
        Color aBlack( nNewColor );
        pTmpFont->SetColor( aBlack );
        if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *pTmpFont );
        pTmpFont->SetColor( aOldColor );
    }
    else if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
        rInf.GetOut().SetFont( *pTmpFont );

    if ( STRING_LEN == rInf.GetLen() )
        rInf.SetLen( rInf.GetText().Len() );
    // "No screen adj"
    if ( bPrt || (
        rInf.GetShell()->GetDoc()->IsBrowseMode() &&
        !rInf.GetShell()->GetViewOptions()->IsPrtFormat() &&
        !rInf.GetBullet() && ( rInf.GetSpace() || !rInf.GetKern() ) &&
        !rInf.GetWrong() && !rInf.GetGreyWave() ) )
    {
        const Fraction aTmp( 1, 1 );
        BOOL bStretch = rInf.GetWidth() && ( rInf.GetLen() > 1 ) && bPrt
                        && ( aTmp != rInf.GetOut().GetMapMode().GetScaleX() );
        if( rInf.GetSpace() )
        // Hack, solange DrawStretchText auf Druckern nicht funktioniert:
        //  || bStretch || rInf.GetKern() )
        {
            long *pKernArray = new long[rInf.GetLen()];
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                         rInf.GetIdx(), rInf.GetLen() );
            if( bStretch )
            {
                xub_StrLen nZwi = rInf.GetLen() - 1;
                long nDiff = rInf.GetWidth() - pKernArray[ nZwi ]
                             - rInf.GetLen() * rInf.GetKern();
                long nRest = nDiff % nZwi;
                long nAdd;
                if( nRest < 0 )
                {
                    nAdd = -1;
                    nRest += nZwi;
                }
                else
                {
                    nAdd = +1;
                    nRest = nZwi - nRest;
                }
                nDiff /= nZwi;
                long nSum = nDiff;
                for( xub_StrLen i = 0; i < nZwi; )
                {
                    pKernArray[ i ] += nSum;
                    if( ++i == nRest )
                        nDiff += nAdd;
                    nSum += nDiff;
                }
            }
            long nKernSum = rInf.GetKern();

            if ( bStretch || bPaintBlank || rInf.GetKern() )
            {
                for( xub_StrLen i = 0; i < rInf.GetLen(); i++,
                     nKernSum += rInf.GetKern() )
                {
                    if ( CH_BLANK == rInf.GetText().GetChar(rInf.GetIdx()+i) )
                        nKernSum += rInf.GetSpace();
                    pKernArray[i] += nKernSum;
                }

                // Bei durch/unterstr. Blocksatz erfordert ein Blank am Ende
                // einer Textausgabe besondere Massnahmen:
#ifdef VERTICAL_LAYOUT
                if ( rInf.GetFrm() && rInf.GetFrm()->IsVertical() )
                    rInf.GetFrm()->SwitchHorizontalToVertical( aPos );
#endif

                if( bPaintBlank && rInf.GetLen() && ( CH_BLANK ==
                    rInf.GetText().GetChar( rInf.GetIdx()+rInf.GetLen()-1 ) ) )
                {
                    // Wenn es sich um ein singulaeres, unterstrichenes Space
                    // handelt, muessen wir zwei ausgeben:
                    if( 1 == rInf.GetLen() )
                    {
                        pKernArray[0] = rInf.GetSpace();
                        rInf.GetOut().DrawTextArray( aPos, XubString( sDoubleSpace,
                            RTL_TEXTENCODING_MS_1252 ), pKernArray, 0, 2 );
                    }
                    else
                    {
                        pKernArray[ rInf.GetLen() - 2 ] += rInf.GetSpace();
                        rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                            pKernArray, rInf.GetIdx(), rInf.GetLen() );
                    }
                }
                else
                    rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                        pKernArray, rInf.GetIdx(), rInf.GetLen() );
            }
            else
            {
                Point aTmpPos( aPos );
                xub_StrLen j = 0;
                xub_StrLen i;
                for( i = 0; i < rInf.GetLen(); i++ )
                {
                    if( CH_BLANK == rInf.GetText().GetChar( rInf.GetIdx()+i ) )
                    {
                        nKernSum += rInf.GetSpace();
                        if( j < i )
                            rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                                    rInf.GetIdx() + j, i - j );
                        j = i + 1;
                        aTmpPos.X() = aPos.X() + pKernArray[ i ] + nKernSum;
                    }
                }
                if( j < i )
                    rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                            rInf.GetIdx() + j, i - j );
            }
            delete[] pKernArray;
        }
        else if( bStretch )
        {
            USHORT nTmpWidth = rInf.GetWidth();
            if( rInf.GetKern() && rInf.GetLen() && nTmpWidth > rInf.GetKern() )
                nTmpWidth -= rInf.GetKern();
            rInf.GetOut().DrawStretchText( aPos, nTmpWidth,
                             rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
        }
        else if( rInf.GetKern() )
        {
            long nTmpWidth = GetTextSize( rInf ).Width();
            if( bChgColor && COL_AUTO == pTmpFont->GetColor().GetColor() )
            {
                Color aOldColor( pTmpFont->GetColor() );
                Color aBlack( nNewColor );
                pTmpFont->SetColor( aBlack );
                if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
                    rInf.GetOut().SetFont( *pTmpFont );
                pTmpFont->SetColor( aOldColor );
            }
            rInf.GetOut().DrawStretchText( aPos, (USHORT)nTmpWidth,
                               rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
        }
        else
            rInf.GetOut().DrawText( aPos, rInf.GetText(),
                                    rInf.GetIdx(), rInf.GetLen() );
    }
    else
    {
        const String* pStr = &rInf.GetText();
        String aStr( aEmptyStr );
        BOOL bBullet = rInf.GetBullet();
        if( bSymbol )
            bBullet = FALSE;
        long *pKernArray = new long[ rInf.GetLen() ];
        CheckScrFont( rInf.GetShell(), rInf.GetpOut() );
        long nScrPos;
        xub_Unicode cCh = rInf.GetText().GetChar( rInf.GetIdx() );
        rInf.GetOut().GetCharWidth( cCh, cCh, &nScrPos );
        USHORT nType;
        if( bCompress && ( SwScriptInfo::NONE !=
            ( nType = rInf.GetScriptInfo()->CompType( rInf.GetIdx() ) ) )
            && SwScriptInfo::SPECIAL_LEFT != nType )
        {
            long nTmp = nScrPos;
            nTmp *= rInf.GetKanaComp();
            if( SwScriptInfo::KANA != nType )
                nTmp *= 5;
            nTmp /= 100000;
            nScrPos -= nTmp;
        }

        if ( pPrinter )
        {
            if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
                pPrinter->SetFont( *pPrtFont );
            pPrinter->GetTextArray( rInf.GetText(), pKernArray, rInf.GetIdx(),
                                    rInf.GetLen() );
        }
        else
        {
            BOOL bRestore = FALSE;
            MapMode aOld( rInf.GetOut().GetMapMode() );
            if( rInf.GetZoom().GetNumerator() &&
                rInf.GetZoom() != aOld.GetScaleX() )
            {
                MapMode aNew( aOld );
                aNew.SetScaleX( rInf.GetZoom() );
                aNew.SetScaleY( rInf.GetZoom() );
                rInf.GetOut().SetMapMode( aNew );
                bRestore = TRUE;
            }
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                        rInf.GetIdx(), rInf.GetLen() );
            if( bRestore )
                rInf.GetOut().SetMapMode( aOld );
        }
        if( bCompress )
            rInf.GetScriptInfo()->Compress( pKernArray, rInf.GetIdx(),
                                    rInf.GetLen(), rInf.GetKanaComp(),
                                    (USHORT) aFont.GetSize().Height(), &aPos );

        if( bBullet )
        {
            aStr = rInf.GetText().Copy( rInf.GetIdx(), rInf.GetLen() );
            pStr = &aStr;
            for( xub_StrLen i = 0; i < aStr.Len(); ++i )
                if( CH_BLANK == aStr.GetChar( i ) )
                    aStr.SetChar( i, CH_BULLET );
        }

        xub_StrLen nCnt = rInf.GetText().Len();
        if ( nCnt < rInf.GetIdx() )
            nCnt = 0;
        else
            nCnt -= rInf.GetIdx();
        nCnt = Min( nCnt, rInf.GetLen() );
        long nKernSum = rInf.GetKern();
        xub_Unicode cChPrev = rInf.GetText().GetChar( rInf.GetIdx() );

        // Wenn es sich um ein singulaeres, unterstrichenes Space
        // im Blocksatz handelt, muessen wir zwei ausgeben:
        if ( ( nCnt == 1 ) && rInf.GetSpace() && ( cChPrev == CH_BLANK ) )
        {
            pKernArray[0] = rInf.GetSpace() + rInf.GetKern();
            rInf.GetOut().DrawTextArray( aPos, XubString( sDoubleSpace,
                RTL_TEXTENCODING_MS_1252 ), pKernArray, 0, 2 );
            if( bBullet )
                rInf.GetOut().DrawTextArray( aPos, *pStr, pKernArray,
                                              0, 1 );
        }
        else
        {
            xub_Unicode nCh;

            // Bei Pairkerning waechst der Printereinfluss auf die Positionierung
            USHORT nMul = 3;
            long *pScrArray = NULL;

            if ( pPrtFont->GetKerning() )
            {
                nMul = 1;
                if( KERNING_ASIAN == pPrtFont->GetKerning() )
                {
                    pScrArray = new long[ rInf.GetLen() ];
                    rInf.GetOut().GetTextArray( rInf.GetText(), pScrArray,
                                        rInf.GetIdx(), rInf.GetLen() );
                    nScrPos = pScrArray[0];
                }
            }

            const USHORT nDiv = nMul+1;
            // In nSpaceSum wird der durch Blocksatz auf die Spaces verteilte
            // Zwischenraum aufsummiert.
            // Die Spaces selbst werden im Normalfall in der Mitte des
            // Zwischenraums positioniert, deshalb die nSpace/2-Mimik.
            // Bei wortweiser Unterstreichung muessen sie am Anfang des
            // Zwischenraums stehen, damit dieser nicht unterstrichen wird.
            // Ein Space am Anfang oder am Ende des Textes muss allerdings
            // vor bzw. hinter den kompletten Zwischenraum gesetzt werden,
            // sonst wuerde das Durch-/Unterstreichen Luecken aufweisen.
            long nSpaceSum = 0;
            USHORT nHalfSpace = pPrtFont->IsWordLineMode() ? 0 : rInf.GetSpace()/2;
            USHORT nOtherHalf = rInf.GetSpace() - nHalfSpace;
            if ( rInf.GetSpace() && ( cChPrev == CH_BLANK ) )
                nSpaceSum = nHalfSpace;
            for ( xub_StrLen i=1; i<nCnt; ++i,nKernSum += rInf.GetKern() )
            {
                nCh = rInf.GetText().GetChar( rInf.GetIdx() + i );
                long nScr;
                if( pScrArray )
                    nScr = pScrArray[ i ] - pScrArray[ i - 1 ];
                else
                    rInf.GetOut().GetCharWidth( nCh, nCh, &nScr );
                if( bCompress && ( SwScriptInfo::NONE != ( nType =
                    rInf.GetScriptInfo()->CompType( rInf.GetIdx() + i ) ) ) )
                {
                    if( SwScriptInfo::SPECIAL_LEFT != nType )
                    {
                        long nTmp = nScr;
                        nTmp *= rInf.GetKanaComp();
                        if( SwScriptInfo::KANA != nType )
                            nTmp *= 5;
                        nTmp /= 100000;
                        nScr -= nTmp;
                    }
                    nScrPos = pKernArray[i-1] + nScr;
                    if ( cChPrev == CH_BLANK )
                        nSpaceSum += nOtherHalf;
                }
                // Wenn vor uns ein (Ex-)SPACE ist, positionieren wir uns optimal,
                // d.h. unseren rechten Rand auf die 100% Druckerposition,
                // sind wir sogar selbst ein Ex-SPACE, so positionieren wir uns
                // linksbuendig zur Druckerposition.
                else if ( nCh == CH_BLANK )
                {
                    nScrPos = pKernArray[i-1]+nScr;
                    if ( cChPrev == CH_BLANK )
                        nSpaceSum += nOtherHalf;
                    if ( i + 1 == nCnt )
                        nSpaceSum += rInf.GetSpace();
                    else
                        nSpaceSum += nHalfSpace;
                }
                else
                {
                    if ( cChPrev == CH_BLANK )
                    {
                        nScrPos = pKernArray[i-1] + nScr;
                        // kein Pixel geht verloren:
                        nSpaceSum += nOtherHalf;
                    }
                    else if ( cChPrev == '-' )
                        nScrPos = pKernArray[i-1] + nScr;
                    else
                    {
                        nScrPos += nScr;
                        nScrPos = ( nMul * nScrPos + pKernArray[i] ) / nDiv;
                    }
                }
                cChPrev = nCh;
                pKernArray[i-1] = nScrPos - nScr + nKernSum + nSpaceSum;
            }
            if( rInf.GetGreyWave() )
            {
                if( rInf.GetLen() )
                {
                    long nHght = rInf.GetOut().LogicToPixel(
                                    pPrtFont->GetSize() ).Height();
                    if( WRONG_SHOW_MIN < nHght )
                    {
                        pKernArray[ rInf.GetLen() - 1 ] += nKernSum + nSpaceSum;
                        if ( rInf.GetOut().GetConnectMetaFile() )
                            rInf.GetOut().Push();

                        USHORT nWave =
                            WRONG_SHOW_MEDIUM < nHght ? WAVE_NORMAL :
                            ( WRONG_SHOW_SMALL < nHght ? WAVE_SMALL :
                            WAVE_FLAT );
                        Color aCol( rInf.GetOut().GetLineColor() );
                        BOOL bColSave = aCol != *pWaveCol;
                        if ( bColSave )
                            rInf.GetOut().SetLineColor( *pWaveCol );

                        Point aEnd;
                        long nKernVal = pKernArray[ USHORT( rInf.GetLen() - 1 ) ];

                        switch ( GetFont()->GetOrientation() )
                        {
                        case 0 :
                            aEnd.X() = rInf.GetPos().X() + nKernVal;
                            aEnd.Y() = rInf.GetPos().Y();
                            break;
                        case 900 :
                            aEnd.X() = rInf.GetPos().X();
                            aEnd.Y() = rInf.GetPos().Y() - nKernVal;
                            break;
                        case 2700 :
                            aEnd.X() = rInf.GetPos().X();
                            aEnd.Y() = rInf.GetPos().Y() + nKernVal;
                            break;
                        }

                        rInf.GetOut().DrawWaveLine( rInf.GetPos(), aEnd, nWave );

                        if ( bColSave )
                            rInf.GetOut().SetLineColor( aCol );

                        if ( rInf.GetOut().GetConnectMetaFile() )
                            rInf.GetOut().Pop();
                    }
                }
            }
            else if( rInf.GetWrong() && !bSymbol )
            {
                if( rInf.GetLen() )
                {
                    xub_StrLen nStart = rInf.GetIdx();
                    xub_StrLen nWrLen = rInf.GetLen();
                    if( rInf.GetWrong()->Check( nStart, nWrLen ) )
                    {
                        long nHght = rInf.GetOut().LogicToPixel(
                                                pPrtFont->GetSize() ).Height();
                        if( WRONG_SHOW_MIN < nHght )
                        {
                            pKernArray[ rInf.GetLen() - 1 ] += nKernSum + nSpaceSum;
                            if ( rInf.GetOut().GetConnectMetaFile() )
                                rInf.GetOut().Push();

                            USHORT nWave =
                                WRONG_SHOW_MEDIUM < nHght ? WAVE_NORMAL :
                                ( WRONG_SHOW_SMALL < nHght ? WAVE_SMALL :
                                WAVE_FLAT );
                            Color aCol( rInf.GetOut().GetLineColor() );
                            BOOL bColSave = aCol != *pSpellCol;
                            if ( bColSave )
                                rInf.GetOut().SetLineColor( *pSpellCol );

                            do
                            {
                                nStart -= rInf.GetIdx();
                                Point aStart( rInf.GetPos() );
                                Point aEnd;
                                short nBlank = 0;
                                const xub_StrLen nEnd = nStart + nWrLen;

                                if( nEnd < nCnt
                                    && CH_BLANK == rInf.GetText().GetChar( rInf.GetIdx() + nEnd ) )
                                {
                                    if( nEnd + 1 == nCnt )
                                        nBlank -= rInf.GetSpace();
                                    else
                                        nBlank -= nHalfSpace;
                                }

                                // determine start, end and length of wave line
                                long nKernStart = nStart ?
                                                  pKernArray[ USHORT( nStart - 1 ) ] :
                                                  0;
                                long nKernEnd = pKernArray[ USHORT( nEnd - 1 ) ];

                                switch ( GetFont()->GetOrientation() )
                                {
                                case 0 :
                                    aStart.X() += nKernStart;
                                    aEnd.X() = nBlank + rInf.GetPos().X() + nKernEnd;
                                    aEnd.Y() = rInf.GetPos().Y();
                                    break;
                                case 900 :
                                    aStart.Y() -= nKernStart;
                                    aEnd.X() = rInf.GetPos().X();
                                    aEnd.Y() = nBlank + rInf.GetPos().Y() - nKernEnd;
                                    break;
                                case 2700 :
                                    aStart.Y() += nKernStart;
                                    aEnd.X() = rInf.GetPos().X();
                                    aEnd.Y() = nBlank + rInf.GetPos().Y() + nKernEnd;
                                    break;
                                }

                                rInf.GetOut().DrawWaveLine( aStart, aEnd, nWave );
                                nStart = nEnd + rInf.GetIdx();
                                nWrLen = rInf.GetIdx() + rInf.GetLen() - nStart;
                            }
                            while( nWrLen && rInf.GetWrong()->Check( nStart, nWrLen ) );

                            if ( bColSave )
                                rInf.GetOut().SetLineColor( aCol );

                            if ( rInf.GetOut().GetConnectMetaFile() )
                                rInf.GetOut().Pop();
                        }
                    }
                }
            }
            xub_StrLen nOffs = 0;
            xub_StrLen nLen = rInf.GetLen();
#ifdef COMING_SOON
            if( aPos.X() < rInf.GetLeft() )
            {
                while( nOffs < nLen &&
                    aPos.X() + pKernArray[ nOffs ] < rInf.GetLeft() )
                    ++nOffs;
                if( nOffs < nLen )
                {
                    --nLen;
                    while( nLen > nOffs &&
                        aPos.X() + pKernArray[ nLen ] > rInf.GetRight() )
                        --nLen;
                    ++nLen;
                    if( nOffs )
                        --nOffs;
                }
                if( nOffs )
                {
                    long nDiff = pKernArray[ nOffs - 1 ];
                    aPos.X() += nDiff;
                    for( xub_StrLen nX = nOffs; nX < nLen; ++nX )
                        pKernArray[ nX ] -= nDiff;
                }
            }
#endif
            if( nOffs < nLen )
            {
                register xub_StrLen nTmpIdx = bBullet ? 0 : rInf.GetIdx();

#ifdef VERTICAL_LAYOUT
                if ( rInf.GetFrm() && rInf.GetFrm()->IsVertical() )
                    rInf.GetFrm()->SwitchHorizontalToVertical( aPos );
#endif

                rInf.GetOut().DrawTextArray( aPos, *pStr, pKernArray + nOffs,
                                    nTmpIdx + nOffs , nLen - nOffs );
            }
            delete[] pScrArray;
        }
        delete[] pKernArray;
    }
}

// Optimierung war fuer DrawText() ausgeschaltet
#if defined( WNT ) && defined( MSC )    // && defined( W40 )
#pragma optimize("",on)
#endif


/*************************************************************************
 *
 *  Size SwFntObj::GetTextSize( const OutputDevice *pOut, const String &rTxt,
 *           const USHORT nIdx, const USHORT nLen, const short nKern = 0 );
 *
 *  Ersterstellung      AMA 16. Dez. 94
 *  Letzte Aenderung    AMA 16. Dez. 94
 *
 *  Beschreibung: ermittelt die TextSize (des Druckers)
 *
 *************************************************************************/

Size SwFntObj::GetTextSize( SwDrawTextInfo& rInf )
{
    Size aTxtSize;
    const xub_StrLen nLn = ( STRING_LEN != rInf.GetLen() ) ? rInf.GetLen() :
                           rInf.GetText().Len();
    BOOL bCompress = rInf.GetKanaComp() && nLn;
    ASSERT( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );
    if ( OUTDEV_PRINTER != rInf.GetpOut()->GetOutDevType() && pPrinter )
    {
        if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
            pPrinter->SetFont(*pPrtFont);
        aTxtSize.Width() = pPrinter->GetTextWidth( rInf.GetText(),
                                                rInf.GetIdx(), nLn );
        aTxtSize.Height() = pPrinter->GetTextHeight();
        long *pKernArray = new long[nLn];
        CheckScrFont( rInf.GetShell(), rInf.GetpOut() );
        if( !GetScrFont()->IsSameInstance( rInf.GetpOut()->GetFont() ) )
            rInf.GetpOut()->SetFont( *pScrFont );
        long nScrPos;

        pPrinter->GetTextArray( rInf.GetText(), pKernArray, rInf.GetIdx(),nLn );
        if( bCompress )
            rInf.SetKanaDiff( rInf.GetScriptInfo()->Compress( pKernArray,
                rInf.GetIdx(), nLn, rInf.GetKanaComp(),
                (USHORT)aFont.GetSize().Height() ) );
        else
            rInf.SetKanaDiff( 0 );

        if ( rInf.GetKanaDiff() )
            nScrPos = pKernArray[ nLn - 1 ];
        else
        {
            xub_Unicode cCh = rInf.GetText().GetChar( rInf.GetIdx() );
            rInf.GetpOut()->GetCharWidth( cCh, cCh, &nScrPos );
            xub_StrLen nCnt = rInf.GetText().Len();
            if ( nCnt < rInf.GetIdx() )
                nCnt=0;
            else
                nCnt -= rInf.GetIdx();
            nCnt = Min (nCnt, nLn);
            xub_Unicode nChPrev = rInf.GetText().GetChar( rInf.GetIdx() );

            xub_Unicode nCh;

            // Bei Pairkerning waechst der Printereinfluss auf die Positionierung
            USHORT nMul = 3;
            if ( pPrtFont->GetKerning() )
                nMul = 1;
            const USHORT nDiv = nMul+1;
            for( xub_StrLen i=1; i<nCnt; i++ )
            {
                nCh = rInf.GetText().GetChar( rInf.GetIdx() + i );
                long nScr;
                rInf.GetpOut()->GetCharWidth( nCh, nCh, &nScr );

                if ( nCh == CH_BLANK )
                    nScrPos = pKernArray[i-1]+nScr;
                else
                {
                    if ( nChPrev == CH_BLANK || nChPrev == '-' )
                        nScrPos = pKernArray[i-1]+nScr;
                    else
                    {
                        nScrPos += nScr;
                        nScrPos = ( nMul * nScrPos + pKernArray[i] ) / nDiv;
                    }
                }
                nChPrev = nCh;
                pKernArray[i-1] = nScrPos - nScr;
            }
        }

        delete[] pKernArray;
        aTxtSize.Width() = nScrPos;
    }
    else
    {
        if( !pPrtFont->IsSameInstance( rInf.GetpOut()->GetFont() ) )
            rInf.GetpOut()->SetFont( *pPrtFont );
        if( bCompress )
        {
            long *pKernArray = new long[nLn];
            rInf.GetpOut()->GetTextArray( rInf.GetText(), pKernArray,
                    rInf.GetIdx(), nLn );
            rInf.SetKanaDiff( rInf.GetScriptInfo()->Compress( pKernArray,
                rInf.GetIdx(), nLn, rInf.GetKanaComp(),
                (USHORT) aFont.GetSize().Height() ) );
            aTxtSize.Width() = pKernArray[ nLn - 1 ];
            delete[] pKernArray;
        }
        else
        {
            aTxtSize.Width() = rInf.GetpOut()->GetTextWidth( rInf.GetText(),
                                                           rInf.GetIdx(), nLn );
            rInf.SetKanaDiff( 0 );
        }
        aTxtSize.Height() = rInf.GetpOut()->GetTextHeight();
    }
    if ( rInf.GetKern() && nLn )
        aTxtSize.Width() += ( nLn - 1 ) * long( rInf.GetKern() );
    aTxtSize.Height() += nLeading;
    return aTxtSize;
}

xub_StrLen SwFntObj::GetCrsrOfst( SwDrawTextInfo &rInf )
{
    short nSpaceAdd = rInf.GetSpace() ? rInf.GetSpace() : - rInf.GetSperren();
    short nKern = rInf.GetKern();

    if( nSpaceAdd < 0 )
    {
        nKern -= nSpaceAdd;
        nSpaceAdd = 0;
    }
    long *pKernArray = new long[ rInf.GetLen() ];

    if ( pPrinter )
        pPrinter->GetTextArray( rInf.GetText(), pKernArray,
                                rInf.GetIdx(), rInf.GetLen() );
    else
        rInf.GetpOut()->GetTextArray( rInf.GetText(), pKernArray,
                            rInf.GetIdx(), rInf.GetLen() );

    if( rInf.GetScriptInfo() && rInf.GetKanaComp() && rInf.GetLen() )
        rInf.GetScriptInfo()->Compress( pKernArray, rInf.GetIdx(),rInf.GetLen(),
                        rInf.GetKanaComp(), (USHORT) aFont.GetSize().Height() );

    long nLeft = 0;
    long nRight = 0;
    xub_StrLen nCnt = 0;
    xub_StrLen nSpaceSum = 0;
    long nKernSum = 0;

    while ( ( nRight < long( rInf.GetOfst() ) ) && ( nCnt < rInf.GetLen() ) )
    {
        nLeft = nRight;
        if ( nSpaceAdd &&
            ( rInf.GetText().GetChar( nCnt + rInf.GetIdx() ) == CH_BLANK ) )
            nSpaceSum += nSpaceAdd;
        nRight = pKernArray[ nCnt++ ] + nKernSum + nSpaceSum;
        nKernSum += nKern;
    }
    if ( nCnt && ( nRight > long( rInf.GetOfst() ) ) &&
                 ( nRight - rInf.GetOfst() > rInf.GetOfst() - nLeft ) )
      --nCnt;

    delete[] pKernArray;
    return nCnt;
}

/*************************************************************************
|*
|*  SwFntAccess::SwFntAccess()
|*
|*  Ersterstellung      AMA 9. Nov. 94
|*  Letzte Aenderung    AMA 9. Nov. 94
|*
|*************************************************************************/

SwFntAccess::SwFntAccess( const void* &rMagic,
                USHORT &rIndex, const void *pOwner, ViewShell *pSh,
                BOOL bCheck ) :
  SwCacheAccess( *pFntCache, rMagic, rIndex ),
  pShell( pSh )
{
    // Der benutzte CTor von SwCacheAccess sucht anhand rMagic+rIndex im Cache
    if ( IsAvail() )
    {
        // Der schnellste Fall: ein bekannter Font ( rMagic ),
        // bei dem Drucker und Zoom nicht ueberprueft werden brauchen.
        if ( !bCheck )
            return;

        // Hier ist zwar der Font bekannt, muss aber noch ueberprueft werden.

    }
    else
        // Hier ist der Font nicht bekannt, muss also gesucht werden.
        bCheck = FALSE;

    {
        // Erstmal den Drucker besorgen
        Printer *pOut = 0;
        USHORT nZoom = USHRT_MAX;
        if ( pSh )
        {
            // "No screen adj"
            if ( !pSh->GetDoc()->IsBrowseMode() ||
                  pSh->GetViewOptions()->IsPrtFormat() )
            {
                pOut = (Printer*)( pSh->GetDoc()->GetPrt() );
                if ( pOut && !pOut->IsValid() )
                    pOut = 0;
            }
            nZoom = pSh->GetViewOptions()->GetZoom();
        }

        SwFntObj *pFntObj;
        if ( bCheck )
        {
            pFntObj = Get( );
            if ( ( pFntObj->GetZoom( ) == nZoom ) &&
                 ( pFntObj->pPrinter == pOut ) &&
                   pFntObj->GetPropWidth() ==
                           ((SwSubFont*)pOwner)->GetPropWidth() )
                return; // Die Ueberpruefung ergab: Drucker+Zoom okay.
            pFntObj->Unlock( ); // Vergiss dies Objekt, es wurde leider
            pObj = NULL;        // eine Drucker/Zoomaenderung festgestellt.
        }

        //Jetzt muss ueber Font-Vergleiche gesucht werden, relativ teuer!
        pFntObj = pFntCache->First();
        // Suchen nach gleichem Font und gleichem Drucker
        while ( pFntObj && !( pFntObj->aFont == *(Font *)pOwner &&
                  pFntObj->GetZoom() == nZoom &&
                  pFntObj->GetPropWidth() ==
                           ((SwSubFont*)pOwner)->GetPropWidth() &&
                ( !pFntObj->pPrinter || pFntObj->pPrinter == pOut ) ) )
            pFntObj = pFntCache->Next( pFntObj );
        if( pFntObj && pFntObj->pPrinter != pOut )
        {
            // Wir haben zwar einen ohne Drucker gefunden, mal sehen, ob es
            // auch noch einen mit identischem Drucker gibt.
            SwFntObj *pTmpObj = pFntObj;
            while( pTmpObj && !( pTmpObj->aFont == *(Font *)pOwner &&
                   pTmpObj->GetZoom()==nZoom && pTmpObj->pPrinter==pOut &&
                   pTmpObj->GetPropWidth() ==
                           ((SwSubFont*)pOwner)->GetPropWidth() ) )
                pTmpObj = pFntCache->Next( pTmpObj );
            if( pTmpObj )
                pFntObj = pTmpObj;
        }

        if ( !pFntObj ) // nichts gefunden, also anlegen
        {
            // Das Objekt muss neu angelegt werden, deshalb muss der Owner ein
            // SwFont sein, spaeter wird als Owner die "MagicNumber" gehalten.
            SwCacheAccess::pOwner = pOwner;
            pFntObj = Get( ); // hier wird via NewObj() angelegt und gelockt.
            ASSERT(pFntObj, "No Font, no Fun.");
        }
        else  // gefunden, also locken
        {
            pFntObj->Lock();
            if( pFntObj->pPrinter != pOut ) // Falls bis dato kein Drucker bekannt
            {
                ASSERT( !pFntObj->pPrinter, "SwFntAccess: Printer Changed" );
                pFntObj->CheckPrtFont( pOut );
                pFntObj->pPrinter = pOut;
                pFntObj->pScrFont = NULL;
                pFntObj->nLeading = USHRT_MAX;
                pFntObj->nPrtAscent = USHRT_MAX;
                pFntObj->nPrtHeight = USHRT_MAX;
            }
            pObj = pFntObj;
        }
        // egal, ob neu oder gefunden, ab jetzt ist der Owner vom Objekt eine
        // MagicNumber und wird auch dem aufrufenden SwFont bekanntgegeben,
        // ebenso der Index fuer spaetere direkte Zugriffe
        rMagic = pFntObj->GetOwner();
        SwCacheAccess::pOwner = rMagic;
        rIndex = pFntObj->GetCachePos();
    }
}

SwCacheObj *SwFntAccess::NewObj( )
{
    // Ein neuer Font, eine neue "MagicNumber".
    return new SwFntObj( *(SwSubFont *)pOwner, ++pMagicNo, pShell );
}

xub_StrLen SwFont::GetTxtBreak( SwDrawTextInfo& rInf, long nTextWidth )
{
    BOOL bCompress = SW_CJK==GetActual() && rInf.GetKanaComp() && rInf.GetLen();
    ASSERT( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

     ChgFnt( rInf.GetShell(), rInf.GetpOut() );

    USHORT nTxtBreak = 0;
    long nKern = 0;

    USHORT nLn = ( rInf.GetLen() == STRING_LEN ? rInf.GetText().Len()
                                               : rInf.GetLen() );
    if( aSub[nActual].IsCapital() && nLn )
        nTxtBreak = GetCapitalBreak( rInf.GetShell(), rInf.GetpOut(),
        rInf.GetScriptInfo(), rInf.GetText(), nTextWidth,0, rInf.GetIdx(),nLn );
    else
    {
        nKern = CheckKerning();
        if( rInf.GetHyphPos() )
        {
            if ( !aSub[nActual].IsCaseMap() )
                nTxtBreak = rInf.GetpOut()->GetTextBreak( rInf.GetText(),
                    nTextWidth,'-',*rInf.GetHyphPos(),rInf.GetIdx(),nLn,nKern );
            else
                nTxtBreak = rInf.GetpOut()->GetTextBreak( aSub[nActual].
                            CalcCaseMap( rInf.GetText() ), nTextWidth, '-',
                            *rInf.GetHyphPos(), rInf.GetIdx(), nLn, nKern );
        }
        else
        {
            if ( !aSub[nActual].IsCaseMap() )
                nTxtBreak = rInf.GetpOut()->GetTextBreak( rInf.GetText(),
                            nTextWidth, rInf.GetIdx(), nLn, nKern );
            else
                nTxtBreak = rInf.GetpOut()->GetTextBreak( aSub[nActual].
                            CalcCaseMap( rInf.GetText() ), nTextWidth,
                            rInf.GetIdx(), nLn, nKern );
        }
    }

    if ( ! bCompress )
        return nTxtBreak;

    nTxtBreak -= rInf.GetIdx();

    if( nTxtBreak < nLn )
    {
        if( !nTxtBreak && nLn )
            nLn = 1;
        else if( nLn > 2 * nTxtBreak )
            nLn = 2 * nTxtBreak;
        long *pKernArray = new long[ nLn ];
        rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                    rInf.GetIdx(), nLn );
        if( rInf.GetScriptInfo()->Compress( pKernArray, rInf.GetIdx(), nLn,
                            rInf.GetKanaComp(), (USHORT)GetHeight( nActual ) ) )
        {
            long nKernAdd = nKern;
            xub_StrLen nTmpBreak = nTxtBreak;
            if( nKern && nTxtBreak )
                nKern *= nTxtBreak - 1;
            while( nTxtBreak<nLn && nTextWidth >= pKernArray[nTxtBreak] +nKern )
            {
                nKern += nKernAdd;
                ++nTxtBreak;
            }
            if( rInf.GetHyphPos() )
                *rInf.GetHyphPos() += nTxtBreak - nTmpBreak; // It's not perfect
        }
        delete[] pKernArray;
    }
    nTxtBreak += rInf.GetIdx();

    return nTxtBreak;
}

// used during painting of small capitals
void SwDrawTextInfo::Shift( USHORT nDir )
{
    ASSERT( bPos, "DrawTextInfo: Undefined Position" );
    ASSERT( bSize, "DrawTextInfo: Undefined Width" );

    switch ( nDir )
    {
    case 0 :
        ((Point*)pPos)->X() += GetSize().Width();
        break;
    case 900 :
        ASSERT( ((Point*)pPos)->Y() >= GetSize().Width(), "Going underground" );
        ((Point*)pPos)->Y() -= GetSize().Width();
        break;
    case 2700 :
        ((Point*)pPos)->Y() += GetSize().Width();
        break;
    }
}
