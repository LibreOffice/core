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

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/flagitem.hxx>
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/misccfg.hxx>
#include <sfx2/app.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/securityoptions.hxx>

#include <vcl/virdev.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

#include "inputopt.hxx"
#include "global.hxx"
#include "table.hxx"
#include "column.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "rechead.hxx"
#include "ddelink.hxx"
#include "scmatrix.hxx"
#include "arealink.hxx"
#include "dociter.hxx"
#include "patattr.hxx"
#include "hints.hxx"
#include "editutil.hxx"
#include "progress.hxx"
#include "document.hxx"
#include "chartlis.hxx"
#include "chartlock.hxx"
#include "refupdat.hxx"
#include "validat.hxx"      // fuer HasMacroCalls
#include "markdata.hxx"
#include "scmod.hxx"
#include "printopt.hxx"
#include "externalrefmgr.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "charthelper.hxx"
#include "macromgr.hxx"
#include "dpobject.hxx"
#include "docuno.hxx"
#include "scresid.hxx"
#include "columniterator.hxx"
#include "globalnames.hxx"
#include "stringutil.hxx"

#include <memory>
#include <boost/scoped_ptr.hpp>

//  states for online spelling in the visible range (0 is set initially)
#define VSPL_START  0
#define VSPL_DONE   1

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

namespace {

inline sal_uInt16 getScaleValue(SfxStyleSheetBase& rStyle, sal_uInt16 nWhich)
{
    return static_cast<const SfxUInt16Item&>(rStyle.GetItemSet().Get(nWhich)).GetValue();
}

}

void ScDocument::ImplCreateOptions()
{
    pDocOptions  = new ScDocOptions();
    pViewOptions = new ScViewOptions();
}

//------------------------------------------------------------------------

void ScDocument::ImplDeleteOptions()
{
    delete pDocOptions;
    delete pViewOptions;
    delete pExtDocOptions;
}

//------------------------------------------------------------------------

SfxPrinter* ScDocument::GetPrinter(bool bCreateIfNotExist)
{
    if ( !pPrinter && bCreateIfNotExist )
    {
        SfxItemSet* pSet =
            new SfxItemSet( *xPoolHelper->GetDocPool(),
                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            SID_PRINT_SELECTEDSHEET,    SID_PRINT_SELECTEDSHEET,
                            SID_SCPRINTOPTIONS,         SID_SCPRINTOPTIONS,
                            NULL );

        ::utl::MiscCfg aMisc;
        sal_uInt16 nFlags = 0;
        if ( aMisc.IsPaperOrientationWarning() )
            nFlags |= SFX_PRINTER_CHG_ORIENTATION;
        if ( aMisc.IsPaperSizeWarning() )
            nFlags |= SFX_PRINTER_CHG_SIZE;
        pSet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlags ) );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning() ) );

        pPrinter = new SfxPrinter( pSet );
        pPrinter->SetMapMode( MAP_100TH_MM );
        UpdateDrawPrinter();
        pPrinter->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
    }

    return pPrinter;
}

//------------------------------------------------------------------------

void ScDocument::SetPrinter( SfxPrinter* pNewPrinter )
{
    if ( pNewPrinter == pPrinter )
    {
        //  #i6706# SetPrinter is called with the same printer again if
        //  the JobSetup has changed. In that case just call UpdateDrawPrinter
        //  (SetRefDevice for drawing layer) because of changed text sizes.
        UpdateDrawPrinter();
    }
    else
    {
        SfxPrinter* pOld = pPrinter;
        pPrinter = pNewPrinter;
        UpdateDrawPrinter();
        pPrinter->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
        delete pOld;
    }
    InvalidateTextWidth(NULL, NULL, false);     // in both cases
}

//------------------------------------------------------------------------

void ScDocument::SetPrintOptions()
{
    if ( !pPrinter ) GetPrinter(); // setzt pPrinter
    OSL_ENSURE( pPrinter, "Error in printer creation :-/" );

    if ( pPrinter )
    {
        ::utl::MiscCfg aMisc;
        SfxItemSet aOptSet( pPrinter->GetOptions() );

        sal_uInt16 nFlags = 0;
        if ( aMisc.IsPaperOrientationWarning() )
            nFlags |= SFX_PRINTER_CHG_ORIENTATION;
        if ( aMisc.IsPaperSizeWarning() )
            nFlags |= SFX_PRINTER_CHG_SIZE;
        aOptSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlags ) );
        aOptSet.Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning() ) );

        pPrinter->SetOptions( aOptSet );
    }
}

//------------------------------------------------------------------------

VirtualDevice* ScDocument::GetVirtualDevice_100th_mm()
{
    if (!pVirtualDevice_100th_mm)
    {
#ifdef IOS
        pVirtualDevice_100th_mm = new VirtualDevice( 8 );
#else
        pVirtualDevice_100th_mm = new VirtualDevice( 1 );
#endif
        pVirtualDevice_100th_mm->SetReferenceDevice(VirtualDevice::REFDEV_MODE_MSO1);
        MapMode aMapMode( pVirtualDevice_100th_mm->GetMapMode() );
        aMapMode.SetMapUnit( MAP_100TH_MM );
        pVirtualDevice_100th_mm->SetMapMode( aMapMode );
    }
    return pVirtualDevice_100th_mm;
}

OutputDevice* ScDocument::GetRefDevice()
{
    // Create printer like ref device, see Writer...
    OutputDevice* pRefDevice = NULL;
    if ( SC_MOD()->GetInputOptions().GetTextWysiwyg() )
        pRefDevice = GetPrinter();
    else
        pRefDevice = GetVirtualDevice_100th_mm();
    return pRefDevice;
}

//------------------------------------------------------------------------

void ScDocument::ModifyStyleSheet( SfxStyleSheetBase& rStyleSheet,
                                   const SfxItemSet&  rChanges )
{
    SfxItemSet& rSet = rStyleSheet.GetItemSet();

    switch ( rStyleSheet.GetFamily() )
    {
        case SFX_STYLE_FAMILY_PAGE:
            {
                const sal_uInt16 nOldScale = getScaleValue(rStyleSheet, ATTR_PAGE_SCALE);
                const sal_uInt16 nOldScaleToPages = getScaleValue(rStyleSheet, ATTR_PAGE_SCALETOPAGES);
                rSet.Put( rChanges );
                const sal_uInt16 nNewScale        = getScaleValue(rStyleSheet, ATTR_PAGE_SCALE);
                const sal_uInt16 nNewScaleToPages = getScaleValue(rStyleSheet, ATTR_PAGE_SCALETOPAGES);

                if ( (nOldScale != nNewScale) || (nOldScaleToPages != nNewScaleToPages) )
                    InvalidateTextWidth( rStyleSheet.GetName() );

                if( SvtLanguageOptions().IsCTLFontEnabled() )
                {
                    const SfxPoolItem *pItem = NULL;
                    if( rChanges.GetItemState(ATTR_WRITINGDIR, true, &pItem ) == SFX_ITEM_SET )
                        ScChartHelper::DoUpdateAllCharts( this );
                }
            }
            break;

        case SFX_STYLE_FAMILY_PARA:
            {
                bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rSet, rChanges ) )
                    InvalidateTextWidth( NULL, NULL, bNumFormatChanged );

                for (SCTAB nTab=0; nTab<=MAXTAB; ++nTab)
                    if (maTabs[nTab] && maTabs[nTab]->IsStreamValid())
                        maTabs[nTab]->SetStreamValid( false );

                sal_uLong nOldFormat =
                    ((const SfxUInt32Item*)&rSet.Get(
                    ATTR_VALUE_FORMAT ))->GetValue();
                sal_uLong nNewFormat =
                    ((const SfxUInt32Item*)&rChanges.Get(
                    ATTR_VALUE_FORMAT ))->GetValue();
                LanguageType eNewLang, eOldLang;
                eNewLang = eOldLang = LANGUAGE_DONTKNOW;
                if ( nNewFormat != nOldFormat )
                {
                    SvNumberFormatter* pFormatter = GetFormatTable();
                    eOldLang = pFormatter->GetEntry( nOldFormat )->GetLanguage();
                    eNewLang = pFormatter->GetEntry( nNewFormat )->GetLanguage();
                }

                // Bedeutung der Items in rChanges:
                //  Item gesetzt    - Aenderung uebernehmen
                //  Dontcare        - Default setzen
                //  Default         - keine Aenderung
                // ("keine Aenderung" geht nicht mit PutExtended, darum Schleife)
                for (sal_uInt16 nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; nWhich++)
                {
                    const SfxPoolItem* pItem;
                    SfxItemState eState = rChanges.GetItemState( nWhich, false, &pItem );
                    if ( eState == SFX_ITEM_SET )
                        rSet.Put( *pItem );
                    else if ( eState == SFX_ITEM_DONTCARE )
                        rSet.ClearItem( nWhich );
                    // bei Default nichts
                }

                if ( eNewLang != eOldLang )
                    rSet.Put(
                        SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }
}

//------------------------------------------------------------------------

void ScDocument::CopyStdStylesFrom( ScDocument* pSrcDoc )
{
    // number format exchange list has to be handled here, too
    NumFmtMergeHandler aNumFmtMergeHdl(this, pSrcDoc);
    xPoolHelper->GetStylePool()->CopyStdStylesFrom( pSrcDoc->xPoolHelper->GetStylePool() );
}

//------------------------------------------------------------------------

void ScDocument::InvalidateTextWidth( const OUString& rStyleName )
{
    const SCTAB nCount = GetTableCount();
    for ( SCTAB i=0; i<nCount && maTabs[i]; i++ )
        if ( maTabs[i]->GetPageStyle() == rStyleName )
            InvalidateTextWidth( i );
}

//------------------------------------------------------------------------

void ScDocument::InvalidateTextWidth( SCTAB nTab )
{
    ScAddress aAdrFrom( 0,    0,        nTab );
    ScAddress aAdrTo  ( MAXCOL, MAXROW, nTab );
    InvalidateTextWidth( &aAdrFrom, &aAdrTo, false );
}

//------------------------------------------------------------------------

bool ScDocument::IsPageStyleInUse( const OUString& rStrPageStyle, SCTAB* pInTab )
{
    bool         bInUse = false;
    const SCTAB nCount = GetTableCount();
    SCTAB i;

    for ( i = 0; !bInUse && i < nCount && maTabs[i]; i++ )
        bInUse = ( maTabs[i]->GetPageStyle() == rStrPageStyle );

    if ( pInTab )
        *pInTab = i-1;

    return bInUse;
}

//------------------------------------------------------------------------

bool ScDocument::RemovePageStyleInUse( const OUString& rStyle )
{
    bool bWasInUse = false;
    const SCTAB nCount = GetTableCount();

    for ( SCTAB i=0; i<nCount && maTabs[i]; i++ )
        if ( maTabs[i]->GetPageStyle() == rStyle )
        {
            bWasInUse = true;
            maTabs[i]->SetPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
        }

    return bWasInUse;
}

bool ScDocument::RenamePageStyleInUse( const OUString& rOld, const OUString& rNew )
{
    bool bWasInUse = false;
    const SCTAB nCount = GetTableCount();

    for ( SCTAB i=0; i<nCount && maTabs[i]; i++ )
        if ( maTabs[i]->GetPageStyle() == rOld )
        {
            bWasInUse = true;
            maTabs[i]->SetPageStyle( rNew );
        }

    return bWasInUse;
}

//------------------------------------------------------------------------

sal_uInt8 ScDocument::GetEditTextDirection(SCTAB nTab) const
{
    EEHorizontalTextDirection eRet = EE_HTEXTDIR_DEFAULT;

    OUString aStyleName = GetPageStyle( nTab );
    SfxStyleSheetBase* pStyle = xPoolHelper->GetStylePool()->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    if ( pStyle )
    {
        SfxItemSet& rStyleSet = pStyle->GetItemSet();
        SvxFrameDirection eDirection = (SvxFrameDirection)
            ((const SvxFrameDirectionItem&)rStyleSet.Get( ATTR_WRITINGDIR )).GetValue();

        if ( eDirection == FRMDIR_HORI_LEFT_TOP )
            eRet = EE_HTEXTDIR_L2R;
        else if ( eDirection == FRMDIR_HORI_RIGHT_TOP )
            eRet = EE_HTEXTDIR_R2L;
        // else (invalid for EditEngine): keep "default"
    }

    return sal::static_int_cast<sal_uInt8>(eRet);
}

ScMacroManager* ScDocument::GetMacroManager()
{
    if (!mpMacroMgr.get())
        mpMacroMgr.reset(new ScMacroManager(this));
    return mpMacroMgr.get();
}

bool ScDocument::IsEmptyData( SCTAB nTab, SCCOL nCol ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return true;

    return pTab->IsEmptyData(nCol);
}

void ScDocument::FillMatrix(
    ScMatrix& rMat, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    if (nCol1 > nCol2 || nRow1 > nRow2)
        return;

    SCSIZE nC, nR;
    rMat.GetDimensions(nC, nR);
    if (static_cast<SCROW>(nR) != nRow2 - nRow1 + 1 || static_cast<SCCOL>(nC) != nCol2 - nCol1 + 1)
        return;

    pTab->FillMatrix(rMat, nCol1, nRow1, nCol2, nRow2);
}

void ScDocument::SetFormulaResults( const ScAddress& rTopPos, const double* pResults, size_t nLen )
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab)
        return;

    pTab->SetFormulaResults(rTopPos.Col(), rTopPos.Row(), pResults, nLen);
}


//------------------------------------------------------------------------

void ScDocument::InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                      bool bNumFormatChanged )
{
    bool bBroadcast = (bNumFormatChanged && GetDocOptions().IsCalcAsShown() && !IsImportingXML() && !IsClipboard());
    if ( pAdrFrom && !pAdrTo )
    {
        const SCTAB nTab = pAdrFrom->Tab();

        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
            maTabs[nTab]->InvalidateTextWidth( pAdrFrom, NULL, bNumFormatChanged, bBroadcast );
    }
    else
    {
        const SCTAB nTabStart = pAdrFrom ? pAdrFrom->Tab() : 0;
        const SCTAB nTabEnd   = pAdrTo   ? pAdrTo->Tab()   : MAXTAB;

        for ( SCTAB nTab=nTabStart; nTab<=nTabEnd && nTab < static_cast<SCTAB>(maTabs.size()); nTab++ )
            if ( maTabs[nTab] )
                maTabs[nTab]->InvalidateTextWidth( pAdrFrom, pAdrTo, bNumFormatChanged, bBroadcast );
    }
}

//------------------------------------------------------------------------

#define CALCMAX                 1000    // Berechnungen
#define ABORT_EVENTS            (VCL_INPUT_ANY & ~VCL_INPUT_TIMER & ~VCL_INPUT_OTHER)

namespace {

class IdleCalcTextWidthScope
{
    ScDocument& mrDoc;
    ScAddress& mrCalcPos;
    MapMode maOldMapMode;
    sal_uLong mnStartTime;
    ScStyleSheetPool* mpStylePool;
    sal_uInt16 mnOldSearchMask;
    SfxStyleFamily meOldFamily;
    bool mbNeedMore;
    bool mbProgress;

public:
    IdleCalcTextWidthScope(ScDocument& rDoc, ScAddress& rCalcPos) :
        mrDoc(rDoc),
        mrCalcPos(rCalcPos),
        mnStartTime(Time::GetSystemTicks()),
        mpStylePool(rDoc.GetStyleSheetPool()),
        mnOldSearchMask(mpStylePool->GetSearchMask()),
        meOldFamily(mpStylePool->GetSearchFamily()),
        mbNeedMore(false),
        mbProgress(false)
    {
        // The old search mask / family flags must be restored so that e.g.
        // the styles dialog shows correct listing when it's opened in-between
        // the calls.

        mrDoc.EnableIdle(false);
        mpStylePool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL);
    }

    ~IdleCalcTextWidthScope()
    {
        SfxPrinter* pDev = mrDoc.GetPrinter();
        if (pDev)
            pDev->SetMapMode(maOldMapMode);

        if (mbProgress)
            ScProgress::DeleteInterpretProgress();

        mpStylePool->SetSearchMask(meOldFamily, mnOldSearchMask);
        mrDoc.EnableIdle(true);
    }

    SCTAB Tab() const { return mrCalcPos.Tab(); }
    SCCOL Col() const { return mrCalcPos.Col(); }
    SCROW Row() const { return mrCalcPos.Row(); }

    const ScAddress& Pos() const { return mrCalcPos; }

    void setTab(SCTAB nTab) { mrCalcPos.SetTab(nTab); }
    void setCol(SCCOL nCol) { mrCalcPos.SetCol(nCol); }
    void setRow(SCROW nRow) { mrCalcPos.SetRow(nRow); }

    void incTab(SCTAB nInc=1) { mrCalcPos.IncTab(nInc); }
    void incCol(SCCOL nInc=1) { mrCalcPos.IncCol(nInc); }
    void incRow(SCROW nInc=1) { mrCalcPos.IncRow(nInc); }

    void setOldMapMode(const MapMode& rOldMapMode) { maOldMapMode = rOldMapMode; }

    void setNeedMore(bool b) { mbNeedMore = b; }
    bool getNeedMore() const { return mbNeedMore; }

    sal_uLong getStartTime() const { return mnStartTime; }

    void createProgressBar()
    {
        ScProgress::CreateInterpretProgress(&mrDoc, false);
        mbProgress = true;
    }

    bool hasProgressBar() const { return mbProgress; }

    ScStyleSheetPool* getStylePool() { return mpStylePool; }
};

}

bool ScDocument::IdleCalcTextWidth()            // true = demnaechst wieder versuchen
{
    // #i75610# if a printer hasn't been set or created yet, don't create one for this
    if (!mbIdleEnabled || IsInLinkUpdate() || GetPrinter(false) == NULL)
        return false;

    IdleCalcTextWidthScope aScope(*this, aCurTextWidthCalcPos);

    if (!ValidRow(aScope.Row()))
    {
        aScope.setRow(0);
        aScope.incCol(-1);
    }

    if (aScope.Col() < 0)
    {
        aScope.setCol(MAXCOL);
        aScope.incTab();
    }

    if (!ValidTab(aScope.Tab()) || aScope.Tab() >= static_cast<SCTAB>(maTabs.size()) || !maTabs[aScope.Tab()])
        aScope.setTab(0);

    ScTable* pTab = maTabs[aScope.Tab()];
    ScStyleSheet* pStyle = (ScStyleSheet*)aScope.getStylePool()->Find(pTab->aPageStyle, SFX_STYLE_FAMILY_PAGE);
    OSL_ENSURE( pStyle, "Missing StyleSheet :-/" );

    if (!pStyle || getScaleValue(*pStyle, ATTR_PAGE_SCALETOPAGES) == 0)
    {
        // Move to the next sheet as the current one has scale-to-pages set,
        // and bail out.
        aScope.incTab();
        return false;
    }

    sal_uInt16 nZoom = getScaleValue(*pStyle, ATTR_PAGE_SCALE);
    Fraction aZoomFract(nZoom, 100);

    // Start at specified cell position (nCol, nRow, nTab).
    ScColumn* pCol  = &pTab->aCol[aScope.Col()];
    boost::scoped_ptr<ScColumnTextWidthIterator> pColIter(new ScColumnTextWidthIterator(*pCol, aScope.Row(), MAXROW));

    OutputDevice* pDev = NULL;
    sal_uInt16 nRestart = 0;
    sal_uInt16 nCount = 0;
    while ( (nZoom > 0) && (nCount < CALCMAX) && (nRestart < 2) )
    {
        if (pColIter->hasCell())
        {
            // More cell in this column.
            SCROW nRow = pColIter->getPos();
            aScope.setRow(nRow);

            if (pColIter->getValue() == TEXTWIDTH_DIRTY)
            {
                // Calculate text width for this cell.
                double nPPTX = 0.0;
                double nPPTY = 0.0;
                if (!pDev)
                {
                    pDev = GetPrinter();
                    aScope.setOldMapMode(pDev->GetMapMode());
                    pDev->SetMapMode( MAP_PIXEL );  // wichtig fuer GetNeededSize

                    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
                    nPPTX = aPix1000.X() / 1000.0;
                    nPPTY = aPix1000.Y() / 1000.0;
                }

                if (!aScope.hasProgressBar() && pCol->IsFormulaDirty(nRow))
                    aScope.createProgressBar();

                sal_uInt16 nNewWidth = (sal_uInt16)GetNeededSize(
                    aScope.Col(), aScope.Row(), aScope.Tab(),
                    pDev, nPPTX, nPPTY, aZoomFract,aZoomFract, true, true);   // bTotalSize

                pColIter->setValue(nNewWidth);
                aScope.setNeedMore(true);
            }
            pColIter->next();
        }
        else
        {
            // No more cell in this column.  Move to the left column and start at row 0.

            bool bNewTab = false;

            aScope.setRow(0);
            aScope.incCol(-1);

            if (aScope.Col() < 0)
            {
                // No more column to the left.  Move to the right-most column of the next sheet.
                aScope.setCol(MAXCOL);
                aScope.incTab();
                bNewTab = true;
            }

            if (!ValidTab(aScope.Tab()) || aScope.Tab() >= static_cast<SCTAB>(maTabs.size()) || !maTabs[aScope.Tab()] )
            {
                // Sheet doesn't exist at specified sheet position.  Restart at sheet 0.
                aScope.setTab(0);
                nRestart++;
                bNewTab = true;
            }

            if ( nRestart < 2 )
            {
                if ( bNewTab )
                {
                    pTab = maTabs[aScope.Tab()];
                    pStyle = (ScStyleSheet*)aScope.getStylePool()->Find(
                        pTab->aPageStyle, SFX_STYLE_FAMILY_PAGE);

                    if ( pStyle )
                    {
                        // Check if the scale-to-pages setting is set. If
                        // set, we exit the loop.  If not, get the page
                        // scale factor of the new sheet.
                        if (getScaleValue(*pStyle, ATTR_PAGE_SCALETOPAGES) == 0)
                        {
                            nZoom = getScaleValue(*pStyle, ATTR_PAGE_SCALE);
                            aZoomFract = Fraction(nZoom, 100);
                        }
                        else
                            nZoom = 0;
                    }
                    else
                    {
                        OSL_FAIL( "Missing StyleSheet :-/" );
                    }
                }

                if ( nZoom > 0 )
                {
                    pCol  = &pTab->aCol[aScope.Col()];
                    pColIter.reset(new ScColumnTextWidthIterator(*pCol, aScope.Row(), MAXROW));
                }
                else
                {
                    aScope.incTab(); // Move to the next sheet as the current one has scale-to-pages set.
                    return false;
                }
            }
        }

        ++nCount;

        // Quit if either 1) its duration exceeds 50 ms, or 2) there is any
        // pending event after processing 32 cells.
        if ((50L < Time::GetSystemTicks() - aScope.getStartTime()) || (nCount > 31 && Application::AnyInput(ABORT_EVENTS)))
            nCount = CALCMAX;
    }

    return aScope.getNeedMore();
}

//------------------------------------------------------------------------

class ScSpellStatus
{
public:
    bool    bModified;

    ScSpellStatus() : bModified(false) {};

    DECL_LINK (EventHdl, EditStatus*);
};

IMPL_LINK( ScSpellStatus, EventHdl, EditStatus *, pStatus )
{
    sal_uLong nStatus = pStatus->GetStatusWord();
    if ( nStatus & EE_STAT_WRONGWORDCHANGED )
        bModified = true;

    return 0;
}

//  SPELL_MAXCELLS muss mindestens 256 sein, solange am Iterator keine
//  Start-Spalte gesetzt werden kann

//! SPELL_MAXTEST fuer Timer und Idle unterschiedlich ???

//  SPELL_MAXTEST now divided between visible and rest of document

#define SPELL_MAXTEST_VIS   1
#define SPELL_MAXTEST_ALL   3
#define SPELL_MAXCELLS      256

bool ScDocument::OnlineSpellInRange( const ScRange& rSpellRange, ScAddress& rSpellPos,
                                     sal_uInt16 nMaxTest )
{
    ScEditEngineDefaulter* pEngine = NULL;              //! am Dokument speichern
    SfxItemSet* pDefaults = NULL;
    ScSpellStatus aStatus;

    sal_uInt16 nCellCount = 0;          // Zellen insgesamt
    sal_uInt16 nTestCount = 0;          // Aufrufe Spelling
    bool bChanged = false;          // Aenderungen?

    SCCOL nCol = rSpellRange.aStart.Col();      // iterator always starts on the left edge
    SCROW nRow = rSpellPos.Row();
    SCTAB nTab = rSpellPos.Tab();
    if ( nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] )                           // sheet deleted?
    {
        nTab = rSpellRange.aStart.Tab();
        nRow = rSpellRange.aStart.Row();
        if ( nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] )
        {
            //  may happen for visible range
            return false;
        }
    }

    ScRangeList aPivotRanges;
    if (pDPCollection)
        aPivotRanges = pDPCollection->GetAllTableRanges(nTab);

    ScHorizontalCellIterator aIter( this, nTab,
                                    rSpellRange.aStart.Col(), nRow,
                                    rSpellRange.aEnd.Col(), rSpellRange.aEnd.Row() );
    ScRefCellValue* pCell = aIter.GetNext( nCol, nRow );
    //  skip everything left of rSpellPos:
    while ( pCell && nRow == rSpellPos.Row() && nCol < rSpellPos.Col() )
        pCell = aIter.GetNext( nCol, nRow );

    for (; pCell; pCell = aIter.GetNext(nCol, nRow))
    {
        if (!aPivotRanges.empty() && aPivotRanges.In(ScAddress(nCol, nRow, nTab)))
            // Don't spell check within pivot tables.
            continue;

        CellType eType = pCell->meType;
        if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
        {
            if (!pEngine)
            {
                //  ScTabEditEngine is needed
                //  because MapMode must be set for some old documents
                pEngine = new ScTabEditEngine( this );
                pEngine->SetControlWord( pEngine->GetControlWord() |
                            ( EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS ) );
                pEngine->SetStatusEventHdl( LINK( &aStatus, ScSpellStatus, EventHdl ) );
                //  Delimiters hier wie in inputhdl.cxx !!!
                pEngine->SetWordDelimiters(
                            ScEditUtil::ModifyDelimiters( pEngine->GetWordDelimiters() ) );
                pDefaults = new SfxItemSet( pEngine->GetEmptyItemSet() );

                com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1> xXSpellChecker1( LinguMgr::GetSpellChecker() );

                pEngine->SetSpeller( xXSpellChecker1 );
            }

            const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
            if ( ScStyleSheet* pPreviewStyle = GetPreviewCellStyle( nCol, nRow, nTab ) )
            {
                ScPatternAttr* pPreviewPattern = new ScPatternAttr( *pPattern );
                pPreviewPattern->SetStyleSheet(pPreviewStyle);
                pPreviewPattern->FillEditItemSet( pDefaults );
                delete pPreviewPattern;
            }
            else
            {
                SfxItemSet* pFontSet = GetPreviewFont( nCol, nRow, nTab );
                pPattern->FillEditItemSet( pDefaults, pFontSet );
            }
            pEngine->SetDefaults( pDefaults, false );               //! noetig ?

            sal_uInt16 nCellLang = ((const SvxLanguageItem&)
                                    pPattern->GetItem(ATTR_FONT_LANGUAGE)).GetValue();
            if ( nCellLang == LANGUAGE_SYSTEM )
                nCellLang = Application::GetSettings().GetLanguageTag().getLanguageType();   // never use SYSTEM for spelling
            pEngine->SetDefaultLanguage( nCellLang );

            if ( eType == CELLTYPE_STRING )
            {
                pEngine->SetText(*pCell->mpString);
            }
            else if (pCell->mpEditText)
                pEngine->SetText(*pCell->mpEditText);
            else
                pEngine->SetText(EMPTY_OUSTRING);

            aStatus.bModified = false;
            pEngine->CompleteOnlineSpelling();
            if ( aStatus.bModified )                // Fehler dazu oder weggekommen?
            {
                bool bNeedEdit = true;                      //  Test auf einfachen Text
                if ( !pEngine->HasOnlineSpellErrors() )
                {
                    ScEditAttrTester aTester( pEngine );
                    bNeedEdit = aTester.NeedsObject();
                }

                if ( bNeedEdit )
                {
                    // The cell will take ownership of pNewData.
                    SetEditText(ScAddress(nCol,nRow,nTab), pEngine->CreateTextObject());
                    aIter.RehashCol(nCol);
                }
                else
                {
                    ScSetStringParam aParam;
                    aParam.setTextInput();
                    SetString(ScAddress(nCol,nRow,nTab), pEngine->GetText(), &aParam);
                    aIter.RehashCol(nCol);
                }

                //  Paint
                if (pShell)
                {
                    //  Seitenvorschau ist davon nicht betroffen
                    //  (sollte jedenfalls nicht)
                    ScPaintHint aHint( ScRange( nCol, nRow, nTab ), PAINT_GRID );
                    aHint.SetPrintFlag( false );
                    pShell->Broadcast( aHint );
                }

                bChanged = true;
            }

            if ( ++nTestCount >= nMaxTest )             // checked enough text?
                break;
        }

        if ( ++nCellCount >= SPELL_MAXCELLS )           // seen enough cells?
            break;
    }

    if ( pCell )
    {
        ++nCol;                                         // continue after last cell
        if ( nCol > rSpellRange.aEnd.Col() )
        {
            nCol = rSpellRange.aStart.Col();
            ++nRow;
            if ( nRow > rSpellRange.aEnd.Row() )
                pCell = NULL;
        }
    }

    if (!pCell)         // end of range reached -> next sheet
    {
        ++nTab;
        if ( nTab > rSpellRange.aEnd.Tab() || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] )
            nTab = rSpellRange.aStart.Tab();
        nCol = rSpellRange.aStart.Col();
        nRow = rSpellRange.aStart.Row();

        nVisSpellState = VSPL_DONE;     //! only if this is for the visible range
    }
    rSpellPos.Set( nCol, nRow, nTab );

    delete pDefaults;
    delete pEngine;         // bevor aStatus out of scope geht

    return bChanged;
}


bool ScDocument::ContinueOnlineSpelling()
{
    if (!mbIdleEnabled || !pDocOptions->IsAutoSpell() || (pShell && pShell->IsReadOnly()))
        return false;

    // #i48433# set bInsertingFromOtherDoc flag so there are no broadcasts when PutCell is called
    // (same behavior as in RemoveAutoSpellObj: just transfer the broadcaster)
    bool bOldInserting = IsInsertingFromOtherDoc();
    SetInsertingFromOtherDoc( true );

    //! use one EditEngine for both calls

    //  first check visible range
    bool bResult = OnlineSpellInRange( aVisSpellRange, aVisSpellPos, SPELL_MAXTEST_VIS );

    //  during first pass through visible range, always continue
    if ( nVisSpellState == VSPL_START )
        bResult = true;

    if (bResult)
    {
        //  if errors found, continue there
        OnlineSpellInRange( aVisSpellRange, aVisSpellPos, SPELL_MAXTEST_ALL );
    }
    else
    {
        //  if nothing found there, continue with rest of document
        ScRange aTotalRange( 0,0,0, MAXCOL,MAXROW,MAXTAB );
        bResult = OnlineSpellInRange( aTotalRange, aOnlineSpellPos, SPELL_MAXTEST_ALL );
    }

    SetInsertingFromOtherDoc( bOldInserting );

    return bResult;
}


void ScDocument::SetOnlineSpellPos( const ScAddress& rPos )
{
    aOnlineSpellPos = rPos;

    //  skip visible area for aOnlineSpellPos
    if ( aVisSpellRange.In( aOnlineSpellPos ) )
        aOnlineSpellPos = aVisSpellRange.aEnd;
}

bool ScDocument::SetVisibleSpellRange( const ScRange& rNewRange )
{
    bool bChange = ( aVisSpellRange != rNewRange );
    if (bChange)
    {
        //  continue spelling through visible range when scrolling down
        bool bContDown = ( nVisSpellState == VSPL_START && rNewRange.In( aVisSpellPos ) &&
                            rNewRange.aStart.Row() >  aVisSpellRange.aStart.Row() &&
                            rNewRange.aStart.Col() == aVisSpellRange.aStart.Col() &&
                            rNewRange.aEnd.Col()   == aVisSpellRange.aEnd.Col() );

        aVisSpellRange = rNewRange;

        if ( !bContDown )
        {
            aVisSpellPos = aVisSpellRange.aStart;
            nVisSpellState = VSPL_START;
        }

        //  skip visible area for aOnlineSpellPos
        if ( aVisSpellRange.In( aOnlineSpellPos ) )
            aOnlineSpellPos = aVisSpellRange.aEnd;
    }
    return bChange;
}

void ScDocument::RemoveAutoSpellObj()
{
    //  alle Spelling-Informationen entfernen

    for (SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()) && maTabs[nTab]; nTab++)
        maTabs[nTab]->RemoveAutoSpellObj();
}

void ScDocument::RepaintRange( const ScRange& rRange )
{
    if ( bIsVisible && pShell )
    {
        ScModelObj* pModel = ScModelObj::getImplementation( pShell->GetModel() );
        if ( pModel )
            pModel->RepaintRange( rRange );     // locked repaints are checked there
    }
}

void ScDocument::RepaintRange( const ScRangeList& rRange )
{
    if ( bIsVisible && pShell )
    {
        ScModelObj* pModel = ScModelObj::getImplementation( pShell->GetModel() );
        if ( pModel )
            pModel->RepaintRange( rRange );     // locked repaints are checked there
    }
}

//------------------------------------------------------------------------

bool ScDocument::IdleCheckLinks()           // true = demnaechst wieder versuchen
{
    bool bAnyLeft = false;

    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
            {
                ScDdeLink* pDdeLink = (ScDdeLink*)pBase;
                if (pDdeLink->NeedsUpdate())
                {
                    pDdeLink->TryUpdate();
                    if (pDdeLink->NeedsUpdate())        // war nix?
                        bAnyLeft = true;
                }
            }
        }
    }

    return bAnyLeft;
}

void ScDocument::SaveDdeLinks(SvStream& rStream) const
{
    //  bei 4.0-Export alle mit Modus != DEFAULT weglassen
    bool bExport40 = ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 );

    const ::sfx2::SvBaseLinks& rLinks = GetLinkManager()->GetLinks();
    sal_uInt16 nCount = rLinks.size();

    //  erstmal zaehlen...

    sal_uInt16 nDdeCount = 0;
    sal_uInt16 i;
    for (i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
            if ( !bExport40 || ((ScDdeLink*)pBase)->GetMode() == SC_DDE_DEFAULT )
                ++nDdeCount;
    }

    //  Header

    ScMultipleWriteHeader aHdr( rStream );
    rStream << nDdeCount;

    //  Links speichern

    for (i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pLink = (ScDdeLink*)pBase;
            if ( !bExport40 || pLink->GetMode() == SC_DDE_DEFAULT )
                pLink->Store( rStream, aHdr );
        }
    }
}

void ScDocument::LoadDdeLinks(SvStream& rStream)
{
    ScMultipleReadHeader aHdr( rStream );

    GetLinkManager();
    sal_uInt16 nCount;
    rStream >> nCount;
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ScDdeLink* pLink = new ScDdeLink( this, rStream, aHdr );
        pLinkManager->InsertDDELink( pLink,
                            pLink->GetAppl(), pLink->GetTopic(), pLink->GetItem() );
    }
}

bool ScDocument::HasDdeLinks() const
{
    if (GetLinkManager())           // Clipboard z.B. hat keinen LinkManager
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        for (sal_uInt16 i=0; i<nCount; i++)
            if ((*rLinks[i])->ISA(ScDdeLink))
                return true;
    }

    return false;
}

void ScDocument::SetInLinkUpdate(bool bSet)
{
    //  called from TableLink and AreaLink

    OSL_ENSURE( bInLinkUpdate != bSet, "SetInLinkUpdate twice" );
    bInLinkUpdate = bSet;
}

bool ScDocument::IsInLinkUpdate() const
{
    return bInLinkUpdate || IsInDdeLinkUpdate();
}

void ScDocument::UpdateExternalRefLinks(Window* pWin)
{
    if (!GetLinkManager())
        return;

    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = rLinks.size();

    bool bAny = false;
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        ScExternalRefLink* pRefLink = dynamic_cast<ScExternalRefLink*>(pBase);
        if (pRefLink)
        {
            if (pRefLink->Update())
                bAny = true;
            else
            {
                // Update failed.  Notify the user.

                OUString aFile;
                pLinkManager->GetDisplayNames(pRefLink, NULL, &aFile, NULL, NULL);
                // Decode encoded URL for display friendliness.
                INetURLObject aUrl(aFile,INetURLObject::WAS_ENCODED);
                aFile = aUrl.GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS);

                OUStringBuffer aBuf;
                aBuf.append(String(ScResId(SCSTR_EXTDOC_NOT_LOADED)));
                aBuf.appendAscii("\n\n");
                aBuf.append(aFile);
                ErrorBox aBox(pWin, WB_OK, aBuf.makeStringAndClear());
                aBox.Execute();
            }
        }
    }
    if (bAny)
    {
        TrackFormulas();
        pShell->Broadcast( SfxSimpleHint(FID_DATACHANGED) );

        // #i101960# set document modified, as in TrackTimeHdl for DDE links
        if (!pShell->IsModified())
        {
            pShell->SetModified( true );
            SfxBindings* pBindings = GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( SID_SAVEDOC );
                pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
    }
}

void ScDocument::UpdateDdeLinks(Window* pWin)
{
    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        sal_uInt16 i;

        //  falls das Updaten laenger dauert, erstmal alle Werte
        //  zuruecksetzen, damit nichts altes (falsches) stehen bleibt
        bool bAny = false;
        for (i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
            if (pDdeLink)
            {
                if (pDdeLink->Update())
                    bAny = true;
                else
                {
                    // Update failed.  Notify the user.
                    OUString aFile = pDdeLink->GetTopic();
                    OUString aElem = pDdeLink->GetItem();
                    OUString aType = pDdeLink->GetAppl();

                    OUStringBuffer aBuf;
                    aBuf.append(String(ScResId(SCSTR_DDEDOC_NOT_LOADED)));
                    aBuf.appendAscii("\n\n");
                    aBuf.appendAscii("Source : ");
                    aBuf.append(aFile);
                    aBuf.appendAscii("\nElement : ");
                    aBuf.append(aElem);
                    aBuf.appendAscii("\nType : ");
                    aBuf.append(aType);
                    ErrorBox aBox(pWin, WB_OK, aBuf.makeStringAndClear());
                    aBox.Execute();
                }
            }
        }
        if (bAny)
        {
            //  Formeln berechnen und painten wie im TrackTimeHdl
            TrackFormulas();
            pShell->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );

            //  wenn FID_DATACHANGED irgendwann mal asynchron werden sollte
            //  (z.B. mit Invalidate am Window), muss hier ein Update erzwungen werden.
        }

        pLinkManager->CloseCachedComps();
    }
}

bool ScDocument::UpdateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem )
{
    //  fuer refresh() per StarOne Api
    //  ResetValue() fuer einzelnen Link nicht noetig
    //! wenn's mal alles asynchron wird, aber auch hier

    bool bFound = false;
    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
            {
                ScDdeLink* pDdeLink = (ScDdeLink*)pBase;
                if ( OUString(pDdeLink->GetAppl()) == rAppl &&
                     OUString(pDdeLink->GetTopic()) == rTopic &&
                     OUString(pDdeLink->GetItem()) == rItem )
                {
                    pDdeLink->TryUpdate();
                    bFound = true;          // koennen theoretisch mehrere sein (Mode), darum weitersuchen
                }
            }
        }
        pLinkManager->CloseCachedComps();
    }
    return bFound;
}

void ScDocument::DisconnectDdeLinks()
{
    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
                pBase->Disconnect();            // bleibt im LinkManager eingetragen
        }
    }
}

void ScDocument::CopyDdeLinks( ScDocument* pDestDoc ) const
{
    if (bIsClip)        // aus Stream erzeugen
    {
        if (pClipData)
        {
            pClipData->Seek(0);
            pDestDoc->LoadDdeLinks(*pClipData);
        }
    }
    else if (GetLinkManager())              // Links direkt kopieren
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        size_t nCount = rLinks.size();
        for (size_t i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
            {
                ScDdeLink* pNew = new ScDdeLink( pDestDoc, *(ScDdeLink*)pBase );

                pDestDoc->pLinkManager->InsertDDELink( pNew,
                                pNew->GetAppl(), pNew->GetTopic(), pNew->GetItem() );
            }
        }
    }
}

size_t ScDocument::GetDdeLinkCount() const
{
    size_t nDdeCount = 0;
    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        size_t nCount = rLinks.size();
        for (size_t i=0; i<nCount; i++)
            if ((*rLinks[i])->ISA(ScDdeLink))
                ++nDdeCount;
    }
    return nDdeCount;
}

// ----------------------------------------------------------------------------

namespace {

/** Tries to find the specified DDE link.
    @param pnDdePos  (out-param) if not 0, the index of the DDE link is returned here
                     (does not include other links from link manager).
    @return  The DDE link, if it exists, otherwise 0. */
ScDdeLink* lclGetDdeLink(
        const sfx2::LinkManager* pLinkManager,
        const OUString& rAppl, const OUString& rTopic, const OUString& rItem, sal_uInt8 nMode,
        size_t* pnDdePos = NULL )
{
    if( pLinkManager )
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        size_t nCount = rLinks.size();
        if( pnDdePos ) *pnDdePos = 0;
        for( size_t nIndex = 0; nIndex < nCount; ++nIndex )
        {
            ::sfx2::SvBaseLink* pLink = *rLinks[ nIndex ];
            if( ScDdeLink* pDdeLink = PTR_CAST( ScDdeLink, pLink ) )
            {
                if( (OUString(pDdeLink->GetAppl()) == rAppl) &&
                    (OUString(pDdeLink->GetTopic()) == rTopic) &&
                    (OUString(pDdeLink->GetItem()) == rItem) &&
                    ((nMode == SC_DDE_IGNOREMODE) || (nMode == pDdeLink->GetMode())) )
                    return pDdeLink;
                if( pnDdePos ) ++*pnDdePos;
            }
        }
    }
    return NULL;
}

/** Returns a pointer to the specified DDE link.
    @param nDdePos  Index of the DDE link (does not include other links from link manager).
    @return  The DDE link, if it exists, otherwise 0. */
ScDdeLink* lclGetDdeLink( const sfx2::LinkManager* pLinkManager, size_t nDdePos )
{
    if( pLinkManager )
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        size_t nCount = rLinks.size();
        size_t nDdeIndex = 0;       // counts only the DDE links
        for( size_t nIndex = 0; nIndex < nCount; ++nIndex )
        {
            ::sfx2::SvBaseLink* pLink = *rLinks[ nIndex ];
            if( ScDdeLink* pDdeLink = PTR_CAST( ScDdeLink, pLink ) )
            {
                if( nDdeIndex == nDdePos )
                    return pDdeLink;
                ++nDdeIndex;
            }
        }
    }
    return NULL;
}

} // namespace

// ----------------------------------------------------------------------------

bool ScDocument::FindDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem,
        sal_uInt8 nMode, size_t& rnDdePos )
{
    return lclGetDdeLink( GetLinkManager(), rAppl, rTopic, rItem, nMode, &rnDdePos ) != NULL;
}

bool ScDocument::GetDdeLinkData( size_t nDdePos, OUString& rAppl, OUString& rTopic, OUString& rItem ) const
{
    if( const ScDdeLink* pDdeLink = lclGetDdeLink( GetLinkManager(), nDdePos ) )
    {
        rAppl  = pDdeLink->GetAppl();
        rTopic = pDdeLink->GetTopic();
        rItem  = pDdeLink->GetItem();
        return true;
    }
    return false;
}

bool ScDocument::GetDdeLinkMode( size_t nDdePos, sal_uInt8& rnMode ) const
{
    if( const ScDdeLink* pDdeLink = lclGetDdeLink( GetLinkManager(), nDdePos ) )
    {
        rnMode = pDdeLink->GetMode();
        return true;
    }
    return false;
}

const ScMatrix* ScDocument::GetDdeLinkResultMatrix( size_t nDdePos ) const
{
    const ScDdeLink* pDdeLink = lclGetDdeLink( GetLinkManager(), nDdePos );
    return pDdeLink ? pDdeLink->GetResult() : NULL;
}

bool ScDocument::CreateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem, sal_uInt8 nMode, ScMatrixRef pResults )
{
    /*  Create a DDE link without updating it (i.e. for Excel import), to prevent
        unwanted connections. First try to find existing link. Set result array
        on existing and new links. */
    //! store DDE links additionally at document (for efficiency)?
    OSL_ENSURE( nMode != SC_DDE_IGNOREMODE, "ScDocument::CreateDdeLink - SC_DDE_IGNOREMODE not allowed here" );
    if( GetLinkManager() && (nMode != SC_DDE_IGNOREMODE) )
    {
        ScDdeLink* pDdeLink = lclGetDdeLink( pLinkManager, rAppl, rTopic, rItem, nMode );
        if( !pDdeLink )
        {
            // create a new DDE link, but without TryUpdate
            pDdeLink = new ScDdeLink( this, rAppl, rTopic, rItem, nMode );
            pLinkManager->InsertDDELink( pDdeLink, rAppl, rTopic, rItem );
        }

        // insert link results
        if( pResults )
            pDdeLink->SetResult( pResults );

        return true;
    }
    return false;
}

bool ScDocument::SetDdeLinkResultMatrix( size_t nDdePos, ScMatrixRef pResults )
{
    if( ScDdeLink* pDdeLink = lclGetDdeLink( GetLinkManager(), nDdePos ) )
    {
        pDdeLink->SetResult( pResults );
        return true;
    }
    return false;
}

//------------------------------------------------------------------------

bool ScDocument::HasAreaLinks() const
{
    if (GetLinkManager())           // Clipboard z.B. hat keinen LinkManager
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        for (sal_uInt16 i=0; i<nCount; i++)
            if ((*rLinks[i])->ISA(ScAreaLink))
                return true;
    }

    return false;
}

void ScDocument::UpdateAreaLinks()
{
    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        for (sal_uInt16 i=0; i<rLinks.size(); i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
                pBase->Update();
        }
    }
}

void ScDocument::DeleteAreaLinksOnTab( SCTAB nTab )
{
    if (GetLinkManager())
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nPos = 0;
        while ( nPos < rLinks.size() )
        {
            const ::sfx2::SvBaseLink* pBase = *rLinks[nPos];
            if ( pBase->ISA(ScAreaLink) &&
                 static_cast<const ScAreaLink*>(pBase)->GetDestArea().aStart.Tab() == nTab )
                pLinkManager->Remove( nPos );
            else
                ++nPos;
        }
    }
}

void ScDocument::UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    if (GetLinkManager())
    {
        bool bAnyUpdate = false;

        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        sal_uInt16 nCount = rLinks.size();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScAreaLink))
            {
                ScAreaLink* pLink = (ScAreaLink*) pBase;
                ScRange aOutRange = pLink->GetDestArea();

                SCCOL nCol1 = aOutRange.aStart.Col();
                SCROW nRow1 = aOutRange.aStart.Row();
                SCTAB nTab1 = aOutRange.aStart.Tab();
                SCCOL nCol2 = aOutRange.aEnd.Col();
                SCROW nRow2 = aOutRange.aEnd.Row();
                SCTAB nTab2 = aOutRange.aEnd.Tab();

                ScRefUpdateRes eRes =
                    ScRefUpdate::Update( this, eUpdateRefMode,
                        rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                        rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
                        nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                if ( eRes != UR_NOTHING )
                {
                    pLink->SetDestArea( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );
                    bAnyUpdate = true;
                }
            }
        }

        if ( bAnyUpdate )
        {
            // #i52120# Look for duplicates (after updating all positions).
            // If several links start at the same cell, the one with the lower index is removed
            // (file format specifies only one link definition for a cell).

            sal_uInt16 nFirstIndex = 0;
            while ( nFirstIndex < nCount )
            {
                bool bFound = false;
                ::sfx2::SvBaseLink* pFirst = *rLinks[nFirstIndex];
                if ( pFirst->ISA(ScAreaLink) )
                {
                    ScAddress aFirstPos = static_cast<ScAreaLink*>(pFirst)->GetDestArea().aStart;
                    for ( sal_uInt16 nSecondIndex = nFirstIndex + 1; nSecondIndex < nCount && !bFound; ++nSecondIndex )
                    {
                        ::sfx2::SvBaseLink* pSecond = *rLinks[nSecondIndex];
                        if ( pSecond->ISA(ScAreaLink) &&
                             static_cast<ScAreaLink*>(pSecond)->GetDestArea().aStart == aFirstPos )
                        {
                            // remove the first link, exit the inner loop, don't increment nFirstIndex
                            pLinkManager->Remove( pFirst );
                            nCount = rLinks.size();
                            bFound = true;
                        }
                    }
                }
                if (!bFound)
                    ++nFirstIndex;
            }
        }
    }
}

//------------------------------------------------------------------------

// TimerDelays etc.
void ScDocument::KeyInput( const KeyEvent& )
{
    if ( pChartListenerCollection->hasListeners() )
        pChartListenerCollection->StartTimer();
    if( apTemporaryChartLock.get() )
        apTemporaryChartLock->StartOrContinueLocking();
}

//  ----------------------------------------------------------------------------

bool ScDocument::CheckMacroWarn()
{
    //  The check for macro configuration, macro warning and disabling is now handled
    //  in SfxObjectShell::AdjustMacroMode, called by SfxObjectShell::CallBasic.

    return true;
}

//------------------------------------------------------------------------

SfxBindings* ScDocument::GetViewBindings()
{
    //  used to invalidate slots after changes to this document

    if ( !pShell )
        return NULL;        // no ObjShell -> no view

    //  first check current view
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame && pViewFrame->GetObjectShell() != pShell )     // wrong document?
        pViewFrame = NULL;

    //  otherwise use first view for this doc
    if ( !pViewFrame )
        pViewFrame = SfxViewFrame::GetFirst( pShell );

    if (pViewFrame)
        return &pViewFrame->GetBindings();
    else
        return NULL;
}

ScDrawLayer* ScDocument::GetDrawLayer()
{
    return pDrawLayer;
}

//------------------------------------------------------------------------

void ScDocument::TransliterateText( const ScMarkData& rMultiMark, sal_Int32 nType )
{
    OSL_ENSURE( rMultiMark.IsMultiMarked(), "TransliterateText: no selection" );

    utl::TransliterationWrapper aTranslitarationWrapper( comphelper::getProcessComponentContext(), nType );
    bool bConsiderLanguage = aTranslitarationWrapper.needLanguageForTheMode();
    sal_uInt16 nLanguage = LANGUAGE_SYSTEM;

    ScEditEngineDefaulter* pEngine = NULL;        // not using pEditEngine member because of defaults

    SCTAB nCount = GetTableCount();
    ScMarkData::const_iterator itr = rMultiMark.begin(), itrEnd = rMultiMark.end();
    for (; itr != itrEnd && *itr < nCount; ++itr)
        if ( maTabs[*itr] )
        {
            SCTAB nTab = *itr;
            SCCOL nCol = 0;
            SCROW nRow = 0;

            bool bFound = rMultiMark.IsCellMarked( nCol, nRow );
            if (!bFound)
                bFound = GetNextMarkedCell( nCol, nRow, nTab, rMultiMark );

            while (bFound)
            {
                ScRefCellValue aCell;
                aCell.assign(*this, ScAddress(nCol, nRow, nTab));

                // fdo#32786 TITLE_CASE/SENTENCE_CASE need the extra handling in EditEngine (loop over words/sentences).
                // Still use TransliterationWrapper directly for text cells with other transliteration types,
                // for performance reasons.
                if (aCell.meType == CELLTYPE_EDIT ||
                    (aCell.meType == CELLTYPE_STRING &&
                     ( nType == i18n::TransliterationModulesExtra::SENTENCE_CASE || nType == i18n::TransliterationModulesExtra::TITLE_CASE)))
                {
                    if (!pEngine)
                        pEngine = new ScFieldEditEngine(this, GetEnginePool(), GetEditPool());

                    // defaults from cell attributes must be set so right language is used
                    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
                    SfxItemSet* pDefaults = new SfxItemSet( pEngine->GetEmptyItemSet() );
                    if ( ScStyleSheet* pPreviewStyle = GetPreviewCellStyle( nCol, nRow, nTab ) )
                    {
                        ScPatternAttr* pPreviewPattern = new ScPatternAttr( *pPattern );
                        pPreviewPattern->SetStyleSheet(pPreviewStyle);
                        pPreviewPattern->FillEditItemSet( pDefaults );
                        delete pPreviewPattern;
                    }
                    else
                    {
                        SfxItemSet* pFontSet = GetPreviewFont( nCol, nRow, nTab );
                        pPattern->FillEditItemSet( pDefaults, pFontSet );
                    }
                    pEngine->SetDefaults( pDefaults,  true );
                    if (aCell.meType == CELLTYPE_STRING)
                        pEngine->SetText(*aCell.mpString);
                    else if (aCell.mpEditText)
                        pEngine->SetText(*aCell.mpEditText);

                    pEngine->ClearModifyFlag();

                    sal_Int32 nLastPar = pEngine->GetParagraphCount();
                    if (nLastPar)
                        --nLastPar;
                    xub_StrLen nTxtLen = pEngine->GetTextLen(nLastPar);
                    ESelection aSelAll( 0, 0, nLastPar, nTxtLen );

                    pEngine->TransliterateText( aSelAll, nType );

                    if ( pEngine->IsModified() )
                    {
                        ScEditAttrTester aTester( pEngine );
                        if ( aTester.NeedsObject() )
                        {
                            // remove defaults (paragraph attributes) before creating text object
                            SfxItemSet* pEmpty = new SfxItemSet( pEngine->GetEmptyItemSet() );
                            pEngine->SetDefaults( pEmpty, true );

                            // The cell will take ownership of the text object instance.
                            SetEditText(ScAddress(nCol,nRow,nTab), pEngine->CreateTextObject());
                        }
                        else
                        {
                            ScSetStringParam aParam;
                            aParam.setTextInput();
                            SetString(ScAddress(nCol,nRow,nTab), pEngine->GetText(), &aParam);
                        }
                    }
                }

                else if (aCell.meType == CELLTYPE_STRING)
                {
                    OUString aOldStr = *aCell.mpString;
                    sal_Int32 nOldLen = aOldStr.getLength();

                    if ( bConsiderLanguage )
                    {
                        sal_uInt8 nScript = GetStringScriptType( aOldStr );        //! cell script type?
                        sal_uInt16 nWhich = ( nScript == SCRIPTTYPE_ASIAN ) ? ATTR_CJK_FONT_LANGUAGE :
                                        ( ( nScript == SCRIPTTYPE_COMPLEX ) ? ATTR_CTL_FONT_LANGUAGE :
                                                                                ATTR_FONT_LANGUAGE );
                        nLanguage = ((const SvxLanguageItem*)GetAttr( nCol, nRow, nTab, nWhich ))->GetValue();
                    }

                    uno::Sequence<sal_Int32> aOffsets;
                    OUString aNewStr = aTranslitarationWrapper.transliterate( aOldStr, nLanguage, 0, nOldLen, &aOffsets );

                    if ( aNewStr != aOldStr )
                    {
                        ScSetStringParam aParam;
                        aParam.setTextInput();
                        SetString(ScAddress(nCol,nRow,nTab), aNewStr, &aParam);
                    }
                }
                bFound = GetNextMarkedCell( nCol, nRow, nTab, rMultiMark );
            }
        }
    delete pEngine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
