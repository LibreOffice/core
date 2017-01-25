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

#include "scitems.hxx"
#include "rangelst.hxx"
#include <editeng/flstitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svx/postattr.hxx>
#include <unotools/misccfg.hxx>
#include <vcl/virdev.hxx>
#include <vcl/msgbox.hxx>

#include "docsh.hxx"
#include "docshimp.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "patattr.hxx"
#include "uiitems.hxx"
#include "hints.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "pntlock.hxx"
#include "chgtrack.hxx"
#include "docfunc.hxx"
#include "formulacell.hxx"
#include "chgviset.hxx"
#include "progress.hxx"
#include "redcom.hxx"
#include "sc.hrc"
#include "inputopt.hxx"
#include "drwlayer.hxx"
#include "inputhdl.hxx"
#include "conflictsdlg.hxx"
#include "globstr.hrc"
#include "markdata.hxx"
#include <memory>

#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

//          Redraw - Benachrichtigungen

void ScDocShell::PostEditView( ScEditEngineDefaulter* pEditEngine, const ScAddress& rCursorPos )
{
//  Broadcast( ScEditViewHint( pEditEngine, rCursorPos ) );

        //  Test: nur aktive ViewShell

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
    aDocument.PrepareFormulaCalc();
    //! Navigator direkt benachrichtigen!
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
        const ScRange& rRange = *rRanges[i];
        SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
        SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
        SCTAB nTab1 = rRange.aStart.Tab(), nTab2 = rRange.aEnd.Tab();

        if (!ValidCol(nCol1)) nCol1 = MAXCOL;
        if (!ValidRow(nRow1)) nRow1 = MAXROW;
        if (!ValidCol(nCol2)) nCol2 = MAXCOL;
        if (!ValidRow(nRow2)) nRow2 = MAXROW;

        if ( pPaintLockData )
        {
            // #i54081# PaintPartFlags::Extras still has to be broadcast because it changes the
            // current sheet if it's invalid. All other flags added to pPaintLockData.
            PaintPartFlags nLockPart = nPart & ~PaintPartFlags::Extras;
            if ( nLockPart != PaintPartFlags::NONE )
            {
                //! nExtFlags ???
                pPaintLockData->AddRange( ScRange( nCol1, nRow1, nTab1,
                                                   nCol2, nRow2, nTab2 ), nLockPart );
            }

            nPart &= PaintPartFlags::Extras;  // for broadcasting
            if (nPart == PaintPartFlags::NONE)
                continue;
        }

        if (nExtFlags & SC_PF_LINES)            // Platz fuer Linien beruecksichtigen
        {
                                                //! Abfrage auf versteckte Spalten/Zeilen!
            if (nCol1>0) --nCol1;
            if (nCol2<MAXCOL) ++nCol2;
            if (nRow1>0) --nRow1;
            if (nRow2<MAXROW) ++nRow2;
        }

                                                // um zusammengefasste erweitern
        if (nExtFlags & SC_PF_TESTMERGE)
            aDocument.ExtendMerge( nCol1, nRow1, nCol2, nRow2, nTab1 );

        if ( nCol1 != 0 || nCol2 != MAXCOL )
        {
            //  Extend to whole rows if SC_PF_WHOLEROWS is set, or rotated or non-left
            //  aligned cells are contained (see UpdatePaintExt).
            //  Special handling for RTL text (#i9731#) is unnecessary now with full
            //  support of right-aligned text.

            if ( ( nExtFlags & SC_PF_WHOLEROWS ) ||
                 aDocument.HasAttrib( nCol1,nRow1,nTab1,
                                      MAXCOL,nRow2,nTab2, HasAttrFlags::Rotate | HasAttrFlags::RightOrCenter ) )
            {
                nCol1 = 0;
                nCol2 = MAXCOL;
            }
        }
        aPaintRanges.Append(ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2));
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
         aDocument.HasAttrib( rRange, HasAttrFlags::Lines | HasAttrFlags::Shadow | HasAttrFlags::Conditional ) )
    {
        //  If the range contains lines, shadow or conditional formats,
        //  set SC_PF_LINES to include one extra cell in all directions.

        rExtFlags |= SC_PF_LINES;
    }

    if ( ( rExtFlags & SC_PF_WHOLEROWS ) == 0 &&
         ( rRange.aStart.Col() != 0 || rRange.aEnd.Col() != MAXCOL ) &&
         aDocument.HasAttrib( rRange, HasAttrFlags::Rotate | HasAttrFlags::RightOrCenter ) )
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
    if ( !pPaintLockData )
        pPaintLockData = new ScPaintLockData;
    pPaintLockData->IncLevel(bDoc);
}

void ScDocShell::UnlockPaint_Impl(bool bDoc)
{
    if ( pPaintLockData )
    {
        if ( pPaintLockData->GetLevel(bDoc) )
            pPaintLockData->DecLevel(bDoc);
        if (!pPaintLockData->GetLevel(!bDoc) && !pPaintLockData->GetLevel(bDoc))
        {
            //      Paint jetzt ausfuehren

            ScPaintLockData* pPaint = pPaintLockData;
            pPaintLockData = nullptr;                      // nicht weitersammeln

            ScRangeListRef xRangeList = pPaint->GetRangeList();
            if ( xRangeList.is() )
            {
                PaintPartFlags nParts = pPaint->GetParts();
                for ( size_t i = 0, nCount = xRangeList->size(); i < nCount; i++ )
                {
                    //! nExtFlags ???
                    ScRange aRange = *(*xRangeList)[i];
                    PostPaint( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab(),
                                nParts );
                }
            }

            if ( pPaint->GetModified() )
                SetDocumentModified();

            delete pPaint;
        }
    }
    else
    {
        OSL_FAIL("UnlockPaint ohne LockPaint");
    }
}

void ScDocShell::LockDocument_Impl(sal_uInt16 nNew)
{
    if (!nDocumentLock)
    {
        ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->setLock(true);
    }
    nDocumentLock = nNew;
}

void ScDocShell::UnlockDocument_Impl(sal_uInt16 nNew)
{
    nDocumentLock = nNew;
    if (!nDocumentLock)
    {
        ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->setLock(false);
    }
}

void ScDocShell::SetLockCount(sal_uInt16 nNew)
{
    if (nNew)                   // setzen
    {
        if ( !pPaintLockData )
            pPaintLockData = new ScPaintLockData;
        pPaintLockData->SetDocLevel(nNew-1);
        LockDocument_Impl(nNew);
    }
    else if (pPaintLockData)    // loeschen
    {
        pPaintLockData->SetDocLevel(0);  // bei Unlock sofort ausfuehren
        UnlockPaint_Impl(true);                 // jetzt
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
    LockDocument_Impl(nDocumentLock + 1);
}

void ScDocShell::UnlockDocument()
{
    if (nDocumentLock)
    {
        UnlockPaint_Impl(true);
        UnlockDocument_Impl(nDocumentLock - 1);
    }
    else
    {
        OSL_FAIL("UnlockDocument without LockDocument");
    }
}

void ScDocShell::SetInplace( bool bInplace )
{
    if (bIsInplace != bInplace)
    {
        bIsInplace = bInplace;
        CalcOutputFactor();
    }
}

void ScDocShell::CalcOutputFactor()
{
    if (bIsInplace)
    {
        nPrtToScreenFactor = 1.0;           // passt sonst nicht zur inaktiven Darstellung
        return;
    }

    bool bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    if (bTextWysiwyg)
    {
        nPrtToScreenFactor = 1.0;
        return;
    }

    OUString aTestString(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890123456789");
    long nPrinterWidth = 0;
    long nWindowWidth = 0;
    const ScPatternAttr* pPattern = static_cast<const ScPatternAttr*>(&aDocument.GetPool()->
                                            GetDefaultItem(ATTR_PATTERN));

    vcl::Font aDefFont;
    OutputDevice* pRefDev = GetRefDevice();
    MapMode aOldMode = pRefDev->GetMapMode();
    vcl::Font aOldFont = pRefDev->GetFont();

    pRefDev->SetMapMode(MapUnit::MapPixel);
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pRefDev); // font color doesn't matter here
    pRefDev->SetFont(aDefFont);
    nPrinterWidth = pRefDev->PixelToLogic( Size( pRefDev->GetTextWidth(aTestString), 0 ), MapUnit::Map100thMM ).Width();
    pRefDev->SetFont(aOldFont);
    pRefDev->SetMapMode(aOldMode);

    ScopedVclPtrInstance< VirtualDevice > pVirtWindow( *Application::GetDefaultDevice() );
    pVirtWindow->SetMapMode(MapUnit::MapPixel);
    pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pVirtWindow);    // font color doesn't matter here
    pVirtWindow->SetFont(aDefFont);
    nWindowWidth = pVirtWindow->GetTextWidth(aTestString);
    nWindowWidth = (long) ( nWindowWidth / ScGlobal::nScreenPPTX * HMM_PER_TWIPS );

    if (nPrinterWidth && nWindowWidth)
        nPrtToScreenFactor = nPrinterWidth / (double) nWindowWidth;
    else
    {
        OSL_FAIL("GetTextSize gibt 0 ??");
        nPrtToScreenFactor = 1.0;
    }
}

void ScDocShell::InitOptions(bool bForLoading)      // called from InitNew and Load
{
    //  Einstellungen aus dem SpellCheckCfg kommen in Doc- und ViewOptions

    sal_uInt16 nDefLang, nCjkLang, nCtlLang;
    bool bAutoSpell;
    ScModule::GetSpellSettings( nDefLang, nCjkLang, nCtlLang, bAutoSpell );
    ScModule* pScMod = SC_MOD();

    ScDocOptions  aDocOpt  = pScMod->GetDocOptions();
    ScFormulaOptions aFormulaOpt = pScMod->GetFormulaOptions();
    ScViewOptions aViewOpt = pScMod->GetViewOptions();
    aDocOpt.SetAutoSpell( bAutoSpell );

    // zweistellige Jahreszahleneingabe aus Extras->Optionen->Allgemein->Sonstiges
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

    aDocument.SetDocOptions( aDocOpt );
    aDocument.SetViewOptions( aViewOpt );
    SetFormulaOptions( aFormulaOpt, bForLoading );

    //  Druck-Optionen werden jetzt direkt vor dem Drucken gesetzt

    aDocument.SetLanguage( (LanguageType) nDefLang, (LanguageType) nCjkLang, (LanguageType) nCtlLang );
}

Printer* ScDocShell::GetDocumentPrinter()       // fuer OLE
{
    return aDocument.GetPrinter();
}

SfxPrinter* ScDocShell::GetPrinter(bool bCreateIfNotExist)
{
    return aDocument.GetPrinter(bCreateIfNotExist);
}

void ScDocShell::UpdateFontList()
{
    delete pImpl->pFontList;
    // pImpl->pFontList = new FontList( GetPrinter(), Application::GetDefaultDevice() );
    pImpl->pFontList = new FontList(GetRefDevice(), nullptr);
    SvxFontListItem aFontListItem( pImpl->pFontList, SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );

    CalcOutputFactor();
}

OutputDevice* ScDocShell::GetRefDevice()
{
    return aDocument.GetRefDevice();
}

sal_uInt16 ScDocShell::SetPrinter( VclPtr<SfxPrinter> const & pNewPrinter, SfxPrinterChangeFlags nDiffFlags )
{
    SfxPrinter *pOld = aDocument.GetPrinter( false );
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    if (nDiffFlags & SfxPrinterChangeFlags::PRINTER)
    {
        if ( aDocument.GetPrinter() != pNewPrinter )
        {
            aDocument.SetPrinter( pNewPrinter );
            aDocument.SetPrintOptions();

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
        SfxPrinter* pOldPrinter = aDocument.GetPrinter();
        if (pOldPrinter)
        {
            pOldPrinter->SetJobSetup( pNewPrinter->GetJobSetup() );

            //  #i6706# Call SetPrinter with the old printer again, so the drawing layer
            //  RefDevice is set (calling ReformatAllTextObjects and rebuilding charts),
            //  because the JobSetup (printer device settings) may affect text layout.
            aDocument.SetPrinter( pOldPrinter );
            CalcOutputFactor();                         // also with the new settings
        }
    }

    if (nDiffFlags & SfxPrinterChangeFlags::OPTIONS)
    {
        aDocument.SetPrintOptions();        //! aus neuem Printer ???
    }

    if (nDiffFlags & (SfxPrinterChangeFlags::CHG_ORIENTATION | SfxPrinterChangeFlags::CHG_SIZE))
    {
        OUString aStyle = aDocument.GetPageStyle( GetCurTab() );
        ScStyleSheetPool* pStPl = aDocument.GetStyleSheetPool();
        SfxStyleSheet* pStyleSheet = static_cast<SfxStyleSheet*>(pStPl->Find(aStyle, SfxStyleFamily::Page));
        if (pStyleSheet)
        {
            SfxItemSet& rSet = pStyleSheet->GetItemSet();

            if (nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION)
            {
                const SvxPageItem& rOldItem = static_cast<const SvxPageItem&>(rSet.Get(ATTR_PAGE));
                bool bWasLand = rOldItem.IsLandscape();
                bool bNewLand = ( pNewPrinter->GetOrientation() == Orientation::Landscape );
                if (bNewLand != bWasLand)
                {
                    SvxPageItem aNewItem( rOldItem );
                    aNewItem.SetLandscape( bNewLand );
                    rSet.Put( aNewItem );

                    //  Groesse umdrehen
                    Size aOldSize = static_cast<const SvxSizeItem&>(rSet.Get(ATTR_PAGE_SIZE)).GetSize();
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
                    pFound = pAction;       // der letzte gewinnt
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
            pTrack->NotifyModified( SC_CTM_CHANGE, nNumber, nNumber );
        }
    }
}

void ScDocShell::ExecuteChangeCommentDialog( ScChangeAction* pAction, vcl::Window* pParent, bool bPrevNext)
{
    if (!pAction) return;           // ohne Aktion ist nichts..

    OUString aComment = pAction->GetComment();
    OUString aAuthor = pAction->GetUser();

    DateTime aDT = pAction->GetDateTime();
    OUString aDate = ScGlobal::pLocaleData->getDate( aDT );
    aDate += " ";
    aDate += ScGlobal::pLocaleData->getTime( aDT, false );

    SfxItemSet aSet( GetPool(),
                      SID_ATTR_POSTIT_AUTHOR, SID_ATTR_POSTIT_AUTHOR,
                      SID_ATTR_POSTIT_DATE,   SID_ATTR_POSTIT_DATE,
                      SID_ATTR_POSTIT_TEXT,   SID_ATTR_POSTIT_TEXT,
                      0 );

    aSet.Put( SvxPostItTextItem  ( aComment, SID_ATTR_POSTIT_TEXT ) );
    aSet.Put( SvxPostItAuthorItem( aAuthor,  SID_ATTR_POSTIT_AUTHOR ) );
    aSet.Put( SvxPostItDateItem  ( aDate,    SID_ATTR_POSTIT_DATE ) );

    std::unique_ptr<ScRedComDialog> pDlg(new ScRedComDialog( pParent, aSet,this,pAction,bPrevNext));

    pDlg->Execute();
}

void ScDocShell::CompareDocument( ScDocument& rOtherDoc )
{
    ScChangeTrack* pTrack = aDocument.GetChangeTrack();
    if ( pTrack && pTrack->GetFirst() )
    {
        //! Changes vorhanden -> Nachfrage ob geloescht werden soll
    }

    aDocument.EndChangeTracking();
    aDocument.StartChangeTracking();

    OUString aOldUser;
    pTrack = aDocument.GetChangeTrack();
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

    aDocument.CompareDocument( rOtherDoc );

    pTrack = aDocument.GetChangeTrack();
    if ( pTrack )
        pTrack->SetUser( aOldUser );

    PostPaintGridAll();
    SetDocumentModified();
}

//              Merge (Aenderungen zusammenfuehren)

static inline bool lcl_Equal( const ScChangeAction* pA, const ScChangeAction* pB, bool bIgnore100Sec )
{
    return pA && pB &&
        pA->GetActionNumber() == pB->GetActionNumber() &&
        pA->GetType()         == pB->GetType() &&
        pA->GetUser()         == pB->GetUser() &&
        (bIgnore100Sec ?
         pA->GetDateTimeUTC().IsEqualIgnoreNanoSec( pB->GetDateTimeUTC() ) :
         pA->GetDateTimeUTC() == pB->GetDateTimeUTC());
    //  State nicht vergleichen, falls eine alte Aenderung akzeptiert wurde
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
            if (aActionDesc.equals(aADesc))
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
    ScTabViewShell* pViewSh = GetBestViewShell( false );    //! Funktionen an die DocShell
    if (!pViewSh)
        return;

    ScChangeTrack* pSourceTrack = rOtherDoc.GetChangeTrack();
    if (!pSourceTrack)
        return;             //! nichts zu tun - Fehlermeldung?

    ScChangeTrack* pThisTrack = aDocument.GetChangeTrack();
    if ( !pThisTrack )
    {   // anschalten
        aDocument.StartChangeTracking();
        pThisTrack = aDocument.GetChangeTrack();
        OSL_ENSURE(pThisTrack,"ChangeTracking nicht angeschaltet?");
        if ( !bShared )
        {
            // visuelles RedLining einschalten
            ScChangeViewSettings aChangeViewSet;
            aChangeViewSet.SetShowChanges(true);
            aDocument.SetChangeViewSettings(aChangeViewSet);
        }
    }

    // include Nano seconds in compare?
    bool bIgnore100Sec = !pSourceTrack->IsTimeNanoSeconds() ||
            !pThisTrack->IsTimeNanoSeconds();

    //  gemeinsame Ausgangsposition suchen
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
    //  pSourceAction und pThisAction zeigen jetzt auf die ersten "eigenen" Aktionen
    //  Die gemeinsamen Aktionen davor interessieren ueberhaupt nicht

    //! Abfrage, ob die Dokumente vor dem Change-Tracking gleich waren !!!

    const ScChangeAction* pFirstMergeAction = pSourceAction;
    const ScChangeAction* pFirstSearchAction = pThisAction;

    // #i94841# [Collaboration] When deleting rows is rejected, the content is sometimes wrong
    const ScChangeAction* pLastSearchAction = pThisTrack->GetLast();

    //  MergeChangeData aus den folgenden Aktionen erzeugen
    sal_uLong nNewActionCount = 0;
    const ScChangeAction* pCount = pSourceAction;
    while ( pCount )
    {
        if ( bShared || !ScChangeTrack::MergeIgnore( *pCount, nFirstNewNumber ) )
            ++nNewActionCount;
        pCount = pCount->GetNext();
    }
    if (!nNewActionCount)
        return;             //! nichts zu tun - Fehlermeldung?
                            //  ab hier kein return mehr

    ScProgress aProgress( this, "...", nNewActionCount, true );

    sal_uLong nLastMergeAction = pSourceTrack->GetLast()->GetActionNumber();
    // UpdateReference-Undo, gueltige Referenzen fuer den letzten gemeinsamen Zustand
    pSourceTrack->MergePrepare( const_cast<ScChangeAction*>(pFirstMergeAction), bShared );

    //  MergeChangeData an alle noch folgenden Aktionen in diesem Dokument anpassen
    //  -> Referenzen gueltig fuer dieses Dokument
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
                    {   // deleted Table enthaelt deleted Cols, die nicht
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

    //  MergeChangeData in das aktuelle Dokument uebernehmen
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
            if ( !bCheckDuplicates || !lcl_FindAction( &rOtherDoc, pSourceAction, &aDocument, pFirstSearchAction, pLastSearchAction, bIgnore100Sec ) )
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
                //! muss hier noch festgestellt werden, ob wirklich in
                //! _diesem_ Dokument geloescht?

                //  liegt in einem Bereich, der in diesem Dokument geloescht wurde
                //  -> wird weggelassen
                //! ??? Loesch-Aktion rueckgaengig machen ???
                //! ??? Aktion irgendwo anders speichern  ???
#if OSL_DEBUG_LEVEL > 0
                OUString aValue;
                if ( eSourceType == SC_CAT_CONTENT )
                    static_cast<const ScChangeActionContent*>(pSourceAction)->GetNewString( aValue, &aDocument );
                OStringBuffer aError(OUStringToOString(aValue,
                    osl_getThreadTextEncoding()));
                aError.append(" weggelassen");
                OSL_FAIL( aError.getStr() );
#endif
            }
            else
            {
                //! Datum/Autor/Kommentar der Source-Aktion uebernehmen!

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
                        //  alte Aktion (aus den gemeinsamen) ablehnen
                        ScChangeAction* pOldAction = pThisTrack->GetAction( nReject );
                        if (pOldAction && pOldAction->GetState() == SC_CAS_VIRGIN)
                        {
                            //! was passiert bei Aktionen, die in diesem Dokument accepted worden sind???
                            //! Fehlermeldung oder was???
                            //! oder Reject-Aenderung normal ausfuehren

                            pThisTrack->Reject(pOldAction);
                            bHasRejected = true;                // fuer Paint
                        }
                        bExecute = false;
                    }
                }

                if ( bExecute )
                {
                    //  normal ausfuehren
                    ScRange aSourceRange = pSourceAction->GetBigRange().MakeRange();
                    rMarkData.SelectOneTable( aSourceRange.aStart.Tab() );
                    switch ( eSourceType )
                    {
                        case SC_CAT_CONTENT:
                        {
                            //! Test, ob es ganz unten im Dokument war, dann automatisches
                            //! Zeilen-Einfuegen ???

                            OSL_ENSURE( aSourceRange.aStart == aSourceRange.aEnd, "huch?" );
                            ScAddress aPos = aSourceRange.aStart;
                            OUString aValue;
                            static_cast<const ScChangeActionContent*>(pSourceAction)->GetNewString( aValue, &aDocument );
                            sal_uInt8 eMatrix = MM_NONE;
                            const ScCellValue& rCell = static_cast<const ScChangeActionContent*>(pSourceAction)->GetNewCell();
                            if (rCell.meType == CELLTYPE_FORMULA)
                                eMatrix = rCell.mpFormula->GetMatrixFlag();
                            switch ( eMatrix )
                            {
                                case MM_NONE :
                                    pViewSh->EnterData( aPos.Col(), aPos.Row(), aPos.Tab(), aValue );
                                break;
                                case MM_FORMULA :
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
                                case MM_REFERENCE :     // do nothing
                                break;
                            }
                        }
                        break;
                        case SC_CAT_INSERT_TABS :
                        {
                            OUString aName;
                            aDocument.CreateValidTabName( aName );
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
                            (void)GetDocFunc().DeleteTable( aSourceRange.aStart.Tab(), true, false );
                        break;
                        case SC_CAT_DELETE_ROWS:
                        {
                            const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(pSourceAction);
                            if ( pDel->IsTopDelete() )
                            {
                                aSourceRange = pDel->GetOverAllRange().MakeRange();
                                (void)GetDocFunc().DeleteCells( aSourceRange, nullptr, DEL_DELROWS, false );

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
                            {   // deleted Table enthaelt deleted Cols, die nicht
                                aSourceRange = pDel->GetOverAllRange().MakeRange();
                                (void)GetDocFunc().DeleteCells( aSourceRange, nullptr, DEL_DELCOLS, false );
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
                        OSL_FAIL( "MergeDocument: wohin mit dem Kommentar?!?" );
                }

                // Referenzen anpassen
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

    pSourceTrack->Clear();      //! der ist jetzt verhunzt

    if (bHasRejected)
        PostPaintGridAll();         // Reject() paintet nicht selber

    UnlockPaint();
}

bool ScDocShell::MergeSharedDocument( ScDocShell* pSharedDocShell )
{
    if ( !pSharedDocShell )
    {
        return false;
    }

    ScChangeTrack* pThisTrack = aDocument.GetChangeTrack();
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
    aDocument.SetChangeViewSettings( aChangeViewSet );

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
            ScDocument* pTmpDoc = new ScDocument;
            for ( sal_Int32 nIndex = 0; nIndex < aDocument.GetTableCount(); ++nIndex )
            {
                OUString sTabName;
                pTmpDoc->CreateValidTabName( sTabName );
                pTmpDoc->InsertTab( SC_TAB_APPEND, sTabName );
            }
            aDocument.GetChangeTrack()->Clone( pTmpDoc );
            ScChangeActionMergeMap aOwnInverseMergeMap;
            pSharedDocShell->MergeDocument( *pTmpDoc, true, true, 0, &aOwnInverseMergeMap, true );
            delete pTmpDoc;
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
                        ScopedVclPtrInstance<QueryBox> aBox( GetActiveDialogParent(), WinBits( WB_YES_NO | WB_DEF_YES ),
                            ScGlobal::GetRscString( STR_DOC_WILLNOTBESAVED ) );
                        if ( aBox->Execute() == RET_YES )
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
            pTmpDoc = new ScDocument;
            for ( sal_Int32 nIndex = 0; nIndex < aDocument.GetTableCount(); ++nIndex )
            {
                OUString sTabName;
                pTmpDoc->CreateValidTabName( sTabName );
                pTmpDoc->InsertTab( SC_TAB_APPEND, sTabName );
            }
            pThisTrack->Clone( pTmpDoc );

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
            aDocument.SetChangeViewSettings( aChangeViewSet );

            // merge own changes back into own document
            sal_uLong nStartOwn = nEndShared + 1;
            ScChangeActionMergeMap aOwnMergeMap;
            MergeDocument( *pTmpDoc, true, true, nEndShared - nStartShared + 1, &aOwnMergeMap );
            delete pTmpDoc;
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
            aDocument.SetChangeViewSettings( aChangeViewSet );
        }

        // update view
        PostPaintExtras();
        PostPaintGridAll();

        ScopedVclPtrInstance< InfoBox > aInfoBox( GetActiveDialogParent(), ScGlobal::GetRscString( STR_DOC_UPDATED ) );
        aInfoBox->Execute();
    }

    return ( pThisAction != nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
