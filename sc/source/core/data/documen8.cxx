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

#include <scitems.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/linkmgr.hxx>
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
#include <unotools/transliterationwrapper.hxx>
#include <sal/log.hxx>

#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>

#include <inputopt.hxx>
#include <global.hxx>
#include <table.hxx>
#include <column.hxx>
#include <poolhelp.hxx>
#include <docpool.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <docoptio.hxx>
#include <viewopti.hxx>
#include <scextopt.hxx>
#include <rechead.hxx>
#include <ddelink.hxx>
#include <scmatrix.hxx>
#include <arealink.hxx>
#include <patattr.hxx>
#include <editutil.hxx>
#include <progress.hxx>
#include <document.hxx>
#include <chartlis.hxx>
#include <chartlock.hxx>
#include <refupdat.hxx>
#include <markdata.hxx>
#include <scmod.hxx>
#include <externalrefmgr.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <sc.hrc>
#include <charthelper.hxx>
#include <macromgr.hxx>
#include <docuno.hxx>
#include <scresid.hxx>
#include <columniterator.hxx>
#include <globalnames.hxx>
#include <stringutil.hxx>
#include <documentlinkmgr.hxx>
#include <scopetools.hxx>
#include <tokenarray.hxx>

#include <memory>
#include <utility>

using namespace com::sun::star;

namespace {

sal_uInt16 getScaleValue(SfxStyleSheetBase& rStyle, sal_uInt16 nWhich)
{
    return static_cast<const SfxUInt16Item&>(rStyle.GetItemSet().Get(nWhich)).GetValue();
}

}

void ScDocument::ImplCreateOptions()
{
    pDocOptions.reset( new ScDocOptions() );
    pViewOptions.reset( new ScViewOptions() );
}

void ScDocument::ImplDeleteOptions()
{
    pDocOptions.reset();
    pViewOptions.reset();
    pExtDocOptions.reset();
}

SfxPrinter* ScDocument::GetPrinter(bool bCreateIfNotExist)
{
    if ( !mpPrinter && bCreateIfNotExist )
    {
        auto pSet =
            std::make_unique<SfxItemSet>( *mxPoolHelper->GetDocPool(),
                            svl::Items<SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            SID_PRINT_SELECTEDSHEET,    SID_PRINT_SELECTEDSHEET,
                            SID_SCPRINTOPTIONS,         SID_SCPRINTOPTIONS>{} );

        ::utl::MiscCfg aMisc;
        SfxPrinterChangeFlags nFlags = SfxPrinterChangeFlags::NONE;
        if ( aMisc.IsPaperOrientationWarning() )
            nFlags |= SfxPrinterChangeFlags::CHG_ORIENTATION;
        if ( aMisc.IsPaperSizeWarning() )
            nFlags |= SfxPrinterChangeFlags::CHG_SIZE;
        pSet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, static_cast<int>(nFlags) ) );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning() ) );

        mpPrinter = VclPtr<SfxPrinter>::Create( std::move(pSet) );
        mpPrinter->SetMapMode(MapMode(MapUnit::Map100thMM));
        UpdateDrawPrinter();
        mpPrinter->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
    }

    return mpPrinter;
}

void ScDocument::SetPrinter( VclPtr<SfxPrinter> const & pNewPrinter )
{
    if ( pNewPrinter == mpPrinter.get() )
    {
        //  #i6706# SetPrinter is called with the same printer again if
        //  the JobSetup has changed. In that case just call UpdateDrawPrinter
        //  (SetRefDevice for drawing layer) because of changed text sizes.
        UpdateDrawPrinter();
    }
    else
    {
        ScopedVclPtr<SfxPrinter> pOld( mpPrinter );
        mpPrinter = pNewPrinter;
        UpdateDrawPrinter();
        mpPrinter->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
    }
    InvalidateTextWidth(nullptr, nullptr, false);     // in both cases
}

void ScDocument::SetPrintOptions()
{
    if ( !mpPrinter ) GetPrinter(); // this sets mpPrinter
    OSL_ENSURE( mpPrinter, "Error in printer creation :-/" );

    if ( mpPrinter )
    {
        ::utl::MiscCfg aMisc;
        SfxItemSet aOptSet( mpPrinter->GetOptions() );

        SfxPrinterChangeFlags nFlags = SfxPrinterChangeFlags::NONE;
        if ( aMisc.IsPaperOrientationWarning() )
            nFlags |= SfxPrinterChangeFlags::CHG_ORIENTATION;
        if ( aMisc.IsPaperSizeWarning() )
            nFlags |= SfxPrinterChangeFlags::CHG_SIZE;
        aOptSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, static_cast<int>(nFlags) ) );
        aOptSet.Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning() ) );

        mpPrinter->SetOptions( aOptSet );
    }
}

VirtualDevice* ScDocument::GetVirtualDevice_100th_mm()
{
    if (!mpVirtualDevice_100th_mm)
    {
#ifdef IOS
        mpVirtualDevice_100th_mm = VclPtr<VirtualDevice>::Create(DeviceFormat::GRAYSCALE);
#else
        mpVirtualDevice_100th_mm = VclPtr<VirtualDevice>::Create(DeviceFormat::BITMASK);
#endif
        mpVirtualDevice_100th_mm->SetReferenceDevice(VirtualDevice::RefDevMode::MSO1);
        MapMode aMapMode( mpVirtualDevice_100th_mm->GetMapMode() );
        aMapMode.SetMapUnit( MapUnit::Map100thMM );
        mpVirtualDevice_100th_mm->SetMapMode( aMapMode );
    }
    return mpVirtualDevice_100th_mm;
}

OutputDevice* ScDocument::GetRefDevice()
{
    // Create printer like ref device, see Writer...
    OutputDevice* pRefDevice = nullptr;
    if ( SC_MOD()->GetInputOptions().GetTextWysiwyg() )
        pRefDevice = GetPrinter();
    else
        pRefDevice = GetVirtualDevice_100th_mm();
    return pRefDevice;
}

void ScDocument::ModifyStyleSheet( SfxStyleSheetBase& rStyleSheet,
                                   const SfxItemSet&  rChanges )
{
    SfxItemSet& rSet = rStyleSheet.GetItemSet();

    switch ( rStyleSheet.GetFamily() )
    {
        case SfxStyleFamily::Page:
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
                    const SfxPoolItem *pItem = nullptr;
                    if( rChanges.GetItemState(ATTR_WRITINGDIR, true, &pItem ) == SfxItemState::SET )
                        ScChartHelper::DoUpdateAllCharts( this );
                }
            }
            break;

        case SfxStyleFamily::Para:
            {
                bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rSet, rChanges ) )
                    InvalidateTextWidth( nullptr, nullptr, bNumFormatChanged );

                for (SCTAB nTab=0; nTab<=MAXTAB; ++nTab)
                    if (maTabs[nTab])
                        maTabs[nTab]->SetStreamValid( false );

                sal_uLong nOldFormat =
                    rSet.Get( ATTR_VALUE_FORMAT ).GetValue();
                sal_uLong nNewFormat =
                    rChanges.Get( ATTR_VALUE_FORMAT ).GetValue();
                LanguageType eNewLang, eOldLang;
                eNewLang = eOldLang = LANGUAGE_DONTKNOW;
                if ( nNewFormat != nOldFormat )
                {
                    SvNumberFormatter* pFormatter = GetFormatTable();
                    eOldLang = pFormatter->GetEntry( nOldFormat )->GetLanguage();
                    eNewLang = pFormatter->GetEntry( nNewFormat )->GetLanguage();
                }

                // Explanation to Items in rChanges:
                //  Set Item        - take over change
                //  Dontcare        - Set Default
                //  Default         - No change
                // ("no change" is not possible with PutExtended, thus the loop)
                for (sal_uInt16 nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; nWhich++)
                {
                    const SfxPoolItem* pItem;
                    SfxItemState eState = rChanges.GetItemState( nWhich, false, &pItem );
                    if ( eState == SfxItemState::SET )
                        rSet.Put( *pItem );
                    else if ( eState == SfxItemState::DONTCARE )
                        rSet.ClearItem( nWhich );
                    // when Default nothing
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

void ScDocument::CopyStdStylesFrom( const ScDocument* pSrcDoc )
{
    // number format exchange list has to be handled here, too
    NumFmtMergeHandler aNumFmtMergeHdl(this, pSrcDoc);
    mxPoolHelper->GetStylePool()->CopyStdStylesFrom( pSrcDoc->mxPoolHelper->GetStylePool() );
}

void ScDocument::InvalidateTextWidth( const OUString& rStyleName )
{
    const SCTAB nCount = GetTableCount();
    for ( SCTAB i=0; i<nCount && maTabs[i]; i++ )
        if ( maTabs[i]->GetPageStyle() == rStyleName )
            InvalidateTextWidth( i );
}

void ScDocument::InvalidateTextWidth( SCTAB nTab )
{
    ScAddress aAdrFrom( 0,    0,        nTab );
    ScAddress aAdrTo  ( MAXCOL, MAXROW, nTab );
    InvalidateTextWidth( &aAdrFrom, &aAdrTo, false );
}

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

bool ScDocument::RemovePageStyleInUse( const OUString& rStyle )
{
    bool bWasInUse = false;
    const SCTAB nCount = GetTableCount();

    for ( SCTAB i=0; i<nCount && maTabs[i]; i++ )
        if ( maTabs[i]->GetPageStyle() == rStyle )
        {
            bWasInUse = true;
            maTabs[i]->SetPageStyle( ScResId(STR_STYLENAME_STANDARD) );
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

EEHorizontalTextDirection ScDocument::GetEditTextDirection(SCTAB nTab) const
{
    EEHorizontalTextDirection eRet = EEHorizontalTextDirection::Default;

    OUString aStyleName = GetPageStyle( nTab );
    SfxStyleSheetBase* pStyle = mxPoolHelper->GetStylePool()->Find( aStyleName, SfxStyleFamily::Page );
    if ( pStyle )
    {
        SfxItemSet& rStyleSet = pStyle->GetItemSet();
        SvxFrameDirection eDirection =
            rStyleSet.Get( ATTR_WRITINGDIR ).GetValue();

        if ( eDirection == SvxFrameDirection::Horizontal_LR_TB )
            eRet = EEHorizontalTextDirection::L2R;
        else if ( eDirection == SvxFrameDirection::Horizontal_RL_TB )
            eRet = EEHorizontalTextDirection::R2L;
        // else (invalid for EditEngine): keep "default"
    }

    return eRet;
}

ScMacroManager* ScDocument::GetMacroManager()
{
    if (!mpMacroMgr)
        mpMacroMgr.reset(new ScMacroManager(this));
    return mpMacroMgr.get();
}

void ScDocument::FillMatrix(
    ScMatrix& rMat, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, svl::SharedStringPool* pPool ) const
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

    pTab->FillMatrix(rMat, nCol1, nRow1, nCol2, nRow2, pPool);
}

void ScDocument::SetFormulaResults( const ScAddress& rTopPos, const double* pResults, size_t nLen )
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab)
        return;

    pTab->SetFormulaResults(rTopPos.Col(), rTopPos.Row(), pResults, nLen);
}

const ScDocumentThreadSpecific& ScDocument::CalculateInColumnInThread( ScInterpreterContext& rContext, const ScAddress& rTopPos, size_t nLen, unsigned nThisThread, unsigned nThreadsTotal)
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab)
        return maNonThreaded;

    assert(IsThreadedGroupCalcInProgress());

    maThreadSpecific.pContext = &rContext;
    ScDocumentThreadSpecific::SetupFromNonThreadedData(maNonThreaded);
    pTab->CalculateInColumnInThread(rContext, rTopPos.Col(), rTopPos.Row(), nLen, nThisThread, nThreadsTotal);

    assert(IsThreadedGroupCalcInProgress());
    maThreadSpecific.pContext = nullptr;

    return maThreadSpecific;
}

void ScDocument::HandleStuffAfterParallelCalculation( const ScAddress& rTopPos, size_t nLen )
{
    assert(!IsThreadedGroupCalcInProgress());
    for( DelayedSetNumberFormat& data : GetNonThreadedContext().maDelayedSetNumberFormat)
        SetNumberFormat( ScAddress( rTopPos.Col(), data.mRow, rTopPos.Tab()), data.mnNumberFormat );
    GetNonThreadedContext().maDelayedSetNumberFormat.clear();

    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab)
        return;

    pTab->HandleStuffAfterParallelCalculation(rTopPos.Col(), rTopPos.Row(), nLen);
}

void ScDocument::InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                      bool bNumFormatChanged )
{
    bool bBroadcast = (bNumFormatChanged && GetDocOptions().IsCalcAsShown() && !IsImportingXML() && !IsClipboard());
    if ( pAdrFrom && !pAdrTo )
    {
        const SCTAB nTab = pAdrFrom->Tab();

        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
            maTabs[nTab]->InvalidateTextWidth( pAdrFrom, nullptr, bNumFormatChanged, bBroadcast );
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

#define CALCMAX                 1000    // Calculations

namespace {

class IdleCalcTextWidthScope
{
    ScDocument& mrDoc;
    ScAddress& mrCalcPos;
    MapMode maOldMapMode;
    sal_uInt64 const mnStartTime;
    ScStyleSheetPool* mpStylePool;
    SfxStyleSearchBits const mnOldSearchMask;
    SfxStyleFamily const meOldFamily;
    bool mbNeedMore;
    bool mbProgress;

public:
    IdleCalcTextWidthScope(ScDocument& rDoc, ScAddress& rCalcPos) :
        mrDoc(rDoc),
        mrCalcPos(rCalcPos),
        mnStartTime(tools::Time::GetSystemTicks()),
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
        mpStylePool->SetSearchMask(SfxStyleFamily::Page);
    }

    ~IdleCalcTextWidthScope() COVERITY_NOEXCEPT_FALSE
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

    void setTab(SCTAB nTab) { mrCalcPos.SetTab(nTab); }
    void setCol(SCCOL nCol) { mrCalcPos.SetCol(nCol); }
    void setRow(SCROW nRow) { mrCalcPos.SetRow(nRow); }

    void incTab() { mrCalcPos.IncTab(); }
    void incCol(SCCOL nInc) { mrCalcPos.IncCol(nInc); }

    void setOldMapMode(const MapMode& rOldMapMode) { maOldMapMode = rOldMapMode; }

    void setNeedMore(bool b) { mbNeedMore = b; }
    bool getNeedMore() const { return mbNeedMore; }

    sal_uInt64 getStartTime() const { return mnStartTime; }

    void createProgressBar()
    {
        ScProgress::CreateInterpretProgress(&mrDoc, false);
        mbProgress = true;
    }

    bool hasProgressBar() const { return mbProgress; }

    ScStyleSheetPool* getStylePool() { return mpStylePool; }
};

}

bool ScDocument::IdleCalcTextWidth()            // true = try next again
{
    // #i75610# if a printer hasn't been set or created yet, don't create one for this
    if (!mbIdleEnabled || IsInLinkUpdate() || GetPrinter(false) == nullptr)
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

    ScTable* pTab = maTabs[aScope.Tab()].get();
    ScStyleSheet* pStyle = static_cast<ScStyleSheet*>(aScope.getStylePool()->Find(pTab->aPageStyle, SfxStyleFamily::Page));
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
    std::unique_ptr<ScColumnTextWidthIterator> pColIter(new ScColumnTextWidthIterator(*pCol, aScope.Row(), MAXROW));

    OutputDevice* pDev = nullptr;
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
                    pDev->SetMapMode(MapMode(MapUnit::MapPixel)); // Important for GetNeededSize

                    Point aPix1000 = pDev->LogicToPixel(Point(1000,1000), MapMode(MapUnit::MapTwip));
                    nPPTX = aPix1000.X() / 1000.0;
                    nPPTY = aPix1000.Y() / 1000.0;
                }

                if (!aScope.hasProgressBar() && pCol->IsFormulaDirty(nRow))
                    aScope.createProgressBar();

                sal_uInt16 nNewWidth = static_cast<sal_uInt16>(GetNeededSize(
                    aScope.Col(), aScope.Row(), aScope.Tab(),
                    pDev, nPPTX, nPPTY, aZoomFract,aZoomFract, true, true));   // bTotalSize

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
                    pTab = maTabs[aScope.Tab()].get();
                    pStyle = static_cast<ScStyleSheet*>(aScope.getStylePool()->Find(
                        pTab->aPageStyle, SfxStyleFamily::Page));

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
        VclInputFlags ABORT_EVENTS = VCL_INPUT_ANY;
        ABORT_EVENTS &= ~VclInputFlags::TIMER;
        ABORT_EVENTS &= ~VclInputFlags::OTHER;
        if ((50L < tools::Time::GetSystemTicks() - aScope.getStartTime()) || (nCount > 31 && Application::AnyInput(ABORT_EVENTS)))
            nCount = CALCMAX;
    }

    return aScope.getNeedMore();
}

void ScDocument::RepaintRange( const ScRange& rRange )
{
    if ( bIsVisible && mpShell )
    {
        ScModelObj* pModel = ScModelObj::getImplementation( mpShell->GetModel() );
        if ( pModel )
            pModel->RepaintRange( rRange );     // locked repaints are checked there
    }
}

void ScDocument::RepaintRange( const ScRangeList& rRange )
{
    if ( bIsVisible && mpShell )
    {
        ScModelObj* pModel = ScModelObj::getImplementation( mpShell->GetModel() );
        if ( pModel )
            pModel->RepaintRange( rRange );     // locked repaints are checked there
    }
}

void ScDocument::SaveDdeLinks(SvStream& rStream) const
{
    //  when 4.0-Export, remove all with mode != DEFAULT
    bool bExport40 = ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 );

    const ::sfx2::SvBaseLinks& rLinks = GetLinkManager()->GetLinks();
    sal_uInt16 nCount = rLinks.size();

    // Count them first

    sal_uInt16 nDdeCount = 0;
    sal_uInt16 i;
    for (i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        if (ScDdeLink* pLink = dynamic_cast<ScDdeLink*>(pBase))
            if ( !bExport40 || pLink->GetMode() == SC_DDE_DEFAULT )
                ++nDdeCount;
    }

    //  Header

    ScMultipleWriteHeader aHdr( rStream );
    rStream.WriteUInt16( nDdeCount );

    // Save links

    for (i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        if (ScDdeLink* pLink = dynamic_cast<ScDdeLink*>(pBase))
        {
            if ( !bExport40 || pLink->GetMode() == SC_DDE_DEFAULT )
                pLink->Store( rStream, aHdr );
        }
    }
}

void ScDocument::LoadDdeLinks(SvStream& rStream)
{
    sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(bAutoCalc);
    if (!pMgr)
        return;

    ScMultipleReadHeader aHdr( rStream );

    sal_uInt16 nCount(0);
    rStream.ReadUInt16( nCount );

    const rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    const size_t nMinStringSize = eCharSet == RTL_TEXTENCODING_UNICODE ? sizeof(sal_uInt32) : sizeof(sal_uInt16);
    const size_t nMinRecordSize = 1 + nMinStringSize*3;
    const size_t nMaxRecords = rStream.remainingSize() / nMinRecordSize;
    if (nCount > nMaxRecords)
    {
        SAL_WARN("sc", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nCount << " claimed, truncating");
        nCount = nMaxRecords;
    }

    for (sal_uInt16 i=0; i<nCount; ++i)
    {
        ScDdeLink* pLink = new ScDdeLink( this, rStream, aHdr );
        pMgr->InsertDDELink(pLink, pLink->GetAppl(), pLink->GetTopic(), pLink->GetItem());
    }
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

void ScDocument::UpdateExternalRefLinks(vcl::Window* pWin)
{
    if (!pExternalRefMgr)
        return;

    sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(bAutoCalc);
    if (!pMgr)
        return;

    const ::sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    sal_uInt16 nCount = rLinks.size();

    bool bAny = false;

    // Collect all the external ref links first.
    std::vector<ScExternalRefLink*> aRefLinks;
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        ScExternalRefLink* pRefLink = dynamic_cast<ScExternalRefLink*>(pBase);
        if (pRefLink)
            aRefLinks.push_back(pRefLink);
    }

    sc::WaitPointerSwitch aWaitSwitch(pWin);

    pExternalRefMgr->enableDocTimer(false);
    ScProgress aProgress(GetDocumentShell(), ScResId(SCSTR_UPDATE_EXTDOCS), aRefLinks.size(), true);
    for (size_t i = 0, n = aRefLinks.size(); i < n; ++i)
    {
        aProgress.SetState(i+1);

        ScExternalRefLink* pRefLink = aRefLinks[i];
        if (pRefLink->Update())
        {
            bAny = true;
            continue;
        }

        // Update failed.  Notify the user.

        OUString aFile;
        sfx2::LinkManager::GetDisplayNames(pRefLink, nullptr, &aFile);
        // Decode encoded URL for display friendliness.
        INetURLObject aUrl(aFile,INetURLObject::EncodeMechanism::WasEncoded);
        aFile = aUrl.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);

        OUStringBuffer aBuf;
        aBuf.append(ScResId(SCSTR_EXTDOC_NOT_LOADED));
        aBuf.append("\n\n");
        aBuf.append(aFile);
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aBuf.makeStringAndClear()));
        xBox->run();
    }

    pExternalRefMgr->enableDocTimer(true);

    if (bAny)
    {
        TrackFormulas();
        mpShell->Broadcast( SfxHint(SfxHintId::ScDataChanged) );

        // #i101960# set document modified, as in TrackTimeHdl for DDE links
        if (!mpShell->IsModified())
        {
            mpShell->SetModified();
            SfxBindings* pBindings = GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( SID_SAVEDOC );
                pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
    }
}

void ScDocument::CopyDdeLinks( ScDocument* pDestDoc ) const
{
    if (bIsClip)        // Create from Stream
    {
        if (pClipData)
        {
            pClipData->Seek(0);
            pDestDoc->LoadDdeLinks(*pClipData);
        }

        return;
    }

    const sfx2::LinkManager* pMgr = GetDocLinkManager().getExistingLinkManager();
    if (!pMgr)
        return;

    sfx2::LinkManager* pDestMgr = pDestDoc->GetDocLinkManager().getLinkManager(pDestDoc->bAutoCalc);
    if (!pDestMgr)
        return;

    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    for (const auto & rLink : rLinks)
    {
        const sfx2::SvBaseLink* pBase = rLink.get();
        if (const ScDdeLink* p = dynamic_cast<const ScDdeLink*>(pBase))
        {
            ScDdeLink* pNew = new ScDdeLink(pDestDoc, *p);
            pDestMgr->InsertDDELink(
                pNew, pNew->GetAppl(), pNew->GetTopic(), pNew->GetItem());
        }
    }
}

namespace {

/** Tries to find the specified DDE link.
    @param pnDdePos  (out-param) if not 0, the index of the DDE link is returned here
                     (does not include other links from link manager).
    @return  The DDE link, if it exists, otherwise 0. */
ScDdeLink* lclGetDdeLink(
        const sfx2::LinkManager* pLinkManager,
        const OUString& rAppl, const OUString& rTopic, const OUString& rItem, sal_uInt8 nMode,
        size_t* pnDdePos = nullptr )
{
    if( pLinkManager )
    {
        const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
        size_t nCount = rLinks.size();
        if( pnDdePos ) *pnDdePos = 0;
        for( size_t nIndex = 0; nIndex < nCount; ++nIndex )
        {
            ::sfx2::SvBaseLink* pLink = rLinks[ nIndex ].get();
            if( ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>( pLink )  )
            {
                if( (pDdeLink->GetAppl() == rAppl) &&
                    (pDdeLink->GetTopic() == rTopic) &&
                    (pDdeLink->GetItem() == rItem) &&
                    ((nMode == SC_DDE_IGNOREMODE) || (nMode == pDdeLink->GetMode())) )
                    return pDdeLink;
                if( pnDdePos ) ++*pnDdePos;
            }
        }
    }
    return nullptr;
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
            ::sfx2::SvBaseLink* pLink = rLinks[ nIndex ].get();
            if( ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>( pLink )  )
            {
                if( nDdeIndex == nDdePos )
                    return pDdeLink;
                ++nDdeIndex;
            }
        }
    }
    return nullptr;
}

} // namespace

bool ScDocument::FindDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem,
        sal_uInt8 nMode, size_t& rnDdePos )
{
    return lclGetDdeLink( GetLinkManager(), rAppl, rTopic, rItem, nMode, &rnDdePos ) != nullptr;
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
    return pDdeLink ? pDdeLink->GetResult() : nullptr;
}

bool ScDocument::CreateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem, sal_uInt8 nMode, const ScMatrixRef& pResults )
{
    /*  Create a DDE link without updating it (i.e. for Excel import), to prevent
        unwanted connections. First try to find existing link. Set result array
        on existing and new links. */
    //TODO: store DDE links additionally at document (for efficiency)?
    OSL_ENSURE( nMode != SC_DDE_IGNOREMODE, "ScDocument::CreateDdeLink - SC_DDE_IGNOREMODE not allowed here" );

    sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(bAutoCalc);
    if (!pMgr)
        return false;

    if (nMode != SC_DDE_IGNOREMODE)
    {
        ScDdeLink* pDdeLink = lclGetDdeLink(pMgr, rAppl, rTopic, rItem, nMode);
        if( !pDdeLink )
        {
            // create a new DDE link, but without TryUpdate
            pDdeLink = new ScDdeLink( this, rAppl, rTopic, rItem, nMode );
            pMgr->InsertDDELink(pDdeLink, rAppl, rTopic, rItem);
        }

        // insert link results
        if( pResults )
            pDdeLink->SetResult( pResults );

        return true;
    }
    return false;
}

bool ScDocument::SetDdeLinkResultMatrix( size_t nDdePos, const ScMatrixRef& pResults )
{
    if( ScDdeLink* pDdeLink = lclGetDdeLink( GetLinkManager(), nDdePos ) )
    {
        pDdeLink->SetResult( pResults );
        return true;
    }
    return false;
}

bool ScDocument::HasAreaLinks() const
{
    const sfx2::LinkManager* pMgr = GetDocLinkManager().getExistingLinkManager();
    if (!pMgr)
        return false;

    const ::sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    sal_uInt16 nCount = rLinks.size();
    for (sal_uInt16 i=0; i<nCount; i++)
        if (nullptr != dynamic_cast<const ScAreaLink* >(rLinks[i].get()))
            return true;

    return false;
}

void ScDocument::UpdateAreaLinks()
{
    sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(false);
    if (!pMgr)
        return;

    const ::sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    for (const auto & rLink : rLinks)
    {
        ::sfx2::SvBaseLink* pBase = rLink.get();
        if (dynamic_cast<const ScAreaLink*>( pBase) !=  nullptr)
            pBase->Update();
    }
}

void ScDocument::DeleteAreaLinksOnTab( SCTAB nTab )
{
    sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(false);
    if (!pMgr)
        return;

    const ::sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    sfx2::SvBaseLinks::size_type nPos = 0;
    while ( nPos < rLinks.size() )
    {
        const ::sfx2::SvBaseLink* pBase = rLinks[nPos].get();
        const ScAreaLink* pLink = dynamic_cast<const ScAreaLink*>(pBase);
        if (pLink && pLink->GetDestArea().aStart.Tab() == nTab)
            pMgr->Remove(nPos);
        else
            ++nPos;
    }
}

void ScDocument::UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(false);
    if (!pMgr)
        return;

    bool bAnyUpdate = false;

    const ::sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    sal_uInt16 nCount = rLinks.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        if (ScAreaLink* pLink = dynamic_cast<ScAreaLink*>(pBase))
        {
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
            ::sfx2::SvBaseLink* pFirst = rLinks[nFirstIndex].get();
            if (ScAreaLink* pFirstLink = dynamic_cast<ScAreaLink*>(pFirst))
            {
                ScAddress aFirstPos = pFirstLink->GetDestArea().aStart;
                for ( sal_uInt16 nSecondIndex = nFirstIndex + 1; nSecondIndex < nCount && !bFound; ++nSecondIndex )
                {
                    ::sfx2::SvBaseLink* pSecond = rLinks[nSecondIndex].get();
                    ScAreaLink* pSecondLink = dynamic_cast<ScAreaLink*>(pSecond);
                    if (pSecondLink && pSecondLink->GetDestArea().aStart == aFirstPos)
                    {
                        // remove the first link, exit the inner loop, don't increment nFirstIndex
                        pMgr->Remove(pFirst);
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

void ScDocument::CheckLinkFormulaNeedingCheck( const ScTokenArray& rCode )
{
    if (HasLinkFormulaNeedingCheck())
        return;

    // Prefer RPN over tokenized formula if available.
    if (rCode.GetCodeLen())
    {
        if (rCode.HasOpCodeRPN(ocDde) || rCode.HasOpCodeRPN(ocWebservice))
            SetLinkFormulaNeedingCheck(true);
    }
    else if (rCode.GetLen())
    {
        if (rCode.HasOpCode(ocDde) || rCode.HasOpCode(ocWebservice))
            SetLinkFormulaNeedingCheck(true);
    }
    else
    {
        // Possible with named expression without expression like Excel
        // internal print ranges, obscure user define names, ... formula error
        // cells without formula ...
        SAL_WARN("sc.core","ScDocument::CheckLinkFormulaNeedingCheck - called with empty ScTokenArray");
    }
}

// TimerDelays etc.
void ScDocument::KeyInput()
{
    if ( pChartListenerCollection->hasListeners() )
        pChartListenerCollection->StartTimer();
    if (apTemporaryChartLock)
        apTemporaryChartLock->StartOrContinueLocking();
}

SfxBindings* ScDocument::GetViewBindings()
{
    //  used to invalidate slots after changes to this document

    if ( !mpShell )
        return nullptr;        // no ObjShell -> no view

    //  first check current view
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame && pViewFrame->GetObjectShell() != mpShell )     // wrong document?
        pViewFrame = nullptr;

    //  otherwise use first view for this doc
    if ( !pViewFrame )
        pViewFrame = SfxViewFrame::GetFirst( mpShell );

    if (pViewFrame)
        return &pViewFrame->GetBindings();
    else
        return nullptr;
}

void ScDocument::TransliterateText( const ScMarkData& rMultiMark, TransliterationFlags nType )
{
    OSL_ENSURE( rMultiMark.IsMultiMarked(), "TransliterateText: no selection" );

    utl::TransliterationWrapper aTransliterationWrapper( comphelper::getProcessComponentContext(), nType );
    bool bConsiderLanguage = aTransliterationWrapper.needLanguageForTheMode();
    LanguageType nLanguage = LANGUAGE_SYSTEM;

    std::unique_ptr<ScEditEngineDefaulter> pEngine;        // not using mpEditEngine member because of defaults

    SCTAB nCount = GetTableCount();
    for (const SCTAB& nTab : rMultiMark)
    {
        if (nTab >= nCount)
            break;

        if ( maTabs[nTab] )
        {
            SCCOL nCol = 0;
            SCROW nRow = 0;

            bool bFound = rMultiMark.IsCellMarked( nCol, nRow );
            if (!bFound)
                bFound = GetNextMarkedCell( nCol, nRow, nTab, rMultiMark );

            while (bFound)
            {
                ScRefCellValue aCell(*this, ScAddress(nCol, nRow, nTab));

                // fdo#32786 TITLE_CASE/SENTENCE_CASE need the extra handling in EditEngine (loop over words/sentences).
                // Still use TransliterationWrapper directly for text cells with other transliteration types,
                // for performance reasons.
                if (aCell.meType == CELLTYPE_EDIT ||
                    (aCell.meType == CELLTYPE_STRING &&
                     ( nType == TransliterationFlags::SENTENCE_CASE || nType == TransliterationFlags::TITLE_CASE)))
                {
                    if (!pEngine)
                        pEngine.reset(new ScFieldEditEngine(this, GetEnginePool(), GetEditPool()));

                    // defaults from cell attributes must be set so right language is used
                    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
                    SfxItemSet* pDefaults = new SfxItemSet( pEngine->GetEmptyItemSet() );
                    if ( ScStyleSheet* pPreviewStyle = GetPreviewCellStyle( nCol, nRow, nTab ) )
                    {
                        std::unique_ptr<ScPatternAttr> pPreviewPattern(new ScPatternAttr( *pPattern ));
                        pPreviewPattern->SetStyleSheet(pPreviewStyle);
                        pPreviewPattern->FillEditItemSet( pDefaults );
                    }
                    else
                    {
                        SfxItemSet* pFontSet = GetPreviewFont( nCol, nRow, nTab );
                        pPattern->FillEditItemSet( pDefaults, pFontSet );
                    }
                    pEngine->SetDefaults( pDefaults );
                    if (aCell.meType == CELLTYPE_STRING)
                        pEngine->SetText(aCell.mpString->getString());
                    else if (aCell.mpEditText)
                        pEngine->SetText(*aCell.mpEditText);

                    pEngine->ClearModifyFlag();

                    sal_Int32 nLastPar = pEngine->GetParagraphCount();
                    if (nLastPar)
                        --nLastPar;
                    sal_Int32 nTxtLen = pEngine->GetTextLen(nLastPar);
                    ESelection aSelAll( 0, 0, nLastPar, nTxtLen );

                    pEngine->TransliterateText( aSelAll, nType );

                    if ( pEngine->IsModified() )
                    {
                        ScEditAttrTester aTester( pEngine.get() );
                        if ( aTester.NeedsObject() )
                        {
                            // remove defaults (paragraph attributes) before creating text object
                            SfxItemSet* pEmpty = new SfxItemSet( pEngine->GetEmptyItemSet() );
                            pEngine->SetDefaults( pEmpty );

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
                    OUString aOldStr = aCell.mpString->getString();
                    sal_Int32 nOldLen = aOldStr.getLength();

                    if ( bConsiderLanguage )
                    {
                        SvtScriptType nScript = GetStringScriptType( aOldStr );        //TODO: cell script type?
                        sal_uInt16 nWhich = ( nScript == SvtScriptType::ASIAN ) ? ATTR_CJK_FONT_LANGUAGE :
                                        ( ( nScript == SvtScriptType::COMPLEX ) ? ATTR_CTL_FONT_LANGUAGE :
                                                                                ATTR_FONT_LANGUAGE );
                        nLanguage = static_cast<const SvxLanguageItem*>(GetAttr( nCol, nRow, nTab, nWhich ))->GetValue();
                    }

                    uno::Sequence<sal_Int32> aOffsets;
                    OUString aNewStr = aTransliterationWrapper.transliterate( aOldStr, nLanguage, 0, nOldLen, &aOffsets );

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
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
