/*************************************************************************
 *
 *  $RCSfile: poolfmt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-23 11:57:39 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

const USHORT PT_3   =  3 * 20;      //  3 pt
const USHORT PT_6   =  6 * 20;      //  6 pt
const USHORT PT_7   =  7 * 20;      //  6 pt
const USHORT PT_8   =  8 * 20;      //  8 pt
const USHORT PT_9   =  9 * 20;      //  9 pt
const USHORT PT_10  = 10 * 20;      // 10 pt
const USHORT PT_11  = 11 * 20;      // 11 pt
const USHORT PT_12  = 12 * 20;      // 12 pt
const USHORT PT_14  = 14 * 20;      // 14 pt
const USHORT PT_16  = 16 * 20;      // 16 pt
const USHORT PT_18  = 18 * 20;      // 18 pt
const USHORT PT_22  = 22 * 20;      // 22 pt
const USHORT PT_24  = 24 * 20;      // 22 pt

#define CM_1  0         // 1 centimeter     or 1/2 inch
#define CM_05 1         // 0.5 centimeter   or 1/4 inch
#define CM_01 2         // 0.1 centimeter   or 1/20 inch

inline USHORT GetMetricVal( int n )
{
#ifdef USE_MEASUREMENT
    USHORT nVal = MEASURE_METRIC == Application::GetAppInternational().
                                        GetMeasurementSystem()
                    ? 567       // 1 cm
                    : 770;      // 1/2 Inch
#else
    USHORT nVal = 567;      // 1 cm
#endif

    if( CM_01 == n )
        nVal /= 10;
    else if( CM_05 == n )
        nVal /= 2;
    return nVal;
}

//const USHORT HTML_PARSPACE = ((CM_05 * 7) / 10);
#define HTML_PARSPACE   GetMetricVal( CM_05 )

static const sal_Char __FAR_DATA sKomma[] = ", ";

static const USHORT aHeadlineSizes[ 2 * MAXLEVEL ] = {
//  PT_16, PT_14, PT_14, PT_12, PT_12,          // normal
//JP 10.12.96: jetzt soll alles prozentual sein:
    115, 100, 100, 85, 85,
    75,   75,  75, 75, 75,  // normal
//  PT_22, PT_16, PT_12, PT_11, PT_9            // HTML-Mode
    PT_24, PT_18, PT_14, PT_12, PT_10,
    PT_7, PT_7, PT_7, PT_7, PT_7            // HTML-Mode
};

// Array fuer alle Pool-Vorlagen-Name:
SvStringsDtor   *SwDoc::pTextNmArray = 0,
                *SwDoc::pListsNmArray = 0,
                *SwDoc::pExtraNmArray = 0,
                *SwDoc::pRegisterNmArray = 0,
                *SwDoc::pDocNmArray = 0,
                *SwDoc::pHTMLNmArray = 0,
                *SwDoc::pFrmFmtNmArray = 0,
                *SwDoc::pChrFmtNmArray = 0,
                *SwDoc::pHTMLChrFmtNmArray = 0,
                *SwDoc::pPageDescNmArray = 0,
                *SwDoc::pNumRuleNmArray = 0;




long lcl_GetRightMargin( SwDoc& rDoc )
{
    // sorge dafuer, dass die Druckereinstellungen in die Standard-
    // Seitenvorlage uebernommen wurden.
    long nLeft, nRight, nWidth;
    const SfxPrinter* pPrt = rDoc.GetPrt( !rDoc.IsInReading() );
    if( !pPrt )
    {
        SvxPaper ePaper;
// JP 26.05.99: so gibts einen GPF -> warum???
//      if( MEASURE_METRIC == Application::GetAppInternational().
//                                  GetMeasurementSystem() )
        International aInter;
        if( MEASURE_METRIC == aInter.GetMeasurementSystem() )
        {
            ePaper = SVX_PAPER_A4;
            nLeft = nRight = 1134;  //2 Zentimeter
        }
        else
        {
            ePaper = SVX_PAPER_LEGAL;
            nLeft = nRight = 1800;  // 1,25 Inch
        }

        nWidth = SvxPaperInfo::GetPaperSize( ePaper ).Width();
    }
    else
    {
        const SwFrmFmt& rPgDscFmt = rDoc.GetPageDesc( 0 ).GetMaster();
        const SvxLRSpaceItem& rLR = rPgDscFmt.GetLRSpace();
        nLeft = rLR.GetLeft();
        nRight = rLR.GetRight();
        nWidth = rPgDscFmt.GetFrmSize().GetWidth();
    }
    return nWidth - nLeft - nRight;
}

void lcl_SetHeadline( SwDoc* pDoc, SwTxtFmtColl* pColl,
                        SfxItemSet& rSet,
                        USHORT nOutLvlBits, BYTE nLevel, BOOL bItalic )
{
    rSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
    if( pDoc->IsHTMLMode() )
        rSet.Put( SvxFontHeightItem( aHeadlineSizes[ MAXLEVEL + nLevel ] ));
    else
        rSet.Put( SvxFontHeightItem( PT_14, aHeadlineSizes[ nLevel ] ));

    if( bItalic && !pDoc->IsHTMLMode() )
        rSet.Put( SvxPostureItem( ITALIC_NORMAL ) );

    if( pDoc->IsHTMLMode() )
    {
        rSet.Put( SvxFontItem( FAMILY_ROMAN, String::CreateFromAscii(
#if defined(OW) || defined (MTF) || defined(UNX)
                            RTL_CONSTASCII_STRINGPARAM("times")
#elif defined(WNT) || defined(WIN) || defined (PM20)
                            RTL_CONSTASCII_STRINGPARAM("Times New Roman")
#elif defined(MAC)
                            RTL_CONSTASCII_STRINGPARAM("Times")
#else
#error Defaultfont fuer diese Plattform?
#endif
                            ), aEmptyStr, PITCH_VARIABLE,
/*?? GetSystemCharSet() ->*/    ::gsl_getSystemTextEncoding() ));   // Font
    }

    if( pColl )
    {
        if( !( nOutLvlBits & ( 1 << nLevel )) )
        {
            pColl->SetOutlineLevel( nLevel );
            if( !pDoc->IsHTMLMode() )
            {
                const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get( nLevel );
                if( rNFmt.GetAbsLSpace() || rNFmt.GetFirstLineOffset() )
                {
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)pColl->GetAttr( RES_LR_SPACE ) );
                    aLR.SetTxtFirstLineOfstValue( rNFmt.GetFirstLineOffset() );
                    aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );
                    pColl->SetAttr( aLR );
                }
            }
        }
        pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool(
                                        RES_POOLCOLL_TEXT ));
    }
}



void lcl_SetRegister( SwDoc* pDoc, SfxItemSet& rSet, USHORT nFact,
                        BOOL bHeader, BOOL bTab )
{
    SvxLRSpaceItem aLR;
    USHORT nLeft = nFact ? GetMetricVal( CM_05 ) * nFact : 0;
    aLR.SetTxtLeft( nLeft );

    rSet.Put( aLR );
    if( bHeader )
    {
        rSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
        rSet.Put( SvxFontHeightItem( PT_16 ) );
    }
    if( bTab )
    {
        long nRightMargin = lcl_GetRightMargin( *pDoc );
        SvxTabStopItem aTStops( 0, 0 );
        aTStops.Insert( SvxTabStop( nRightMargin - nLeft,
                                    SVX_TAB_ADJUST_RIGHT,
                                    cDfltDecimalChar, '.' ));
        rSet.Put( aTStops );
    }
}



void lcl_SetNumBul( SwDoc* pDoc, SwTxtFmtColl* pColl,
                        SfxItemSet& rSet,
                        USHORT nNxt, SwTwips nEZ, SwTwips nLeft,
                        SwTwips nUpper, SwTwips nLower )
{

    SvxLRSpaceItem aLR; SvxULSpaceItem aUL;
    aLR.SetTxtFirstLineOfst( USHORT(nEZ) ); aLR.SetTxtLeft( USHORT(nLeft) );
    aUL.SetUpper( USHORT(nUpper) ); aUL.SetLower( USHORT(nLower) );
    rSet.Put( aLR );
    rSet.Put( aUL );

    if( !pColl )
        pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool( nNxt ));
}



    // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
    // sie noch nicht, dann erzeuge sie
    // Ist der String-Pointer definiert, dann erfrage nur die
    // Beschreibung der Attribute, !! es legt keine Vorlage an !!

SwTxtFmtColl* SwDoc::GetTxtCollFromPool( USHORT nId, String* pDesc,
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric )
{
    ASSERT(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Falsche AutoFormat-Id" );

    SwTxtFmtColl* pNewColl;
    USHORT nOutLvlBits = 0;
    for( USHORT n = 0; n < pTxtFmtCollTbl->Count(); ++n )
    {
        if( nId == ( pNewColl = (*pTxtFmtCollTbl)[ n ] )->GetPoolFmtId() )
        {
            if( pDesc )
                pNewColl->GetPresentation( ePres, eCoreMetric,
                                           ePresMetric, *pDesc );
            return pNewColl;
        }
        if( !pDesc && pNewColl->GetOutlineLevel() < MAXLEVEL )
            nOutLvlBits |= ( 1 << pNewColl->GetOutlineLevel() );
    }

    // bis hierher nicht gefunden -> neu anlegen
    USHORT nResId = 0;
    if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        nResId = RC_POOLCOLL_TEXT_BEGIN - RES_POOLCOLL_TEXT_BEGIN;
    else if (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END)
        nResId = RC_POOLCOLL_LISTS_BEGIN - RES_POOLCOLL_LISTS_BEGIN;
    else if (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END)
        nResId = RC_POOLCOLL_EXTRA_BEGIN - RES_POOLCOLL_EXTRA_BEGIN;
    else if (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END)
        nResId = RC_POOLCOLL_REGISTER_BEGIN - RES_POOLCOLL_REGISTER_BEGIN;
    else if (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END)
        nResId = RC_POOLCOLL_DOC_BEGIN - RES_POOLCOLL_DOC_BEGIN;
    else if (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END)
        nResId = RC_POOLCOLL_HTML_BEGIN - RES_POOLCOLL_HTML_BEGIN;

    ASSERT( nResId, "Ungueltige Pool-ID" );
    if( !nResId )
        return GetTxtCollFromPool( RES_POOLCOLL_STANDARD, pDesc, ePres,
                                    eCoreMetric, ePresMetric );

    ResId aResId( nResId + nId, pSwResMgr );
    String aNm( aResId );

    // ein Set fuer alle zusetzenden Attribute
    SwAttrSet aSet( GetAttrPool(), aTxtFmtCollSetRange );
    USHORT nParent = GetPoolParent( nId );
    if( pDesc )
    {
        pNewColl = 0;
        if( nParent )
            *pDesc = SW_RESSTR( nResId + nParent );
        else
            *pDesc = aEmptyStr;
    }
    else
    {

//FEATURE::CONDCOLL
        if(::IsConditionalByPoolId( nId ))
            pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), aNm, !nParent
                                                ? pDfltTxtFmtColl
                                                : GetTxtCollFromPool( nParent ));
        else
//FEATURE::CONDCOLL
        pNewColl = new SwTxtFmtColl( GetAttrPool(), aNm, !nParent
                                            ? pDfltTxtFmtColl
                                            : GetTxtCollFromPool( nParent ));
        pNewColl->SetPoolFmtId( nId );
        pTxtFmtCollTbl->Insert( pNewColl, pTxtFmtCollTbl->Count() );
    }

    switch( nId )
    {
    // allgemeine Inhaltsformen
    case RES_POOLCOLL_STANDARD:
        break;

    case RES_POOLCOLL_TEXT:                 // Textkoerper
        {
            SvxULSpaceItem aUL( 0, PT_6 );
            if( IsHTMLMode() ) aUL.SetLower( HTML_PARSPACE );
            aSet.Put( aUL );
        }
        break;
    case RES_POOLCOLL_TEXT_IDENT:           // Textkoerper Einzug
        {
            SvxLRSpaceItem aLR;
            aLR.SetTxtFirstLineOfst( GetMetricVal( CM_05 ));
            aSet.Put( aLR );
        }
        break;
    case RES_POOLCOLL_TEXT_NEGIDENT:        // Textkoerper neg. Einzug
        {
            SvxLRSpaceItem aLR;
            aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
            aLR.SetTxtLeft( GetMetricVal( CM_1 ));
            SvxTabStopItem aTStops;     aTStops.Insert( SvxTabStop( 0 ));

            aSet.Put( aLR );
            aSet.Put( aTStops );
        }
        break;
    case RES_POOLCOLL_TEXT_MOVE:            // Textkoerper Einrueckung
        {
            SvxLRSpaceItem aLR;
            aLR.SetTxtLeft( GetMetricVal( CM_05 ));
            aSet.Put( aLR );
        }
        break;

    case RES_POOLCOLL_CONFRONTATION:    // Textkoerper Gegenueberstellung
        {
            SvxLRSpaceItem aLR;
            aLR.SetTxtFirstLineOfst( - short( GetMetricVal( CM_1 ) * 4 +
                                              GetMetricVal( CM_05)) );
            aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 5 );
            SvxTabStopItem aTStops;     aTStops.Insert( SvxTabStop( 0 ));

            aSet.Put( aLR );
            aSet.Put( aTStops );
        }
        break;
    case RES_POOLCOLL_MARGINAL:         // Textkoerper maginalie
        {
            SvxLRSpaceItem aLR;
            aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 4 );
            aSet.Put( aLR );
        }
        break;

    case RES_POOLCOLL_HEADLINE_BASE:            // Basis Ueberschrift
        {
            SvxFontItem aFont( FAMILY_SWISS, String::CreateFromAscii(
#if defined(OW) || defined (MTF) || defined(UNX)
                    RTL_CONSTASCII_STRINGPARAM("helvetica")
#elif defined(WNT) || defined(WIN)
                    RTL_CONSTASCII_STRINGPARAM("Arial")
#elif defined(MAC) || defined(PM20)
                    RTL_CONSTASCII_STRINGPARAM("Helvetica")
#else
#error Defaultfont fuer diese Plattform?
#endif
                    ), aEmptyStr, PITCH_VARIABLE,
/*?? GetSystemCharSet() ->*/    ::gsl_getSystemTextEncoding()); // Font

            SvxFontHeightItem aFntSize( PT_14 );
            SvxULSpaceItem aUL( PT_12, PT_6 );
            if( IsHTMLMode() ) aUL.SetLower( HTML_PARSPACE );
            aSet.Put( SvxFmtKeepItem( TRUE ));

            if( !pDesc )
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_TEXT ));

            aSet.Put( aUL );
            aSet.Put( aFntSize );
            aSet.Put( aFont );
        }
        break;

    case RES_POOLCOLL_NUMBUL_BASE:          // Basis Numerierung/Aufzaehlung
        break;

    case RES_POOLCOLL_GREETING:             // Grussformel
    case RES_POOLCOLL_REGISTER_BASE:        // Basis Verzeichnisse
    case RES_POOLCOLL_SIGNATURE:            // Unterschrift
    case RES_POOLCOLL_TABLE:                // Tabelle-Inhalt
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_HEADLINE1:        // Ueberschrift 1
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 0, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE2:        // Ueberschrift 2
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 1, TRUE );
        break;
    case RES_POOLCOLL_HEADLINE3:        // Ueberschrift 3
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 2, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE4:        // Ueberschrift 4
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 3, TRUE );
        break;
    case RES_POOLCOLL_HEADLINE5:        // Ueberschrift 5
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 4, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE6:        // Ueberschrift 6
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 5, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE7:        // Ueberschrift 7
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 6, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE8:        // Ueberschrift 8
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 7, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE9:        // Ueberschrift 9
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 8, FALSE );
        break;
    case RES_POOLCOLL_HEADLINE10:       // Ueberschrift 10
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 9, FALSE );
        break;


    // Sonderbereiche:
    // Kopfzeilen
    case RES_POOLCOLL_HEADER:
    case RES_POOLCOLL_HEADERL:
    case RES_POOLCOLL_HEADERR:
    // Fusszeilen
    case RES_POOLCOLL_FOOTER:
    case RES_POOLCOLL_FOOTERL:
    case RES_POOLCOLL_FOOTERR:
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );

            long nRightMargin = lcl_GetRightMargin( *this );

            SvxTabStopItem aTStops( 0, 0 );
            aTStops.Insert( SvxTabStop( nRightMargin / 2, SVX_TAB_ADJUST_CENTER ) );
            aTStops.Insert( SvxTabStop( nRightMargin, SVX_TAB_ADJUST_RIGHT ) );

            aSet.Put( aTStops );
        }
        break;

    case RES_POOLCOLL_TABLE_HDLN:
        {
            aSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
            if( !IsHTMLMode() )
                aSet.Put( SvxPostureItem( ITALIC_NORMAL ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_FOOTNOTE:             // Fussnote
    case RES_POOLCOLL_ENDNOTE:
        {
            SvxLRSpaceItem aLR;
            aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
            aLR.SetTxtLeft( GetMetricVal( CM_05 ));
            aSet.Put( SvxFontHeightItem( PT_10 ) );
            aSet.Put( aLR );
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_LABEL:                // Beschriftung-Basis
        {
            SvxULSpaceItem aUL; aUL.SetUpper( PT_6 ); aUL.SetLower( PT_6 );
            aSet.Put( aUL );
            aSet.Put( SvxPostureItem( ITALIC_NORMAL ) );
            aSet.Put( SvxFontHeightItem( PT_10 ) );
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_FRAME:                // Rahmen Inhalt
    case RES_POOLCOLL_LABEL_ABB:            // Beschriftung-Abbildung
    case RES_POOLCOLL_LABEL_TABLE:          // Beschriftung-Tabelle
    case RES_POOLCOLL_LABEL_FRAME:          // Beschriftung-Rahmen
    case RES_POOLCOLL_LABEL_DRAWING:        // Beschriftung-Zeichnung
        break;

    case RES_POOLCOLL_JAKETADRESS:          // UmschlagAdresse
        {
            SvxULSpaceItem aUL; aUL.SetLower( PT_3 );
            aSet.Put( aUL );
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_SENDADRESS:           // AbsenderAdresse
        {
            if( IsHTMLMode() )
                aSet.Put( SvxPostureItem(ITALIC_NORMAL) );
            else
            {
                SvxULSpaceItem aUL; aUL.SetLower( PT_3 );
                aSet.Put( aUL );
            }
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    // Benutzer-Verzeichnisse:
    case RES_POOLCOLL_TOX_USERH:            // Header
        lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_USER1:            // 1. Ebene
        lcl_SetRegister( this, aSet, 0, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER2:            // 2. Ebene
        lcl_SetRegister( this, aSet, 1, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER3:            // 3. Ebene
        lcl_SetRegister( this, aSet, 2, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER4:            // 4. Ebene
        lcl_SetRegister( this, aSet, 3, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER5:            // 5. Ebene
        lcl_SetRegister( this, aSet, 4, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER6:            // 6. Ebene
        lcl_SetRegister( this, aSet, 5, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER7:            // 7. Ebene
        lcl_SetRegister( this, aSet, 6, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER8:            // 8. Ebene
        lcl_SetRegister( this, aSet, 7, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER9:            // 9. Ebene
        lcl_SetRegister( this, aSet, 8, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_USER10:           // 10. Ebene
        lcl_SetRegister( this, aSet, 9, FALSE, TRUE );
        break;

    // Index-Verzeichnisse
    case RES_POOLCOLL_TOX_IDXH:         // Header
        lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_IDX1:         // 1. Ebene
        lcl_SetRegister( this, aSet, 0, FALSE, FALSE );
        break;
    case RES_POOLCOLL_TOX_IDX2:         // 2. Ebene
        lcl_SetRegister( this, aSet, 1, FALSE, FALSE );
        break;
    case RES_POOLCOLL_TOX_IDX3:         // 3. Ebene
        lcl_SetRegister( this, aSet, 2, FALSE, FALSE );
        break;
    case RES_POOLCOLL_TOX_IDXBREAK:     // Trenner
        lcl_SetRegister( this, aSet, 0, FALSE, FALSE );
        break;

    // Inhalts-Verzeichnisse
    case RES_POOLCOLL_TOX_CNTNTH:       // Header
        lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_CNTNT1:       // 1. Ebene
        lcl_SetRegister( this, aSet, 0, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT2:       // 2. Ebene
        lcl_SetRegister( this, aSet, 1, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT3:       // 3. Ebene
        lcl_SetRegister( this, aSet, 2, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT4:       // 4. Ebene
        lcl_SetRegister( this, aSet, 3, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT5:       // 5. Ebene
        lcl_SetRegister( this, aSet, 4, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT6:       // 6. Ebene
        lcl_SetRegister( this, aSet, 5, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT7:       // 7. Ebene
        lcl_SetRegister( this, aSet, 6, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT8:       // 8. Ebene
        lcl_SetRegister( this, aSet, 7, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT9:       // 9. Ebene
        lcl_SetRegister( this, aSet, 8, FALSE, TRUE );
        break;
    case RES_POOLCOLL_TOX_CNTNT10:      // 10. Ebene
        lcl_SetRegister( this, aSet, 9, FALSE, TRUE );
        break;

    case RES_POOLCOLL_TOX_ILLUSH:
    case RES_POOLCOLL_TOX_OBJECTH:
    case RES_POOLCOLL_TOX_TABLESH:
    case RES_POOLCOLL_TOX_AUTHORITIESH:
        lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_ILLUS1:
    case RES_POOLCOLL_TOX_OBJECT1:
    case RES_POOLCOLL_TOX_TABLES1:
    case RES_POOLCOLL_TOX_AUTHORITIES1:
        lcl_SetRegister( this, aSet, 0, FALSE, TRUE );
    break;



    case RES_POOLCOLL_NUM_LEVEL1S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL1E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM1,
                        0, SwNumRule::GetNumIndent( 0 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL2S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL2E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM2,
                        0, SwNumRule::GetNumIndent( 1 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL3S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL3E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM3,
                        0, SwNumRule::GetNumIndent( 2 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL4S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL4E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM4,
                        0, SwNumRule::GetNumIndent( 3 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL5S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL5E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM5,
                        0, SwNumRule::GetNumIndent( 4 ), 0, PT_6 );
        break;

    case RES_POOLCOLL_BUL_LEVEL1S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL1E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM1,
                        0, SwNumRule::GetBullIndent( 0 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL2S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL2E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM2,
                        0, SwNumRule::GetBullIndent( 1 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL3S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL3E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM3,
                        0, SwNumRule::GetBullIndent( 2 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL4S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL4E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM4,
                        0, SwNumRule::GetBullIndent( 3 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL5S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL5E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM5,
                        0, SwNumRule::GetBullIndent( 4 ), 0, PT_6 );
        break;

    case RES_POOLCOLL_DOC_TITEL:            // Doc. Titel
        {
            aSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
            aSet.Put( SvxFontHeightItem( PT_18 ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );

            if( !pDesc )
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_DOC_SUBTITEL ));
        }
        break;

    case RES_POOLCOLL_DOC_SUBTITEL:         // Doc. UnterTitel
        {
            aSet.Put( SvxPostureItem( ITALIC_NORMAL ));
            aSet.Put( SvxFontHeightItem( PT_14 ));
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ));

            if( !pDesc )
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_TEXT ));
        }
        break;

    case RES_POOLCOLL_HTML_BLOCKQUOTE:
        {
            SvxLRSpaceItem aLR;
            aLR.SetLeft( GetMetricVal( CM_1 ));
            aLR.SetRight( GetMetricVal( CM_1 ));
            aSet.Put( aLR );
//          aSet.Put( SvxAdjustItem( SVX_ADJUST_BLOCK ) );
            SvxULSpaceItem aUL;
            if( !pDesc )
                aUL = pNewColl->GetULSpace();
            aUL.SetLower( HTML_PARSPACE );
            aSet.Put( aUL);
        }
        break;

    case RES_POOLCOLL_HTML_PRE:
        {
            aSet.Put( SvxFontItem( FAMILY_ROMAN,
                        System::GetStandardFont( STDFONT_FIXED ).GetName(),
                        aEmptyStr,PITCH_FIXED,
/*?? GetSystemCharSet() ->*/    ::gsl_getSystemTextEncoding() ));   // Font

// WORKAROUND: PRE auf 10pt setzten
            aSet.Put( SvxFontHeightItem(PT_10) );
// WORKAROUND: PRE auf 10pt setzten

            // der untere Absatz-Abstand wird explizit gesetzt (macht
            // die harte Attributierung einfacher)
            SvxULSpaceItem aULSpaceItem;
            if( !pDesc )
                aULSpaceItem = pNewColl->GetULSpace();
            aULSpaceItem.SetLower( 0 );
            aSet.Put( aULSpaceItem );
        }
        break;

    case RES_POOLCOLL_HTML_HR:
        {
            SvxBoxItem aBox;
            Color aColor( COL_GRAY );
            SvxBorderLine aNew( &aColor, DEF_DOUBLE_LINE0_OUT,
                                         DEF_DOUBLE_LINE0_IN,
                                         DEF_DOUBLE_LINE0_DIST );
            aBox.SetLine( &aNew, BOX_LINE_BOTTOM );

            aSet.Put( aBox );
            aSet.Put( SvxFontHeightItem(120) );

            SvxULSpaceItem aUL;
            if( !pDesc )
            {
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_TEXT ));
                aUL = pNewColl->GetULSpace();
            }
            aUL.SetLower( HTML_PARSPACE );
            aSet.Put( aUL);
            SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_HTML_DD:
        {
            SvxLRSpaceItem aLR;
            if( !pDesc )
                aLR = pNewColl->GetLRSpace();
            // es wird um 1cm eingerueckt. Die IDs liegen immer 2 auseinander!
            aLR.SetLeft( GetMetricVal( CM_1 ));
            aSet.Put( aLR );
        }
        break;
    case RES_POOLCOLL_HTML_DT:
        {
            SvxLRSpaceItem aLR;
            if( !pDesc )
            {
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                    RES_POOLCOLL_HTML_DD ));
                aLR = pNewColl->GetLRSpace();
            }
            // es wird um 0cm eingerueckt. Die IDs liegen immer 2 auseinander!
            aLR.SetLeft( 0 );
            aSet.Put( aLR );
        }
        break;
    }

    if( aSet.Count() )
    {
        if( pDesc )
        {
            String aStr;
            aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
            pDesc->AppendAscii( sKomma );
            *pDesc += aStr;
        }
        else
        {
            pNewColl->SetAttr( aSet );
            // JP 31.08.95: erzeugen einer PoolVorlage ist keine Modifikation
            //              (Bug: 18545)
            // SetModified();
        }
    }
    return pNewColl;
}



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
BOOL SwDoc::IsPoolTxtCollUsed( USHORT nId ) const
{
    ASSERT(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Falsche AutoFormat-Id" );

    SwTxtFmtColl* pNewColl;
    BOOL bFnd = FALSE;
    for( USHORT n = 0; !bFnd && n < pTxtFmtCollTbl->Count(); ++n )
        if( nId == ( pNewColl = (*pTxtFmtCollTbl)[ n ] )->GetPoolFmtId() )
            bFnd = TRUE;

    if( !bFnd || !pNewColl->GetDepends() )
        return FALSE;

    SwAutoFmtGetDocNode aGetHt( &aNodes );
    return !pNewColl->GetInfo( aGetHt );
}



USHORT lcl_CheckAutoFmtId( USHORT nId )
{
    if( RES_POOLCHR_BEGIN <= nId && nId < RES_POOLCHR_END )
        return RES_POOL_CHRFMT;
    if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
        return RES_POOL_FRMFMT;
    if( (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END) )
        return RES_POOL_TXTCOLL;

    ASSERT( FALSE, "ungueltige Id" );
    return 0;
}


    // Gebe das "Auto[matische]-Format" mit der Id zurueck. Existiert
    // es noch nicht, dann erzeuge es

typedef SwFmt* (SwDoc::*FnMakeFmt)( const String &, SwFmt * );
SwFmt* SwDoc::GetFmtFromPool( USHORT nId, String* pDesc,
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric )
{
    SwFmt *pNewFmt, *pDeriveFmt;

    SvPtrarr* pArray[ 2 ];
    USHORT nArrCnt = 1, nRCId = 0;
    FnMakeFmt fnMkFmt;
    USHORT* pWhichRange = 0;

    switch( nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
    {
    case POOLGRP_CHARFMT:
        {
            pArray[0] = pCharFmtTbl;
            pDeriveFmt = pDfltCharFmt;
            fnMkFmt= (FnMakeFmt)&SwDoc::MakeCharFmt;

            if( nId > RES_POOLCHR_NORMAL_END )
                nRCId = RC_POOLCHRFMT_HTML_BEGIN - RES_POOLCHR_HTML_BEGIN;
            else
                nRCId = RC_POOLCHRFMT_BEGIN - RES_POOLCHR_BEGIN;
            pWhichRange =  aCharFmtSetRange;

            // Fehlerfall: unbekanntes Format, aber CharFormat
            //          -> returne das erste
            if( RES_POOLCHR_BEGIN > nId || nId >= RES_POOLCHR_END )
            {
                ASSERT( !this, "ungueltige Id" );
                nId = RES_POOLCHR_BEGIN;
            }
        }
        break;
    case POOLGRP_FRAMEFMT:
        {
            pArray[0] = pFrmFmtTbl;
            pArray[1] = pSpzFrmFmtTbl;
            pDeriveFmt = pDfltFrmFmt;
            fnMkFmt= (FnMakeFmt)&SwDoc::MakeFrmFmt;
            nArrCnt = 2;
            nRCId = RC_POOLFRMFMT_BEGIN - RES_POOLFRM_BEGIN;
            pWhichRange = aFrmFmtSetRange;

            // Fehlerfall: unbekanntes Format, aber FrameFormat
            //          -> returne das erste
            if( RES_POOLFRM_BEGIN > nId || nId >= RES_POOLFRM_END )
            {
                ASSERT( !this, "ungueltige Id" );
                nId = RES_POOLFRM_BEGIN;
            }
        }
        break;

    default:
        // Fehlerfall, unbekanntes Format
        ASSERT( nId, "ungueltige Id" );
        return 0;
    }
    ASSERT( nRCId, "ungueltige Id" );

    while( nArrCnt-- )
        for( USHORT n = 0; n < (*pArray[nArrCnt]).Count(); ++n )
            if( nId == ( pNewFmt = (SwFmt*)(*pArray[ nArrCnt ] )[ n ] )->
                    GetPoolFmtId() )
            {
                if( pDesc )
                    pNewFmt->GetPresentation( ePres, eCoreMetric,
                                              ePresMetric, *pDesc );
                return pNewFmt;
            }

    ResId aResId( nRCId + nId, pSwResMgr );
    String aNm( aResId );
    SwAttrSet aSet( GetAttrPool(), pWhichRange );

    if( pDesc )
    {
        pNewFmt = 0;
//      *pDesc = aEmptyStr; // oder den Namen ?? aNm;
        *pDesc = aNm;
    }
    else
    {
        BOOL bIsModified = IsModified();
        pNewFmt = (this->*fnMkFmt)( aNm, pDeriveFmt );
        if( !bIsModified )
            ResetModified();
        pNewFmt->SetPoolFmtId( nId );
        pNewFmt->SetAuto( FALSE );      // kein Auto-Format
    }

    switch( nId )
    {
    case RES_POOLCHR_FOOTNOTE:              // Fussnote
    case RES_POOLCHR_PAGENO:                // Seiten/Feld
    case RES_POOLCHR_LABEL:                 // Beschriftung
    case RES_POOLCHR_DROPCAPS:              // Initialien
    case RES_POOLCHR_NUM_LEVEL:             // Aufzaehlungszeichen
    case RES_POOLCHR_TOXJUMP:               // Verzeichnissprung
    case RES_POOLCHR_ENDNOTE:               // Endnote
    case RES_POOLCHR_LINENUM:               // Zeilennummerierung
        break;

    case RES_POOLCHR_ENDNOTE_ANCHOR:        // Endnotenanker
    case RES_POOLCHR_FOOTNOTE_ANCHOR:       // Fussnotenanker
        {
            aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, 58 ) );
        }
        break;


    case RES_POOLCHR_BUL_LEVEL:             // Aufzaehlungszeichen
        {
            const Font& rBulletFont = SwNumRule::GetDefBulletFont();
            aSet.Put( SvxFontItem( rBulletFont.GetFamily(),
                        rBulletFont.GetName(), rBulletFont.GetStyleName(),
                        rBulletFont.GetPitch(), rBulletFont.GetCharSet() ));
            aSet.Put( SvxFontHeightItem( PT_9 ));
        }
        break;

    case RES_POOLCHR_INET_NORMAL:
        {
            Color aCol( COL_BLUE );
            aSet.Put( SvxColorItem( aCol ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE ) );
        }
        break;
    case RES_POOLCHR_INET_VISIT:
        {
            Color aCol( COL_RED );
            aSet.Put( SvxColorItem( aCol ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE ) );
        }
        break;
    case RES_POOLCHR_JUMPEDIT:
        {
            Color aCol( COL_CYAN );
            aSet.Put( SvxColorItem( aCol ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_DOTTED ) );
            aSet.Put( SvxCaseMapItem( SVX_CASEMAP_KAPITAELCHEN ) );
        }
        break;

    case RES_POOLCHR_RUBYTEXT:
        {
// ???          aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, 58 ) );
        }
        break;

    case RES_POOLCHR_HTML_EMPHASIS:
    case RES_POOLCHR_HTML_CITIATION:
    case RES_POOLCHR_HTML_VARIABLE:
        {
            aSet.Put( SvxPostureItem( ITALIC_NORMAL ) );
        }
        break;

    case RES_POOLCHR_IDX_MAIN_ENTRY:
    case RES_POOLCHR_HTML_STRONG:
        {
            aSet.Put( SvxWeightItem( WEIGHT_BOLD ));
        }
        break;

    case RES_POOLCHR_HTML_CODE:
    case RES_POOLCHR_HTML_SAMPLE:
    case RES_POOLCHR_HTML_KEYBOARD:
    case RES_POOLCHR_HTML_TELETYPE:
        {
            aSet.Put( SvxFontItem( FAMILY_ROMAN,
                        System::GetStandardFont( STDFONT_FIXED ).GetName(),
                        aEmptyStr,PITCH_FIXED,
/*?? GetSystemCharSet() ->*/    ::gsl_getSystemTextEncoding()));    // Font
        }
        break;

//nichts besonderes
//  case RES_POOLCHR_HTML_DEFINSTANCE:
//          break;


    case RES_POOLFRM_FRAME:
        {
            if ( IsBrowseMode() )
            {
                aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ));
                aSet.Put( SwFmtVertOrient( 0, VERT_LINE_CENTER, PRTAREA ) );
                aSet.Put( SwFmtSurround( SURROUND_NONE ) );
            }
            else
            {
                aSet.Put( SwFmtAnchor( FLY_AT_CNTNT ));
                aSet.Put( SwFmtSurround( SURROUND_PARALLEL ) );
                aSet.Put( SwFmtHoriOrient( 0, HORI_CENTER, PRTAREA ) );
                aSet.Put( SwFmtVertOrient( 0, VERT_TOP, PRTAREA ) );
                Color aCol( COL_BLACK );
                SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
                SvxBoxItem aBox;
                aBox.SetLine( &aLine, BOX_LINE_TOP );
                aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
                aBox.SetLine( &aLine, BOX_LINE_LEFT );
                aBox.SetLine( &aLine, BOX_LINE_RIGHT );
                aBox.SetDistance( 85 );
                aSet.Put( aBox );
                aSet.Put( SvxLRSpaceItem( 114, 114 ) );
                aSet.Put( SvxULSpaceItem( 114, 114 ) );
            }
        }
        break;
    case RES_POOLFRM_GRAPHIC:
    case RES_POOLFRM_OLE:
        {
            aSet.Put( SwFmtAnchor( FLY_AT_CNTNT ));
            aSet.Put( SwFmtHoriOrient( 0, HORI_CENTER, FRAME ));
            aSet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));
            aSet.Put( SwFmtSurround( SURROUND_NONE ));
        }
        break;
    case RES_POOLFRM_FORMEL:
        {
            aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ) );
            aSet.Put( SwFmtVertOrient( 0, VERT_CHAR_CENTER, FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114 ) );
        }
        break;
    case RES_POOLFRM_MARGINAL:
        {
            aSet.Put( SwFmtAnchor( FLY_AT_CNTNT ));
            aSet.Put( SwFmtHoriOrient( 0, HORI_LEFT, FRAME ));
            aSet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));
            aSet.Put( SwFmtSurround( SURROUND_PARALLEL ));
            // Breite 3.5 centimeter vorgegeben, als Hoehe nur den
            // min. Wert benutzen
            aSet.Put( SwFmtFrmSize( ATT_MIN_SIZE,
                    GetMetricVal( CM_1 ) * 3 + GetMetricVal( CM_05 ),
                    MM50 ));
        }
        break;
    case RES_POOLFRM_WATERSIGN:
        {
            aSet.Put( SwFmtAnchor( FLY_PAGE ));
            aSet.Put( SwFmtHoriOrient( 0, HORI_CENTER, FRAME ));
            aSet.Put( SwFmtVertOrient( 0, VERT_CENTER, FRAME ));
            aSet.Put( SvxOpaqueItem( FALSE ));
            aSet.Put( SwFmtSurround( SURROUND_THROUGHT ));
        }
        break;

    case RES_POOLFRM_LABEL:
        {
            aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ) );
            aSet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114 ) );

            SvxProtectItem aProtect;
            aProtect.SetSizeProtect( TRUE );
            aProtect.SetPosProtect( TRUE );
            aSet.Put( aProtect );

            if( !pDesc )
                pNewFmt->SetAutoUpdateFmt( TRUE );
        }
        break;
    }
    if( aSet.Count() )
    {
        if( pDesc )
        {
            String aStr;
            aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
            pDesc->AppendAscii( sKomma );
            *pDesc += aStr;
        }
        else
        {
            pNewFmt->SetAttr( aSet );
            // JP 31.08.95: erzeugen einer PoolVorlage ist keine Modifikation
            //              (Bug: 18545)
            // SetModified();
        }
    }
    return pNewFmt;
}



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
BOOL SwDoc::IsPoolFmtUsed( USHORT nId ) const
{
    SwFmt *pNewFmt;
    const SvPtrarr* pArray[ 2 ];
    USHORT nArrCnt = 1;
    BOOL bFnd = TRUE;

    if( RES_POOLCHR_BEGIN <= nId && nId < RES_POOLCHR_END )
    {
        pArray[0] = pCharFmtTbl;
    }
    if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
    {
        pArray[0] = pFrmFmtTbl;
        pArray[1] = pSpzFrmFmtTbl;
        nArrCnt = 2;
    }
    else
    {
        ASSERT( FALSE, "ungueltige Id" );
        bFnd = FALSE;
    }

    if( bFnd )
    {
        bFnd = FALSE;
        while( nArrCnt-- && !bFnd )
            for( USHORT n = 0; !bFnd && n < (*pArray[nArrCnt]).Count(); ++n )
                if( nId == ( pNewFmt = (SwFmt*)(*pArray[ nArrCnt ] )[ n ] )->
                        GetPoolFmtId() )
                    bFnd = TRUE;
    }

    // nicht gefunden oder keine Abhaengigen ?
    if( bFnd && pNewFmt->GetDepends() )
    {
        // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
        // (auch indirekte fuer Format-Ableitung! )
        SwAutoFmtGetDocNode aGetHt( &aNodes );
        bFnd = !pNewFmt->GetInfo( aGetHt );
    }
    else
        bFnd = FALSE;

    return bFnd;
}



void lcl_GetStdPgSize( SwDoc* pDoc, SfxItemSet& rSet )
{
    SwPageDesc* pStdPgDsc = pDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    SwFmtFrmSize aFrmSz( pStdPgDsc->GetMaster().GetFrmSize() );
    if( pStdPgDsc->GetLandscape() )
    {
        SwTwips nTmp = aFrmSz.GetHeight();
        aFrmSz.SetHeight( aFrmSz.GetWidth() );
        aFrmSz.SetWidth( nTmp );
    }
    rSet.Put( aFrmSz );
}



SwPageDesc* SwDoc::GetPageDescFromPool( USHORT nId, String* pDesc,
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric )
{
    ASSERT( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Falsche AutoFormat-Id" );

    SwPageDesc *pNewPgDsc;
    for( USHORT n = 0; n < aPageDescs.Count(); ++n )
        if( nId == ( pNewPgDsc = aPageDescs[ n ] )->GetPoolFmtId() )
        {
            if( pDesc )
                pNewPgDsc->GetPresentation( ePres, eCoreMetric,
                                            ePresMetric, *pDesc );
            return pNewPgDsc;
        }

    // Fehlerfall: unbekannte Poolvorlage
    if( RES_POOLPAGE_BEGIN > nId ||  nId >= RES_POOLPAGE_END )
    {
        ASSERT( !this, "ungueltige Id" );
        nId = RES_POOLPAGE_BEGIN;
    }

    ResId aResId( RC_POOLPAGEDESC_BEGIN + nId - RES_POOLPAGE_BEGIN, pSwResMgr );
    String aNm( aResId );
    if( pDesc )
    {
        pNewPgDsc = 0;
//      *pDesc = aEmptyStr; // oder den Namen ?? aNm;
        *pDesc = aNm;
    }
    else
    {
        BOOL bIsModified = IsModified();
        n = MakePageDesc( aNm );
        pNewPgDsc = aPageDescs[ n ];
        pNewPgDsc->SetPoolFmtId( nId );
        if( !bIsModified )
            ResetModified();
    }


    SvxLRSpaceItem aLR;
    aLR.SetLeft( GetMetricVal( CM_1 ) * 2 );
    aLR.SetRight( aLR.GetLeft() );
    SvxULSpaceItem aUL;
    aUL.SetUpper( aLR.GetLeft() );
    aUL.SetLower( aLR.GetLeft() );

    SwAttrSet aSet( GetAttrPool(), aPgFrmFmtSetRange );
    BOOL bSetLeft = TRUE;

    switch( nId )
    {
    case RES_POOLPAGE_STANDARD:             // Standard-Seite
        {
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( PD_ALL );
        }
        break;

    case RES_POOLPAGE_FIRST:                // Erste Seite
    case RES_POOLPAGE_REGISTER:             // Verzeichnis
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( PD_ALL );
                if( RES_POOLPAGE_FIRST == nId )
                    pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_STANDARD ));
            }
        }
        break;

    case RES_POOLPAGE_LEFT:                 // Linke Seite
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = FALSE;
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( PD_LEFT );
        }
        break;
    case RES_POOLPAGE_RIGHT:                // Rechte Seite
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = FALSE;
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( PD_RIGHT );
        }
        break;

    case RES_POOLPAGE_JAKET:                // Umschlag
        {
            aLR.SetLeft( 0 ); aLR.SetRight( 0 );
            aUL.SetUpper( 0 ); aUL.SetLower( 0 );
            Size aPSize( SvxPaperInfo::GetPaperSize( SVX_PAPER_C65 ) );
            LandscapeSwap( aPSize );
            aSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPSize.Width(), aPSize.Height() ));
            aSet.Put( aLR );
            aSet.Put( aUL );

            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( PD_ALL );
                pNewPgDsc->SetLandscape( TRUE );
            }
        }
        break;

    case RES_POOLPAGE_HTML:             // HTML
        {
            aLR.SetRight( GetMetricVal( CM_1 ));
            aUL.SetUpper( aLR.GetRight() );
            aUL.SetLower( aLR.GetRight() );
            Size aPSize( SvxPaperInfo::GetPaperSize( SVX_PAPER_A4 ) );
            aSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPSize.Width(), aPSize.Height() ));
            aSet.Put( aLR );
            aSet.Put( aUL );

            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( PD_ALL );
        }
        break;
    case RES_POOLPAGE_FOOTNOTE:
    case RES_POOLPAGE_ENDNOTE:
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( PD_ALL );
            SwPageFtnInfo aInf( pNewPgDsc->GetFtnInfo() );
            aInf.SetLineWidth( 0 );
            aInf.SetTopDist( 0 );
            aInf.SetBottomDist( 0 );
            pNewPgDsc->SetFtnInfo( aInf );
        }
        break;
    }

    if( aSet.Count() )
    {
        if( pDesc )
        {
            String aStr;
            aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
            pDesc->AppendAscii( sKomma );
            *pDesc += aStr;
        }
        else
        {
            if( bSetLeft )
                pNewPgDsc->GetLeft().SetAttr( aSet );
            pNewPgDsc->GetMaster().SetAttr( aSet );
            // JP 31.08.95: erzeugen einer PoolVorlage ist keine Modifikation
            //              (Bug: 18545)
            // SetModified();
        }
    }
    return pNewPgDsc;
}

SwNumRule* SwDoc::GetNumRuleFromPool( USHORT nId, String* pDesc,
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric )
{
    ASSERT( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
            "Falsche AutoFormat-Id" );

    SwNumRule* pNewRule;
    for( USHORT n = 0; n < GetNumRuleTbl().Count(); ++n )
        if( nId == ( pNewRule = GetNumRuleTbl()[ n ] )->GetPoolFmtId() )
        {
            if( pDesc )
                *pDesc = pNewRule->GetName();
            return pNewRule;
        }

    // Fehlerfall: unbekannte Poolvorlage
    if( RES_POOLNUMRULE_BEGIN > nId ||  nId >= RES_POOLNUMRULE_END )
    {
        ASSERT( !this, "ungueltige Id" );
        nId = RES_POOLNUMRULE_BEGIN;
    }

    ResId aResId( RC_POOLNUMRULE_BEGIN + nId - RES_POOLNUMRULE_BEGIN, pSwResMgr );
    String aNm( aResId );

    SwCharFmt *pNumCFmt = 0, *pBullCFmt = 0;

    if( pDesc )
    {
        pNewRule = new SwNumRule( aNm );
        *pDesc = aNm;
    }
    else
    {
        BOOL bIsModified = IsModified();
        n = MakeNumRule( aNm );
        pNewRule = GetNumRuleTbl()[ n ];
        pNewRule->SetPoolFmtId( nId );
        pNewRule->SetAutoRule( FALSE );

        if( RES_POOLNUMRULE_NUM1 <= nId && nId <= RES_POOLNUMRULE_NUM5 )
            pNumCFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );

        if( ( RES_POOLNUMRULE_BUL1 <= nId && nId <= RES_POOLNUMRULE_BUL5 ) ||
            RES_POOLNUMRULE_NUM5 == nId )
            pBullCFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );

        if( !bIsModified )
            ResetModified();
    }

    switch( nId )
    {
    case RES_POOLNUMRULE_NUM1:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_ARABIC;
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetPostfix( aDotStr );

            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif

            aFmt.SetFirstLineOffset( - (*pArr) );
            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                aFmt.SetAbsLSpace( *pArr );
                pNewRule->Set( n, aFmt );
            }
        }
        break;

    case RES_POOLNUMRULE_NUM2:
        {
            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
                    283,  283,  567,  709,      // 0.50, 0.50, 1.00, 1.25
                    850, 1021, 1304, 1474,      // 1.50, 1.80, 2.30, 2.60
                   1588, 1758                   // 2.80, 3.10
                };

#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    385,  385,  770,  963,
                   1155, 1386, 1771, 2002,
                   2156, 2387
                };

            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_ARABIC;
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetUpperLevel( 1 );
            USHORT nSpace = 0;
            for( n = 0; n < MAXLEVEL; ++n )
            {
                aFmt.SetAbsLSpace( nSpace += pArr[ n ] );
                aFmt.SetFirstLineOffset( - pArr[ n ] );
                aFmt.SetStartValue( n+1 );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM3:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_ARABIC;
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetUpperLevel( 1 );
            USHORT nOffs = GetMetricVal( CM_1 ) * 3;
            aFmt.SetFirstLineOffset( - nOffs );

            for( n = 0; n < MAXLEVEL; ++n )
            {
                aFmt.SetAbsLSpace( (n+1) * nOffs );
                aFmt.SetStartValue( n+1 );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM4:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_ROMAN_UPPER;
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetPostfix( aDotStr );

            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif

            aFmt.SetFirstLineOffset( - (*pArr) );
            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                aFmt.SetStartValue( n + 1 );
                aFmt.SetAbsLSpace( *pArr );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM5:
        {
            // [ First, LSpace ]
            static const USHORT aAbsSpace0to2[] =
                {
                    227,  227,      // 0.40, 0.40,
                    369,  624,      // 0.65, 1.10,
                    255,  879       // 0.45, 1.55
                };
            static const USHORT aAbsSpaceInch0to2[] =
                {
                    308,  308,
                    501,  847,
                    347, 1194
                };

#ifdef USE_MEASUREMENT
            const USHORT* pArr0to2 = MEASURE_METRIC == Application::
                            GetAppInternational().GetMeasurementSystem()
                                ? aAbsSpace0to2
                                : aAbsSpaceInch0to2;
#else
            const USHORT* pArr0to2 = aAbsSpace0to2;
#endif
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_ARABIC;
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetPostfix( aDotStr );
            aFmt.SetFirstLineOffset( -pArr0to2[0] );    // == 0.40 cm
            aFmt.SetAbsLSpace( pArr0to2[1] );           // == 0.40 cm

            aFmt.SetCharFmt( pNumCFmt );
            pNewRule->Set( 0, aFmt );

            aFmt.SetUpperLevel( 2 );
            aFmt.SetStartValue( 2 );
            aFmt.SetFirstLineOffset( -pArr0to2[2] );    // == 0.65 cm
            aFmt.SetAbsLSpace( pArr0to2[3] );           // == 1.10 cm
            pNewRule->Set( 1, aFmt );

            aFmt.eType = SVX_NUM_CHARS_LOWER_LETTER;
            aFmt.SetPostfix( ')');
            aFmt.SetUpperLevel( 1 );
            aFmt.SetStartValue( 3 );
            aFmt.SetFirstLineOffset( - pArr0to2[4] );   // == 0.45cm
            aFmt.SetAbsLSpace( pArr0to2[5] );           // == 1.55 cm
            pNewRule->Set( 2, aFmt );


            aFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
            aFmt.SetBulletChar( cBulletChar );
            USHORT nOffs = GetMetricVal( CM_01 ) * 4,
                   nOffs2 = GetMetricVal( CM_1 ) * 2;

            aFmt.SetFirstLineOffset( - nOffs );
            aFmt.SetPostfix( aEmptyStr );
            for( n = 3; n < MAXLEVEL; ++n )
            {
                aFmt.SetStartValue( n+1 );
                aFmt.SetAbsLSpace( nOffs2 + ((n-3) * nOffs) );
                pNewRule->Set( n, aFmt );
            }
        }
        break;

    case RES_POOLNUMRULE_BUL1:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetBulletFont( &SwNumRule::GetDefBulletFont() );
            aFmt.SetBulletChar( cBulletChar );

            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif

            aFmt.SetFirstLineOffset( - (*pArr) );
            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                aFmt.SetAbsLSpace( *pArr );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL2:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
            aFmt.SetBulletChar( 0xF000 + 150 );

            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,3  0,6  0,9  1,2  1,5  1,8   2,1   2,4   2,7   3,0
                    170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
                };
#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
                };
            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif

            aFmt.SetFirstLineOffset( - (*pArr) );
            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                aFmt.SetAbsLSpace( *pArr );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL3:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
            USHORT nOffs = GetMetricVal( CM_01 ) * 4;
            aFmt.SetFirstLineOffset( - nOffs );

            for( n = 0; n < MAXLEVEL; ++n )
            {
                aFmt.SetBulletChar( 0xF000 + ( n & 1 ? 37 : 52 ) );
                aFmt.SetAbsLSpace( ((n & 1) +1) * nOffs );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL4:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );

            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif

            aFmt.SetFirstLineOffset( - (*pArr) );
            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                switch( n )
                {
                case 0:     aFmt.SetBulletChar( 0xF000 + 47 );  break;
                case 1:     aFmt.SetBulletChar( 0xF000 + 48 );  break;
                default:    aFmt.SetBulletChar( 0xF000 + 45 );  break;
                }
                aFmt.SetAbsLSpace( *pArr );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL5:
        {
            SwNumFmt aFmt;
            aFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStartValue( 1 );
            aFmt.SetUpperLevel( 1 );
            aFmt.SetBulletChar( 0xF000 + 79 );
            aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );

            static const USHORT aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
#ifdef USE_MEASUREMENT
            static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
                {
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const USHORT* pArr = MEASURE_METRIC == Application::
                                GetAppInternational().GetMeasurementSystem()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const USHORT* pArr = aAbsSpace;
#endif

            aFmt.SetFirstLineOffset( - (*pArr) );
            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                aFmt.SetAbsLSpace( *pArr );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    }

    if( pDesc )
    {
//JP 25.02.98: wie soll die Beschreibung sein??
//      String aStr;
//      aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
//      *pDesc += sKomma;
//      *pDesc += aStr;

        delete pNewRule, pNewRule = 0;
    }

    return pNewRule;
}



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
BOOL SwDoc::IsPoolPageDescUsed( USHORT nId ) const
{
    ASSERT( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Falsche AutoFormat-Id" );
    SwPageDesc *pNewPgDsc;
    BOOL bFnd = FALSE;
    for( USHORT n = 0; !bFnd && n < aPageDescs.Count(); ++n )
        if( nId == ( pNewPgDsc = aPageDescs[ n ] )->GetPoolFmtId() )
            bFnd = TRUE;

    // nicht gefunden oder keine Abhaengigen ?
    if( !bFnd || !pNewPgDsc->GetDepends() )     // ??????
        return FALSE;

    // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
    // (auch indirekte fuer Format-Ableitung! )
    SwAutoFmtGetDocNode aGetHt( &aNodes );
    return !pNewPgDsc->GetInfo( aGetHt );
}

    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
BOOL SwDoc::IsPoolNumRuleUsed( USHORT nId ) const
{
    ASSERT( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
            "Falsche AutoFormat-Id" );
    SwNumRule *pNewRule;
    BOOL bUsed = FALSE;
    for( USHORT n = 0; n < GetNumRuleTbl().Count(); ++n )
        if( nId == ( pNewRule = GetNumRuleTbl()[ n ] )->GetPoolFmtId() )
        {
            bUsed = IsUsed( *pNewRule );
            break;
        }
    return bUsed;
}



// erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
BOOL SwDoc::IsUsed( const SwModify& rModify ) const
{
    // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
    // (auch indirekte fuer Format-Ableitung! )
    SwAutoFmtGetDocNode aGetHt( &aNodes );
    return !rModify.GetInfo( aGetHt );
}

// erfrage ob die NumRule benutzt wird
BOOL SwDoc::IsUsed( const SwNumRule& rRule ) const
{
    // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
    // (auch indirekte fuer Format-Ableitung! )
    BOOL bUsed = FALSE;
    SwAutoFmtGetDocNode aGetHt( &aNodes );
    SwModify* pMod;
    const SfxPoolItem* pItem;
    USHORT i, nMaxItems = GetAttrPool().GetItemCount( RES_PARATR_NUMRULE);
    for( i = 0; i < nMaxItems; ++i )
        if( 0 != (pItem = GetAttrPool().GetItem( RES_PARATR_NUMRULE, i ) ) &&
            0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn()) &&
            ((SwNumRuleItem*)pItem)->GetValue().Len() &&
            ((SwNumRuleItem*)pItem)->GetValue() == rRule.GetName() )
        {
            if( pMod->IsA( TYPE( SwFmt )) )
            {
                bUsed = !pMod->GetInfo( aGetHt );
                if( bUsed )
                    break;
            }
            else if( ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() )
            {
                bUsed = TRUE;
                break;
            }
        }

    return bUsed;
}


// erfrage zu einer PoolId den Namen
String& GetDocPoolNm( USHORT nId, String& rFillNm )
{
    return SwDoc::GetPoolNm( nId, rFillNm );
}

// erfrage zu einer PoolId den Namen
String& SwDoc::GetPoolNm( USHORT nId, String& rFillNm )
{
    USHORT nRC = 0, nStt = 0, nEnd = 0;
    SvStringsDtor** ppStrArr = 0;

    switch( (USER_FMT | COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID) & nId )
    {
    case COLL_TEXT_BITS:
        if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        {
            ppStrArr = &SwDoc::pTextNmArray;
            nRC = RC_POOLCOLL_TEXT_BEGIN;
            nStt = RES_POOLCOLL_TEXT_BEGIN;
            nEnd = RES_POOLCOLL_TEXT_END;
        }
        break;
    case COLL_LISTS_BITS:
        if( RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END )
        {
            ppStrArr = &SwDoc::pListsNmArray;
            nRC = RC_POOLCOLL_LISTS_BEGIN;
            nStt = RES_POOLCOLL_LISTS_BEGIN;
            nEnd = RES_POOLCOLL_LISTS_END;
        }
        break;
    case COLL_EXTRA_BITS:
        if( RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END )
        {
            ppStrArr = &SwDoc::pExtraNmArray;
            nRC = RC_POOLCOLL_EXTRA_BEGIN;
            nStt = RES_POOLCOLL_EXTRA_BEGIN;
            nEnd = RES_POOLCOLL_EXTRA_END;
        }
        break;
    case COLL_REGISTER_BITS:
        if( RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END )
        {
            ppStrArr = &SwDoc::pRegisterNmArray;
            nRC = RC_POOLCOLL_REGISTER_BEGIN;
            nStt = RES_POOLCOLL_REGISTER_BEGIN;
            nEnd = RES_POOLCOLL_REGISTER_END;
        }
        break;
    case COLL_DOC_BITS:
        if( RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END )
        {
            ppStrArr = &SwDoc::pDocNmArray;
            nRC = RC_POOLCOLL_DOC_BEGIN;
            nStt = RES_POOLCOLL_DOC_BEGIN;
            nEnd = RES_POOLCOLL_DOC_END;
        }
        break;
    case COLL_HTML_BITS:
        if( RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END )
        {
            ppStrArr = &SwDoc::pHTMLNmArray;
            nRC = RC_POOLCOLL_HTML_BEGIN;
            nStt = RES_POOLCOLL_HTML_BEGIN;
            nEnd = RES_POOLCOLL_HTML_END;
        }
        break;
    case POOLGRP_CHARFMT:
        if( RES_POOLCHR_NORMAL_BEGIN <= nId && nId < RES_POOLCHR_NORMAL_END )
        {
            ppStrArr = &SwDoc::pChrFmtNmArray;
            nRC = RC_POOLCHRFMT_BEGIN;
            nStt = RES_POOLCHR_NORMAL_BEGIN;
            nEnd = RES_POOLCHR_NORMAL_END;
        }
        else if( RES_POOLCHR_HTML_BEGIN <= nId && nId < RES_POOLCHR_HTML_END )
        {
            ppStrArr = &SwDoc::pHTMLChrFmtNmArray;
            nRC = RC_POOLCHRFMT_HTML_BEGIN;
            nStt = RES_POOLCHR_HTML_BEGIN;
            nEnd = RES_POOLCHR_HTML_END;
        }
        break;
    case POOLGRP_FRAMEFMT:
        if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
        {
            ppStrArr = &SwDoc::pFrmFmtNmArray;
            nRC = RC_POOLFRMFMT_BEGIN;
            nStt = RES_POOLFRM_BEGIN;
            nEnd = RES_POOLFRM_END;
        }
        break;
    case POOLGRP_PAGEDESC:
        if( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END )
        {
            ppStrArr = &SwDoc::pPageDescNmArray;
            nRC = RC_POOLPAGEDESC_BEGIN;
            nStt = RES_POOLPAGE_BEGIN;
            nEnd = RES_POOLPAGE_END;
        }
        break;
    case POOLGRP_NUMRULE:
        if( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END )
        {
            ppStrArr = &SwDoc::pNumRuleNmArray;
            nRC = RC_POOLNUMRULE_BEGIN;
            nStt = RES_POOLNUMRULE_BEGIN;
            nEnd = RES_POOLNUMRULE_END;
        }
        break;
    }

    if( ppStrArr )
    {
        if( !*ppStrArr )
            SwDoc::NewNmArray( *ppStrArr, nRC, (nRC - nStt + nEnd ));
        rFillNm = * (*ppStrArr)->operator[]( nId - nStt );
    }
//JP 13.04.95: soll der Name geloescht werden?
//              erstmal bleibt er erhalten
//  else
//      rFillNm.Erase();
    return rFillNm;
}




    // loeche die nicht mehr benutzten Pattern-Namen aus dem Array.
    // alle nicht mehr referenzierten Namen werden durch 0-Pointer
    // ersetzt. Diese Positionen koennen wieder vergeben werden.
void SwDoc::ReOrgPatternHelpIds()
{
#if defined( MAC ) || defined( UNX )
    const SvPtrarr* pFmtArray[ 3 ];
    pFmtArray[ 0 ] = pCharFmtTbl;       pFmtArray[ 1 ] = pFrmFmtTbl;
    pFmtArray[ 2 ] = pSpzFrmFmtTbl;
#else
    const SvPtrarr* pFmtArray[ 3 ] = {
        pCharFmtTbl,        pFrmFmtTbl,     pSpzFrmFmtTbl
        };
#endif


    USHORT i, n, nFmt;
    for( n = 0; n < aPatternNms.Count(); ++n )
    {
        BOOL bFnd = FALSE;
        // suche im Absatz-Vorlagen Array
        for( nFmt = 0; !bFnd && nFmt < pTxtFmtCollTbl->Count(); ++nFmt )
            if( n == (*pTxtFmtCollTbl)[ nFmt ]->GetPoolHlpFileId() )
                bFnd = TRUE;

        // suche im Format-Vorlagen Array
        for( i = 0; !bFnd && i < 3; ++i )
        {
            const SvPtrarr* pArr = *(pFmtArray + i);
            for( nFmt = 0; !bFnd && nFmt < pArr->Count(); ++nFmt )
                if( n == ((SwFmt*)(*pArr)[ nFmt ])->GetPoolHlpFileId() )
                    bFnd = TRUE;
        }
        // suche im Format-Vorlagen Array
        for( nFmt = 0; !bFnd && nFmt < aPageDescs.Count(); ++nFmt )
            if( n == aPageDescs[ nFmt ]->GetPoolHlpFileId() )
                bFnd = TRUE;

        if( !bFnd )
        {
            // losche den alten String, setze die Position auf 0
            aPatternNms.DeleteAndDestroy( n+1 );
            String* pNull = 0;
            aPatternNms.Insert( pNull, n );
        }
    }
}



    // Suche die Position vom Vorlagen-Namen. Ist nicht vorhanden
    // dann fuege neu ein
USHORT SwDoc::SetDocPattern( const String& rPatternName )
{
    ASSERT( rPatternName.Len(), "kein Dokument-Vorlagenname" );

    USHORT nNewPos = aPatternNms.Count();
    for( USHORT n = 0; n < aPatternNms.Count(); ++n )
        if( !aPatternNms[n] )
        {
            if( nNewPos == aPatternNms.Count() )
                nNewPos = n;
        }
        else if( rPatternName == *aPatternNms[n] )
            return n;

    if( nNewPos < aPatternNms.Count() )
        aPatternNms.Remove( nNewPos );      // Platz wieder frei machen

    String* pNewNm = new String( rPatternName );
    aPatternNms.Insert( pNewNm, nNewPos );
    SetModified();
    return nNewPos;
}

        // ist der Name ein Pool-Vorlagen-Name, returne seine ID,
        // sonst USHRT_MAX


// falls mit gechacht werden soll:
USHORT lcl_ChkNm( USHORT nStt, USHORT nEnd, USHORT nResStt,
                    const String& rName, SvStringsDtor*& pArray )
{
    USHORT n, i;
    if( !pArray )
    {
        USHORT nFnd = 0;
        pArray = new SvStringsDtor( BYTE(nEnd - nStt), 1 );
        for( n = nStt, i = 0; n < nEnd; ++n, ++i )
        {
            const ResId rRId( nResStt + i, pSwResMgr );
            String* pStr = new String( rRId );
            pArray->Insert( pStr, i );
            if( !nFnd && *pStr == rName )
                nFnd = n;
        }
        return nFnd ? nFnd : USHRT_MAX;
    }

    for( n = nStt, i = 0; n < nEnd; ++n, ++i )
        if( *(*pArray)[i] == rName )
            return n;

    return USHRT_MAX;
}



USHORT SwDoc::GetPoolId( const String& rName,
                        SwGetPoolIdFromName eFlags ) const
{
    USHORT nId;
    if( GET_POOLID_TXTCOLL & eFlags )
    {
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCOLL_TEXT_BEGIN,
                RES_POOLCOLL_TEXT_END, RC_POOLCOLL_TEXT_BEGIN, rName,
                SwDoc::pTextNmArray )))
            return nId;
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCOLL_LISTS_BEGIN,
                RES_POOLCOLL_LISTS_END, RC_POOLCOLL_LISTS_BEGIN, rName,
                SwDoc::pListsNmArray )))
            return nId;
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_EXTRA_END, RC_POOLCOLL_EXTRA_BEGIN, rName,
                SwDoc::pExtraNmArray )))
            return nId;
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCOLL_REGISTER_BEGIN,
                RES_POOLCOLL_REGISTER_END, RC_POOLCOLL_REGISTER_BEGIN, rName,
                SwDoc::pRegisterNmArray )))
            return nId;
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCOLL_DOC_BEGIN,
                RES_POOLCOLL_DOC_END, RC_POOLCOLL_DOC_BEGIN, rName,
                SwDoc::pDocNmArray )))
            return nId;
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCOLL_HTML_BEGIN,
                RES_POOLCOLL_HTML_END, RC_POOLCOLL_HTML_BEGIN, rName,
                SwDoc::pHTMLNmArray )))
            return nId;
    }

    if( GET_POOLID_CHRFMT & eFlags )
    {
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCHR_NORMAL_BEGIN,
                RES_POOLCHR_NORMAL_END, RC_POOLCHRFMT_BEGIN, rName,
                SwDoc::pChrFmtNmArray )))
            return nId;
        if( USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLCHR_HTML_BEGIN,
                RES_POOLCHR_HTML_END, RC_POOLCHRFMT_HTML_BEGIN, rName,
                SwDoc::pHTMLChrFmtNmArray )))
            return nId;
    }
    if( GET_POOLID_FRMFMT & eFlags &&
        USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLFRM_BEGIN,
                RES_POOLFRM_END, RC_POOLFRMFMT_BEGIN, rName,
                SwDoc::pFrmFmtNmArray )))
            return nId;

    if( GET_POOLID_PAGEDESC & eFlags &&
        USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLPAGE_BEGIN,
                RES_POOLPAGE_END, RC_POOLPAGEDESC_BEGIN, rName,
                SwDoc::pPageDescNmArray )))
            return nId;

    if( GET_POOLID_NUMRULE & eFlags &&
        USHRT_MAX != ( nId = lcl_ChkNm( RES_POOLNUMRULE_BEGIN,
                RES_POOLNUMRULE_END, RC_POOLNUMRULE_BEGIN, rName,
                SwDoc::pNumRuleNmArray )))
            return nId;

    return USHRT_MAX;
}





SvStringsDtor* SwDoc::NewNmArray( SvStringsDtor*& pNmArray, USHORT nStt, USHORT nEnd )
{
    if( !pNmArray )
    {
        pNmArray = new SvStringsDtor( BYTE(nEnd - nStt), 1 );
        while( nStt < nEnd )
        {
            const ResId rRId( nStt, pSwResMgr );
            String* pStr = new String( rRId );
            pNmArray->Insert( pStr, pNmArray->Count() );
            ++nStt;
        }
    }
    return pNmArray;
}



SvStringsDtor* SwDoc::NewTextNmArray() const
{
    return NewNmArray( pTextNmArray, RC_POOLCOLL_TEXT_BEGIN,
            ( RC_POOLCOLL_TEXT_BEGIN +
                    (RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN )) );
}



SvStringsDtor* SwDoc::NewListsNmArray() const
{
    return NewNmArray( pListsNmArray, RC_POOLCOLL_LISTS_BEGIN,
            ( RC_POOLCOLL_LISTS_BEGIN +
                    (RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN )) );
}



SvStringsDtor* SwDoc::NewExtraNmArray() const
{
    return NewNmArray( pExtraNmArray, RC_POOLCOLL_EXTRA_BEGIN,
                ( RC_POOLCOLL_EXTRA_BEGIN +
                    (RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN )) );
}



SvStringsDtor* SwDoc::NewRegisterNmArray() const
{
    return NewNmArray( pRegisterNmArray, RC_POOLCOLL_REGISTER_BEGIN,
            ( RC_POOLCOLL_REGISTER_BEGIN +
                (RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN )) );
}



SvStringsDtor* SwDoc::NewDocNmArray() const
{
    return NewNmArray( pDocNmArray, RC_POOLCOLL_DOC_BEGIN,
                    ( RC_POOLCOLL_DOC_BEGIN +
                        (RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN )) );
}



SvStringsDtor* SwDoc::NewHTMLNmArray() const
{
    return NewNmArray( pHTMLNmArray, RC_POOLCOLL_HTML_BEGIN,
                    ( RC_POOLCOLL_HTML_BEGIN +
                        (RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN )) );
}



SvStringsDtor* SwDoc::NewFrmFmtNmArray() const
{
    return NewNmArray( pFrmFmtNmArray, RC_POOLFRMFMT_BEGIN,
                    ( RC_POOLFRMFMT_BEGIN +
                        (RES_POOLFRM_END - RES_POOLFRM_BEGIN )) );
}




SvStringsDtor* SwDoc::NewChrFmtNmArray() const
{
    return NewNmArray( pChrFmtNmArray, RC_POOLCHRFMT_BEGIN,
            ( RC_POOLCHRFMT_BEGIN +
                    (RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN )) );
}



SvStringsDtor* SwDoc::NewHTMLChrFmtNmArray() const
{
    return NewNmArray( pHTMLChrFmtNmArray, RC_POOLCHRFMT_HTML_BEGIN,
            ( RC_POOLCHRFMT_HTML_BEGIN +
                    (RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN )) );
}



SvStringsDtor* SwDoc::NewPageDescNmArray() const
{
    return NewNmArray( pPageDescNmArray, RC_POOLPAGEDESC_BEGIN,
            ( RC_POOLPAGEDESC_BEGIN +
                    (RES_POOLPAGE_END - RES_POOLPAGE_BEGIN )) );
}

SvStringsDtor* SwDoc::NewNumRuleNmArray() const
{
    return NewNmArray( pNumRuleNmArray, RC_POOLNUMRULE_BEGIN,
            ( RC_POOLNUMRULE_BEGIN +
                    (RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN )) );
}



USHORT GetPoolParent( USHORT nId )
{
    USHORT nRet = USHRT_MAX;
    if( POOLGRP_NOCOLLID & nId )        // 1 == Formate / 0 == Collections
    {
        switch( ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID ) & nId )
        {
        case POOLGRP_CHARFMT:
        case POOLGRP_FRAMEFMT:
            nRet = 0;           // vom default abgeleitet
            break;
        case POOLGRP_PAGEDESC:
        case POOLGRP_NUMRULE:
            break;              // es gibt keine Ableitung
        }
    }
    else
    {
        switch( COLL_GET_RANGE_BITS & nId )
        {
        case COLL_TEXT_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_STANDARD:
                    nRet = 0;                                   break;
            case RES_POOLCOLL_TEXT_IDENT:
            case RES_POOLCOLL_TEXT_NEGIDENT:
            case RES_POOLCOLL_TEXT_MOVE:
            case RES_POOLCOLL_CONFRONTATION:
            case RES_POOLCOLL_MARGINAL:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            case RES_POOLCOLL_TEXT:
            case RES_POOLCOLL_GREETING:
            case RES_POOLCOLL_SIGNATURE:
            case RES_POOLCOLL_HEADLINE_BASE:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_HEADLINE1:
            case RES_POOLCOLL_HEADLINE2:
            case RES_POOLCOLL_HEADLINE3:
            case RES_POOLCOLL_HEADLINE4:
            case RES_POOLCOLL_HEADLINE5:
            case RES_POOLCOLL_HEADLINE6:
            case RES_POOLCOLL_HEADLINE7:
            case RES_POOLCOLL_HEADLINE8:
            case RES_POOLCOLL_HEADLINE9:
            case RES_POOLCOLL_HEADLINE10:
                    nRet = RES_POOLCOLL_HEADLINE_BASE;          break;
            }
            break;

        case COLL_LISTS_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_NUMBUL_BASE:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            default:
                nRet = RES_POOLCOLL_NUMBUL_BASE;                break;
            }
            break;

        case COLL_EXTRA_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_FRAME:
            case RES_POOLCOLL_TABLE:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            case RES_POOLCOLL_TABLE_HDLN:
                    nRet = RES_POOLCOLL_TABLE;                  break;

            case RES_POOLCOLL_FOOTNOTE:
            case RES_POOLCOLL_ENDNOTE:
            case RES_POOLCOLL_JAKETADRESS:
            case RES_POOLCOLL_SENDADRESS:
            case RES_POOLCOLL_HEADER:
            case RES_POOLCOLL_HEADERL:
            case RES_POOLCOLL_HEADERR:
            case RES_POOLCOLL_FOOTER:
            case RES_POOLCOLL_FOOTERL:
            case RES_POOLCOLL_FOOTERR:
            case RES_POOLCOLL_LABEL:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_LABEL_ABB:
            case RES_POOLCOLL_LABEL_TABLE:
            case RES_POOLCOLL_LABEL_FRAME:
            case RES_POOLCOLL_LABEL_DRAWING:
                    nRet = RES_POOLCOLL_LABEL;                  break;
            }
            break;

        case COLL_REGISTER_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_REGISTER_BASE:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_TOX_USERH:
            case RES_POOLCOLL_TOX_CNTNTH:
            case RES_POOLCOLL_TOX_IDXH:
            case RES_POOLCOLL_TOX_ILLUSH:
            case RES_POOLCOLL_TOX_OBJECTH:
            case RES_POOLCOLL_TOX_TABLESH:
            case RES_POOLCOLL_TOX_AUTHORITIESH:
                    nRet = RES_POOLCOLL_HEADLINE_BASE;          break;

            default:
                    nRet = RES_POOLCOLL_REGISTER_BASE;          break;
            }
            break;

        case COLL_DOC_BITS:
            nRet = RES_POOLCOLL_HEADLINE_BASE;
            break;

        case COLL_HTML_BITS:
            nRet = RES_POOLCOLL_STANDARD;
            break;
        }
    }

    return nRet;
}


