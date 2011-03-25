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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/algitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <svl/zforlist.hxx>
#include <svl/broadcast.hxx>
#include <svl/listeneriter.hxx>
#include <vcl/outdev.hxx>

#include "column.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "collect.hxx"
#include "stlsheet.hxx"
#include "rechead.hxx"
#include "brdcst.hxx"
#include "editutil.hxx"
#include "subtotal.hxx"
#include "markdata.hxx"
#include "compiler.hxx"         // ScTokenArray GetCodeLen
#include "dbcolect.hxx"
#include "fillinfo.hxx"
#include "segmenttree.hxx"

#include <math.h>

// -----------------------------------------------------------------------

// factor from font size to optimal cell height (text width)
#define SC_ROT_BREAK_FACTOR     6

// -----------------------------------------------------------------------

inline sal_Bool IsAmbiguousScript( sal_uInt8 nScript )
{
    //! move to a header file
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}

// -----------------------------------------------------------------------------------------

//
//  Datei-Operationen
//

// -----------------------------------------------------------------------------------------

//UNUSED2008-05  SCROW ScColumn::NoteCount( SCROW nMaxRow ) const
//UNUSED2008-05  {
//UNUSED2008-05      SCROW nNoteCount = 0;
//UNUSED2008-05      SCSIZE i;
//UNUSED2008-05
//UNUSED2008-05      for (i=0; i<nCount; i++)
//UNUSED2008-05          if ( pItems[i].pCell->GetNotePtr() && pItems[i].nRow<=nMaxRow )
//UNUSED2008-05              ++nNoteCount;
//UNUSED2008-05
//UNUSED2008-05      return nNoteCount;
//UNUSED2008-05  }

// -----------------------------------------------------------------------------------------

//UNUSED2008-05  void ScColumn::CorrectSymbolCells( CharSet eStreamCharSet )
//UNUSED2008-05  {
//UNUSED2008-05      //  #99139# find and correct string cells that are formatted with a symbol font,
//UNUSED2008-05      //  but are not in the LoadedSymbolStringCellsList
//UNUSED2008-05      //  (because CELLTYPE_SYMBOLS wasn't written in the file)
//UNUSED2008-05
//UNUSED2008-05      ScFontToSubsFontConverter_AutoPtr xFontConverter;
//UNUSED2008-05      const sal_uLong nFontConverterFlags = FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;
//UNUSED2008-05
//UNUSED2008-05      sal_Bool bListInitialized = sal_False;
//UNUSED2008-05      ScSymbolStringCellEntry* pCurrentEntry = NULL;
//UNUSED2008-05
//UNUSED2008-05      ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
//UNUSED2008-05      SCROW nStt, nEnd;
//UNUSED2008-05      const ScPatternAttr* pAttr = aAttrIter.Next( nStt, nEnd );
//UNUSED2008-05      while ( pAttr )
//UNUSED2008-05      {
//UNUSED2008-05          if ( (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags )) ||
//UNUSED2008-05                  pAttr->IsSymbolFont() )
//UNUSED2008-05          {
//UNUSED2008-05              ScColumnIterator aCellIter( this, nStt, nEnd );
//UNUSED2008-05              SCROW nRow;
//UNUSED2008-05              ScBaseCell* pCell;
//UNUSED2008-05              while ( aCellIter.Next( nRow, pCell ) )
//UNUSED2008-05              {
//UNUSED2008-05                  if ( pCell->GetCellType() == CELLTYPE_STRING )
//UNUSED2008-05                  {
//UNUSED2008-05                      List& rList = pDocument->GetLoadedSymbolStringCellsList();
//UNUSED2008-05                      if (!bListInitialized)
//UNUSED2008-05                      {
//UNUSED2008-05                          pCurrentEntry = (ScSymbolStringCellEntry*)rList.First();
//UNUSED2008-05                          bListInitialized = sal_True;
//UNUSED2008-05                      }
//UNUSED2008-05
//UNUSED2008-05                      while ( pCurrentEntry && pCurrentEntry->nRow < nRow )
//UNUSED2008-05                          pCurrentEntry = (ScSymbolStringCellEntry*)rList.Next();
//UNUSED2008-05
//UNUSED2008-05                      if ( pCurrentEntry && pCurrentEntry->nRow == nRow )
//UNUSED2008-05                      {
//UNUSED2008-05                          //  found
//UNUSED2008-05                      }
//UNUSED2008-05                      else
//UNUSED2008-05                      {
//UNUSED2008-05                          //  not in list -> convert and put into list
//UNUSED2008-05
//UNUSED2008-05                          ScStringCell* pStrCell = (ScStringCell*)pCell;
//UNUSED2008-05                          String aOldStr;
//UNUSED2008-05                          pStrCell->GetString( aOldStr );
//UNUSED2008-05
//UNUSED2008-05                          //  convert back to stream character set (get original data)
//UNUSED2008-05                          ByteString aByteStr( aOldStr, eStreamCharSet );
//UNUSED2008-05
//UNUSED2008-05                          //  convert using symbol encoding, as for CELLTYPE_SYMBOLS cells
//UNUSED2008-05                          String aNewStr( aByteStr, RTL_TEXTENCODING_SYMBOL );
//UNUSED2008-05                          pStrCell->SetString( aNewStr );
//UNUSED2008-05
//UNUSED2008-05                          ScSymbolStringCellEntry * pEntry = new ScSymbolStringCellEntry;
//UNUSED2008-05                          pEntry->pCell = pStrCell;
//UNUSED2008-05                          pEntry->nRow = nRow;
//UNUSED2008-05
//UNUSED2008-05                          if ( pCurrentEntry )
//UNUSED2008-05                              rList.Insert( pEntry );     // before current entry - pCurrentEntry stays valid
//UNUSED2008-05                          else
//UNUSED2008-05                              rList.Insert( pEntry, LIST_APPEND );    // append if already behind last entry
//UNUSED2008-05                      }
//UNUSED2008-05                  }
//UNUSED2008-05              }
//UNUSED2008-05          }
//UNUSED2008-05
//UNUSED2008-05          pAttr = aAttrIter.Next( nStt, nEnd );
//UNUSED2008-05      }
//UNUSED2008-05  }

// -----------------------------------------------------------------------------------------

                                    //  GetNeededSize: optimale Hoehe / Breite in Pixeln

long ScColumn::GetNeededSize( SCROW nRow, OutputDevice* pDev,
                              double nPPTX, double nPPTY,
                              const Fraction& rZoomX, const Fraction& rZoomY,
                              sal_Bool bWidth, const ScNeededSizeOptions& rOptions )
{
    long nValue=0;
    SCSIZE nIndex;
    double nPPT = bWidth ? nPPTX : nPPTY;
    if (Search(nRow,nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        const ScPatternAttr* pPattern = rOptions.pPattern;
        if (!pPattern)
            pPattern = pAttrArray->GetPattern( nRow );

        //      zusammengefasst?
        //      Merge nicht in bedingter Formatierung

        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);

        if ( bWidth )
        {
            if ( pFlag->IsHorOverlapped() )
                return 0;
            if ( rOptions.bSkipMerged && pMerge->GetColMerge() > 1 )
                return 0;
        }
        else
        {
            if ( pFlag->IsVerOverlapped() )
                return 0;
            if ( rOptions.bSkipMerged && pMerge->GetRowMerge() > 1 )
                return 0;
        }

        //      bedingte Formatierung
        const SfxItemSet* pCondSet = NULL;
        if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
            pCondSet = pDocument->GetCondResult( nCol, nRow, nTab );

        //  Zeilenumbruch?

        const SfxPoolItem* pCondItem;
        SvxCellHorJustify eHorJust;
        if (pCondSet &&
                pCondSet->GetItemState(ATTR_HOR_JUSTIFY, sal_True, &pCondItem) == SFX_ITEM_SET)
            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem*)pCondItem)->GetValue();
        else
            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem( ATTR_HOR_JUSTIFY )).GetValue();
        bool bBreak;
        if ( eHorJust == SVX_HOR_JUSTIFY_BLOCK )
            bBreak = true;
        else if ( pCondSet &&
                    pCondSet->GetItemState(ATTR_LINEBREAK, sal_True, &pCondItem) == SFX_ITEM_SET)
            bBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
        else
            bBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

        if (pCell->HasValueData())
            // Cell has a value.  Disable line break.
            bBreak = false;

        //  get other attributes from pattern and conditional formatting

        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
        sal_Bool bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
                ((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet )).GetValue() );
        if ( bAsianVertical )
            bBreak = false;

        if ( bWidth && bBreak )     // after determining bAsianVertical (bBreak may be reset)
            return 0;

        long nRotate = 0;
        SvxRotateMode eRotMode = SVX_ROTATE_MODE_STANDARD;
        if ( eOrient == SVX_ORIENTATION_STANDARD )
        {
            if (pCondSet &&
                    pCondSet->GetItemState(ATTR_ROTATE_VALUE, sal_True, &pCondItem) == SFX_ITEM_SET)
                nRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
            else
                nRotate = ((const SfxInt32Item&)pPattern->GetItem(ATTR_ROTATE_VALUE)).GetValue();
            if ( nRotate )
            {
                if (pCondSet &&
                        pCondSet->GetItemState(ATTR_ROTATE_MODE, sal_True, &pCondItem) == SFX_ITEM_SET)
                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem*)pCondItem)->GetValue();
                else
                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                                pPattern->GetItem(ATTR_ROTATE_MODE)).GetValue();

                if ( nRotate == 18000 )
                    eRotMode = SVX_ROTATE_MODE_STANDARD;    // keinen Ueberlauf
            }
        }

        if ( eHorJust == SVX_HOR_JUSTIFY_REPEAT )
        {
            // ignore orientation/rotation if "repeat" is active
            eOrient = SVX_ORIENTATION_STANDARD;
            nRotate = 0;
            bAsianVertical = sal_False;
        }

        const SvxMarginItem* pMargin;
        if (pCondSet &&
                pCondSet->GetItemState(ATTR_MARGIN, sal_True, &pCondItem) == SFX_ITEM_SET)
            pMargin = (const SvxMarginItem*) pCondItem;
        else
            pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        sal_uInt16 nIndent = 0;
        if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
        {
            if (pCondSet &&
                    pCondSet->GetItemState(ATTR_INDENT, sal_True, &pCondItem) == SFX_ITEM_SET)
                nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
            else
                nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
        }

        sal_uInt8 nScript = pDocument->GetScriptType( nCol, nRow, nTab, pCell );
        if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

        //  also call SetFont for edit cells, because bGetFont may be set only once
        //  bGetFont is set also if script type changes
        if (rOptions.bGetFont)
        {
            Fraction aFontZoom = ( eOrient == SVX_ORIENTATION_STANDARD ) ? rZoomX : rZoomY;
            Font aFont;
            // font color doesn't matter here
            pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &aFontZoom, pCondSet, nScript );
            pDev->SetFont(aFont);
        }

        sal_Bool bAddMargin = sal_True;
        CellType eCellType = pCell->GetCellType();

        sal_Bool bEditEngine = ( eCellType == CELLTYPE_EDIT ||
                                eOrient == SVX_ORIENTATION_STACKED ||
                                IsAmbiguousScript( nScript ) ||
                                ((eCellType == CELLTYPE_FORMULA) && ((ScFormulaCell*)pCell)->IsMultilineResult()) );

        if (!bEditEngine)                                   // direkte Ausgabe
        {
            String aValStr;
            Color* pColor;
            SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
            sal_uLong nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
            ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
                                        *pFormatter,
                                        sal_True, rOptions.bFormula, ftCheck );
            if (aValStr.Len())
            {
                //  SetFont ist nach oben verschoben

                Size aSize( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
                if ( eOrient != SVX_ORIENTATION_STANDARD )
                {
                    long nTemp = aSize.Width();
                    aSize.Width() = aSize.Height();
                    aSize.Height() = nTemp;
                }
                else if ( nRotate )
                {
                    //! unterschiedliche Skalierung X/Y beruecksichtigen

                    double nRealOrient = nRotate * F_PI18000;   // nRotate sind 1/100 Grad
                    double nCosAbs = fabs( cos( nRealOrient ) );
                    double nSinAbs = fabs( sin( nRealOrient ) );
                    long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                    long nWidth;
                    if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                        nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                    else if ( rOptions.bTotalSize )
                    {
                        nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
                        bAddMargin = sal_False;
                        //  nur nach rechts:
                        //! unterscheiden nach Ausrichtung oben/unten (nur Text/ganze Hoehe)
                        if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                            nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                                nPPT * nCosAbs / nSinAbs );
                    }
                    else
                        nWidth  = (long)( aSize.Height() / nSinAbs );   //! begrenzen?

                    if ( bBreak && !rOptions.bTotalSize )
                    {
                        //  #47744# limit size for line break
                        long nCmp = pDev->GetFont().GetSize().Height() * SC_ROT_BREAK_FACTOR;
                        if ( nHeight > nCmp )
                            nHeight = nCmp;
                    }

                    aSize = Size( nWidth, nHeight );
                }
                nValue = bWidth ? aSize.Width() : aSize.Height();

                if ( bAddMargin )
                {
                    if (bWidth)
                    {
                        nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
                                  (long) ( pMargin->GetRightMargin() * nPPT );
                        if ( nIndent )
                            nValue += (long) ( nIndent * nPPT );
                    }
                    else
                        nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
                                  (long) ( pMargin->GetBottomMargin() * nPPT );
                }

                                                //  Zeilenumbruch ausgefuehrt ?

                if ( bBreak && !bWidth )
                {
                    //  Test mit EditEngine zur Sicherheit schon bei 90%
                    //  (wegen Rundungsfehlern und weil EditEngine teilweise anders formatiert)

                    long nDocPixel = (long) ( ( pDocument->GetColWidth( nCol,nTab ) -
                                        pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
                                        nIndent )
                                        * nPPT );
                    nDocPixel = (nDocPixel * 9) / 10;           // zur Sicherheit
                    if ( aSize.Width() > nDocPixel )
                        bEditEngine = sal_True;
                }
            }
        }

        if (bEditEngine)
        {
            //  der Font wird bei !bEditEngine nicht jedesmal neu gesetzt
            Font aOldFont = pDev->GetFont();

            MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );

            // am Dokument speichern ?
            ScFieldEditEngine* pEngine = pDocument->CreateFieldEditEngine();

            pEngine->SetUpdateMode( sal_False );
            sal_Bool bTextWysiwyg = ( pDev->GetOutDevType() == OUTDEV_PRINTER );
            sal_uInt32 nCtrl = pEngine->GetControlWord();
            if ( bTextWysiwyg )
                nCtrl |= EE_CNTRL_FORMAT100;
            else
                nCtrl &= ~EE_CNTRL_FORMAT100;
            pEngine->SetControlWord( nCtrl );
            MapMode aOld = pDev->GetMapMode();
            pDev->SetMapMode( aHMMMode );
            pEngine->SetRefDevice( pDev );
            pDocument->ApplyAsianEditSettings( *pEngine );
            SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
            pPattern->FillEditItemSet( pSet, pCondSet );

//          no longer needed, are setted with the text (is faster)
//          pEngine->SetDefaults( pSet );

            if ( ((const SfxBoolItem&)pSet->Get(EE_PARA_HYPHENATE)).GetValue() ) {

                com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
                pEngine->SetHyphenator( xXHyphenator );
            }

            Size aPaper = Size( 1000000, 1000000 );
            if ( eOrient==SVX_ORIENTATION_STACKED && !bAsianVertical )
                aPaper.Width() = 1;
            else if (bBreak)
            {
                double fWidthFactor = nPPTX;
                if ( bTextWysiwyg )
                {
                    //  #95593# if text is formatted for printer, don't use PixelToLogic,
                    //  to ensure the exact same paper width (and same line breaks) as in
                    //  ScEditUtil::GetEditArea, used for output.

                    fWidthFactor = HMM_PER_TWIPS;
                }

                // use original width for hidden columns:
                long nDocWidth = (long) ( pDocument->GetOriginalWidth(nCol,nTab) * fWidthFactor );
                SCCOL nColMerge = pMerge->GetColMerge();
                if (nColMerge > 1)
                    for (SCCOL nColAdd=1; nColAdd<nColMerge; nColAdd++)
                        nDocWidth += (long) ( pDocument->GetColWidth(nCol+nColAdd,nTab) * fWidthFactor );
                nDocWidth -= (long) ( pMargin->GetLeftMargin() * fWidthFactor )
                           + (long) ( pMargin->GetRightMargin() * fWidthFactor )
                           + 1;     // Ausgabebereich ist Breite-1 Pixel (wegen Gitterlinien)
                if ( nIndent )
                    nDocWidth -= (long) ( nIndent * fWidthFactor );

                // space for AutoFilter button:  20 * nZoom/100
                if ( pFlag->HasAutoFilter() && !bTextWysiwyg )
                    nDocWidth -= (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();

                aPaper.Width() = nDocWidth;

                if ( !bTextWysiwyg )
                    aPaper = pDev->PixelToLogic( aPaper, aHMMMode );
            }
            pEngine->SetPaperSize(aPaper);

            if ( pCell->GetCellType() == CELLTYPE_EDIT )
            {
                const EditTextObject* pData;
                ((ScEditCell*)pCell)->GetData(pData);
                pEngine->SetTextNewDefaults(*pData, pSet);
            }
            else
            {
                Color* pColor;
                SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
                sal_uLong nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
                String aString;
                ScCellFormat::GetString( pCell, nFormat, aString, &pColor,
                                            *pFormatter,
                                            sal_True, rOptions.bFormula, ftCheck );
                if (aString.Len())
                    pEngine->SetTextNewDefaults(aString, pSet);
                else
                    pEngine->SetDefaults(pSet);
            }

            sal_Bool bEngineVertical = pEngine->IsVertical();
            pEngine->SetVertical( bAsianVertical );
            pEngine->SetUpdateMode( sal_True );

            sal_Bool bEdWidth = bWidth;
            if ( eOrient != SVX_ORIENTATION_STANDARD && eOrient != SVX_ORIENTATION_STACKED )
                bEdWidth = !bEdWidth;
            if ( nRotate )
            {
                //! unterschiedliche Skalierung X/Y beruecksichtigen

                Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
                double nRealOrient = nRotate * F_PI18000;   // nRotate sind 1/100 Grad
                double nCosAbs = fabs( cos( nRealOrient ) );
                double nSinAbs = fabs( sin( nRealOrient ) );
                long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                long nWidth;
                if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                    nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                else if ( rOptions.bTotalSize )
                {
                    nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
                    bAddMargin = sal_False;
                    if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                        nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                            nPPT * nCosAbs / nSinAbs );
                }
                else
                    nWidth  = (long)( aSize.Height() / nSinAbs );   //! begrenzen?
                aSize = Size( nWidth, nHeight );

                Size aPixSize = pDev->LogicToPixel( aSize, aHMMMode );
                if ( bEdWidth )
                    nValue = aPixSize.Width();
                else
                {
                    nValue = aPixSize.Height();

                    if ( bBreak && !rOptions.bTotalSize )
                    {
                        //  #47744# limit size for line break
                        long nCmp = aOldFont.GetSize().Height() * SC_ROT_BREAK_FACTOR;
                        if ( nValue > nCmp )
                            nValue = nCmp;
                    }
                }
            }
            else if ( bEdWidth )
            {
                if (bBreak)
                    nValue = 0;
                else
                    nValue = pDev->LogicToPixel(Size( pEngine->CalcTextWidth(), 0 ),
                                        aHMMMode).Width();
            }
            else            // Hoehe
            {
                nValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ),
                                    aHMMMode).Height();

                // With non-100% zoom and several lines or paragraphs, don't shrink below the result with FORMAT100 set
                if ( !bTextWysiwyg && ( rZoomY.GetNumerator() != 1 || rZoomY.GetDenominator() != 1 ) &&
                     ( pEngine->GetParagraphCount() > 1 || ( bBreak && pEngine->GetLineCount(0) > 1 ) ) )
                {
                    pEngine->SetControlWord( nCtrl | EE_CNTRL_FORMAT100 );
                    pEngine->QuickFormatDoc( sal_True );
                    long nSecondValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ), aHMMMode).Height();
                    if ( nSecondValue > nValue )
                        nValue = nSecondValue;
                }
            }

            if ( nValue && bAddMargin )
            {
                if (bWidth)
                {
                    nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
                              (long) ( pMargin->GetRightMargin() * nPPT );
                    if (nIndent)
                        nValue += (long) ( nIndent * nPPT );
                }
                else
                {
                    nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
                              (long) ( pMargin->GetBottomMargin() * nPPT );

                    if ( bAsianVertical && pDev->GetOutDevType() != OUTDEV_PRINTER )
                    {
                        //  add 1pt extra (default margin value) for line breaks with SetVertical
                        nValue += (long) ( 20 * nPPT );
                    }
                }
            }

            //  EditEngine is cached and re-used, so the old vertical flag must be restored
            pEngine->SetVertical( bEngineVertical );

            pDocument->DisposeFieldEditEngine(pEngine);

            pDev->SetMapMode( aOld );
            pDev->SetFont( aOldFont );
        }

        if (bWidth)
        {
            //      Platz fuer Autofilter-Button
            //      20 * nZoom/100
            //      bedingte Formatierung hier nicht interessant

            sal_Int16 nFlags = ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).GetValue();
            if (nFlags & SC_MF_AUTO)
                nValue += (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();
        }
    }
    return nValue;
}

long ScColumn::GetSimpleTextNeededSize( SCSIZE nIndex, OutputDevice* pDev,
        sal_Bool bWidth )
{
    long nValue=0;
    if ( nIndex < nCount )
    {
        SCROW nRow = pItems[nIndex].nRow;
        const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
        ScBaseCell* pCell = pItems[nIndex].pCell;
        String aValStr;
        Color* pColor;
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        sal_uLong nFormat = pPattern->GetNumberFormat( pFormatter );
        ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
                                    *pFormatter, sal_True, sal_False, ftCheck );
        if ( aValStr.Len() )
        {
            if ( bWidth )
                nValue = pDev->GetTextWidth( aValStr );
            else
                nValue = pDev->GetTextHeight();
        }
    }
    return nValue;
}

sal_uInt16 ScColumn::GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        sal_Bool bFormula, sal_uInt16 nOldWidth,
                                        const ScMarkData* pMarkData,
                                        sal_Bool bSimpleTextImport )
{
    if (nCount == 0)
        return nOldWidth;

    sal_uInt16  nWidth = (sal_uInt16) (nOldWidth * nPPTX);
    sal_Bool    bFound = sal_False;

    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, pMarkData, sal_True);
    if ( bSimpleTextImport )
    {   // alles eins bis auf NumberFormate
        const ScPatternAttr* pPattern = GetPattern( 0 );
        Font aFont;
        // font color doesn't matter here
        pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &rZoomX, NULL );
        pDev->SetFont( aFont );
        const SvxMarginItem* pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        long nMargin = (long) ( pMargin->GetLeftMargin() * nPPTX ) +
                        (long) ( pMargin->GetRightMargin() * nPPTX );

        while (aDataIter.Next( nIndex ))
        {
            sal_uInt16 nThis = (sal_uInt16) (GetSimpleTextNeededSize( nIndex, pDev,
                sal_True ) + nMargin);
            if (nThis)
            {
                if (nThis>nWidth || !bFound)
                {
                    nWidth = nThis;
                    bFound = sal_True;
                }
            }
        }
    }
    else
    {
        ScNeededSizeOptions aOptions;
        aOptions.bFormula = bFormula;
        const ScPatternAttr* pOldPattern = NULL;
        sal_uInt8 nOldScript = 0;

        while (aDataIter.Next( nIndex ))
        {
            SCROW nRow = pItems[nIndex].nRow;

            sal_uInt8 nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
            if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

            const ScPatternAttr* pPattern = GetPattern( nRow );
            aOptions.pPattern = pPattern;
            aOptions.bGetFont = (pPattern != pOldPattern || nScript != nOldScript);
            sal_uInt16 nThis = (sal_uInt16) GetNeededSize( nRow, pDev, nPPTX, nPPTY,
                rZoomX, rZoomY, sal_True, aOptions );
            pOldPattern = pPattern;
            if (nThis)
            {
                if (nThis>nWidth || !bFound)
                {
                    nWidth = nThis;
                    bFound = sal_True;
                }
            }
        }
    }

    if (bFound)
    {
        nWidth += 2;
        sal_uInt16 nTwips = (sal_uInt16) (nWidth / nPPTX);
        return nTwips;
    }
    else
        return nOldWidth;
}

sal_uInt16 lcl_GetAttribHeight( const ScPatternAttr& rPattern, sal_uInt16 nFontHeightId )
{
    sal_uInt16 nHeight = (sal_uInt16) ((const SvxFontHeightItem&) rPattern.GetItem(nFontHeightId)).GetHeight();
    const SvxMarginItem* pMargin = (const SvxMarginItem*) &rPattern.GetItem(ATTR_MARGIN);
    nHeight += nHeight / 5;
    //  gibt bei 10pt 240

    if ( ((const SvxEmphasisMarkItem&)rPattern.
            GetItem(ATTR_FONT_EMPHASISMARK)).GetEmphasisMark() != EMPHASISMARK_NONE )
    {
        //  add height for emphasis marks
        //! font metrics should be used instead
        nHeight += nHeight / 4;
    }

    if ( nHeight + 240 > ScGlobal::nDefFontHeight )
    {
        nHeight = sal::static_int_cast<sal_uInt16>( nHeight + ScGlobal::nDefFontHeight );
        nHeight -= 240;
    }

    //  Standard-Hoehe: TextHeight + Raender - 23
    //  -> 257 unter Windows

    if (nHeight > STD_ROWHEIGHT_DIFF)
        nHeight -= STD_ROWHEIGHT_DIFF;

    nHeight += pMargin->GetTopMargin() + pMargin->GetBottomMargin();

    return nHeight;
}

//  pHeight in Twips
//  nMinHeight, nMinStart zur Optimierung: ab nRow >= nMinStart ist mindestens nMinHeight
//  (wird nur bei bStdAllowed ausgewertet)

void ScColumn::GetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16* pHeight,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                sal_Bool bShrink, sal_uInt16 nMinHeight, SCROW nMinStart )
{
    ScAttrIterator aIter( pAttrArray, nStartRow, nEndRow );

    SCROW nStart = -1;
    SCROW nEnd = -1;
    SCROW nEditPos = 0;
    SCROW nNextEnd = 0;

    //  bei bedingter Formatierung werden immer die einzelnen Zellen angesehen

    const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
    while ( pPattern )
    {
        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
        if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
        {
            //  nix - vertikal bei der zusammengefassten und den ueberdeckten,
            //        horizontal nur bei den ueberdeckten (unsichtbaren) -
            //        eine nur horizontal zusammengefasste wird aber beruecksichtigt
        }
        else
        {
            SCROW nRow = 0;
            sal_Bool bStdAllowed = (pPattern->GetCellOrientation() == SVX_ORIENTATION_STANDARD);
            sal_Bool bStdOnly = sal_False;
            if (bStdAllowed)
            {
                sal_Bool bBreak = ((SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue() ||
                                ((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                                    GetItem( ATTR_HOR_JUSTIFY )).GetValue() ==
                                    SVX_HOR_JUSTIFY_BLOCK);
                bStdOnly = !bBreak;

                // bedingte Formatierung: Zellen durchgehen
                if ( bStdOnly && ((const SfxUInt32Item&)pPattern->
                                    GetItem(ATTR_CONDITIONAL)).GetValue() )
                    bStdOnly = sal_False;

                // gedrehter Text: Zellen durchgehen
                if ( bStdOnly && ((const SfxInt32Item&)pPattern->
                                    GetItem(ATTR_ROTATE_VALUE)).GetValue() )
                    bStdOnly = sal_False;
            }

            if (bStdOnly)
                if (HasEditCells(nStart,nEnd,nEditPos))     // includes mixed script types
                {
                    if (nEditPos == nStart)
                    {
                        bStdOnly = sal_False;
                        if (nEnd > nEditPos)
                            nNextEnd = nEnd;
                        nEnd = nEditPos;                // einzeln ausrechnen
                        bStdAllowed = sal_False;            // wird auf jeden Fall per Zelle berechnet
                    }
                    else
                    {
                        nNextEnd = nEnd;
                        nEnd = nEditPos - 1;            // Standard - Teil
                    }
                }

            if (bStdAllowed)
            {
                sal_uInt16 nLatHeight = 0;
                sal_uInt16 nCjkHeight = 0;
                sal_uInt16 nCtlHeight = 0;
                sal_uInt16 nDefHeight;
                sal_uInt8 nDefScript = ScGlobal::GetDefaultScriptType();
                if ( nDefScript == SCRIPTTYPE_ASIAN )
                    nDefHeight = nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
                else if ( nDefScript == SCRIPTTYPE_COMPLEX )
                    nDefHeight = nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
                else
                    nDefHeight = nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );

                //  if everything below is already larger, the loop doesn't have to
                //  be run again
                SCROW nStdEnd = nEnd;
                if ( nDefHeight <= nMinHeight && nStdEnd >= nMinStart )
                    nStdEnd = (nMinStart>0) ? nMinStart-1 : 0;

                for (nRow=nStart; nRow<=nStdEnd; nRow++)
                    if (nDefHeight > pHeight[nRow-nStartRow])
                        pHeight[nRow-nStartRow] = nDefHeight;

                if ( bStdOnly )
                {
                    //  if cells are not handled individually below,
                    //  check for cells with different script type

                    SCSIZE nIndex;
                    Search(nStart,nIndex);
                    while ( nIndex < nCount && (nRow=pItems[nIndex].nRow) <= nEnd )
                    {
                        sal_uInt8 nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
                        if ( nScript != nDefScript )
                        {
                            if ( nScript == SCRIPTTYPE_ASIAN )
                            {
                                if ( nCjkHeight == 0 )
                                    nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
                                if (nCjkHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nCjkHeight;
                            }
                            else if ( nScript == SCRIPTTYPE_COMPLEX )
                            {
                                if ( nCtlHeight == 0 )
                                    nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
                                if (nCtlHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nCtlHeight;
                            }
                            else
                            {
                                if ( nLatHeight == 0 )
                                    nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );
                                if (nLatHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nLatHeight;
                            }
                        }
                        ++nIndex;
                    }
                }
            }

            if (!bStdOnly)                      // belegte Zellen suchen
            {
                ScNeededSizeOptions aOptions;

                SCSIZE nIndex;
                Search(nStart,nIndex);
                while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEnd) : sal_False )
                {
                    //  Zellhoehe nur berechnen, wenn sie spaeter auch gebraucht wird (#37928#)

                    if ( bShrink || !(pDocument->GetRowFlags(nRow, nTab) & CR_MANUALSIZE) )
                    {
                        aOptions.pPattern = pPattern;
                        sal_uInt16 nHeight = (sal_uInt16)
                                ( GetNeededSize( nRow, pDev, nPPTX, nPPTY,
                                                    rZoomX, rZoomY, sal_False, aOptions ) / nPPTY );
                        if (nHeight > pHeight[nRow-nStartRow])
                            pHeight[nRow-nStartRow] = nHeight;
                    }
                    ++nIndex;
                }
            }
        }

        if (nNextEnd > 0)
        {
            nStart = nEnd + 1;
            nEnd = nNextEnd;
            nNextEnd = 0;
        }
        else
            pPattern = aIter.Next(nStart,nEnd);
    }
}

sal_Bool ScColumn::GetNextSpellingCell(SCROW& nRow, sal_Bool bInSel, const ScMarkData& rData) const
{
    sal_Bool bStop = sal_False;
    CellType eCellType;
    SCSIZE nIndex;
    if (!bInSel && Search(nRow, nIndex))
    {
        eCellType = GetCellType(nRow);
        if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
             !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
               pDocument->IsTabProtected(nTab)) )
                return sal_True;
    }
    while (!bStop)
    {
        if (bInSel)
        {
            nRow = rData.GetNextMarked(nCol, nRow, sal_False);
            if (!ValidRow(nRow))
            {
                nRow = MAXROW+1;
                bStop = sal_True;
            }
            else
            {
                eCellType = GetCellType(nRow);
                if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
                     !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                       pDocument->IsTabProtected(nTab)) )
                        return sal_True;
                else
                    nRow++;
            }
        }
        else if (GetNextDataPos(nRow))
        {
            eCellType = GetCellType(nRow);
            if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
                 !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                   pDocument->IsTabProtected(nTab)) )
                    return sal_True;
            else
                nRow++;
        }
        else
        {
            nRow = MAXROW+1;
            bStop = sal_True;
        }
    }
    return sal_False;
}

// =========================================================================================

void ScColumn::RemoveAutoSpellObj()
{
    ScTabEditEngine* pEngine = NULL;

    for (SCSIZE i=0; i<nCount; i++)
        if ( pItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) pItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();
            //  keine Abfrage auf HasOnlineSpellErrors, damit es auch
            //  nach dem Laden funktioniert

            //  Fuer den Test auf harte Formatierung (ScEditAttrTester) sind die Defaults
            //  in der EditEngine unwichtig. Wenn der Tester spaeter einmal gleiche
            //  Attribute in Default und harter Formatierung erkennen und weglassen sollte,
            //  muessten an der EditEngine zu jeder Zelle die richtigen Defaults gesetzt
            //  werden!

            //  auf Attribute testen
            if ( !pEngine )
                pEngine = new ScTabEditEngine(pDocument);
            pEngine->SetText( *pData );
            ScEditAttrTester aTester( pEngine );
            if ( aTester.NeedsObject() )                    // nur Spell-Errors entfernen
            {
                EditTextObject* pNewData = pEngine->CreateTextObject(); // ohne BIGOBJ
                pOldCell->SetData( pNewData, pEngine->GetEditTextObjectPool() );
                delete pNewData;
            }
            else                                            // String erzeugen
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
                pNewCell->TakeNote( pOldCell->ReleaseNote() );
                pItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

void ScColumn::RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow )
{
    ScFieldEditEngine* pEngine = NULL;

    SCSIZE i;
    Search( nStartRow, i );
    for (; i<nCount && pItems[i].nRow <= nEndRow; i++)
        if ( pItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) pItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();

            //  Fuer den Test auf harte Formatierung (ScEditAttrTester) sind die Defaults
            //  in der EditEngine unwichtig. Wenn der Tester spaeter einmal gleiche
            //  Attribute in Default und harter Formatierung erkennen und weglassen sollte,
            //  muessten an der EditEngine zu jeder Zelle die richtigen Defaults gesetzt
            //  werden!

            //  auf Attribute testen
            if ( !pEngine )
            {
                //pEngine = new ScTabEditEngine(pDocument);
                pEngine = new ScFieldEditEngine( pDocument->GetEditPool() );
                //  EE_CNTRL_ONLINESPELLING falls schon Fehler drin sind
                pEngine->SetControlWord( pEngine->GetControlWord() | EE_CNTRL_ONLINESPELLING );
                pDocument->ApplyAsianEditSettings( *pEngine );
            }
            pEngine->SetText( *pData );
            sal_uInt16 nParCount = pEngine->GetParagraphCount();
            for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
            {
                pEngine->QuickRemoveCharAttribs( nPar );
                const SfxItemSet& rOld = pEngine->GetParaAttribs( nPar );
                if ( rOld.Count() )
                {
                    SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   // leer
                    pEngine->SetParaAttribs( nPar, aNew );
                }
            }
            //  URL-Felder in Text wandeln (andere gibt's nicht, darum pType=0)
            pEngine->RemoveFields( sal_True );

            sal_Bool bSpellErrors = pEngine->HasOnlineSpellErrors();
            sal_Bool bNeedObject = bSpellErrors || nParCount>1;         // Errors/Absaetze behalten
            //  ScEditAttrTester nicht mehr noetig, Felder sind raus

            if ( bNeedObject )                                      // bleibt Edit-Zelle
            {
                sal_uLong nCtrl = pEngine->GetControlWord();
                sal_uLong nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
                if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                    pEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );
                EditTextObject* pNewData = pEngine->CreateTextObject();
                pOldCell->SetData( pNewData, pEngine->GetEditTextObjectPool() );
                delete pNewData;
            }
            else                                            // String erzeugen
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
                pNewCell->TakeNote( pOldCell->ReleaseNote() );
                pItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

// =========================================================================================

sal_Bool ScColumn::TestTabRefAbs(SCTAB nTable)
{
    sal_Bool bRet = sal_False;
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
            if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pItems[i].pCell)->TestTabRefAbs(nTable))
                    bRet = sal_True;
    return bRet;
}

// =========================================================================================

ScColumnIterator::ScColumnIterator( const ScColumn* pCol, SCROW nStart, SCROW nEnd ) :
    pColumn( pCol ),
    nTop( nStart ),
    nBottom( nEnd )
{
    pColumn->Search( nTop, nPos );
}

ScColumnIterator::~ScColumnIterator()
{
}

sal_Bool ScColumnIterator::Next( SCROW& rRow, ScBaseCell*& rpCell )
{
    if ( nPos < pColumn->nCount )
    {
        rRow = pColumn->pItems[nPos].nRow;
        if ( rRow <= nBottom )
        {
            rpCell = pColumn->pItems[nPos].pCell;
            ++nPos;
            return sal_True;
        }
    }

    rRow = 0;
    rpCell = NULL;
    return sal_False;
}

SCSIZE ScColumnIterator::GetIndex() const           // Index zur letzen abgefragten Zelle
{
    return nPos - 1;        // bei Next ist Pos hochgezaehlt worden
}

// -----------------------------------------------------------------------------------------

ScMarkedDataIter::ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    sal_Bool bAllIfNone ) :
    pColumn( pCol ),
    pMarkIter( NULL ),
    bNext( sal_True ),
    bAll( bAllIfNone )
{
    if (pMarkData && pMarkData->IsMultiMarked())
        pMarkIter = new ScMarkArrayIter( pMarkData->GetArray() + pCol->GetCol() );
}

ScMarkedDataIter::~ScMarkedDataIter()
{
    delete pMarkIter;
}

sal_Bool ScMarkedDataIter::Next( SCSIZE& rIndex )
{
    sal_Bool bFound = sal_False;
    do
    {
        if (bNext)
        {
            if (!pMarkIter || !pMarkIter->Next( nTop, nBottom ))
            {
                if (bAll)                   // ganze Spalte
                {
                    nTop    = 0;
                    nBottom = MAXROW;
                }
                else
                    return sal_False;
            }
            pColumn->Search( nTop, nPos );
            bNext = sal_False;
            bAll  = sal_False;                  // nur beim ersten Versuch
        }

        if ( nPos >= pColumn->nCount )
            return sal_False;

        if ( pColumn->pItems[nPos].nRow <= nBottom )
            bFound = sal_True;
        else
            bNext = sal_True;
    }
    while (!bFound);

    rIndex = nPos++;
    return sal_True;
}

//UNUSED2009-05 sal_uInt16 ScColumn::GetErrorData( SCROW nRow ) const
//UNUSED2009-05 {
//UNUSED2009-05     SCSIZE  nIndex;
//UNUSED2009-05     if (Search(nRow, nIndex))
//UNUSED2009-05     {
//UNUSED2009-05         ScBaseCell* pCell = pItems[nIndex].pCell;
//UNUSED2009-05         switch (pCell->GetCellType())
//UNUSED2009-05         {
//UNUSED2009-05             case CELLTYPE_FORMULA :
//UNUSED2009-05                 return ((ScFormulaCell*)pCell)->GetErrCode();
//UNUSED2009-05 //            break;
//UNUSED2009-05             default:
//UNUSED2009-05             return 0;
//UNUSED2009-05         }
//UNUSED2009-05     }
//UNUSED2009-05     return 0;
//UNUSED2009-05 }

//------------

sal_Bool ScColumn::IsEmptyData() const
{
    return (nCount == 0);
}

sal_Bool ScColumn::IsEmptyVisData(sal_Bool bNotes) const
{
    if (!pItems || nCount == 0)
        return sal_True;
    else
    {
        sal_Bool bVisData = sal_False;
        SCSIZE i;
        for (i=0; i<nCount && !bVisData; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->HasNote()) )
                bVisData = sal_True;
        }
        return !bVisData;
    }
}

SCSIZE ScColumn::VisibleCount( SCROW nStartRow, SCROW nEndRow ) const
{
    //  Notizen werden nicht mitgezaehlt

    SCSIZE nVisCount = 0;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
    {
        if ( pItems[nIndex].nRow >= nStartRow &&
             pItems[nIndex].pCell->GetCellType() != CELLTYPE_NOTE )
        {
            ++nVisCount;
        }
        ++nIndex;
    }
    return nVisCount;
}

SCROW ScColumn::GetLastVisDataPos(sal_Bool bNotes) const
{
    SCROW nRet = 0;
    if (pItems)
    {
        SCSIZE i;
        sal_Bool bFound = sal_False;
        for (i=nCount; i>0 && !bFound; )
        {
            --i;
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->HasNote()) )
            {
                bFound = sal_True;
                nRet = pItems[i].nRow;
            }
        }
    }
    return nRet;
}

SCROW ScColumn::GetFirstVisDataPos(sal_Bool bNotes) const
{
    SCROW nRet = 0;
    if (pItems)
    {
        SCSIZE i;
        sal_Bool bFound = sal_False;
        for (i=0; i<nCount && !bFound; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->HasNote()) )
            {
                bFound = sal_True;
                nRet = pItems[i].nRow;
            }
        }
    }
    return nRet;
}

sal_Bool ScColumn::HasVisibleDataAt(SCROW nRow) const
{
    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (!pItems[nIndex].pCell->IsBlank())
            return sal_True;

    return sal_False;
}

sal_Bool ScColumn::IsEmptyAttr() const
{
    if (pAttrArray)
        return pAttrArray->IsEmpty();
    else
        return sal_True;
}

sal_Bool ScColumn::IsEmpty() const
{
    return (IsEmptyData() && IsEmptyAttr());
}

sal_Bool ScColumn::IsEmptyBlock(SCROW nStartRow, SCROW nEndRow, bool bIgnoreNotes) const
{
    if ( nCount == 0 || !pItems )
        return sal_True;

    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
    {
        if ( !pItems[nIndex].pCell->IsBlank( bIgnoreNotes ) )   // found a cell
            return sal_False;                           // not empty
        ++nIndex;
    }
    return sal_True;                                    // no cell found
}

SCSIZE ScColumn::GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const
{
    SCSIZE nLines = 0;
    sal_Bool bFound = sal_False;
    SCSIZE i;
    if (pItems && (nCount > 0))
    {
        if (eDir == DIR_BOTTOM)
        {
            i = nCount;
            while (!bFound && (i > 0))
            {
                i--;
                if ( pItems[i].nRow < nStartRow )
                    break;
                bFound = pItems[i].nRow <= nEndRow && !pItems[i].pCell->IsBlank();
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(nEndRow - pItems[i].nRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
        else if (eDir == DIR_TOP)
        {
            i = 0;
            while (!bFound && (i < nCount))
            {
                if ( pItems[i].nRow > nEndRow )
                    break;
                bFound = pItems[i].nRow >= nStartRow && !pItems[i].pCell->IsBlank();
                i++;
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(pItems[i-1].nRow - nStartRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
    }
    else
        nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
    return nLines;
}

SCROW ScColumn::GetFirstDataPos() const
{
    if (nCount)
        return pItems[0].nRow;
    else
        return 0;
}

SCROW ScColumn::GetLastDataPos() const
{
    if (nCount)
        return pItems[nCount-1].nRow;
    else
        return 0;
}

sal_Bool ScColumn::GetPrevDataPos(SCROW& rRow) const
{
    sal_Bool bFound = sal_False;
    SCSIZE i = nCount;
    while (!bFound && (i > 0))
    {
        --i;
        bFound = (pItems[i].nRow < rRow);
        if (bFound)
            rRow = pItems[i].nRow;
    }
    return bFound;
}

sal_Bool ScColumn::GetNextDataPos(SCROW& rRow) const        // greater than rRow
{
    SCSIZE nIndex;
    if (Search( rRow, nIndex ))
        ++nIndex;                   // next cell

    sal_Bool bMore = ( nIndex < nCount );
    if ( bMore )
        rRow = pItems[nIndex].nRow;
    return bMore;
}

void ScColumn::FindDataAreaPos(SCROW& rRow, long nMovY) const
{
    if (!nMovY) return;
    sal_Bool bForward = (nMovY>0);

    SCSIZE nIndex;
    sal_Bool bThere = Search(rRow, nIndex);
    if (bThere && pItems[nIndex].pCell->IsBlank())
        bThere = sal_False;

    if (bThere)
    {
        SCROW nLast = rRow;
        SCSIZE nOldIndex = nIndex;
        if (bForward)
        {
            if (nIndex<nCount-1)
            {
                ++nIndex;
                while (nIndex<nCount-1 && pItems[nIndex].nRow==nLast+1
                                        && !pItems[nIndex].pCell->IsBlank())
                {
                    ++nIndex;
                    ++nLast;
                }
                if (nIndex==nCount-1)
                    if (pItems[nIndex].nRow==nLast+1 && !pItems[nIndex].pCell->IsBlank())
                        ++nLast;
            }
        }
        else
        {
            if (nIndex>0)
            {
                --nIndex;
                while (nIndex>0 && pItems[nIndex].nRow+1==nLast
                                        && !pItems[nIndex].pCell->IsBlank())
                {
                    --nIndex;
                    --nLast;
                }
                if (nIndex==0)
                    if (pItems[nIndex].nRow+1==nLast && !pItems[nIndex].pCell->IsBlank())
                        --nLast;
            }
        }
        if (nLast==rRow)
        {
            bThere = sal_False;
            nIndex = bForward ? nOldIndex+1 : nOldIndex;
        }
        else
            rRow = nLast;
    }

    if (!bThere)
    {
        if (bForward)
        {
            while (nIndex<nCount && pItems[nIndex].pCell->IsBlank())
                ++nIndex;
            if (nIndex<nCount)
                rRow = pItems[nIndex].nRow;
            else
                rRow = MAXROW;
        }
        else
        {
            while (nIndex>0 && pItems[nIndex-1].pCell->IsBlank())
                --nIndex;
            if (nIndex>0)
                rRow = pItems[nIndex-1].nRow;
            else
                rRow = 0;
        }
    }
}

sal_Bool ScColumn::HasDataAt(SCROW nRow) const
{
/*  SCSIZE nIndex;
    return Search( nRow, nIndex );
*/
        //  immer nur sichtbare interessant ?
        //! dann HasVisibleDataAt raus

    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (!pItems[nIndex].pCell->IsBlank())
            return sal_True;

    return sal_False;

}

sal_Bool ScColumn::IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray && rCol.pAttrArray)
        return pAttrArray->IsAllEqual( *rCol.pAttrArray, nStartRow, nEndRow );
    else
        return !pAttrArray && !rCol.pAttrArray;
}

sal_Bool ScColumn::IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray && rCol.pAttrArray)
        return pAttrArray->IsVisibleEqual( *rCol.pAttrArray, nStartRow, nEndRow );
    else
        return !pAttrArray && !rCol.pAttrArray;
}

sal_Bool ScColumn::GetFirstVisibleAttr( SCROW& rFirstRow ) const
{
    if (pAttrArray)
        return pAttrArray->GetFirstVisibleAttr( rFirstRow );
    else
        return sal_False;
}

sal_Bool ScColumn::GetLastVisibleAttr( SCROW& rLastRow ) const
{
    if (pAttrArray)
    {
        // row of last cell is needed
        SCROW nLastData = GetLastVisDataPos( sal_True );    // always including notes, 0 if none

        return pAttrArray->GetLastVisibleAttr( rLastRow, nLastData );
    }
    else
        return sal_False;
}

sal_Bool ScColumn::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray)
        return pAttrArray->HasVisibleAttrIn( nStartRow, nEndRow );
    else
        return sal_False;
}

void ScColumn::FindUsed( SCROW nStartRow, SCROW nEndRow, sal_Bool* pUsed ) const
{
    SCROW nRow = 0;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEndRow) : sal_False )
    {
        pUsed[nRow-nStartRow] = sal_True;
        ++nIndex;
    }
}

void ScColumn::StartListening( SvtListener& rLst, SCROW nRow )
{
    SvtBroadcaster* pBC = NULL;
    ScBaseCell* pCell;

    SCSIZE nIndex;
    if (Search(nRow,nIndex))
    {
        pCell = pItems[nIndex].pCell;
        pBC = pCell->GetBroadcaster();
    }
    else
    {
        pCell = new ScNoteCell;
        Insert(nRow, pCell);
    }

    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        pCell->TakeBroadcaster(pBC);
    }
    rLst.StartListening(*pBC);
}

void ScColumn::MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow )
{
    SvtBroadcaster* pBC = NULL;
    ScBaseCell* pCell;

    SCSIZE nIndex;
    if (Search(nDestRow,nIndex))
    {
        pCell = pItems[nIndex].pCell;
        pBC = pCell->GetBroadcaster();
    }
    else
    {
        pCell = new ScNoteCell;
        Insert(nDestRow, pCell);
    }

    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        pCell->TakeBroadcaster(pBC);
    }

    if (rSource.HasListeners())
    {
        SvtListenerIter aIter( rSource);
        for (SvtListener* pLst = aIter.GoStart(); pLst; pLst = aIter.GoNext())
        {
            pLst->StartListening( *pBC);
            pLst->EndListening( rSource);
        }
    }
}

void ScColumn::EndListening( SvtListener& rLst, SCROW nRow )
{
    SCSIZE nIndex;
    if (Search(nRow,nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        SvtBroadcaster* pBC = pCell->GetBroadcaster();
        if (pBC)
        {
            rLst.EndListening(*pBC);

            if (!pBC->HasListeners())
            {
                if (pCell->IsBlank())
                    DeleteAtIndex(nIndex);
                else
                    pCell->DeleteBroadcaster();
            }
        }
//      else
//          DBG_ERROR("ScColumn::EndListening - kein Broadcaster");
    }
//  else
//      DBG_ERROR("ScColumn::EndListening - keine Zelle");
}

void ScColumn::CompileDBFormula()
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula();
        }
}

void ScColumn::CompileDBFormula( sal_Bool bCreateFormulaString )
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileNameFormula( sal_Bool bCreateFormulaString )
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileNameFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileColRowNameFormula()
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileColRowNameFormula();
        }
}

void lcl_UpdateSubTotal( ScFunctionData& rData, ScBaseCell* pCell )
{
    double nValue = 0.0;
    sal_Bool bVal = sal_False;
    sal_Bool bCell = sal_True;
    switch (pCell->GetCellType())
    {
        case CELLTYPE_VALUE:
            nValue = ((ScValueCell*)pCell)->GetValue();
            bVal = sal_True;
            break;
        case CELLTYPE_FORMULA:
            {
                if ( rData.eFunc != SUBTOTAL_FUNC_CNT2 )        // da interessiert's nicht
                {
                    ScFormulaCell* pFC = (ScFormulaCell*)pCell;
                    if ( pFC->GetErrCode() )
                    {
                        if ( rData.eFunc != SUBTOTAL_FUNC_CNT ) // fuer Anzahl einfach weglassen
                            rData.bError = sal_True;
                    }
                    else if (pFC->IsValue())
                    {
                        nValue = pFC->GetValue();
                        bVal = sal_True;
                    }
                    // sonst Text
                }
            }
            break;
        case CELLTYPE_NOTE:
            bCell = sal_False;
            break;
        // bei Strings nichts
        default:
        {
            // added to avoid warnings
        }
    }

    if (!rData.bError)
    {
        switch (rData.eFunc)
        {
            case SUBTOTAL_FUNC_SUM:
            case SUBTOTAL_FUNC_AVE:
                if (bVal)
                {
                    ++rData.nCount;
                    if (!SubTotal::SafePlus( rData.nVal, nValue ))
                        rData.bError = sal_True;
                }
                break;
            case SUBTOTAL_FUNC_CNT:             // nur Werte
                if (bVal)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_CNT2:            // alle
                if (bCell)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_MAX:
                if (bVal)
                    if (++rData.nCount == 1 || nValue > rData.nVal )
                        rData.nVal = nValue;
                break;
            case SUBTOTAL_FUNC_MIN:
                if (bVal)
                    if (++rData.nCount == 1 || nValue < rData.nVal )
                        rData.nVal = nValue;
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

//  Mehrfachselektion:
void ScColumn::UpdateSelectionFunction( const ScMarkData& rMark,
                                        ScFunctionData& rData,
                                        ScFlatBoolRowSegments& rHiddenRows,
                                        sal_Bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow )
{
    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, &rMark, sal_False);
    while (aDataIter.Next( nIndex ))
    {
        SCROW nRow = pItems[nIndex].nRow;
        bool bRowHidden = rHiddenRows.getValue(nRow);
        if ( !bRowHidden )
            if ( !bDoExclude || nRow < nExStartRow || nRow > nExEndRow )
                lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
    }
}

//  bei bNoMarked die Mehrfachselektion weglassen
void ScColumn::UpdateAreaFunction( ScFunctionData& rData,
                                   ScFlatBoolRowSegments& rHiddenRows,
                                    SCROW nStartRow, SCROW nEndRow )
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex<nCount && pItems[nIndex].nRow<=nEndRow )
    {
        SCROW nRow = pItems[nIndex].nRow;
        bool bRowHidden = rHiddenRows.getValue(nRow);
        if ( !bRowHidden )
            lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
        ++nIndex;
    }
}

sal_uLong ScColumn::GetWeightedCount() const
{
    sal_uLong nTotal = 0;

    //  Notizen werden nicht gezaehlt

    for (SCSIZE i=0; i<nCount; i++)
    {
        ScBaseCell* pCell = pItems[i].pCell;
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE:
            case CELLTYPE_STRING:
                ++nTotal;
                break;
            case CELLTYPE_FORMULA:
                nTotal += 5 + ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
                break;
            case CELLTYPE_EDIT:
                nTotal += 50;
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    return nTotal;
}

sal_uLong ScColumn::GetCodeCount() const
{
    sal_uLong nCodeCount = 0;

    for (SCSIZE i=0; i<nCount; i++)
    {
        ScBaseCell* pCell = pItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            nCodeCount += ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
    }

    return nCodeCount;
}





