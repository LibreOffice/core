/*************************************************************************
 *
 *  $RCSfile: impedit3.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-02 16:31:50 $
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

#include <eeng_pch.hxx>

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#pragma hdrstop

#include <adjitem.hxx>
#include <tstpitem.hxx>
#include <lspcitem.hxx>

#ifdef DBG_UTIL
#include <flditem.hxx>
#endif

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <txtrange.hxx>
#include <cscoitem.hxx>
#include <colritem.hxx>
#include <fhgtitem.hxx>
#include <kernitem.hxx>
#include <lrspitem.hxx>
#include <ulspitem.hxx>
#include <fontitem.hxx>
#include <wghtitem.hxx>
#include <postitem.hxx>
#include <langitem.hxx>
#include <scriptspaceitem.hxx>
#include <charscaleitem.hxx>

#include <unotools/localedatawrapper.hxx>

#include <textconv.hxx>

#include <math.h>

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#include <comphelper/processfactory.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

SV_DECL_VARARR_SORT( SortedPositions, sal_uInt32, 16, 8 );
SV_IMPL_VARARR_SORT( SortedPositions, sal_uInt32 );

#define RESDIFF     10
#define SCRLRANGE   20      // 1/20 der Breite/Hoehe scrollen, wenn im QueryDrop

#define CH_HYPH     '-'

#define WRONG_SHOW_MIN       5
#define WRONG_SHOW_SMALL    11
#define WRONG_SHOW_MEDIUM   15

struct TabInfo
{
    BOOL        bValid;

    SvxTabStop  aTabStop;
    xub_StrLen  nCharPos;
    USHORT      nTabPortion;
    long        nStartPosX;
    long        nTabPos;

    TabInfo::TabInfo() { bValid = FALSE; }
};

Point Rotate( const Point& rPoint, short nOrientation, const Point& rOrigin )
{
    double nRealOrientation = nOrientation*F_PI1800;
    double nCos = cos( nRealOrientation );
    double nSin = sin( nRealOrientation );

    Point aRotatedPos;
    Point aTranslatedPos( rPoint );

    // Translation
    aTranslatedPos -= rOrigin;

    // Rotation...
    aRotatedPos.X() = (long)   ( nCos*aTranslatedPos.X() + nSin*aTranslatedPos.Y() );
    aRotatedPos.Y() = (long) - ( nSin*aTranslatedPos.X() - nCos*aTranslatedPos.Y() );
    aTranslatedPos = aRotatedPos;

    // Translation...
    aTranslatedPos += rOrigin;
    return aTranslatedPos;
}

void lcl_DrawRedLines( OutputDevice* pOutDev, long nFontHeight, const Point& rPnt, sal_uInt16 nIndex, sal_uInt16 nMaxEnd, const long* pDXArray, WrongList* pWrongs, short nOrientation, const Point& rOrigin )
{
#ifndef SVX_LIGHT
    // Aber nur, wenn Font nicht zu klein...
    long nHght = pOutDev->LogicToPixel( Size( 0, nFontHeight ) ).Height();
    if( WRONG_SHOW_MIN < nHght )
    {
        sal_uInt16 nStyle;
        if( WRONG_SHOW_MEDIUM < nHght )
            nStyle = WAVE_NORMAL;
        else if( WRONG_SHOW_SMALL < nHght )
            nStyle = WAVE_SMALL;
        else
            nStyle = WAVE_FLAT;

        sal_uInt16 nEnd, nStart = nIndex;
        sal_Bool bWrong = pWrongs->NextWrong( nStart, nEnd );
        while ( bWrong )
        {
            if ( nStart >= nMaxEnd )
                break;

            if ( nStart < nIndex )  // Wurde korrigiert
                nStart = nIndex;
            if ( nEnd > nMaxEnd )
                nEnd = nMaxEnd;
            Point aPnt1( rPnt );
            if ( nStart > nIndex )
                aPnt1.X() += pDXArray[ nStart - nIndex - 1 ];
            Point aPnt2( rPnt );
            DBG_ASSERT( nEnd > nIndex, "RedLine: aPnt2?" );
            aPnt2.X() += pDXArray[ nEnd - nIndex - 1 ];
            if ( nOrientation )
            {
                aPnt1 = Rotate( aPnt1, nOrientation, rOrigin );
                aPnt2 = Rotate( aPnt2, nOrientation, rOrigin );
            }

            Color aOldColor( pOutDev->GetLineColor() );
            pOutDev->SetLineColor( COL_LIGHTRED );
            pOutDev->DrawWaveLine( aPnt1, aPnt2, nStyle );
            pOutDev->SetLineColor( aOldColor );

            nStart = nEnd+1;
            if ( nEnd < nMaxEnd )
                bWrong = pWrongs->NextWrong( nStart, nEnd );
            else
                bWrong = sal_False;
        }
    }
#endif // !SVX_LIGHT
}

Point lcl_ImplCalcRotatedPos( Point rPos, Point rOrigin, double nSin, double nCos )
{
    Point aRotatedPos;
    // Translation...
    Point aTranslatedPos( rPos);
    aTranslatedPos -= rOrigin;

    aRotatedPos.X() = (long)   ( nCos*aTranslatedPos.X() + nSin*aTranslatedPos.Y() );
    aRotatedPos.Y() = (long) - ( nSin*aTranslatedPos.X() - nCos*aTranslatedPos.Y() );
    aTranslatedPos = aRotatedPos;
    // Translation...
    aTranslatedPos += rOrigin;

    return aTranslatedPos;
}




// ----------------------------------------------------------------------
//  class ImpEditEngine
//  ----------------------------------------------------------------------
void ImpEditEngine::UpdateViews( EditView* pCurView )
{
    if ( !GetUpdateMode() || IsFormatting() || aInvalidRec.IsEmpty() )
        return;

    DBG_ASSERT( IsFormatted(), "UpdateViews: Doc nicht formatiert!" );

    for ( sal_uInt16 nView = 0; nView < aEditViews.Count(); nView++ )
    {
        EditView* pView = aEditViews[nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->HideCursor();

        Rectangle aClipRec( aInvalidRec );
        Rectangle aVisArea( pView->GetVisArea() );
        aClipRec.Intersection( aVisArea );

        if ( !aClipRec.IsEmpty() )
        {
            // in Fensterkoordinaten umwandeln....
            aClipRec = pView->pImpEditView->GetWindowPos( aClipRec );

            if ( ( pView == pCurView )  )
                Paint( pView->pImpEditView, aClipRec, sal_True );
            else
                pView->GetWindow()->Invalidate( aClipRec );
        }
    }

    if ( pCurView )
    {
        sal_Bool bGotoCursor = pCurView->pImpEditView->DoAutoScroll();
        pCurView->ShowCursor( bGotoCursor );
    }

    aInvalidRec = Rectangle();
    CallStatusHdl();
}

IMPL_LINK( ImpEditEngine, OnlineSpellHdl, Timer *, EMPTYARG )
{
    if ( !Application::AnyInput( INPUT_KEYBOARD ) && GetUpdateMode() && IsFormatted() )
        DoOnlineSpelling();
    else
        aOnlineSpellTimer.Start();

    return 0;
}

IMPL_LINK_INLINE_START( ImpEditEngine, IdleFormatHdl, Timer *, EMPTYARG )
{
    aIdleFormatter.ResetRestarts();
    FormatAndUpdate( aIdleFormatter.GetView() );
    return 0;
}
IMPL_LINK_INLINE_END( ImpEditEngine, IdleFormatHdl, Timer *, EMPTYARG )

void ImpEditEngine::CheckIdleFormatter()
{
    aIdleFormatter.ForceTimeout();
    // Falls kein Idle, aber trotzdem nicht formatiert:
    if ( !IsFormatted() )
        FormatDoc();
}

void ImpEditEngine::FormatFullDoc()
{
    for ( sal_uInt16 nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
        GetParaPortions()[nPortion]->MarkSelectionInvalid( 0, GetParaPortions()[nPortion]->GetNode()->Len() );
    FormatDoc();
}

void ImpEditEngine::FormatDoc()
{
    if ( !GetUpdateMode() || IsFormatting() )
        return;

    bIsFormatting = sal_True;

    // Dann kann ich auch den Spell-Timer starten...
    if ( GetStatus().DoOnlineSpelling() )
        StartOnlineSpellTimer();

    long nY = 0;
    sal_Bool bGrow = sal_False;

    Font aOldFont( GetRefDevice()->GetFont() );

    // Hier schon, damit nicht jedesmal in CreateLines...
    sal_Bool bMapChanged = ImpCheckRefMapMode();

    aInvalidRec = Rectangle();  // leermachen
    for ( sal_uInt16 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
    {
        ParaPortion* pParaPortion = GetParaPortions().GetObject( nPara );
        if ( pParaPortion->MustRepaint() || ( pParaPortion->IsInvalid() && pParaPortion->IsVisible() ) )
        {
            if ( pParaPortion->IsInvalid() )
            {
                sal_Bool bChangedByDerivedClass = GetEditEnginePtr()->FormattingParagraph( nPara );
                if ( bChangedByDerivedClass )
                {
                    pParaPortion->GetTextPortions().Reset();
                    pParaPortion->MarkSelectionInvalid( 0, pParaPortion->GetNode()->Len() );
                }
            }
            // bei MustRepaint() sollte keine Formatierung noetig sein!
            // 23.1.95: Evtl. ist sie durch eine andere Aktion aber doch
            // ungueltig geworden!
//          if ( pParaPortion->MustRepaint() || CreateLines( nPara ) )
            if ( ( pParaPortion->MustRepaint() && !pParaPortion->IsInvalid() )
                    || CreateLines( nPara, nY ) )
            {
                if ( !bGrow && GetTextRanger() )
                {
                    // Bei einer Aenderung der Hoehe muss alles weiter unten
                    // neu formatiert werden...
                    for ( sal_uInt16 n = nPara+1; n < GetParaPortions().Count(); n++ )
                    {
                        ParaPortion* pPP = GetParaPortions().GetObject( n );
                        pPP->MarkSelectionInvalid( 0, pPP->GetNode()->Len() );
                        pPP->GetLines().Reset();
                    }
                }
                bGrow = sal_True;
                if ( IsCallParaInsertedOrDeleted() )
                    GetEditEnginePtr()->ParagraphHeightChanged( nPara );
                pParaPortion->SetMustRepaint( sal_False );
            }

            // InvalidRec nur einmal setzen...
            if ( aInvalidRec.IsEmpty() )
            {
                // Bei Paperwidth 0 (AutoPageSize) bleibt es sonst Empty()...
                long nWidth = Max( (long)1, ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() ) );
                Range aInvRange( GetInvalidYOffsets( pParaPortion ) );
                aInvalidRec = Rectangle( Point( 0, nY+aInvRange.Min() ),
                    Size( nWidth, aInvRange.Len() ) );
            }
            else
            {
                aInvalidRec.Bottom() = nY + pParaPortion->GetHeight();
            }
        }
        else if ( bGrow )
        {
            aInvalidRec.Bottom() = nY + pParaPortion->GetHeight();
        }
        nY += pParaPortion->GetHeight();
    }

    // Man kann auch durch UpdateMode An=>AUS=>AN in die Formatierung gelangen...
    // Optimierung erst nach Vobis-Auslieferung aktivieren...
//  if ( !aInvalidRec.IsEmpty() )
    {
        sal_uInt32 nNewHeight = CalcTextHeight();
        long nDiff = nNewHeight - nCurTextHeight;
        if ( nDiff )
            aStatus.GetStatusWord() |= !IsVertical() ? EE_STAT_TEXTHEIGHTCHANGED : EE_STAT_TEXTWIDTHCHANGED;
        if ( nNewHeight < nCurTextHeight )
        {
            aInvalidRec.Bottom() = (long)Max( nNewHeight, nCurTextHeight );
            if ( aInvalidRec.IsEmpty() )
            {
                aInvalidRec.Top() = 0;
                // Left und Right werden nicht ausgewertet, aber wegen IsEmpty gesetzt.
                aInvalidRec.Left() = 0;
                aInvalidRec.Right() = !IsVertical() ? aPaperSize.Width() : aPaperSize.Height();
            }
        }

        nCurTextHeight = nNewHeight;

        if ( aStatus.AutoPageSize() )
            CheckAutoPageSize();
        else if ( nDiff )
        {
            for ( sal_uInt16 nView = 0; nView < aEditViews.Count(); nView++ )
            {
                EditView* pView = aEditViews[nView];
                ImpEditView* pImpView = pView->pImpEditView;
                if ( pImpView->DoAutoHeight() )
                {
                    Size aSz( pImpView->GetOutputArea().GetWidth(), nCurTextHeight );
                    if ( aSz.Height() > aMaxAutoPaperSize.Height() )
                        aSz.Height() = aMaxAutoPaperSize.Height();
                    else if ( aSz.Height() < aMinAutoPaperSize.Height() )
                        aSz.Height() = aMinAutoPaperSize.Height();
                    pImpView->ResetOutputArea( Rectangle(
                        pImpView->GetOutputArea().TopLeft(), aSz ) );
                }
            }
        }
    }

    if ( aStatus.DoRestoreFont() )
        GetRefDevice()->SetFont( aOldFont );
    bIsFormatting = sal_False;
    bFormatted = sal_True;

    if ( bMapChanged )
        GetRefDevice()->Pop();

    CallStatusHdl();    // Falls Modified...
}

sal_Bool ImpEditEngine::ImpCheckRefMapMode()
{
    sal_Bool bChange = sal_False;

    if ( aStatus.DoFormat100() )
    {
        MapMode aMapMode( GetRefDevice()->GetMapMode() );
        if ( aMapMode.GetScaleX().GetNumerator() != aMapMode.GetScaleX().GetDenominator() )
            bChange = sal_True;
        else if ( aMapMode.GetScaleY().GetNumerator() != aMapMode.GetScaleY().GetDenominator() )
            bChange = sal_True;

        if ( bChange )
        {
            Fraction Scale1( 1, 1 );
            aMapMode.SetScaleX( Scale1 );
            aMapMode.SetScaleY( Scale1 );
            GetRefDevice()->Push();
            GetRefDevice()->SetMapMode( aMapMode );
        }
    }

    return bChange;
}

void ImpEditEngine::CheckAutoPageSize()
{
    Size aPrevPaperSize( GetPaperSize() );
    if ( GetStatus().AutoPageWidth() )
        aPaperSize.Width() = (long) !IsVertical() ? CalcTextWidth() : GetTextHeight();
    if ( GetStatus().AutoPageHeight() )
        aPaperSize.Height() = (long) !IsVertical() ? GetTextHeight() : CalcTextWidth();

    SetValidPaperSize( aPaperSize );    //Min, Max beruecksichtigen

    if ( aPaperSize != aPrevPaperSize )
    {
        if ( ( !IsVertical() && ( aPaperSize.Width() != aPrevPaperSize.Width() ) )
             || ( IsVertical() && ( aPaperSize.Height() != aPrevPaperSize.Height() ) ) )
        {
            // Falls davor zentriert/rechts oder Tabs...
            aStatus.GetStatusWord() |= !IsVertical() ? EE_STAT_TEXTWIDTHCHANGED : EE_STAT_TEXTHEIGHTCHANGED;
            for ( sal_uInt16 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
            {
                // Es brauchen nur Absaetze neu formatiert werden,
                // die nicht linksbuendig sind.
                // Die Hoehe kann sich hier nicht mehr aendern.
                ParaPortion* pParaPortion = GetParaPortions().GetObject( nPara );
                ContentNode* pNode = pParaPortion->GetNode();
                SvxAdjust eJustification = SVX_ADJUST_LEFT;
                if ( !aStatus.IsOutliner() )
                    eJustification = ((const SvxAdjustItem&)pNode->GetContentAttribs().GetItem( EE_PARA_JUST)).GetAdjust();
                if ( eJustification != SVX_ADJUST_LEFT )
                {
                    pParaPortion->MarkSelectionInvalid( 0, pNode->Len() );
                    CreateLines( nPara, 0 );    // 0: Bei AutoPageSize kein TextRange!
                }
            }
        }

        Size aInvSize = aPaperSize;
        if ( aPaperSize.Width() < aPrevPaperSize.Width() )
            aInvSize.Width() = aPrevPaperSize.Width();
        if ( aPaperSize.Height() < aPrevPaperSize.Height() )
            aInvSize.Height() = aPrevPaperSize.Height();

        Size aSz( aInvSize );
        if ( IsVertical() )
        {
            aSz.Width() = aInvSize.Height();
            aSz.Height() = aInvSize.Width();
        }
        aInvalidRec = Rectangle( Point(), aSz );


        for ( sal_uInt16 nView = 0; nView < aEditViews.Count(); nView++ )
        {
            EditView* pView = aEditViews[nView];
            pView->pImpEditView->RecalcOutputArea();
        }
    }
}

sal_Bool ImpEditEngine::CreateLines( USHORT nPara, sal_uInt32 nStartPosY )
{
    ParaPortion* pParaPortion = GetParaPortions().GetObject( nPara );

    // sal_Bool: Aenderung der Hoehe des Absatzes Ja/Nein - sal_True/sal_False
    DBG_ASSERT( pParaPortion->GetNode(), "Portion ohne Node in CreateLines" );
    DBG_ASSERT( pParaPortion->IsVisible(), "Unsichtbare Absaetze nicht formatieren!" );
    DBG_ASSERT( pParaPortion->IsInvalid(), "CreateLines: Portion nicht invalid!" );

    BOOL bProcessingEmptyLine = ( pParaPortion->GetNode()->Len() == 0 );
    BOOL bEmptyNodeWithPolygon = ( pParaPortion->GetNode()->Len() == 0 ) && GetTextRanger();

    // ---------------------------------------------------------------
    // Schnelle Sonderbehandlung fuer leere Absaetze...
    // ---------------------------------------------------------------
    if ( ( pParaPortion->GetNode()->Len() == 0 ) && !GetTextRanger() )
    {
        // schnelle Sonderbehandlung...
        if ( pParaPortion->GetTextPortions().Count() )
            pParaPortion->GetTextPortions().Reset();
        if ( pParaPortion->GetLines().Count() )
            pParaPortion->GetLines().Reset();
        CreateAndInsertEmptyLine( pParaPortion, nStartPosY );
        return FinishCreateLines( pParaPortion );
    }

    // ---------------------------------------------------------------
    // Initialisierung......
    // ---------------------------------------------------------------

    // Immer fuer 100% formatieren:
    sal_Bool bMapChanged = ImpCheckRefMapMode();

    if ( pParaPortion->GetLines().Count() == 0 )
    {
        EditLine* pL = new EditLine;
        pParaPortion->GetLines().Insert( pL, 0 );
    }

    // ---------------------------------------------------------------
    // Absatzattribute holen......
    // ---------------------------------------------------------------
    ContentNode* const pNode = pParaPortion->GetNode();
    SvxAdjust eJustification = SVX_ADJUST_LEFT;
    if ( !aStatus.IsOutliner() )
        eJustification = ((const SvxAdjustItem&)pNode->GetContentAttribs().GetItem( EE_PARA_JUST)).GetAdjust();
    sal_Bool bHyphenatePara = ((const SfxBoolItem&)pNode->GetContentAttribs().GetItem( EE_PARA_HYPHENATE )).GetValue();
    const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pNode );
    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&) pNode->GetContentAttribs().GetItem( EE_PARA_SBL );
    const BOOL bScriptSpace = ((const SvxScriptSpaceItem&) pNode->GetContentAttribs().GetItem( EE_PARA_ASIANCJKSPACING )).GetValue();

//  const sal_uInt16 nInvalidEnd = ( pParaPortion->GetInvalidDiff() > 0 )
//      ? pParaPortion->GetInvalidPosStart() + pParaPortion->GetInvalidDiff()
//      : pNode->Len();
    const short nInvalidDiff = pParaPortion->GetInvalidDiff();
    const sal_uInt16 nInvalidStart = pParaPortion->GetInvalidPosStart();
    const sal_uInt16 nInvalidEnd =  nInvalidStart + Abs( nInvalidDiff );

    sal_Bool bQuickFormat = sal_False;
    if ( !bEmptyNodeWithPolygon )
    {
        if ( ( pParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff > 0 ) &&
             ( pNode->Search( CH_FEATURE, nInvalidStart ) > nInvalidEnd ) )
        {
            bQuickFormat = sal_True;
        }
        else if ( ( pParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff < 0 ) )
        {
            // pruefen, ob loeschen ueber Portiongrenzen erfolgte...
            sal_uInt16 nStart = nInvalidStart;  // DOPPELT !!!!!!!!!!!!!!!
            sal_uInt16 nEnd = nStart - nInvalidDiff;  // neg.
            bQuickFormat = sal_True;
            sal_uInt16 nPos = 0;
            sal_uInt16 nPortions = pParaPortion->GetTextPortions().Count();
            for ( sal_uInt16 nTP = 0; nTP < nPortions; nTP++ )
            {
                // Es darf kein Start/Ende im geloeschten Bereich liegen.
                TextPortion* const pTP = pParaPortion->GetTextPortions()[ nTP ];
                nPos += pTP->GetLen();
                if ( ( nPos > nStart ) && ( nPos < nEnd ) )
                {
                    bQuickFormat = sal_False;
                    break;
                }
            }
        }
    }

    sal_uInt16 nRealInvalidStart = nInvalidStart;
    if ( bEmptyNodeWithPolygon )
    {
        TextPortion* pDummyPortion = new TextPortion( 0 );
        pParaPortion->GetTextPortions().Reset();
        pParaPortion->GetTextPortions().Insert( pDummyPortion, 0 );
    }
    else if ( bQuickFormat )
    {
        // schnellere Methode:
        RecalcTextPortion( pParaPortion, nInvalidStart, nInvalidDiff );
    }
    else    // nRealInvalidStart kann vor InvalidStart liegen, weil Portions geloescht....
        CreateTextPortions( pParaPortion, nRealInvalidStart );

    // ---------------------------------------------------------------
    // Zeile mit InvalidPos suchen, eine Zeile davor beginnen...
    // Zeilen flaggen => nicht removen !
    // ---------------------------------------------------------------

    sal_uInt16 nLine = pParaPortion->GetLines().Count()-1;
    for ( sal_uInt16 nL = 0; nL <= nLine; nL++ )
    {
        EditLine* pLine = pParaPortion->GetLines().GetObject( nL );
        if ( pLine->GetEnd() > nRealInvalidStart )  // nicht nInvalidStart!
        {
            nLine = nL;
            break;
        }
        pLine->SetValid();
    }
    // Eine Zeile davor beginnen...
    // Wenn ganz hinten getippt wird, kann sich die Zeile davor nicht aendern.
    if ( nLine && ( !pParaPortion->IsSimpleInvalid() || ( nInvalidEnd < pNode->Len() ) || ( nInvalidDiff <= 0 ) ) )
        nLine--;

    EditLine* pLine = pParaPortion->GetLines().GetObject( nLine );

    static Rectangle aZeroArea = Rectangle( Point(), Point() );
    Rectangle aBulletArea( aZeroArea );
    if ( !nLine )
    {
        aBulletArea = GetEditEnginePtr()->GetBulletArea( GetParaPortions().GetPos( pParaPortion ) );
        if ( aBulletArea.Right() > 0 )
            pParaPortion->SetBulletX( (sal_uInt16) GetXValue( aBulletArea.Right() ) );
        else
            pParaPortion->SetBulletX( 0 ); // Falls Bullet falsch eingestellt.
    }

    // ---------------------------------------------------------------
    // Ab hier alle Zeilen durchformatieren...
    // ---------------------------------------------------------------
    sal_uInt16 nDelFromLine = 0xFFFF;
    sal_Bool bLineBreak = sal_False;

    sal_uInt16 nIndex = pLine->GetStart();
    EditLine aSaveLine( *pLine );
    SvxFont aTmpFont( pNode->GetCharAttribs().GetDefFont() );

    sal_Bool bCalcCharPositions = sal_True;
    long* pBuf = new long[ pNode->Len() ];

    sal_Bool bSameLineAgain = sal_False;    // Fuer TextRanger, wenn sich die Hoehe aendert.
    TabInfo aCurrentTab;

    BOOL bForceOneRun = bEmptyNodeWithPolygon;

    while ( ( nIndex < pNode->Len() ) || bForceOneRun )
    {
        bForceOneRun = FALSE;

        sal_Bool bEOL = sal_False;
        sal_Bool bEOC = sal_False;
        sal_uInt16 nPortionStart = 0;
        sal_uInt16 nPortionEnd = 0;

        long nStartX = GetXValue( rLRItem.GetTxtLeft() );
        if ( nIndex == 0 )
        {
            long nFI = GetXValue( rLRItem.GetTxtFirstLineOfst() );
            nStartX += nFI;

            if ( !nLine && ( pParaPortion->GetBulletX() > nStartX ) )
            {
                nStartX -= nFI; // Vielleicht reicht der LI?
                if ( pParaPortion->GetBulletX() > nStartX )
                    nStartX = pParaPortion->GetBulletX();
            }
        }


        long nMaxLineWidth;
        if ( !IsVertical() )
            nMaxLineWidth = aStatus.AutoPageWidth() ? aMaxAutoPaperSize.Width() : aPaperSize.Width();
        else
            nMaxLineWidth = aStatus.AutoPageHeight() ? aMaxAutoPaperSize.Height() : aPaperSize.Height();

        nMaxLineWidth -= GetXValue( rLRItem.GetRight() );
        nMaxLineWidth -= nStartX;

        // Wenn PaperSize == long_max, kann ich keinen neg. Erstzeileneinzug
        // abziehen (Overflow)
        if ( ( nMaxLineWidth < 0 ) && ( nStartX < 0 ) )
            nMaxLineWidth = ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() ) - GetXValue( rLRItem.GetRight() );

        // Wenn jetzt noch kleiner 0, kann es nur der rechte Rand sein.
        if ( nMaxLineWidth < 0 )
            nMaxLineWidth = 1;

        // Problem: Da eine Zeile _vor_ der ungueltigen Position mit der
        // Formatierung begonnen wird, werden hier leider auch die Positionen
        // neu bestimmt...
        // Loesungsansatz:
        // Die Zeile davor kann nur groesser werden, nicht kleiner
        // => ...
        if ( bCalcCharPositions )
            pLine->GetCharPosArray().Remove( 0, pLine->GetCharPosArray().Count() );

        sal_uInt16 nTmpPos = nIndex;
        sal_uInt16 nTmpPortion = pLine->GetStartPortion();
        long nTmpWidth = 0;
        long nXWidth = nMaxLineWidth;
        if ( nXWidth <= nTmpWidth ) // while muss 1x durchlaufen werden
            nXWidth = nTmpWidth+1;

        SvLongsPtr pTextRanges = 0;
        long nTextExtraYOffset = 0;
        long nTextXOffset = 0;
        long nTextLineHeight = 0;
        if ( GetTextRanger() )
        {
            GetTextRanger()->SetVertical( IsVertical() );

            long nTextY = nStartPosY + GetEditCursor( pParaPortion, pLine->GetStart() ).Top();
            if ( !bSameLineAgain )
            {
                SeekCursor( pNode, nTmpPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                nTextLineHeight = aTmpFont.GetPhysTxtSize( GetRefDevice(), String() ).Height();
                // Metriken koennen groesser sein
                FormatterFontMetric aTempFormatterMetrics;
                RecalcFormatterFontMetrics( aTempFormatterMetrics, aTmpFont );
                sal_uInt16 nLineHeight = aTempFormatterMetrics.GetHeight();
                if ( nLineHeight > nTextLineHeight )
                    nTextLineHeight = nLineHeight;
            }
            else
                nTextLineHeight = pLine->GetHeight();

            nXWidth = 0;
            while ( !nXWidth )
            {
                long nYOff = nTextY + nTextExtraYOffset;
                long nYDiff = nTextLineHeight;
                if ( IsVertical() )
                {
                    long nMaxPolygonX = GetTextRanger()->GetBoundRect().Right();
                    nYOff = nMaxPolygonX-nYOff;
                    nYDiff = -nTextLineHeight;
                }
                pTextRanges = GetTextRanger()->GetTextRanges( Range( nYOff, nYOff + nYDiff ) );
                DBG_ASSERT( pTextRanges, "GetTextRanges?!" );
                long nMaxRangeWidth = 0;
                // Den breitesten Bereich verwenden...
                // Der breiteste Bereich koennte etwas verwirren, also
                // generell den ersten. Am besten mal richtig mit Luecken.
//              for ( sal_uInt16 n = 0; n < pTextRanges->Count(); )
                if ( pTextRanges->Count() )
                {
                    sal_uInt16 n = 0;
                    long nA = pTextRanges->GetObject( n++ );
                    long nB = pTextRanges->GetObject( n++ );
                    DBG_ASSERT( nA <= nB, "TextRange verdreht?" );
                    long nW = nB - nA;
                    if ( nW > nMaxRangeWidth )
                    {
                        nMaxRangeWidth = nW;
                        nTextXOffset = nA;
                    }
                }
                nXWidth = nMaxRangeWidth;
                if ( nXWidth )
                    nMaxLineWidth = nXWidth - nStartX - GetXValue( rLRItem.GetRight() );
                else
                {
                    // Weiter unten im Polygon versuchen.
                    // Unterhalb des Polygons die Paperbreite verwenden.
                    nTextExtraYOffset += Max( (long)(nTextLineHeight / 10), (long)1 );
                    if ( ( nTextY + nTextExtraYOffset  ) > GetTextRanger()->GetBoundRect().Bottom() )
                    {
                        nXWidth = !IsVertical() ? GetPaperSize().Width() : GetPaperSize().Height();
                        if ( !nXWidth ) // AutoPaperSize
                            nXWidth = 0x7FFFFFFF;
                    }
                }
            }
        }

        // Portion suchen, die nicht mehr in Zeile passt....
        TextPortion* pPortion;
        sal_Bool bBrokenLine = sal_False;
        bLineBreak = sal_False;
        EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( pLine->GetStart() );
        while ( ( nTmpWidth < nXWidth ) && !bEOL && ( nTmpPortion < pParaPortion->GetTextPortions().Count() ) )
        {
            nPortionStart = nTmpPos;
            pPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
            if ( pPortion->GetKind() == PORTIONKIND_HYPHENATOR )
            {
                // Portion wegschmeissen, ggf. die davor korrigieren, wenn
                // die Hyph-Portion ein Zeichen geschluckt hat...
                pParaPortion->GetTextPortions().Remove( nTmpPortion );
                if ( nTmpPortion && pPortion->GetLen() )
                {
                    nTmpPortion--;
                    TextPortion* pPrev = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
                    DBG_ASSERT( pPrev->GetKind() == PORTIONKIND_TEXT, "Portion?!" );
                    nTmpWidth -= pPrev->GetSize().Width();
                    nTmpPos -= pPrev->GetLen();
                    pPrev->SetLen( pPrev->GetLen() + pPortion->GetLen() );
                    pPrev->GetSize().Width() = (-1);
                }
                delete pPortion;
                DBG_ASSERT( nTmpPortion < pParaPortion->GetTextPortions().Count(), "Keine Portion mehr da!" );
                pPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
            }
            DBG_ASSERT( pPortion->GetKind() != PORTIONKIND_HYPHENATOR, "CreateLines: Hyphenator-Portion!" );
            DBG_ASSERT( pPortion->GetLen() || bProcessingEmptyLine, "Leere Portion in CreateLines ?!" );
            if ( pNextFeature && ( pNextFeature->GetStart() == nTmpPos ) )
            {
                sal_uInt16 nWhich = pNextFeature->GetItem()->Which();
                switch ( nWhich )
                {
                    case EE_FEATURE_TAB:
                    {
                        long nOldTmpWidth = nTmpWidth;

                        // Tab-Pos suchen...
                        long nCurPos = nTmpWidth+nStartX;
                        nCurPos -= rLRItem.GetTxtLeft();    // Tabs relativ zu LI
                        // Skalierung rausrechnen
                        if ( aStatus.DoStretch() && ( nStretchX != 100 ) )
                            nCurPos = nCurPos*100/nStretchX;

                        aCurrentTab.aTabStop = pNode->GetContentAttribs().FindTabStop( nCurPos, aEditDoc.GetDefTab() );
                        aCurrentTab.nTabPos = GetXValue( (long) ( aCurrentTab.aTabStop.GetTabPos() + rLRItem.GetTxtLeft() ) );
                        aCurrentTab.bValid = FALSE;
                        if ( ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_RIGHT ) ||
                             ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_CENTER ) ||
                             ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_DECIMAL ) )
                        {
                            // Bei LEFT/DEFAULT wird dieses Tab nicht mehr betrachtet.
                            aCurrentTab.bValid = TRUE;
                            aCurrentTab.nStartPosX = nTmpWidth;
                            aCurrentTab.nCharPos = nTmpPos;
                            aCurrentTab.nTabPortion = nTmpPortion;
                        }

                        pPortion->GetKind() = PORTIONKIND_TAB;
                        pPortion->SetExtraValue( aCurrentTab.aTabStop.GetFill() );
                        pPortion->GetSize().Height() = 0;
                        pPortion->GetSize().Width() = aCurrentTab.nTabPos - (nTmpWidth+nStartX);

                        DBG_ASSERT( pPortion->GetSize().Width() >= 0, "Tab falsch berechnet!" );

                        nTmpWidth = aCurrentTab.nTabPos-nStartX;

                        // Wenn dies das erste Token in der Zeile ist,
                        // und nTmpWidth > aPaperSize.Width, habe ich eine
                        // Endlos-Schleife!
                        if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                        {
                            // Aber was jetzt ?
                            // Tab passend machen
                            pPortion->GetSize().Width() = nXWidth-nOldTmpWidth;
                            nTmpWidth = nXWidth-1;
                            bEOL = sal_True;
                            bBrokenLine = sal_True;
                        }
                        pLine->GetCharPosArray().Insert( pPortion->GetSize().Width(), nTmpPos-pLine->GetStart() );
                    }
                    break;
                    case EE_FEATURE_LINEBR:
                    {
                        DBG_ASSERT( pPortion, "?!" );
                        pPortion->GetSize().Width() = 0;
                        bEOL = sal_True;
                        bLineBreak = sal_True;
                        pPortion->GetKind() = PORTIONKIND_LINEBREAK;
                    }
                    break;
                    case EE_FEATURE_FIELD:
                    {
                        long nCurWidth = nTmpWidth;
                        SeekCursor( pNode, nTmpPos+1, aTmpFont );
                        sal_Unicode cChar = 0;  // later: NBS?
                        aTmpFont.SetPhysFont( GetRefDevice() );
                        String aFieldValue = cChar ? String(cChar) : ((EditCharAttribField*)pNextFeature)->GetFieldValue();
                        if ( bCalcCharPositions || !pPortion->HasValidSize() )
                        {
                            pPortion->GetSize() = aTmpFont.QuickGetTextSize( GetRefDevice(), aFieldValue, 0, aFieldValue.Len(), 0 );
                            // Damit kein Scrollen bei ueberlangen Feldern
                            if ( pPortion->GetSize().Width() > nXWidth )
                                pPortion->GetSize().Width() = nXWidth;
                        }
                        nTmpWidth += pPortion->GetSize().Width();
                        pLine->GetCharPosArray().Insert( pPortion->GetSize().Width()-nCurWidth, nTmpPos-pLine->GetStart() );
                        pPortion->GetKind() = cChar ? PORTIONKIND_TEXT : PORTIONKIND_FIELD;
                        // Wenn dies das erste Token in der Zeile ist,
                        // und nTmpWidth > aPaperSize.Width, habe ich eine
                        // Endlos-Schleife!
                        if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                        {
                            nTmpWidth = nXWidth-1;
                            bEOL = sal_True;
                            bBrokenLine = sal_True;
                        }
                    }
                    break;
                    default:    DBG_ERROR( "Was fuer ein Feature ?" );
                }
                pNextFeature = pNode->GetCharAttribs().FindFeature( pNextFeature->GetStart() + 1  );
            }
            else
            {
                DBG_ASSERT( pPortion->GetLen() || bProcessingEmptyLine, "Empty Portion - Extra Space?!" );
                SeekCursor( pNode, nTmpPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                if ( bCalcCharPositions || !pPortion->HasValidSize() )
                    pPortion->GetSize() = aTmpFont.QuickGetTextSize( GetRefDevice(), *pParaPortion->GetNode(), nTmpPos, pPortion->GetLen(), pBuf );
                nTmpWidth += pPortion->GetSize().Width();
                if ( bCalcCharPositions )
                {
                    sal_uInt16 nLen = pPortion->GetLen();
                    // Es wird am Anfang generell das Array geplaettet
                    // => Immer einfach schnelles insert.
                    sal_uInt16 nPos = nTmpPos - pLine->GetStart();
                    pLine->GetCharPosArray().Insert( pBuf, nLen, nPos );
                }

                if( bScriptSpace && ( nTmpWidth < nXWidth ) && IsScriptChange( EditPaM( pNode, nTmpPos+pPortion->GetLen() ) ) )
                {
                    USHORT nSpacePortion = nTmpPortion+1;
                    TextPortion* pSpacePortion = NULL;

                    TextPortion* pNextPortion = ( nSpacePortion < pParaPortion->GetTextPortions().Count() ) ? pParaPortion->GetTextPortions().GetObject( nSpacePortion ): NULL;
                    if ( pNextPortion && ( pNextPortion->GetKind() == PORTIONKIND_EXTRASPACE ) )
                    {
                        pSpacePortion = pNextPortion;
                    }
                    else
                    {
                        pSpacePortion = new TextPortion( 0 );
                        pSpacePortion->GetKind() = PORTIONKIND_EXTRASPACE;
                        pParaPortion->GetTextPortions().Insert( pSpacePortion, nSpacePortion );
                    }
                    nTmpPortion++;  // Skip this Portion
                    pSpacePortion->GetSize().Width() = pPortion->GetSize().Height()/5;
                    nTmpWidth += pSpacePortion->GetSize().Width();
                }
            }

            if ( aCurrentTab.bValid && ( nTmpPortion != aCurrentTab.nTabPortion ) )
            {
                long nWidthAfterTab = 0;
                for ( USHORT n = aCurrentTab.nTabPortion+1; n <= nTmpPortion; n++  )
                {
                    TextPortion* pTP = pParaPortion->GetTextPortions().GetObject( n );
                    nWidthAfterTab += pTP->GetSize().Width();
                }
                long nW;    // Length before tab position
                if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_RIGHT )
                    nW = nWidthAfterTab;
                else if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_CENTER )
                    nW = nWidthAfterTab/2;
                else if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_DECIMAL )
                {
                    nW = nWidthAfterTab;
                    String aText = GetSelected( EditSelection(  EditPaM( pParaPortion->GetNode(), nTmpPos ),
                                                                EditPaM( pParaPortion->GetNode(), nTmpPos + pPortion->GetLen() ) ) );
                        USHORT nDecPos = aText.Search( aCurrentTab.aTabStop.GetDecimal() );
                    if ( nDecPos != STRING_NOTFOUND )
                    {
                        nW = aTmpFont.QuickGetTextSize( GetRefDevice(), *pParaPortion->GetNode(), nTmpPos, nDecPos, NULL ).Width();
                        aCurrentTab.bValid = FALSE;
                    }
                }
                long nMaxW = aCurrentTab.nTabPos - aCurrentTab.nStartPosX;
                if ( nW >= nMaxW )
                {
                    nW = nMaxW;
                    aCurrentTab.bValid = FALSE;
                }
                TextPortion* pTabPortion = pParaPortion->GetTextPortions().GetObject( aCurrentTab.nTabPortion );
                pTabPortion->GetSize().Width() = aCurrentTab.nTabPos - aCurrentTab.nStartPosX - nW;
                nTmpWidth = aCurrentTab.nStartPosX + pTabPortion->GetSize().Width() + nWidthAfterTab;
            }

            nTmpPos += pPortion->GetLen();
            nPortionEnd = nTmpPos;
            nTmpPortion++;
            if ( aStatus.OneCharPerLine() )
                bEOL = sal_True;
        }

        aCurrentTab.bValid = FALSE;

        // das war evtl. eine Portion zu weit:
        sal_Bool bFixedEnd = sal_False;
        if ( aStatus.OneCharPerLine() )
        {
            // Zustand vor Portion: ( bis auf nTmpWidth )
            nPortionEnd = nTmpPos;
            nTmpPos -= pPortion->GetLen();
            nPortionStart = nTmpPos;
            nTmpPortion--;

            bEOL = sal_True;
            bEOC = sal_False;

            // Und jetzt genau ein Zeichen:
            nTmpPos++;
            nTmpPortion++;
            nPortionEnd = nTmpPortion;
            // Eine Nicht-Feature-Portion muss gebrochen werden
            if ( pPortion->GetLen() > 1 )
            {
                DBG_ASSERT( pPortion->GetKind() == PORTIONKIND_TEXT, "Len>1, aber keine TextPortion?" );
                nTmpWidth -= pPortion->GetSize().Width();
                sal_uInt16 nP = SplitTextPortion( pParaPortion, nTmpPos, pLine );
                TextPortion* p = pParaPortion->GetTextPortions().GetObject( nP );
                DBG_ASSERT( p, "Portion ?!" );
                nTmpWidth += p->GetSize().Width();
            }
        }
        else if ( nTmpWidth >= nXWidth )
        {
            nPortionEnd = nTmpPos;
            nTmpPos -= pPortion->GetLen();
            nPortionStart = nTmpPos;
            nTmpPortion--;
            bEOL = sal_False;
            bEOC = sal_False;
            switch ( pPortion->GetKind() )
            {
                case PORTIONKIND_TEXT:
                {
                    nTmpWidth -= pPortion->GetSize().Width();
                }
                break;
                case PORTIONKIND_FIELD:
                case PORTIONKIND_TAB:
                {
                    nTmpWidth -= pPortion->GetSize().Width();
                    bEOL = sal_True;
                    bFixedEnd = sal_True;
                }
                break;
                default:
                {
                    // Ein Feature wird nicht umgebrochen:
                    DBG_ASSERT( ( pPortion->GetKind() == PORTIONKIND_LINEBREAK ) /*|| ( pPortion->GetKind() == PORTIONKIND_EXTRASPACE )*/, "Was fuer ein Feature ?" );
                    bEOL = sal_True;
                    bFixedEnd = sal_True;
                }
            }
        }
        else
        {
            bEOL = sal_True;
            bEOC = sal_True;
            pLine->SetEnd( nPortionEnd );
            DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "Keine TextPortions?" );
            pLine->SetEndPortion( (sal_uInt16)pParaPortion->GetTextPortions().Count() - 1 );
        }

        if ( aStatus.OneCharPerLine() )
        {
            pLine->SetEnd( nPortionEnd );
            pLine->SetEndPortion( nTmpPortion-1 );
        }
        else if ( bFixedEnd )
        {
            pLine->SetEnd( nPortionStart );
            pLine->SetEndPortion( nTmpPortion-1 );
        }
        else if ( bLineBreak || bBrokenLine )
        {
            pLine->SetEnd( nPortionStart+1 );
            pLine->SetEndPortion( nTmpPortion-1 );
            bEOC = sal_False; // wurde oben gesetzt, vielleich mal die if's umstellen?
        }
        else if ( !bEOL )
        {
            DBG_ASSERT( (nPortionEnd-nPortionStart) == pPortion->GetLen(), "Doch eine andere Portion?!" );
            long nRemainingWidth = nMaxLineWidth - nTmpWidth;
            sal_Bool bCanHyphenate = ( aTmpFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL );
            ImpBreakLine( pParaPortion, pLine, pPortion, nPortionStart,
                                            nRemainingWidth, bCanHyphenate && bHyphenatePara );
        }

        // ------------------------------------------------------------------
        // Zeile fertig => justieren
        // ------------------------------------------------------------------

        // CalcTextSize sollte besser durch ein kontinuierliches
        // Registrieren ersetzt werden !
        Size aTextSize = pLine->CalcTextSize( *pParaPortion );

        if ( aTextSize.Height() == 0 )
        {
            SeekCursor( pNode, pLine->GetStart(), aTmpFont );
            aTmpFont.SetPhysFont( pRefDev );
            aTextSize.Height() = aTmpFont.GetPhysTxtSize( pRefDev, String() ).Height();
            pLine->SetHeight( (sal_uInt16)aTextSize.Height() );
        }


        // Die Fontmetriken koennen nicht kontinuierlich berechnet werden,
        // wenn der Font sowieso eingestellt ist, weil ggf. ein grosser Font
        // erst nach dem Umbrechen ploetzlich in der naechsten Zeile landet
        // => Font-Metriken zu gross.
        FormatterFontMetric aFormatterMetrics;
        sal_uInt16 nTPos = pLine->GetStart();
        for ( sal_uInt16 nP = pLine->GetStartPortion(); nP <= pLine->GetEndPortion(); nP++ )
        {
            SeekCursor( pNode, nTPos+1, aTmpFont );
            aTmpFont.SetPhysFont( GetRefDevice() );
            RecalcFormatterFontMetrics( aFormatterMetrics, aTmpFont );
            TextPortion* pTP = pParaPortion->GetTextPortions().GetObject( nP );
            nTPos += pTP->GetLen();
        }
        sal_uInt16 nLineHeight = aFormatterMetrics.GetHeight();
        if ( nLineHeight > pLine->GetHeight() )
            pLine->SetHeight( nLineHeight );
        pLine->SetMaxAscent( aFormatterMetrics.nMaxAscent );

        bSameLineAgain = sal_False;
        if ( GetTextRanger() && ( pLine->GetHeight() > nTextLineHeight ) )
        {
            // Nochmal mit der anderen Groesse aufsetzen!
            bSameLineAgain = sal_True;
        }


        if ( !aStatus.IsOutliner() )
        {
            if ( rLSItem.GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
            {
                sal_uInt16 nMinHeight = GetYValue( rLSItem.GetLineHeight() );
                sal_uInt16 nTxtHeight = pLine->GetHeight();
                if ( nTxtHeight < nMinHeight )
                {
                    // Der Ascent muss um die Differenz angepasst werden:
                    long nDiff = nMinHeight - nTxtHeight;
                    pLine->SetMaxAscent( (sal_uInt16)(pLine->GetMaxAscent() + nDiff) );
                    pLine->SetHeight( nMinHeight, nTxtHeight );
                }
            }
            else if ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                if ( nPara || ( pLine->GetStartPortion() != 0 ) ) // Nicht die aller erste Zeile
                {
                    sal_uInt16 nTxtHeight = pLine->GetHeight();
                    sal_uInt32 nH = nTxtHeight;
                    nH *= rLSItem.GetPropLineSpace();
                    nH /= 100;
                    // Der Ascent muss um die Differenz angepasst werden:
                    long nDiff = pLine->GetHeight() - nH;
                    if ( nDiff > pLine->GetMaxAscent() )
                        nDiff = pLine->GetMaxAscent();
                    pLine->SetMaxAscent( (sal_uInt16)(pLine->GetMaxAscent() - nDiff) );
                    pLine->SetHeight( (sal_uInt16)nH, nTxtHeight );
                }
            }
        }


        // #80582# - Bullet should not influence line height
//      if ( !nLine )
//      {
//          long nBulletHeight = aBulletArea.GetHeight();
//          if ( nBulletHeight > (long)pLine->GetHeight() )
//          {
//              long nDiff =  nBulletHeight - (long)pLine->GetHeight();
//              // nDiff auf oben und unten verteilen.
//              pLine->SetMaxAscent( (sal_uInt16)(pLine->GetMaxAscent() + nDiff/2) );
//              pLine->SetHeight( (sal_uInt16)nBulletHeight );
//          }
//      }

        // Zeilenhoehe auf Window-Pixel alignen?
        // Nein, waere Positionsabhaengig.

        if ( ( !IsVertical() && aStatus.AutoPageWidth() ) ||
             ( IsVertical() && aStatus.AutoPageHeight() ) )
        {
            // Wenn die Zeile in die aktuelle Papierbreite passt, muss
            // diese Breite fuer die Ausrichting verwendet werden.
            // Wenn sie nicht passt oder sie die Papierbreite aendert,
            // wird bei Justification != LEFT sowieso noch mal formatiert.
            long nMaxLineWidthFix = ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() )
                                        - GetXValue( rLRItem.GetRight() ) - nStartX;
            if ( aTextSize.Width() < nMaxLineWidthFix )
                nMaxLineWidth = nMaxLineWidthFix;
        }
        switch ( eJustification )
        {
            case SVX_ADJUST_CENTER:
            {
                long n = ( nMaxLineWidth - aTextSize.Width() ) / 2;
                n += nStartX;  // Einrueckung bleibt erhalten.
                if ( n > 0 )
                    pLine->SetStartPosX( (sal_uInt16)n );
                else
                    pLine->SetStartPosX( 0 );

            }
            break;
            case SVX_ADJUST_RIGHT:
            {
                // Bei automatisch umgebrochenen Zeilen, die ein Blank
                // am Ende enthalten, darf das Blank nicht ausgegeben werden!

                long n = nMaxLineWidth - aTextSize.Width();
                n += nStartX;  // Einrueckung bleibt erhalten.
                if ( n > 0 )
                    pLine->SetStartPosX( (sal_uInt16)n );
                else
                    pLine->SetStartPosX( 0 );
            }
            break;
            case SVX_ADJUST_BLOCK:
            {
                long nRemainingSpace = nMaxLineWidth - aTextSize.Width();
                pLine->SetStartPosX( (sal_uInt16)nStartX );
                if ( !bEOC && ( nRemainingSpace > 0 ) ) // nicht die letzte Zeile...
                    pParaPortion->AdjustBlocks( pLine, nRemainingSpace );
            }
            break;
            default:
            {
                pLine->SetStartPosX( (sal_uInt16)nStartX ); // FI, LI
            }
            break;
        }

        // -----------------------------------------------------------------
        // pruefen, ob die Zeile neu ausgegeben werden muss...
        // -----------------------------------------------------------------
        pLine->SetInvalid();

        // Wenn eine Portion umgebrochen wurde sind ggf. viel zu viele Positionen
        // im CharPosArray:
        if ( bCalcCharPositions )
        {
            sal_uInt16 nLen = pLine->GetLen();
            sal_uInt16 nCount = pLine->GetCharPosArray().Count();
            if ( nCount > nLen )
                pLine->GetCharPosArray().Remove( nLen, nCount-nLen );
        }

        if ( GetTextRanger() )
        {
            if ( nTextXOffset )
                pLine->SetStartPosX( (sal_uInt16) ( pLine->GetStartPosX() + nTextXOffset ) );
            if ( nTextExtraYOffset )
            {
                pLine->SetHeight( (sal_uInt16) ( pLine->GetHeight() + nTextExtraYOffset ), 0, pLine->GetHeight() );
                pLine->SetMaxAscent( (sal_uInt16) ( pLine->GetMaxAscent() + nTextExtraYOffset ) );
            }
        }

        // Fuer kleiner 0 noch ueberlegen!
        if ( pParaPortion->IsSimpleInvalid() /* && ( nInvalidDiff > 0 ) */ )
        {
            // Aenderung durch einfache Textaenderung...
            // Formatierung nicht abbrechen, da Portions evtl. wieder
            // gesplittet werden muessen!
            // Wenn irgendwann mal abbrechbar, dann fogende Zeilen Validieren!
            // Aber ggf. als Valid markieren, damit weniger Ausgabe...
            if ( pLine->GetEnd() < nInvalidStart )
            {
                if ( *pLine == aSaveLine )
                {
                    pLine->SetValid();
                }
            }
            else
            {
                sal_uInt16 nStart = pLine->GetStart();
                sal_uInt16 nEnd = pLine->GetEnd();

                if ( nStart > nInvalidEnd )
                {
                    if ( ( ( nStart-nInvalidDiff ) == aSaveLine.GetStart() ) &&
                            ( ( nEnd-nInvalidDiff ) == aSaveLine.GetEnd() ) )
                    {
                        pLine->SetValid();
                        if ( bCalcCharPositions && bQuickFormat )
                        {
                            bCalcCharPositions = sal_False;
                            bLineBreak = sal_False;
                            pParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                            break;
                        }
                    }
                }
                else if ( bCalcCharPositions && bQuickFormat && ( nEnd > nInvalidEnd) )
                {
                    // Wenn die ungueltige Zeile so endet, dass die naechste an
                    // der 'gleichen' Textstelle wie vorher beginnt, also nicht
                    // anders umgebrochen wird, brauche ich dort auch nicht die
                    // textbreiten neu bestimmen:
                    if ( nEnd == ( aSaveLine.GetEnd() + nInvalidDiff ) )
                    {
                        bCalcCharPositions = sal_False;
                        bLineBreak = sal_False;
                        pParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                        break;
                    }
                }
            }
        }

        if ( !bSameLineAgain )
        {
            nIndex = pLine->GetEnd();   // naechste Zeile Start = letzte Zeile Ende
                                        // weil nEnd hinter das letzte Zeichen zeigt!

            sal_uInt16 nEndPortion = pLine->GetEndPortion();

            // Naechste Zeile oder ggf. neue Zeile....
            pLine = 0;
            if ( nLine < pParaPortion->GetLines().Count()-1 )
                pLine = pParaPortion->GetLines().GetObject( ++nLine );
            if ( pLine && ( nIndex >= pNode->Len() ) )
            {
                nDelFromLine = nLine;
                break;
            }
            if ( !pLine )
            {
                if ( nIndex < pNode->Len() )
                {
                    pLine = new EditLine;
                    pParaPortion->GetLines().Insert( pLine, ++nLine );
                }
                else if ( nIndex && bLineBreak && GetTextRanger() )
                {
                    // normaly CreateAndInsertEmptyLine would be called, but I want to use
                    // CreateLines, so I need Polygon code only here...
                    TextPortion* pDummyPortion = new TextPortion( 0 );
                    pParaPortion->GetTextPortions().Insert( pDummyPortion, pParaPortion->GetTextPortions().Count() );
                    pLine = new EditLine;
                    pParaPortion->GetLines().Insert( pLine, ++nLine );
                    bForceOneRun = TRUE;
                    bProcessingEmptyLine = TRUE;
                }
            }
            if ( pLine )
            {
                aSaveLine = *pLine;
                pLine->SetStart( nIndex );
                pLine->SetEnd( nIndex );
                pLine->SetStartPortion( nEndPortion+1 );
                pLine->SetEndPortion( nEndPortion+1 );
            }
        }
    }   // while ( Index < Len )

    if ( nDelFromLine != 0xFFFF )
        pParaPortion->GetLines().DeleteFromLine( nDelFromLine );

    DBG_ASSERT( pParaPortion->GetLines().Count(), "Keine Zeile nach CreateLines!" );

    if ( bLineBreak == sal_True )
        CreateAndInsertEmptyLine( pParaPortion, nStartPosY );

    delete pBuf;

    sal_Bool bHeightChanged = FinishCreateLines( pParaPortion );

    if ( bMapChanged )
        GetRefDevice()->Pop();

    return bHeightChanged;
}

void ImpEditEngine::CreateAndInsertEmptyLine( ParaPortion* pParaPortion, sal_uInt32 nStartPosY )
{
    DBG_ASSERT( !GetTextRanger(), "Don't use CreateAndInsertEmptyLine with a polygon!" );

    EditLine* pTmpLine = new EditLine;
    pTmpLine->SetStart( pParaPortion->GetNode()->Len() );
    pTmpLine->SetEnd( pParaPortion->GetNode()->Len() );
    pParaPortion->GetLines().Insert( pTmpLine, pParaPortion->GetLines().Count() );

    sal_Bool bLineBreak = pParaPortion->GetNode()->Len() ? sal_True : sal_False;
    const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pParaPortion->GetNode() );
    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pParaPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    short nStartX = GetXValue( (short)(rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst()) );

    Rectangle aBulletArea = Rectangle( Point(), Point() );
    if ( bLineBreak == sal_True )
    {
        nStartX = (short)GetXValue( rLRItem.GetTxtLeft() );
    }
    else
    {
        aBulletArea = GetEditEnginePtr()->GetBulletArea( GetParaPortions().GetPos( pParaPortion ) );
        if ( aBulletArea.Right() > 0 )
            pParaPortion->SetBulletX( (sal_uInt16) GetXValue( aBulletArea.Right() ) );
        else
            pParaPortion->SetBulletX( 0 ); // Falls Bullet falsch eingestellt.
        if ( pParaPortion->GetBulletX() > nStartX )
        {
            nStartX = (short)GetXValue( rLRItem.GetTxtLeft() );
            if ( pParaPortion->GetBulletX() > nStartX )
                nStartX = pParaPortion->GetBulletX();
        }
    }

    SvxFont aTmpFont;
    SeekCursor( pParaPortion->GetNode(), bLineBreak ? pParaPortion->GetNode()->Len() : 0, aTmpFont );
    aTmpFont.SetPhysFont( pRefDev );

    TextPortion* pDummyPortion = new TextPortion( 0 );
    pDummyPortion->GetSize() = aTmpFont.GetPhysTxtSize( pRefDev, String() );
    pParaPortion->GetTextPortions().Insert( pDummyPortion, pParaPortion->GetTextPortions().Count() );
    FormatterFontMetric aFormatterMetrics;
    RecalcFormatterFontMetrics( aFormatterMetrics, aTmpFont );
    pTmpLine->SetMaxAscent( aFormatterMetrics.nMaxAscent );
    pTmpLine->SetHeight( (sal_uInt16) pDummyPortion->GetSize().Height() );
    sal_uInt16 nLineHeight = aFormatterMetrics.GetHeight();
    if ( nLineHeight > pTmpLine->GetHeight() )
        pTmpLine->SetHeight( nLineHeight );

    if ( !aStatus.IsOutliner() )
    {
        SvxAdjust eJustification = ((const SvxAdjustItem&)pParaPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_JUST)).GetAdjust();
        long nMaxLineWidth = !IsVertical() ? aPaperSize.Width() : aPaperSize.Height();
        nMaxLineWidth -= GetXValue( rLRItem.GetRight() );
        long nTextXOffset = 0;
        if ( nMaxLineWidth < 0 )
            nMaxLineWidth = 1;
        if ( eJustification ==  SVX_ADJUST_CENTER )
            nStartX = nMaxLineWidth / 2;
        else if ( eJustification ==  SVX_ADJUST_RIGHT )
            nStartX = (short)nMaxLineWidth;

        nStartX += (short)nTextXOffset;
    }

    pTmpLine->SetStartPosX( nStartX );

    if ( !aStatus.IsOutliner() )
    {
        if ( rLSItem.GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
        {
            sal_uInt16 nMinHeight = rLSItem.GetLineHeight();
            sal_uInt16 nTxtHeight = pTmpLine->GetHeight();
            if ( nTxtHeight < nMinHeight )
            {
                // Der Ascent muss um die Differenz angepasst werden:
                long nDiff = nMinHeight - nTxtHeight;
                pTmpLine->SetMaxAscent( (sal_uInt16)(pTmpLine->GetMaxAscent() + nDiff) );
                pTmpLine->SetHeight( nMinHeight, nTxtHeight );
            }
        }
    }

    if ( !bLineBreak )
    {
        long nMinHeight = aBulletArea.GetHeight();
        if ( nMinHeight > (long)pTmpLine->GetHeight() )
        {
            long nDiff = nMinHeight - (long)pTmpLine->GetHeight();
            // nDiff auf oben und unten verteilen.
            pTmpLine->SetMaxAscent( (sal_uInt16)(pTmpLine->GetMaxAscent() + nDiff/2) );
            pTmpLine->SetHeight( (sal_uInt16)nMinHeight );
        }
    }
    else
    {
        // -2: Die neue ist bereits eingefuegt.
        EditLine* pLastLine = pParaPortion->GetLines().GetObject( pParaPortion->GetLines().Count()-2 );
        DBG_ASSERT( pLastLine, "Weicher Umbruch, keine Zeile ?!" );
        DBG_ASSERT( pLastLine->GetEnd() == pParaPortion->GetNode()->Len(), "Doch anders?" );
//      pTmpLine->SetStart( pLastLine->GetEnd() );
//      pTmpLine->SetEnd( pLastLine->GetEnd() );
        sal_uInt16 nPos = (sal_uInt16) pParaPortion->GetTextPortions().Count() - 1 ;
        pTmpLine->SetStartPortion( nPos );
        pTmpLine->SetEndPortion( nPos );
    }
}

sal_Bool ImpEditEngine::FinishCreateLines( ParaPortion* pParaPortion )
{
//  CalcCharPositions( pParaPortion );
    pParaPortion->SetValid();
    long nOldHeight = pParaPortion->GetHeight();
//  sal_uInt16 nPos = GetParaPortions().GetPos( pParaPortion );
//  DBG_ASSERT( nPos != USHRT_MAX, "FinishCreateLines: Portion nicht in Liste!" );
//  ParaPortion* pPrev = nPos ? GetParaPortions().GetObject( nPos-1 ) : 0;
    CalcHeight( pParaPortion );

    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "FinishCreateLines: Keine Text-Portion?" );
    sal_Bool bRet = ( pParaPortion->GetHeight() != nOldHeight );
    return bRet;
}

void ImpEditEngine::ImpBreakLine( ParaPortion* pParaPortion, EditLine* pLine, TextPortion* pPortion, sal_uInt16 nPortionStart, long nRemainingWidth, sal_Bool bCanHyphenate )
{
    ContentNode* const pNode = pParaPortion->GetNode();

    sal_uInt16 nBreakInLine = nPortionStart - pLine->GetStart();
    sal_uInt16 nMax = nBreakInLine + pPortion->GetLen();
    while ( ( nBreakInLine < nMax ) && ( pLine->GetCharPosArray()[nBreakInLine] <= nRemainingWidth ) )
        nBreakInLine++;
    sal_uInt16 nMaxBreakPos = nBreakInLine + pLine->GetStart();

    sal_uInt16 nMinBreakPos = 0;
    USHORT nAttrs = pNode->GetCharAttribs().GetAttribs().Count();
    for ( USHORT nAttr = nAttrs; nAttr; )
    {
        EditCharAttrib* pAttr = pNode->GetCharAttribs().GetAttribs()[--nAttr];
        if ( pAttr->IsFeature() && ( pAttr->GetEnd() <= nMaxBreakPos ) )
        {
            nMinBreakPos = pAttr->GetEnd();
            break;
        }
    }

    sal_uInt16 nBreakPos = nMaxBreakPos;

    lang::Locale aLocale = GetLocale( EditPaM( pNode, nBreakPos ) );

    Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
    OUString aText( *pNode );
    Reference< XHyphenator > xHyph;
    if ( bCanHyphenate )
        xHyph = GetHyphenator();
    i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, 1 );
    i18n::LineBreakUserOptions aUserOptions;

    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    LocaleDataWrapper aWrapper( xMSF, aLocale );
    i18n::ForbiddenCharacters aForbidden = aWrapper.getForbiddenCharacters();
    aUserOptions.forbiddenBeginCharacters = aForbidden.beginLine;
    aUserOptions.forbiddenEndCharacters = aForbidden.endLine;
    aUserOptions.applyForbiddenRules = ((const SfxBoolItem&)pNode->GetContentAttribs().GetItem( EE_PARA_FORBIDDENRULES )).GetValue();
    aUserOptions.allowPunctuationOutsideMargin = FALSE; // ((const SfxBoolItem&)pNode->GetContentAttribs().GetItem( EE_PARA_HANGINGPUNCTUATION )).GetValue();
    aUserOptions.allowHyphenateEnglish = FALSE;

    i18n::LineBreakResults aLBR = xBI->getLineBreak( *pNode, nBreakPos, aLocale, nMinBreakPos, aHyphOptions, aUserOptions );
    nBreakPos = (USHORT)aLBR.breakIndex;

    sal_Bool bBlankSeparator = ( ( nBreakPos >= pLine->GetStart() ) &&
                    ( pNode->GetChar( nBreakPos ) == ' ' ) ) ? sal_True : sal_False;

    // Egal ob Trenner oder nicht: Das Wort nach dem Trenner durch
    // die Silbentrennung jagen...
    // nMaxBreakPos ist das letzte Zeichen was in die Zeile passt,
    // nBreakPos ist der Wort-Anfang
    // Ein Problem gibt es, wenn das Dok so schmal ist, dass ein Wort
    // auf mehr als Zwei Zeilen gebrochen wird...

    sal_Bool bHyphenated = sal_False;
    sal_Unicode cAlternateReplChar = 0;
    sal_Unicode cAlternateExtraChar = 0;

#ifndef SVX_LIGHT
    if ( bCanHyphenate && GetHyphenator().is() )
    {
        // MT: I18N Umstellen auf getWordBoundary !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        sal_uInt16 nWordStart = nBreakPos;
        sal_uInt16 nWordEnd = nBreakPos;
        while ( ( nWordEnd < pNode->Len() ) &&
                    ( pNode->GetChar( nWordEnd ) != ' ' ) &&
                    ( !pNode->IsFeature( nWordEnd ) ) )
        {
            nWordEnd++;
        }
        String aWord( *pNode, nWordStart, nWordEnd - nWordStart );
        if ( aWord.Len() > 3 )
        {
            DBG_ASSERT( nWordEnd >= nMaxBreakPos, "Hyph: Break?" );
            sal_uInt16 nMinTrail = nWordEnd-nMaxBreakPos+1;     //+1: Vor dem angeknacksten Buchstaben
            Reference< XHyphenatedWord > xHyphWord;
            if (xHyphenator.is())
                xHyphWord = xHyphenator->hyphenate( aWord, aLocale, aWord.Len() - nMinTrail, Sequence< PropertyValue >() );
            if (xHyphWord.is())
            {
                sal_Bool bAlternate = xHyphWord->isAlternativeSpelling();
                sal_uInt16 nWordLen = 1 + xHyphWord->getHyphenPos();

                if ( ( nWordLen >= 2 ) && ( (nWordStart+nWordLen) >= (pLine->GetStart() + 2 ) ) )
                {
                    if ( !bAlternate )
                    {
                        bHyphenated = sal_True;
                        bBlankSeparator = sal_False;
                        nBreakPos = nWordStart + nWordLen;
                    }
                    else
                    {
                        String aAlt( xHyphWord->getHyphenatedWord() );

                        // Wir gehen von zwei Faellen aus, die nun
                        // vorliegen koennen:
                        // 1) packen wird zu pak-ken
                        // 2) Schiffahrt wird zu Schiff-fahrt
                        // In Fall 1 muss ein Zeichen ersetzt werden,
                        // in Fall 2 wird ein Zeichen hinzugefuegt.
                        // Die Identifikation wird erschwert durch Worte wie
                        // "Schiffahrtsbrennesseln", da der Hyphenator alle
                        // Position des Wortes auftrennt und "Schifffahrtsbrennnesseln"
                        // ermittelt. Wir koennen also eigentlich nicht unmittelbar vom
                        // Index des AlternativWord auf aWord schliessen.

                        // Das ganze geraffel wird durch eine Funktion am
                        // Hyphenator vereinfacht werden, sobald AMA sie einbaut...
                        sal_uInt16 nAltStart = nWordLen - 1;
                        sal_uInt16 nTxtStart = nAltStart - (aAlt.Len() - aWord.Len());
                        sal_uInt16 nTxtEnd = nTxtStart;
                        sal_uInt16 nAltEnd = nAltStart;

                        // Die Bereiche zwischen den nStart und nEnd ist
                        // die Differenz zwischen Alternativ- und OriginalString.
                        while( nTxtEnd < aWord.Len() && nAltEnd < aAlt.Len() &&
                               aWord.GetChar(nTxtEnd) != aAlt.GetChar(nAltEnd) )
                        {
                            ++nTxtEnd;
                            ++nAltEnd;
                        }

                        // Wenn ein Zeichen hinzugekommen ist, dann bemerken wir es jetzt:
                        if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
                            aWord.GetChar( nTxtEnd ) == aAlt.GetChar(nAltEnd) )
                        {
                            ++nAltEnd;
                            ++nTxtStart;
                            ++nTxtEnd;
                        }

                        DBG_ASSERT( ( nAltEnd - nAltStart ) == 1, "Alternate: Falsche Annahme!" );

                        if ( nTxtEnd > nTxtStart )
                            cAlternateReplChar = aAlt.GetChar( nAltStart );
                        else
                            cAlternateExtraChar = aAlt.GetChar( nAltStart );

                        bHyphenated = sal_True;
                        bBlankSeparator = sal_False;
                        nBreakPos = nWordStart + nTxtStart;
                        if ( cAlternateReplChar )
                            nBreakPos++;
                    }
                }
            }
        }
    }

#endif // !SVX_LIGHT

    if ( nBreakPos <= pLine->GetStart() )
    {
        // keine Trenner in Zeile => abhacken !
        nBreakPos = nMaxBreakPos;
        // MT: I18N nextCharacters !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if ( nBreakPos <= pLine->GetStart() )
            nBreakPos = pLine->GetStart() + 1;  // Sonst Endlosschleife!
    }

    // die angeknackste Portion ist die End-Portion
    pLine->SetEnd( nBreakPos );
    sal_uInt16 nEndPortion = SplitTextPortion( pParaPortion, nBreakPos, pLine );
    if ( bBlankSeparator /* && ( eJustification == SVX_ADJUST_RIGHT ) */ )
    {
        // Blanks am Zeilenende generell unterdruecken...
        TextPortion* pTP = pParaPortion->GetTextPortions().GetObject( nEndPortion );
        DBG_ASSERT( pTP->GetKind() == PORTIONKIND_TEXT, "BlankRubber: Keine TextPortion!" );
        DBG_ASSERT( nBreakPos > pLine->GetStart(), "SplitTextPortion am Anfang der Zeile?" );
        sal_uInt16 nBlankPosInArray = nBreakPos-1-pLine->GetStart();
        pTP->GetSize().Width() = ( nBlankPosInArray && ( pTP->GetLen() > 1 ) ) ? pLine->GetCharPosArray()[ nBlankPosInArray-1 ] : 0;
    }
    else if ( bHyphenated )
    {
        // Eine Portion fuer den Trenner einbauen...
        TextPortion* pHyphPortion = new TextPortion( 0 );
        pHyphPortion->GetKind() = PORTIONKIND_HYPHENATOR;
        String aHyphText( CH_HYPH );
        if ( cAlternateReplChar )
        {
            TextPortion* pPrev = pParaPortion->GetTextPortions().GetObject( nEndPortion );
            DBG_ASSERT( pPrev && pPrev->GetLen(), "Hyphenate: Prev portion?!" );
            pPrev->SetLen( pPrev->GetLen() - 1 );
            pHyphPortion->SetLen( 1 );
            pHyphPortion->SetExtraValue( cAlternateReplChar );
            // Breite der Portion davor korrigieren:
            pPrev->GetSize().Width() =
                pLine->GetCharPosArray()[ nBreakPos-1 - pLine->GetStart() - 1 ];
        }
        else if ( cAlternateExtraChar )
        {
            pHyphPortion->SetExtraValue( cAlternateExtraChar );
            aHyphText.Insert( cAlternateExtraChar, 0 );
        }

        // Breite der Hyph-Portion ermitteln:
        SvxFont aFont;
        SeekCursor( pParaPortion->GetNode(), nBreakPos, aFont );
        aFont.SetPhysFont( GetRefDevice() );
        pHyphPortion->GetSize().Height() = GetRefDevice()->GetTextHeight();
        pHyphPortion->GetSize().Width() = GetRefDevice()->GetTextWidth( aHyphText );

        pParaPortion->GetTextPortions().Insert( pHyphPortion, ++nEndPortion );
    }
    pLine->SetEndPortion( nEndPortion );
}

sal_uInt16 ImpEditEngine::SplitTextPortion( ParaPortion* pPortion, sal_uInt16 nPos, EditLine* pCurLine )
{
    DBG_ASSERT( pPortion, "SplitTextPortion: Welche ?" );

    // Die Portion bei nPos wird geplittet, wenn bei nPos nicht
    // sowieso ein Wechsel ist
    if ( nPos == 0 )
        return 0;

    sal_uInt16 nSplitPortion;
    sal_uInt16 nTmpPos = 0;
    TextPortion* pTextPortion = 0;
    sal_uInt16 nPortions = pPortion->GetTextPortions().Count();
    for ( nSplitPortion = 0; nSplitPortion < nPortions; nSplitPortion++ )
    {
        TextPortion* pTP = pPortion->GetTextPortions().GetObject(nSplitPortion);
        nTmpPos += pTP->GetLen();
        if ( nTmpPos >= nPos )
        {
            if ( nTmpPos == nPos )  // dann braucht nichts geteilt werden
            {
                // Skip Portions with ExtraSpace
                while ( ( (nSplitPortion+1) < nPortions ) && (pPortion->GetTextPortions().GetObject(nSplitPortion+1)->GetKind() == PORTIONKIND_EXTRASPACE ) )
                    nSplitPortion++;

                return nSplitPortion;
            }
            pTextPortion = pTP;
            break;
        }
    }

    DBG_ASSERT( pTextPortion, "Position ausserhalb des Bereichs!" );
    DBG_ASSERT( pTextPortion->GetKind() == PORTIONKIND_TEXT, "SplitTextPortion: Keine TextPortion!" );

    sal_uInt16 nOverlapp = nTmpPos - nPos;
    pTextPortion->GetLen() -= nOverlapp;
    TextPortion* pNewPortion = new TextPortion( nOverlapp );
    pPortion->GetTextPortions().Insert( pNewPortion, nSplitPortion+1 );
    // Groessen setzen:
    if ( pCurLine )
    {
        // Kein neues GetTextSize, sondern Werte aus Array verwenden:
        DBG_ASSERT( nPos > pCurLine->GetStart(), "SplitTextPortion am Anfang der Zeile?" );
        pTextPortion->GetSize().Width() = pCurLine->GetCharPosArray()[ nPos-pCurLine->GetStart()-1 ];
    }
    else
        pTextPortion->GetSize().Width() = (-1);

    return nSplitPortion;
}

void ImpEditEngine::CreateTextPortions( ParaPortion* pParaPortion, sal_uInt16& rStart /* , sal_Bool bCreateBlockPortions */ )
{
    sal_uInt16 nStartPos = rStart;
    ContentNode* pNode = pParaPortion->GetNode();
    DBG_ASSERT( pNode->Len(), "CreateTextPortions sollte nicht fuer leere Absaetze verwendet werden!" );

    SortedPositions aPositions;
    aPositions.Insert( (sal_uInt32) 0 );

    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttrib )
    {
        // Start und Ende in das Array eintragen...
        // Die InsertMethode laesst keine doppelten Werte zu....
        aPositions.Insert( pAttrib->GetStart() );
        aPositions.Insert( pAttrib->GetEnd() );
        nAttr++;
        pAttrib = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
    aPositions.Insert( pNode->Len() );

    if ( !pParaPortion->aScriptInfos.Count() )
        ((ImpEditEngine*)this)->InitScriptTypes( GetParaPortions().GetPos( pParaPortion ) );

    const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for ( USHORT nT = 0; nT < rTypes.Count(); nT++ )
        aPositions.Insert( rTypes[nT].nStartPos );

    if ( mpIMEInfos && mpIMEInfos->nLen && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetNode() == pNode ) )
    {
        sal_uInt16 nLastAttr = 0xFFFF;
        for( sal_uInt16 n = 0; n < mpIMEInfos->nLen; n++ )
        {
            if ( mpIMEInfos->pAttribs[n] != nLastAttr )
            {
                aPositions.Insert( mpIMEInfos->aPos.GetIndex() + n );
                nLastAttr = mpIMEInfos->pAttribs[n];
            }
        }
        aPositions.Insert( mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen );
    }

    // Ab ... loeschen:
    // Leider muss die Anzahl der TextPortions mit aPositions.Count()
    // nicht uebereinstimmen, da evtl. Zeilenumbrueche...
    sal_uInt16 nPortionStart = 0;
    sal_uInt16 nInvPortion = 0;
    for ( sal_uInt16 nP = 0; nP < pParaPortion->GetTextPortions().Count(); nP++ )
    {
        TextPortion* pTmpPortion = pParaPortion->GetTextPortions().GetObject(nP);
        nPortionStart += pTmpPortion->GetLen();
        if ( nPortionStart >= nStartPos )
        {
            nPortionStart -= pTmpPortion->GetLen();
            rStart = nPortionStart;
            nInvPortion = nP;
            break;
        }
    }
    DBG_ASSERT( nP < pParaPortion->GetTextPortions().Count() || !pParaPortion->GetTextPortions().Count(), "Nichts zum loeschen: CreateTextPortions" );
    if ( nInvPortion && ( nPortionStart+pParaPortion->GetTextPortions().GetObject(nInvPortion)->GetLen() > nStartPos ) )
    {
        // lieber eine davor...
        // Aber nur wenn es mitten in der Portion war, sonst ist es evtl.
        // die einzige in der Zeile davor !
        nInvPortion--;
        nPortionStart -= pParaPortion->GetTextPortions().GetObject(nInvPortion)->GetLen();
    }
    pParaPortion->GetTextPortions().DeleteFromPortion( nInvPortion );

    // Eine Portion kann auch durch einen Zeilenumbruch entstanden sein:
    aPositions.Insert( nPortionStart );

    sal_uInt16 nInvPos;
    sal_Bool bFound = aPositions.Seek_Entry( nPortionStart, &nInvPos );
    DBG_ASSERT( bFound && ( nInvPos < (aPositions.Count()-1) ), "InvPos ?!" );
    for ( sal_uInt16 i = nInvPos+1; i < aPositions.Count(); i++ )
    {
        TextPortion* pNew = new TextPortion( (sal_uInt16)aPositions[i] - (sal_uInt16)aPositions[i-1] );
        pParaPortion->GetTextPortions().Insert( pNew, pParaPortion->GetTextPortions().Count());
    }

    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "Keine Portions?!" );
#ifdef EDITDEBUG
    DBG_ASSERT( pParaPortion->DbgCheckTextPortions(), "Portions kaputt?" );
#endif
}

void ImpEditEngine::RecalcTextPortion( ParaPortion* pParaPortion, sal_uInt16 nStartPos, short nNewChars )
{
    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "Keine Portions!" );
    DBG_ASSERT( nNewChars, "RecalcTextPortion mit Diff == 0" );

    ContentNode* const pNode = pParaPortion->GetNode();
    if ( nNewChars > 0 )
    {
        // Wenn an nStartPos ein Attribut beginnt/endet, faengt eine neue Portion
        // an, ansonsten wird die Portion an nStartPos erweitert.

        if ( pNode->GetCharAttribs().HasBoundingAttrib( nStartPos ) || IsScriptChange( EditPaM( pNode, nStartPos ) ) )
        {
            sal_uInt16 nNewPortionPos = 0;
            if ( nStartPos )
                nNewPortionPos = SplitTextPortion( pParaPortion, nStartPos ) + 1;

            // Eine leere Portion kann hier stehen, wenn der Absatz leer war,
            // oder eine Zeile durch einen harten Zeilenumbruch entstanden ist.
            if ( ( nNewPortionPos < pParaPortion->GetTextPortions().Count() ) &&
                    !pParaPortion->GetTextPortions()[nNewPortionPos]->GetLen() )
            {
                DBG_ASSERT( pParaPortion->GetTextPortions()[nNewPortionPos]->GetKind() == PORTIONKIND_TEXT, "Leere Portion war keine TextPortion!" );
                pParaPortion->GetTextPortions()[nNewPortionPos]->GetLen() += nNewChars;
            }
            else
            {
                TextPortion* pNewPortion = new TextPortion( nNewChars );
                pParaPortion->GetTextPortions().Insert( pNewPortion, nNewPortionPos );
            }
        }
        else
        {
            sal_uInt16 nPortionStart;
            const sal_uInt16 nTP = pParaPortion->GetTextPortions().
                FindPortion( nStartPos, nPortionStart );
            TextPortion* const pTP = pParaPortion->GetTextPortions()[ nTP ];
            DBG_ASSERT( pTP, "RecalcTextPortion: Portion nicht gefunden"  );
            pTP->GetLen() += nNewChars;
            pTP->GetSize().Width() = (-1);
        }
    }
    else
    {
        // Portion schrumpfen oder ggf. entfernen.
        // Vor Aufruf dieser Methode muss sichergestellt sein, dass
        // keine Portions in dem geloeschten Bereich lagen!

        // Es darf keine reinragende oder im Bereich startende Portion geben,
        // also muss nStartPos <= nPos <= nStartPos - nNewChars(neg.) sein
        sal_uInt16 nPortion = 0;
        sal_uInt16 nPos = 0;
        sal_uInt16 nEnd = nStartPos-nNewChars;
        sal_uInt16 nPortions = pParaPortion->GetTextPortions().Count();
        TextPortion* pTP = 0;
        for ( nPortion = 0; nPortion < nPortions; nPortion++ )
        {
            pTP = pParaPortion->GetTextPortions()[ nPortion ];
            if ( ( nPos+pTP->GetLen() ) > nStartPos )
            {
                DBG_ASSERT( nPos <= nStartPos, "Start falsch!" );
                DBG_ASSERT( nPos+pTP->GetLen() >= nEnd, "End falsch!" );
                break;
            }
            nPos += pTP->GetLen();
        }
        DBG_ASSERT( pTP, "RecalcTextPortion: Portion nicht gefunden" );
        if ( ( nPos == nStartPos ) && ( (nPos+pTP->GetLen()) == nEnd ) )
        {
            // Portion entfernen;
            BYTE nType = pTP->GetKind();
            pParaPortion->GetTextPortions().Remove( nPortion );
            delete pTP;
            if ( nType == PORTIONKIND_LINEBREAK )
            {
                TextPortion* pNext = pParaPortion->GetTextPortions()[ nPortion ];
                if ( pNext && !pNext->GetLen() )
                {
                    // Dummy-Portion entfernen
                    pParaPortion->GetTextPortions().Remove( nPortion );
                    delete pNext;
                }
            }
        }
        else
        {
            DBG_ASSERT( pTP->GetLen() > (-nNewChars), "Portion zu klein zum schrumpfen!" );
            pTP->GetLen() += nNewChars;
        }

        // ganz am Schluss darf keine HYPHENATOR-Portion stehen bleiben...
        DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "RecalcTextPortions: Keine mehr da!" );
        sal_uInt16 nLastPortion = pParaPortion->GetTextPortions().Count() - 1;
        pTP = pParaPortion->GetTextPortions().GetObject( nLastPortion );
        if ( pTP->GetKind() == PORTIONKIND_HYPHENATOR )
        {
            // Portion wegschmeissen, ggf. die davor korrigieren, wenn
            // die Hyph-Portion ein Zeichen geschluckt hat...
            pParaPortion->GetTextPortions().Remove( nLastPortion );
            if ( nLastPortion && pTP->GetLen() )
            {
                TextPortion* pPrev = pParaPortion->GetTextPortions().GetObject( nLastPortion - 1 );
                DBG_ASSERT( pPrev->GetKind() == PORTIONKIND_TEXT, "Portion?!" );
                pPrev->SetLen( pPrev->GetLen() + pTP->GetLen() );
                pPrev->GetSize().Width() = (-1);
            }
            delete pTP;
        }
    }
#ifdef EDITDEBUG
    DBG_ASSERT( pParaPortion->DbgCheckTextPortions(), "Portions kaputt?" );
#endif
}

void ImpEditEngine::SetTextRanger( TextRanger* pRanger )
{
    if ( pTextRanger != pRanger )
    {
        delete pTextRanger;
        pTextRanger = pRanger;

        for ( sal_uInt16 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
        {
            ParaPortion* pParaPortion = GetParaPortions().GetObject( nPara );
            pParaPortion->MarkSelectionInvalid( 0, pParaPortion->GetNode()->Len() );
            pParaPortion->GetLines().Reset();
        }

        FormatFullDoc();
        UpdateViews( GetActiveView() );
        if ( GetUpdateMode() && GetActiveView() )
            pActiveView->ShowCursor( sal_False, sal_False );
    }
}

void ImpEditEngine::SetVertical( BOOL bVertical )
{
    if ( IsVertical() != bVertical )
    {
        GetEditDoc().SetVertical( bVertical );
        sal_Bool bUseCharAttribs = ( aStatus.GetControlWord() & EE_CNTRL_USECHARATTRIBS ) ? sal_True : sal_False;
        GetEditDoc().CreateDefFont( bUseCharAttribs );
        if ( IsFormatted() )
        {
            FormatFullDoc();
            UpdateViews( GetActiveView() );
        }
    }
}


void ImpEditEngine::SeekCursor( ContentNode* pNode, sal_uInt16 nPos, SvxFont& rFont, OutputDevice* pOut, sal_uInt16 nIgnoreWhich )
{
    // Es war mal geplant, SeekCursor( nStartPos, nEndPos, ... ), damit nur
    // ab der StartPosition neu gesucht wird.
    // Problem: Es mussten zwei Listen beruecksichtigt/gefuehrt werden:
    // OrderedByStart,OrderedByEnd.

    if ( nPos > pNode->Len() )
        nPos = pNode->Len();

    rFont = pNode->GetCharAttribs().GetDefFont();

    short nScriptType = GetScriptType( EditPaM( pNode, nPos ) );
    if ( ( nScriptType == i18n::ScriptType::ASIAN ) || ( nScriptType == i18n::ScriptType::COMPLEX ) )
    {
        const SvxFontItem& rFontItem = (const SvxFontItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_FONTINFO, nScriptType ) );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        Size aSz( rFont.GetSize() );
        aSz.Height() = ((const SvxFontHeightItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType ) ) ).GetHeight();
        rFont.SetSize( aSz );
        rFont.SetWeight( ((const SvxWeightItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_WEIGHT, nScriptType ))).GetWeight() );
        rFont.SetItalic( ((const SvxPostureItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_ITALIC, nScriptType ))).GetPosture() );
        rFont.SetLanguage( ((const SvxLanguageItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType ))).GetLanguage() );
    }

    sal_uInt16 nRelWidth = ((const SvxCharScaleWidthItem&)pNode->GetContentAttribs().GetItem( EE_CHAR_FONTWIDTH)).GetValue();

    if ( pOut )
        pOut->SetTextLineColor();

    if ( aStatus.UseCharAttribs() )
    {
        const CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
        sal_uInt16 nAttr = 0;
        EditCharAttrib* pAttrib = GetAttrib( rAttribs, nAttr );
        while ( pAttrib && ( pAttrib->GetStart() <= nPos ) )
        {
            // Beim Seeken nicht die Attr beruecksichtigen, die dort beginnen!
            // Leere Attribute werden beruecksichtigt( verwendet), da diese
            // gerade eingestellt wurden.
            // 12.4.95: Doch keine Leeren Attribute verwenden:
            // - Wenn gerade eingestellt und leer => keine Auswirkung auf Font
            // In einem leeren Absatz eingestellte Zeichen werden sofort wirksam.
            if ( ( pAttrib->Which() != nIgnoreWhich ) &&
                 ( ( ( pAttrib->GetStart() < nPos ) && ( pAttrib->GetEnd() >= nPos ) )
                     || ( !pNode->Len() ) ) )
            {
                DBG_ASSERT( ( pAttrib->Which() >= EE_CHAR_START ) && ( pAttrib->Which() <= EE_FEATURE_END ), "Unglueltiges Attribut in Seek() " );
                if ( IsScriptItemValid( pAttrib->Which(), nScriptType ) )
                #if SUPD >= 615
                    pAttrib->SetFont( rFont, pOut );
                #else
                    pAttrib->SetFont( rFont );
                #endif
                if ( pAttrib->Which() == EE_CHAR_FONTWIDTH )
                    nRelWidth = ((const SvxCharScaleWidthItem*)pAttrib->GetItem())->GetValue();
            }
            pAttrib = GetAttrib( rAttribs, ++nAttr );
        }
    }

    if ( aStatus.DoNotUseColors() )
    {
        // Hack fuer DL,weil JOE staendig die Pooldefaults verbiegt!
        // const SvxColorItem& rColorItem = (const SvxColorItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_COLOR );
        rFont.SetColor( /* rColorItem.GetValue() */ COL_BLACK );
    }

    if ( aStatus.DoStretch() || ( nRelWidth != 100 ) )
    {
        // Fuer das aktuelle Ausgabegeraet, weil es sonst bei einem
        // Drucker als RefDev auf dem Bildschirm #?!@' aussieht!
        OutputDevice* pDev = pOut ? pOut : GetRefDevice();
        rFont.SetPhysFont( pDev );
        FontMetric aMetric( pDev->GetFontMetric() );
        // Fuer die Hoehe nicht die Metriken nehmen, da das bei
        // Hoch-/Tiefgestellt schief geht.
        Size aRealSz( aMetric.GetSize().Width(), rFont.GetSize().Height() );
        if ( aStatus.DoStretch() )
        {
            if ( nStretchY != 100 )
            {
                aRealSz.Height() *= nStretchY;
                aRealSz.Height() /= 100;
            }
            if ( nStretchX != 100 )
            {
                aRealSz.Width() *= nStretchX;
                aRealSz.Width() /= 100;

                // Auch das Kerning: (long wegen Zwischenergebnis)
                long nKerning = rFont.GetFixKerning();
/*
                Die Ueberlegung war: Wenn neg. Kerning, aber StretchX = 200
                => Nicht das Kerning verdoppelt, also die Buchstaben weiter
                zusammenziehen
                ---------------------------
                Kern    StretchX    =>Kern
                ---------------------------
                 >0     <100        < (Proportional)
                 <0     <100        < (Proportional)
                 >0     >100        > (Proportional)
                 <0     >100        < (Der Betrag, also Antiprop)
*/
                if ( ( nKerning < 0  ) && ( nStretchX > 100 ) )
                {
                    // Antiproportional
                    nKerning *= 100;
                    nKerning /= nStretchX;
                }
                else if ( nKerning )
                {
                    // Proportional
                    nKerning *= nStretchX;
                    nKerning /= 100;
                }
                rFont.SetFixKerning( (short)nKerning );
            }
        }
        if ( nRelWidth != 100 )
        {
            aRealSz.Width() *= nRelWidth;
            aRealSz.Width() /= 100;
        }
        rFont.SetSize( aRealSz );
        // Font wird nicht restauriert...
    }

    if ( mpIMEInfos && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetNode() == pNode ) &&
        ( nPos > mpIMEInfos->aPos.GetIndex() ) && ( nPos <= ( mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen ) ) )
    {
        sal_uInt16 nAttr = mpIMEInfos->pAttribs[ nPos - mpIMEInfos->aPos.GetIndex() - 1 ];
        if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
            rFont.SetUnderline( UNDERLINE_BOLD );
        else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
            rFont.SetUnderline( UNDERLINE_DOTTED );
        else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
            rFont.SetUnderline( UNDERLINE_DOTTED );
        else if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
            rFont.SetColor( Color( COL_RED ) );
        else if ( nAttr & EXTTEXTINPUT_ATTR_HALFTONETEXT )
            rFont.SetColor( Color( COL_LIGHTGRAY ) );
        if ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT )
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            rFont.SetColor( rStyleSettings.GetHighlightTextColor() );
            rFont.SetFillColor( rStyleSettings.GetHighlightColor() );
            rFont.SetTransparent( FALSE );
        }
        else if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
        {
            rFont.SetUnderline( UNDERLINE_WAVE );
            if( pOut )
                pOut->SetTextLineColor( Color( COL_LIGHTGRAY ) );
        }
    }
}

void ImpEditEngine::RecalcFormatterFontMetrics( FormatterFontMetric& rCurMetrics, SvxFont& rFont )
{
    // Fuer Zeilenhoehe bei Hoch/Tief erstmal ohne Propr!
    sal_uInt16 nPropr = rFont.GetPropr();
    DBG_ASSERT( ( nPropr == 100 ) || rFont.GetEscapement(), "Propr ohne Escape?!" );
    if ( nPropr != 100 )
    {
        rFont.SetPropr( 100 );
        rFont.SetPhysFont( pRefDev );
    }
    FontMetric aMetric( pRefDev->GetFontMetric() );

    sal_uInt16 nAscent = (sal_uInt16)aMetric.GetAscent();
    sal_uInt16 nDescent = (sal_uInt16)aMetric.GetDescent();
    sal_uInt16 nLeading = ( aMetric.GetLeading() > 0 ) ? (sal_uInt16)aMetric.GetLeading() : 0;
    // Fonts ohne Leading bereiten Probleme
    if ( ( nLeading == 0 ) && ( pRefDev->GetOutDevType() == OUTDEV_PRINTER ) )
    {
        // Da schaun wir mal, was fuer eine Leading ich auf dem
        // Bildschirm erhalte
        VirtualDevice* pVDev = GetVirtualDevice( pRefDev->GetMapMode() );
        rFont.SetPhysFont( pVDev );
        aMetric = pVDev->GetFontMetric();

        // Damit sich die Leading nicht wieder rausrechnet,
        // wenn die ganze Zeile den Font hat, nTmpLeading.

        // 4/96: Kommt bei HP Laserjet 4V auch nicht hin
        // => Werte komplett vom Bildschirm holen.
//      sal_uInt16 nTmpLeading = (sal_uInt16)aMetric.GetLeading();
//      nAscent += nTmpLeading;
        nAscent = (sal_uInt16)aMetric.GetAscent();
        nDescent = (sal_uInt16)aMetric.GetDescent();
//      nLeading = (sal_uInt16)aMetric.GetLeading();
    }
    if ( nAscent > rCurMetrics.nMaxAscent )
        rCurMetrics.nMaxAscent = nAscent;
    if ( nDescent > rCurMetrics.nMaxDescent )
        rCurMetrics.nMaxDescent= nDescent;

    // Sonderbehandlung Hoch/Tief:
    if ( rFont.GetEscapement() )
    {
        // Jetzt unter Beruecksichtigung von Escape/Propr
        // Ascent oder Descent ggf vergroessern
        short nDiff = (short)(rFont.GetSize().Height()*rFont.GetEscapement()/100L);
        if ( rFont.GetEscapement() > 0 )
        {
            nAscent = (sal_uInt16) (((long)nAscent)*nPropr/100 + nDiff);
            if ( nAscent > rCurMetrics.nMaxAscent )
                rCurMetrics.nMaxAscent = nAscent;
        }
        else    // muss < 0 sein
        {
            nDescent = (sal_uInt16) (((long)nDescent)*nPropr/100 - nDiff);
            if ( nDescent > rCurMetrics.nMaxDescent )
                rCurMetrics.nMaxDescent= nDescent;
        }
    }
}

void ImpEditEngine::Paint( OutputDevice* pOutDev, Rectangle aClipRec, Point aStartPos, sal_Bool bStripOnly, short nOrientation )
{
    if ( !GetUpdateMode() && !bStripOnly )
        return;

    if ( !IsFormatted() )
        FormatDoc();

    long nFirstVisXPos = - pOutDev->GetMapMode().GetOrigin().X();
    long nFirstVisYPos = - pOutDev->GetMapMode().GetOrigin().Y();

    EditLine* pLine;
    Point aTmpPos;
    DBG_ASSERT( GetParaPortions().Count(), "Keine ParaPortion?!" );
    SvxFont aTmpFont( GetParaPortions()[0]->GetNode()->GetCharAttribs().GetDefFont() );
    Font aOldFont( pOutDev->GetFont() );
    // Bei gedrehtem Text wird aStartPos als TopLeft angesehen, da andere
    // Informationen fehlen, und sowieso das ganze Object ungescrollt
    // dargestellt wird.
    // Das Rechteck ist unendlich gross.
    Point aOrigin( aStartPos );
    double nCos, nSin;
    if ( nOrientation )
    {
        double nRealOrientation = nOrientation*F_PI1800;
        nCos = cos( nRealOrientation );
        nSin = sin( nRealOrientation );
    }

    // Fuer OnlineSpelling:
//  EditPaM aCursorPos;
//  if( GetStatus().DoOnlineSpelling() && pActiveView )
//      aCurPos = pActiveView->pImpEditView->GetEditSelections().Max();

    // --------------------------------------------------
    // Ueber alle Absaetze...
    // --------------------------------------------------
    for ( sal_uInt16 n = 0; n < GetParaPortions().Count(); n++ )
    {
        ParaPortion* pPortion = GetParaPortions().GetObject( n );
        DBG_ASSERT( pPortion, "NULL-Pointer in TokenList in Paint" );
        // falls beim Tippen Idle-Formatierung, asynchrones Paint.
        // Unsichtbare Portions koennen ungueltig sein.
        if ( pPortion->IsVisible() && pPortion->IsInvalid() )
            return;
        long nParaHeight = pPortion->GetHeight();
        sal_uInt16 nIndex = 0;
        if ( pPortion->IsVisible() && (
                ( !IsVertical() && ( ( aStartPos.Y() + nParaHeight ) > aClipRec.Top() ) ) ||
                ( IsVertical() && ( ( aStartPos.X() - nParaHeight ) < aClipRec.Right() ) ) ) )

        {
            // --------------------------------------------------
            // Ueber die Zeilen des Absatzes...
            // --------------------------------------------------
            sal_uInt16 nLines = pPortion->GetLines().Count();
            sal_uInt16 nLastLine = nLines-1;

            long nParaStartY( aStartPos.Y() );
            if ( !IsVertical() )
                aStartPos.Y() += pPortion->GetFirstLineOffset();
            else
                aStartPos.X() -= pPortion->GetFirstLineOffset();

            const SvxLineSpacingItem& rLSItem = ((const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL ));
            sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                                ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;
            for ( sal_uInt16 nLine = 0; nLine < nLines; nLine++ )
            {
                pLine = pPortion->GetLines().GetObject(nLine);
                DBG_ASSERT( pLine, "NULL-Pointer im Zeileniterator in UpdateViews" );
                aTmpPos = aStartPos;
                if ( !IsVertical() )
                {
                    aTmpPos.X() += pLine->GetStartPosX();
                    aTmpPos.Y() += pLine->GetMaxAscent();
                    aStartPos.Y() += pLine->GetHeight();
                }
                else
                {
                    aTmpPos.Y() += pLine->GetStartPosX();
                    aTmpPos.X() -= pLine->GetMaxAscent();
                    aStartPos.X() -= pLine->GetHeight();
                }

                if ( ( !IsVertical() && ( aStartPos.Y() > aClipRec.Top() ) )
                    || ( IsVertical() && aStartPos.X() < aClipRec.Right() ) )
                {
                    if ( ( nLine == 0 ) && !bStripOnly )    // erste Zeile
                    {
                        // VERT???
                        GetEditEnginePtr()->PaintingFirstLine( n, Point( aStartPos.X(), nParaStartY ), aTmpPos.Y(), aOrigin, nOrientation, pOutDev );
                    }
                    // --------------------------------------------------
                    // Ueber die Portions der Zeile...
                    // --------------------------------------------------
                    nIndex = pLine->GetStart();
                    for ( sal_uInt16 y = pLine->GetStartPortion(); y <= pLine->GetEndPortion(); y++ )
                    {
                        DBG_ASSERT( pPortion->GetTextPortions().Count(), "Zeile ohne Textportion im Paint!" );
                        TextPortion* pTextPortion = pPortion->GetTextPortions().GetObject( y );
                        DBG_ASSERT( pTextPortion, "NULL-Pointer im Portioniterator in UpdateViews" );
                        switch ( pTextPortion->GetKind() )
                        {
                            case PORTIONKIND_TEXT:
                            case PORTIONKIND_FIELD:
                            case PORTIONKIND_HYPHENATOR:
                            {
                                SeekCursor( pPortion->GetNode(), nIndex+1, aTmpFont, pOutDev );
#ifdef EDITDEBUG
                                if ( pTextPortion->GetKind() == PORTIONKIND_HYPHENATOR )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTGRAY );
                                    aTmpFont.SetTransparent( sal_False );
                                }
#endif
                                aTmpFont.SetPhysFont( pOutDev );

                                XubString aText;
                                const long* pDXArray = 0;
                                long* pTmpDXArray = 0;

                                if ( pTextPortion->GetKind() == PORTIONKIND_TEXT )
                                {
                                    aText = XubString( *(pPortion->GetNode()), nIndex, pTextPortion->GetLen() );
                                    pDXArray = pLine->GetCharPosArray().GetData()+( nIndex-pLine->GetStart() );
                                }
                                else if ( pTextPortion->GetKind() == PORTIONKIND_FIELD )
                                {
                                    EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature( nIndex );
                                    DBG_ASSERT( pAttr, "Feld nicht gefunden" );
                                    DBG_ASSERT( pAttr && pAttr->GetItem()->ISA( SvxFieldItem ), "Feld vom falschen Typ!" );
                                    aText = ((EditCharAttribField*)pAttr)->GetFieldValue();

                                    if ( pOutDev != GetRefDevice() )
                                    {
                                        pTmpDXArray = new long[ aText.Len() ];
                                        pDXArray = pTmpDXArray;
                                        Font aOldFont( GetRefDevice()->GetFont() );
                                        aTmpFont.SetPhysFont( GetRefDevice() );
                                        aTmpFont.QuickGetTextSize( GetRefDevice(), aText, 0, aText.Len(), pTmpDXArray );
                                        if ( aStatus.DoRestoreFont() )
                                            GetRefDevice()->SetFont( aOldFont );
                                    }
                                    else
                                        pDXArray = 0;
                                }
                                else if ( pTextPortion->GetKind() == PORTIONKIND_HYPHENATOR )
                                {
                                    if ( pTextPortion->GetExtraValue() )
                                        aText = pTextPortion->GetExtraValue();
                                    aText += CH_HYPH;
                                }

                                long nTxtWidth = pTextPortion->GetSize().Width();

                                if ( bStripOnly )
                                {
                                    // VERT???
                                    GetEditEnginePtr()->DrawingText( aTmpPos, aText, pDXArray, aTmpFont, n, nIndex );
                                }
                                else
                                {
                                    Point aOutPos( aTmpPos );
                                    short nEsc = aTmpFont.GetEscapement();
                                    if ( nOrientation )
                                    {
                                        aTmpFont.SetOrientation( aTmpFont.GetOrientation()+nOrientation );
                                        // aTmpFont.SetCharOrientation( nOrientation );

                                        // Bei Hoch/Tief selbst Hand anlegen:
                                        if ( aTmpFont.GetEscapement() )
                                        {
                                            aOutPos.Y() -= aTmpFont.GetEscapement() * aTmpFont.GetSize().Height() / 100L ;
                                            aTmpFont.SetEscapement( 0 );
                                        }

                                        aTmpFont.SetPhysFont( pOutDev );

                                        aOutPos = lcl_ImplCalcRotatedPos( aOutPos, aOrigin, nSin, nCos );
                                    }
                                    // nur ausgeben, was im sichtbaren Bereich beginnt:
                                    // Wichtig, weil Bug bei einigen Grafikkarten bei transparentem Font, Ausgabe bei neg.
                                    if ( nOrientation || ( !IsVertical() && ( ( aTmpPos.X() + nTxtWidth ) >= nFirstVisXPos ) )
                                            || ( IsVertical() && ( ( aTmpPos.Y() + nTxtWidth ) >= nFirstVisYPos ) ) )
                                    {
                                        if ( nEsc && ( ( aTmpFont.GetUnderline() != UNDERLINE_NONE ) ) )
                                        {
                                            // Das Hoch/Tief ohne Underline malen, das Underline
                                            // auf der BaseLine der Original-Fonthoehe ausgeben...

                                            // Aber nur, wenn davor auch Unterstrichen!
                                            sal_Bool bSpecialUnderline = sal_False;
                                            EditCharAttrib* pPrev = pPortion->GetNode()->GetCharAttribs().FindAttrib( EE_CHAR_ESCAPEMENT, nIndex );
                                            if ( pPrev )
                                            {
                                                SvxFont aDummy;
                                                // Unterstreichung davor?
                                                if ( pPrev->GetStart() )
                                                {
                                                    SeekCursor( pPortion->GetNode(), pPrev->GetStart(), aDummy );
                                                    if ( aDummy.GetUnderline() != UNDERLINE_NONE )
                                                        bSpecialUnderline = sal_True;
                                                }
                                                if ( !bSpecialUnderline && ( pPrev->GetEnd() < pPortion->GetNode()->Len() ) )
                                                {
                                                    SeekCursor( pPortion->GetNode(), pPrev->GetEnd()+1, aDummy );
                                                    if ( aDummy.GetUnderline() != UNDERLINE_NONE )
                                                        bSpecialUnderline = sal_True;
                                                }
                                            }
                                            if ( bSpecialUnderline )
                                            {
                                                Size aSz = aTmpFont.GetPhysTxtSize( pOutDev, aText, 0, aText.Len() );
                                                BYTE nProp = aTmpFont.GetPropr();
                                                aTmpFont.SetEscapement( 0 );
                                                aTmpFont.SetPropr( 100 );
                                                aTmpFont.SetPhysFont( pOutDev );
                                                String aBlanks;
                                                aBlanks.Fill( aText.Len(), ' ' );
                                                Point aUnderlinePos( aOutPos );
                                                if ( nOrientation )
                                                    aUnderlinePos = lcl_ImplCalcRotatedPos( aTmpPos, aOrigin, nSin, nCos );
                                                pOutDev->DrawStretchText( aUnderlinePos, aSz.Width(), aBlanks, 0, aText.Len() );

                                                aTmpFont.SetUnderline( UNDERLINE_NONE );
                                                if ( !nOrientation )
                                                    aTmpFont.SetEscapement( nEsc );
                                                aTmpFont.SetPropr( nProp );
                                                aTmpFont.SetPhysFont( pOutDev );
                                            }
                                        }
                                        aTmpFont.QuickDrawText( pOutDev, aOutPos, aText, 0, aText.Len(), pDXArray );
                                    }

#ifndef SVX_LIGHT
                                    if ( GetStatus().DoOnlineSpelling() && GetStatus().DoDrawRedLines() && pPortion->GetNode()->GetWrongList()->HasWrongs() && pTextPortion->GetLen() )
                                    {
                                        lcl_DrawRedLines( pOutDev, aTmpFont.GetSize().Height(), aTmpPos, nIndex, nIndex + pTextPortion->GetLen(), pDXArray, pPortion->GetNode()->GetWrongList(), nOrientation, aOrigin );
                                    }
#endif // !SVX_LIGHT
                                }

                                if ( pTmpDXArray )
                                    delete pTmpDXArray;
                                if ( !IsVertical() )
                                    aTmpPos.X() += nTxtWidth;
                                else
                                    aTmpPos.Y() += nTxtWidth;
                            }
                            break;
                            case PORTIONKIND_EXTRASPACE:
                            case PORTIONKIND_TAB:
                            {
                                if ( pTextPortion->GetExtraValue() && ( pTextPortion->GetExtraValue() != ' ' ) )
                                {
                                    aTmpFont.SetTransparent( sal_False );
                                    aTmpFont.SetEscapement( 0 );
                                    aTmpFont.SetPhysFont( pOutDev );
                                    long nCharWidth = aTmpFont.QuickGetTextSize( pOutDev, pTextPortion->GetExtraValue(), 0, 1, NULL ).Width();
                                    long nChars = pTextPortion->GetSize().Width() / nCharWidth;
                                    if ( nChars < 2 )
                                        nChars = 2; // wird durch DrawStretchText gestaucht.
                                    else if ( nChars == 2 )
                                        nChars = 3; // sieht besser aus

                                    String aText;
                                    aText.Fill( (USHORT)nChars, pTextPortion->GetExtraValue() );
                                    pOutDev->DrawStretchText( aTmpPos, pTextPortion->GetSize().Width(), aText );
                                }
                                if ( !IsVertical() )
                                    aTmpPos.X() += pTextPortion->GetSize().Width();
                                else
                                    aTmpPos.Y() += pTextPortion->GetSize().Width();
                            }
                            break;
                        }
                        if ( !IsVertical() && ( aTmpPos.X() > aClipRec.Right() ) )
                            break;  // Keine weitere Ausgabe in Zeile noetig
                        else if ( IsVertical() && ( aTmpPos.Y() > aClipRec.Bottom() ) )
                            break;  // Keine weitere Ausgabe in Zeile noetig
                        nIndex += pTextPortion->GetLen();
                    }
                }

                if ( ( nLine != nLastLine ) && !aStatus.IsOutliner() )
                {
                    if ( !IsVertical() )
                        aStartPos.Y() += nSBL;
                    else
                        aStartPos.X() -= nSBL;
                }

                // keine sichtbaren Aktionen mehr?
                if ( !IsVertical() && ( aStartPos.Y() >= aClipRec.Bottom() ) )
                    break;
                else if ( IsVertical() && ( aStartPos.X() <= aClipRec.Left() ) )
                    break;
            }

            if ( !aStatus.IsOutliner() )
            {
                const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
                long nUL = GetYValue( rULItem.GetLower() );
                if ( !IsVertical() )
                    aStartPos.Y() += nUL;
                else
                    aStartPos.X() -= nUL;
            }
        }
        else
        {
            if ( !IsVertical() )
                aStartPos.Y() += nParaHeight;
            else
                aStartPos.X() -= nParaHeight;
        }

        // keine sichtbaren Aktionen mehr?
        if ( !IsVertical() && ( aStartPos.Y() > aClipRec.Bottom() ) )
            break;
        if ( IsVertical() && ( aStartPos.X() < aClipRec.Left() ) )
            break;
    }
    if ( aStatus.DoRestoreFont() )
        pOutDev->SetFont( aOldFont );
}

void ImpEditEngine::Paint( ImpEditView* pView, const Rectangle& rRec, sal_Bool bUseVirtDev )
{
    DBG_ASSERT( pView, "Keine View - Kein Paint!" );
    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

    if ( !GetUpdateMode() || IsInUndo() )
        return;

    // Schnittmenge aus Paintbereich und OutputArea.
    Rectangle aClipRec( pView->GetOutputArea() );
    aClipRec.Intersection( rRec );

    Window* pOutWin = pView->GetWindow();

    if ( bUseVirtDev )
    {
        Rectangle aClipRecPixel( pOutWin->LogicToPixel( aClipRec ) );
        aClipRecPixel.Right() += 1; // falls abgerundet!
        aClipRecPixel.Bottom() += 1;    // falls abgerundet!

        // Wenn aClipRecPixel > XXXX, dann invalidieren ?!

        VirtualDevice* pVDev = GetVirtualDevice( pOutWin->GetMapMode() );

        pVDev->SetBackground( pView->GetBackgroundColor() );

        sal_Bool bVDevValid = sal_True;
        Size aOutSz( pVDev->GetOutputSizePixel() );
        if ( (  aOutSz.Width() < aClipRecPixel.GetWidth() ) ||
             (  aOutSz.Height() < aClipRecPixel.GetHeight() ) )
        {
            bVDevValid = pVDev->SetOutputSizePixel( aClipRecPixel.GetSize() );
        }
        else
        {
            // Das VirtDev kann bei einem Resize sehr gross werden =>
            // irgendwann mal kleiner machen!
            if ( ( aOutSz.Height() > ( aClipRecPixel.GetHeight() + RESDIFF ) ) ||
                 ( aOutSz.Width() > ( aClipRecPixel.GetWidth() + RESDIFF ) ) )
            {
                bVDevValid = pVDev->SetOutputSizePixel( aClipRecPixel.GetSize() );
            }
            else
            {
                pVDev->Erase();
            }
        }
        DBG_ASSERT( bVDevValid, "VDef konnte nicht vergroessert werden!" );
        if ( !bVDevValid )
        {
            Paint( pView, rRec, sal_False /* ohne VDev */ );
            return;
        }

        // PaintRect fuer VDev nicht mit alignter Groesse,
        // da sonst die Zeile darunter auch ausgegeben werden muss:
        Rectangle aTmpRec( Point( 0, 0 ), aClipRec.GetSize() );

        aClipRec = pOutWin->PixelToLogic( aClipRecPixel );
        Point aStartPos;
        if ( !IsVertical() )
        {
            aStartPos = aClipRec.TopLeft();
            aStartPos = pView->GetDocPos( aStartPos );
            aStartPos.X() *= (-1);
            aStartPos.Y() *= (-1);
        }
        else
        {
            aStartPos = aClipRec.TopRight();
            Point aDocPos( pView->GetDocPos( aStartPos ) );
            aStartPos.X() = aClipRec.GetSize().Width() + aDocPos.Y();
            aStartPos.Y() = -aDocPos.X();
        }

        Paint( pVDev, aTmpRec, aStartPos );

        sal_Bool bClipRegion;
        Region aOldRegion;
        MapMode aOldMapMode;
        if ( GetTextRanger() )
        {
            // Some problems here with push/pop, why?!
//          pOutWin->Push( PUSH_CLIPREGION|PUSH_MAPMODE );
            bClipRegion = pOutWin->IsClipRegion();
            aOldRegion = pOutWin->GetClipRegion();
            // Wie bekomme ich das Polygon an die richtige Stelle????
            // Das Polygon bezieht sich auf die View, nicht auf das Window
            // => Origin umsetzen...
            aOldMapMode = pOutWin->GetMapMode();
            Point aOrigin = aOldMapMode.GetOrigin();
            Point aViewPos = pView->GetOutputArea().TopLeft();
            aOrigin.Move( aViewPos.X(), aViewPos.Y() );
            aClipRec.Move( -aViewPos.X(), -aViewPos.Y() );
            MapMode aNewMapMode( aOldMapMode );
            aNewMapMode.SetOrigin( aOrigin );
            pOutWin->SetMapMode( aNewMapMode );
            pOutWin->SetClipRegion( Region( GetTextRanger()->GetPolyPolygon() ) );
        }

        pOutWin->DrawOutDev( aClipRec.TopLeft(), aClipRec.GetSize(),
                            Point(0,0), aClipRec.GetSize(), *pVDev );

        if ( GetTextRanger() )
        {
//          pOutWin->Pop();
            if ( bClipRegion )
                pOutWin->SetClipRegion( aOldRegion );
            else
                pOutWin->SetClipRegion();
            pOutWin->SetMapMode( aOldMapMode );
        }


        pView->DrawSelection();
    }
    else
    {
        Point aStartPos;
        if ( !IsVertical() )
        {
            aStartPos = pView->GetOutputArea().TopLeft();
            aStartPos.X() -= pView->GetVisDocLeft();
            aStartPos.Y() -= pView->GetVisDocTop();
        }
        else
        {
            aStartPos = pView->GetOutputArea().TopRight();
            aStartPos.X() += pView->GetVisDocTop();
            aStartPos.Y() -= pView->GetVisDocLeft();
        }

        // Wenn Doc-Breite < OutputArea,Width, nicht umgebrochene Felder,
        // stehen die Felder sonst ber, wenn > Zeile.
        // ( Oben nicht, da dort bereits Doc-Breite von Formatierung mit drin )
        if ( !IsVertical() && ( pView->GetOutputArea().GetWidth() > GetPaperSize().Width() ) )
        {
            long nMaxX = pView->GetOutputArea().Left() + GetPaperSize().Width();
            if ( aClipRec.Left() > nMaxX )
                return;
            if ( aClipRec.Right() > nMaxX )
                aClipRec.Right() = nMaxX;
        }

        sal_Bool bClipRegion = pOutWin->IsClipRegion();
        Region aOldRegion = pOutWin->GetClipRegion();
        pOutWin->IntersectClipRegion( aClipRec );

        Paint( pOutWin, aClipRec, aStartPos );

        if ( bClipRegion )
            pOutWin->SetClipRegion( aOldRegion );
        else
            pOutWin->SetClipRegion();

        pView->DrawSelection();
    }

}

void ImpEditEngine::InsertContent( ContentNode* pNode, sal_uInt16 nPos )
{
    DBG_ASSERT( pNode, "NULL-Poointer in InsertContent! " );
    DBG_ASSERT( IsInUndo(), "InsertContent nur fuer Undo()!" );
    ParaPortion* pNew = new ParaPortion( pNode );
    GetParaPortions().Insert( pNew, nPos );
    aEditDoc.Insert( pNode, nPos );
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPos );
}

EditPaM ImpEditEngine::SplitContent( sal_uInt16 nNode, sal_uInt16 nSepPos )
{
    ContentNode* pNode = aEditDoc.SaveGetObject( nNode );
    DBG_ASSERT( pNode, "Ungueltiger Node in SplitContent" );
    DBG_ASSERT( IsInUndo(), "SplitContent nur fuer Undo()!" );
    DBG_ASSERT( nSepPos <= pNode->Len(), "Index im Wald: SplitContent" );
    EditPaM aPaM( pNode, nSepPos );
    return ImpInsertParaBreak( aPaM );
}

EditPaM ImpEditEngine::ConnectContents( sal_uInt16 nLeftNode, sal_Bool bBackward )
{
    ContentNode* pLeftNode = aEditDoc.SaveGetObject( nLeftNode );
    ContentNode* pRightNode = aEditDoc.SaveGetObject( nLeftNode+1 );
    DBG_ASSERT( pLeftNode, "Ungueltiger linker Node in ConnectContents" );
    DBG_ASSERT( pRightNode, "Ungueltiger rechter Node in ConnectContents" );
    DBG_ASSERT( IsInUndo(), "ConnectContent nur fuer Undo()!" );
    return ImpConnectParagraphs( pLeftNode, pRightNode, bBackward );
}

void ImpEditEngine::SetUpdateMode( sal_Bool bUp, EditView* pCurView, sal_Bool bForceUpdate )
{
    sal_Bool bChanged = ( GetUpdateMode() != bUp );

    // Beim Umschalten von sal_True auf sal_False waren alle Selektionen sichtbar,
    // => Wegmalen
    // Umgekehrt waren alle unsichtbar => malen

//  DrawAllSelections();    sieht im Outliner schlecht aus !
//  EditView* pView = aEditViewList.First();
//  while ( pView )
//  {
//      DBG_CHKOBJ( pView, EditView, 0 );
//      pView->pImpEditView->DrawSelection();
//      pView = aEditViewList.Next();
//  }

    // Wenn !bFormatted, also z.B. nach SetText, braucht bei UpdateMode sal_True
    // nicht sofort formatiert werden, weil warscheinlich noch Text kommt.
    // Spaetestens bei einem Paint / CalcTextWidth wird formatiert.

    bUpdate = bUp;
    if ( bUpdate && ( bChanged || bForceUpdate ) )
        FormatAndUpdate( pCurView );
}

void ImpEditEngine::ShowParagraph( sal_uInt16 nParagraph, sal_Bool bShow )
{
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "ShowParagraph: Absatz existiert nicht!" );
    if ( pPPortion && ( pPPortion->IsVisible() != bShow ) )
    {
        pPPortion->SetVisible( bShow );

        if ( !bShow )
        {
            // Als deleted kenzeichnen, damit keine Selektion auf diesem
            // Absatz beginnt oder endet...
            DeletedNodeInfo* pDelInfo = new DeletedNodeInfo( (sal_uInt32)pPPortion->GetNode(), nParagraph );
            aDeletedNodes.Insert( pDelInfo, aDeletedNodes.Count() );
            UpdateSelections();
            // Dann kriege ich den unteren Bereich nicht invalidiert,
            // wenn UpdateMode = sal_False!
            // Wenn doch, dann vor SetVisible auf sal_False merken!
//          nCurTextHeight -= pPPortion->GetHeight();
        }

        if ( bShow && pPPortion->IsInvalid() )
        {
            if ( GetUpdateMode() && !GetTextRanger() )
            {
                if ( pPPortion->IsInvalid() )
                {
                    Font aOldFont( GetRefDevice()->GetFont() );
                    CreateLines( nParagraph, 0 );   // 0: Kein TextRanger
                    if ( aStatus.DoRestoreFont() )
                        GetRefDevice()->SetFont( aOldFont );
                }
                else
                    CalcHeight( pPPortion );
                nCurTextHeight += pPPortion->GetHeight();
            }
            else
                nCurTextHeight = 0x7fffffff;
        }

        pPPortion->SetMustRepaint( sal_True );
        if ( GetUpdateMode() && !IsInUndo() && !GetTextRanger() )
        {
            aInvalidRec = Rectangle(    Point( 0, GetParaPortions().GetYOffset( pPPortion ) ),
                                        Point( GetPaperSize().Width(), nCurTextHeight ) );
            UpdateViews( GetActiveView() );
        }
    }
}

sal_Bool ImpEditEngine::IsParagraphVisible( sal_uInt16 nParagraph )
{
    ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "IsParagraphVisible: Absatz existiert nicht!" );
    if ( pPPortion )
        return pPPortion->IsVisible();
    return sal_False;
}

EditSelection ImpEditEngine::MoveParagraphs( Range aOldPositions, sal_uInt16 nNewPos, EditView* pCurView )
{
    DBG_ASSERT( GetParaPortions().Count() != 0, "Keine Absaetze gefunden: MoveParagraphs" );
    if ( GetParaPortions().Count() == 0 )
        return EditSelection();
    aOldPositions.Justify();

    EditSelection aSel( ImpMoveParagraphs( aOldPositions, nNewPos ) );

    if ( nNewPos >= GetParaPortions().Count() )
        nNewPos = GetParaPortions().Count() - 1;

    // Dort, wo der Absatz eingefuegt wurde, muss richtig gepainted werden:
    // Dort, wo der Absatz entfernt wurde, muss richtig gepainted werden:
    // ( Und dazwischen entsprechend auch...)
    if ( pCurView && ( GetUpdateMode() == sal_True ) )
    {
        // in diesem Fall kann ich direkt neu malen, ohne die
        // Portions zu Invalidieren.
        sal_uInt16 nFirstPortion = Min( (sal_uInt16)aOldPositions.Min(), nNewPos );
        sal_uInt16 nLastPortion = Max( (sal_uInt16)aOldPositions.Max(), nNewPos );

        ParaPortion* pUpperPortion = GetParaPortions().SaveGetObject( nFirstPortion );
        ParaPortion* pLowerPortion = GetParaPortions().SaveGetObject( nLastPortion );

        aInvalidRec = Rectangle();  // leermachen
        aInvalidRec.Left() = 0;
        aInvalidRec.Right() = aPaperSize.Width();
        aInvalidRec.Top() = GetParaPortions().GetYOffset( pUpperPortion );
        aInvalidRec.Bottom() = GetParaPortions().GetYOffset( pLowerPortion ) + pLowerPortion->GetHeight();

        UpdateViews( pCurView );
    }
    else
    {
        // aber der oberen ungueltigen Position neu painten...
        sal_uInt16 nFirstInvPara = Min( (sal_uInt16)aOldPositions.Min(), nNewPos );
        InvalidateFromParagraph( nFirstInvPara );
    }
    return aSel;
}

void ImpEditEngine::InvalidateFromParagraph( sal_uInt16 nFirstInvPara )
{
    // Es werden nicht die folgenden Absaetze invalidiert,
    // da ResetHeight() => Groessenanderung => alles folgende wird
    // sowieso neu ausgegeben.
    ParaPortion* pTmpPortion;
    if ( nFirstInvPara != 0 )
    {
        pTmpPortion = GetParaPortions().GetObject( nFirstInvPara-1 );
        pTmpPortion->MarkInvalid( pTmpPortion->GetNode()->Len(), 0 );
    }
    else
    {
        pTmpPortion = GetParaPortions().GetObject( 0 );
        pTmpPortion->MarkSelectionInvalid( 0, pTmpPortion->GetNode()->Len() );
    }
    pTmpPortion->ResetHeight();
}

IMPL_LINK_INLINE_START( ImpEditEngine, StatusTimerHdl, Timer *, EMPTYARG )
{
    CallStatusHdl();
    return 0;
}
IMPL_LINK_INLINE_END( ImpEditEngine, StatusTimerHdl, Timer *, EMPTYARG )

void ImpEditEngine::CallStatusHdl()
{
    if ( aStatusHdlLink.IsSet() && aStatus.GetStatusWord() )
    {
        // Der Status muss vor Call zurueckgesetzt werden,
        // da im Hdl evtl. weitere Fags gesetzt werden...
        EditStatus aTmpStatus( aStatus );
        aStatus.Clear();
        aStatusHdlLink.Call( &aTmpStatus );
        aStatusTimer.Stop();    // Falls von Hand gerufen...
    }
}

ContentNode* ImpEditEngine::GetPrevVisNode( ContentNode* pCurNode )
{
    ParaPortion* pPortion = FindParaPortion( pCurNode );
    DBG_ASSERT( pPortion, "GetPrevVisibleNode: Keine passende Portion!" );
    pPortion = GetPrevVisPortion( pPortion );
    if ( pPortion )
        return pPortion->GetNode();
    return 0;
}

ContentNode* ImpEditEngine::GetNextVisNode( ContentNode* pCurNode )
{
    ParaPortion* pPortion = FindParaPortion( pCurNode );
    DBG_ASSERT( pPortion, "GetNextVisibleNode: Keine passende Portion!" );
    pPortion = GetNextVisPortion( pPortion );
    if ( pPortion )
        return pPortion->GetNode();
    return 0;
}

ParaPortion* ImpEditEngine::GetPrevVisPortion( ParaPortion* pCurPortion )
{
    sal_uInt16 nPara = GetParaPortions().GetPos( pCurPortion );
    DBG_ASSERT( nPara < GetParaPortions().Count() , "Portion nicht gefunden: GetPrevVisPortion" );
    ParaPortion* pPortion = nPara ? GetParaPortions()[--nPara] : 0;
    while ( pPortion && !pPortion->IsVisible() )
        pPortion = nPara ? GetParaPortions()[--nPara] : 0;

    return pPortion;
}

ParaPortion* ImpEditEngine::GetNextVisPortion( ParaPortion* pCurPortion )
{
    sal_uInt16 nPara = GetParaPortions().GetPos( pCurPortion );
    DBG_ASSERT( nPara < GetParaPortions().Count() , "Portion nicht gefunden: GetPrevVisNode" );
    ParaPortion* pPortion = GetParaPortions().SaveGetObject( ++nPara );
    while ( pPortion && !pPortion->IsVisible() )
        pPortion = GetParaPortions().SaveGetObject( ++nPara );

    return pPortion;
}

EditPaM ImpEditEngine::InsertParagraph( sal_uInt16 nPara )
{
    EditPaM aPaM;
    if ( nPara != 0 )
    {
        ContentNode* pNode = GetEditDoc().SaveGetObject( nPara-1 );
        if ( !pNode )
            pNode = GetEditDoc().SaveGetObject( GetEditDoc().Count() - 1 );
        DBG_ASSERT( pNode, "Kein einziger Absatz in InsertParagraph ?" );
        aPaM = EditPaM( pNode, pNode->Len() );
    }
    else
    {
        ContentNode* pNode = GetEditDoc().SaveGetObject( 0 );
        aPaM = EditPaM( pNode, 0 );
    }

    return ImpInsertParaBreak( aPaM );
}

EditSelection* ImpEditEngine::SelectParagraph( sal_uInt16 nPara )
{
    EditSelection* pSel = 0;
    ContentNode* pNode = GetEditDoc().SaveGetObject( nPara );
    DBG_ASSERTWARNING( pNode, "Absatz existiert nicht: SelectParagraph" );
    if ( pNode )
        pSel = new EditSelection( EditPaM( pNode, 0 ), EditPaM( pNode, pNode->Len() ) );

    return pSel;
}

void ImpEditEngine::FormatAndUpdate( EditView* pCurView )
{
    if ( bDowning )
        return ;

    if ( IsInUndo() )
        IdleFormatAndUpdate( pCurView );
    else
    {
        FormatDoc();
        UpdateViews( pCurView );
    }
}

void ImpEditEngine::BeginDrag( EditView* pView )
{
    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );
    DBG_CHKOBJ( pView, EditView, 0 );

    if ( !pDragAndDropInfo )
        pDragAndDropInfo = new DragAndDropInfo( *pView->GetWindow() );
    pDragAndDropInfo->bStarterOfDD = sal_True;

    // Sensibler Bereich, wo gescrollt werden soll.
    Size aSz( 5, 0 );
    aSz = pView->GetWindow()->PixelToLogic( aSz );
    pDragAndDropInfo->nSensibleRange = (sal_uInt16) aSz.Width();
    pDragAndDropInfo->nCursorWidth = (sal_uInt16) aSz.Width() / 2;
    MapMode aMapMode( pView->GetWindow()->GetMapMode() );
    aMapMode.SetOrigin( Point( 0, 0 ) );
    pDragAndDropInfo->aBackground.SetMapMode( aMapMode );

    EditSelection aCopySel( pView->pImpEditView->GetEditSelection() );
    ESelection aSaveSel = pView->GetSelection();
    if ( pDragAndDropInfo->pField )
    {
        aCopySel = CreateSel( pDragAndDropInfo->aDropSel );
        aSaveSel = pDragAndDropInfo->aDropSel;
    }
    else
    {
        aSaveSel.Adjust();
        pDragAndDropInfo->aDropSel = aSaveSel;  // Wird evtl. im Drop modifiziert.
    }
    CopyData( aCopySel, EXCHANGE_DRAGSERVER );

    // Falls Drag&Move in einer Engine, muessen Copy&Del geklammert sein!
    UndoActionStart( EDITUNDO_DRAGANDDROP );
    pView->HideCursor();

    sal_Bool bDestroyed = sal_False;
    pDestroyedMarker = &bDestroyed;

    sal_uInt16 nDragOptions = DRAG_ALL;
    if ( pView->IsReadOnly() )
        nDragOptions &= ~DRAG_MOVEABLE;
    DropAction aAction = pView->GetWindow()->ExecuteDrag
        ( Pointer( POINTER_MOVEDATA ), Pointer( POINTER_COPYDATA ), nDragOptions, NULL );

    if ( bDestroyed )
        return;

    pDestroyedMarker = NULL;

    // Nicht dass mich jemand im Drop abgeschossen hat...
    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );
    DBG_CHKOBJ( pView, EditView, 0 );

    if ( ( aAction == DROP_MOVE ) && !pView->IsReadOnly() )
    {
        if ( pDragAndDropInfo->bStarterOfDD && pDragAndDropInfo->bDroppedInMe )
        {
            // DropPos: Wohin wurde gedroppt, unabhaengig von laenge.
            ESelection aDropPos( pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos, pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos );
            ESelection aToBeDelSel = aSaveSel;
            ESelection aNewSel( pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos,
                                pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos );
            sal_Bool bBeforeSelection = aDropPos.IsLess( aSaveSel );
            DBG_ASSERT( ( aSaveSel.nEndPara - aSaveSel.nStartPara) >= 0, "Drop: SaveSelection nicht justiert!" );
            sal_uInt16 nParaDiff = aSaveSel.nEndPara - aSaveSel.nStartPara;
            if ( bBeforeSelection )
            {
                // aToBeDelSel anpassen.
                DBG_ASSERT( aSaveSel.nStartPara >= pDragAndDropInfo->aDropSel.nStartPara, "Doch nicht davor?" );
                aToBeDelSel.nStartPara += nParaDiff;
                aToBeDelSel.nEndPara += nParaDiff;
                // Zeichen korrigieren?
                if ( aToBeDelSel.nStartPara == pDragAndDropInfo->aDropSel.nEndPara )
                {
                    sal_uInt16 nMoreChars;
                    if ( pDragAndDropInfo->aDropSel.nStartPara == pDragAndDropInfo->aDropSel.nEndPara )
                        nMoreChars = pDragAndDropInfo->aDropSel.nEndPos - pDragAndDropInfo->aDropSel.nStartPos;
                    else
                        nMoreChars = pDragAndDropInfo->aDropSel.nEndPos;
                    aToBeDelSel.nStartPos += nMoreChars;
                    if ( aToBeDelSel.nStartPara == aToBeDelSel.nEndPara )
                        aToBeDelSel.nEndPos += nMoreChars;
                }
            }
            else
            {
                // aToBeDelSel ist ok, aber Selektion der View
                // muss angepasst werden, wenn davor geloescht wird!
                DBG_ASSERT( aSaveSel.nStartPara <= pDragAndDropInfo->aDropSel.nStartPara, "Doch nicht davor?" );
                aNewSel.nStartPara -= nParaDiff;
                aNewSel.nEndPara -= nParaDiff;
                // Zeichen korrigieren?
                if ( aSaveSel.nEndPara == pDragAndDropInfo->aDropSel.nStartPara )
                {
                    sal_uInt16 nLessChars;
                    if ( aSaveSel.nStartPara == aSaveSel.nEndPara )
                        nLessChars = aSaveSel.nEndPos - aSaveSel.nStartPos;
                    else
                        nLessChars = aSaveSel.nEndPos;
                    aNewSel.nStartPos -= nLessChars;
                    if ( aNewSel.nStartPara == aNewSel.nEndPara )
                        aNewSel.nEndPos -= nLessChars;
                }
            }

            pView->pImpEditView->DrawSelection();
            EditSelection aDelSel( CreateSel( aToBeDelSel ) );
            DBG_ASSERT( !aDelSel.DbgIsBuggy( aEditDoc ), "ToBeDel ist buggy!" );
            ImpDeleteSelection( aDelSel );
            if ( !bBeforeSelection )
            {
                DBG_ASSERT( !CreateSel( aNewSel ).DbgIsBuggy(aEditDoc), "Bad" );
                pView->pImpEditView->SetEditSelection( CreateSel( aNewSel ) );
            }
            FormatAndUpdate( pView );
            pView->pImpEditView->DrawSelection();
        }
        else
        {
            // andere EditEngine...
            pView->DeleteSelected();
        }


    }

    UndoActionEnd( EDITUNDO_DRAGANDDROP );

    DragServer::Clear();
    HideDDCursor( pView->GetWindow() );
    pView->ShowCursor();
    delete pDragAndDropInfo; pDragAndDropInfo = 0;
}

sal_Bool ImpEditEngine::Drop( const DropEvent& rEvt, EditView* pCurView )
{
    DBG_ASSERT( pCurView, "Keine View - Kein (Drag&)Drops" );

    HideDDCursor( pCurView->GetWindow() );

    Point aMousePos( rEvt.GetPosPixel() );
    // Logische Einheiten...
    aMousePos = pCurView->GetWindow()->PixelToLogic( aMousePos );
    if ( !pCurView->GetOutputArea().IsInside( aMousePos ) )
        return sal_False;

    // Erstmal kopieren, evtl. spaeter bei Move loeschen...
    // Durch QueryDrop ist sichergestellt, dass Move oder Copy.
    // Bei Move wird die Selection nach ExecuteDrag geloescht.

    DBG_ASSERT( HasData( EXCHANGE_DRAGSERVER ), "Drop ohne brauchbare Daten ?!" );
    DBG_ASSERT( pDragAndDropInfo->aDropDest.GetNode(), "Drop: PaM nicht initialisiert!" );
    if ( !pDragAndDropInfo->aDropDest.GetNode() )
        return sal_False;   // Unerlaubter Drop-Aufruf.

    EditPaM aPaM( pDragAndDropInfo->aDropDest );
    EditSelection aNewSel;

    UndoActionStart( EDITUNDO_DRAGANDDROP );

    // Selektion wegmalen...
    pCurView->pImpEditView->DrawSelection();

    aNewSel = PasteData( aPaM, EXCHANGE_DRAGSERVER, aStatus.AllowPasteSpecial() );

    pCurView->pImpEditView->SetEditSelection( aNewSel );
    FormatAndUpdate( pCurView );
    if ( pDragAndDropInfo && pDragAndDropInfo->bStarterOfDD )
    {
        // Nur dann setzen, wenn in gleicher Engine!
        pDragAndDropInfo->aDropSel.nStartPara = aEditDoc.GetPos( aPaM.GetNode() );
        pDragAndDropInfo->aDropSel.nStartPos = aPaM.GetIndex();
        pDragAndDropInfo->aDropSel.nEndPara = aEditDoc.GetPos( aNewSel.Max().GetNode() );
        pDragAndDropInfo->aDropSel.nEndPos = aNewSel.Max().GetIndex();
        pDragAndDropInfo->bDroppedInMe = sal_True;
    }
    else
    {
        delete pDragAndDropInfo;
        pDragAndDropInfo = 0;
    }

    UndoActionEnd( EDITUNDO_DRAGANDDROP );

    return sal_True;
}

sal_Bool ImpEditEngine::QueryDrop( const DropEvent& rEvt, EditView* pCurView )
{
    DBG_ASSERT( pCurView, "Keine View - Kein (Drag&)Drops" );
    Window* pWindow = pCurView->GetWindow();
    DBG_ASSERT( pWindow, "View ohne Fenster in QueryDrop!" );

    if ( !pDragAndDropInfo )
        pDragAndDropInfo = new DragAndDropInfo( *pCurView->GetWindow() );

    if ( rEvt.IsLeaveWindow() )
    {
        HideDDCursor( pWindow );
        return sal_False;
    }

    // Wenn ich die Aktion nicht kann....
    if ( ( rEvt.GetAction() != DROP_COPY ) && ( rEvt.GetAction() != DROP_MOVE ) )
        return sal_False;
    // Wenn ich das Format nicht kann....
    if ( !HasData( EXCHANGE_DRAGSERVER ) )
        return sal_False;

    Point aMousePos( rEvt.GetPosPixel() );
    // Logische Einheiten...
    aMousePos = pWindow->PixelToLogic( aMousePos );

    sal_Bool bInOutputArea = pCurView->GetOutputArea().IsInside( aMousePos );

    sal_Bool bAllowScroll = pCurView->pImpEditView->DoAutoScroll();
    if ( bAllowScroll )
    {
        long nScrollX = 0;
        long nScrollY = 0;
        // pruefen, ob im sensitiven Bereich
        if ( ( (aMousePos.X()-pDragAndDropInfo->nSensibleRange) < pCurView->GetOutputArea().Left() ) && ( ( aMousePos.X() + pDragAndDropInfo->nSensibleRange ) > pCurView->GetOutputArea().Left() ) )
                nScrollX = pCurView->GetOutputArea().GetWidth() / SCRLRANGE;
        else if ( ( (aMousePos.X()+pDragAndDropInfo->nSensibleRange) > pCurView->GetOutputArea().Right() ) && ( ( aMousePos.X() - pDragAndDropInfo->nSensibleRange ) < pCurView->GetOutputArea().Right() ) )
                nScrollX = -( pCurView->GetOutputArea().GetWidth() / SCRLRANGE );

        if ( ( (aMousePos.Y()-pDragAndDropInfo->nSensibleRange) < pCurView->GetOutputArea().Top() ) && ( ( aMousePos.Y() + pDragAndDropInfo->nSensibleRange ) > pCurView->GetOutputArea().Top() ) )
                nScrollY = pCurView->GetOutputArea().GetHeight() / SCRLRANGE;
        else if ( ( (aMousePos.Y()+pDragAndDropInfo->nSensibleRange) > pCurView->GetOutputArea().Bottom() ) && ( ( aMousePos.Y() - pDragAndDropInfo->nSensibleRange ) < pCurView->GetOutputArea().Bottom() ) )
                nScrollY = -( pCurView->GetOutputArea().GetHeight() / SCRLRANGE );

        if ( nScrollX || nScrollY )
        {
            HideDDCursor( pWindow );
            pCurView->Scroll( nScrollX, nScrollY, RGCHK_PAPERSZ1 );
        }
    }

    if ( !bInOutputArea )
    {
        HideDDCursor( pWindow );
        return sal_False;
    }

    Point aDocPos( pCurView->pImpEditView->GetDocPos( aMousePos ) );
    EditPaM aPaM = GetPaM( aDocPos );
    pDragAndDropInfo->aDropDest = aPaM;
    // Pruefen, ob der PaM in der Selektion liegt...
    if ( pCurView->HasSelection() )
    {
        // es darf nicht in eine Selektion gedroppt werden
        EPaM aP = CreateEPaM( aPaM );
        ESelection aDestSel( aP.nPara, aP.nIndex, aP.nPara, aP.nIndex);
        ESelection aCurSel = pCurView->GetSelection();
        aCurSel.Adjust();
        if ( !aDestSel.IsLess( aCurSel ) && !aDestSel.IsGreater( aCurSel ) )
        {
            HideDDCursor( pWindow );
            return sal_False;
        }
    }

    Rectangle aEditCursor = PaMtoEditCursor( aPaM );
    Point aTopLeft( pCurView->pImpEditView->GetWindowPos( aEditCursor.TopLeft() ) );
    aEditCursor.SetPos( aTopLeft );
    aEditCursor.Right() = aEditCursor.Left() + pDragAndDropInfo->nCursorWidth;

    aEditCursor = pWindow->LogicToPixel( aEditCursor );
    aEditCursor = pWindow->PixelToLogic( aEditCursor );
    sal_Bool bCursorChanged = !pDragAndDropInfo->bVisCursor || ( pDragAndDropInfo->aCurCursor != aEditCursor );

    // Alten Cursor wegzeichnen...
    if ( bCursorChanged )
    {
        HideDDCursor( pWindow );
        ShowDDCursor( pWindow, aEditCursor );
    }

    return sal_True;
}

void ImpEditEngine::HideDDCursor( Window* pWin )
{
    if ( pDragAndDropInfo && pDragAndDropInfo->bVisCursor )
    {
        pWin->DrawOutDev( pDragAndDropInfo->aCurSavedCursor.TopLeft(), pDragAndDropInfo->aCurSavedCursor.GetSize(),
                            Point(0,0), pDragAndDropInfo->aCurSavedCursor.GetSize(), pDragAndDropInfo->aBackground );
        pDragAndDropInfo->bVisCursor = sal_False;
    }
}

void ImpEditEngine::ShowDDCursor( Window* pWin, const Rectangle& rRect )
{
    if ( !pDragAndDropInfo->bVisCursor )
    {
        Brush aOldBrush = pWin->GetFillInBrush( );
        pWin->SetFillInBrush( Brush( Color( COL_GRAY), BRUSH_50 ) );

        // Hintergrund sichern...
        Rectangle aSaveRec( pWin->LogicToPixel( rRect ) );
        // lieber etwas mehr sichern...
        aSaveRec.Right() += 1;
        aSaveRec.Bottom() += 1;

        Size aNewSzPx( aSaveRec.GetSize() );
        Size aCurSzPx( pDragAndDropInfo->aBackground.GetOutputSizePixel() );
        if ( ( aCurSzPx.Width() < aNewSzPx.Width() ) ||( aCurSzPx.Height() < aNewSzPx.Height() ) )
        {
            sal_Bool bDone = pDragAndDropInfo->aBackground.SetOutputSizePixel( aNewSzPx );
            DBG_ASSERT( bDone, "Virtuelles Device kaputt?" );
        }

        aSaveRec = pWin->PixelToLogic( aSaveRec );

        MapMode aMapMode( pWin->GetMapMode() );
        aMapMode.SetOrigin( Point( 0, 0 ) );
        pDragAndDropInfo->aBackground.SetMapMode( aMapMode );
        pDragAndDropInfo->aBackground.DrawOutDev( Point(0,0), aSaveRec.GetSize(),
                            aSaveRec.TopLeft(), aSaveRec.GetSize(), *pWin);
        pDragAndDropInfo->aCurSavedCursor = aSaveRec;

        // Cursor malen...
        pWin->DrawRect( rRect );

        pDragAndDropInfo->bVisCursor = sal_True;
        pDragAndDropInfo->aCurCursor = rRect;

        pWin->SetFillInBrush( aOldBrush );
    }
}

ESelection ImpEditEngine::GetDropPos()
{
    ESelection aESel;
    if ( pDragAndDropInfo && pDragAndDropInfo->aDropDest.GetNode() )
        aESel = CreateESel( EditSelection( pDragAndDropInfo->aDropDest, pDragAndDropInfo->aDropDest ) );
    return aESel;
}

void ImpEditEngine::SetFlatMode( sal_Bool bFlat )
{
    if ( bFlat != aStatus.UseCharAttribs() )
        return;

    if ( !bFlat )
        aStatus.TurnOnFlags( EE_CNTRL_USECHARATTRIBS );
    else
        aStatus.TurnOffFlags( EE_CNTRL_USECHARATTRIBS );

    aEditDoc.CreateDefFont( !bFlat );

    FormatFullDoc();
    UpdateViews( (EditView*) 0);
    if ( pActiveView )
        pActiveView->ShowCursor();
}

void ImpEditEngine::SetCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{
    if ( !IsVertical() )
    {
        nStretchX = nX;
        nStretchY = nY;
    }
    else
    {
        nStretchX = nY;
        nStretchY = nX;
    }

    if ( aStatus.DoStretch() )
    {
        FormatFullDoc();
        UpdateViews( GetActiveView() );
    }
}

void ImpEditEngine::DoStretchChars( sal_uInt16 nX, sal_uInt16 nY )
{
    UndoActionStart( EDITUNDO_STRETCH );
    sal_uInt16 nParas = GetEditDoc().Count();
    for ( sal_uInt16 nPara = 0; nPara < nParas; nPara++ )
    {
        ContentNode* pNode = GetEditDoc()[nPara];
        SfxItemSet aTmpSet( pNode->GetContentAttribs().GetItems() );

        if ( nX != 100 )
        {
            // Fontbreite
            SvxCharScaleWidthItem* pNewWidth = (SvxCharScaleWidthItem*) pNode->GetContentAttribs().GetItem( EE_CHAR_FONTWIDTH ).Clone();
            sal_uInt32 nProp = pNewWidth->GetValue();   // sal_uInt32, kann temporaer gross werden
            nProp *= nX;
            nProp /= 100;
            pNewWidth->SetValue( (sal_uInt16)nProp );
            aTmpSet.Put( *pNewWidth );
            delete pNewWidth;

            // Kerning:
            const SvxKerningItem& rKerningItem =
                (const SvxKerningItem&)pNode->GetContentAttribs().GetItem( EE_CHAR_KERNING );
            SvxKerningItem* pNewKerning = (SvxKerningItem*)rKerningItem.Clone();
            long nKerning = pNewKerning->GetValue();
            if ( nKerning > 0 )
            {
                nKerning *= nX;
                nKerning /= 100;
            }
            else if ( nKerning < 0 )
            {
                // Bei Negativen Werten:
                // Bei Stretching > 100 muessen die Werte kleiner werden und umgekehrt.
                nKerning *= 100;
                nKerning /= nX;
            }
            pNewKerning->SetValue( (short)nKerning );
            aTmpSet.Put( *pNewKerning);
            delete pNewKerning;
        }
        else
            aTmpSet.ClearItem( EE_CHAR_FONTWIDTH );

        if ( nY != 100 )
        {
            // Fonthoehe
            const SvxFontHeightItem& rHeightItem =
                (const SvxFontHeightItem&)pNode->GetContentAttribs().GetItem( EE_CHAR_FONTHEIGHT );
            SvxFontHeightItem* pNewHeight = (SvxFontHeightItem*)rHeightItem.Clone();
            sal_uInt32 nHeight = pNewHeight->GetHeight();
            nHeight *= nY;
            nHeight /= 100;
            pNewHeight->SetHeightValue( nHeight );
            aTmpSet.Put( *pNewHeight );
            delete pNewHeight;

            // Absatzabstaende
            const SvxULSpaceItem& rULSpaceItem =
                (const SvxULSpaceItem&)pNode->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
            SvxULSpaceItem* pNewUL = (SvxULSpaceItem*)rULSpaceItem.Clone();
            sal_uInt32 nUpper = pNewUL->GetUpper();
            nUpper *= nY;
            nUpper /= 100;
            pNewUL->SetUpper( (sal_uInt16)nUpper );
            sal_uInt32 nLower = pNewUL->GetLower();
            nLower *= nY;
            nLower /= 100;
            pNewUL->SetLower( (sal_uInt16)nLower );
            aTmpSet.Put( *pNewUL );
            delete pNewUL;
        }
        else
            aTmpSet.ClearItem( EE_CHAR_FONTHEIGHT );

        SetParaAttribs( nPara, aTmpSet );

        // harte Attribute:
        sal_uInt16 nLastEnd = 0;    // damit nach entfernen und neu nicht nochmal
        CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
        sal_uInt16 nAttribs = rAttribs.Count();
        for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            EditCharAttrib* pAttr = rAttribs[nAttr];
            if ( pAttr->GetStart() >= nLastEnd )
            {
                sal_uInt16 nWhich = pAttr->Which();
                SfxPoolItem* pNew = 0;
                if ( nWhich == EE_CHAR_FONTHEIGHT )
                {
                    SvxFontHeightItem* pNewHeight = (SvxFontHeightItem*)pAttr->GetItem()->Clone();
                    sal_uInt32 nHeight = pNewHeight->GetHeight();
                    nHeight *= nY;
                    nHeight /= 100;
                    pNewHeight->SetHeightValue( nHeight );
                    pNew = pNewHeight;
                }
                else if ( nWhich == EE_CHAR_FONTWIDTH )
                {
                    SvxCharScaleWidthItem* pNewWidth = (SvxCharScaleWidthItem*)pAttr->GetItem()->Clone();
                    sal_uInt32 nProp = pNewWidth->GetValue();
                    nProp *= nX;
                    nProp /= 100;
                    pNewWidth->SetValue( (sal_uInt16)nProp );
                    pNew = pNewWidth;
                }
                else if ( nWhich == EE_CHAR_KERNING )
                {
                    SvxKerningItem* pNewKerning = (SvxKerningItem*)pAttr->GetItem()->Clone();
                    long nKerning = pNewKerning->GetValue();
                    if ( nKerning > 0 )
                    {
                        nKerning *= nX;
                        nKerning /= 100;
                    }
                    else if ( nKerning < 0 )
                    {
                        // Bei Negativen Werten:
                        // Bei Stretching > 100 muessen die Werte kleiner werden und umgekehrt.
                        nKerning *= 100;
                        nKerning /= nX;
                    }
                    pNewKerning->SetValue( (short)nKerning );
                    pNew = pNewKerning;
                }
                if ( pNew )
                {
                    SfxItemSet aTmpSet( GetEmptyItemSet() );
                    aTmpSet.Put( *pNew );
                    SetAttribs( EditSelection( EditPaM( pNode, pAttr->GetStart() ),
                        EditPaM( pNode, pAttr->GetEnd() ) ), aTmpSet );

                    nLastEnd = pAttr->GetEnd();
                    delete pNew;
                }
            }
        }
    }
    UndoActionEnd( EDITUNDO_STRETCH );
}

const SvxLRSpaceItem& ImpEditEngine::GetLRSpaceItem( ContentNode* pNode )
{
    // Wenn Outliner1, dann immer OUTLLR
    // Wenn Outliner2, dann nur, wenn kein LRSpace
    if ( aStatus.IsOutliner() ||
         ( aStatus.IsOutliner2() && !pNode->GetContentAttribs().HasItem( EE_PARA_LRSPACE ) ) )
    {
        return (const SvxLRSpaceItem&)pNode->GetContentAttribs().GetItem( EE_PARA_OUTLLRSPACE );
    }
    return (const SvxLRSpaceItem&)pNode->GetContentAttribs().GetItem( EE_PARA_LRSPACE );
}


Reference < i18n::XBreakIterator > ImpEditEngine::ImplGetBreakIterator()
{
    if ( !xBI.is() )
    {
        Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        Reference < XInterface > xI = xMSF->createInstance( OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
        if ( xI.is() )
        {
            Any x = xI->queryInterface( ::getCppuType((const Reference< i18n::XBreakIterator >*)0) );
            x >>= xBI;
        }
    }
    return xBI;
}
