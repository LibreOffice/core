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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/adjustitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/escapementitem.hxx>
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
#include "eeparser.hxx"
#include "drwlayer.hxx"
#include "rangenam.hxx"
#include "progress.hxx"
#include "stringutil.hxx"
#include <rowheightcontext.hxx>
#include "fuinsert.hxx"

#include "globstr.hrc"

#include <memory>

ScEEImport::ScEEImport( ScDocument* pDocP, const ScRange& rRange ) :
    maRange( rRange ),
    mpDoc( pDocP ),
    mpParser( nullptr )
{
    const ScPatternAttr* pPattern = mpDoc->GetPattern(
        maRange.aStart.Col(), maRange.aStart.Row(), maRange.aStart.Tab() );
    mpEngine = new ScTabEditEngine(*pPattern, mpDoc->GetEditPool(), mpDoc->GetEditPool());
    mpEngine->SetUpdateMode( false );
    mpEngine->EnableUndo( false );
}

ScEEImport::~ScEEImport()
{
    // Sequence important, or else we crash in some dtor!
    // Is guaranteed as ScEEImport is base class
    delete mpEngine; // After Parser!
}

sal_uLong ScEEImport::Read( SvStream& rStream, const OUString& rBaseURL )
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
    bool bHasGraphics = false;
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
        // Determine RowMerge
        // Pure ColMerge and ColMerge of the first MergeRow already done during parsing
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
        // Add for second run
        pE->nCol = nCol;
        pE->nRow = nRow;
        if ( ValidCol(nCol) && ValidRow(nRow) )
        {
            SfxItemSet aSet = mpEngine->GetAttribs( pE->aSel );
            // Remove default: we set left/right ourselves depending on Text or
            // Number
            // EditView.GetAttribs always returns complete Set filled with
            // defaults
            const SfxPoolItem& rItem = aSet.Get( EE_PARA_JUST );
            if ( static_cast<const SvxAdjustItem&>(rItem).GetAdjust() == SVX_ADJUST_LEFT )
                aSet.ClearItem( EE_PARA_JUST );

            // Test whether simple String without mixed attributes
            bool bSimple = ( pE->aSel.nStartPara == pE->aSel.nEndPara );
            for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END && bSimple; nId++)
            {
                const SfxPoolItem* pItem = nullptr;
                SfxItemState eState = aSet.GetItemState( nId, true, &pItem );
                if (eState == SfxItemState::DONTCARE)
                    bSimple = false;
                else if (eState == SfxItemState::SET)
                {
                    if ( nId == EE_CHAR_ESCAPEMENT ) // Super-/Subscript always via EE
                    {
                        if ( (SvxEscapement)static_cast<const SvxEscapementItem*>(pItem)->GetEnumValue()
                                != SVX_ESCAPEMENT_OFF )
                            bSimple = false;
                    }
                }
            }
            if ( bSimple )
            {   //  Contains field commands?
                SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, false );
                if ( eFieldState == SfxItemState::DONTCARE || eFieldState == SfxItemState::SET )
                    bSimple = false;
            }

            // HTML
            OUString aValStr, aNumStr;
            double fVal;
            sal_uInt32 nNumForm = 0;
            LanguageType eNumLang = LANGUAGE_NONE;
            if ( pE->pNumStr )
            {   // SDNUM needs to be if SDVAL
                aNumStr = *pE->pNumStr;
                if ( pE->pValStr )
                    aValStr = *pE->pValStr;
                fVal = SfxHTMLParser::GetTableDataOptionsValNum(
                    nNumForm, eNumLang, aValStr, aNumStr, *pFormatter );
            }

            // Set attributes
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
                if ( rESet.GetItemState( ATTR_BACKGROUND, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_BORDER, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_SHADOW, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                // HTML
                if ( rESet.GetItemState( ATTR_HOR_JUSTIFY, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_VER_JUSTIFY, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_LINEBREAK, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_FONT_COLOR, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                if ( rESet.GetItemState( ATTR_FONT_UNDERLINE, false, &pItem) == SfxItemState::SET )
                    rSet.Put( *pItem );
                // HTML LATIN/CJK/CTL script type dependent
                const SfxPoolItem* pFont;
                if ( rESet.GetItemState( ATTR_FONT, false, &pFont) != SfxItemState::SET )
                    pFont = nullptr;
                const SfxPoolItem* pHeight;
                if ( rESet.GetItemState( ATTR_FONT_HEIGHT, false, &pHeight) != SfxItemState::SET )
                    pHeight = nullptr;
                const SfxPoolItem* pWeight;
                if ( rESet.GetItemState( ATTR_FONT_WEIGHT, false, &pWeight) != SfxItemState::SET )
                    pWeight = nullptr;
                const SfxPoolItem* pPosture;
                if ( rESet.GetItemState( ATTR_FONT_POSTURE, false, &pPosture) != SfxItemState::SET )
                    pPosture = nullptr;
                // Number format
                const SfxPoolItem* pNumFmt = nullptr;
                if ( rESet.GetItemState(ATTR_VALUE_FORMAT, false, &pNumFmt) == SfxItemState::SET )
                    rSet.Put(*pNumFmt);
                if ( pFont || pHeight || pWeight || pPosture )
                {
                    OUString aStr( mpEngine->GetText( pE->aSel ) );
                    SvtScriptType nScriptType = mpDoc->GetStringScriptType( aStr );
                    const SvtScriptType nScripts[3] = { SvtScriptType::LATIN,
                        SvtScriptType::ASIAN, SvtScriptType::COMPLEX };
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
            {   // Merged cells, with SfxItemSet.Put() is faster than
                // with ScDocument.DoMerge() afterwards
                ScMergeAttr aMerge( pE->nColOverlap, pE->nRowOverlap );
                rSet.Put( aMerge );
                SCROW nRO = 0;
                if ( pE->nColOverlap > 1 )
                    mpDoc->ApplyFlagsTab( nCol+1, nRow,
                        nCol + pE->nColOverlap - 1, nRow, nTab,
                        ScMF::Hor );
                if ( pE->nRowOverlap > 1 )
                {
                    nRO = nRow + pE->nRowOverlap - 1;
                    mpDoc->ApplyFlagsTab( nCol, nRow+1,
                        nCol, nRO , nTab,
                        ScMF::Ver );
                    if ( nRO > nOverlapRowMax )
                        nOverlapRowMax = nRO;
                }
                if ( pE->nColOverlap > 1 && pE->nRowOverlap > 1 )
                    mpDoc->ApplyFlagsTab( nCol+1, nRow+1,
                        nCol + pE->nColOverlap - 1, nRO, nTab,
                        ScMF::Hor | ScMF::Ver );
            }
            const ScStyleSheet* pStyleSheet =
                mpDoc->GetPattern( nCol, nRow, nTab )->GetStyleSheet();
            aAttr.SetStyleSheet( const_cast<ScStyleSheet*>(pStyleSheet) );
            mpDoc->SetPattern( nCol, nRow, nTab, aAttr, true );

            // Add data
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
                    // If SelRange is completely empty, the succeeding text can be in the same paragraph!
                }
                else
                {
                    OUString aStr;
                    if( pE->bEntirePara )
                    {
                        aStr = mpEngine->GetText( pE->aSel.nStartPara );
                    }
                    else
                    {
                        aStr = comphelper::string::strip(mpEngine->GetText(pE->aSel), ' ');
                    }

                    bool bTextFormat = false;

                    const SfxPoolItem* pNumFmt = nullptr;
                    if (rSet.GetItemState(ATTR_VALUE_FORMAT, false, &pNumFmt) == SfxItemState::SET)
                    {
                        sal_uInt32 nNumFmt = static_cast<const SfxUInt32Item*>(pNumFmt)->GetValue();
                        sal_uInt16 nType = pFormatter->GetType(nNumFmt);
                        if (nType == css::util::NumberFormat::TEXT)
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
                    aStr = aStr.replaceAll( "\t", " " );
                    aStr = aStr.replaceAll( "\n", " " );

                    if (bTextFormat)
                    {
                        aParam.mbDetectNumberFormat = false;
                        aParam.meSetTextNumFormat = ScSetStringParam::Always;
                    }
                    else
                        aParam.mbDetectNumberFormat = bConvertDate;

                    mpDoc->SetString(nCol, nRow, nTab, aStr, &aParam);
                }
            }
            else
            {
                // The cell will own the text object instance.
                mpDoc->SetEditText(ScAddress(nCol,nRow,nTab), mpEngine->CreateTextObject(pE->aSel));
            }
            if ( !pE->maImageList.empty() )
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
        // Column widths
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
        DELETEZ( pProgress ); // SetOptimalHeight has its own ProgressBar
        // Adjust line height, base is 100% zoom
        Fraction aZoom( 1, 1 );
        // Factor is printer to display ratio
        double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom / nOutputFactor;
        double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
        ScopedVclPtrInstance< VirtualDevice > pVirtDev;
        sc::RowHeightContext aCxt(nPPTX, nPPTY, aZoom, aZoom, pVirtDev);
        aCxt.setExtraHeight(ScGlobal::nLastRowHeightExtra);
        mpDoc->SetOptimalHeight(aCxt, 0, nEndRow, 0);

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
        // Insert graphics
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
    delete pProgress;
}

bool ScEEImport::GraphicSize( SCCOL nCol, SCROW nRow, SCTAB /*nTab*/, ScEEParseEntry* pE )
{
    if ( pE->maImageList.empty() )
        return false;
    bool bHasGraphics = false;
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    long nWidth, nHeight;
    nWidth = nHeight = 0;
    sal_Char nDir = nHorizontal;
    for ( size_t i = 0; i < pE->maImageList.size() ; ++i )
    {
        ScHTMLImage* pI = pE->maImageList[ i ].get();
        if ( pI->pGraphic )
            bHasGraphics = true;
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
    // Column widths
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
    {   // Only insert difference in first column
        rColWidths[ nCol ] = nWidth - nColWidths + nThisWidth;
    }
    // Distribute line height difference between all affected lines
    SCROW nRowSpan = pE->nRowOverlap;
    nHeight /= nRowSpan;
    if ( nHeight == 0 )
        nHeight = 1; // For definite comparison
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
    if ( pE->maImageList.empty() )
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
    for ( size_t i = 0; i < pE->maImageList.size(); ++i )
    {
        ScHTMLImage* pI = pE->maImageList[ i ].get();
        if ( nDir & nHorizontal )
        {   // Horizontal
            aInsertPos.X() += aLogicSize.Width();
            aInsertPos.X() += aSpace.X();
            aInsertPos.Y() = aCellInsertPos.Y();
        }
        else
        {   // Vertical
            aInsertPos.X() = aCellInsertPos.X();
            aInsertPos.Y() += aLogicSize.Height();
            aInsertPos.Y() += aSpace.Y();
        }
        // Add offset of Spacing
        aSpace = pDefaultDev->PixelToLogic( pI->aSpace, MapMode( MAP_100TH_MM ) );
        aInsertPos += aSpace;

        Size aSizePix = pI->aSize;
        aLogicSize = pDefaultDev->PixelToLogic( aSizePix, MapMode( MAP_100TH_MM ) );

        // Limit size
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
            pObj->SetGraphicLink( pI->aURL, ""/*TODO?*/, pI->aFilterName );

            pObj->SetLogicRect( aRect ); // Only after InsertObject!
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
    // pPool is foisted on SvxRTFParser at RTFIMP_START later on
    pPool->SetSecondaryPool( pDocPool );
    pPool->FreezeIdRanges();
    NewActEntry( nullptr );
}

ScEEParser::~ScEEParser()
{
    delete pActEntry;
    while ( !maList.empty() )
    {
        delete maList.back();
        maList.pop_back();
    }

    // Don't delete Pool until the lists have been deleted
    pPool->SetSecondaryPool( nullptr );
    SfxItemPool::Free(pDocPool);
    SfxItemPool::Free(pPool);
}

void ScEEParser::NewActEntry( ScEEParseEntry* pE )
{   // New free-flying pActEntry
    pActEntry = new ScEEParseEntry( pPool );
    pActEntry->aSel.nStartPara = (pE ? pE->aSel.nEndPara + 1 : 0);
    pActEntry->aSel.nStartPos = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
