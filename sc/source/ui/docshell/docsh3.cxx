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

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <scitems.hxx>
#include <rangelst.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <sal/log.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svx/postattr.hxx>
#include <unotools/misccfg.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>

#include <docsh.hxx>
#include "docshimp.hxx"
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <docpool.hxx>
#include <stlpool.hxx>
#include <patattr.hxx>
#include <uiitems.hxx>
#include <hints.hxx>
#include <docoptio.hxx>
#include <viewopti.hxx>
#include <pntlock.hxx>
#include <chgtrack.hxx>
#include <docfunc.hxx>
#include <formulacell.hxx>
#include <chgviset.hxx>
#include <progress.hxx>
#include <redcom.hxx>
#include <inputopt.hxx>
#include <drwlayer.hxx>
#include <inputhdl.hxx>
#include <conflictsdlg.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <markdata.hxx>
#include <memory>
#include <formulaopt.hxx>

#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

//          Redraw - Notifications

void ScDocShell::PostEditView( ScEditEngineDefaulter* pEditEngine, const ScAddress& rCursorPos )
{
//  Broadcast( ScEditViewHint( pEditEngine, rCursorPos ) );

        //  Test: only active ViewShell

    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh && pViewSh->GetViewData().GetDocShell() == this)
    {
        ScEditViewHint aHint( pEditEngine, rCursorPos );
        pViewSh->Notify( *this, aHint );
    }
}

void ScDocShell::PostDataChanged()
{
    Broadcast( SfxHint( SfxHintId::ScDataChanged ) );
    SfxGetpApp()->Broadcast(SfxHint( SfxHintId::ScAnyDataChanged ));      // Navigator
    m_aDocument.PrepareFormulaCalc();
    //! notify navigator directly!
}

void ScDocShell::PostPaint( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab, PaintPartFlags nPart,
                            sal_uInt16 nExtFlags )
{
    ScRange aRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
    PostPaint(aRange, nPart, nExtFlags);
}

void ScDocShell::PostPaint( const ScRangeList& rRanges, PaintPartFlags nPart, sal_uInt16 nExtFlags )
{
    ScRangeList aPaintRanges;
    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange& rRange = rRanges[i];
        SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
        SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
        SCTAB nTab1 = rRange.aStart.Tab(), nTab2 = rRange.aEnd.Tab();

        if (!ValidCol(nCol1)) nCol1 = MAXCOL;
        if (!ValidRow(nRow1)) nRow1 = MAXROW;
        if (!ValidCol(nCol2)) nCol2 = MAXCOL;
        if (!ValidRow(nRow2)) nRow2 = MAXROW;

        if ( m_pPaintLockData )
        {
            // #i54081# PaintPartFlags::Extras still has to be broadcast because it changes the
            // current sheet if it's invalid. All other flags added to pPaintLockData.
            PaintPartFlags nLockPart = nPart & ~PaintPartFlags::Extras;
            if ( nLockPart != PaintPartFlags::NONE )
            {
                //! nExtFlags ???
                m_pPaintLockData->AddRange( ScRange( nCol1, nRow1, nTab1,
                                                   nCol2, nRow2, nTab2 ), nLockPart );
            }

            nPart &= PaintPartFlags::Extras;  // for broadcasting
            if (nPart == PaintPartFlags::NONE)
                continue;
        }

        if (nExtFlags & SC_PF_LINES)            // respect space for lines
        {
                                                //! check for hidden columns/rows!
            if (nCol1>0) --nCol1;
            if (nCol2<MAXCOL) ++nCol2;
            if (nRow1>0) --nRow1;
            if (nRow2<MAXROW) ++nRow2;
        }

                                                // expand for the merged ones
        if (nExtFlags & SC_PF_TESTMERGE)
            m_aDocument.ExtendMerge( nCol1, nRow1, nCol2, nRow2, nTab1 );

        if ( nCol1 != 0 || nCol2 != MAXCOL )
        {
            //  Extend to whole rows if SC_PF_WHOLEROWS is set, or rotated or non-left
            //  aligned cells are contained (see UpdatePaintExt).
            //  Special handling for RTL text (#i9731#) is unnecessary now with full
            //  support of right-aligned text.

            if ( ( nExtFlags & SC_PF_WHOLEROWS ) ||
                 m_aDocument.HasAttrib( nCol1,nRow1,nTab1,
                                      MAXCOL,nRow2,nTab2, HasAttrFlags::Rotate | HasAttrFlags::RightOrCenter ) )
            {
                nCol1 = 0;
                nCol2 = MAXCOL;
            }
        }
        aPaintRanges.push_back(ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2));
    }

    Broadcast(ScPaintHint(aPaintRanges.Combine(), nPart));

    // LOK: we are supposed to update the row / columns headers (and actually
    // the document size too - cell size affects that, obviously)
    if ((nPart & (PaintPartFlags::Top | PaintPartFlags::Left)) && comphelper::LibreOfficeKit::isActive())
    {
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while (pViewShell)
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
            pViewShell = SfxViewShell::GetNext(*pViewShell);
        }
    }
}

void ScDocShell::PostPaintGridAll()
{
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Grid );
}

void ScDocShell::PostPaintCell( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PaintPartFlags::Grid, SC_PF_TESTMERGE );
}

void ScDocShell::PostPaintCell( const ScAddress& rPos )
{
    PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
}

void ScDocShell::PostPaintExtras()
{
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Extras );
}

void ScDocShell::UpdatePaintExt( sal_uInt16& rExtFlags, const ScRange& rRange )
{
    if ( ( rExtFlags & SC_PF_LINES ) == 0 &&
         m_aDocument.HasAttrib( rRange, HasAttrFlags::Lines | HasAttrFlags::Shadow | HasAttrFlags::Conditional ) )
    {
        //  If the range contains lines, shadow or conditional formats,
        //  set SC_PF_LINES to include one extra cell in all directions.

        rExtFlags |= SC_PF_LINES;
    }

    if ( ( rExtFlags & SC_PF_WHOLEROWS ) == 0 &&
         ( rRange.aStart.Col() != 0 || rRange.aEnd.Col() != MAXCOL ) &&
         m_aDocument.HasAttrib( rRange, HasAttrFlags::Rotate | HasAttrFlags::RightOrCenter ) )
    {
        //  If the range contains (logically) right- or center-aligned cells,
        //  or rotated cells, set SC_PF_WHOLEROWS to paint the whole rows.
        //  This test isn't needed after the cell changes, because it's also
        //  tested in PostPaint. UpdatePaintExt may later be changed to do this
        //  only if called before the changes.

        rExtFlags |= SC_PF_WHOLEROWS;
    }
}

void ScDocShell::UpdatePaintExt( sal_uInt16& rExtFlags, SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                                   SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab )
{
    UpdatePaintExt( rExtFlags, ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab ) );
}

void ScDocShell::LockPaint_Impl(bool bDoc)
{
    if ( !m_pPaintLockData )
        m_pPaintLockData.reset( new ScPaintLockData );
    m_pPaintLockData->IncLevel(bDoc);
}

void ScDocShell::UnlockPaint_Impl(bool bDoc)
{
    if ( m_pPaintLockData )
    {
        if ( m_pPaintLockData->GetLevel(bDoc) )
            m_pPaintLockData->DecLevel(bDoc);
        if (!m_pPaintLockData->GetLevel(!bDoc) && !m_pPaintLockData->GetLevel(bDoc))
        {
            //     Execute Paint now

            // don't continue collecting
            std::unique_ptr<ScPaintLockData> pPaint = std::move(m_pPaintLockData);

            ScRangeListRef xRangeList = pPaint->GetRangeList();
            if ( xRangeList.is() )
            {
                PaintPartFlags nParts = pPaint->GetParts();
                for ( size_t i = 0, nCount = xRangeList->size(); i < nCount; i++ )
                {
                    //! nExtFlags ???
                    ScRange const & rRange = (*xRangeList)[i];
                    PostPaint( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                                rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(),
                                nParts );
                }
            }

            if ( pPaint->GetModified() )
                SetDocumentModified();
        }
    }
    else
    {
        OSL_FAIL("UnlockPaint without LockPaint");
    }
}

void ScDocShell::LockDocument_Impl(sal_uInt16 nNew)
{
    if (!m_nDocumentLock)
    {
        ScDrawLayer* pDrawLayer = m_aDocument.GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->setLock(true);
    }
    m_nDocumentLock = nNew;
}

void ScDocShell::UnlockDocument_Impl(sal_uInt16 nNew)
{
    m_nDocumentLock = nNew;
    if (!m_nDocumentLock)
    {
        ScDrawLayer* pDrawLayer = m_aDocument.GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->setLock(false);
    }
}

void ScDocShell::SetLockCount(sal_uInt16 nNew)
{
    if (nNew)                   // set
    {
        if ( !m_pPaintLockData )
            m_pPaintLockData.reset( new ScPaintLockData );
        m_pPaintLockData->SetDocLevel(nNew-1);
        LockDocument_Impl(nNew);
    }
    else if (m_pPaintLockData)    // delete
    {
        m_pPaintLockData->SetDocLevel(0);  // at unlock, execute immediately
        UnlockPaint_Impl(true);                 // now
        UnlockDocument_Impl(0);
    }
}

void ScDocShell::LockPaint()
{
    LockPaint_Impl(false);
}

void ScDocShell::UnlockPaint()
{
    UnlockPaint_Impl(false);
}

void ScDocShell::LockDocument()
{
    LockPaint_Impl(true);
    LockDocument_Impl(m_nDocumentLock + 1);
}

void ScDocShell::UnlockDocument()
{
    if (m_nDocumentLock)
    {
        UnlockPaint_Impl(true);
        UnlockDocument_Impl(m_nDocumentLock - 1);
    }
    else
    {
        OSL_FAIL("UnlockDocument without LockDocument");
    }
}

void ScDocShell::SetInplace( bool bInplace )
{
    if (m_bIsInplace != bInplace)
    {
        m_bIsInplace = bInplace;
        CalcOutputFactor();
    }
}

void ScDocShell::CalcOutputFactor()
{
    if (m_bIsInplace)
    {
        m_nPrtToScreenFactor = 1.0;           // otherwise it does not match the inactive display
        return;
    }

    bool bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    if (bTextWysiwyg)
    {
        m_nPrtToScreenFactor = 1.0;
        return;
    }

    OUString aTestString(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890123456789");
    long nPrinterWidth = 0;
    long nWindowWidth = 0;
    const ScPatternAttr* pPattern = &m_aDocument.GetPool()->GetDefaultItem(ATTR_PATTERN);

    vcl::Font aDefFont;
    OutputDevice* pRefDev = GetRefDevice();
    MapMode aOldMode = pRefDev->GetMapMode();
    vcl::Font aOldFont = pRefDev->GetFont();

    pRefDev->SetMapMode(MapMode(MapUnit::MapPixel));
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pRefDev); // font color doesn't matter here
    pRefDev->SetFont(aDefFont);
    nPrinterWidth = pRefDev->PixelToLogic(Size(pRefDev->GetTextWidth(aTestString), 0), MapMode(MapUnit::Map100thMM)).Width();
    pRefDev->SetFont(aOldFont);
    pRefDev->SetMapMode(aOldMode);

    ScopedVclPtrInstance< VirtualDevice > pVirtWindow( *Application::GetDefaultDevice() );
    pVirtWindow->SetMapMode(MapMode(MapUnit::MapPixel));
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pVirtWindow);    // font color doesn't matter here
    pVirtWindow->SetFont(aDefFont);
    nWindowWidth = pVirtWindow->GetTextWidth(aTestString);
    nWindowWidth = static_cast<long>( nWindowWidth / ScGlobal::nScreenPPTX * HMM_PER_TWIPS );

    if (nPrinterWidth && nWindowWidth)
        m_nPrtToScreenFactor = nPrinterWidth / static_cast<double>(nWindowWidth);
    else
    {
        OSL_FAIL("GetTextSize returns 0 ??");
        m_nPrtToScreenFactor = 1.0;
    }
}

void ScDocShell::InitOptions(bool bForLoading)      // called from InitNew and Load
{
    //  Settings from the SpellCheckCfg get into Doc- and ViewOptions

    LanguageType nDefLang, nCjkLang, nCtlLang;
    bool bAutoSpell;
    ScModule::GetSpellSettings( nDefLang, nCjkLang, nCtlLang, bAutoSpell );
    ScModule* pScMod = SC_MOD();

    ScDocOptions  aDocOpt  = pScMod->GetDocOptions();
    ScFormulaOptions aFormulaOpt = pScMod->GetFormulaOptions();
    ScViewOptions aViewOpt = pScMod->GetViewOptions();
    aDocOpt.SetAutoSpell( bAutoSpell );

    // two-digit year entry from Tools->Options->General
    aDocOpt.SetYear2000( sal::static_int_cast<sal_uInt16>( ::utl::MiscCfg().GetYear2000() ) );

    if (bForLoading)
    {
        // #i112123# No style:decimal-places attribute means automatic decimals, not the configured default,
        // so it must not be taken from the global options.
        // Calculation settings are handled separately in ScXMLBodyContext::EndElement.
        aDocOpt.SetStdPrecision( SvNumberFormatter::UNLIMITED_PRECISION );

        // fdo#78294 The default null-date if
        // <table:null-date table:date-value='...' />
        // is absent is 1899-12-30 regardless what the configuration is set to.
        // Import filters may override this value.
        aDocOpt.SetDate( 30, 12, 1899);
    }

    m_aDocument.SetDocOptions( aDocOpt );
    m_aDocument.SetViewOptions( aViewOpt );
    SetFormulaOptions( aFormulaOpt, bForLoading );

    //  print options are now set directly before the printing

    m_aDocument.SetLanguage( nDefLang, nCjkLang, nCtlLang );
}

Printer* ScDocShell::GetDocumentPrinter()       // for OLE
{
    return m_aDocument.GetPrinter();
}

SfxPrinter* ScDocShell::GetPrinter(bool bCreateIfNotExist)
{
    return m_aDocument.GetPrinter(bCreateIfNotExist);
}

void ScDocShell::UpdateFontList()
{
    // pImpl->pFontList = new FontList( GetPrinter(), Application::GetDefaultDevice() );
    m_pImpl->pFontList.reset(new FontList(GetRefDevice(), nullptr));
    SvxFontListItem aFontListItem( m_pImpl->pFontList.get(), SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );

    CalcOutputFactor();
}

OutputDevice* ScDocShell::GetRefDevice()
{
    return m_aDocument.GetRefDevice();
}

sal_uInt16 ScDocShell::SetPrinter( VclPtr<SfxPrinter> const & pNewPrinter, SfxPrinterChangeFlags nDiffFlags )
{
    SfxPrinter *pOld = m_aDocument.GetPrinter( false );
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    if (nDiffFlags & SfxPrinterChangeFlags::PRINTER)
    {
        if ( m_aDocument.GetPrinter() != pNewPrinter )
        {
            m_aDocument.SetPrinter( pNewPrinter );
            m_aDocument.SetPrintOptions();

            // MT: Use UpdateFontList: Will use Printer fonts only if needed!
            /*
            delete pImpl->pFontList;
            pImpl->pFontList = new FontList( pNewPrinter, Application::GetDefaultDevice() );
            SvxFontListItem aFontListItem( pImpl->pFontList, SID_ATTR_CHAR_FONTLIST );
            PutItem( aFontListItem );

            CalcOutputFactor();
            */
            if ( SC_MOD()->GetInputOptions().GetTextWysiwyg() )
                UpdateFontList();

            ScModule* pScMod = SC_MOD();
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
            while (pFrame)
            {
                SfxViewShell* pSh = pFrame->GetViewShell();
                if (ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(pSh))
                {
                    ScInputHandler* pInputHdl = pScMod->GetInputHdl(pViewSh);
                    if (pInputHdl)
                        pInputHdl->UpdateRefDevice();
                }
                pFrame = SfxViewFrame::GetNext( *pFrame, this );
            }
        }
    }
    else if (nDiffFlags & SfxPrinterChangeFlags::JOBSETUP)
    {
        SfxPrinter* pOldPrinter = m_aDocument.GetPrinter();
        if (pOldPrinter)
        {
            pOldPrinter->SetJobSetup( pNewPrinter->GetJobSetup() );

            //  #i6706# Call SetPrinter with the old printer again, so the drawing layer
            //  RefDevice is set (calling ReformatAllTextObjects and rebuilding charts),
            //  because the JobSetup (printer device settings) may affect text layout.
            m_aDocument.SetPrinter( pOldPrinter );
            CalcOutputFactor();                         // also with the new settings
        }
    }

    if (nDiffFlags & SfxPrinterChangeFlags::OPTIONS)
    {
        m_aDocument.SetPrintOptions();        //! from new printer ???
    }

    if (nDiffFlags & (SfxPrinterChangeFlags::CHG_ORIENTATION | SfxPrinterChangeFlags::CHG_SIZE))
    {
        OUString aStyle = m_aDocument.GetPageStyle( GetCurTab() );
        ScStyleSheetPool* pStPl = m_aDocument.GetStyleSheetPool();
        SfxStyleSheet* pStyleSheet = static_cast<SfxStyleSheet*>(pStPl->Find(aStyle, SfxStyleFamily::Page));
        if (pStyleSheet)
        {
            SfxItemSet& rSet = pStyleSheet->GetItemSet();

            if (nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION)
            {
                const SvxPageItem& rOldItem = rSet.Get(ATTR_PAGE);
                bool bWasLand = rOldItem.IsLandscape();
                bool bNewLand = ( pNewPrinter->GetOrientation() == Orientation::Landscape );
                if (bNewLand != bWasLand)
                {
                    SvxPageItem aNewItem( rOldItem );
                    aNewItem.SetLandscape( bNewLand );
                    rSet.Put( aNewItem );

                    // flip size
                    Size aOldSize = rSet.Get(ATTR_PAGE_SIZE).GetSize();
                    Size aNewSize(aOldSize.Height(),aOldSize.Width());
                    SvxSizeItem aNewSItem(ATTR_PAGE_SIZE,aNewSize);
                    rSet.Put( aNewSItem );
                }
            }
            if (nDiffFlags & SfxPrinterChangeFlags::CHG_SIZE)
            {
                SvxSizeItem aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetPaperSize(pNewPrinter) );
                rSet.Put( aPaperSizeItem );
            }
        }
    }

    PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PaintPartFlags::All);

    return 0;
}

ScChangeAction* ScDocShell::GetChangeAction( const ScAddress& rPos )
{
    ScChangeTrack* pTrack = GetDocument().GetChangeTrack();
    if (!pTrack)
        return nullptr;

    SCTAB nTab = rPos.Tab();

    const ScChangeAction* pFound = nullptr;
    const ScChangeAction* pAction = pTrack->GetFirst();
    while (pAction)
    {
        ScChangeActionType eType = pAction->GetType();
        //! ScViewUtil::IsActionShown( *pAction, *pSettings, *pDoc )...
        if ( pAction->IsVisible() && eType != SC_CAT_DELETE_TABS )
        {
            const ScBigRange& rBig = pAction->GetBigRange();
            if ( rBig.aStart.Tab() == nTab )
            {
                ScRange aRange = rBig.MakeRange();

                if ( eType == SC_CAT_DELETE_ROWS )
                    aRange.aEnd.SetRow( aRange.aStart.Row() );
                else if ( eType == SC_CAT_DELETE_COLS )
                    aRange.aEnd.SetCol( aRange.aStart.Col() );

                if ( aRange.In( rPos ) )
                {
                    pFound = pAction;       // the last one wins
                }
            }
            if ( pAction->GetType() == SC_CAT_MOVE )
            {
                ScRange aRange =
                    static_cast<const ScChangeActionMove*>(pAction)->
                    GetFromRange().MakeRange();
                if ( aRange.In( rPos ) )
                {
                    pFound = pAction;
                }
            }
        }
        pAction = pAction->GetNext();
    }

    return const_cast<ScChangeAction*>(pFound);
}

void ScDocShell::SetChangeComment( ScChangeAction* pAction, const OUString& rComment )
{
    if (pAction)
    {
        pAction->SetComment( rComment );
        //! Undo ???
        SetDocumentModified();

        //  Dialog-Notify
        ScChangeTrack* pTrack = GetDocument().GetChangeTrack();
        if (pTrack)
        {
            sal_uLong nNumber = pAction->GetActionNumber();
            pTrack->NotifyModified( ScChangeTrackMsgType::Change, nNumber, nNumber );
        }
    }
}

void ScDocShell::ExecuteChangeCommentDialog( ScChangeAction* pAction, weld::Window* pParent, bool bPrevNext)
{
    if (!pAction) return;           // without action is nothing..

    OUString aComment = pAction->GetComment();
    OUString aAuthor = pAction->GetUser();

    DateTime aDT = pAction->GetDateTime();
    OUString aDate = ScGlobal::pLocaleData->getDate( aDT );
    aDate += " ";
    aDate += ScGlobal::pLocaleData->getTime( aDT, false );

    SfxItemSet aSet(
        GetPool(), svl::Items<SID_ATTR_POSTIT_AUTHOR, SID_ATTR_POSTIT_TEXT>{});

    aSet.Put( SvxPostItTextItem  ( aComment, SID_ATTR_POSTIT_TEXT ) );
    aSet.Put( SvxPostItAuthorItem( aAuthor,  SID_ATTR_POSTIT_AUTHOR ) );
    aSet.Put( SvxPostItDateItem  ( aDate,    SID_ATTR_POSTIT_DATE ) );

    std::unique_ptr<ScRedComDialog> pDlg(new ScRedComDialog( pParent, aSet,this,pAction,bPrevNext));

    pDlg->Execute();
}

void ScDocShell::CompareDocument( ScDocument& rOtherDoc )
{
    ScChangeTrack* pTrack = m_aDocument.GetChangeTrack();
    if ( pTrack && pTrack->GetFirst() )
    {
        //! there are changes -> inquiry if needs to be deleted
    }

    m_aDocument.EndChangeTracking();
    m_aDocument.StartChangeTracking();

    OUString aOldUser;
    pTrack = m_aDocument.GetChangeTrack();
    if ( pTrack )
    {
        aOldUser = pTrack->GetUser();

        //  check if comparing to same document

        OUString aThisFile;
        const SfxMedium* pThisMed = GetMedium();
        if (pThisMed)
            aThisFile = pThisMed->GetName();
        OUString aOtherFile;
        SfxObjectShell* pOtherSh = rOtherDoc.GetDocumentShell();
        if (pOtherSh)
        {
            const SfxMedium* pOtherMed = pOtherSh->GetMedium();
            if (pOtherMed)
                aOtherFile = pOtherMed->GetName();
        }
        bool bSameDoc = ( aThisFile == aOtherFile && !aThisFile.isEmpty() );
        if ( !bSameDoc )
        {
            //  create change actions from comparing with the name of the user
            //  who last saved the document
            //  (only if comparing different documents)

            using namespace ::com::sun::star;
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps(
                xDPS->getDocumentProperties());
            OSL_ENSURE(xDocProps.is(), "no DocumentProperties");
            OUString aDocUser = xDocProps->getModifiedBy();

            if ( !aDocUser.isEmpty() )
                pTrack->SetUser( aDocUser );
        }
    }

    m_aDocument.CompareDocument( rOtherDoc );

    pTrack = m_aDocument.GetChangeTrack();
    if ( pTrack )
        pTrack->SetUser( aOldUser );

    PostPaintGridAll();
    SetDocumentModified();
}

//              Merge (combine documents)

static bool lcl_Equal( const ScChangeAction* pA, const ScChangeAction* pB, bool bIgnore100Sec )
{
    return pA && pB &&
        pA->GetActionNumber() == pB->GetActionNumber() &&
        pA->GetType()         == pB->GetType() &&
        pA->GetUser()         == pB->GetUser() &&
        (bIgnore100Sec ?
         pA->GetDateTimeUTC().IsEqualIgnoreNanoSec( pB->GetDateTimeUTC() ) :
         pA->GetDateTimeUTC() == pB->GetDateTimeUTC());
    //  don't compare state if an old change has been accepted
}

static bool lcl_FindAction( ScDocument* pDoc, const ScChangeAction* pAction, ScDocument* pSearchDoc, const ScChangeAction* pFirstSearchAction, const ScChangeAction* pLastSearchAction, bool bIgnore100Sec )
{
    if ( !pDoc || !pAction || !pSearchDoc || !pFirstSearchAction || !pLastSearchAction )
    {
        return false;
    }

    sal_uLong nLastSearchAction = pLastSearchAction->GetActionNumber();
    const ScChangeAction* pA = pFirstSearchAction;
    while ( pA && pA->GetActionNumber() <= nLastSearchAction )
    {
        if ( pAction->GetType() == pA->GetType() &&
             pAction->GetUser() == pA->GetUser() &&
             (bIgnore100Sec ?
                pAction->GetDateTimeUTC().IsEqualIgnoreNanoSec( pA->GetDateTimeUTC() ) :
                pAction->GetDateTimeUTC() == pA->GetDateTimeUTC() ) &&
             pAction->GetBigRange() == pA->GetBigRange() )
        {
            OUString aActionDesc;
            pAction->GetDescription(aActionDesc, pDoc, true);
            OUString aADesc;
            pA->GetDescription(aADesc, pSearchDoc, true);
            if (aActionDesc == aADesc)
            {
                OSL_FAIL( "lcl_FindAction(): found equal action!" );
                return true;
            }
        }
        pA = pA->GetNext();
    }

    return false;
}

void ScDocShell::MergeDocument( ScDocument& rOtherDoc, bool bShared, bool bCheckDuplicates, sal_uLong nOffset, ScChangeActionMergeMap* pMergeMap, bool bInverseMap )
{
    ScTabViewShell* pViewSh = GetBestViewShell( false );    //! functions to the DocShell
    if (!pViewSh)
        return;

    ScChangeTrack* pSourceTrack = rOtherDoc.GetChangeTrack();
    if (!pSourceTrack)
        return;             //! nothing to do - error notification?

    ScChangeTrack* pThisTrack = m_aDocument.GetChangeTrack();
    if ( !pThisTrack )
    {   // turn on
        m_aDocument.StartChangeTracking();
        pThisTrack = m_aDocument.GetChangeTrack();
        OSL_ENSURE(pThisTrack,"ChangeTracking not enabled?");
        if ( !bShared )
        {
            // turn on visual RedLining
            ScChangeViewSettings aChangeViewSet;
            aChangeViewSet.SetShowChanges(true);
            m_aDocument.SetChangeViewSettings(aChangeViewSet);
        }
    }

    // include Nano seconds in compare?
    bool bIgnore100Sec = !pSourceTrack->IsTimeNanoSeconds() ||
            !pThisTrack->IsTimeNanoSeconds();

    //  find common initial position
    sal_uLong nFirstNewNumber = 0;
    const ScChangeAction* pSourceAction = pSourceTrack->GetFirst();
    const ScChangeAction* pThisAction = pThisTrack->GetFirst();
    // skip identical actions
    while ( lcl_Equal( pSourceAction, pThisAction, bIgnore100Sec ) )
    {
        nFirstNewNumber = pSourceAction->GetActionNumber() + 1;
        pSourceAction = pSourceAction->GetNext();
        pThisAction = pThisAction->GetNext();
    }
    //  pSourceAction and pThisAction now point to the first "own" actions
    //  The common actions before don't interest at all

    //! Inquiry if the documents where equal before the change tracking !!!

    const ScChangeAction* pFirstMergeAction = pSourceAction;
    const ScChangeAction* pFirstSearchAction = pThisAction;

    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
    const ScChangeAction* pLastSearchAction = pThisTrack->GetLast();

    //  Create MergeChangeData from the following actions
    sal_uLong nNewActionCount = 0;
    const ScChangeAction* pCount = pSourceAction;
    while ( pCount )
    {
        if ( bShared || !ScChangeTrack::MergeIgnore( *pCount, nFirstNewNumber ) )
            ++nNewActionCount;
        pCount = pCount->GetNext();
    }
    if (!nNewActionCount)
        return;             //! nothing to do - error notification?
                            //  from here on no return

    ScProgress aProgress( this, "...", nNewActionCount, true );

    sal_uLong nLastMergeAction = pSourceTrack->GetLast()->GetActionNumber();
    // UpdateReference-Undo, valid references for the last common state
    pSourceTrack->MergePrepare( pFirstMergeAction, bShared );

    //  adjust MergeChangeData to all yet following actions in this document
    //  -> references valid for this document
    while ( pThisAction )
    {
        // #i87049# [Collaboration] Conflict between delete row and insert content is not merged correctly
        if ( !bShared || !ScChangeTrack::MergeIgnore( *pThisAction, nFirstNewNumber ) )
        {
            ScChangeActionType eType = pThisAction->GetType();
            switch ( eType )
            {
                case SC_CAT_INSERT_COLS :
                case SC_CAT_INSERT_ROWS :
                case SC_CAT_INSERT_TABS :
                    pSourceTrack->AppendInsert( pThisAction->GetBigRange().MakeRange() );
                break;
                case SC_CAT_DELETE_COLS :
                case SC_CAT_DELETE_ROWS :
                case SC_CAT_DELETE_TABS :
                {
                    const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(pThisAction);
                    if ( pDel->IsTopDelete() && !pDel->IsTabDeleteCol() )
                    {   // deleted table contains deleted cols, which are not
                        sal_uLong nStart, nEnd;
                        pSourceTrack->AppendDeleteRange(
                            pDel->GetOverAllRange().MakeRange(), nullptr, nStart, nEnd );
                    }
                }
                break;
                case SC_CAT_MOVE :
                {
                    const ScChangeActionMove* pMove = static_cast<const ScChangeActionMove*>(pThisAction);
                    pSourceTrack->AppendMove( pMove->GetFromRange().MakeRange(),
                        pMove->GetBigRange().MakeRange(), nullptr );
                }
                break;
                default:
                {
                    // added to avoid warnings
                }
            }
        }
        pThisAction = pThisAction->GetNext();
    }

    LockPaint();    // #i73877# no repainting after each action

    //  take over MergeChangeData into the current document
    bool bHasRejected = false;
    OUString aOldUser = pThisTrack->GetUser();
    pThisTrack->SetUseFixDateTime( true );
    ScMarkData& rMarkData = pViewSh->GetViewData().GetMarkData();
    ScMarkData aOldMarkData( rMarkData );
    pSourceAction = pFirstMergeAction;
    while ( pSourceAction && pSourceAction->GetActionNumber() <= nLastMergeAction )
    {
        bool bMergeAction = false;
        if ( bShared )
        {
            if ( !bCheckDuplicates || !lcl_FindAction( &rOtherDoc, pSourceAction, &m_aDocument, pFirstSearchAction, pLastSearchAction, bIgnore100Sec ) )
            {
                bMergeAction = true;
            }
        }
        else
        {
            if ( !ScChangeTrack::MergeIgnore( *pSourceAction, nFirstNewNumber ) )
            {
                bMergeAction = true;
            }
        }

        if ( bMergeAction )
        {
            ScChangeActionType eSourceType = pSourceAction->GetType();
            if ( !bShared && pSourceAction->IsDeletedIn() )
            {
                //! does it need to be determined yet if really deleted in
                //! _this_ document?

                //  lies in a range, which was deleted in this document
                //  -> is omitted
                //! ??? revert deletion action ???
                //! ??? save action somewhere else  ???
#if OSL_DEBUG_LEVEL > 0
                OUString aValue;
                if ( eSourceType == SC_CAT_CONTENT )
                    static_cast<const ScChangeActionContent*>(pSourceAction)->GetNewString( aValue, &m_aDocument );
                SAL_WARN( "sc", aValue << " omitted");
#endif
            }
            else
            {
                //! Take over date/author/comment of the source action!

                pThisTrack->SetUser( pSourceAction->GetUser() );
                pThisTrack->SetFixDateTimeUTC( pSourceAction->GetDateTimeUTC() );
                sal_uLong nOldActionMax = pThisTrack->GetActionMax();

                bool bExecute = true;
                sal_uLong nReject = pSourceAction->GetRejectAction();
                if ( nReject )
                {
                    if ( bShared )
                    {
                        if ( nReject >= nFirstNewNumber )
                        {
                            nReject += nOffset;
                        }
                        ScChangeAction* pOldAction = pThisTrack->GetAction( nReject );
                        if ( pOldAction && pOldAction->IsVirgin() )
                        {
                            pThisTrack->Reject( pOldAction );
                            bHasRejected = true;
                            bExecute = false;
                        }
                    }
                    else
                    {
                        //  decline old action (of the common ones)
                        ScChangeAction* pOldAction = pThisTrack->GetAction( nReject );
                        if (pOldAction && pOldAction->GetState() == SC_CAS_VIRGIN)
                        {
                            //! what happens at actions, which were accepted in this document???
                            //! error notification or what???
                            //! or execute reject change normally

                            pThisTrack->Reject(pOldAction);
                            bHasRejected = true;                // for Paint
                        }
                        bExecute = false;
                    }
                }

                if ( bExecute )
                {
                    //  execute normally
                    ScRange aSourceRange = pSourceAction->GetBigRange().MakeRange();
                    rMarkData.SelectOneTable( aSourceRange.aStart.Tab() );
                    switch ( eSourceType )
                    {
                        case SC_CAT_CONTENT:
                        {
                            //! Test if it was at the very bottom in the document, then automatic
                            //! row insert ???

                            OSL_ENSURE( aSourceRange.aStart == aSourceRange.aEnd, "huch?" );
                            ScAddress aPos = aSourceRange.aStart;
                            OUString aValue;
                            static_cast<const ScChangeActionContent*>(pSourceAction)->GetNewString( aValue, &m_aDocument );
                            ScMatrixMode eMatrix = ScMatrixMode::NONE;
                            const ScCellValue& rCell = static_cast<const ScChangeActionContent*>(pSourceAction)->GetNewCell();
                            if (rCell.meType == CELLTYPE_FORMULA)
                                eMatrix = rCell.mpFormula->GetMatrixFlag();
                            switch ( eMatrix )
                            {
                                case ScMatrixMode::NONE :
                                    pViewSh->EnterData( aPos.Col(), aPos.Row(), aPos.Tab(), aValue );
                                break;
                                case ScMatrixMode::Formula :
                                {
                                    SCCOL nCols;
                                    SCROW nRows;
                                    rCell.mpFormula->GetMatColsRows(nCols, nRows);
                                    aSourceRange.aEnd.SetCol( aPos.Col() + nCols - 1 );
                                    aSourceRange.aEnd.SetRow( aPos.Row() + nRows - 1 );
                                    aValue = aValue.copy(1, aValue.getLength()-2); // remove the 1st and last characters.
                                    GetDocFunc().EnterMatrix( aSourceRange,
                                        nullptr, nullptr, aValue, false, false,
                                        EMPTY_OUSTRING, formula::FormulaGrammar::GRAM_DEFAULT );
                                }
                                break;
                                case ScMatrixMode::Reference :     // do nothing
                                break;
                            }
                        }
                        break;
                        case SC_CAT_INSERT_TABS :
                        {
                            OUString aName;
                            m_aDocument.CreateValidTabName( aName );
                            (void)GetDocFunc().InsertTable( aSourceRange.aStart.Tab(), aName, true, false );
                        }
                        break;
                        case SC_CAT_INSERT_ROWS:
                            (void)GetDocFunc().InsertCells( aSourceRange, nullptr, INS_INSROWS_BEFORE, true, false );
                        break;
                        case SC_CAT_INSERT_COLS:
                            (void)GetDocFunc().InsertCells( aSourceRange, nullptr, INS_INSCOLS_BEFORE, true, false );
                        break;
                        case SC_CAT_DELETE_TABS :
                            (void)GetDocFunc().DeleteTable( aSourceRange.aStart.Tab(), true );
                        break;
                        case SC_CAT_DELETE_ROWS:
                        {
                            const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(pSourceAction);
                            if ( pDel->IsTopDelete() )
                            {
                                aSourceRange = pDel->GetOverAllRange().MakeRange();
                                (void)GetDocFunc().DeleteCells( aSourceRange, nullptr, DelCellCmd::Rows, false );

                                // #i101099# [Collaboration] Changes are not correctly shown
                                if ( bShared )
                                {
                                    ScChangeAction* pAct = pThisTrack->GetLast();
                                    if ( pAct && pAct->GetType() == eSourceType && pAct->IsDeletedIn() && !pSourceAction->IsDeletedIn() )
                                    {
                                        pAct->RemoveAllDeletedIn();
                                    }
                                }
                            }
                        }
                        break;
                        case SC_CAT_DELETE_COLS:
                        {
                            const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(pSourceAction);
                            if ( pDel->IsTopDelete() && !pDel->IsTabDeleteCol() )
                            {   // deleted table contains deleted cols, which are not
                                aSourceRange = pDel->GetOverAllRange().MakeRange();
                                (void)GetDocFunc().DeleteCells( aSourceRange, nullptr, DelCellCmd::Cols, false );
                            }
                        }
                        break;
                        case SC_CAT_MOVE :
                        {
                            const ScChangeActionMove* pMove = static_cast<const ScChangeActionMove*>(pSourceAction);
                            ScRange aFromRange( pMove->GetFromRange().MakeRange() );
                            (void)GetDocFunc().MoveBlock( aFromRange,
                                aSourceRange.aStart, true, true, false, false );
                        }
                        break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
                const OUString& rComment = pSourceAction->GetComment();
                if ( !rComment.isEmpty() )
                {
                    ScChangeAction* pAct = pThisTrack->GetLast();
                    if ( pAct && pAct->GetActionNumber() > nOldActionMax )
                        pAct->SetComment( rComment );
                    else
                        OSL_FAIL( "MergeDocument: what to do with the comment?!?" );
                }

                // adjust references
                pSourceTrack->MergeOwn( const_cast<ScChangeAction*>(pSourceAction), nFirstNewNumber, bShared );

                // merge action state
                if ( bShared && !pSourceAction->IsRejected() )
                {
                    ScChangeAction* pAct = pThisTrack->GetLast();
                    if ( pAct && pAct->GetActionNumber() > nOldActionMax )
                    {
                        ScChangeTrack::MergeActionState( pAct, pSourceAction );
                    }
                }

                // fill merge map
                if ( bShared && pMergeMap )
                {
                    ScChangeAction* pAct = pThisTrack->GetLast();
                    if ( pAct && pAct->GetActionNumber() > nOldActionMax )
                    {
                        sal_uLong nActionMax = pAct->GetActionNumber();
                        sal_uLong nActionCount = nActionMax - nOldActionMax;
                        sal_uLong nAction = nActionMax - nActionCount + 1;
                        sal_uLong nSourceAction = pSourceAction->GetActionNumber() - nActionCount + 1;
                        while ( nAction <= nActionMax )
                        {
                            if ( bInverseMap )
                            {
                                (*pMergeMap)[ nAction++ ] = nSourceAction++;
                            }
                            else
                            {
                                (*pMergeMap)[ nSourceAction++ ] = nAction++;
                            }
                        }
                    }
                }
            }
            aProgress.SetStateCountDown( --nNewActionCount );
        }
        pSourceAction = pSourceAction->GetNext();
    }

    rMarkData = aOldMarkData;
    pThisTrack->SetUser(aOldUser);
    pThisTrack->SetUseFixDateTime( false );

    pSourceTrack->Clear();      //! this one is bungled now

    if (bHasRejected)
        PostPaintGridAll();         // Reject() doesn't paint itself

    UnlockPaint();
}

bool ScDocShell::MergeSharedDocument( ScDocShell* pSharedDocShell )
{
    if ( !pSharedDocShell )
    {
        return false;
    }

    ScChangeTrack* pThisTrack = m_aDocument.GetChangeTrack();
    if ( !pThisTrack )
    {
        return false;
    }

    ScDocument& rSharedDoc = pSharedDocShell->GetDocument();
    ScChangeTrack* pSharedTrack = rSharedDoc.GetChangeTrack();
    if ( !pSharedTrack )
    {
        return false;
    }

    // reset show changes
    ScChangeViewSettings aChangeViewSet;
    aChangeViewSet.SetShowChanges( false );
    m_aDocument.SetChangeViewSettings( aChangeViewSet );

    // find first merge action in this document
    bool bIgnore100Sec = !pThisTrack->IsTimeNanoSeconds() || !pSharedTrack->IsTimeNanoSeconds();
    ScChangeAction* pThisAction = pThisTrack->GetFirst();
    ScChangeAction* pSharedAction = pSharedTrack->GetFirst();
    while ( lcl_Equal( pThisAction, pSharedAction, bIgnore100Sec ) )
    {
        pThisAction = pThisAction->GetNext();
        pSharedAction = pSharedAction->GetNext();
    }

    if ( pSharedAction )
    {
        if ( pThisAction )
        {
            // merge own changes into shared document
            sal_uLong nActStartShared = pSharedAction->GetActionNumber();
            sal_uLong nActEndShared = pSharedTrack->GetActionMax();
            std::unique_ptr<ScDocument> pTmpDoc(new ScDocument);
            for ( sal_Int32 nIndex = 0; nIndex < m_aDocument.GetTableCount(); ++nIndex )
            {
                OUString sTabName;
                pTmpDoc->CreateValidTabName( sTabName );
                pTmpDoc->InsertTab( SC_TAB_APPEND, sTabName );
            }
            m_aDocument.GetChangeTrack()->Clone( pTmpDoc.get() );
            ScChangeActionMergeMap aOwnInverseMergeMap;
            pSharedDocShell->MergeDocument( *pTmpDoc, true, true, 0, &aOwnInverseMergeMap, true );
            pTmpDoc.reset();
            sal_uLong nActStartOwn = nActEndShared + 1;
            sal_uLong nActEndOwn = pSharedTrack->GetActionMax();

            // find conflicts
            ScConflictsList aConflictsList;
            ScConflictsFinder aFinder( pSharedTrack, nActStartShared, nActEndShared, nActStartOwn, nActEndOwn, aConflictsList );
            if ( aFinder.Find() )
            {
                ScConflictsListHelper::TransformConflictsList( aConflictsList, nullptr, &aOwnInverseMergeMap );
                bool bLoop = true;
                while ( bLoop )
                {
                    bLoop = false;
                    ScopedVclPtrInstance< ScConflictsDlg > aDlg( GetActiveDialogParent(), GetViewData(), &rSharedDoc, aConflictsList );
                    if ( aDlg->Execute() == RET_CANCEL )
                    {
                        vcl::Window* pWin = GetActiveDialogParent();
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       ScResId(STR_DOC_WILLNOTBESAVED)));
                        xQueryBox->set_default_response(RET_YES);
                        if (xQueryBox->run() == RET_YES)
                        {
                            return false;
                        }
                        else
                        {
                            bLoop = true;
                        }
                    }
                }
            }

            // undo own changes in shared document
            pSharedTrack->Undo( nActStartOwn, nActEndOwn );

            // clone change track for merging into own document
            pTmpDoc.reset(new ScDocument);
            for ( sal_Int32 nIndex = 0; nIndex < m_aDocument.GetTableCount(); ++nIndex )
            {
                OUString sTabName;
                pTmpDoc->CreateValidTabName( sTabName );
                pTmpDoc->InsertTab( SC_TAB_APPEND, sTabName );
            }
            pThisTrack->Clone( pTmpDoc.get() );

            // undo own changes since last save in own document
            sal_uLong nStartShared = pThisAction->GetActionNumber();
            ScChangeAction* pAction = pThisTrack->GetLast();
            while ( pAction && pAction->GetActionNumber() >= nStartShared )
            {
                pThisTrack->Reject( pAction, true );
                pAction = pAction->GetPrev();
            }

            // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
            pThisTrack->Undo( nStartShared, pThisTrack->GetActionMax(), true );

            // merge shared changes into own document
            ScChangeActionMergeMap aSharedMergeMap;
            MergeDocument( rSharedDoc, true, true, 0, &aSharedMergeMap );
            sal_uLong nEndShared = pThisTrack->GetActionMax();

            // resolve conflicts for shared non-content actions
            if ( !aConflictsList.empty() )
            {
                ScConflictsListHelper::TransformConflictsList( aConflictsList, &aSharedMergeMap, nullptr );
                ScConflictsResolver aResolver( pThisTrack, aConflictsList );
                pAction = pThisTrack->GetAction( nEndShared );
                while ( pAction && pAction->GetActionNumber() >= nStartShared )
                {
                    aResolver.HandleAction( pAction, true /*bIsSharedAction*/,
                        false /*bHandleContentAction*/, true /*bHandleNonContentAction*/ );
                    pAction = pAction->GetPrev();
                }
            }
            nEndShared = pThisTrack->GetActionMax();

            // only show changes from shared document
            aChangeViewSet.SetShowChanges( true );
            aChangeViewSet.SetShowAccepted( true );
            aChangeViewSet.SetHasActionRange();
            aChangeViewSet.SetTheActionRange( nStartShared, nEndShared );
            m_aDocument.SetChangeViewSettings( aChangeViewSet );

            // merge own changes back into own document
            sal_uLong nStartOwn = nEndShared + 1;
            ScChangeActionMergeMap aOwnMergeMap;
            MergeDocument( *pTmpDoc, true, true, nEndShared - nStartShared + 1, &aOwnMergeMap );
            pTmpDoc.reset();
            sal_uLong nEndOwn = pThisTrack->GetActionMax();

            // resolve conflicts for shared content actions and own actions
            if ( !aConflictsList.empty() )
            {
                ScConflictsListHelper::TransformConflictsList( aConflictsList, nullptr, &aOwnMergeMap );
                ScConflictsResolver aResolver( pThisTrack, aConflictsList );
                pAction = pThisTrack->GetAction( nEndShared );
                while ( pAction && pAction->GetActionNumber() >= nStartShared )
                {
                    aResolver.HandleAction( pAction, true /*bIsSharedAction*/,
                        true /*bHandleContentAction*/, false /*bHandleNonContentAction*/ );
                    pAction = pAction->GetPrev();
                }

                pAction = pThisTrack->GetAction( nEndOwn );
                while ( pAction && pAction->GetActionNumber() >= nStartOwn )
                {
                    aResolver.HandleAction( pAction, false /*bIsSharedAction*/,
                        true /*bHandleContentAction*/, true /*bHandleNonContentAction*/ );
                    pAction = pAction->GetPrev();
                }
            }
        }
        else
        {
            // merge shared changes into own document
            sal_uLong nStartShared = pThisTrack->GetActionMax() + 1;
            MergeDocument( rSharedDoc, true, true );
            sal_uLong nEndShared = pThisTrack->GetActionMax();

            // only show changes from shared document
            aChangeViewSet.SetShowChanges( true );
            aChangeViewSet.SetShowAccepted( true );
            aChangeViewSet.SetHasActionRange();
            aChangeViewSet.SetTheActionRange( nStartShared, nEndShared );
            m_aDocument.SetChangeViewSettings( aChangeViewSet );
        }

        // update view
        PostPaintExtras();
        PostPaintGridAll();

        vcl::Window* pWin = GetActiveDialogParent();
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(STR_DOC_UPDATED)));
        xInfoBox->run();
    }

    return ( pThisAction != nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
