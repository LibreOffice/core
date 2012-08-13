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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <editeng/adjitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/sfxhtml.hxx>
#include <svtools/parhtml.hxx>
#include <svl/zforlist.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/string.hxx>

#include "eeimport.hxx"
#include "global.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "stlsheet.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "eeparser.hxx"
#include "drwlayer.hxx"
#include "rangenam.hxx"
#include "progress.hxx"
#include "stringutil.hxx"

#include "globstr.hrc"

// in fuins1.cxx
extern void ScLimitSizeOnDrawPage( Size& rSize, Point& rPos, const Size& rPage );

//------------------------------------------------------------------------

ScEEImport::ScEEImport( ScDocument* pDocP, const ScRange& rRange ) :
    maRange( rRange ),
    mpDoc( pDocP ),
    mpParser( NULL )
{
    const ScPatternAttr* pPattern = mpDoc->GetPattern(
        maRange.aStart.Col(), maRange.aStart.Row(), maRange.aStart.Tab() );
    mpEngine = new ScTabEditEngine( *pPattern, mpDoc->GetEditPool() );
    mpEngine->SetUpdateMode( false );
    mpEngine->EnableUndo( false );
}


ScEEImport::~ScEEImport()
{
    // Reihenfolge wichtig, sonst knallt's irgendwann irgendwo in irgendeinem Dtor!
    // Ist gewaehrleistet, da ScEEImport Basisklasse ist
    delete mpEngine;        // nach Parser!
}


sal_uLong ScEEImport::Read( SvStream& rStream, const String& rBaseURL )
{
    sal_uLong nErr = mpParser->Read( rStream, rBaseURL );

    SCCOL nEndCol;
    SCROW nEndRow;
    mpParser->GetDimensions( nEndCol, nEndRow );
    if ( nEndCol != 0 )
    {
        nEndCol += maRange.aStart.Col() - 1;
        if ( nEndCol > MAXCOL )
            nEndCol = MAXCOL;
    }
    else
        nEndCol = maRange.aStart.Col();
    if ( nEndRow != 0 )
    {
        nEndRow += maRange.aStart.Row() - 1;
        if ( nEndRow > MAXROW )
            nEndRow = MAXROW;
    }
    else
        nEndRow = maRange.aStart.Row();
    maRange.aEnd.Set( nEndCol, nEndRow, maRange.aStart.Tab() );

    return nErr;
}


void ScEEImport::WriteToDocument( bool bSizeColsRows, double nOutputFactor, SvNumberFormatter* pFormatter, bool bConvertDate )
{
    ScProgress* pProgress = new ScProgress( mpDoc->GetDocumentShell(),
        ScGlobal::GetRscString( STR_LOAD_DOC ), mpParser->ListSize() );
    sal_uLong nProgress = 0;

    SCCOL nStartCol, nEndCol;
        SCROW nStartRow, nEndRow;
        SCTAB nTab;
        SCROW nOverlapRowMax, nLastMergedRow;
        SCCOL nMergeColAdd;
    nStartCol = maRange.aStart.Col();
    nStartRow = maRange.aStart.Row();
    nTab = maRange.aStart.Tab();
    nEndCol = maRange.aEnd.Col();
    nEndRow = maRange.aEnd.Row();
    nOverlapRowMax = 0;
    nMergeColAdd = 0;
    nLastMergedRow = SCROW_MAX;
    sal_Bool bHasGraphics = false;
    ScEEParseEntry* pE;
    if (!pFormatter)
        pFormatter = mpDoc->GetFormatTable();
    bool bNumbersEnglishUS = false;
    if (pFormatter->GetLanguage() == LANGUAGE_SYSTEM)
    {
        // Automatic language option selected.  Check for the global 'use US English' option.
        SvxHtmlOptions aOpt;
        bNumbersEnglishUS = aOpt.IsNumbersEnglishUS();
    }
    ScDocumentPool* pDocPool = mpDoc->GetPool();
    ScRangeName* pRangeNames = mpDoc->GetRangeName();
    for ( size_t i = 0, n = mpParser->ListSize(); i < n; ++i )
    {
        pE = mpParser->ListEntry( i );
        SCROW nRow = nStartRow + pE->nRow;
        if ( nRow != nLastMergedRow )
            nMergeColAdd = 0;
        SCCOL nCol = nStartCol + pE->nCol + nMergeColAdd;
        // RowMerge feststellen, pures ColMerge und ColMerge der ersten
        // MergeRow bereits beim parsen
        if ( nRow <= nOverlapRowMax )
        {
            while ( nCol <= MAXCOL && mpDoc->HasAttrib( nCol, nRow, nTab,
                nCol, nRow, nTab, HASATTR_OVERLAPPED ) )
            {
                nCol++;
                nMergeColAdd++;
            }
            nLastMergedRow = nRow;
        }
        // fuer zweiten Durchlauf eintragen
        pE->nCol = nCol;
        pE->nRow = nRow;
        if ( ValidCol(nCol) && ValidRow(nRow) )
        {
            SfxItemSet aSet = mpEngine->GetAttribs( pE->aSel );
            // Default raus, wir setzen selber links/rechts je nachdem ob Text
            // oder Zahl; EditView.GetAttribs liefert immer kompletten Set
            // mit Defaults aufgefuellt
            const SfxPoolItem& rItem = aSet.Get( EE_PARA_JUST );
            if ( ((const SvxAdjustItem&)rItem).GetAdjust() == SVX_ADJUST_LEFT )
                aSet.ClearItem( EE_PARA_JUST );

            // Testen, ob einfacher String ohne gemischte Attribute
            sal_Bool bSimple = ( pE->aSel.nStartPara == pE->aSel.nEndPara );
            for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END && bSimple; nId++)
            {
                const SfxPoolItem* pItem = 0;
                SfxItemState eState = aSet.GetItemState( nId, sal_True, &pItem );
                if (eState == SFX_ITEM_DONTCARE)
                    bSimple = false;
                else if (eState == SFX_ITEM_SET)
                {
                    if ( nId == EE_CHAR_ESCAPEMENT )        // Hoch-/Tiefstellen immer ueber EE
                    {
                        if ( (SvxEscapement)((const SvxEscapementItem*)pItem)->GetEnumValue()
                                != SVX_ESCAPEMENT_OFF )
                            bSimple = false;
                    }
                }
            }
            if ( bSimple )
            {   //  Feldbefehle enthalten?
                SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, false );
                if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
                    bSimple = false;
            }

            // HTML
            rtl::OUString aValStr, aNumStr;
            double fVal;
            sal_uInt32 nNumForm = 0;
            LanguageType eNumLang = LANGUAGE_NONE;
            if ( pE->pNumStr )
            {   // SDNUM muss sein wenn SDVAL
                aNumStr = *pE->pNumStr;
                if ( pE->pValStr )
                    aValStr = *pE->pValStr;
                fVal = SfxHTMLParser::GetTableDataOptionsValNum(
                    nNumForm, eNumLang, aValStr, aNumStr, *pFormatter );
            }

            // Attribute setzen
            ScPatternAttr aAttr( pDocPool );
            aAttr.GetFromEditItemSet( &aSet );
            SfxItemSet& rSet = aAttr.GetItemSet();
            if (!aNumStr.isEmpty())
            {
                rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumForm ) );
                rSet.Put( SvxLanguageItem( eNumLang, ATTR_LANGUAGE_FORMAT ) );
            }
            const SfxItemSet& rESet = pE->aItemSet;
            if ( rESet.Count() )
            {
                const SfxPoolItem* pItem;
                if ( rESet.GetItemState( ATTR_BACKGROUND, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_BORDER, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_SHADOW, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                // HTML
                if ( rESet.GetItemState( ATTR_HOR_JUSTIFY, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_VER_JUSTIFY, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_LINEBREAK, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_FONT_COLOR, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_FONT_UNDERLINE, false, &pItem) == SFX_ITEM_SET )
                    rSet.Put( *pItem );
                // HTML LATIN/CJK/CTL script type dependent
                const SfxPoolItem* pFont;
                if ( rESet.GetItemState( ATTR_FONT, false, &pFont) != SFX_ITEM_SET )
                    pFont = 0;
                const SfxPoolItem* pHeight;
                if ( rESet.GetItemState( ATTR_FONT_HEIGHT, false, &pHeight) != SFX_ITEM_SET )
                    pHeight = 0;
                const SfxPoolItem* pWeight;
                if ( rESet.GetItemState( ATTR_FONT_WEIGHT, false, &pWeight) != SFX_ITEM_SET )
                    pWeight = 0;
                const SfxPoolItem* pPosture;
                if ( rESet.GetItemState( ATTR_FONT_POSTURE, false, &pPosture) != SFX_ITEM_SET )
                    pPosture = 0;
                // Number format
                const SfxPoolItem* pNumFmt = NULL;
                if ( rESet.GetItemState(ATTR_VALUE_FORMAT, false, &pNumFmt) == SFX_ITEM_SET )
                    rSet.Put(*pNumFmt);
                if ( pFont || pHeight || pWeight || pPosture )
                {
                    String aStr( mpEngine->GetText( pE->aSel ) );
                    sal_uInt8 nScriptType = mpDoc->GetStringScriptType( aStr );
                    const sal_uInt8 nScripts[3] = { SCRIPTTYPE_LATIN,
                        SCRIPTTYPE_ASIAN, SCRIPTTYPE_COMPLEX };
                    for ( sal_uInt8 j=0; j<3; ++j )
                    {
                        if ( nScriptType & nScripts[j] )
                        {
                            if ( pFont )
                                rSet.Put( *pFont, ScGlobal::GetScriptedWhichID(
                                            nScripts[j], ATTR_FONT ));
                            if ( pHeight )
                                rSet.Put( *pHeight, ScGlobal::GetScriptedWhichID(
                                            nScripts[j], ATTR_FONT_HEIGHT ));
                            if ( pWeight )
                                rSet.Put( *pWeight, ScGlobal::GetScriptedWhichID(
                                            nScripts[j], ATTR_FONT_WEIGHT ));
                            if ( pPosture )
                                rSet.Put( *pPosture, ScGlobal::GetScriptedWhichID(
                                            nScripts[j], ATTR_FONT_POSTURE ));
                        }
                    }
                }
            }
            if ( pE->nColOverlap > 1 || pE->nRowOverlap > 1 )
            {   // merged cells, mit SfxItemSet Put schneller als mit
                // nachtraeglichem ScDocument DoMerge
                ScMergeAttr aMerge( pE->nColOverlap, pE->nRowOverlap );
                rSet.Put( aMerge );
                SCROW nRO = 0;
                if ( pE->nColOverlap > 1 )
                    mpDoc->ApplyFlagsTab( nCol+1, nRow,
                        nCol + pE->nColOverlap - 1, nRow, nTab,
                        SC_MF_HOR );
                if ( pE->nRowOverlap > 1 )
                {
                    nRO = nRow + pE->nRowOverlap - 1;
                    mpDoc->ApplyFlagsTab( nCol, nRow+1,
                        nCol, nRO , nTab,
                        SC_MF_VER );
                    if ( nRO > nOverlapRowMax )
                        nOverlapRowMax = nRO;
                }
                if ( pE->nColOverlap > 1 && pE->nRowOverlap > 1 )
                    mpDoc->ApplyFlagsTab( nCol+1, nRow+1,
                        nCol + pE->nColOverlap - 1, nRO, nTab,
                        SC_MF_HOR | SC_MF_VER );
            }
            const ScStyleSheet* pStyleSheet =
                mpDoc->GetPattern( nCol, nRow, nTab )->GetStyleSheet();
            aAttr.SetStyleSheet( (ScStyleSheet*)pStyleSheet );
            mpDoc->SetPattern( nCol, nRow, nTab, aAttr, sal_True );

            // Daten eintragen
            if (bSimple)
            {
                ScSetStringParam aParam;
                aParam.mpNumFormatter = pFormatter;
                aParam.mbDetectNumberFormat = true;
                aParam.meSetTextNumFormat = ScSetStringParam::SpecialNumberOnly;
                aParam.mbHandleApostrophe = false;

                if (!aValStr.isEmpty())
                    mpDoc->SetValue( nCol, nRow, nTab, fVal );
                else if ( !pE->aSel.HasRange() )
                {
                    // maybe ALT text of IMG or similar
                    mpDoc->SetString( nCol, nRow, nTab, pE->aAltText, &aParam );
                    // wenn SelRange komplett leer kann nachfolgender Text im gleichen Absatz liegen!
                }
                else
                {
                    String aStr;
                    if( pE->bEntirePara )
                    {
                        aStr = mpEngine->GetText( pE->aSel.nStartPara );
                    }
                    else
                    {
                        aStr = comphelper::string::strip(mpEngine->GetText(pE->aSel), ' ');
                    }

                    bool bTextFormat = false;

                    const SfxPoolItem* pNumFmt = NULL;
                    if (rSet.GetItemState(ATTR_VALUE_FORMAT, false, &pNumFmt) == SFX_ITEM_SET)
                    {
                        sal_uInt32 nNumFmt = static_cast<const SfxUInt32Item*>(pNumFmt)->GetValue();
                        sal_uInt16 nType = pFormatter->GetType(nNumFmt);
                        if (nType == NUMBERFORMAT_TEXT)
                            // Format is set to Text.
                            bTextFormat = true;
                    }

                    // TODO: RTF import should follow the language tag,
                    // currently this follows the HTML options for both, HTML
                    // and RTF.
                    if (bNumbersEnglishUS)
                    {
                        pFormatter->ChangeIntl( LANGUAGE_ENGLISH_US);
                        sal_uInt32 nIndex = pFormatter->GetStandardIndex( LANGUAGE_ENGLISH_US);
                        double fEnVal = 0.0;
                        if (pFormatter->IsNumberFormat( aStr, nIndex, fEnVal))
                        {
                            sal_uInt32 nNewIndex =
                                pFormatter->GetFormatForLanguageIfBuiltIn(
                                        nIndex, LANGUAGE_SYSTEM);
                            OSL_ENSURE( nNewIndex != nIndex, "ScEEImport::WriteToDocument: NumbersEnglishUS not a built-in format?");
                            pFormatter->GetInputLineString( fEnVal, nNewIndex, aStr);
                        }
                        else
                            bTextFormat = true;
                        pFormatter->ChangeIntl( LANGUAGE_SYSTEM);
                    }

                    //  #105460#, #i4180# String cells can't contain tabs or linebreaks
                    //  -> replace with spaces
                    aStr.SearchAndReplaceAll( (sal_Unicode)'\t', (sal_Unicode)' ' );
                    aStr.SearchAndReplaceAll( (sal_Unicode)'\n', (sal_Unicode)' ' );

                    if (bTextFormat)
                        mpDoc->PutCell( nCol, nRow, nTab, new ScStringCell( aStr));
                    else
                    {
                        aParam.mbDetectNumberFormat = bConvertDate;
                        mpDoc->SetString( nCol, nRow, nTab, aStr, &aParam );
                    }
                }
            }
            else
            {
                EditTextObject* pObject = mpEngine->CreateTextObject( pE->aSel );
                mpDoc->PutCell( nCol, nRow, nTab, new ScEditCell( pObject,
                    mpDoc, mpEngine->GetEditTextObjectPool() ) );
                delete pObject;
            }
            if ( pE->maImageList.size() )
                bHasGraphics |= GraphicSize( nCol, nRow, nTab, pE );
            if ( pE->pName )
            {   // Anchor Name => RangeName
                if (!pRangeNames->findByUpperName(ScGlobal::pCharClass->uppercase(*pE->pName)))
                {
                    ScRangeData* pData = new ScRangeData( mpDoc, *pE->pName,
                        ScAddress( nCol, nRow, nTab ) );
                    pRangeNames->insert( pData );
                }
            }
        }
        pProgress->SetStateOnPercent( ++nProgress );
    }
    if ( bSizeColsRows )
    {
        // Spaltenbreiten
        ColWidthsMap& rColWidths = mpParser->GetColWidths();
        if ( !rColWidths.empty() )
        {
            nProgress = 0;
            pProgress->SetState( nProgress, nEndCol - nStartCol + 1 );
            for ( SCCOL nCol = nStartCol; nCol <= nEndCol; nCol++ )
            {
                sal_uInt16 nWidth = 0;
                ColWidthsMap::const_iterator it = rColWidths.find( nCol );
                if ( it != rColWidths.end() )
                    nWidth = it->second;
                if ( nWidth )
                    mpDoc->SetColWidth( nCol, nTab, nWidth );
                pProgress->SetState( ++nProgress );
            }
        }
        DELETEZ( pProgress );   // SetOptimalHeight hat seinen eigenen ProgressBar
        // Zeilenhoehen anpassen, Basis 100% Zoom
        Fraction aZoom( 1, 1 );
        double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom
            / nOutputFactor;        // Faktor ist Drucker zu Bildschirm
        double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
        VirtualDevice aVirtDev;
        mpDoc->SetOptimalHeight( 0, nEndRow, 0,
            static_cast< sal_uInt16 >( ScGlobal::nLastRowHeightExtra ), &aVirtDev,
            nPPTX, nPPTY, aZoom, aZoom, false );
        if ( !maRowHeights.empty() )
        {
            for ( SCROW nRow = nStartRow; nRow <= nEndRow; nRow++ )
            {
                RowHeightMap::const_iterator it = maRowHeights.find( nRow );
                sal_uInt16 nHeight = it == maRowHeights.end() ? 0 : it->second;
                if ( nHeight > mpDoc->GetRowHeight( nRow, nTab ) )
                    mpDoc->SetRowHeight( nRow, nTab, nHeight );
            }
        }
    }
    if ( bHasGraphics )
    {
        // Grafiken einfuegen
        for ( size_t i = 0, nListSize = mpParser->ListSize(); i < nListSize; ++i )
        {
            pE = mpParser->ListEntry( i );
            if ( !pE->maImageList.empty() )
            {
                SCCOL nCol = pE->nCol;
                SCROW nRow = pE->nRow;
                if ( ValidCol(nCol) && ValidRow(nRow) )
                    InsertGraphic( nCol, nRow, nTab, pE );
            }
        }
    }
    if ( pProgress )
        delete pProgress;
}


sal_Bool ScEEImport::GraphicSize( SCCOL nCol, SCROW nRow, SCTAB /*nTab*/, ScEEParseEntry* pE )
{
    if ( !pE->maImageList.size() )
        return false;
    sal_Bool bHasGraphics = false;
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    long nWidth, nHeight;
    nWidth = nHeight = 0;
    sal_Char nDir = nHorizontal;
    for ( sal_uInt32 i = 0; i < pE->maImageList.size() ; ++i )
    {
        ScHTMLImage* pI = &pE->maImageList[ i ];
        if ( pI->pGraphic )
            bHasGraphics = sal_True;
        Size aSizePix = pI->aSize;
        aSizePix.Width() += 2 * pI->aSpace.X();
        aSizePix.Height() += 2 * pI->aSpace.Y();
        Size aLogicSize = pDefaultDev->PixelToLogic( aSizePix, MapMode( MAP_TWIP ) );
        if ( nDir & nHorizontal )
            nWidth += aLogicSize.Width();
        else if ( nWidth < aLogicSize.Width() )
            nWidth = aLogicSize.Width();
        if ( nDir & nVertical )
            nHeight += aLogicSize.Height();
        else if ( nHeight < aLogicSize.Height() )
            nHeight = aLogicSize.Height();
        nDir = pI->nDir;
    }
    // Spaltenbreiten
    ColWidthsMap& rColWidths = mpParser->GetColWidths();
    long nThisWidth = 0;
    ColWidthsMap::const_iterator it = rColWidths.find( nCol );
    if ( it != rColWidths.end() )
        nThisWidth = it->second;
    long nColWidths = nThisWidth;
    SCCOL nColSpanCol = nCol + pE->nColOverlap;
    for ( SCCOL nC = nCol + 1; nC < nColSpanCol; nC++ )
    {
        ColWidthsMap::const_iterator it2 = rColWidths.find( nC   );
        if ( it2 != rColWidths.end() )
            nColWidths += it2->second;
    }
    if ( nWidth > nColWidths )
    {   // Differenz nur in der ersten Spalte eintragen
        rColWidths[ nCol ] = nWidth - nColWidths + nThisWidth;
    }
    // Zeilenhoehen, Differenz auf alle betroffenen Zeilen verteilen
    SCROW nRowSpan = pE->nRowOverlap;
    nHeight /= nRowSpan;
    if ( nHeight == 0 )
        nHeight = 1;        // fuer eindeutigen Vergleich
    for ( SCROW nR = nRow; nR < nRow + nRowSpan; nR++ )
    {
        RowHeightMap::const_iterator it2 = maRowHeights.find( nR );
        long nRowHeight = it2 == maRowHeights.end() ? 0 : it2->second;
        if ( nHeight > nRowHeight )
        {
            maRowHeights[ nR ] = nHeight;
        }
    }
    return bHasGraphics;
}


void ScEEImport::InsertGraphic( SCCOL nCol, SCROW nRow, SCTAB nTab,
        ScEEParseEntry* pE )
{
    if ( !pE->maImageList.size() )
        return ;
    ScDrawLayer* pModel = mpDoc->GetDrawLayer();
    if (!pModel)
    {
        mpDoc->InitDrawLayer();
        pModel = mpDoc->GetDrawLayer();
    }
    SdrPage* pPage = pModel->GetPage( static_cast<sal_uInt16>(nTab) );
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();

    Point aCellInsertPos(
        (long)((double) mpDoc->GetColOffset( nCol, nTab ) * HMM_PER_TWIPS),
        (long)((double) mpDoc->GetRowOffset( nRow, nTab ) * HMM_PER_TWIPS) );

    Point aInsertPos( aCellInsertPos );
    Point aSpace;
    Size aLogicSize;
    sal_Char nDir = nHorizontal;
    for ( sal_uInt32 i = 0; i < pE->maImageList.size(); ++i )
    {
        ScHTMLImage* pI = &pE->maImageList[ i ];
        if ( nDir & nHorizontal )
        {   // horizontal
            aInsertPos.X() += aLogicSize.Width();
            aInsertPos.X() += aSpace.X();
            aInsertPos.Y() = aCellInsertPos.Y();
        }
        else
        {   // vertikal
            aInsertPos.X() = aCellInsertPos.X();
            aInsertPos.Y() += aLogicSize.Height();
            aInsertPos.Y() += aSpace.Y();
        }
        // Offset des Spacings drauf
        aSpace = pDefaultDev->PixelToLogic( pI->aSpace, MapMode( MAP_100TH_MM ) );
        aInsertPos += aSpace;

        Size aSizePix = pI->aSize;
        aLogicSize = pDefaultDev->PixelToLogic( aSizePix, MapMode( MAP_100TH_MM ) );
        //  Groesse begrenzen
        ::ScLimitSizeOnDrawPage( aLogicSize, aInsertPos, pPage->GetSize() );

        if ( pI->pGraphic )
        {
            Rectangle aRect ( aInsertPos, aLogicSize );
            SdrGrafObj* pObj = new SdrGrafObj( *pI->pGraphic, aRect );
            // calling SetGraphicLink here doesn't work
            pObj->SetName( pI->aURL );

            pPage->InsertObject( pObj );

            // SetGraphicLink has to be used after inserting the object,
            // otherwise an empty graphic is swapped in and the contact stuff crashes.
            // See #i37444#.
            pObj->SetGraphicLink( pI->aURL, pI->aFilterName );

            pObj->SetLogicRect( aRect );        // erst nach InsertObject !!!
        }
        nDir = pI->nDir;
    }
}


ScEEParser::ScEEParser( EditEngine* pEditP ) :
        pEdit( pEditP ),
        pPool( EditEngine::CreatePool() ),
        pDocPool( new ScDocumentPool ),
        nLastToken(0),
        nColCnt(0),
        nRowCnt(0),
        nColMax(0),
        nRowMax(0)
{
    // pPool wird spaeter bei RTFIMP_START dem SvxRTFParser untergejubelt
    pPool->SetSecondaryPool( pDocPool );
    pPool->FreezeIdRanges();
    NewActEntry( NULL );
}


ScEEParser::~ScEEParser()
{
    delete pActEntry;
    if ( !maList.empty() ) maList.clear();

    // Pool erst loeschen nachdem die Listen geloescht wurden
    pPool->SetSecondaryPool( NULL );
    SfxItemPool::Free(pDocPool);
    SfxItemPool::Free(pPool);
}


void ScEEParser::NewActEntry( ScEEParseEntry* pE )
{   // neuer freifliegender pActEntry
    pActEntry = new ScEEParseEntry( pPool );
    pActEntry->aSel.nStartPara = (pE ? pE->aSel.nEndPara + 1 : 0);
    pActEntry->aSel.nStartPos = 0;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
