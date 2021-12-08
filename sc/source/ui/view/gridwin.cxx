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

#include <cstdlib>
#include <memory>
#include <editeng/adjustitem.hxx>
#include <sal/log.hxx>
#include <sot/storage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/misspellrange.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/ipclient.hxx>
#include <svl/stritem.hxx>
#include <svl/sharedstringpool.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/cursor.hxx>
#include <vcl/dialoghelper.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weldutils.hxx>
#include <sot/formats.hxx>
#include <comphelper/classids.hxx>

#include <svx/drawitem.hxx>
#include <svx/svdview.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/optionsdrawinglayer.hxx>

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

#include <gridwin.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>
#include <tabview.hxx>
#include <select.hxx>
#include <scmod.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <dbdata.hxx>
#include <stlpool.hxx>
#include <printfun.hxx>
#include <cbutton.hxx>
#include <sc.hrc>
#include <helpids.h>
#include <globstr.hrc>
#include <strings.hrc>
#include <editutil.hxx>
#include <scresid.hxx>
#include <inputhdl.hxx>
#include <uiitems.hxx>
#include <formulacell.hxx>
#include <patattr.hxx>
#include <notemark.hxx>
#include <rfindlst.hxx>
#include <output.hxx>
#include <docfunc.hxx>
#include <dbdocfun.hxx>
#include <dpobject.hxx>
#include <transobj.hxx>
#include <drwtrans.hxx>
#include <seltrans.hxx>
#include <sizedev.hxx>
#include <AccessibilityHints.hxx>
#include <dpsave.hxx>
#include <viewuno.hxx>
#include <compiler.hxx>
#include <editable.hxx>
#include <fillinfo.hxx>
#include <filterentries.hxx>
#include <drwlayer.hxx>
#include <validat.hxx>
#include <tabprotection.hxx>
#include <postit.hxx>
#include <dpcontrol.hxx>
#include <checklistmenu.hxx>
#include <clipparam.hxx>
#include <overlayobject.hxx>
#include <cellsuno.hxx>
#include <drawview.hxx>
#include <dragdata.hxx>
#include <cliputil.hxx>
#include <queryentry.hxx>
#include <markdata.hxx>
#include <externalrefmgr.hxx>
#include <spellcheckcontext.hxx>
#include <uiobject.hxx>
#include <undoblk.hxx>
#include <datamapper.hxx>
#include <inputopt.hxx>
#include <queryparam.hxx>

#include <officecfg/Office/Common.hxx>

#include <svx/PaletteManager.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <vcl/svapp.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <vector>
#include <boost/property_tree/json_parser.hpp>

#include <FilterListBox.hxx>

using namespace css;
using namespace css::uno;

struct ScGridWindow::MouseEventState
{
    bool mbActivatePart;

    MouseEventState() :
        mbActivatePart(false)
    {}
};

#define SC_FILTERLISTBOX_LINES  12

ScGridWindow::VisibleRange::VisibleRange(const ScDocument& rDoc)
    : mnCol1(0)
    , mnCol2(rDoc.MaxCol())
    , mnRow1(0)
    , mnRow2(rDoc.MaxRow())
{
}

bool ScGridWindow::VisibleRange::isInside(SCCOL nCol, SCROW nRow) const
{
    return mnCol1 <= nCol && nCol <= mnCol2 && mnRow1 <= nRow && nRow <= mnRow2;
}

bool ScGridWindow::VisibleRange::set(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    bool bChanged = mnCol1 != nCol1 || mnRow1 != nRow1 || mnCol2 != nCol2 || mnRow2 != nRow2;

    mnCol1 = nCol1;
    mnRow1 = nRow1;
    mnCol2 = nCol2;
    mnRow2 = nRow2;

    return bChanged;
}

//  ListBox in a FloatingWindow (pParent)
ScFilterListBox::ScFilterListBox(weld::Window* pParent, ScGridWindow* pGrid,
                                 SCCOL nNewCol, SCROW nNewRow, ScFilterBoxMode eNewMode)
    : xBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/filterlist.ui"))
    , xPopover(xBuilder->weld_popover("FilterList"))
    , xTreeView(xBuilder->weld_tree_view("list"))
    , pGridWin(pGrid)
    , nCol(nNewCol)
    , nRow(nNewRow)
    , bInit(true)
    , bCancelled(false)
    , bGridHadMouseCaptured(pGrid->IsMouseCaptured())
    , nSel(0)
    , eMode(eNewMode)
    , nAsyncSelectHdl(nullptr)
{
    xTreeView->connect_row_activated(LINK(this, ScFilterListBox, SelectHdl));
    xTreeView->connect_key_press(LINK(this, ScFilterListBox, KeyInputHdl));
}

ScFilterListBox::~ScFilterListBox()
{
    if (nAsyncSelectHdl)
    {
        Application::RemoveUserEvent(nAsyncSelectHdl);
        nAsyncSelectHdl = nullptr;
    }
}

void ScFilterListBox::EndInit()
{
    sal_Int32 nPos = xTreeView->get_selected_index();
    if (nPos == -1)
        nSel = 0;
    else
        nSel = nPos;

    bInit = false;
}

IMPL_LINK(ScFilterListBox, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    bool bDone = false;

    vcl::KeyCode aCode = rKeyEvent.GetKeyCode();
    // esc with no modifiers
    if (!aCode.GetModifier() && aCode.GetCode() == KEY_ESCAPE)
    {
        pGridWin->ClickExtern();  // clears the listbox
        bDone = true;
    }

    // nowhere to tab to
    if (aCode.GetCode() == KEY_TAB)
        bDone = true;

    return bDone;
}

IMPL_LINK_NOARG(ScFilterListBox, SelectHdl, weld::TreeView&, bool)
{
    if (!bInit && !bCancelled && !nAsyncSelectHdl)
    {
        int nPos = xTreeView->get_selected_index();
        if (nPos != -1)
        {
            nSel = nPos;
            // #i81298# launch async so the box isn't deleted from modifications within FilterSelect
            nAsyncSelectHdl = Application::PostUserEvent(LINK(this, ScFilterListBox, AsyncSelectHdl));
        }
    }
    return true;
}

IMPL_LINK_NOARG(ScFilterListBox, AsyncSelectHdl, void*, void)
{
    nAsyncSelectHdl = nullptr;

    //tdf#133971 hold self-ref until we return
    auto xThis(shared_from_this());
    pGridWin->FilterSelect(nSel);
    if (xThis.use_count() == 1)
    {
        // tdf#133855 we got disposed by FilterSelect
        return;
    }
    pGridWin->ClickExtern();
}

static bool lcl_IsEditableMatrix( ScDocument& rDoc, const ScRange& rRange )
{
    // If it is an editable range and if there is a Matrix cell at the bottom right with an
    // origin top left then the range will be set to contain the exact matrix.
    //! Extract the MatrixEdges functions directly from the column ???
    if ( !rDoc.IsBlockEditable( rRange.aStart.Tab(), rRange.aStart.Col(),rRange.aStart.Row(),
                                    rRange.aEnd.Col(),rRange.aEnd.Row() ) )
        return false;

    ScRefCellValue aCell(rDoc, rRange.aEnd);
    ScAddress aPos;
    return (aCell.meType == CELLTYPE_FORMULA && aCell.mpFormula->GetMatrixOrigin(rDoc, aPos) && aPos == rRange.aStart);
}

static void lcl_UnLockComment( ScDrawView* pView, const Point& rPos, const ScViewData& rViewData )
{
    if (!pView)
        return;

    ScDocument& rDoc = rViewData.GetDocument();
    ScAddress aCellPos( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
    ScPostIt* pNote = rDoc.GetNote( aCellPos );
    SdrObject* pObj = pNote ? pNote->GetCaption() : nullptr;
    if( pObj && pObj->GetLogicRect().Contains( rPos ) && ScDrawLayer::IsNoteCaption( pObj ) )
    {
        const ScProtectionAttr* pProtAttr = rDoc.GetAttr( aCellPos, ATTR_PROTECTION );
        bool bProtectAttr = pProtAttr->GetProtection() || pProtAttr->GetHideCell() ;
        bool bProtectDoc =  rDoc.IsTabProtected( aCellPos.Tab() ) || rViewData.GetSfxDocShell()->IsReadOnly() ;
        // unlock internal layer (if not protected), will be relocked in ScDrawView::MarkListHasChanged()
        pView->LockInternalLayer( bProtectDoc && bProtectAttr );
    }
}

static bool lcl_GetHyperlinkCell(
    ScDocument& rDoc, SCCOL& rPosX, SCROW nPosY, SCTAB nTab, ScRefCellValue& rCell, OUString& rURL )
{
    bool bFound = false;
    do
    {
        ScAddress aPos(rPosX, nPosY, nTab);
        rCell.assign(rDoc, aPos);
        if (rCell.isEmpty())
        {
            if ( rPosX <= 0 )
                return false;                           // everything empty to the links
            else
                --rPosX;                                // continue search
        }
        else
        {
            const ScPatternAttr* pPattern = rDoc.GetPattern(aPos);
            if ( !pPattern->GetItem(ATTR_HYPERLINK).GetValue().isEmpty() )
            {
                rURL = pPattern->GetItem(ATTR_HYPERLINK).GetValue();
                bFound = true;
            }
            else if (rCell.meType == CELLTYPE_EDIT)
                bFound = true;
            else if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->IsHyperLinkCell())
                bFound = true;
            else
                return false;                               // other cell
        }
    }
    while ( !bFound );

    return bFound;
}

//  WB_DIALOGCONTROL needed for UNO-Controls
ScGridWindow::ScGridWindow( vcl::Window* pParent, ScViewData& rData, ScSplitPos eWhichPos )
:           Window( pParent, WB_CLIPCHILDREN | WB_DIALOGCONTROL ),
            DropTargetHelper( this ),
            DragSourceHelper( this ),
            maVisibleRange(rData.GetDocument()),
            mrViewData( rData ),
            eWhich( eWhichPos ),
            nCursorHideCount( 0 ),
            nButtonDown( 0 ),
            nMouseStatus( SC_GM_NONE ),
            nNestedButtonState( ScNestedButtonState::NONE ),
            nDPField( 0 ),
            pDragDPObj( nullptr ),
            nRFIndex( 0 ),
            nRFAddX( 0 ),
            nRFAddY( 0 ),
            nPagebreakMouse( SC_PD_NONE ),
            nPagebreakBreak( 0 ),
            nPagebreakPrev( 0 ),
            nPageScript( SvtScriptType::NONE ),
            nDragStartX( -1 ),
            nDragStartY( -1 ),
            nDragEndX( -1 ),
            nDragEndY( -1 ),
            meDragInsertMode( INS_NONE ),
            aComboButton( GetOutDev() ),
            aCurMousePos( 0,0 ),
            nPaintCount( 0 ),
            aRFSelectedCorned( NONE ),
            maShowPageBreaksTimer("ScGridWindow maShowPageBreaksTimer"),
            bEEMouse( false ),
            bDPMouse( false ),
            bRFMouse( false ),
            bRFSize( false ),
            bPagebreakDrawn( false ),
            bDragRect( false ),
            bIsInPaint( false ),
            bNeedsRepaint( false ),
            bAutoMarkVisible( false ),
            bListValButton( false )
{
    set_id("grid_window");
    switch(eWhich)
    {
        case SC_SPLIT_TOPLEFT:
            eHWhich = SC_SPLIT_LEFT;
            eVWhich = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_TOPRIGHT:
            eHWhich = SC_SPLIT_RIGHT;
            eVWhich = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_BOTTOMLEFT:
            eHWhich = SC_SPLIT_LEFT;
            eVWhich = SC_SPLIT_BOTTOM;
            break;
        case SC_SPLIT_BOTTOMRIGHT:
            eHWhich = SC_SPLIT_RIGHT;
            eVWhich = SC_SPLIT_BOTTOM;
            break;
        default:
            OSL_FAIL("GridWindow: wrong position");
    }

    SetBackground();

    SetMapMode(mrViewData.GetLogicMode(eWhich));
    EnableChildTransparentMode();
    SetDialogControlFlags( DialogControlFlags::Return | DialogControlFlags::WantFocus );

    SetHelpId( HID_SC_WIN_GRIDWIN );

    GetOutDev()->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
    EnableRTL( false );

    bInitialPageBreaks = true;
    maShowPageBreaksTimer.SetInvokeHandler(LINK(this, ScGridWindow, InitiatePageBreaksTimer));
    maShowPageBreaksTimer.SetTimeout(1);
}

ScGridWindow::~ScGridWindow()
{
    disposeOnce();
}

void ScGridWindow::dispose()
{
    maShowPageBreaksTimer.Stop();

    ImpDestroyOverlayObjects();

    mpFilterBox.reset();
    mpNoteMarker.reset();
    mpAutoFilterPopup.reset();
    mpDPFieldPopup.reset();
    aComboButton.SetOutputDevice(nullptr);

    if (mpSpellCheckCxt)
        mpSpellCheckCxt->reset();
    mpSpellCheckCxt.reset();

    vcl::Window::dispose();
}

void ScGridWindow::ClickExtern()
{
    do
    {
        // #i84277# when initializing the filter box, a Basic error can deactivate the view
        if (mpFilterBox && mpFilterBox->IsInInit())
            break;
        mpFilterBox.reset();
    }
    while (false);

    if (mpDPFieldPopup)
    {
        mpDPFieldPopup->close(false);
        mpDPFieldPopup.reset();
    }
}

IMPL_LINK_NOARG(ScGridWindow, PopupModeEndHdl, weld::Popover&, void)
{
    if (mpFilterBox)
    {
        bool bMouseWasCaptured = mpFilterBox->MouseWasCaptured();
        mpFilterBox->SetCancelled();     // cancel select
        // restore the mouse capture state of the GridWindow to
        // what it was at initial popup time
        if (bMouseWasCaptured)
            CaptureMouse();
    }
    GrabFocus();
}

IMPL_LINK( ScGridWindow, PopupSpellingHdl, SpellCallbackInfo&, rInfo, void )
{
    if( rInfo.nCommand == SpellCallbackCommand::STARTSPELLDLG )
        mrViewData.GetDispatcher().Execute( SID_SPELL_DIALOG, SfxCallMode::ASYNCHRON );
    else if (rInfo.nCommand == SpellCallbackCommand::AUTOCORRECT_OPTIONS)
        mrViewData.GetDispatcher().Execute( SID_AUTO_CORRECT_DLG, SfxCallMode::ASYNCHRON );
}

namespace {

struct AutoFilterData : public ScCheckListMenuControl::ExtendedData
{
    ScAddress maPos;
    ScDBData* mpData;
};

class AutoFilterAction : public ScCheckListMenuControl::Action
{
protected:
    VclPtr<ScGridWindow> mpWindow;
    ScGridWindow::AutoFilterMode meMode;
public:
    AutoFilterAction(ScGridWindow* p, ScGridWindow::AutoFilterMode eMode) :
        mpWindow(p), meMode(eMode) {}
    virtual bool execute() override
    {
        mpWindow->UpdateAutoFilterFromMenu(meMode);
        // UpdateAutoFilterFromMenu manually closes the popup so return
        // false to not attempt a second close
        return false;
    }
};

class AutoFilterPopupEndAction : public ScCheckListMenuControl::Action
{
    VclPtr<ScGridWindow> mpWindow;
    ScAddress maPos;
public:
    AutoFilterPopupEndAction(ScGridWindow* p, const ScAddress& rPos) :
        mpWindow(p), maPos(rPos) {}
    virtual bool execute() override
    {
        mpWindow->RefreshAutoFilterButton(maPos);
        return false; // this is called after the popup has been closed
    }
};

class AutoFilterSubMenuAction : public AutoFilterAction
{
protected:
    ScListSubMenuControl* m_pSubMenu;

public:
    AutoFilterSubMenuAction(ScGridWindow* p, ScListSubMenuControl* pSubMenu, ScGridWindow::AutoFilterMode eMode)
        : AutoFilterAction(p, eMode)
        , m_pSubMenu(pSubMenu)
    {
    }
};

class AutoFilterColorAction : public AutoFilterSubMenuAction
{
private:
    Color m_aColor;

public:
    AutoFilterColorAction(ScGridWindow* p, ScListSubMenuControl* pSubMenu, ScGridWindow::AutoFilterMode eMode, const Color& rColor)
        : AutoFilterSubMenuAction(p, pSubMenu, eMode)
        , m_aColor(rColor)
    {
    }

    virtual bool execute() override
    {
        const AutoFilterData* pData =
            static_cast<const AutoFilterData*>(m_pSubMenu->getExtendedData());

        if (!pData)
            return false;

        ScDBData* pDBData = pData->mpData;
        if (!pDBData)
            return false;

        const ScAddress& rPos = pData->maPos;

        ScViewData& rViewData = m_pSubMenu->GetViewData();
        ScDocument& rDoc = rViewData.GetDocument();

        ScQueryParam aParam;
        pDBData->GetQueryParam(aParam);

        // Try to use the existing entry for the column (if one exists).
        ScQueryEntry* pEntry = aParam.FindEntryByField(rPos.Col(), true);

        if (!pEntry)
        {
            // Something went terribly wrong!
            return false;
        }

        if (ScTabViewShell::isAnyEditViewInRange(rViewData.GetViewShell(), /*bColumns*/ false, aParam.nRow1, aParam.nRow2))
            return false;

        pEntry->bDoQuery = true;
        pEntry->nField = rPos.Col();
        pEntry->eConnect = SC_AND;

        ScFilterEntries aFilterEntries;
        rDoc.GetFilterEntries(rPos.Col(), rPos.Row(), rPos.Tab(), aFilterEntries);

        bool bActive = false;
        auto aItem = pEntry->GetQueryItem();
        if (aItem.maColor == m_aColor
            && ((meMode == ScGridWindow::AutoFilterMode::TextColor
                 && aItem.meType == ScQueryEntry::ByTextColor)
                || (meMode == ScGridWindow::AutoFilterMode::BackgroundColor
                    && aItem.meType == ScQueryEntry::ByBackgroundColor)))
        {
            bActive = true;
        }

        // Disable color filter when active color was selected
        if (bActive)
        {
            aParam.RemoveAllEntriesByField(rPos.Col());
            pEntry = nullptr;   // invalidated by RemoveAllEntriesByField call

            // tdf#46184 reset filter options to default values
            aParam.eSearchType = utl::SearchParam::SearchType::Normal;
            aParam.bCaseSens = false;
            aParam.bDuplicate = true;
            aParam.bInplace = true;
        }
        else
        {
            if (meMode == ScGridWindow::AutoFilterMode::TextColor)
                pEntry->SetQueryByTextColor(m_aColor);
            else
                pEntry->SetQueryByBackgroundColor(m_aColor);
        }

        rViewData.GetView()->Query(aParam, nullptr, true);
        pDBData->SetQueryParam(aParam);

        return true;
    }
};

class AutoFilterColorPopupStartAction : public AutoFilterSubMenuAction
{
public:
    AutoFilterColorPopupStartAction(ScGridWindow* p, ScListSubMenuControl* pSubMenu)
        : AutoFilterSubMenuAction(p, pSubMenu, ScGridWindow::AutoFilterMode::Normal)
    {
    }

    virtual bool execute() override
    {
        const AutoFilterData* pData =
            static_cast<const AutoFilterData*>(m_pSubMenu->getExtendedData());

        if (!pData)
            return false;

        ScDBData* pDBData = pData->mpData;
        if (!pDBData)
            return false;

        ScViewData& rViewData = m_pSubMenu->GetViewData();
        ScDocument& rDoc = rViewData.GetDocument();
        const ScAddress& rPos = pData->maPos;

        ScFilterEntries aFilterEntries;
        rDoc.GetFilterEntries(rPos.Col(), rPos.Row(), rPos.Tab(), aFilterEntries);

        m_pSubMenu->clearMenuItems();

        XColorListRef xUserColorList;

        OUString aPaletteName(officecfg::Office::Common::UserColors::PaletteName::get());
        PaletteManager aPaletteManager;
        std::vector<OUString> aPaletteNames = aPaletteManager.GetPaletteList();
        for (size_t i = 0, nLen = aPaletteNames.size(); i < nLen; ++i)
        {
            if (aPaletteName == aPaletteNames[i])
            {
                aPaletteManager.SetPalette(i);
                xUserColorList = XPropertyList::AsColorList(
                                XPropertyList::CreatePropertyListFromURL(
                                XPropertyListType::Color, aPaletteManager.GetSelectedPalettePath()));
                if (!xUserColorList->Load())
                    xUserColorList = nullptr;
                break;
            }
        }

        ScQueryParam aParam;
        pDBData->GetQueryParam(aParam);
        ScQueryEntry* pEntry = aParam.FindEntryByField(rPos.Col(), true);

        int nMenu = 0;
        for (auto eMode : {ScGridWindow::AutoFilterMode::BackgroundColor, ScGridWindow::AutoFilterMode::TextColor})
        {
            std::set<Color> aColors = eMode == ScGridWindow::AutoFilterMode::TextColor
                                          ? aFilterEntries.getTextColors()
                                          : aFilterEntries.getBackgroundColors();

            for (auto& rColor : aColors)
            {
                bool bActive = false;

                if (pEntry)
                {
                    auto aItem = pEntry->GetQueryItem();
                    if (aItem.maColor == rColor
                        && ((eMode == ScGridWindow::AutoFilterMode::TextColor
                             && aItem.meType == ScQueryEntry::ByTextColor)
                            || (eMode == ScGridWindow::AutoFilterMode::BackgroundColor
                                && aItem.meType == ScQueryEntry::ByBackgroundColor)))
                    {
                        bActive = true;
                    }
                }

                const bool bAutoColor = rColor == COL_AUTO;

                // ColorListBox::ShowPreview is similar
                ScopedVclPtr<VirtualDevice> xDev(m_pSubMenu->create_virtual_device());
                const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                Size aImageSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());
                xDev->SetOutputSize(aImageSize);
                const tools::Rectangle aRect(Point(0, 0), aImageSize);

                if (bAutoColor)
                {
                    const Color aW(COL_WHITE);
                    const Color aG(0xef, 0xef, 0xef);
                    int nMinDim = std::min(aImageSize.Width(), aImageSize.Height()) + 1;
                    int nCheckSize = nMinDim / 3;
                    xDev->DrawCheckered(aRect.TopLeft(), aRect.GetSize(), std::min(nCheckSize, 8), aW, aG);
                    xDev->SetFillColor();
                }
                else
                    xDev->SetFillColor(rColor);

                xDev->SetLineColor(rStyleSettings.GetDisableColor());
                xDev->DrawRect(aRect);

                if (bAutoColor)
                {
                    OUString sText = eMode == ScGridWindow::AutoFilterMode::TextColor
                                         ? ScResId(SCSTR_FILTER_AUTOMATIC_COLOR)
                                         : ScResId(SCSTR_FILTER_NO_FILL);
                    m_pSubMenu->addMenuColorItem(sText, bActive, *xDev, nMenu,
                                                 new AutoFilterColorAction(mpWindow, m_pSubMenu, eMode, rColor));
                }
                else
                {
                    OUString sName;

                    bool bFoundColorName = false;
                    if (xUserColorList)
                    {
                        sal_Int32 nPos = xUserColorList->GetIndexOfColor(rColor);
                        if (nPos != -1)
                        {
                            XColorEntry* pColorEntry = xUserColorList->GetColor(nPos);
                            sName = pColorEntry->GetName();
                            bFoundColorName = true;
                        }
                    }
                    if (!bFoundColorName)
                        sName = "#" + rColor.AsRGBHexString().toAsciiUpperCase();

                    m_pSubMenu->addMenuColorItem(sName, bActive, *xDev, nMenu,
                                                 new AutoFilterColorAction(mpWindow, m_pSubMenu, eMode, rColor));
                }
            }

            ++nMenu;
        }

        m_pSubMenu->resizeToFitMenuItems();

        return false;
    }
};

class AddItemToEntry
{
    ScQueryEntry::QueryItemsType& mrItems;
    svl::SharedStringPool& mrPool;
public:
    AddItemToEntry(ScQueryEntry::QueryItemsType& rItems, svl::SharedStringPool& rPool) :
        mrItems(rItems), mrPool(rPool) {}
    void operator() (const ScCheckListMenuControl::ResultEntry& rEntry)
    {
        if (rEntry.bValid)
        {
            ScQueryEntry::Item aNew;
            aNew.maString = mrPool.intern(rEntry.aName);
            // set the filter type to ByValue, if the filter condition is value
            aNew.meType = rEntry.bDate ? ScQueryEntry::ByDate : rEntry.bValue ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
            aNew.mfVal = rEntry.nValue;
            mrItems.push_back(aNew);
        }
    }
};

class AddSelectedItemString
{
    std::unordered_set<OUString>& mrSetString;
    std::unordered_set<double>& mrSetValue;
public:
    explicit AddSelectedItemString(std::unordered_set<OUString>& rString, std::unordered_set<double>& rValue) :
        mrSetString(rString), mrSetValue(rValue) {}

    void operator() (const ScQueryEntry::Item& rItem)
    {
        if( rItem.meType == ScQueryEntry::QueryType::ByValue )
            mrSetValue.insert(rItem.mfVal);
        else
            mrSetString.insert(rItem.maString.getString());
    }
};

void collectUIInformation(const OUString& aRow, const OUString& aCol , const OUString& aevent)
{
    EventDescription aDescription;
    aDescription.aAction = "LAUNCH";
    aDescription.aID = "grid_window";
    aDescription.aParameters = {{aevent, ""},
        {"ROW", aRow}, {"COL", aCol}};
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "ScGridWinUIObject";

    UITestLogger::getInstance().logEvent(aDescription);
}

}

void ScGridWindow::LaunchAutoFilterMenu(SCCOL nCol, SCROW nRow)
{
    SCTAB nTab = mrViewData.GetTabNo();
    ScDocument& rDoc = mrViewData.GetDocument();
    bool bLOKActive = comphelper::LibreOfficeKit::isActive();

    mpAutoFilterPopup.reset();

    // Estimate the width (in pixels) of the longest text in the list
    ScFilterEntries aFilterEntries;
    rDoc.GetFilterEntries(nCol, nRow, nTab, aFilterEntries);

    vcl::ILibreOfficeKitNotifier* pNotifier = nullptr;
    if (bLOKActive)
        pNotifier = SfxViewShell::Current();

    weld::Window* pPopupParent = GetFrameWeld();
    int nColWidth = ScViewData::ToPixel(rDoc.GetColWidth(nCol, nTab), mrViewData.GetPPTX());
    mpAutoFilterPopup.reset(new ScCheckListMenuControl(pPopupParent, mrViewData,
                                                       aFilterEntries.mbHasDates, nColWidth, pNotifier));

    int nMaxTextWidth = 0;
    if (aFilterEntries.size() <= 10)
    {
        // do pixel calculation for all elements of short lists
        for (const auto& rEntry : aFilterEntries)
        {
            const OUString& aText = rEntry.GetString();
            nMaxTextWidth = std::max<int>(nMaxTextWidth, mpAutoFilterPopup->GetTextWidth(aText) + aText.getLength() * 2);
        }
    }
    else
    {
        // find the longest string, probably it will be the longest rendered text, too
        // (performance optimization for long lists)
        auto itMax = aFilterEntries.begin();
        for (auto it = itMax; it != aFilterEntries.end(); ++it)
        {
            int nTextWidth = it->GetString().getLength();
            if (nMaxTextWidth < nTextWidth)
            {
                nMaxTextWidth = nTextWidth;
                itMax = it;
            }
        }
        nMaxTextWidth = mpAutoFilterPopup->GetTextWidth(itMax->GetString()) + nMaxTextWidth * 2;
    }

    // window should be at least as wide as the column, or the longest text + checkbox, scrollbar ... (it is estimated with 70 pixel now)
    // window should be maximum 1024 pixel wide.
    int nWindowWidth = std::min<int>(1024, nMaxTextWidth + 70);
    nWindowWidth = mpAutoFilterPopup->IncreaseWindowWidthToFitText(nWindowWidth);
    nMaxTextWidth = std::max<int>(nMaxTextWidth, nWindowWidth - 70);

    mpAutoFilterPopup->setOKAction(new AutoFilterAction(this, AutoFilterMode::Normal));
    mpAutoFilterPopup->setPopupEndAction(
        new AutoFilterPopupEndAction(this, ScAddress(nCol, nRow, nTab)));
    std::unique_ptr<AutoFilterData> pData(new AutoFilterData);
    pData->maPos = ScAddress(nCol, nRow, nTab);

    Point aPos = mrViewData.GetScrPos(nCol, nRow, eWhich);
    tools::Long nSizeX  = 0;
    tools::Long nSizeY  = 0;
    mrViewData.GetMergeSizePixel(nCol, nRow, nSizeX, nSizeY);
    if (bLOKActive)
    {
        // Reverse the zoom factor from aPos and nSize[X|Y]
        // before letting the autofilter window convert the to twips
        // with no zoom information.
        double fZoomX(mrViewData.GetZoomX());
        double fZoomY(mrViewData.GetZoomY());
        aPos.setX(aPos.getX() / fZoomX);
        aPos.setY(aPos.getY() / fZoomY);
        nSizeX = nSizeX / fZoomX;
        nSizeY = nSizeY / fZoomY;
    }
    tools::Rectangle aCellRect(OutputToScreenPixel(aPos), Size(nSizeX, nSizeY));

    ScDBData* pDBData = rDoc.GetDBAtCursor(nCol, nRow, nTab, ScDBDataPortion::AREA);
    if (!pDBData)
        return;

    pData->mpData = pDBData;
    mpAutoFilterPopup->setExtendedData(std::move(pData));

    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    std::vector<ScQueryEntry*> aEntries = aParam.FindAllEntriesByField(nCol);
    std::unordered_set<OUString> aSelectedString;
    std::unordered_set<double> aSelectedValue;
    bool bQueryByNonEmpty = aEntries.size() == 1 && aEntries[0]->IsQueryByNonEmpty();

    if (!bQueryByNonEmpty)
    {
        for (ScQueryEntry* pEntry : aEntries)
        {
            if (pEntry && pEntry->eOp == SC_EQUAL)
            {
                ScQueryEntry::QueryItemsType& rItems = pEntry->GetQueryItems();
                std::for_each(rItems.begin(), rItems.end(), AddSelectedItemString(aSelectedString, aSelectedValue));
            }
        }
    }

    // Populate the check box list.
    mpAutoFilterPopup->setMemberSize(aFilterEntries.size());
    for (auto it = aFilterEntries.begin(); it != aFilterEntries.end(); ++it)
    {
        // tdf#140745 show (empty) entry on top of the checkbox list
        if (it->GetString().isEmpty())
        {
            const OUString& aStringVal = it->GetString();
            const double aDoubleVal = it->GetValue();
            bool bSelected = true;
            if (!aSelectedValue.empty() || !aSelectedString.empty())
                bSelected = aSelectedString.count(aStringVal) > 0;
            else if (bQueryByNonEmpty)
                bSelected = false;
            mpAutoFilterPopup->addMember(aStringVal, aDoubleVal, bSelected);
            aFilterEntries.maStrData.erase(it);
            break;
        }
    }
    for (const auto& rEntry : aFilterEntries)
    {
        const OUString& aStringVal = rEntry.GetString();
        const double aDoubleVal = rEntry.GetValue();
        const double aRDoubleVal = rEntry.GetRoundedValue();
        bool bSelected = true;

        if (!aSelectedValue.empty() || !aSelectedString.empty())
        {
            if (aDoubleVal == aRDoubleVal)
                bSelected = aSelectedValue.count(aDoubleVal) > 0 || aSelectedString.count(aStringVal) > 0;
            else
                bSelected = aSelectedValue.count(aDoubleVal) > 0 || aSelectedValue.count(aRDoubleVal) > 0 || aSelectedString.count(aStringVal) > 0;
        }

        if ( rEntry.IsDate() )
            mpAutoFilterPopup->addDateMember( aStringVal, rEntry.GetValue(), bSelected );
        else
            mpAutoFilterPopup->addMember( aStringVal, aRDoubleVal, bSelected, rEntry.GetStringType() == ScTypedStrData::Value );
    }

    // Populate the menu.
    mpAutoFilterPopup->addMenuItem(
        ScResId(STR_MENU_SORT_ASC),
        new AutoFilterAction(this, AutoFilterMode::SortAscending));
    mpAutoFilterPopup->addMenuItem(
        ScResId(STR_MENU_SORT_DESC),
        new AutoFilterAction(this, AutoFilterMode::SortDescending));
    mpAutoFilterPopup->addSeparator();
    if (ScListSubMenuControl* pSubMenu = mpAutoFilterPopup->addSubMenuItem(ScResId(SCSTR_FILTER_COLOR), true, true))
        pSubMenu->setPopupStartAction(new AutoFilterColorPopupStartAction(this, pSubMenu));
    if (ScListSubMenuControl* pSubMenu = mpAutoFilterPopup->addSubMenuItem(ScResId(SCSTR_FILTER_CONDITION), true, false))
    {
        pSubMenu->addMenuItem(
            ScResId(SCSTR_TOP10FILTER), new AutoFilterAction(this, AutoFilterMode::Top10));
        pSubMenu->addMenuItem(
            ScResId(SCSTR_FILTER_EMPTY), new AutoFilterAction(this, AutoFilterMode::Empty));
        pSubMenu->addMenuItem(
            ScResId(SCSTR_FILTER_NOTEMPTY), new AutoFilterAction(this, AutoFilterMode::NonEmpty));
        pSubMenu->addSeparator();
        pSubMenu->addMenuItem(
            ScResId(SCSTR_STDFILTER), new AutoFilterAction(this, AutoFilterMode::Custom));
        pSubMenu->resizeToFitMenuItems();
    }
    if (aEntries.size())
        mpAutoFilterPopup->addMenuItem(
            ScResId(SCSTR_CLEAR_FILTER), new AutoFilterAction(this, AutoFilterMode::Clear));

    mpAutoFilterPopup->initMembers(nMaxTextWidth + 20); // 20 pixel estimated for the checkbox

    ScCheckListMenuControl::Config aConfig;
    aConfig.mbAllowEmptySet = false;
    aConfig.mbRTL = mrViewData.GetDocument().IsLayoutRTL(mrViewData.GetTabNo());
    mpAutoFilterPopup->setConfig(aConfig);
    if (IsMouseCaptured())
        ReleaseMouse();
    mpAutoFilterPopup->launch(pPopupParent, aCellRect);

    // remember filter rules before modification
    mpAutoFilterPopup->getResult(aSaveAutoFilterResult);

    collectUIInformation(OUString::number(nRow), OUString::number(nCol),"AUTOFILTER");
}

void ScGridWindow::RefreshAutoFilterButton(const ScAddress& rPos)
{
    if (mpFilterButton)
    {
        bool bFilterActive = IsAutoFilterActive(rPos.Col(), rPos.Row(), rPos.Tab());
        mpFilterButton->setHasHiddenMember(bFilterActive);
        mpFilterButton->setPopupPressed(false);
        mpFilterButton->draw();
    }
}

void ScGridWindow::UpdateAutoFilterFromMenu(AutoFilterMode eMode)
{
    // Terminate autofilter popup now when there is no further user input needed
    bool bColorMode = eMode == AutoFilterMode::TextColor || eMode == AutoFilterMode::BackgroundColor;
    if (!bColorMode)
        mpAutoFilterPopup->terminateAllPopupMenus();

    const AutoFilterData* pData =
        static_cast<const AutoFilterData*>(mpAutoFilterPopup->getExtendedData());

    if (!pData)
        return;

    const ScAddress& rPos = pData->maPos;
    ScDBData* pDBData = pData->mpData;
    if (!pDBData)
        return;

    ScDocument& rDoc = mrViewData.GetDocument();
    svl::SharedStringPool& rPool = rDoc.GetSharedStringPool();
    switch (eMode)
    {
        case AutoFilterMode::SortAscending:
        case AutoFilterMode::SortDescending:
        {
            SCCOL nCol = rPos.Col();
            ScSortParam aSortParam;
            pDBData->GetSortParam(aSortParam);
            if (nCol < aSortParam.nCol1 || nCol > aSortParam.nCol2)
                // out of bound
                return;

            bool bHasHeader = pDBData->HasHeader();

            aSortParam.bHasHeader = bHasHeader;
            aSortParam.bByRow = true;
            aSortParam.bCaseSens = false;
            aSortParam.bNaturalSort = false;
            aSortParam.aDataAreaExtras.mbCellNotes = false;
            aSortParam.aDataAreaExtras.mbCellDrawObjects = true;
            aSortParam.aDataAreaExtras.mbCellFormats = true;
            aSortParam.bInplace = true;
            aSortParam.maKeyState[0].bDoSort = true;
            aSortParam.maKeyState[0].nField = nCol;
            aSortParam.maKeyState[0].bAscending = (eMode == AutoFilterMode::SortAscending);

            for (size_t i = 1; i < aSortParam.GetSortKeyCount(); ++i)
                aSortParam.maKeyState[i].bDoSort = false;

            mrViewData.GetViewShell()->UISort(aSortParam);
            return;
        }
        case AutoFilterMode::Custom:
        {
            ScRange aRange;
            pDBData->GetArea(aRange);
            mrViewData.GetView()->MarkRange(aRange);
            mrViewData.GetView()->SetCursor(rPos.Col(), rPos.Row());
            mrViewData.GetDispatcher().Execute(SID_FILTER, SfxCallMode::SLOT | SfxCallMode::RECORD);
            return;
        }
        default:
            ;
    }

    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);

    if (eMode == AutoFilterMode::Normal)
    {
        // Do not recreate autofilter rules if there are no changes from the user
        ScCheckListMenuControl::ResultType aResult;
        mpAutoFilterPopup->getResult(aResult);

        if (aResult == aSaveAutoFilterResult)
        {
            SAL_INFO("sc.ui", "Apply autofilter to data when entries are the same");

            if (!mpAutoFilterPopup->isAllSelected())
            {
                // Apply autofilter to data
                ScQueryEntry* pEntry = aParam.FindEntryByField(rPos.Col(), true);
                pEntry->bDoQuery = true;
                pEntry->nField = rPos.Col();
                pEntry->eConnect = SC_AND;
                pEntry->eOp = SC_EQUAL;
                mrViewData.GetView()->Query(aParam, nullptr, true);
            }

            return;
        }
    }

    // Remove old entries in auto-filter rules
    if (!bColorMode)
    {
        aParam.RemoveAllEntriesByField(rPos.Col());

        // tdf#46184 reset filter options to default values
        aParam.eSearchType = utl::SearchParam::SearchType::Normal;
        aParam.bCaseSens = false;
        aParam.bDuplicate = true;
        aParam.bInplace = true;
    }

    if (eMode != AutoFilterMode::Clear
        && !(eMode == AutoFilterMode::Normal && mpAutoFilterPopup->isAllSelected()))
    {
        // Try to use the existing entry for the column (if one exists).
        ScQueryEntry* pEntry = aParam.FindEntryByField(rPos.Col(), true);

        if (!pEntry)
            // Something went terribly wrong!
            return;

        if (ScTabViewShell::isAnyEditViewInRange(mrViewData.GetViewShell(), /*bColumns*/ false, aParam.nRow1, aParam.nRow2))
            return;

        pEntry->bDoQuery = true;
        pEntry->nField = rPos.Col();
        pEntry->eConnect = SC_AND;

        switch (eMode)
        {
            case AutoFilterMode::Normal:
            {
                pEntry->eOp = SC_EQUAL;

                ScCheckListMenuControl::ResultType aResult;
                mpAutoFilterPopup->getResult(aResult);

                ScQueryEntry::QueryItemsType& rItems = pEntry->GetQueryItems();
                rItems.clear();
                std::for_each(aResult.begin(), aResult.end(), AddItemToEntry(rItems, rPool));
            }
            break;
            case AutoFilterMode::Top10:
                pEntry->eOp = SC_TOPVAL;
                pEntry->GetQueryItem().meType = ScQueryEntry::ByString;
                pEntry->GetQueryItem().maString = rPool.intern("10");
            break;
            case AutoFilterMode::Empty:
                pEntry->SetQueryByEmpty();
            break;
            case AutoFilterMode::NonEmpty:
                pEntry->SetQueryByNonEmpty();
            break;
            case AutoFilterMode::TextColor:
            case AutoFilterMode::BackgroundColor:
                assert(false && "should be handled by AutoFilterColorAction::execute");
            break;
            break;
            default:
                // We don't know how to handle this!
                return;
        }
    }

    mrViewData.GetView()->Query(aParam, nullptr, true);
    pDBData->SetQueryParam(aParam);
}

namespace {

void getCellGeometry(Point& rScrPos, Size& rScrSize, const ScViewData& rViewData, SCCOL nCol, SCROW nRow, ScSplitPos eWhich)
{
    // Get the screen position of the cell.
    rScrPos = rViewData.GetScrPos(nCol, nRow, eWhich);

    // Get the screen size of the cell.
    tools::Long nSizeX, nSizeY;
    rViewData.GetMergeSizePixel(nCol, nRow, nSizeX, nSizeY);
    rScrSize = Size(nSizeX-1, nSizeY-1);
}

}

void ScGridWindow::LaunchPageFieldMenu( SCCOL nCol, SCROW nRow )
{
    if (nCol == 0)
        // We assume that the page field button is located in cell to the immediate left.
        return;

    SCTAB nTab = mrViewData.GetTabNo();
    ScDPObject* pDPObj = mrViewData.GetDocument().GetDPAtCursor(nCol, nRow, nTab);
    if (!pDPObj)
        return;

    Point aScrPos;
    Size aScrSize;
    getCellGeometry(aScrPos, aScrSize, mrViewData, nCol, nRow, eWhich);
    DPLaunchFieldPopupMenu(OutputToScreenPixel(aScrPos), aScrSize, ScAddress(nCol-1, nRow, nTab), pDPObj);
}

void ScGridWindow::LaunchDPFieldMenu( SCCOL nCol, SCROW nRow )
{
    SCTAB nTab = mrViewData.GetTabNo();
    ScDPObject* pDPObj = mrViewData.GetDocument().GetDPAtCursor(nCol, nRow, nTab);
    if (!pDPObj)
        return;

    Point aScrPos;
    Size aScrSize;
    getCellGeometry(aScrPos, aScrSize, mrViewData, nCol, nRow, eWhich);
    DPLaunchFieldPopupMenu(OutputToScreenPixel(aScrPos), aScrSize, ScAddress(nCol, nRow, nTab), pDPObj);
}

void ScGridWindow::ShowFilterMenu(weld::Window* pParent, const tools::Rectangle& rCellRect, bool bLayoutRTL)
{
    auto nSizeX = rCellRect.GetWidth();

    // minimum width in pixel
    if (comphelper::LibreOfficeKit::isActive())
    {
        const tools::Long nMinLOKWinWidth = o3tl::convert(STD_COL_WIDTH * 13 / 10, o3tl::Length::twip, o3tl::Length::px);
        if (nSizeX < nMinLOKWinWidth)
            nSizeX = nMinLOKWinWidth;
    }

    weld::TreeView& rFilterBox = mpFilterBox->get_widget();
    int nEntryCount = rFilterBox.n_children();
    if (nEntryCount > SC_FILTERLISTBOX_LINES)
        nEntryCount = SC_FILTERLISTBOX_LINES;
    auto nHeight = rFilterBox.get_height_rows(nEntryCount);
    rFilterBox.set_size_request(-1, nHeight);
    Size aSize(rFilterBox.get_preferred_size());
    auto nMaxToExpandTo = std::min(nSizeX, static_cast<decltype(nSizeX)>(300));     // do not over do it (Pixel)
    if (aSize.Width() < nMaxToExpandTo)
        aSize.setWidth(nMaxToExpandTo);

    aSize.AdjustWidth(4); // add a little margin
    nSizeX += 4;
    aSize.AdjustHeight(4);

    tools::Rectangle aCellRect(rCellRect);
    aCellRect.AdjustLeft(-2); // offset the little margin above

    if (!bLayoutRTL && aSize.Width() > nSizeX)
    {
        //  move popup position
        tools::Long nDiff = aSize.Width() - nSizeX;
        tools::Long nNewX = aCellRect.Left() - nDiff;
        if ( nNewX < 0 )
            nNewX = 0;
        aCellRect.SetLeft( nNewX );
    }

    rFilterBox.set_size_request(aSize.Width(), aSize.Height());

    if (IsMouseCaptured())
        ReleaseMouse();
    mpFilterBox->popup_at_rect(pParent, aCellRect);
}

void ScGridWindow::DoScenarioMenu( const ScRange& rScenRange )
{
    bool bMenuAtTop = true;

    ScDocument& rDoc = mrViewData.GetDocument();
    mpFilterBox.reset();

    SCCOL nCol = rScenRange.aEnd.Col();     // Cell is below the Buttons
    SCROW nRow = rScenRange.aStart.Row();
    if (nRow == 0)
    {
        nRow = rScenRange.aEnd.Row() + 1;   // Range at very the top -> Button below
        if (nRow>rDoc.MaxRow()) nRow = rDoc.MaxRow();
        bMenuAtTop = false;
    }

    SCTAB nTab = mrViewData.GetTabNo();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

    tools::Long nSizeX  = 0;
    tools::Long nSizeY  = 0;
    mrViewData.GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    // The button height should not use the merged cell height, should still use single row height
    nSizeY = ScViewData::ToPixel(rDoc.GetRowHeight(nRow, nTab), mrViewData.GetPPTY());
    Point aPos = mrViewData.GetScrPos( nCol, nRow, eWhich );
    if ( bLayoutRTL )
        aPos.AdjustX( -nSizeX );
    tools::Rectangle aCellRect(aPos, Size(nSizeX, nSizeY));
    aCellRect.AdjustTop( -nSizeY );
    aCellRect.AdjustBottom( -(nSizeY - 1) );
    if (!bMenuAtTop)
    {
        Size aButSize = mrViewData.GetScenButSize();
        aCellRect.AdjustBottom(aButSize.Height());
    }

    //  Place the ListBox directly below the black line of the cell grid
    //  (It looks odd if the line gets hidden...)

    weld::Window* pParent = weld::GetPopupParent(*this, aCellRect);
    mpFilterBox = std::make_shared<ScFilterListBox>(pParent, this, nCol, nRow, ScFilterBoxMode::Scenario);
    mpFilterBox->connect_closed(LINK(this, ScGridWindow, PopupModeEndHdl));
    weld::TreeView& rFilterBox = mpFilterBox->get_widget();
    rFilterBox.set_direction(bLayoutRTL); // Fix for bug fdo#44925 use sheet direction for widget RTL/LTR

    //  Listbox fill
    rFilterBox.freeze();
    OUString aCurrent;
    OUString aTabName;
    SCTAB nTabCount = rDoc.GetTableCount();
    SCTAB nEntryCount = 0;
    for (SCTAB i=nTab+1; i<nTabCount && rDoc.IsScenario(i); i++)
    {
        if (rDoc.HasScenarioRange( i, rScenRange ))
            if (rDoc.GetName( i, aTabName ))
            {
                rFilterBox.append_text(aTabName);
                if (rDoc.IsActiveScenario(i))
                    aCurrent = aTabName;
                ++nEntryCount;
            }
    }
    rFilterBox.thaw();

    ShowFilterMenu(pParent, aCellRect, bLayoutRTL);

    rFilterBox.grab_focus();

    sal_Int32 nPos = -1;
    if (!aCurrent.isEmpty())
    {
        nPos = rFilterBox.find_text(aCurrent);
    }
    if (nPos == -1 && rFilterBox.n_children() > 0 )
    {
        nPos = 0;
    }
    if (nPos != -1)
    {
        rFilterBox.set_cursor(nPos);
        rFilterBox.select(nPos);
    }
    mpFilterBox->EndInit();
}

void ScGridWindow::LaunchDataSelectMenu( SCCOL nCol, SCROW nRow )
{
    mpFilterBox.reset();

    ScDocument& rDoc = mrViewData.GetDocument();
    SCTAB nTab = mrViewData.GetTabNo();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

    tools::Long nSizeX  = 0;
    tools::Long nSizeY  = 0;
    mrViewData.GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    Point aPos = mrViewData.GetScrPos( nCol, nRow, eWhich );
    bool bLOKActive = comphelper::LibreOfficeKit::isActive();

    if (bLOKActive)
    {
        // aPos is now view-zoom adjusted and in pixels an more importantly this is pixel aligned to the view-zoom,
        // but once we use this to set the position of the floating window, it has no information of view-zoom level
        // so if we don't reverse the zoom now, a simple PixelToLogic(aPos, MapMode(MapUnit::MapTwip)) employed in
        // FloatingWindow::ImplCalcPos will produce a 'scaled' twips position which will again get zoom scaled in the
        // client (effective double scaling) causing wrong positioning/size.
        double fZoomX(mrViewData.GetZoomX());
        double fZoomY(mrViewData.GetZoomY());
        aPos.setX(aPos.getX() / fZoomX);
        aPos.setY(aPos.getY() / fZoomY);
        nSizeX = nSizeX / fZoomX;
        nSizeY = nSizeY / fZoomY;
    }

    if ( bLayoutRTL )
        aPos.AdjustX( -nSizeX );
    tools::Rectangle aCellRect(aPos, Size(nSizeX, nSizeY));

    weld::Window* pParent = weld::GetPopupParent(*this, aCellRect);
    mpFilterBox = std::make_shared<ScFilterListBox>(pParent, this, nCol, nRow, ScFilterBoxMode::DataSelect);
    mpFilterBox->connect_closed(LINK(this, ScGridWindow, PopupModeEndHdl));
    weld::TreeView& rFilterBox = mpFilterBox->get_widget();
    rFilterBox.set_direction(bLayoutRTL); // Fix for bug fdo#44925 use sheet direction for widget RTL/LTR

    // SetSize later

    bool bEmpty = false;
    std::vector<ScTypedStrData> aStrings; // case sensitive
    // Fill List
    rDoc.GetDataEntries(nCol, nRow, nTab, aStrings, true /* bValidation */);
    if (aStrings.empty())
        bEmpty = true;

    if (!bEmpty)
    {
        rFilterBox.freeze();

        // Fill Listbox
        bool bWait = aStrings.size() > 100;

        if (bWait)
            EnterWait();

        for (const auto& rString : aStrings)
            rFilterBox.append_text(rString.GetString());

        if (bWait)
            LeaveWait();

        rFilterBox.thaw();

        ShowFilterMenu(pParent, aCellRect, bLayoutRTL);
    }

    sal_Int32 nSelPos = -1;

    sal_uLong nIndex = rDoc.GetAttr( nCol, nRow, nTab, ATTR_VALIDDATA )->GetValue();
    if ( nIndex )
    {
        const ScValidationData* pData = rDoc.GetValidationEntry( nIndex );
        if (pData)
        {
            std::unique_ptr<ScTypedStrData> pNew;
            OUString aDocStr = rDoc.GetString(nCol, nRow, nTab);
            if ( rDoc.HasValueData( nCol, nRow, nTab ) )
            {
                double fVal = rDoc.GetValue(ScAddress(nCol, nRow, nTab));
                pNew.reset(new ScTypedStrData(aDocStr, fVal, fVal, ScTypedStrData::Value));
            }
            else
                pNew.reset(new ScTypedStrData(aDocStr, 0.0, 0.0, ScTypedStrData::Standard));

            if (pData->GetListType() == css::sheet::TableValidationVisibility::SORTEDASCENDING)
            {
                auto it = std::lower_bound(aStrings.begin(), aStrings.end(), *pNew, ScTypedStrData::LessCaseSensitive());
                if (it != aStrings.end() && ScTypedStrData::EqualCaseSensitive()(*it, *pNew))
                    nSelPos = static_cast<sal_Int32>(std::distance(aStrings.begin(), it));
            }
            else
            {
                auto it = std::find_if(aStrings.begin(), aStrings.end(), FindTypedStrData(*pNew, true));
                if (it != aStrings.end())
                    nSelPos = static_cast<sal_Int32>(std::distance(aStrings.begin(), it));
            }
        }
    }

    // Do not show an empty selection List:

    if ( bEmpty )
    {
        mpFilterBox.reset();
    }
    else
    {
        rFilterBox.grab_focus();

        // Select only after GrabFocus, so that the focus rectangle gets correct
        if (nSelPos != -1)
        {
            rFilterBox.set_cursor(nSelPos);
            rFilterBox.select(nSelPos);
        }
        else
            rFilterBox.unselect_all();

        mpFilterBox->EndInit();
    }
    collectUIInformation(OUString::number(nRow), OUString::number(nCol),"SELECTMENU");
}

void ScGridWindow::FilterSelect( sal_uLong nSel )
{
    weld::TreeView& rFilterBox = mpFilterBox->get_widget();
    OUString aString = rFilterBox.get_text(static_cast<sal_Int32>(nSel));

    SCCOL nCol = mpFilterBox->GetCol();
    SCROW nRow = mpFilterBox->GetRow();
    switch (mpFilterBox->GetMode())
    {
        case ScFilterBoxMode::DataSelect:
            ExecDataSelect(nCol, nRow, aString);
            break;
        case ScFilterBoxMode::Scenario:
            mrViewData.GetView()->UseScenario(aString);
            break;
    }

    if (mpFilterBox)
        mpFilterBox->popdown();

    GrabFocus();        // Otherwise the focus would be wrong on OS/2
}

void ScGridWindow::ExecDataSelect( SCCOL nCol, SCROW nRow, const OUString& rStr )
{
    ScModule* pScMod = SC_MOD();
    ScInputHandler* pViewHdl = pScMod->GetInputHdl(mrViewData.GetViewShell());
    if (pViewHdl && mrViewData.HasEditView(mrViewData.GetActivePart()))
        pViewHdl->CancelHandler();

    SCTAB nTab = mrViewData.GetTabNo();
    ScViewFunc* pView = mrViewData.GetView();
    pView->EnterData( nCol, nRow, nTab, rStr );

    // #i52307# CellContentChanged is not in EnterData so it isn't called twice
    // if the cursor is moved afterwards.
    pView->CellContentChanged();
}

void ScGridWindow::MoveMouseStatus( ScGridWindow& rDestWin )
{
    if (nButtonDown)
    {
        rDestWin.nButtonDown = nButtonDown;
        rDestWin.nMouseStatus = nMouseStatus;
    }

    if (bRFMouse)
    {
        rDestWin.bRFMouse = bRFMouse;
        rDestWin.bRFSize  = bRFSize;
        rDestWin.nRFIndex = nRFIndex;
        rDestWin.nRFAddX  = nRFAddX;
        rDestWin.nRFAddY  = nRFAddY;
        bRFMouse = false;
    }

    if (nPagebreakMouse)
    {
        rDestWin.nPagebreakMouse  = nPagebreakMouse;
        rDestWin.nPagebreakBreak  = nPagebreakBreak;
        rDestWin.nPagebreakPrev   = nPagebreakPrev;
        rDestWin.aPagebreakSource = aPagebreakSource;
        rDestWin.aPagebreakDrag   = aPagebreakDrag;
        nPagebreakMouse = SC_PD_NONE;
    }
}

bool ScGridWindow::TestMouse( const MouseEvent& rMEvt, bool bAction )
{
    //  MouseEvent buttons must only be checked if bAction==TRUE
    //  to allow changing the mouse pointer in MouseMove,
    //  but not start AutoFill with right button (#74229#).
    //  with bAction==sal_True, SetFillMode / SetDragMode is called

    if ( bAction && !rMEvt.IsLeft() )
        return false;

    bool bNewPointer = false;

    SfxInPlaceClient* pClient = mrViewData.GetViewShell()->GetIPClient();
    bool bOleActive = ( pClient && pClient->IsObjectInPlaceActive() );

    if ( mrViewData.IsActive() && !bOleActive )
    {
        ScDocument& rDoc = mrViewData.GetDocument();
        SCTAB nTab = mrViewData.GetTabNo();
        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

        //  Auto-Fill

        ScRange aMarkRange;
        if (mrViewData.GetSimpleArea( aMarkRange ) == SC_MARK_SIMPLE)
        {
            if (aMarkRange.aStart.Tab() == mrViewData.GetTabNo() && mpAutoFillRect)
            {
                Point aMousePos = rMEvt.GetPosPixel();
                if (mpAutoFillRect->Contains(aMousePos))
                {
                    SetPointer( PointerStyle::Cross );     //! bold cross ?
                    if (bAction)
                    {
                        SCCOL nX = aMarkRange.aEnd.Col();
                        SCROW nY = aMarkRange.aEnd.Row();

                        if ( lcl_IsEditableMatrix( mrViewData.GetDocument(), aMarkRange ) )
                            mrViewData.SetDragMode(
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nX, nY, ScFillMode::MATRIX );
                        else
                            mrViewData.SetFillMode(
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nX, nY );

                        //  The simple selection must also be recognized when dragging,
                        //  where the Marking flag is set and MarkToSimple won't work anymore.
                        mrViewData.GetMarkData().MarkToSimple();
                    }
                    bNewPointer = true;
                }
            }
        }

        //  Embedded rectangle

        if (rDoc.IsEmbedded())
        {
            ScRange aRange;
            rDoc.GetEmbedded( aRange );
            if ( mrViewData.GetTabNo() == aRange.aStart.Tab() )
            {
                Point aStartPos = mrViewData.GetScrPos( aRange.aStart.Col(), aRange.aStart.Row(), eWhich );
                Point aEndPos   = mrViewData.GetScrPos( aRange.aEnd.Col()+1, aRange.aEnd.Row()+1, eWhich );
                Point aMousePos = rMEvt.GetPosPixel();
                if ( bLayoutRTL )
                {
                    aStartPos.AdjustX(2 );
                    aEndPos.AdjustX(2 );
                }
                bool bTop = ( aMousePos.X() >= aStartPos.X()-3 && aMousePos.X() <= aStartPos.X()+1 &&
                              aMousePos.Y() >= aStartPos.Y()-3 && aMousePos.Y() <= aStartPos.Y()+1 );
                bool bBottom = ( aMousePos.X() >= aEndPos.X()-3 && aMousePos.X() <= aEndPos.X()+1 &&
                                 aMousePos.Y() >= aEndPos.Y()-3 && aMousePos.Y() <= aEndPos.Y()+1 );
                if ( bTop || bBottom )
                {
                    SetPointer( PointerStyle::Cross );
                    if (bAction)
                    {
                        ScFillMode nMode = bTop ? ScFillMode::EMBED_LT : ScFillMode::EMBED_RB;
                        mrViewData.SetDragMode(
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), nMode );
                    }
                    bNewPointer = true;
                }
            }
        }
    }

    if (!bNewPointer && bAction)
    {
        mrViewData.ResetFillMode();
    }

    return bNewPointer;
}

void ScGridWindow::LogicMouseButtonDown(const MouseEvent& rMouseEvent)
{
    MouseButtonDown(rMouseEvent);
}

void ScGridWindow::LogicMouseButtonUp(const MouseEvent& rMouseEvent)
{
    MouseButtonUp(rMouseEvent);
}

void ScGridWindow::LogicMouseMove(const MouseEvent& rMouseEvent)
{
    MouseMove(rMouseEvent);
}

void ScGridWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (SfxLokHelper::getDeviceFormFactor() == LOKDeviceFormFactor::MOBILE)
    {
        ScViewFunc* pView = mrViewData.GetView();
        ScTabViewShell* pViewShell = mrViewData.GetViewShell();
        bool bRefMode = pViewShell && pViewShell->IsRefInputMode();

        Point aPos(rMEvt.GetPosPixel());
        SCCOL nPosX;
        SCROW nPosY;
        mrViewData.GetPosFromPixel(aPos.X(), aPos.Y(), eWhich, nPosX, nPosY);

        if (bRefMode && pView->GetFunctionSet().CheckRefBounds(nPosX, nPosY))
            return;
    }

    nNestedButtonState = ScNestedButtonState::Down;

    MouseEventState aState;
    HandleMouseButtonDown(rMEvt, aState);
    if (aState.mbActivatePart)
        mrViewData.GetView()->ActivatePart(eWhich);

    if ( nNestedButtonState == ScNestedButtonState::Up )
    {
        // #i41690# If an object is deactivated from MouseButtonDown, it might reschedule,
        // so MouseButtonUp comes before the MouseButtonDown call is finished. In this case,
        // simulate another MouseButtonUp call, so the selection state is consistent.

        nButtonDown = rMEvt.GetButtons();
        FakeButtonUp();

        if ( IsTracking() )
            EndTracking();      // normally done in VCL as part of MouseButtonUp handling
    }
    nNestedButtonState = ScNestedButtonState::NONE;
}

bool ScGridWindow::IsCellCoveredByText(SCCOL nPosX, SCROW nPosY, SCTAB nTab, SCCOL &rTextStartPosX)
{
    ScDocument& rDoc = mrViewData.GetDocument();

    // find the first non-empty cell (this, or to the left)
    SCCOL nNonEmptyX = nPosX;
    for (; nNonEmptyX >= 0; --nNonEmptyX)
    {
        ScRefCellValue aCell(rDoc, ScAddress(nNonEmptyX, nPosY, nTab));
        if (!aCell.isEmpty())
            break;
    }

    // the initial cell already contains text
    if (nNonEmptyX == nPosX)
    {
        rTextStartPosX = nNonEmptyX;
        return true;
    }

    // to the left, there is no cell that would contain (potentially
    // overrunning) text
    if (nNonEmptyX < 0 || rDoc.HasAttrib(nNonEmptyX, nPosY, nTab, nPosX, nPosY, nTab, HasAttrFlags::Merged | HasAttrFlags::Overlapped))
        return false;

    double nPPTX = mrViewData.GetPPTX();
    double nPPTY = mrViewData.GetPPTY();

    ScTableInfo aTabInfo;
    rDoc.FillInfo(aTabInfo, 0, nPosY, nPosX, nPosY, nTab, nPPTX, nPPTY, false, false);

    Fraction aZoomX = mrViewData.GetZoomX();
    Fraction aZoomY = mrViewData.GetZoomY();
    ScOutputData aOutputData(GetOutDev(), OUTTYPE_WINDOW, aTabInfo, &rDoc, nTab,
            0, 0, 0, nPosY, nPosX, nPosY, nPPTX, nPPTY,
            &aZoomX, &aZoomY);

    MapMode aCurrentMapMode(GetMapMode());
    SetMapMode(MapMode(MapUnit::MapPixel));

    // obtain the bounding box of the text in first non-empty cell
    // to the left
    tools::Rectangle aRect(aOutputData.LayoutStrings(false, false, ScAddress(nNonEmptyX, nPosY, nTab)));

    SetMapMode(aCurrentMapMode);

    // the text does not overrun from the cell
    if (aRect.IsEmpty())
        return false;

    SCCOL nTextEndX;
    SCROW nTextEndY;

    // test the rightmost position of the text bounding box
    tools::Long nMiddle = (aRect.Top() + aRect.Bottom()) / 2;
    mrViewData.GetPosFromPixel(aRect.Right(), nMiddle, eWhich, nTextEndX, nTextEndY);
    if (nTextEndX >= nPosX)
    {
        rTextStartPosX = nNonEmptyX;
        return true;
    }

    return false;
}

void ScGridWindow::HandleMouseButtonDown( const MouseEvent& rMEvt, MouseEventState& rState )
{
    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the event.
    // Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the context menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    SfxViewShell* pViewSh = mrViewData.GetViewShell();
    SfxInPlaceClient* pClient = pViewSh->GetIPClient();
    if ( pClient &&
         pClient->IsObjectInPlaceActive() &&
         vcl::IsInPopupMenuExecute() )
        return;

    aCurMousePos = rMEvt.GetPosPixel();

    // Filter popup is ended with its own mouse click, not when clicking into the Grid Window,
    // so the following query is no longer necessary:
    ClickExtern();  // deletes FilterBox when available

    HideNoteMarker();

    bEEMouse = false;

    ScModule* pScMod = SC_MOD();
    if (pScMod->IsModalMode(mrViewData.GetSfxDocShell()))
        return;

    pScActiveViewShell = mrViewData.GetViewShell();         // if left is clicked
    nScClickMouseModifier = rMEvt.GetModifier();            // to always catch a control click

    bool bDetective = mrViewData.GetViewShell()->IsAuditShell();
    bool bRefMode = mrViewData.IsRefMode();                 // Start reference
    bool bFormulaMode = pScMod->IsFormulaMode();            // next click -> reference
    bool bEditMode = mrViewData.HasEditView(eWhich);        // also in Mode==SC_INPUT_TYPE
    bool bDouble = (rMEvt.GetClicks() == 2);
    ScDocument& rDoc = mrViewData.GetDocument();
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();

    // DeactivateIP does only happen when MarkListHasChanged

    // An error message can show up during GrabFocus call
    // (for instance when renaming tables per sheet title)

    if ( !nButtonDown || !bDouble )             // single (first) click is always valid
        nButtonDown = rMEvt.GetButtons();       // set nButtonDown first, so StopMarking works

    // special handling of empty cells with tiled rendering
    if (bIsTiledRendering)
    {
        Point aPos(rMEvt.GetPosPixel());
        SCCOL nPosX, nNonEmptyX(0);
        SCROW nPosY;
        SCTAB nTab = mrViewData.GetTabNo();
        mrViewData.GetPosFromPixel(aPos.X(), aPos.Y(), eWhich, nPosX, nPosY);

        ScRefCellValue aCell(rDoc, ScAddress(nPosX, nPosY, nTab));
        bool bIsEmpty = aCell.isEmpty();
        bool bIsCoveredByText = bIsEmpty && IsCellCoveredByText(nPosX, nPosY, nTab, nNonEmptyX);

        if (bIsCoveredByText)
        {
            // if there's any text flowing to this cell, activate the
            // editengine, so that the text actually gets the events
            if (bDouble)
            {
                ScViewFunc* pView = mrViewData.GetView();

                pView->SetCursor(nNonEmptyX, nPosY);
                SC_MOD()->SetInputMode(SC_INPUT_TABLE);

                bEditMode = mrViewData.HasEditView(eWhich);
                assert(bEditMode);

                // synthesize the 1st click
                EditView* pEditView = mrViewData.GetEditView(eWhich);
                MouseEvent aEditEvt(rMEvt.GetPosPixel(), 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0);
                pEditView->MouseButtonDown(aEditEvt);
                pEditView->MouseButtonUp(aEditEvt);
            }
        }
        else if (bIsEmpty && bEditMode && bDouble)
        {
            // double-click in an empty cell: the entire cell is selected
            SetCellSelectionPixel(LOK_SETTEXTSELECTION_START, aPos.X(), aPos.Y());
            SetCellSelectionPixel(LOK_SETTEXTSELECTION_END, aPos.X(), aPos.Y());
            return;
        }
    }

    if ( ( bEditMode && mrViewData.GetActivePart() == eWhich ) || !bFormulaMode )
        GrabFocus();

    // #i31846# need to cancel a double click if the first click has set the "ignore" state,
    // but a single (first) click is always valid
    if ( nMouseStatus == SC_GM_IGNORE && bDouble )
    {
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    if ( bDetective )               // Detectiv fill mode
    {
        if ( rMEvt.IsLeft() && !rMEvt.GetModifier() )
        {
            Point   aPos = rMEvt.GetPosPixel();
            SCCOL  nPosX;
            SCROW  nPosY;
            mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

            SfxInt16Item aPosXItem( SID_RANGE_COL, nPosX );
            SfxInt32Item aPosYItem( SID_RANGE_ROW, nPosY );
            mrViewData.GetDispatcher().ExecuteList(SID_FILL_SELECT,
                    SfxCallMode::SLOT | SfxCallMode::RECORD,
                    { &aPosXItem, &aPosYItem });

        }
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    if (!bDouble)
        nMouseStatus = SC_GM_NONE;

    rState.mbActivatePart = !bFormulaMode; // Don't activate when in formula mode.

    if (bFormulaMode)
    {
        ScViewSelectionEngine* pSelEng = mrViewData.GetView()->GetSelEngine();
        pSelEng->SetWindow(this);
        pSelEng->SetWhich(eWhich);
        pSelEng->SetVisibleArea( tools::Rectangle(Point(), GetOutputSizePixel()) );
    }

    if (bEditMode && (mrViewData.GetRefTabNo() == mrViewData.GetTabNo()))
    {
        Point   aPos = rMEvt.GetPosPixel();
        SCCOL  nPosX;
        SCROW  nPosY;
        mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        EditView*   pEditView;
        SCCOL       nEditCol;
        SCROW       nEditRow;
        mrViewData.GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        SCCOL nEndCol = mrViewData.GetEditEndCol();
        SCROW nEndRow = mrViewData.GetEditEndRow();

        if ( nPosX >= nEditCol && nPosX <= nEndCol &&
             nPosY >= nEditRow && nPosY <= nEndRow )
        {
            // when clicking in the table EditView, always reset the focus
            if (bFormulaMode)   // otherwise this has already happen above
                GrabFocus();

            pScMod->SetInputMode( SC_INPUT_TABLE );
            bEEMouse = true;
            pEditView->MouseButtonDown( rMEvt );
            return;
        }
    }

    if (pScMod->GetIsWaterCan())
    {
        //!     what's up with the Mac ???
        if ( rMEvt.GetModifier() + rMEvt.GetButtons() == MOUSE_RIGHT )
        {
            nMouseStatus = SC_GM_WATERUNDO;
            return;
        }
    }

    // Order that matches the displayed Cursor:
    // RangeFinder, AutoFill, PageBreak, Drawing

    RfCorner rCorner = NONE;
    bool bFound = HitRangeFinder(rMEvt.GetPosPixel(), rCorner, &nRFIndex, &nRFAddX, &nRFAddY);
    bRFSize = (rCorner != NONE);
    aRFSelectedCorned = rCorner;

    if (bFound)
    {
        bRFMouse = true;        // the other variables are initialized above

        rState.mbActivatePart = true; // always activate ?
        StartTracking();
        return;
    }

    bool bCrossPointer = TestMouse( rMEvt, true );
    if ( bCrossPointer )
    {
        if ( bDouble )
            mrViewData.GetView()->FillCrossDblClick();
        else
            pScMod->InputEnterHandler();                                // Autofill etc.
    }

    if ( !bCrossPointer )
    {
        nPagebreakMouse = HitPageBreak( rMEvt.GetPosPixel(), &aPagebreakSource,
                                            &nPagebreakBreak, &nPagebreakPrev );
        if (nPagebreakMouse)
        {
            bPagebreakDrawn = false;
            StartTracking();
            PagebreakMove( rMEvt, false );
            return;
        }
    }

    // in the tiled rendering case, single clicks into drawing objects take
    // precedence over bEditMode
    if (((!bFormulaMode && !bEditMode) || bIsTiledRendering) && rMEvt.IsLeft())
    {
        if ( !bCrossPointer && DrawMouseButtonDown(rMEvt) )
        {
            return;
        }

        mrViewData.GetViewShell()->SetDrawShell( false );               // no Draw-object selected

        // TestMouse has already happened above
    }

    Point aPos = rMEvt.GetPosPixel();
    SCCOL nPosX;
    SCROW nPosY;
    mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    SCTAB nTab = mrViewData.GetTabNo();

    // FIXME: this is to limit the number of rows handled in the Online
    // to 1000; this will be removed again when the performance
    // bottlenecks are sorted out
    if ( comphelper::LibreOfficeKit::isActive() && nPosY > MAXTILEDROW - 1 )
    {
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    // Auto filter / pivot table / data select popup.  This shouldn't activate the part.

    if ( !bDouble && !bFormulaMode && rMEvt.IsLeft() )
    {
        SCCOL nRealPosX;
        SCROW nRealPosY;
        mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nRealPosX, nRealPosY, false );//the real row/col

        // show in the merged cells the filter of the first cell (nPosX instead of nRealPosX)
        const ScMergeFlagAttr* pRealPosAttr = rDoc.GetAttr(nPosX, nRealPosY, nTab, ATTR_MERGE_FLAG);
        if( pRealPosAttr->HasAutoFilter() )
        {
            SC_MOD()->InputEnterHandler();
            if (DoAutoFilterButton(nPosX, nRealPosY, rMEvt))
                return;
        }

        const ScMergeFlagAttr* pAttr = rDoc.GetAttr(nPosX, nPosY, nTab, ATTR_MERGE_FLAG);
        if (pAttr->HasAutoFilter())
        {
            if (DoAutoFilterButton(nPosX, nPosY, rMEvt))
            {
                rState.mbActivatePart = false;
                return;
            }
        }

        if (pAttr->HasPivotButton() || pAttr->HasPivotPopupButton())
        {
            DoPushPivotButton(nPosX, nPosY, rMEvt, pAttr->HasPivotButton(), pAttr->HasPivotPopupButton());
            rState.mbActivatePart = false;
            return;
        }

        //  List Validity drop-down button

        if ( bListValButton )
        {
            tools::Rectangle aButtonRect = GetListValButtonRect( aListValPos );
            if ( aButtonRect.Contains( aPos ) )
            {
                // tdf#125917 typically we have the mouse captured already, except if are editing the cell.
                // Ensure its captured before the menu is launched even in the cell editing case
                CaptureMouse();

                LaunchDataSelectMenu( aListValPos.Col(), aListValPos.Row() );

                nMouseStatus = SC_GM_FILTER;    // not set in DoAutoFilterMenue for bDataSelect
                rState.mbActivatePart = false;
                return;
            }
        }
    }

            //      scenario selection

    ScRange aScenRange;
    if ( rMEvt.IsLeft() && HasScenarioButton( aPos, aScenRange ) )
    {
        CaptureMouse();

        DoScenarioMenu( aScenRange );

        // Scenario selection comes from MouseButtonDown:
        // The next MouseMove on the FilterBox is like a ButtonDown
        nMouseStatus = SC_GM_FILTER;
        return;
    }

            //      double click started ?

    // StopMarking can be called from DrawMouseButtonDown

    if ( nMouseStatus != SC_GM_IGNORE && !bRefMode )
    {
        if ( bDouble && !bCrossPointer )
        {
            if (nMouseStatus == SC_GM_TABDOWN)
                nMouseStatus = SC_GM_DBLDOWN;
        }
        else
            nMouseStatus = SC_GM_TABDOWN;
    }

            //      links in the edit cell

    bool bAlt = rMEvt.IsMod2();
    if ( !bAlt && rMEvt.IsLeft() && ScGlobal::ShouldOpenURL() &&
            GetEditUrl(rMEvt.GetPosPixel()) )           // click on link: do not move cursor
    {
        SetPointer( PointerStyle::RefHand );
        nMouseStatus = SC_GM_URLDOWN;                   // also only execute when ButtonUp
        return;
    }

            //      Gridwin - Selection Engine

    if ( !rMEvt.IsLeft() )
        return;

    ScViewSelectionEngine* pSelEng = mrViewData.GetView()->GetSelEngine();
    pSelEng->SetWindow(this);
    pSelEng->SetWhich(eWhich);
    pSelEng->SetVisibleArea( tools::Rectangle(Point(), GetOutputSizePixel()) );

    //  SelMouseButtonDown on the View is still setting the bMoveIsShift flag
    if ( mrViewData.GetView()->SelMouseButtonDown( rMEvt ) )
    {
        if (IsMouseCaptured())
        {
            //  Tracking instead of CaptureMouse, so it can be canceled cleanly
            //! Someday SelectionEngine should call StartTracking on its own!?!
            ReleaseMouse();
            StartTracking();
        }
        mrViewData.GetMarkData().SetMarking(true);
        return;
    }
}

void ScGridWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    aCurMousePos = rMEvt.GetPosPixel();
    ScDocument& rDoc = mrViewData.GetDocument();
    ScMarkData& rMark = mrViewData.GetMarkData();
    // #i41690# detect a MouseButtonUp call from within MouseButtonDown
    // (possible through Reschedule from storing an OLE object that is deselected)

    if ( nNestedButtonState == ScNestedButtonState::Down )
        nNestedButtonState = ScNestedButtonState::Up;

    if (nButtonDown != rMEvt.GetButtons())
        nMouseStatus = SC_GM_IGNORE;            // reset and return

    nButtonDown = 0;

    if (nMouseStatus == SC_GM_IGNORE)
    {
        nMouseStatus = SC_GM_NONE;
                                        // Selection engine: cancel selection
        mrViewData.GetView()->GetSelEngine()->Reset();
        rMark.SetMarking(false);
        if (mrViewData.IsAnyFillMode())
        {
            mrViewData.GetView()->StopRefMode();
            mrViewData.ResetFillMode();
        }
        StopMarking();
        DrawEndAction();                // cancel selection/moving in drawing layer
        ReleaseMouse();
        return;
    }

    if (nMouseStatus == SC_GM_FILTER)
    {
        nMouseStatus = SC_GM_NONE;
        ReleaseMouse();
        return;                         // nothing more should happen here
    }

    ScModule* pScMod = SC_MOD();
    if (pScMod->IsModalMode(mrViewData.GetSfxDocShell()))
        return;

    SfxBindings& rBindings = mrViewData.GetBindings();
    if (bEEMouse && mrViewData.HasEditView( eWhich ))
    {
        EditView*   pEditView;
        SCCOL       nEditCol;
        SCROW       nEditRow;
        mrViewData.GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        pEditView->MouseButtonUp( rMEvt );

        if ( rMEvt.IsMiddle() &&
                 GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection )
        {
            //  EditView may have pasted from selection
            pScMod->InputChanged( pEditView );
        }
        else
            pScMod->InputSelection( pEditView );            // parentheses etc.

        mrViewData.GetView()->InvalidateAttribs();
        rBindings.Invalidate( SID_HYPERLINK_GETLINK );
        bEEMouse = false;
        return;
    }

    if (bDPMouse)
    {
        DPMouseButtonUp( rMEvt );       // resets bDPMouse
        return;
    }

    if (bRFMouse)
    {
        RFMouseMove( rMEvt, true );     // Again the proper range
        bRFMouse = false;
        SetPointer( PointerStyle::Arrow );
        ReleaseMouse();
        return;
    }

    if (nPagebreakMouse)
    {
        PagebreakMove( rMEvt, true );
        nPagebreakMouse = SC_PD_NONE;
        SetPointer( PointerStyle::Arrow );
        ReleaseMouse();
        return;
    }

    if (nMouseStatus == SC_GM_WATERUNDO)    // Undo in format paintbrush mode
    {
        SfxUndoManager* pMgr = mrViewData.GetDocShell()->GetUndoManager();
        if ( pMgr->GetUndoActionCount() && dynamic_cast<ScUndoSelectionStyle*>(pMgr->GetUndoAction()) )
            pMgr->Undo();
        return;
    }

    if (DrawMouseButtonUp(rMEvt))       // includes format paint brush handling for drawing objects
    {
        ScTabViewShell* pViewShell = mrViewData.GetViewShell();
        SfxBindings& rFrmBindings=pViewShell->GetViewFrame()->GetBindings();
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_WIDTH);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_HEIGHT);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_POS_X);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_POS_Y);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_ANGLE);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_ROT_X);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_ROT_Y);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_AUTOWIDTH);
        rFrmBindings.Invalidate(SID_ATTR_TRANSFORM_AUTOHEIGHT);
        return;
    }

    rMark.SetMarking(false);

    SetPointer( mrViewData.IsThemedCursor() ? PointerStyle::FatCross : PointerStyle::Arrow );

    if (mrViewData.IsFillMode() ||
        ( mrViewData.GetFillMode() == ScFillMode::MATRIX && rMEvt.IsMod1() ))
    {
        nScFillModeMouseModifier = rMEvt.GetModifier();
        SCCOL nStartCol;
        SCROW nStartRow;
        SCCOL nEndCol;
        SCROW nEndRow;
        mrViewData.GetFillData( nStartCol, nStartRow, nEndCol, nEndRow );
        ScRange aDelRange;
        bool bIsDel = mrViewData.GetDelMark( aDelRange );

        ScViewFunc* pView = mrViewData.GetView();
        pView->StopRefMode();
        mrViewData.ResetFillMode();
        pView->GetFunctionSet().SetAnchorFlag( false );    // #i5819# don't use AutoFill anchor flag for selection

        if ( bIsDel )
        {
            pView->MarkRange( aDelRange, false );
            pView->DeleteContents( InsertDeleteFlags::CONTENTS );
            SCTAB nTab = mrViewData.GetTabNo();
            ScRange aBlockRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
            if ( aBlockRange != aDelRange )
            {
                if ( aDelRange.aStart.Row() == nStartRow )
                    aBlockRange.aEnd.SetCol( aDelRange.aStart.Col() - 1 );
                else
                    aBlockRange.aEnd.SetRow( aDelRange.aStart.Row() - 1 );
                pView->MarkRange( aBlockRange, false );
            }
        }
        else
            mrViewData.GetDispatcher().Execute( FID_FILL_AUTO, SfxCallMode::SLOT | SfxCallMode::RECORD );
    }
    else if (mrViewData.GetFillMode() == ScFillMode::MATRIX)
    {
        SCTAB nTab = mrViewData.GetTabNo();
        SCCOL nStartCol;
        SCROW nStartRow;
        SCCOL nEndCol;
        SCROW nEndRow;
        mrViewData.GetFillData( nStartCol, nStartRow, nEndCol, nEndRow );
        ScRange aBlockRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
        SCCOL nFillCol = mrViewData.GetRefEndX();
        SCROW nFillRow = mrViewData.GetRefEndY();
        ScAddress aEndPos( nFillCol, nFillRow, nTab );

        ScTabView* pView = mrViewData.GetView();
        pView->StopRefMode();
        mrViewData.ResetFillMode();
        pView->GetFunctionSet().SetAnchorFlag( false );

        if ( aEndPos != aBlockRange.aEnd )
        {
            mrViewData.GetDocShell()->GetDocFunc().ResizeMatrix( aBlockRange, aEndPos );
            mrViewData.GetView()->MarkRange( ScRange( aBlockRange.aStart, aEndPos ) );
        }
    }
    else if (mrViewData.IsAnyFillMode())
    {
                                                // Embedded area has been changed
        ScTabView* pView = mrViewData.GetView();
        pView->StopRefMode();
        mrViewData.ResetFillMode();
        pView->GetFunctionSet().SetAnchorFlag( false );
        mrViewData.GetDocShell()->UpdateOle(mrViewData);
    }

    bool bRefMode = mrViewData.IsRefMode();
    if (bRefMode)
        pScMod->EndReference();

        // Format paintbrush mode (Switch)

    if (pScMod->GetIsWaterCan())
    {
        // Check on undo already done above

        ScStyleSheetPool* pStylePool = mrViewData.GetDocument().
                                            GetStyleSheetPool();
        if ( pStylePool )
        {
            SfxStyleSheet* pStyleSheet = static_cast<SfxStyleSheet*>(
                                         pStylePool->GetActualStyleSheet());

            if ( pStyleSheet )
            {
                SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                switch ( eFamily )
                {
                    case SfxStyleFamily::Para:
                        mrViewData.GetView()->SetStyleSheetToMarked( pStyleSheet );
                        mrViewData.GetView()->DoneBlockMode();
                        break;

                    case SfxStyleFamily::Page:
                        mrViewData.GetDocument().SetPageStyle( mrViewData.GetTabNo(),
                                                               pStyleSheet->GetName() );

                        ScPrintFunc( mrViewData.GetDocShell(),
                                     mrViewData.GetViewShell()->GetPrinter(true),
                                     mrViewData.GetTabNo() ).UpdatePages();

                        rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                        break;

                    default:
                        break;
                }
            }
        }
    }

    ScDBFunc* pView = mrViewData.GetView();
    ScDocument* pBrushDoc = pView->GetBrushDocument();
    if ( pBrushDoc )
    {
        pView->PasteFromClip( InsertDeleteFlags::ATTRIB, pBrushDoc );
        if ( !pView->IsPaintBrushLocked() )
            pView->ResetBrushDocument();            // invalidates pBrushDoc pointer
    }

    Point aPos = rMEvt.GetPosPixel();
    SCCOL nPosX;
    SCROW nPosY;
    SCTAB nTab = mrViewData.GetTabNo();
    mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    ScDPObject* pDPObj  = rDoc.GetDPAtCursor( nPosX, nPosY, nTab );

    bool bInDataPilotTable = (pDPObj != nullptr);

    // double click (only left button)
    // in the tiled rendering case, single click works this way too

    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    bool bDouble = ( rMEvt.GetClicks() == 2 && rMEvt.IsLeft() );
    if ((bDouble || (bIsTiledRendering && !bInDataPilotTable))
            && !bRefMode
            && (nMouseStatus == SC_GM_DBLDOWN || (bIsTiledRendering && nMouseStatus != SC_GM_URLDOWN))
            && !pScMod->IsRefDialogOpen())
    {
        //  data pilot table
        if ( pDPObj && pDPObj->GetSaveData()->GetDrillDown() )
        {
            ScAddress aCellPos( nPosX, nPosY, mrViewData.GetTabNo() );

            // Check for header drill-down first.
            sheet::DataPilotTableHeaderData aData;
            pDPObj->GetHeaderPositionData(aCellPos, aData);

            if ( ( aData.Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                 ! ( aData.Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                css::sheet::DataPilotFieldOrientation nDummy;
                if ( pView->HasSelectionForDrillDown( nDummy ) )
                {
                    // execute slot to show dialog
                    mrViewData.GetDispatcher().Execute( SID_OUTLINE_SHOW, SfxCallMode::SLOT | SfxCallMode::RECORD );
                }
                else
                {
                    // toggle single entry
                    ScDPObject aNewObj( *pDPObj );
                    pDPObj->ToggleDetails( aData, &aNewObj );
                    ScDBDocFunc aFunc( *mrViewData.GetDocShell() );
                    aFunc.DataPilotUpdate( pDPObj, &aNewObj, true, false );
                    mrViewData.GetView()->CursorPosChanged();       // shells may be switched
                }
            }
            else
            {
                // Check if the data area is double-clicked.

                Sequence<sheet::DataPilotFieldFilter> aFilters;
                if ( pDPObj->GetDataFieldPositionData(aCellPos, aFilters) )
                    mrViewData.GetView()->ShowDataPilotSourceData( *pDPObj, aFilters );
            }

            return;
        }

        // Check for cell protection attribute.
        const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        bool bEditAllowed = true;
        if ( pProtect && pProtect->isProtected() )
        {
            bool bCellProtected = rDoc.HasAttrib(nPosX, nPosY, nTab, nPosX, nPosY, nTab, HasAttrFlags::Protected);
            bool bSkipProtected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
            bool bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);

            if ( bSkipProtected && bSkipUnprotected )
                bEditAllowed = false;
            else if ( (bCellProtected && bSkipProtected) || (!bCellProtected && bSkipUnprotected) )
                bEditAllowed = false;
        }

        // We don't want to activate the edit view for a single click in tiled rendering
        // (but we should probably keep the same behaviour for double clicks).
        if ( bEditAllowed && (!bIsTiledRendering || bDouble) )
        {
            // don't forward the event to an empty cell, causes deselection in
            // case we used the double-click to select the empty cell
            if (bIsTiledRendering && bDouble)
            {
                ScRefCellValue aCell(mrViewData.GetDocument(), ScAddress(nPosX, nPosY, nTab));
                if (aCell.isEmpty())
                    return;
            }

            //  edit cell contents
            mrViewData.GetViewShell()->UpdateInputHandler();
            pScMod->SetInputMode( SC_INPUT_TABLE );
            if (mrViewData.HasEditView(eWhich))
            {
                // Set text cursor where clicked
                EditView* pEditView = mrViewData.GetEditView( eWhich );
                MouseEvent aEditEvt( rMEvt.GetPosPixel(), 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0 );
                pEditView->MouseButtonDown( aEditEvt );
                pEditView->MouseButtonUp( aEditEvt );
            }
        }

        if ( bIsTiledRendering && rMEvt.IsLeft() && mrViewData.GetView()->GetSelEngine()->SelMouseButtonUp( rMEvt ) )
        {
            mrViewData.GetView()->SelectionChanged();
        }

        if ( bDouble )
            return;
    }

            //      Links in edit cells

    bool bAlt = rMEvt.IsMod2();
    if ( !bAlt && !bRefMode && !bDouble && nMouseStatus == SC_GM_URLDOWN )
    {
        //  Only execute on ButtonUp, if ButtonDown also was done on a URL

        OUString aName, aUrl, aTarget;
        if ( GetEditUrl( rMEvt.GetPosPixel(), &aName, &aUrl, &aTarget ) )
        {
            nMouseStatus = SC_GM_NONE;              // Ignore double-click
            bool isTiledRendering = comphelper::LibreOfficeKit::isActive();
            // ScGlobal::OpenURL() only understands Calc A1 style syntax.
            // Convert it to Calc A1 before calling OpenURL().
            if (rDoc.GetAddressConvention() == formula::FormulaGrammar::CONV_OOO)
            {
                if (aUrl.startsWith("#")) {
                        ScGlobal::OpenURL(aUrl, aTarget, isTiledRendering);
                        return;
                }
                // On a mobile device view there is no ctrl+click and for hyperlink popup
                // the cell coordinates must be sent along with click position for elegance
                ScTabViewShell* pViewShell = mrViewData.GetViewShell();
                if (isTiledRendering && pViewShell &&
                    (pViewShell->isLOKMobilePhone() || pViewShell->isLOKTablet()))
                {
                    aPos = rMEvt.GetPosPixel();
                    mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
                    OString aCursor = pViewShell->GetViewData().describeCellCursorAt(nPosX, nPosY);
                    double fPPTX = pViewShell->GetViewData().GetPPTX();
                    int mouseX = aPos.X() / fPPTX;
                    OString aMsg(aUrl.toUtf8() + " coordinates: " + aCursor + ", " + OString::number(mouseX));
                    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_HYPERLINK_CLICKED, aMsg.getStr());
                } else
                    ScGlobal::OpenURL(aUrl, aTarget);
            }
            else
            {
                ScAddress aTempAddr;
                ScAddress::ExternalInfo aExtInfo;
                ScRefFlags nRes = aTempAddr.Parse(aUrl, rDoc, rDoc.GetAddressConvention(), &aExtInfo);
                if (!(nRes & ScRefFlags::VALID))
                {
                    // Not a reference string. Pass it through unmodified.
                    ScGlobal::OpenURL(aUrl, aTarget);
                    return;
                }

                OUStringBuffer aBuf;
                if (aExtInfo.mbExternal)
                {
                    // External reference.
                    ScExternalRefManager* pRefMgr = rDoc.GetExternalRefManager();
                    const OUString* pStr = pRefMgr->getExternalFileName(aExtInfo.mnFileId);
                    if (pStr)
                        aBuf.append(*pStr);

                    aBuf.append('#');
                    aBuf.append(aExtInfo.maTabName);
                    aBuf.append('.');
                    OUString aRefCalcA1(aTempAddr.Format(ScRefFlags::ADDR_ABS, nullptr, formula::FormulaGrammar::CONV_OOO));
                    aBuf.append(aRefCalcA1);
                    ScGlobal::OpenURL(aBuf.makeStringAndClear(), aTarget);
                }
                else
                {
                    // Internal reference.
                    aBuf.append('#');
                    OUString aUrlCalcA1(aTempAddr.Format(ScRefFlags::ADDR_ABS_3D, &rDoc, formula::FormulaGrammar::CONV_OOO));
                    aBuf.append(aUrlCalcA1);
                    ScGlobal::OpenURL(aBuf.makeStringAndClear(), aTarget, isTiledRendering);
                }
            }

            // fire worksheet_followhyperlink event
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = rDoc.GetVbaEventProcessor();
            if( xVbaEvents.is() ) try
            {
                aPos = rMEvt.GetPosPixel();
                nTab = mrViewData.GetTabNo();
                mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
                OUString sURL;
                ScRefCellValue aCell;
                if (lcl_GetHyperlinkCell(rDoc, nPosX, nPosY, nTab, aCell, sURL))
                {
                    ScAddress aCellPos( nPosX, nPosY, nTab );
                    uno::Reference< table::XCell > xCell( new ScCellObj( mrViewData.GetDocShell(), aCellPos ) );
                    uno::Sequence< uno::Any > aArgs{ uno::Any(xCell) };
                    xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKSHEET_FOLLOWHYPERLINK, aArgs );
                }
            }
            catch( uno::Exception& )
            {
            }

            return;
        }
    }

            //      Gridwin - SelectionEngine

    //  SelMouseButtonDown is called only for left button, but SelMouseButtonUp would return
    //  sal_True for any call, so IsLeft must be checked here, too.

    if ( !(rMEvt.IsLeft() && mrViewData.GetView()->GetSelEngine()->SelMouseButtonUp( rMEvt )) )
        return;

    mrViewData.GetView()->SelectionChanged();

    SfxDispatcher* pDisp = mrViewData.GetViewShell()->GetDispatcher();
    bool bFormulaMode = pScMod->IsFormulaMode();
    OSL_ENSURE( pDisp || bFormulaMode, "Cursor moved on inactive View ?" );

    //  #i14927# execute SID_CURRENTCELL (for macro recording) only if there is no
    //  multiple selection, so the argument string completely describes the selection,
    //  and executing the slot won't change the existing selection (executing the slot
    //  here and from a recorded macro is treated equally)
    if ( pDisp && !bFormulaMode && !rMark.IsMultiMarked() )
    {
        OUString aAddr;                               // CurrentCell
        if( rMark.IsMarked() )
        {
            ScRange aScRange;
            rMark.GetMarkArea( aScRange );
            aAddr = aScRange.Format(rDoc, ScRefFlags::RANGE_ABS);
            if ( aScRange.aStart == aScRange.aEnd )
            {
                //  make sure there is a range selection string even for a single cell
                aAddr += ":" + aAddr;
            }

            //! SID_MARKAREA does not exist anymore ???
            //! What happens when selecting with the cursor ???
        }
        else                                        // only move cursor
        {
            ScAddress aScAddress( mrViewData.GetCurX(), mrViewData.GetCurY(), 0 );
            aAddr = aScAddress.Format(ScRefFlags::ADDR_ABS);
        }

        SfxStringItem aPosItem( SID_CURRENTCELL, aAddr );
        // We don't want to align to the cursor position because if the
        // cell cursor isn't visible after making selection, it would jump
        // back to the origin of the selection where the cell cursor is.
        SfxBoolItem aAlignCursorItem( FN_PARAM_2, false );
        pDisp->ExecuteList(SID_CURRENTCELL,
                SfxCallMode::SLOT | SfxCallMode::RECORD,
                { &aPosItem, &aAlignCursorItem });

        mrViewData.GetView()->InvalidateAttribs();

    }
    mrViewData.GetViewShell()->SelectionChanged();

    return;
}

void ScGridWindow::FakeButtonUp()
{
    if ( nButtonDown )
    {
        MouseEvent aEvent( aCurMousePos );      // nButtons = 0 -> ignore
        MouseButtonUp( aEvent );
    }
}

void ScGridWindow::MouseMove( const MouseEvent& rMEvt )
{
    aCurMousePos = rMEvt.GetPosPixel();

    if (rMEvt.IsLeaveWindow() && mpNoteMarker && !mpNoteMarker->IsByKeyboard())
        HideNoteMarker();

    ScModule* pScMod = SC_MOD();
    if (pScMod->IsModalMode(mrViewData.GetSfxDocShell()))
        return;

    // If the Drag&Drop is started in the edit mode then sadly nothing else is kept
    if (bEEMouse && nButtonDown && !rMEvt.GetButtons())
    {
        bEEMouse = false;
        nButtonDown = 0;
        nMouseStatus = SC_GM_NONE;
        return;
    }

    if (nMouseStatus == SC_GM_IGNORE)
        return;

    if (nMouseStatus == SC_GM_WATERUNDO)    // Undo in format paintbrush mode -> only what for Up
        return;

    if ( mrViewData.GetViewShell()->IsAuditShell() )        // Detective Fill Mode
    {
        SetPointer( PointerStyle::Fill );
        return;
    }

    bool bFormulaMode = pScMod->IsFormulaMode();            // next click -> reference

    if (bEEMouse && mrViewData.HasEditView( eWhich ))
    {
        EditView*   pEditView;
        SCCOL       nEditCol;
        SCROW       nEditRow;
        mrViewData.GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        pEditView->MouseMove( rMEvt );
        return;
    }

    if (bDPMouse)
    {
        DPMouseMove( rMEvt );
        return;
    }

    if (bRFMouse)
    {
        RFMouseMove( rMEvt, false );
        return;
    }

    if (nPagebreakMouse)
    {
        PagebreakMove( rMEvt, false );
        return;
    }

    // Show other mouse pointer?

    bool bEditMode = mrViewData.HasEditView(eWhich);

    //! Test if refMode dragging !!!
    if ( bEditMode && (mrViewData.GetRefTabNo() == mrViewData.GetTabNo()) )
    {
        Point   aPos = rMEvt.GetPosPixel();
        SCCOL  nPosX;
        SCROW  nPosY;
        mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        EditView*   pEditView;
        SCCOL       nEditCol;
        SCROW       nEditRow;
        mrViewData.GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        SCCOL nEndCol = mrViewData.GetEditEndCol();
        SCROW nEndRow = mrViewData.GetEditEndRow();

        if ( nPosX >= nEditCol && nPosX <= nEndCol &&
             nPosY >= nEditRow && nPosY <= nEndRow )
        {
            if ( !pEditView )
            {
                SetPointer( PointerStyle::Text );
                return;
            }

            const SvxFieldItem* pFld;
            if ( comphelper::LibreOfficeKit::isActive() )
            {
                Point aLogicClick = pEditView->GetOutputDevice().PixelToLogic(aPos);
                pFld = pEditView->GetField( aLogicClick );
            }
            else
            {
                pFld = pEditView->GetFieldUnderMousePointer();
            }
            //  Field can only be URL field
            bool bAlt = rMEvt.IsMod2();
            if ( !bAlt && !nButtonDown && ScGlobal::ShouldOpenURL() && pFld )
                SetPointer( PointerStyle::RefHand );
            else if ( pEditView->GetEditEngine()->IsEffectivelyVertical() )
                SetPointer( PointerStyle::TextVertical );
            else
                SetPointer( PointerStyle::Text );
            return;
        }
    }

    bool bWater = SC_MOD()->GetIsWaterCan() || mrViewData.GetView()->HasPaintBrush();
    if (bWater)
        SetPointer( PointerStyle::Fill );

    if (!bWater)
    {
        bool bCross = false;

        // range finder

        RfCorner rCorner = NONE;
        if ( HitRangeFinder( rMEvt.GetPosPixel(), rCorner, nullptr, nullptr, nullptr ) )
        {
            if (rCorner != NONE)
                SetPointer( PointerStyle::Cross );
            else
                SetPointer( PointerStyle::Hand );
            bCross = true;
        }

        //  Page-Break-Mode

        if ( !nButtonDown && mrViewData.IsPagebreakMode() )
        {
            sal_uInt16 nBreakType = HitPageBreak( rMEvt.GetPosPixel(), nullptr, nullptr, nullptr );
            if (nBreakType != 0 )
            {
                PointerStyle eNew = PointerStyle::Arrow;
                switch ( nBreakType )
                {
                    case SC_PD_RANGE_L:
                    case SC_PD_RANGE_R:
                    case SC_PD_BREAK_H:
                        eNew = PointerStyle::ESize;
                        break;
                    case SC_PD_RANGE_T:
                    case SC_PD_RANGE_B:
                    case SC_PD_BREAK_V:
                        eNew = PointerStyle::SSize;
                        break;
                    case SC_PD_RANGE_TL:
                    case SC_PD_RANGE_BR:
                        eNew = PointerStyle::SESize;
                        break;
                    case SC_PD_RANGE_TR:
                    case SC_PD_RANGE_BL:
                        eNew = PointerStyle::NESize;
                        break;
                }
                SetPointer( eNew );
                bCross = true;
            }
        }

        // Show fill cursor?

        if ( !bFormulaMode && !nButtonDown )
            if (TestMouse( rMEvt, false ))
                bCross = true;

        if ( nButtonDown && mrViewData.IsAnyFillMode() )
        {
            SetPointer( PointerStyle::Cross );
            bCross = true;
            nScFillModeMouseModifier = rMEvt.GetModifier(); // evaluated for AutoFill and Matrix
        }

        if (!bCross)
        {
            bool bAlt = rMEvt.IsMod2();

            if (bEditMode)                                  // First has to be in edit mode!
                SetPointer( mrViewData.IsThemedCursor() ? PointerStyle::FatCross : PointerStyle::Arrow );
            else if ( !bAlt && !nButtonDown && ScGlobal::ShouldOpenURL() &&
                        GetEditUrl(rMEvt.GetPosPixel()) )
                SetPointer( PointerStyle::RefHand );
            else if ( DrawMouseMove(rMEvt) )                // Reset pointer
                return;
        }
    }

    // In LOK case, avoid spurious "leavingwindow" mouse move events which has negative coordinates.
    // Such events occur for some reason when a user is selecting a range, (even when not leaving the view area)
    // with one or more other viewers in that sheet.
    bool bSkipSelectionUpdate = comphelper::LibreOfficeKit::isActive() &&
        rMEvt.IsLeaveWindow() && (aCurMousePos.X() < 0 || aCurMousePos.Y() < 0);

    if (!bSkipSelectionUpdate)
        mrViewData.GetView()->GetSelEngine()->SelMouseMove( rMEvt );
}

static void lcl_InitMouseEvent(css::awt::MouseEvent& rEvent, const MouseEvent& rEvt)
{
    rEvent.Modifiers = 0;
    if ( rEvt.IsShift() )
        rEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
    if ( rEvt.IsMod1() )
        rEvent.Modifiers |= css::awt::KeyModifier::MOD1;
    if ( rEvt.IsMod2() )
        rEvent.Modifiers |= css::awt::KeyModifier::MOD2;
    if ( rEvt.IsMod3() )
        rEvent.Modifiers |= css::awt::KeyModifier::MOD3;

    rEvent.Buttons = 0;
    if ( rEvt.IsLeft() )
        rEvent.Buttons |= css::awt::MouseButton::LEFT;
    if ( rEvt.IsRight() )
        rEvent.Buttons |= css::awt::MouseButton::RIGHT;
    if ( rEvt.IsMiddle() )
        rEvent.Buttons |= css::awt::MouseButton::MIDDLE;

    rEvent.X = rEvt.GetPosPixel().X();
    rEvent.Y = rEvt.GetPosPixel().Y();
    rEvent.ClickCount = rEvt.GetClicks();
    rEvent.PopupTrigger = false;
}

bool ScGridWindow::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    MouseNotifyEvent nType = rNEvt.GetType();
    if ( nType == MouseNotifyEvent::MOUSEBUTTONUP || nType == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        vcl::Window* pWindow = rNEvt.GetWindow();
        if (pWindow == this)
        {
            SfxViewFrame* pViewFrame = mrViewData.GetViewShell()->GetViewFrame();
            if (pViewFrame)
            {
                css::uno::Reference<css::frame::XController> xController = pViewFrame->GetFrame().GetController();
                if (xController.is())
                {
                    ScTabViewObj* pImp = comphelper::getFromUnoTunnel<ScTabViewObj>( xController );
                    if (pImp && pImp->IsMouseListening())
                    {
                        css::awt::MouseEvent aEvent;
                        lcl_InitMouseEvent( aEvent, *rNEvt.GetMouseEvent() );
                        if ( rNEvt.GetWindow() )
                            aEvent.Source = rNEvt.GetWindow()->GetComponentInterface();
                        if ( nType == MouseNotifyEvent::MOUSEBUTTONDOWN)
                            bDone = pImp->MousePressed( aEvent );
                        else
                            bDone = pImp->MouseReleased( aEvent );
                    }
                }
            }
        }
    }
    if (bDone)      // event consumed by a listener
    {
        if ( nType == MouseNotifyEvent::MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMouseEvent = rNEvt.GetMouseEvent();
            if ( pMouseEvent->IsRight() && pMouseEvent->GetClicks() == 1 )
            {
                // If a listener returned true for a right-click call, also prevent opening the context menu
                // (this works only if the context menu is opened on mouse-down)
                nMouseStatus = SC_GM_IGNORE;
            }
        }

        return true;
    }
    else
        return Window::PreNotify( rNEvt );
}

void ScGridWindow::Tracking( const TrackingEvent& rTEvt )
{
    // Since the SelectionEngine does not track, the events have to be
    // handed to the different MouseHandler...

    const MouseEvent& rMEvt = rTEvt.GetMouseEvent();

    if ( rTEvt.IsTrackingCanceled() )           // Cancel everything...
    {
        if (!mrViewData.GetView()->IsInActivatePart() && !SC_MOD()->IsRefDialogOpen())
        {
            if (bDPMouse)
                bDPMouse = false;               // Paint for each bDragRect
            if (bDragRect)
            {
                bDragRect = false;
                UpdateDragRectOverlay();
            }
            if (bRFMouse)
            {
                RFMouseMove( rMEvt, true );     // Not possible to cancel properly...
                bRFMouse = false;
            }
            if (nPagebreakMouse)
            {
                bPagebreakDrawn = false;
                UpdateDragRectOverlay();
                nPagebreakMouse = SC_PD_NONE;
            }

            SetPointer( PointerStyle::Arrow );
            StopMarking();
            MouseButtonUp( rMEvt );     // With status SC_GM_IGNORE from StopMarking

            bool bRefMode = mrViewData.IsRefMode();
            if (bRefMode)
                SC_MOD()->EndReference();       // Do not let the Dialog remain minimized
        }
    }
    else if ( rTEvt.IsTrackingEnded() )
    {
        if ( !comphelper::LibreOfficeKit::isActive() )
        {
            // MouseButtonUp always with matching buttons (eg for test tool, # 63148 #)
            // The tracking event will indicate if it was completed and not canceled.
            MouseEvent aUpEvt( rMEvt.GetPosPixel(), rMEvt.GetClicks(),
                                rMEvt.GetMode(), nButtonDown, rMEvt.GetModifier() );
            MouseButtonUp( aUpEvt );
        }
    }
    else if ( !comphelper::LibreOfficeKit::isActive() )
        MouseMove( rMEvt );
}

void ScGridWindow::StartDrag( sal_Int8 /* nAction */, const Point& rPosPixel )
{
    if (mpFilterBox || nPagebreakMouse)
        return;

    HideNoteMarker();

    CommandEvent aDragEvent( rPosPixel, CommandEventId::StartDrag, true );

    if (bEEMouse && mrViewData.HasEditView( eWhich ))
    {
        EditView*   pEditView;
        SCCOL       nEditCol;
        SCROW       nEditRow;
        mrViewData.GetEditView( eWhich, pEditView, nEditCol, nEditRow );

        // don't remove the edit view while switching views
        ScModule* pScMod = SC_MOD();
        pScMod->SetInEditCommand( true );

        pEditView->Command( aDragEvent );

        ScInputHandler* pHdl = pScMod->GetInputHdl();
        if (pHdl)
            pHdl->DataChanged();

        pScMod->SetInEditCommand( false );
        if (!mrViewData.IsActive())             // dropped to different view?
        {
            ScInputHandler* pViewHdl = pScMod->GetInputHdl( mrViewData.GetViewShell() );
            if ( pViewHdl && mrViewData.HasEditView( eWhich ) )
            {
                pViewHdl->CancelHandler();
                ShowCursor();   // missing from KillEditView
            }
        }
    }
    else
        if ( !DrawCommand(aDragEvent) )
            mrViewData.GetView()->GetSelEngine()->Command( aDragEvent );
}

static void lcl_SetTextCursorPos( ScViewData& rViewData, ScSplitPos eWhich, vcl::Window* pWin )
{
    SCCOL nCol = rViewData.GetCurX();
    SCROW nRow = rViewData.GetCurY();
    tools::Rectangle aEditArea = rViewData.GetEditArea( eWhich, nCol, nRow, pWin, nullptr, true );
    aEditArea.SetRight( aEditArea.Left() );
    aEditArea = pWin->PixelToLogic( aEditArea );
    pWin->SetCursorRect( &aEditArea );
}

void ScGridWindow::Command( const CommandEvent& rCEvt )
{
    // The command event is send to the window after a possible context
    // menu from an inplace client is closed. Now we have the chance to
    // deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    CommandEventId nCmd = rCEvt.GetCommand();
    ScTabViewShell* pTabViewSh = mrViewData.GetViewShell();
    SfxInPlaceClient* pClient = pTabViewSh->GetIPClient();
    if ( pClient &&
         pClient->IsObjectInPlaceActive() &&
         nCmd == CommandEventId::ContextMenu )
    {
        pTabViewSh->DeactivateOle();
        return;
    }

    ScModule* pScMod = SC_MOD();
    OSL_ENSURE( nCmd != CommandEventId::StartDrag, "ScGridWindow::Command called with CommandEventId::StartDrag" );

    if (nCmd == CommandEventId::ModKeyChange)
    {
        Window::Command(rCEvt);
        return;
    }

    if ( nCmd == CommandEventId::StartExtTextInput ||
         nCmd == CommandEventId::EndExtTextInput ||
         nCmd == CommandEventId::ExtTextInput ||
         nCmd == CommandEventId::CursorPos ||
         nCmd == CommandEventId::QueryCharPosition )
    {
        bool bEditView = mrViewData.HasEditView( eWhich );
        if (!bEditView)
        {
            //  only if no cell editview is active, look at drawview
            SdrView* pSdrView = mrViewData.GetView()->GetScDrawView();
            if ( pSdrView )
            {
                OutlinerView* pOlView = pSdrView->GetTextEditOutlinerView();
                if ( pOlView && pOlView->GetWindow() == this )
                {
                    pOlView->Command( rCEvt );
                    return;                             // done
                }
            }
        }

        if ( nCmd == CommandEventId::CursorPos && !bEditView )
        {
            //  CURSORPOS may be called without following text input,
            //  to set the input method window position
            //  -> input mode must not be started,
            //  manually calculate text insert position if not in input mode

            lcl_SetTextCursorPos( mrViewData, eWhich, this );
            return;
        }

        ScInputHandler* pHdl = pScMod->GetInputHdl( mrViewData.GetViewShell() );
        if ( pHdl )
        {
            pHdl->InputCommand( rCEvt );
            return;                                     // done
        }

        Window::Command( rCEvt );
        return;
    }

    if ( nCmd == CommandEventId::PasteSelection )
    {
        if ( bEEMouse )
        {
            //  EditEngine handles selection in MouseButtonUp - no action
            //  needed in command handler
        }
        else
        {
            PasteSelection( rCEvt.GetMousePosPixel() );
        }
        return;
    }

    if ( nCmd == CommandEventId::InputLanguageChange )
    {
        // #i55929# Font and font size state depends on input language if nothing is selected,
        // so the slots have to be invalidated when the input language is changed.

        SfxBindings& rBindings = mrViewData.GetBindings();
        rBindings.Invalidate( SID_ATTR_CHAR_FONT );
        rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
        return;
    }

    if ( nCmd == CommandEventId::Wheel || nCmd == CommandEventId::StartAutoScroll || nCmd == CommandEventId::AutoScroll )
    {
        bool bDone = mrViewData.GetView()->ScrollCommand( rCEvt, eWhich );
        if (!bDone)
            Window::Command(rCEvt);
        return;
    }
    // #i7560# FormulaMode check is below scrolling - scrolling is allowed during formula input
    bool bDisable = pScMod->IsFormulaMode() ||
                    pScMod->IsModalMode(mrViewData.GetSfxDocShell());
    if (bDisable)
        return;

    if (nCmd != CommandEventId::ContextMenu || SC_MOD()->GetIsWaterCan())
        return;

    bool bMouse = rCEvt.IsMouseEvent();
    if ( bMouse && nMouseStatus == SC_GM_IGNORE )
        return;

    if (mrViewData.IsAnyFillMode())
    {
        mrViewData.GetView()->StopRefMode();
        mrViewData.ResetFillMode();
    }
    ReleaseMouse();
    StopMarking();

    Point aPosPixel = rCEvt.GetMousePosPixel();
    Point aMenuPos = aPosPixel;

    SCCOL nCellX = -1;
    SCROW nCellY = -1;
    mrViewData.GetPosFromPixel(aPosPixel.X(), aPosPixel.Y(), eWhich, nCellX, nCellY);

    bool bSpellError = false;
    SCCOL nColSpellError = nCellX;

    if ( bMouse )
    {
        ScDocument& rDoc = mrViewData.GetDocument();
        SCTAB nTab = mrViewData.GetTabNo();
        const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        bool bSelectAllowed = true;
        if ( pProtect && pProtect->isProtected() )
        {
            // This sheet is protected.  Check if a context menu is allowed on this cell.
            bool bCellProtected = rDoc.HasAttrib(nCellX, nCellY, nTab, nCellX, nCellY, nTab, HasAttrFlags::Protected);
            bool bSelProtected   = pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
            bool bSelUnprotected = pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);

            if (bCellProtected)
                bSelectAllowed = bSelProtected;
            else
                bSelectAllowed = bSelUnprotected;
        }
        if (!bSelectAllowed)
            // Selecting this cell is not allowed, neither is context menu.
            return;

        if (mpSpellCheckCxt)
        {
            // Find the first string to the left for spell checking in case the current cell is empty.
            ScAddress aPos(nCellX, nCellY, nTab);
            ScRefCellValue aSpellCheckCell(rDoc, aPos);
            while (aSpellCheckCell.meType == CELLTYPE_NONE)
            {
                // Loop until we get the first non-empty cell in the row.
                aPos.IncCol(-1);
                if (aPos.Col() < 0)
                    break;

                aSpellCheckCell.assign(rDoc, aPos);
            }

            if (aPos.Col() >= 0 && (aSpellCheckCell.meType == CELLTYPE_STRING || aSpellCheckCell.meType == CELLTYPE_EDIT))
                nColSpellError = aPos.Col();

            bSpellError = (mpSpellCheckCxt->isMisspelled(nColSpellError, nCellY));
            if (bSpellError)
            {
                // Check and see if a misspelled word is under the mouse pointer.
                bSpellError = IsSpellErrorAtPos(aPosPixel, nColSpellError, nCellY);
            }
        }

        //  #i18735# First select the item under the mouse pointer.
        //  This can change the selection, and the view state (edit mode, etc).
        SelectForContextMenu(aPosPixel, bSpellError ? nColSpellError : nCellX, nCellY);
    }

    bool bDone = false;
    bool bEdit = mrViewData.HasEditView(eWhich);

    if ( !bEdit )
    {
            // Edit cell with spelling errors ?
        if (bMouse && (GetEditUrl(aPosPixel) || bSpellError))
        {
            //  GetEditUrlOrError has already moved the Cursor

            pScMod->SetInputMode( SC_INPUT_TABLE );
            bEdit = mrViewData.HasEditView(eWhich);     // Did it work?

            OSL_ENSURE( bEdit, "Can not be switched in edit mode" );
        }
    }
    if ( bEdit )
    {
        EditView* pEditView = mrViewData.GetEditView( eWhich );     // is then not 0

        if ( !bMouse )
        {
            vcl::Cursor* pCur = pEditView->GetCursor();
            if ( pCur )
            {
                Point aLogicPos = pCur->GetPos();
                //  use the position right of the cursor (spell popup is opened if
                //  the cursor is before the word, but not if behind it)
                aLogicPos.AdjustX(pCur->GetWidth() );
                aLogicPos.AdjustY(pCur->GetHeight() / 2 );     // center vertically
                aMenuPos = LogicToPixel( aLogicPos );
            }
        }

        //  if edit mode was just started above, online spelling may be incomplete
        pEditView->GetEditEngine()->CompleteOnlineSpelling();

        //  IsCursorAtWrongSpelledWord could be used for !bMouse
        //  if there was a corresponding ExecuteSpellPopup call

        if (bSpellError)
        {
            // On OS/2 when clicking next to the Popup menu, the MouseButtonDown
            // comes before the end of menu execute, thus the SetModified has to
            // be done prior to this (Bug #40968#)
            ScInputHandler* pHdl = pScMod->GetInputHdl();
            if (pHdl)
                pHdl->SetModified();

            Link<SpellCallbackInfo&,void> aLink = LINK( this, ScGridWindow, PopupSpellingHdl );
            pEditView->ExecuteSpellPopup(aMenuPos, aLink);

            bDone = true;
        }
    }
    else if ( !bMouse )
    {
        //  non-edit menu by keyboard -> use lower right of cell cursor position
        ScDocument& rDoc = mrViewData.GetDocument();
        SCTAB nTabNo = mrViewData.GetTabNo();
        bool bLayoutIsRTL = rDoc.IsLayoutRTL(nTabNo);

        SCCOL nCurX = mrViewData.GetCurX();
        SCROW nCurY = mrViewData.GetCurY();
        aMenuPos = mrViewData.GetScrPos( nCurX, nCurY, eWhich, true );
        tools::Long nSizeXPix;
        tools::Long nSizeYPix;
        mrViewData.GetMergeSizePixel( nCurX, nCurY, nSizeXPix, nSizeYPix );
        // fdo#55432 take the correct position for RTL sheet
        aMenuPos.AdjustX(bLayoutIsRTL ? -nSizeXPix : nSizeXPix );
        aMenuPos.AdjustY(nSizeYPix );

        ScTabViewShell* pViewSh = mrViewData.GetViewShell();
        if (pViewSh)
        {
            //  Is a draw object selected?

            SdrView* pDrawView = pViewSh->GetScDrawView();
            if (pDrawView && pDrawView->AreObjectsMarked())
            {
                // #100442#; the context menu should open in the middle of the selected objects
                tools::Rectangle aSelectRect(LogicToPixel(pDrawView->GetAllMarkedBoundRect()));
                aMenuPos = aSelectRect.Center();
            }
        }
    }

    if (bDone)
        return;

    // tdf#140361 at this context menu popup time get what the
    // DisableEditHyperlink would be for this position
    bool bShouldDisableEditHyperlink = mrViewData.GetViewShell()->ShouldDisableEditHyperlink();

    SfxDispatcher::ExecutePopup( this, &aMenuPos );

    if (!bShouldDisableEditHyperlink)
    {
        SfxBindings& rBindings = mrViewData.GetBindings();
        // tdf#140361 set what the menu popup state for this was
        mrViewData.GetViewShell()->EnableEditHyperlink();
        // ensure moAtContextMenu_DisableEditHyperlink will be cleared
        // in the case that EditHyperlink is not dispatched by the menu
        rBindings.Invalidate(SID_EDIT_HYPERLINK);
    }
}

void ScGridWindow::SelectForContextMenu( const Point& rPosPixel, SCCOL nCellX, SCROW nCellY )
{
    //  #i18735# if the click was outside of the current selection,
    //  the cursor is moved or an object at the click position selected.
    //  (see SwEditWin::SelectMenuPosition in Writer)

    ScTabView* pView = mrViewData.GetView();
    ScDrawView* pDrawView = pView->GetScDrawView();

    //  check cell edit mode

    if ( mrViewData.HasEditView(eWhich) )
    {
        ScModule* pScMod = SC_MOD();
        SCCOL nEditStartCol = mrViewData.GetEditViewCol(); //! change to GetEditStartCol after calcrtl is integrated
        SCROW nEditStartRow = mrViewData.GetEditViewRow();
        SCCOL nEditEndCol = mrViewData.GetEditEndCol();
        SCROW nEditEndRow = mrViewData.GetEditEndRow();

        if ( nCellX >= nEditStartCol && nCellX <= nEditEndCol &&
             nCellY >= nEditStartRow && nCellY <= nEditEndRow )
        {
            //  handle selection within the EditView

            EditView* pEditView = mrViewData.GetEditView( eWhich );     // not NULL (HasEditView)
            EditEngine* pEditEngine = pEditView->GetEditEngine();
            tools::Rectangle aOutputArea = pEditView->GetOutputArea();
            tools::Rectangle aVisArea = pEditView->GetVisArea();

            Point aTextPos = PixelToLogic( rPosPixel );
            if ( pEditEngine->IsEffectivelyVertical() )            // have to manually transform position
            {
                aTextPos -= aOutputArea.TopRight();
                tools::Long nTemp = -aTextPos.X();
                aTextPos.setX( aTextPos.Y() );
                aTextPos.setY( nTemp );
            }
            else
                aTextPos -= aOutputArea.TopLeft();
            aTextPos += aVisArea.TopLeft();             // position in the edit document

            EPosition aDocPosition = pEditEngine->FindDocPosition(aTextPos);
            ESelection aCompare(aDocPosition.nPara, aDocPosition.nIndex);
            ESelection aSelection = pEditView->GetSelection();
            aSelection.Adjust();    // needed for IsLess/IsGreater
            if ( aCompare < aSelection || aCompare > aSelection )
            {
                // clicked outside the selected text - deselect and move text cursor
                MouseEvent aEvent( rPosPixel );
                pEditView->MouseButtonDown( aEvent );
                pEditView->MouseButtonUp( aEvent );
                pScMod->InputSelection( pEditView );
            }

            return;     // clicked within the edit view - keep edit mode
        }
        else
        {
            // outside of the edit view - end edit mode, regardless of cell selection, then continue
            pScMod->InputEnterHandler();
        }
    }

    //  check draw text edit mode

    Point aLogicPos = PixelToLogic( rPosPixel );        // after cell edit mode is ended
    if ( pDrawView && pDrawView->GetTextEditObject() && pDrawView->GetTextEditOutlinerView() )
    {
        OutlinerView* pOlView = pDrawView->GetTextEditOutlinerView();
        tools::Rectangle aOutputArea = pOlView->GetOutputArea();
        if ( aOutputArea.Contains( aLogicPos ) )
        {
            //  handle selection within the OutlinerView

            Outliner* pOutliner = pOlView->GetOutliner();
            const EditEngine& rEditEngine = pOutliner->GetEditEngine();
            tools::Rectangle aVisArea = pOlView->GetVisArea();

            Point aTextPos = aLogicPos;
            if ( pOutliner->IsVertical() )              // have to manually transform position
            {
                aTextPos -= aOutputArea.TopRight();
                tools::Long nTemp = -aTextPos.X();
                aTextPos.setX( aTextPos.Y() );
                aTextPos.setY( nTemp );
            }
            else
                aTextPos -= aOutputArea.TopLeft();
            aTextPos += aVisArea.TopLeft();             // position in the edit document

            EPosition aDocPosition = rEditEngine.FindDocPosition(aTextPos);
            ESelection aCompare(aDocPosition.nPara, aDocPosition.nIndex);
            ESelection aSelection = pOlView->GetSelection();
            aSelection.Adjust();    // needed for IsLess/IsGreater
            if ( aCompare < aSelection || aCompare > aSelection )
            {
                // clicked outside the selected text - deselect and move text cursor
                // use DrawView to allow extra handling there (none currently)
                MouseEvent aEvent( rPosPixel );
                pDrawView->MouseButtonDown( aEvent, GetOutDev() );
                pDrawView->MouseButtonUp( aEvent, GetOutDev() );
            }

            return;     // clicked within the edit area - keep edit mode
        }
        else
        {
            // Outside of the edit area - end text edit mode, then continue.
            // DrawDeselectAll also ends text edit mode and updates the shells.
            // If the click was on the edited object, it will be selected again below.
            pView->DrawDeselectAll();
        }
    }

    //  look for existing selection

    bool bHitSelected = false;
    if ( pDrawView && pDrawView->IsMarkedObjHit( aLogicPos ) )
    {
        //  clicked on selected object -> don't change anything
        bHitSelected = true;
    }
    else if ( mrViewData.GetMarkData().IsCellMarked(nCellX, nCellY) )
    {
        //  clicked on selected cell -> don't change anything
        bHitSelected = true;
    }

    //  select drawing object or move cell cursor

    if ( bHitSelected )
        return;

    bool bWasDraw = ( pDrawView && pDrawView->AreObjectsMarked() );
    bool bHitDraw = false;
    if ( pDrawView )
    {
        pDrawView->UnmarkAllObj();
        // Unlock the Internal Layer in order to activate the context menu.
        // re-lock in ScDrawView::MarkListHasChanged()
        lcl_UnLockComment(pDrawView, aLogicPos, mrViewData);
        bHitDraw = pDrawView->MarkObj( aLogicPos );
        // draw shell is activated in MarkListHasChanged
    }
    if ( !bHitDraw )
    {
        pView->Unmark();
        pView->SetCursor(nCellX, nCellY);
        if ( bWasDraw )
            mrViewData.GetViewShell()->SetDrawShell( false );   // switch shells
    }
}

void ScGridWindow::KeyInput(const KeyEvent& rKEvt)
{
    // Cursor control for ref input dialog
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

#ifdef DBG_UTIL

    if (rKeyCode.IsMod1() && rKeyCode.IsShift())
    {
        if (rKeyCode.GetCode() == KEY_F12)
        {
            dumpColumnInformationPixel();
        }
        else if (rKeyCode.GetCode() == KEY_F11)
        {
            dumpGraphicInformation();
        }
        else if (rKeyCode.GetCode() == KEY_F10)
        {
            dumpColumnInformationHmm();
        }
        else if (rKeyCode.GetCode() == KEY_F6)
        {
            dumpCellProperties();
        }
        else if (rKeyCode.GetCode() == KEY_F8)
        {
            dumpColumnCellStorage();
        }
        else if (rKeyCode.GetCode() == KEY_F7)
        {
            ScDocument& rDoc = mrViewData.GetDocument();
            auto& rMapper = rDoc.GetExternalDataMapper();
            for (auto& itr : rMapper.getDataSources())
            {
                itr.refresh(&rDoc);
            }
            return;
        }
    }

#endif

    if( SC_MOD()->IsRefDialogOpen() )
    {
        if( !rKeyCode.GetModifier() && (rKeyCode.GetCode() == KEY_F2) )
        {
            SC_MOD()->EndReference();
        }
        else if( mrViewData.GetViewShell()->MoveCursorKeyInput( rKEvt ) )
        {
            ScRange aRef(
                mrViewData.GetRefStartX(), mrViewData.GetRefStartY(), mrViewData.GetRefStartZ(),
                mrViewData.GetRefEndX(), mrViewData.GetRefEndY(), mrViewData.GetRefEndZ() );
            SC_MOD()->SetReference( aRef, mrViewData.GetDocument() );
        }
        mrViewData.GetViewShell()->SelectionChanged();
        return ;
    }
    else if( rKeyCode.GetCode() == KEY_RETURN && mrViewData.IsPasteMode()
            && SC_MOD()->GetInputOptions().GetEnterPasteMode() )
    {
        ScTabViewShell* pTabViewShell = mrViewData.GetViewShell();
        ScClipUtil::PasteFromClipboard( mrViewData, pTabViewShell, true );

        // Clear clipboard content.
        uno::Reference<datatransfer::clipboard::XClipboard> xSystemClipboard =
            GetClipboard();
        if (xSystemClipboard.is())
        {
            xSystemClipboard->setContents(
                    uno::Reference<datatransfer::XTransferable>(),
                    uno::Reference<datatransfer::clipboard::XClipboardOwner>());
        }

        // hide the border around the copy source
        mrViewData.SetPasteMode( ScPasteFlags::NONE );
        // Clear CopySourceOverlay in each window of a split/frozen tabview
        mrViewData.GetView()->UpdateCopySourceOverlay();
        return;
    }
    // if semi-modeless SfxChildWindow dialog above, then no KeyInputs:
    else if( !mrViewData.IsAnyFillMode() )
    {
        if (rKeyCode.GetCode() == KEY_ESCAPE)
        {
            mrViewData.SetPasteMode( ScPasteFlags::NONE );
            // Clear CopySourceOverlay in each window of a split/frozen tabview
            mrViewData.GetView()->UpdateCopySourceOverlay();
        }
        //  query for existing note marker before calling ViewShell's keyboard handling
        //  which may remove the marker
        bool bHadKeyMarker = mpNoteMarker && mpNoteMarker->IsByKeyboard();
        ScTabViewShell* pViewSh = mrViewData.GetViewShell();

        if (mrViewData.GetDocShell()->GetProgress())
            return;

        if (DrawKeyInput(rKEvt, this))
        {
            const vcl::KeyCode& rLclKeyCode = rKEvt.GetKeyCode();
            if (rLclKeyCode.GetCode() == KEY_DOWN
                || rLclKeyCode.GetCode() == KEY_UP
                || rLclKeyCode.GetCode() == KEY_LEFT
                || rLclKeyCode.GetCode() == KEY_RIGHT)
            {
                ScTabViewShell* pViewShell = mrViewData.GetViewShell();
                SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
                rBindings.Invalidate(SID_ATTR_TRANSFORM_POS_X);
                rBindings.Invalidate(SID_ATTR_TRANSFORM_POS_Y);
            }
            return;
        }

        if (!mrViewData.GetView()->IsDrawSelMode() && !DrawHasMarkedObj())  // No entries in draw mode
        {                                                           //! check DrawShell !!!
            if (pViewSh->TabKeyInput(rKEvt))
                return;
        }
        else
            if (pViewSh->SfxViewShell::KeyInput(rKEvt))             // from SfxViewShell
                return;

        vcl::KeyCode aCode = rKEvt.GetKeyCode();
        if ( aCode.GetCode() == KEY_ESCAPE && aCode.GetModifier() == 0 )
        {
            if ( bHadKeyMarker )
                HideNoteMarker();
            else
                pViewSh->Escape();
            return;
        }
        if ( aCode.GetCode() == KEY_F1 && aCode.GetModifier() == KEY_MOD1 )
        {
            //  ctrl-F1 shows or hides the note or redlining info for the cursor position
            //  (hard-coded because F1 can't be configured)

            if ( bHadKeyMarker )
                HideNoteMarker();       // hide when previously visible
            else
                ShowNoteMarker( mrViewData.GetCurX(), mrViewData.GetCurY(), true );
            return;
        }
        if (aCode.GetCode() == KEY_BRACKETLEFT && aCode.GetModifier() == KEY_MOD1)
        {
            pViewSh->DetectiveMarkPred();
            return;
        }
        if (aCode.GetCode() == KEY_BRACKETRIGHT && aCode.GetModifier() == KEY_MOD1)
        {
            pViewSh->DetectiveMarkSucc();
            return;
        }

    }

    Window::KeyInput(rKEvt);
}

OUString ScGridWindow::GetSurroundingText() const
{
    bool bEditView = mrViewData.HasEditView(eWhich);
    if (bEditView)
    {
        ScModule* pScMod = SC_MOD();
        ScInputHandler* pHdl = pScMod->GetInputHdl(mrViewData.GetViewShell());
        if (pHdl)
            return pHdl->GetSurroundingText();
    }
    else if (SdrView* pSdrView = mrViewData.GetView()->GetScDrawView())
    {
        // if no cell editview is active, look at drawview
        OutlinerView* pOlView = pSdrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            return pOlView->GetSurroundingText();
    }

    return Window::GetSurroundingText();
}

Selection ScGridWindow::GetSurroundingTextSelection() const
{
    bool bEditView = mrViewData.HasEditView(eWhich);
    if (bEditView)
    {
        ScModule* pScMod = SC_MOD();
        ScInputHandler* pHdl = pScMod->GetInputHdl(mrViewData.GetViewShell());
        if (pHdl)
            return pHdl->GetSurroundingTextSelection();
    }
    else if (SdrView* pSdrView = mrViewData.GetView()->GetScDrawView())
    {
        // if no cell editview is active, look at drawview
        OutlinerView* pOlView = pSdrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            return pOlView->GetSurroundingTextSelection();
    }

    return Window::GetSurroundingTextSelection();
}

bool ScGridWindow::DeleteSurroundingText(const Selection& rSelection)
{
    bool bEditView = mrViewData.HasEditView(eWhich);
    if (bEditView)
    {
        ScModule* pScMod = SC_MOD();
        ScInputHandler* pHdl = pScMod->GetInputHdl(mrViewData.GetViewShell());
        if (pHdl)
            return pHdl->DeleteSurroundingText(rSelection);
    }
    else if (SdrView* pSdrView = mrViewData.GetView()->GetScDrawView())
    {
        // if no cell editview is active, look at drawview
        OutlinerView* pOlView = pSdrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            return pOlView->DeleteSurroundingText(rSelection);
    }

    return Window::DeleteSurroundingText(rSelection);
}

void ScGridWindow::StopMarking()
{
    DrawEndAction();                // Cancel Select/move on Drawing-Layer

    if (nButtonDown)
    {
        mrViewData.GetMarkData().SetMarking(false);
        nMouseStatus = SC_GM_IGNORE;
    }
}

void ScGridWindow::UpdateInputContext()
{
    bool bReadOnly = mrViewData.GetDocShell()->IsReadOnly();
    InputContextFlags nOptions = bReadOnly ? InputContextFlags::NONE : ( InputContextFlags::Text | InputContextFlags::ExtText );

    //  when font from InputContext is used,
    //  it must be taken from the cursor position's cell attributes

    InputContext aContext;
    aContext.SetOptions( nOptions );
    SetInputContext( aContext );
}

                                // sensitive range (Pixel)
#define SCROLL_SENSITIVE 20

void ScGridWindow::DropScroll( const Point& rMousePos )
{
    ScDocument& rDoc = mrViewData.GetDocument();
    SCCOL nDx = 0;
    SCROW nDy = 0;
    Size aSize = GetOutputSizePixel();

    if (aSize.Width() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.X() < SCROLL_SENSITIVE && mrViewData.GetPosX(WhichH(eWhich)) > 0 )
            nDx = -1;
        if ( rMousePos.X() >= aSize.Width() - SCROLL_SENSITIVE
                && mrViewData.GetPosX(WhichH(eWhich)) < rDoc.MaxCol() )
            nDx = 1;
    }
    if (aSize.Height() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.Y() < SCROLL_SENSITIVE && mrViewData.GetPosY(WhichV(eWhich)) > 0 )
            nDy = -1;
        if ( rMousePos.Y() >= aSize.Height() - SCROLL_SENSITIVE
                && mrViewData.GetPosY(WhichV(eWhich)) < rDoc.MaxRow() )
            nDy = 1;
    }

    if ( nDx != 0 || nDy != 0 )
    {
        if ( nDx != 0 )
            mrViewData.GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 )
            mrViewData.GetView()->ScrollY( nDy, WhichV(eWhich) );
    }
}

static bool lcl_TestScenarioRedliningDrop( const ScDocument* pDoc, const ScRange& aDragRange)
{
    //  Test, if a scenario is affected by a drop when turing on RedLining,
    bool bReturn = false;
    SCTAB nTab = aDragRange.aStart.Tab();
    SCTAB nTabCount = pDoc->GetTableCount();

    if(pDoc->GetChangeTrack()!=nullptr)
    {
        if( pDoc->IsScenario(nTab) && pDoc->HasScenarioRange(nTab, aDragRange))
        {
            bReturn = true;
        }
        else
        {
            for(SCTAB i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            {
                if(pDoc->HasScenarioRange(i, aDragRange))
                {
                    bReturn = true;
                    break;
                }
            }
        }
    }
    return bReturn;
}

static ScRange lcl_MakeDropRange( const ScDocument& rDoc, SCCOL nPosX, SCROW nPosY, SCTAB nTab, const ScRange& rSource )
{
    SCCOL nCol1 = nPosX;
    SCCOL nCol2 = nCol1 + ( rSource.aEnd.Col() - rSource.aStart.Col() );
    if ( nCol2 > rDoc.MaxCol() )
    {
        nCol1 -= nCol2 - rDoc.MaxCol();
        nCol2 = rDoc.MaxCol();
    }
    SCROW nRow1 = nPosY;
    SCROW nRow2 = nRow1 + ( rSource.aEnd.Row() - rSource.aStart.Row() );
    if ( nRow2 > rDoc.MaxRow() )
    {
        nRow1 -= nRow2 - rDoc.MaxRow();
        nRow2 = rDoc.MaxRow();
    }

    return ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
}

sal_Int8 ScGridWindow::AcceptPrivateDrop( const AcceptDropEvent& rEvt )
{
    if ( rEvt.mbLeaving )
    {
        bDragRect = false;
        UpdateDragRectOverlay();
        return rEvt.mnAction;
    }

    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rData.pCellTransfer )
    {
        // Don't move source that would include filtered rows.
        if ((rEvt.mnAction & DND_ACTION_MOVE) && rData.pCellTransfer->HasFilteredRows())
        {
            if (bDragRect)
            {
                bDragRect = false;
                UpdateDragRectOverlay();
            }
            return DND_ACTION_NONE;
        }

        Point aPos = rEvt.maPosPixel;

        ScDocument* pSourceDoc = rData.pCellTransfer->GetSourceDocument();
        ScDocument& rThisDoc   = mrViewData.GetDocument();
        if (pSourceDoc == &rThisDoc)
        {
            OUString aName;
            if ( rThisDoc.HasChartAtPoint(mrViewData.GetTabNo(), PixelToLogic(aPos), aName ))
            {
                if (bDragRect)          // Remove rectangle
                {
                    bDragRect = false;
                    UpdateDragRectOverlay();
                }

                //! highlight chart? (selection border?)

                sal_Int8 nRet = rEvt.mnAction;
                return nRet;
            }
        }

        if (rData.pCellTransfer->GetDragSourceFlags() & ScDragSrc::Table) // whole sheet?
        {
            bool bOk = rThisDoc.IsDocEditable();
            return bOk ? rEvt.mnAction : 0;                     // don't draw selection frame
        }

        SCCOL  nPosX;
        SCROW  nPosY;
        mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

        ScRange aSourceRange = rData.pCellTransfer->GetRange();
        SCCOL nSourceStartX = aSourceRange.aStart.Col();
        SCROW nSourceStartY = aSourceRange.aStart.Row();
        SCCOL nSourceEndX = aSourceRange.aEnd.Col();
        SCROW nSourceEndY = aSourceRange.aEnd.Row();
        SCCOL nSizeX = nSourceEndX - nSourceStartX + 1;
        SCROW nSizeY = nSourceEndY - nSourceStartY + 1;

        if ( rEvt.mnAction != DND_ACTION_MOVE )
            nSizeY = rData.pCellTransfer->GetNonFilteredRows();     // copy/link: no filtered rows

        SCCOL nNewDragX = nPosX - rData.pCellTransfer->GetDragHandleX();
        if (nNewDragX<0) nNewDragX=0;
        if (nNewDragX+(nSizeX-1) > rThisDoc.MaxCol())
            nNewDragX = rThisDoc.MaxCol()-(nSizeX-1);
        SCROW nNewDragY = nPosY - rData.pCellTransfer->GetDragHandleY();
        if (nNewDragY<0) nNewDragY=0;
        if (nNewDragY+(nSizeY-1) > rThisDoc.MaxRow())
            nNewDragY = rThisDoc.MaxRow()-(nSizeY-1);

        //  don't break scenario ranges, don't drop on filtered
        SCTAB nTab = mrViewData.GetTabNo();
        ScRange aDropRange = lcl_MakeDropRange( rThisDoc, nNewDragX, nNewDragY, nTab, aSourceRange );
        if ( lcl_TestScenarioRedliningDrop( &rThisDoc, aDropRange ) ||
             lcl_TestScenarioRedliningDrop( pSourceDoc, aSourceRange ) ||
             ScViewUtil::HasFiltered( aDropRange, rThisDoc) )
        {
            if (bDragRect)
            {
                bDragRect = false;
                UpdateDragRectOverlay();
            }
            return DND_ACTION_NONE;
        }

        InsCellCmd eDragInsertMode = INS_NONE;
        Window::PointerState aState = GetPointerState();

        // check for datapilot item sorting
        ScDPObject* pDPObj = nullptr;
        if ( &rThisDoc == pSourceDoc && ( pDPObj = rThisDoc.GetDPAtCursor( nNewDragX, nNewDragY, nTab ) ) != nullptr )
        {
            // drop on DataPilot table: sort or nothing

            bool bDPSort = false;
            if ( rThisDoc.GetDPAtCursor( nSourceStartX, nSourceStartY, aSourceRange.aStart.Tab() ) == pDPObj )
            {
                sheet::DataPilotTableHeaderData aDestData;
                pDPObj->GetHeaderPositionData( ScAddress(nNewDragX, nNewDragY, nTab), aDestData );
                bool bValid = ( aDestData.Dimension >= 0 );        // dropping onto a field

                // look through the source range
                for (SCROW nRow = aSourceRange.aStart.Row(); bValid && nRow <= aSourceRange.aEnd.Row(); ++nRow )
                    for (SCCOL nCol = aSourceRange.aStart.Col(); bValid && nCol <= aSourceRange.aEnd.Col(); ++nCol )
                    {
                        sheet::DataPilotTableHeaderData aSourceData;
                        pDPObj->GetHeaderPositionData( ScAddress( nCol, nRow, aSourceRange.aStart.Tab() ), aSourceData );
                        if ( aSourceData.Dimension != aDestData.Dimension || aSourceData.MemberName.isEmpty() )
                            bValid = false;     // empty (subtotal) or different field
                    }

                if ( bValid )
                {
                    bool bIsDataLayout;
                    OUString aDimName = pDPObj->GetDimName( aDestData.Dimension, bIsDataLayout );
                    const ScDPSaveDimension* pDim = pDPObj->GetSaveData()->GetExistingDimensionByName( aDimName );
                    if ( pDim )
                    {
                        ScRange aOutRange = pDPObj->GetOutRange();

                        sheet::DataPilotFieldOrientation nOrient = pDim->GetOrientation();
                        if ( nOrient == sheet::DataPilotFieldOrientation_COLUMN )
                        {
                            eDragInsertMode = INS_CELLSRIGHT;
                            nSizeY = aOutRange.aEnd.Row() - nNewDragY + 1;
                            bDPSort = true;
                        }
                        else if ( nOrient == sheet::DataPilotFieldOrientation_ROW )
                        {
                            eDragInsertMode = INS_CELLSDOWN;
                            nSizeX = aOutRange.aEnd.Col() - nNewDragX + 1;
                            bDPSort = true;
                        }
                    }
                }
            }

            if ( !bDPSort )
            {
                // no valid sorting in a DataPilot table -> disallow
                if ( bDragRect )
                {
                    bDragRect = false;
                    UpdateDragRectOverlay();
                }
                return DND_ACTION_NONE;
            }
        }
        else if ( aState.mnState & KEY_MOD2 )
        {
            if ( &rThisDoc == pSourceDoc && nTab == aSourceRange.aStart.Tab() )
            {
                tools::Long nDeltaX = std::abs( static_cast< tools::Long >( nNewDragX - nSourceStartX ) );
                tools::Long nDeltaY = std::abs( static_cast< tools::Long >( nNewDragY - nSourceStartY ) );
                if ( nDeltaX <= nDeltaY )
                {
                    eDragInsertMode = INS_CELLSDOWN;
                }
                else
                {
                    eDragInsertMode = INS_CELLSRIGHT;
                }

                if ( ( eDragInsertMode == INS_CELLSDOWN && nNewDragY <= nSourceEndY &&
                       ( nNewDragX + nSizeX - 1 ) >= nSourceStartX && nNewDragX <= nSourceEndX &&
                       ( nNewDragX != nSourceStartX || nNewDragY >= nSourceStartY ) ) ||
                     ( eDragInsertMode == INS_CELLSRIGHT && nNewDragX <= nSourceEndX &&
                       ( nNewDragY + nSizeY - 1 ) >= nSourceStartY && nNewDragY <= nSourceEndY &&
                       ( nNewDragY != nSourceStartY || nNewDragX >= nSourceStartX ) ) )
                {
                    if ( bDragRect )
                    {
                        bDragRect = false;
                        UpdateDragRectOverlay();
                    }
                    return DND_ACTION_NONE;
                }
            }
            else
            {
                if ( static_cast< tools::Long >( nSizeX ) >= static_cast< tools::Long >( nSizeY ) )
                {
                    eDragInsertMode = INS_CELLSDOWN;

                }
                else
                {
                    eDragInsertMode = INS_CELLSRIGHT;
                }
            }
        }

        if ( nNewDragX != nDragStartX || nNewDragY != nDragStartY ||
             nDragStartX+nSizeX-1 != nDragEndX || nDragStartY+nSizeY-1 != nDragEndY ||
             !bDragRect || eDragInsertMode != meDragInsertMode )
        {
            nDragStartX = nNewDragX;
            nDragStartY = nNewDragY;
            nDragEndX = nDragStartX+nSizeX-1;
            nDragEndY = nDragStartY+nSizeY-1;
            bDragRect = true;
            meDragInsertMode = eDragInsertMode;

            UpdateDragRectOverlay();
        }
    }

    return rEvt.mnAction;
}

sal_Int8 ScGridWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    const ScDragData& rData = SC_MOD()->GetDragData();
    if ( rEvt.mbLeaving )
    {
        DrawMarkDropObj( nullptr );
        if ( rData.pCellTransfer )
            return AcceptPrivateDrop( rEvt );   // hide drop marker for internal D&D
        else
            return rEvt.mnAction;
    }

    if ( mrViewData.GetDocShell()->IsReadOnly() )
        return DND_ACTION_NONE;

    ScDocument& rThisDoc = mrViewData.GetDocument();
    sal_Int8 nRet = DND_ACTION_NONE;

    if (rData.pCellTransfer)
    {
        ScRange aSource = rData.pCellTransfer->GetRange();
        if ( aSource.aStart.Col() != 0 || aSource.aEnd.Col() != rThisDoc.MaxCol() ||
             aSource.aStart.Row() != 0 || aSource.aEnd.Row() != rThisDoc.MaxRow() )
            DropScroll( rEvt.maPosPixel );

        nRet = AcceptPrivateDrop( rEvt );
    }
    else
    {
        if ( !rData.aLinkDoc.isEmpty() )
        {
            OUString aThisName;
            ScDocShell* pDocSh = mrViewData.GetDocShell();
            if (pDocSh && pDocSh->HasName())
                aThisName = pDocSh->GetMedium()->GetName();

            if ( rData.aLinkDoc != aThisName )
                nRet = rEvt.mnAction;
        }
        else if (!rData.aJumpTarget.isEmpty())
        {
            //  internal bookmarks (from Navigator)
            //  local jumps from an unnamed document are possible only within a document

            if ( !rData.pJumpLocalDoc || rData.pJumpLocalDoc == &mrViewData.GetDocument() )
                nRet = rEvt.mnAction;
        }
        else
        {
            sal_Int8 nMyAction = rEvt.mnAction;

            // clear DND_ACTION_LINK when other actions are set. The usage below cannot handle
            // multiple set values
            if((nMyAction & DND_ACTION_LINK) && (nMyAction & DND_ACTION_COPYMOVE))
            {
                nMyAction &= ~DND_ACTION_LINK;
            }

            if ( !rData.pDrawTransfer ||
                    !IsMyModel(rData.pDrawTransfer->GetDragSourceView()) )      // drawing within the document
                if ( rEvt.mbDefault && nMyAction == DND_ACTION_MOVE )
                    nMyAction = DND_ACTION_COPY;

            SdrObject* pHitObj = rThisDoc.GetObjectAtPoint(
                        mrViewData.GetTabNo(), PixelToLogic(rEvt.maPosPixel) );
            if ( pHitObj && nMyAction == DND_ACTION_LINK )
            {
                if ( IsDropFormatSupported(SotClipboardFormatId::SVXB)
                    || IsDropFormatSupported(SotClipboardFormatId::GDIMETAFILE)
                    || IsDropFormatSupported(SotClipboardFormatId::PNG)
                    || IsDropFormatSupported(SotClipboardFormatId::BITMAP) )
                {
                    //  graphic dragged onto drawing object
                    DrawMarkDropObj( pHitObj );
                    nRet = nMyAction;
                }
            }
            if (!nRet)
            {
                DrawMarkDropObj(nullptr);

                switch ( nMyAction )
                {
                    case DND_ACTION_COPY:
                    case DND_ACTION_MOVE:
                    case DND_ACTION_COPYMOVE:
                        {
                            bool bMove = ( nMyAction == DND_ACTION_MOVE );
                            if ( IsDropFormatSupported( SotClipboardFormatId::EMBED_SOURCE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::LINK_SOURCE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::EMBED_SOURCE_OLE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::LINK_SOURCE_OLE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::EMBEDDED_OBJ_OLE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::STRING ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::STRING_TSVC ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::SYLK ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::LINK ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::HTML ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::HTML_SIMPLE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::DIF ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::DRAWING ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::SVXB ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::RTF ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::RICHTEXT ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::GDIMETAFILE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::PNG ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::BITMAP ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::SBA_DATAEXCHANGE ) ||
                                 IsDropFormatSupported( SotClipboardFormatId::SBA_FIELDDATAEXCHANGE ) ||
                                 ( !bMove && (
                                    IsDropFormatSupported( SotClipboardFormatId::FILE_LIST ) ||
                                     IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
                                     IsDropFormatSupported( SotClipboardFormatId::SOLK ) ||
                                     IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) ||
                                     IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK ) ||
                                     IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR ) ) ) )
                            {
                                nRet = nMyAction;
                            }
                        }
                        break;
                    case DND_ACTION_LINK:
                        if ( IsDropFormatSupported( SotClipboardFormatId::LINK_SOURCE ) ||
                             IsDropFormatSupported( SotClipboardFormatId::LINK_SOURCE_OLE ) ||
                             IsDropFormatSupported( SotClipboardFormatId::LINK ) ||
                             IsDropFormatSupported( SotClipboardFormatId::FILE_LIST ) ||
                             IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
                             IsDropFormatSupported( SotClipboardFormatId::SOLK ) ||
                             IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) ||
                             IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK ) ||
                             IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR ) )
                        {
                            nRet = nMyAction;
                        }
                        break;
                }

                if ( nRet )
                {
                    // Simple check for protection: It's not known here if the drop will result
                    // in cells or drawing objects (some formats can be both) and how many cells
                    // the result will be. But if IsFormatEditable for the drop cell position
                    // is sal_False (ignores matrix formulas), nothing can be pasted, so the drop
                    // can already be rejected here.

                    Point aPos = rEvt.maPosPixel;
                    SCCOL nPosX;
                    SCROW nPosY;
                    mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
                    SCTAB nTab = mrViewData.GetTabNo();
                    ScDocument& rDoc = mrViewData.GetDocument();

                    ScEditableTester aTester( rDoc, nTab, nPosX,nPosY, nPosX,nPosY );
                    if ( !aTester.IsFormatEditable() )
                        nRet = DND_ACTION_NONE;             // forbidden
                }
            }
        }

        //  scroll only for accepted formats
        if (nRet)
            DropScroll( rEvt.maPosPixel );
    }

    return nRet;
}

static SotClipboardFormatId lcl_GetDropFormatId( const uno::Reference<datatransfer::XTransferable>& xTransfer, bool bPreferText )
{
    TransferableDataHelper aDataHelper( xTransfer );

    if ( !aDataHelper.HasFormat( SotClipboardFormatId::SBA_DATAEXCHANGE ) )
    {
        //  use bookmark formats if no sba is present

        if ( aDataHelper.HasFormat( SotClipboardFormatId::SOLK ) )
            return SotClipboardFormatId::SOLK;
        else if ( aDataHelper.HasFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) )
            return SotClipboardFormatId::UNIFORMRESOURCELOCATOR;
        else if ( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) )
            return SotClipboardFormatId::NETSCAPE_BOOKMARK;
        else if ( aDataHelper.HasFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR ) )
            return SotClipboardFormatId::FILEGRPDESCRIPTOR;
    }

    SotClipboardFormatId nFormatId = SotClipboardFormatId::NONE;
    if ( aDataHelper.HasFormat( SotClipboardFormatId::DRAWING ) )
        nFormatId = SotClipboardFormatId::DRAWING;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SVXB ) )
        nFormatId = SotClipboardFormatId::SVXB;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE ) )
    {
        //  If it's a Writer object, insert RTF instead of OLE

        bool bDoRtf = false;
        tools::SvRef<SotTempStream> xStm;
        TransferableObjectDescriptor aObjDesc;
        if( aDataHelper.GetTransferableObjectDescriptor( SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDesc ) &&
            aDataHelper.GetSotStorageStream( SotClipboardFormatId::EMBED_SOURCE, xStm ) )
        {
            bDoRtf = ( ( aObjDesc.maClassName == SvGlobalName( SO3_SW_CLASSID ) ||
                         aObjDesc.maClassName == SvGlobalName( SO3_SWWEB_CLASSID ) )
                       && ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) || aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) ) );
        }
        if ( bDoRtf )
            nFormatId = aDataHelper.HasFormat( SotClipboardFormatId::RTF ) ? SotClipboardFormatId::RTF : SotClipboardFormatId::RICHTEXT;
        else
            nFormatId = SotClipboardFormatId::EMBED_SOURCE;
    }
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE ) )
        nFormatId = SotClipboardFormatId::LINK_SOURCE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SBA_DATAEXCHANGE ) )
        nFormatId = SotClipboardFormatId::SBA_DATAEXCHANGE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SBA_FIELDDATAEXCHANGE ) )
        nFormatId = SotClipboardFormatId::SBA_FIELDDATAEXCHANGE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::BIFF_8 ) )
        nFormatId = SotClipboardFormatId::BIFF_8;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::BIFF_5 ) )
        nFormatId = SotClipboardFormatId::BIFF_5;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE_OLE ) )
        nFormatId = SotClipboardFormatId::EMBED_SOURCE_OLE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ_OLE ) )
        nFormatId = SotClipboardFormatId::EMBEDDED_OBJ_OLE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE_OLE ) )
        nFormatId = SotClipboardFormatId::LINK_SOURCE_OLE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) )
        nFormatId = SotClipboardFormatId::RTF;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) )
        nFormatId = SotClipboardFormatId::RICHTEXT;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::HTML ) )
        nFormatId = SotClipboardFormatId::HTML;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::HTML_SIMPLE ) )
        nFormatId = SotClipboardFormatId::HTML_SIMPLE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SYLK ) )
        nFormatId = SotClipboardFormatId::SYLK;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::LINK ) )
        nFormatId = SotClipboardFormatId::LINK;
    else if ( bPreferText && aDataHelper.HasFormat( SotClipboardFormatId::STRING ) ) // #i86734# the behaviour introduced in #i62773# is wrong when pasting
        nFormatId = SotClipboardFormatId::STRING;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::FILE_LIST ) )
        nFormatId = SotClipboardFormatId::FILE_LIST;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SIMPLE_FILE ) )    // #i62773# FILE_LIST/FILE before STRING (Unix file managers)
        nFormatId = SotClipboardFormatId::SIMPLE_FILE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::STRING_TSVC ) )
        nFormatId = SotClipboardFormatId::STRING_TSVC;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) )
        nFormatId = SotClipboardFormatId::STRING;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::GDIMETAFILE ) )
        nFormatId = SotClipboardFormatId::GDIMETAFILE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::PNG ) )
        nFormatId = SotClipboardFormatId::PNG;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::BITMAP ) )
        nFormatId = SotClipboardFormatId::BITMAP;

    return nFormatId;
}

static SotClipboardFormatId lcl_GetDropLinkId( const uno::Reference<datatransfer::XTransferable>& xTransfer )
{
    TransferableDataHelper aDataHelper( xTransfer );

    SotClipboardFormatId nFormatId = SotClipboardFormatId::NONE;
    if ( aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE ) )
        nFormatId = SotClipboardFormatId::LINK_SOURCE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE_OLE ) )
        nFormatId = SotClipboardFormatId::LINK_SOURCE_OLE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::LINK ) )
        nFormatId = SotClipboardFormatId::LINK;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::FILE_LIST ) )
        nFormatId = SotClipboardFormatId::FILE_LIST;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SIMPLE_FILE ) )
        nFormatId = SotClipboardFormatId::SIMPLE_FILE;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::SOLK ) )
        nFormatId = SotClipboardFormatId::SOLK;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) )
        nFormatId = SotClipboardFormatId::UNIFORMRESOURCELOCATOR;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) )
        nFormatId = SotClipboardFormatId::NETSCAPE_BOOKMARK;
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR ) )
        nFormatId = SotClipboardFormatId::FILEGRPDESCRIPTOR;

    return nFormatId;
}

sal_Int8 ScGridWindow::ExecutePrivateDrop( const ExecuteDropEvent& rEvt )
{
    // hide drop marker
    bDragRect = false;
    UpdateDragRectOverlay();

    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();

    return DropTransferObj( rData.pCellTransfer, nDragStartX, nDragStartY,
                                PixelToLogic(rEvt.maPosPixel), rEvt.mnAction );
}

sal_Int8 ScGridWindow::DropTransferObj( ScTransferObj* pTransObj, SCCOL nDestPosX, SCROW nDestPosY,
                                        const Point& rLogicPos, sal_Int8 nDndAction )
{
    if ( !pTransObj )
        return 0;

    ScDocument* pSourceDoc = pTransObj->GetSourceDocument();
    ScDocShell* pDocSh     = mrViewData.GetDocShell();
    ScDocument& rThisDoc   = mrViewData.GetDocument();
    ScViewFunc* pView      = mrViewData.GetView();
    SCTAB       nThisTab   = mrViewData.GetTabNo();
    ScDragSrc   nFlags     = pTransObj->GetDragSourceFlags();

    bool bIsNavi = (nFlags & ScDragSrc::Navigator) == ScDragSrc::Navigator;
    bool bIsMove = ( nDndAction == DND_ACTION_MOVE && !bIsNavi );

    // workaround for wrong nDndAction on Windows when pressing solely
    // the Alt key during drag and drop;
    // can be removed after #i79215# has been fixed
    if ( meDragInsertMode != INS_NONE )
    {
        bIsMove = ( nDndAction & DND_ACTION_MOVE && !bIsNavi );
    }

    bool bIsLink = ( nDndAction == DND_ACTION_LINK );

    ScRange aSource = pTransObj->GetRange();

    //  only use visible tab from source range - when dragging within one table,
    //  all selected tables at the time of dropping are used (handled in MoveBlockTo)
    SCTAB nSourceTab = pTransObj->GetVisibleTab();
    aSource.aStart.SetTab( nSourceTab );
    aSource.aEnd.SetTab( nSourceTab );

    SCCOL nSizeX = aSource.aEnd.Col() - aSource.aStart.Col() + 1;
    SCROW nSizeY = (bIsMove ? (aSource.aEnd.Row() - aSource.aStart.Row() + 1) :
            pTransObj->GetNonFilteredRows());   // copy/link: no filtered rows
    ScRange aDest( nDestPosX, nDestPosY, nThisTab,
                   nDestPosX + nSizeX - 1, nDestPosY + nSizeY - 1, nThisTab );

    /* NOTE: AcceptPrivateDrop() already checked for filtered conditions during
     * dragging and adapted drawing of the selection frame. We check here
     * (again) because this may actually also be called from PasteSelection(),
     * we would have to duplicate determination of flags and destination range
     * and would lose the context of the "filtered destination is OK" cases
     * below, which is already awkward enough as is. */

    // Don't move filtered source.
    bool bFiltered = (bIsMove && pTransObj->HasFilteredRows());
    if (!bFiltered)
    {
        if (pSourceDoc != &rThisDoc && ((nFlags & ScDragSrc::Table) ||
                    (!bIsLink && meDragInsertMode == INS_NONE)))
        {
            // Nothing. Either entire sheet to be dropped, or the one case
            // where PasteFromClip() is to be called that handles a filtered
            // destination itself. Drag-copy from another document without
            // inserting cells.
        }
        else
            // Don't copy or move to filtered destination.
            bFiltered = ScViewUtil::HasFiltered(aDest, rThisDoc);
    }

    bool bDone = false;

    if (!bFiltered && pSourceDoc == &rThisDoc)
    {
        if (nFlags & ScDragSrc::Table) // whole sheet?
        {
            if ( rThisDoc.IsDocEditable() )
            {
                SCTAB nSrcTab = aSource.aStart.Tab();
                mrViewData.GetDocShell()->MoveTable( nSrcTab, nThisTab, !bIsMove, true );   // with Undo
                pView->SetTabNo( nThisTab, true );
                bDone = true;
            }
        }
        else                                        // move/copy block
        {
            OUString aChartName;
            if (rThisDoc.HasChartAtPoint( nThisTab, rLogicPos, aChartName ))
            {
                OUString aRangeName(aSource.Format(rThisDoc, ScRefFlags::RANGE_ABS_3D,
                            rThisDoc.GetAddressConvention()));
                SfxStringItem aNameItem( SID_CHART_NAME, aChartName );
                SfxStringItem aRangeItem( SID_CHART_SOURCE, aRangeName );
                sal_uInt16 nId = bIsMove ? SID_CHART_SOURCE : SID_CHART_ADDSOURCE;
                mrViewData.GetDispatcher().ExecuteList(nId,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aRangeItem, &aNameItem });
                bDone = true;
            }
            else if ( rThisDoc.GetDPAtCursor( nDestPosX, nDestPosY, nThisTab ) )
            {
                // drop on DataPilot table: try to sort, fail if that isn't possible

                ScAddress aDestPos( nDestPosX, nDestPosY, nThisTab );
                if ( aDestPos != aSource.aStart )
                    bDone = mrViewData.GetView()->DataPilotMove( aSource, aDestPos );
                else
                    bDone = true;   // same position: nothing
            }
            else if ( nDestPosX != aSource.aStart.Col() || nDestPosY != aSource.aStart.Row() ||
                        nSourceTab != nThisTab )
            {
                OUString aUndo = ScResId( bIsMove ? STR_UNDO_MOVE : STR_UNDO_COPY );
                pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, mrViewData.GetViewShell()->GetViewShellId() );

                SCCOL nCorrectCursorPosCol = 0;
                SCROW nCorrectCursorPosRow = 0;

                bDone = true;
                if ( meDragInsertMode != INS_NONE )
                {
                    // call with bApi = sal_True to avoid error messages in drop handler
                    bDone = pDocSh->GetDocFunc().InsertCells( aDest, nullptr, meDragInsertMode, true /*bRecord*/, true /*bApi*/, true /*bPartOfPaste*/ );
                    if ( bDone )
                    {
                        if ( nThisTab == nSourceTab )
                        {
                            if ( meDragInsertMode == INS_CELLSDOWN &&
                                 nDestPosX == aSource.aStart.Col() && nDestPosY < aSource.aStart.Row() )
                            {
                                ScRange aErrorRange( ScAddress::UNINITIALIZED );
                                bDone = aSource.Move( 0, nSizeY, 0, aErrorRange, pSourceDoc );
                                nCorrectCursorPosRow = nSizeY;
                            }
                            else if ( meDragInsertMode == INS_CELLSRIGHT &&
                                      nDestPosY == aSource.aStart.Row() && nDestPosX < aSource.aStart.Col() )
                            {
                                ScRange aErrorRange( ScAddress::UNINITIALIZED );
                                bDone = aSource.Move( nSizeX, 0, 0, aErrorRange, pSourceDoc );
                                nCorrectCursorPosCol = nSizeX;
                            }
                        }
                        pDocSh->UpdateOle(mrViewData);
                        pView->CellContentChanged();
                    }
                }

                if ( bDone )
                {
                    if ( bIsLink )
                    {
                        bDone = pView->LinkBlock( aSource, aDest.aStart );
                    }
                    else
                    {
                        bDone = pView->MoveBlockTo( aSource, aDest.aStart, bIsMove );
                    }
                }

                if ( bDone && meDragInsertMode != INS_NONE && bIsMove && nThisTab == nSourceTab )
                {
                    DelCellCmd eCmd = DelCellCmd::NONE;
                    if ( meDragInsertMode == INS_CELLSDOWN )
                    {
                        eCmd = DelCellCmd::CellsUp;
                    }
                    else if ( meDragInsertMode == INS_CELLSRIGHT )
                    {
                        eCmd = DelCellCmd::CellsLeft;
                    }

                    if ( ( eCmd == DelCellCmd::CellsUp  && nDestPosX == aSource.aStart.Col() ) ||
                         ( eCmd == DelCellCmd::CellsLeft && nDestPosY == aSource.aStart.Row() ) )
                    {
                        // call with bApi = sal_True to avoid error messages in drop handler
                        bDone = pDocSh->GetDocFunc().DeleteCells( aSource, nullptr, eCmd, true /*bApi*/ );
                        if ( bDone )
                        {
                            if ( eCmd == DelCellCmd::CellsUp && nDestPosY > aSource.aEnd.Row() )
                            {
                                ScRange aErrorRange( ScAddress::UNINITIALIZED );
                                bDone = aDest.Move( 0, -nSizeY, 0, aErrorRange, &rThisDoc );
                            }
                            else if ( eCmd == DelCellCmd::CellsLeft && nDestPosX > aSource.aEnd.Col() )
                            {
                                ScRange aErrorRange( ScAddress::UNINITIALIZED );
                                bDone = aDest.Move( -nSizeX, 0, 0, aErrorRange, &rThisDoc );
                            }
                            pDocSh->UpdateOle(mrViewData);
                            pView->CellContentChanged();
                        }
                    }
                }

                if ( bDone )
                {
                    pView->MarkRange( aDest, false );

                    SCCOL nDCol;
                    SCROW nDRow;
                    if (pTransObj->WasSourceCursorInSelection())
                    {
                        nDCol = pTransObj->GetSourceCursorX() - aSource.aStart.Col() + nCorrectCursorPosCol;
                        nDRow = pTransObj->GetSourceCursorY() - aSource.aStart.Row() + nCorrectCursorPosRow;
                    }
                    else
                    {
                        nDCol = 0;
                        nDRow = 0;
                    }
                    pView->SetCursor( aDest.aStart.Col() + nDCol, aDest.aStart.Row() + nDRow );
                }

                pDocSh->GetUndoManager()->LeaveListAction();

            }
            else
                bDone = true;       // nothing to do
        }

        if (bDone)
            pTransObj->SetDragWasInternal();    // don't delete source in DragFinished
    }
    else if ( !bFiltered && pSourceDoc )                        // between documents
    {
        if (nFlags & ScDragSrc::Table)          // copy/link sheets between documents
        {
            if ( rThisDoc.IsDocEditable() )
            {
                ScDocShell* pSrcShell = pTransObj->GetSourceDocShell();

                std::vector<SCTAB> nTabs;

                ScMarkData  aMark       = pTransObj->GetSourceMarkData();
                SCTAB       nTabCount   = pSourceDoc->GetTableCount();

                for(SCTAB i=0; i<nTabCount; i++)
                {
                    if(aMark.GetTableSelect(i))
                    {
                        nTabs.push_back(i);
                        for(SCTAB j=i+1;j<nTabCount;j++)
                        {
                            if((!pSourceDoc->IsVisible(j))&&(pSourceDoc->IsScenario(j)))
                            {
                                nTabs.push_back( j );
                                i=j;
                            }
                            else break;
                        }
                    }
                }

                pView->ImportTables( pSrcShell,static_cast<SCTAB>(nTabs.size()), nTabs.data(), bIsLink, nThisTab );
                bDone = true;
            }
        }
        else if ( bIsLink )
        {
            //  as in PasteDDE
            //  (external references might be used instead?)

            SfxObjectShell* pSourceSh = pSourceDoc->GetDocumentShell();
            OSL_ENSURE(pSourceSh, "drag document has no shell");
            if (pSourceSh)
            {
                OUString aUndo = ScResId( STR_UNDO_COPY );
                pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, mrViewData.GetViewShell()->GetViewShellId() );

                bDone = true;
                if ( meDragInsertMode != INS_NONE )
                {
                    // call with bApi = sal_True to avoid error messages in drop handler
                    bDone = pDocSh->GetDocFunc().InsertCells( aDest, nullptr, meDragInsertMode, true /*bRecord*/, true /*bApi*/, true /*bPartOfPaste*/ );
                    if ( bDone )
                    {
                        pDocSh->UpdateOle(mrViewData);
                        pView->CellContentChanged();
                    }
                }

                if ( bDone )
                {
                    OUString aApp = Application::GetAppName();
                    OUString aTopic = pSourceSh->GetTitle( SFX_TITLE_FULLNAME );
                    OUString aItem(aSource.Format(*pSourceDoc, ScRefFlags::VALID | ScRefFlags::TAB_3D));

                    // TODO: we could define ocQuote for "
                    const OUString aQuote('"');
                    const OUString& sSep = ScCompiler::GetNativeSymbol( ocSep);
                    OUString aFormula =
                            "=" +
                            ScCompiler::GetNativeSymbol(ocDde) +
                            ScCompiler::GetNativeSymbol(ocOpen) +
                            aQuote +
                            aApp +
                            aQuote +
                            sSep +
                            aQuote +
                            aTopic +
                            aQuote +
                            sSep +
                            aQuote +
                            aItem +
                            aQuote +
                            ScCompiler::GetNativeSymbol(ocClose);

                    pView->DoneBlockMode();
                    pView->InitBlockMode( nDestPosX, nDestPosY, nThisTab );
                    pView->MarkCursor( nDestPosX + nSizeX - 1,
                                       nDestPosY + nSizeY - 1, nThisTab );

                    pView->EnterMatrix( aFormula, ::formula::FormulaGrammar::GRAM_NATIVE );

                    pView->MarkRange( aDest, false );
                    pView->SetCursor( aDest.aStart.Col(), aDest.aStart.Row() );
                }

                pDocSh->GetUndoManager()->LeaveListAction();
            }
        }
        else
        {
            //! HasSelectedBlockMatrixFragment without selected sheet?
            //! or don't start dragging on a part of a matrix

            OUString aUndo = ScResId( bIsMove ? STR_UNDO_MOVE : STR_UNDO_COPY );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, mrViewData.GetViewShell()->GetViewShellId() );

            bDone = true;
            if ( meDragInsertMode != INS_NONE )
            {
                // call with bApi = sal_True to avoid error messages in drop handler
                bDone = pDocSh->GetDocFunc().InsertCells( aDest, nullptr, meDragInsertMode, true /*bRecord*/, true /*bApi*/, true /*bPartOfPaste*/ );
                if ( bDone )
                {
                    pDocSh->UpdateOle(mrViewData);
                    pView->CellContentChanged();
                }
            }

            if ( bDone )
            {
                pView->Unmark();  // before SetCursor, so CheckSelectionTransfer isn't called with a selection
                pView->SetCursor( nDestPosX, nDestPosY );
                bDone = pView->PasteFromClip( InsertDeleteFlags::ALL, pTransObj->GetDocument() );  // clip-doc
                if ( bDone )
                {
                    pView->MarkRange( aDest, false );
                    pView->SetCursor( aDest.aStart.Col(), aDest.aStart.Row() );
                }
            }

            pDocSh->GetUndoManager()->LeaveListAction();

            //  no longer call ResetMark here - the inserted block has been selected
            //  and may have been copied to primary selection
        }
    }

    sal_Int8 nRet = bDone ? nDndAction : DND_ACTION_NONE;
    return nRet;
}

sal_Int8 ScGridWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    DrawMarkDropObj( nullptr );    // drawing layer

    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    if (rData.pCellTransfer)
        return ExecutePrivateDrop( rEvt );

    Point aPos = rEvt.maPosPixel;

    if ( !rData.aLinkDoc.isEmpty() )
    {
        //  try to insert a link

        bool bOk = true;
        OUString aThisName;
        ScDocShell* pDocSh = mrViewData.GetDocShell();
        if (pDocSh && pDocSh->HasName())
            aThisName = pDocSh->GetMedium()->GetName();

        if ( rData.aLinkDoc == aThisName )              // error - no link within a document
            bOk = false;
        else
        {
            ScViewFunc* pView = mrViewData.GetView();
            if ( !rData.aLinkTable.isEmpty() )
                pView->InsertTableLink( rData.aLinkDoc, OUString(), OUString(),
                                        rData.aLinkTable );
            else if ( !rData.aLinkArea.isEmpty() )
            {
                SCCOL  nPosX;
                SCROW  nPosY;
                mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
                pView->MoveCursorAbs( nPosX, nPosY, SC_FOLLOW_NONE, false, false );

                pView->InsertAreaLink( rData.aLinkDoc, OUString(), OUString(),
                                        rData.aLinkArea );
            }
            else
            {
                OSL_FAIL("drop with link: no sheet nor area");
                bOk = false;
            }
        }

        return bOk ? rEvt.mnAction : DND_ACTION_NONE;           // don't try anything else
    }

    Point aLogicPos = PixelToLogic(aPos);
    bool bIsLink = ( rEvt.mnAction == DND_ACTION_LINK );

    if (!bIsLink && rData.pDrawTransfer)
    {
        ScDragSrc nFlags = rData.pDrawTransfer->GetDragSourceFlags();

        bool bIsNavi = (nFlags & ScDragSrc::Navigator) == ScDragSrc::Navigator;
        bool bIsMove = ( rEvt.mnAction == DND_ACTION_MOVE && !bIsNavi );

        bPasteIsMove = bIsMove;

        mrViewData.GetView()->PasteDraw(
            aLogicPos, rData.pDrawTransfer->GetModel(), false, u"A", u"B");

        if (bPasteIsMove)
            rData.pDrawTransfer->SetDragWasInternal();
        bPasteIsMove = false;

        return rEvt.mnAction;
    }

    SCCOL  nPosX;
    SCROW  nPosY;
    mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

    if (!rData.aJumpTarget.isEmpty())
    {
        //  internal bookmark (from Navigator)
        //  bookmark clipboard formats are in PasteScDataObject

        if ( !rData.pJumpLocalDoc || rData.pJumpLocalDoc == &mrViewData.GetDocument() )
        {
            mrViewData.GetViewShell()->InsertBookmark( rData.aJumpText, rData.aJumpTarget,
                                                        nPosX, nPosY );
            return rEvt.mnAction;
        }
    }

    ScDocument& rThisDoc = mrViewData.GetDocument();
    SdrObject* pHitObj = rThisDoc.GetObjectAtPoint( mrViewData.GetTabNo(), PixelToLogic(aPos) );
    if ( pHitObj && bIsLink )
    {
        //  dropped on drawing object
        //  PasteOnDrawObjectLinked checks for valid formats
        if ( mrViewData.GetView()->PasteOnDrawObjectLinked( rEvt.maDropEvent.Transferable, *pHitObj ) )
            return rEvt.mnAction;
    }

    bool bDone = false;

    SotClipboardFormatId nFormatId = bIsLink ?
                        lcl_GetDropLinkId( rEvt.maDropEvent.Transferable ) :
                        lcl_GetDropFormatId( rEvt.maDropEvent.Transferable, false );
    if ( nFormatId != SotClipboardFormatId::NONE )
    {
        pScMod->SetInExecuteDrop( true );   // #i28468# prevent error messages from PasteDataFormat
        bDone = mrViewData.GetView()->PasteDataFormat(
                    nFormatId, rEvt.maDropEvent.Transferable, nPosX, nPosY, &aLogicPos, bIsLink );
        pScMod->SetInExecuteDrop( false );
    }

    sal_Int8 nRet = bDone ? rEvt.mnAction : DND_ACTION_NONE;
    return nRet;
}

void ScGridWindow::PasteSelection( const Point& rPosPixel )
{
    Point aLogicPos = PixelToLogic( rPosPixel );

    SCCOL  nPosX;
    SCROW  nPosY;
    mrViewData.GetPosFromPixel( rPosPixel.X(), rPosPixel.Y(), eWhich, nPosX, nPosY );

    // If the mouse down was inside a visible note window, ignore it and
    // leave it up to the ScPostIt to handle it
    SdrView* pDrawView = mrViewData.GetViewShell()->GetScDrawView();
    if (pDrawView)
    {
        const size_t nCount = pDrawView->GetMarkedObjectCount();
        for (size_t i = 0; i < nCount; ++i)
        {
            SdrObject* pObj = pDrawView->GetMarkedObjectByIndex(i);
            if (pObj && pObj->GetLogicRect().Contains(aLogicPos))
            {
                // Inside an active drawing object.  Bail out.
                return;
            }
        }
    }

    ScSelectionTransferObj* pOwnSelection = SC_MOD()->GetSelectionTransfer();
    if ( pOwnSelection )
    {
        //  within Calc

        // keep a reference to the data in case the selection is changed during paste
        rtl::Reference<ScTransferObj> pCellTransfer = pOwnSelection->GetCellData();
        if ( pCellTransfer )
        {
            DropTransferObj( pCellTransfer.get(), nPosX, nPosY, aLogicPos, DND_ACTION_COPY );
        }
        else
        {
            // keep a reference to the data in case the selection is changed during paste
            rtl::Reference<ScDrawTransferObj> pDrawTransfer = pOwnSelection->GetDrawData();
            if ( pDrawTransfer )
            {
                //  bSameDocClipboard argument for PasteDraw is needed
                //  because only DragData is checked directly inside PasteDraw
                mrViewData.GetView()->PasteDraw(
                    aLogicPos, pDrawTransfer->GetModel(), false,
                    pDrawTransfer->GetShellID(), SfxObjectShell::CreateShellID(mrViewData.GetDocShell()));
            }
        }
    }
    else
    {
        //  get selection from system
        TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromPrimarySelection());
        const uno::Reference<datatransfer::XTransferable>& xTransferable = aDataHelper.GetTransferable();
        if ( xTransferable.is() )
        {
            SotClipboardFormatId nFormatId = lcl_GetDropFormatId( xTransferable, true );
            if ( nFormatId != SotClipboardFormatId::NONE )
                mrViewData.GetView()->PasteDataFormat( nFormatId, xTransferable, nPosX, nPosY, &aLogicPos );
        }
    }
}

void ScGridWindow::UpdateEditViewPos()
{
    if (!mrViewData.HasEditView(eWhich))
        return;

    EditView* pView;
    SCCOL nCol;
    SCROW nRow;
    mrViewData.GetEditView( eWhich, pView, nCol, nRow );
    SCCOL nEndCol = mrViewData.GetEditEndCol();
    SCROW nEndRow = mrViewData.GetEditEndRow();

    //  hide EditView?

    bool bHide = ( nEndCol<mrViewData.GetPosX(eHWhich) || nEndRow<mrViewData.GetPosY(eVWhich) );
    if ( SC_MOD()->IsFormulaMode() )
        if ( mrViewData.GetTabNo() != mrViewData.GetRefTabNo() )
            bHide = true;

    if (bHide)
    {
        tools::Rectangle aRect = pView->GetOutputArea();
        tools::Long nHeight = aRect.Bottom() - aRect.Top();
        aRect.SetTop( PixelToLogic(GetOutputSizePixel(), mrViewData.GetLogicMode()).
                        Height() * 2 );
        aRect.SetBottom( aRect.Top() + nHeight );
        pView->SetOutputArea( aRect );
        pView->HideCursor();
    }
    else
    {
        // bForceToTop = sal_True for editing
        tools::Rectangle aPixRect = mrViewData.GetEditArea( eWhich, nCol, nRow, this, nullptr, true );

        if (comphelper::LibreOfficeKit::isActive() &&
            comphelper::LibreOfficeKit::isCompatFlagSet(
                comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
        {
            tools::Rectangle aPTwipsRect = mrViewData.GetEditArea(eWhich, nCol, nRow, this, nullptr,
                    true, true /* bInPrintTwips */);
            tools::Rectangle aOutputAreaPTwips = pView->GetLOKSpecialOutputArea();
            aOutputAreaPTwips.SetPos(aPTwipsRect.TopLeft());
            pView->SetLOKSpecialOutputArea(aOutputAreaPTwips);
        }

        Point aScrPos = PixelToLogic( aPixRect.TopLeft(), mrViewData.GetLogicMode() );

        tools::Rectangle aRect = pView->GetOutputArea();
        aRect.SetPos( aScrPos );
        pView->SetOutputArea( aRect );
        pView->ShowCursor();
    }
}

void ScGridWindow::ScrollPixel( tools::Long nDifX, tools::Long nDifY )
{
    ClickExtern();
    HideNoteMarker();

    SetMapMode(MapMode(MapUnit::MapPixel));
    Scroll( nDifX, nDifY, ScrollFlags::Children );
    SetMapMode( GetDrawMapMode() );             // generated shifted MapMode

    UpdateEditViewPos();

    DrawAfterScroll();
}

// Update Formulas ------------------------------------------------------

void ScGridWindow::UpdateFormulas(SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2)
{
    if (mrViewData.GetView()->IsMinimized())
        return;

    if ( nPaintCount )
    {
        // Do not start, switched to paint
        //  (then at least the MapMode would no longer be right)

        bNeedsRepaint = true;           // -> at end of paint run Invalidate on all
        aRepaintPixel = tools::Rectangle();    // All
        return;
    }

    if ( comphelper::LibreOfficeKit::isActive() )
    {
        ScTabViewShell* pViewShell = mrViewData.GetViewShell();
        if (nX1 < 0)
            nX1 = pViewShell->GetLOKStartHeaderCol() + 1;
        if (nY1 < 0)
            nY1 = pViewShell->GetLOKStartHeaderRow() + 1;
        if (nX2 < 0)
            nX2 = pViewShell->GetLOKEndHeaderCol();
        if (nY2 < 0)
            nY2 = pViewShell->GetLOKEndHeaderRow();

        if (nX1 < 0 || nY1 < 0) return;
    }
    else
    {
        nX1 = mrViewData.GetPosX( eHWhich );
        nY1 = mrViewData.GetPosY( eVWhich );
        nX2 = nX1 + mrViewData.VisibleCellsX( eHWhich );
        nY2 = nY1 + mrViewData.VisibleCellsY( eVWhich );
    }

    if (nX2 < nX1) nX2 = nX1;
    if (nY2 < nY1) nY2 = nY1;

    ScDocument& rDoc = mrViewData.GetDocument();

    if (nX2 > rDoc.MaxCol()) nX2 = rDoc.MaxCol();
    if (nY2 > rDoc.MaxRow()) nY2 = rDoc.MaxRow();

    // Draw( nX1, nY1, nX2, nY2, SC_UPDATE_CHANGED );

    // don't draw directly - instead use OutputData to find changed area and invalidate

    SCROW nPosY = nY1;

    SCTAB nTab = mrViewData.GetTabNo();

    if ( !comphelper::LibreOfficeKit::isActive() )
    {
        rDoc.ExtendHidden( nX1, nY1, nX2, nY2, nTab );
    }

    Point aScrPos = mrViewData.GetScrPos( nX1, nY1, eWhich );
    tools::Long nMirrorWidth = GetSizePixel().Width();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
    if ( bLayoutRTL )
    {
        tools::Long nEndPixel = mrViewData.GetScrPos( nX2+1, nPosY, eWhich ).X();
        nMirrorWidth = aScrPos.X() - nEndPixel;
        aScrPos.setX( nEndPixel + 1 );
    }

    tools::Long nScrX = aScrPos.X();
    tools::Long nScrY = aScrPos.Y();

    double nPPTX = mrViewData.GetPPTX();
    double nPPTY = mrViewData.GetPPTY();

    ScTableInfo aTabInfo;
    rDoc.FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nTab, nPPTX, nPPTY, false, false );

    Fraction aZoomX = mrViewData.GetZoomX();
    Fraction aZoomY = mrViewData.GetZoomY();
    ScOutputData aOutputData( GetOutDev(), OUTTYPE_WINDOW, aTabInfo, &rDoc, nTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nPPTX, nPPTY,
                                &aZoomX, &aZoomY );
    aOutputData.SetMirrorWidth( nMirrorWidth );

    aOutputData.FindChanged();

    // #i122149# do not use old GetChangedArea() which used polygon-based Regions, but use
    // the region-band based new version; anyways, only rectangles are added
    vcl::Region aChangedRegion( aOutputData.GetChangedAreaRegion() );   // logic (PixelToLogic)
    if(!aChangedRegion.IsEmpty())
    {
        Invalidate(aChangedRegion);
    }

    CheckNeedsRepaint();    // #i90362# used to be called via Draw() - still needed here
}

void ScGridWindow::UpdateAutoFillMark(bool bMarked, const ScRange& rMarkRange)
{
    if ( bMarked != bAutoMarkVisible || ( bMarked && rMarkRange.aEnd != aAutoMarkPos ) )
    {
        bAutoMarkVisible = bMarked;
        if ( bMarked )
            aAutoMarkPos = rMarkRange.aEnd;

        UpdateAutoFillOverlay();
    }
}

void ScGridWindow::updateLOKInputHelp(const OUString& title, const OUString& content) const
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();

    boost::property_tree::ptree aTree;
    aTree.put("title", title);
    aTree.put("content", content);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_VALIDITY_INPUT_HELP, aStream.str().c_str());
}

void ScGridWindow::updateLOKValListButton( bool bVisible, const ScAddress& rPos ) const
{
    SCCOL nX = rPos.Col();
    SCROW nY = rPos.Row();
    std::stringstream ss;
    ss << nX << ", " << nY << ", " << static_cast<unsigned int>(bVisible);
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_VALIDITY_LIST_BUTTON, ss.str().c_str());
}

void ScGridWindow::notifyKitCellFollowJump( ) const
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();

    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_SC_FOLLOW_JUMP, getCellCursor().getStr());
}

void ScGridWindow::UpdateListValPos( bool bVisible, const ScAddress& rPos )
{
    bool bOldButton = bListValButton;
    ScAddress aOldPos = aListValPos;

    bListValButton = bVisible;
    aListValPos = rPos;

    if ( bListValButton )
    {
        if ( !bOldButton || aListValPos != aOldPos )
        {
            // paint area of new button
            if ( comphelper::LibreOfficeKit::isActive() )
            {
                updateLOKValListButton( true, aListValPos );
            }
            else
            {
                Invalidate( PixelToLogic( GetListValButtonRect( aListValPos ) ) );
            }
        }
    }
    if ( !bOldButton )
        return;

    if ( !bListValButton || aListValPos != aOldPos )
    {
        // paint area of old button
        if ( comphelper::LibreOfficeKit::isActive() )
        {
            updateLOKValListButton( false, aOldPos );
        }
        else
        {
            Invalidate( PixelToLogic( GetListValButtonRect( aOldPos ) ) );
        }
    }
}

void ScGridWindow::HideCursor()
{
    ++nCursorHideCount;
}

void ScGridWindow::ShowCursor()
{
    --nCursorHideCount;
}

void ScGridWindow::GetFocus()
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    pViewShell->SetFormShellAtTop( false );     // focus in GridWindow -> FormShell no longer on top

    if (pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility(ScAccGridWinFocusGotHint(eWhich));

    if ( !SC_MOD()->IsFormulaMode() )
    {
        pViewShell->UpdateInputHandler();
//      StopMarking();      // If Dialog (error), because then no ButtonUp
                            // MO: only when not in RefInput mode
                            //     -> GetFocus/MouseButtonDown order on Mac
    }

    mrViewData.GetDocShell()->CheckConfigOptions();
    Window::GetFocus();
}

void ScGridWindow::LoseFocus()
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();

    if (pViewShell && pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility(ScAccGridWinFocusLostHint(eWhich));

    Window::LoseFocus();
}

bool ScGridWindow::HitRangeFinder( const Point& rMouse, RfCorner& rCorner,
                                sal_uInt16* pIndex, SCCOL* pAddX, SCROW* pAddY)
{
    bool bFound = false;
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( mrViewData.GetViewShell() );
    if (pHdl)
    {
        ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
        if ( pRangeFinder && !pRangeFinder->IsHidden() &&
                pRangeFinder->GetDocName() == mrViewData.GetDocShell()->GetTitle() )
        {
            ScDocument& rDoc = mrViewData.GetDocument();
            SCTAB nTab = mrViewData.GetTabNo();
            bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
            tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

            SCCOL nPosX;
            SCROW nPosY;
            mrViewData.GetPosFromPixel( rMouse.X(), rMouse.Y(), eWhich, nPosX, nPosY );
            //  merged (single/Range) ???
            ScAddress aAddr( nPosX, nPosY, nTab );

            Point aCellStart = mrViewData.GetScrPos( nPosX, nPosY, eWhich, true );
            Point aCellEnd = aCellStart;
            tools::Long nSizeXPix;
            tools::Long nSizeYPix;
            mrViewData.GetMergeSizePixel( nPosX, nPosY, nSizeXPix, nSizeYPix );

            aCellEnd.AdjustX(nSizeXPix * nLayoutSign );
            aCellEnd.AdjustY(nSizeYPix );

            bool bCornerHorizontalRight;
            bool bCornerHorizontalLeft;
            if ( bLayoutRTL )
            {
                bCornerHorizontalRight = ( rMouse.X() >= aCellEnd.X()       && rMouse.X() <= aCellEnd.X() + 8 );
                bCornerHorizontalLeft  = ( rMouse.X() >= aCellStart.X() - 8 && rMouse.X() <= aCellStart.X() );
            }
            else
            {
                bCornerHorizontalRight = ( rMouse.X() >= aCellEnd.X() - 8 && rMouse.X() <= aCellEnd.X() );
                bCornerHorizontalLeft  = ( rMouse.X() >= aCellStart.X()   && rMouse.X() <= aCellStart.X() + 8 );
            }

            bool bCornerVerticalDown = rMouse.Y() >= aCellEnd.Y() - 8  && rMouse.Y() <= aCellEnd.Y();
            bool bCornerVerticalUp   = rMouse.Y() >= aCellStart.Y()    && rMouse.Y() <= aCellStart.Y() + 8;

            //  corner is hit only if the mouse is within the cell
            sal_uInt16 nCount = static_cast<sal_uInt16>(pRangeFinder->Count());
            for (sal_uInt16 i=nCount; i;)
            {
                //  search backwards so that the last repainted frame is found
                --i;
                ScRangeFindData& rData = pRangeFinder->GetObject(i);
                if ( rData.aRef.Contains(aAddr) )
                {
                    if (pIndex)
                        *pIndex = i;
                    if (pAddX)
                        *pAddX = nPosX - rData.aRef.aStart.Col();
                    if (pAddY)
                        *pAddY = nPosY - rData.aRef.aStart.Row();

                    bFound = true;

                    rCorner = NONE;

                    ScAddress aEnd = rData.aRef.aEnd;
                    ScAddress aStart = rData.aRef.aStart;

                    if ( bCornerHorizontalLeft && bCornerVerticalUp &&
                         aAddr == aStart)
                    {
                        rCorner = LEFT_UP;
                    }
                    else if (bCornerHorizontalRight && bCornerVerticalDown &&
                             aAddr == aEnd)
                    {
                        rCorner = RIGHT_DOWN;
                    }
                    else if (bCornerHorizontalRight && bCornerVerticalUp &&
                             aAddr == ScAddress(aEnd.Col(), aStart.Row(), aStart.Tab()))
                    {
                        rCorner = RIGHT_UP;
                    }
                    else if (bCornerHorizontalLeft && bCornerVerticalDown &&
                             aAddr == ScAddress(aStart.Col(), aEnd.Row(), aStart.Tab()))
                    {
                        rCorner = LEFT_DOWN;
                    }
                    break;
                }
            }
        }
    }
    return bFound;
}

#define SCE_TOP     1
#define SCE_BOTTOM  2
#define SCE_LEFT    4
#define SCE_RIGHT   8
#define SCE_ALL     15

static void lcl_PaintOneRange( ScDocShell* pDocSh, const ScRange& rRange, sal_uInt16 nEdges )
{
    // the range is always properly orientated

    SCCOL nCol1 = rRange.aStart.Col();
    SCROW nRow1 = rRange.aStart.Row();
    SCTAB nTab1 = rRange.aStart.Tab();
    SCCOL nCol2 = rRange.aEnd.Col();
    SCROW nRow2 = rRange.aEnd.Row();
    SCTAB nTab2 = rRange.aEnd.Tab();
    bool bHiddenEdge = false;
    SCROW nTmp;

    ScDocument& rDoc = pDocSh->GetDocument();
    while ( nCol1 > 0 && rDoc.ColHidden(nCol1, nTab1) )
    {
        --nCol1;
        bHiddenEdge = true;
    }
    while ( nCol2 < rDoc.MaxCol() && rDoc.ColHidden(nCol2, nTab1) )
    {
        ++nCol2;
        bHiddenEdge = true;
    }
    nTmp = rDoc.FirstVisibleRow(0, nRow1, nTab1);
    if (!rDoc.ValidRow(nTmp))
        nTmp = 0;
    if (nTmp < nRow1)
    {
        nRow1 = nTmp;
        bHiddenEdge = true;
    }
    nTmp = rDoc.FirstVisibleRow(nRow2, rDoc.MaxRow(), nTab1);
    if (!rDoc.ValidRow(nTmp))
        nTmp = rDoc.MaxRow();
    if (nTmp > nRow2)
    {
        nRow2 = nTmp;
        bHiddenEdge = true;
    }

    if ( nCol2 > nCol1 + 1 && nRow2 > nRow1 + 1 && !bHiddenEdge )
    {
        // Only along the edges (The corners are hit twice)
        if ( nEdges & SCE_TOP )
            pDocSh->PostPaint( nCol1, nRow1, nTab1, nCol2, nRow1, nTab2, PaintPartFlags::Marks );
        if ( nEdges & SCE_LEFT )
            pDocSh->PostPaint( nCol1, nRow1, nTab1, nCol1, nRow2, nTab2, PaintPartFlags::Marks );
        if ( nEdges & SCE_RIGHT )
            pDocSh->PostPaint( nCol2, nRow1, nTab1, nCol2, nRow2, nTab2, PaintPartFlags::Marks );
        if ( nEdges & SCE_BOTTOM )
            pDocSh->PostPaint( nCol1, nRow2, nTab1, nCol2, nRow2, nTab2, PaintPartFlags::Marks );
    }
    else    // everything in one call
        pDocSh->PostPaint( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, PaintPartFlags::Marks );
}

static void lcl_PaintRefChanged( ScDocShell* pDocSh, const ScRange& rOldUn, const ScRange& rNewUn )
{
    //  Repaint for the parts of the frame in old, which in are no more in New

    ScRange aOld = rOldUn;
    ScRange aNew = rNewUn;
    aOld.PutInOrder();
    aNew.PutInOrder();

    if ( aOld.aStart == aOld.aEnd )                 //! Ignore sheet ?
        pDocSh->GetDocument().ExtendMerge(aOld);
    if ( aNew.aStart == aNew.aEnd )                 //! Ignore sheet ?
        pDocSh->GetDocument().ExtendMerge(aNew);

    SCCOL nOldCol1 = aOld.aStart.Col();
    SCROW nOldRow1 = aOld.aStart.Row();
    SCCOL nOldCol2 = aOld.aEnd.Col();
    SCROW nOldRow2 = aOld.aEnd.Row();
    SCCOL nNewCol1 = aNew.aStart.Col();
    SCROW nNewRow1 = aNew.aStart.Row();
    SCCOL nNewCol2 = aNew.aEnd.Col();
    SCROW nNewRow2 = aNew.aEnd.Row();
    SCTAB nTab1 = aOld.aStart.Tab();        // sheet is not changed
    SCTAB nTab2 = aOld.aEnd.Tab();

    if ( nNewRow2 < nOldRow1 || nNewRow1 > nOldRow2 ||
         nNewCol2 < nOldCol1 || nNewCol1 > nOldCol2 ||
         ( nNewCol1 != nOldCol1 && nNewRow1 != nOldRow1 &&
           nNewCol2 != nOldCol2 && nNewRow2 != nOldRow2 ) )
    {
        // Completely removed or changed all sides
        // (check <= instead of <  goes wrong for single rows/columns)

        lcl_PaintOneRange( pDocSh, aOld, SCE_ALL );
    }
    else        // Test all four corners separately
    {
        // upper part
        if ( nNewRow1 < nOldRow1 )                  // only delete upper line
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nOldCol2, nOldRow1, nTab2 ), SCE_ALL );
        else if ( nNewRow1 > nOldRow1 )             // the upper part which is will be removed
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nOldCol2, nNewRow1-1, nTab2 ),
                    SCE_ALL &~ SCE_BOTTOM );

        //  bottom part
        if ( nNewRow2 > nOldRow2 )                  // only delete bottom line
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow2, nTab1, nOldCol2, nOldRow2, nTab2 ), SCE_ALL );
        else if ( nNewRow2 < nOldRow2 )             // the bottom part which is will be removed
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nNewRow2+1, nTab1, nOldCol2, nOldRow2, nTab2 ),
                    SCE_ALL &~ SCE_TOP );

        //  left part
        if ( nNewCol1 < nOldCol1 )                  // only delete left line
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nOldCol1, nOldRow2, nTab2 ), SCE_ALL );
        else if ( nNewCol1 > nOldCol1 )             // the left part which is will be removed
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol1, nOldRow1, nTab1, nNewCol1-1, nOldRow2, nTab2 ),
                    SCE_ALL &~ SCE_RIGHT );

        // right part
        if ( nNewCol2 > nOldCol2 )                  // only delete right line
            lcl_PaintOneRange( pDocSh, ScRange(
                    nOldCol2, nOldRow1, nTab1, nOldCol2, nOldRow2, nTab2 ), SCE_ALL );
        else if ( nNewCol2 < nOldCol2 )             // the right part which is will be removed
            lcl_PaintOneRange( pDocSh, ScRange(
                    nNewCol2+1, nOldRow1, nTab1, nOldCol2, nOldRow2, nTab2 ),
                    SCE_ALL &~ SCE_LEFT );
    }
}

void ScGridWindow::RFMouseMove( const MouseEvent& rMEvt, bool bUp )
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( mrViewData.GetViewShell() );
    if (!pHdl)
        return;
    ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
    if (!pRangeFinder || nRFIndex >= pRangeFinder->Count())
        return;
    ScRangeFindData& rData = pRangeFinder->GetObject( nRFIndex );

    // Mouse pointer

    if (bRFSize)
        SetPointer( PointerStyle::Cross );
    else
        SetPointer( PointerStyle::Hand );

    //  Scrolling

    bool bTimer = false;
    Point aPos = rMEvt.GetPosPixel();
    SCCOL nDx = 0;
    SCROW nDy = 0;
    if ( aPos.X() < 0 ) nDx = -1;
    if ( aPos.Y() < 0 ) nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        if ( nDx != 0) mrViewData.GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 ) mrViewData.GetView()->ScrollY( nDy, WhichV(eWhich) );
        bTimer = true;
    }

    // Switching when fixating (so Scrolling works)

    if ( eWhich == mrViewData.GetActivePart() )     //??
    {
        if ( mrViewData.GetHSplitMode() == SC_SPLIT_FIX )
            if ( nDx > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    mrViewData.GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    mrViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }

        if ( mrViewData.GetVSplitMode() == SC_SPLIT_FIX )
            if ( nDy > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    mrViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    mrViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }
    }

    // Move

    SCCOL  nPosX;
    SCROW  nPosY;
    mrViewData.GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );

    ScRange aOld = rData.aRef;
    ScRange aNew = aOld;
    if ( bRFSize )
    {
        switch (aRFSelectedCorned)
        {
            case LEFT_UP:
                aNew.aStart.SetCol(nPosX);
                aNew.aStart.SetRow(nPosY);
                break;
            case LEFT_DOWN:
                aNew.aStart.SetCol(nPosX);
                aNew.aEnd.SetRow(nPosY);
                break;
            case RIGHT_UP:
                aNew.aEnd.SetCol(nPosX);
                aNew.aStart.SetRow(nPosY);
                break;
            case RIGHT_DOWN:
                aNew.aEnd.SetCol(nPosX);
                aNew.aEnd.SetRow(nPosY);
                break;
            default:
                break;
        }
    }
    else
    {
        ScDocument& rDoc = mrViewData.GetDocument();
        tools::Long nStartX = nPosX - nRFAddX;
        if ( nStartX < 0 ) nStartX = 0;
        tools::Long nStartY = nPosY - nRFAddY;
        if ( nStartY < 0 ) nStartY = 0;
        tools::Long nEndX = nStartX + aOld.aEnd.Col() - aOld.aStart.Col();
        if ( nEndX > rDoc.MaxCol() )
        {
            nStartX -= ( nEndX - rDoc.MaxRow() );
            nEndX = rDoc.MaxCol();
        }
        tools::Long nEndY = nStartY + aOld.aEnd.Row() - aOld.aStart.Row();
        if ( nEndY > rDoc.MaxRow() )
        {
            nStartY -= ( nEndY - rDoc.MaxRow() );
            nEndY = rDoc.MaxRow();
        }

        aNew.aStart.SetCol(static_cast<SCCOL>(nStartX));
        aNew.aStart.SetRow(static_cast<SCROW>(nStartY));
        aNew.aEnd.SetCol(static_cast<SCCOL>(nEndX));
        aNew.aEnd.SetRow(static_cast<SCROW>(nEndY));
    }

    if ( bUp )
        aNew.PutInOrder();             // For ButtonUp again in the proper order

    if ( aNew != aOld )
    {
        pHdl->UpdateRange( nRFIndex, aNew );

        ScDocShell* pDocSh = mrViewData.GetDocShell();

        pHdl->UpdateLokReferenceMarks();

        // only redrawing what has been changed...
        lcl_PaintRefChanged( pDocSh, aOld, aNew );

        // only redraw new frame (synchronously)
        pDocSh->Broadcast( ScIndexHint( SfxHintId::ScShowRangeFinder, nRFIndex ) );

        PaintImmediately();   // what you move, will be seen immediately
    }

    //  Timer for Scrolling

    if (bTimer)
        mrViewData.GetView()->SetTimer( this, rMEvt );          // repeat event
    else
        mrViewData.GetView()->ResetTimer();
}

namespace {

SvxAdjust toSvxAdjust( const ScPatternAttr& rPat )
{
    SvxCellHorJustify eHorJust =
            rPat.GetItem(ATTR_HOR_JUSTIFY).GetValue();

    SvxAdjust eSvxAdjust = SvxAdjust::Left;
    switch (eHorJust)
    {
        case SvxCellHorJustify::Left:
        case SvxCellHorJustify::Repeat:            // not implemented
        case SvxCellHorJustify::Standard:          // always Text if an EditCell type
                eSvxAdjust = SvxAdjust::Left;
                break;
        case SvxCellHorJustify::Right:
                eSvxAdjust = SvxAdjust::Right;
                break;
        case SvxCellHorJustify::Center:
                eSvxAdjust = SvxAdjust::Center;
                break;
        case SvxCellHorJustify::Block:
                eSvxAdjust = SvxAdjust::Block;
                break;
    }

    return eSvxAdjust;
}

std::shared_ptr<ScFieldEditEngine> createEditEngine( ScDocShell* pDocSh, const ScPatternAttr& rPat )
{
    ScDocument& rDoc = pDocSh->GetDocument();

    auto pEngine = std::make_shared<ScFieldEditEngine>(&rDoc, rDoc.GetEditPool());
    ScSizeDeviceProvider aProv(pDocSh);
    pEngine->SetRefDevice(aProv.GetDevice());
    pEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
    SfxItemSet aDefault = pEngine->GetEmptyItemSet();
    rPat.FillEditItemSet(&aDefault);
    aDefault.Put( SvxAdjustItem(toSvxAdjust(rPat), EE_PARA_JUST) );
    pEngine->SetDefaults(aDefault);

    return pEngine;
}

bool extractURLInfo( const SvxFieldItem* pFieldItem, OUString* pName, OUString* pUrl, OUString* pTarget )
{
    if (!pFieldItem)
        return false;

    const SvxFieldData* pField = pFieldItem->GetField();
    if (pField->GetClassId() != text::textfield::Type::URL)
        return false;

    const SvxURLField* pURLField = static_cast<const SvxURLField*>(pField);

    if (pName)
        *pName = pURLField->GetRepresentation();
    if (pUrl)
        *pUrl = pURLField->GetURL();
    if (pTarget)
        *pTarget = pURLField->GetTargetFrame();

    return true;
}

}

bool ScGridWindow::GetEditUrl( const Point& rPos,
                               OUString* pName, OUString* pUrl, OUString* pTarget )
{
    ScTabViewShell* pViewSh = mrViewData.GetViewShell();
    ScInputHandler* pInputHdl = nullptr;
    if (pViewSh)
        pInputHdl = pViewSh->GetInputHandler();
    EditView* pView = (pInputHdl && pInputHdl->IsInputMode()) ? pInputHdl->GetTableView() : nullptr;
    if (pView)
        return extractURLInfo(pView->GetFieldUnderMousePointer(), pName, pUrl, pTarget);

    //! Pass on nPosX/Y?
    SCCOL nPosX;
    SCROW nPosY;
    mrViewData.GetPosFromPixel( rPos.X(), rPos.Y(), eWhich, nPosX, nPosY );

    SCTAB nTab = mrViewData.GetTabNo();
    ScDocShell* pDocSh = mrViewData.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    OUString sURL;
    ScRefCellValue aCell;
    bool bFound = lcl_GetHyperlinkCell(rDoc, nPosX, nPosY, nTab, aCell, sURL);
    if( !bFound )
        return false;

    const ScPatternAttr* pPattern = rDoc.GetPattern( nPosX, nPosY, nTab );
    // bForceToTop = sal_False, use the cell's real position
    tools::Rectangle aEditRect = mrViewData.GetEditArea( eWhich, nPosX, nPosY, this, pPattern, false );
    if (rPos.Y() < aEditRect.Top())
        return false;

        // vertical can not (yet) be clicked:

    if (pPattern->GetCellOrientation() != SvxCellOrientation::Standard)
        return false;

    bool bBreak = pPattern->GetItem(ATTR_LINEBREAK).GetValue() ||
                    (pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue() == SvxCellHorJustify::Block);
    SvxCellHorJustify eHorJust = pPattern->GetItem(ATTR_HOR_JUSTIFY).GetValue();

        //  EditEngine

    std::shared_ptr<ScFieldEditEngine> pEngine = createEditEngine(pDocSh, *pPattern);

    MapMode aEditMode = mrViewData.GetLogicMode(eWhich);            // without draw scaling
    tools::Rectangle aLogicEdit = PixelToLogic( aEditRect, aEditMode );
    tools::Long nThisColLogic = aLogicEdit.Right() - aLogicEdit.Left() + 1;
    Size aPaperSize( 1000000, 1000000 );
    if (aCell.meType == CELLTYPE_FORMULA)
    {
        tools::Long nSizeX  = 0;
        tools::Long nSizeY  = 0;
        mrViewData.GetMergeSizePixel( nPosX, nPosY, nSizeX, nSizeY );
        aPaperSize = Size(nSizeX, nSizeY );
        aPaperSize = PixelToLogic(aPaperSize);
    }

    if (bBreak)
        aPaperSize.setWidth( nThisColLogic );
    pEngine->SetPaperSize( aPaperSize );

    std::unique_ptr<EditTextObject> pTextObj;
    if (aCell.meType == CELLTYPE_EDIT)
    {
        if (aCell.mpEditText)
            pEngine->SetTextCurrentDefaults(*aCell.mpEditText);
    }
    else  // Not an Edit cell and is a formula cell with 'Hyperlink'
          // function if we have no URL, otherwise it could be a formula
          // cell ( or other type ? ) with a hyperlink associated with it.
    {
        if (sURL.isEmpty())
            pTextObj = aCell.mpFormula->CreateURLObject();
        else
        {
            OUString aRepres = sURL;

            // TODO: text content of formatted numbers can be different
            if (aCell.hasNumeric())
                aRepres = OUString::number(aCell.getValue());
            else if (aCell.meType == CELLTYPE_FORMULA)
                aRepres = aCell.mpFormula->GetString().getString();

            pTextObj = ScEditUtil::CreateURLObjectFromURL(rDoc, sURL, aRepres);
        }

        if (pTextObj)
            pEngine->SetTextCurrentDefaults(*pTextObj);
    }

    tools::Long nStartX = aLogicEdit.Left();

    tools::Long nTextWidth = pEngine->CalcTextWidth();
    tools::Long nTextHeight = pEngine->GetTextHeight();
    if ( nTextWidth < nThisColLogic )
    {
        if (eHorJust == SvxCellHorJustify::Right)
            nStartX += nThisColLogic - nTextWidth;
        else if (eHorJust == SvxCellHorJustify::Center)
            nStartX += (nThisColLogic - nTextWidth) / 2;
    }

    aLogicEdit.SetLeft( nStartX );
    if (!bBreak)
        aLogicEdit.SetRight( nStartX + nTextWidth );

    // There is one glitch when dealing with a hyperlink cell and
    // the cell content is NUMERIC. This defaults to right aligned and
    // we need to adjust accordingly.
    if (aCell.hasNumeric() && eHorJust == SvxCellHorJustify::Standard)
    {
        aLogicEdit.SetRight( aLogicEdit.Left() + nThisColLogic - 1 );
        aLogicEdit.SetLeft(  aLogicEdit.Right() - nTextWidth );
    }
    aLogicEdit.SetBottom( aLogicEdit.Top() + nTextHeight );

    Point aLogicClick = PixelToLogic(rPos,aEditMode);
    if ( aLogicEdit.Contains(aLogicClick) )
    {
        EditView aTempView(pEngine.get(), this);
        aTempView.SetOutputArea( aLogicEdit );

        bool bRet;
        if (comphelper::LibreOfficeKit::isActive())
        {
            bRet = extractURLInfo(aTempView.GetField(aLogicClick), pName, pUrl, pTarget);
        }
        else
        {
            MapMode aOld = GetMapMode();
            SetMapMode(aEditMode);                  // no return anymore
            bRet = extractURLInfo(aTempView.GetFieldUnderMousePointer(), pName, pUrl, pTarget);
            SetMapMode(aOld);
        }
        return bRet;
    }
    return false;
}

bool ScGridWindow::IsSpellErrorAtPos( const Point& rPos, SCCOL nCol1, SCROW nRow )
{
    if (!mpSpellCheckCxt)
        return false;

    SCTAB nTab = mrViewData.GetTabNo();
    ScDocShell* pDocSh = mrViewData.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();

    ScAddress aCellPos(nCol1, nRow, nTab);
    ScRefCellValue aCell(rDoc, aCellPos);
    if (aCell.meType != CELLTYPE_STRING && aCell.meType != CELLTYPE_EDIT)
        return false;

    const std::vector<editeng::MisspellRanges>* pRanges = mpSpellCheckCxt->getMisspellRanges(nCol1, nRow);
    if (!pRanges)
        return false;

    const ScPatternAttr* pPattern = rDoc.GetPattern(nCol1, nRow, nTab);

    tools::Rectangle aEditRect = mrViewData.GetEditArea(eWhich, nCol1, nRow, this, pPattern, false);
    if (rPos.Y() < aEditRect.Top())
        return false;

    std::shared_ptr<ScFieldEditEngine> pEngine = createEditEngine(pDocSh, *pPattern);

    Size aPaperSize(1000000, 1000000);
    pEngine->SetPaperSize(aPaperSize);

    if (aCell.meType == CELLTYPE_EDIT)
        pEngine->SetTextCurrentDefaults(*aCell.mpEditText);
    else
        pEngine->SetTextCurrentDefaults(aCell.mpString->getString());

    tools::Long nTextWidth = static_cast<tools::Long>(pEngine->CalcTextWidth());

    MapMode aEditMode = mrViewData.GetLogicMode(eWhich);
    tools::Rectangle aLogicEdit = PixelToLogic(aEditRect, aEditMode);
    Point aLogicClick = PixelToLogic(rPos, aEditMode);

    aLogicEdit.setWidth(nTextWidth + 1);

    if (!aLogicEdit.Contains(aLogicClick))
        return false;

    pEngine->SetControlWord(pEngine->GetControlWord() | EEControlBits::ONLINESPELLING);
    pEngine->SetAllMisspellRanges(*pRanges);

    EditView aTempView(pEngine.get(), this);
    aTempView.SetOutputArea(aLogicEdit);

    return aTempView.IsWrongSpelledWordAtPos(rPos);
}

bool ScGridWindow::HasScenarioButton( const Point& rPosPixel, ScRange& rScenRange )
{
    ScDocument& rDoc = mrViewData.GetDocument();
    SCTAB nTab = mrViewData.GetTabNo();
    SCTAB nTabCount = rDoc.GetTableCount();
    if ( nTab+1<nTabCount && rDoc.IsScenario(nTab+1) && !rDoc.IsScenario(nTab) )
    {
        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

        Size aButSize = mrViewData.GetScenButSize();
        tools::Long nBWidth  = aButSize.Width();
        if (!nBWidth)
            return false;                   // No Button drawn yet -> there is none
        tools::Long nBHeight = aButSize.Height();
        tools::Long nHSpace  = static_cast<tools::Long>( SC_SCENARIO_HSPACE * mrViewData.GetPPTX() );

        //! cache the Ranges in Table!!!!

        ScMarkData aMarks(rDoc.GetSheetLimits());
        for (SCTAB i=nTab+1; i<nTabCount && rDoc.IsScenario(i); i++)
            rDoc.MarkScenario( i, nTab, aMarks, false, ScScenarioFlags::ShowFrame );
        ScRangeList aRanges;
        aMarks.FillRangeListWithMarks( &aRanges, false );

        size_t nRangeCount = aRanges.size();
        for (size_t j=0;  j< nRangeCount; ++j)
        {
            ScRange aRange = aRanges[j];
            // Always extend scenario frame to merged cells where no new non-covered cells
            // are framed
            rDoc.ExtendTotalMerge( aRange );

            bool bTextBelow = ( aRange.aStart.Row() == 0 );

            Point aButtonPos;
            if ( bTextBelow )
            {
                aButtonPos = mrViewData.GetScrPos( aRange.aEnd.Col()+1, aRange.aEnd.Row()+1,
                                                    eWhich, true );
            }
            else
            {
                aButtonPos = mrViewData.GetScrPos( aRange.aEnd.Col()+1, aRange.aStart.Row(),
                                                    eWhich, true );
                aButtonPos.AdjustY( -nBHeight );
            }
            if ( bLayoutRTL )
                aButtonPos.AdjustX( -(nHSpace - 1) );
            else
                aButtonPos.AdjustX( -(nBWidth - nHSpace) );    // same for top or bottom

            tools::Rectangle aButRect( aButtonPos, Size(nBWidth,nBHeight) );
            if ( aButRect.Contains( rPosPixel ) )
            {
                rScenRange = aRange;
                return true;
            }
        }
    }

    return false;
}

void ScGridWindow::DrawLayerCreated()
{
    SetMapMode( GetDrawMapMode() );

    // initially create overlay objects
    ImpCreateOverlayObjects();
}

void ScGridWindow::SetAutoSpellContext( const std::shared_ptr<sc::SpellCheckContext> &ctx )
{
    mpSpellCheckCxt = ctx;
}

void ScGridWindow::ResetAutoSpell()
{
    if (mpSpellCheckCxt)
        mpSpellCheckCxt->reset();
}

void ScGridWindow::ResetAutoSpellForContentChange()
{
    if (mpSpellCheckCxt)
        mpSpellCheckCxt->resetForContentChange();
}

void ScGridWindow::SetAutoSpellData( SCCOL nPosX, SCROW nPosY, const std::vector<editeng::MisspellRanges>* pRanges )
{
    if (!mpSpellCheckCxt)
        return;

    mpSpellCheckCxt->setMisspellRanges(nPosX, nPosY, pRanges);
}

const std::vector<editeng::MisspellRanges>* ScGridWindow::GetAutoSpellData( SCCOL nPosX, SCROW nPosY )
{
    if (!mpSpellCheckCxt)
        return nullptr;

    if (!maVisibleRange.isInside(nPosX, nPosY))
        return nullptr;

    return mpSpellCheckCxt->getMisspellRanges(nPosX, nPosY);
}

bool ScGridWindow::InsideVisibleRange( SCCOL nPosX, SCROW nPosY )
{
    return maVisibleRange.isInside(nPosX, nPosY);
}

OString ScGridWindow::getCellCursor() const
{
    // GridWindow stores a shown cell cursor in mpOOCursors, hence
    // we can use that to determine whether we would want to be showing
    // one (client-side) for tiled rendering too.
    if (!mpOOCursors)
        return "EMPTY";

    if (comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
        return mrViewData.describeCellCursorInPrintTwips();

    return mrViewData.describeCellCursor();
}

void ScGridWindow::notifyKitCellCursor() const
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();

    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_CURSOR, getCellCursor().getStr());
    if (bListValButton && aListValPos == mrViewData.GetCurPos())
        updateLOKValListButton(true, aListValPos);
}

void ScGridWindow::notifyKitCellViewCursor(const SfxViewShell* pForShell) const
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();

    if (pViewShell->GetDocId() != pForShell->GetDocId())
        return;

    OString aCursor("EMPTY");
    if (mpOOCursors) // cf. getCellCursor above
    {
        auto pForTabView = dynamic_cast<const ScTabViewShell *>(pForShell);
        if (!pForTabView)
            return;

        if (comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
            aCursor = mrViewData.describeCellCursorInPrintTwips();
        else
            aCursor = pForTabView->GetViewData().describeCellCursorAt(
                mrViewData.GetCurX(), mrViewData.GetCurY()); // our position.
    }
    SfxLokHelper::notifyOtherView(pViewShell, pForShell, LOK_CALLBACK_CELL_VIEW_CURSOR, "rectangle", aCursor);
}

// Send our cursor details to a view described by @pForShell, or all views
// if @pForShell is null. In each case send the current view a cell-cursor
// event, and others a cell_view_cursor event.
//
// NB. we need to re-construct the cursor details for each other view in their
// own zoomed co-ordinate system (but not in scPrintTwipsMsgs mode).
void ScGridWindow::updateKitCellCursor(const SfxViewShell* pForShell) const
{
    if (comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
    {
        ScTabViewShell* pViewShell = mrViewData.GetViewShell();
        // Generate the cursor info string just once and directly send to all.
        // Calling notifyKitCellViewCursor() would regenerate the
        // cursor-string unnecessarily.
        OString aCursor = getCellCursor();

        if (pForShell)
        {
            SfxLokHelper::notifyOtherView(pViewShell, pForShell,
                    LOK_CALLBACK_CELL_VIEW_CURSOR, "rectangle", aCursor);
        }
        else
        {
            notifyKitCellCursor();
            SfxLokHelper::notifyOtherViews(pViewShell,
                    LOK_CALLBACK_CELL_VIEW_CURSOR, "rectangle", aCursor);
        }

        return;
    }

    if (!pForShell)
    {
        for (SfxViewShell* it = SfxViewShell::GetFirst(); it;
             it = SfxViewShell::GetNext(*it))
            updateKitCellCursor(it);
        return;
    }

    if (pForShell == mrViewData.GetViewShell())
        notifyKitCellCursor();
    else
        notifyKitCellViewCursor(pForShell);
}

void ScGridWindow::updateKitOtherCursors() const
{
    for (SfxViewShell* it = SfxViewShell::GetFirst(); it;
         it = SfxViewShell::GetNext(*it))
    {
        auto pOther = dynamic_cast<const ScTabViewShell *>(it);
        if (!pOther)
            continue;
        const ScGridWindow *pGrid = pOther->GetViewData().GetActiveWin();
        assert(pGrid);
        if (pGrid == this)
            notifyKitCellCursor();
        else
            pGrid->notifyKitCellViewCursor(mrViewData.GetViewShell());
    }
}

void ScGridWindow::CursorChanged()
{
    // here the created OverlayObjects may be transformed in later versions. For
    // now, just re-create them

    UpdateCursorOverlay();
}

void ScGridWindow::ImpCreateOverlayObjects()
{
    UpdateCursorOverlay();
    UpdateCopySourceOverlay();
    UpdateSelectionOverlay();
    UpdateAutoFillOverlay();
    UpdateDragRectOverlay();
    UpdateHeaderOverlay();
    UpdateShrinkOverlay();
}

void ScGridWindow::ImpDestroyOverlayObjects()
{
    DeleteCursorOverlay();
    DeleteCopySourceOverlay();
    DeleteSelectionOverlay();
    DeleteAutoFillOverlay();
    DeleteDragRectOverlay();
    DeleteHeaderOverlay();
    DeleteShrinkOverlay();
}

void ScGridWindow::UpdateAllOverlays()
{
    // delete and re-allocate all overlay objects

    ImpDestroyOverlayObjects();
    ImpCreateOverlayObjects();
}

void ScGridWindow::DeleteCursorOverlay()
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_CURSOR, "EMPTY");
    SfxLokHelper::notifyOtherViews(pViewShell, LOK_CALLBACK_CELL_VIEW_CURSOR, "rectangle", "EMPTY");
    mpOOCursors.reset();
}

void ScGridWindow::DeleteCopySourceOverlay()
{
    mpOOSelectionBorder.reset();
}

void ScGridWindow::UpdateCopySourceOverlay()
{
    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    DeleteCopySourceOverlay();

    if (comphelper::LibreOfficeKit::isActive())
        return;
    if (!mrViewData.ShowPasteSource())
        return;
    if (!SC_MOD()->GetInputOptions().GetEnterPasteMode())
        return;
    rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();
    if (!xOverlayManager.is())
        return;
    const ScTransferObj* pTransObj = ScTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(mrViewData.GetActiveWin()));
    if (!pTransObj)
        return;
    ScDocument* pClipDoc = pTransObj->GetDocument();
    if (!pClipDoc)
        return;

    SCTAB nCurTab = mrViewData.GetCurPos().Tab();

    ScClipParam& rClipParam = pClipDoc->GetClipParam();
    mpOOSelectionBorder.reset(new sdr::overlay::OverlayObjectList);
    for ( size_t i = 0; i < rClipParam.maRanges.size(); ++i )
    {
        ScRange const & r = rClipParam.maRanges[i];
        if (r.aStart.Tab() != nCurTab)
            continue;

        SCCOL nClipStartX = r.aStart.Col();
        SCROW nClipStartY = r.aStart.Row();
        SCCOL nClipEndX   = r.aEnd.Col();
        SCROW nClipEndY   = r.aEnd.Row();

        Point aClipStartScrPos = mrViewData.GetScrPos( nClipStartX, nClipStartY, eWhich );
        Point aClipEndScrPos   = mrViewData.GetScrPos( nClipEndX + 1, nClipEndY + 1, eWhich );
        aClipStartScrPos -= Point(1, 1);
        tools::Long nSizeXPix = aClipEndScrPos.X() - aClipStartScrPos.X();
        tools::Long nSizeYPix = aClipEndScrPos.Y() - aClipStartScrPos.Y();

        tools::Rectangle aRect( aClipStartScrPos, Size(nSizeXPix, nSizeYPix) );

        Color aHighlight = GetSettings().GetStyleSettings().GetHighlightColor();

        tools::Rectangle aLogic = PixelToLogic(aRect, aDrawMode);
        ::basegfx::B2DRange aRange = vcl::unotools::b2DRectangleFromRectangle(aLogic);
        std::unique_ptr<ScOverlayDashedBorder> pDashedBorder(new ScOverlayDashedBorder(aRange, aHighlight));
        xOverlayManager->add(*pDashedBorder);
        mpOOSelectionBorder->append(std::move(pDashedBorder));
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

static std::vector<tools::Rectangle> convertPixelToLogical(
    const ScViewData& rViewData,
    const std::vector<tools::Rectangle>& rRectangles,
    tools::Rectangle &rBoundingBox)
{
    std::vector<tools::Rectangle> aLogicRects;

    double nPPTX = rViewData.GetPPTX();
    double nPPTY = rViewData.GetPPTY();

    for (const auto& rRectangle : rRectangles)
    {
        // We explicitly create a copy, since we need to expand
        // the rectangle before coordinate conversion
        tools::Rectangle aRectangle(rRectangle);
        aRectangle.AdjustRight(1 );
        aRectangle.AdjustBottom(1 );

        tools::Rectangle aRect(aRectangle.Left() / nPPTX, aRectangle.Top() / nPPTY,
                aRectangle.Right() / nPPTX, aRectangle.Bottom() / nPPTY);

        rBoundingBox.Union(aRect);
        aLogicRects.push_back(aRect);
    }
    return aLogicRects;
}

static OString rectanglesToString(const std::vector<tools::Rectangle> &rLogicRects)
{
    bool bFirst = true;
    OStringBuffer aRects;
    for (const auto &rRect : rLogicRects)
    {
        if (!bFirst)
            aRects.append("; ");
        bFirst = false;
        aRects.append(rRect.toString());
    }
    return aRects.makeStringAndClear();
}

/**
 * Turn the selection ranges rRectangles into the LibreOfficeKit selection, and send to other views.
 *
 * @param pLogicRects - if set then don't invoke the callback, just collect the rectangles in the pointed vector.
 */
void ScGridWindow::UpdateKitSelection(const std::vector<tools::Rectangle>& rRectangles, std::vector<tools::Rectangle>* pLogicRects)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    // If this is true, rRectangles should already in print twips.
    // If false, rRectangles are in pixels.
    bool bInPrintTwips = comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    tools::Rectangle aBoundingBox;
    std::vector<tools::Rectangle> aConvertedRects;

    if (bInPrintTwips)
        std::for_each(rRectangles.begin(), rRectangles.end(),
                      [&aBoundingBox](const tools::Rectangle& rRect) { aBoundingBox.Union(rRect); });
    else
        aConvertedRects = convertPixelToLogical(mrViewData, rRectangles, aBoundingBox);

    const std::vector<tools::Rectangle>& rLogicRects = bInPrintTwips ? rRectangles : aConvertedRects;
    if (pLogicRects)
    {
        *pLogicRects = rLogicRects;
        return;
    }

    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    pViewShell->UpdateInputHandler();
    OString sBoundingBoxString = "EMPTY";
    if (!aBoundingBox.IsEmpty())
        sBoundingBoxString = aBoundingBox.toString();
    OString aRectListString = rectanglesToString(rLogicRects);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_SELECTION_AREA, sBoundingBoxString.getStr());
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, aRectListString.getStr());

    if (bInPrintTwips)
    {
        SfxLokHelper::notifyOtherViews(pViewShell, LOK_CALLBACK_TEXT_VIEW_SELECTION,
                                       "selection", aRectListString);
        return;
    }

    for (SfxViewShell* it = SfxViewShell::GetFirst(); it;
         it = SfxViewShell::GetNext(*it))
    {
        if (it == pViewShell)
            continue;
        auto pOther = dynamic_cast<const ScTabViewShell *>(it);
        if (!pOther)
            return;

        const ScGridWindow *pGrid = pOther->GetViewData().GetActiveWin();
        assert(pGrid);

        // Fetch pixels & convert for each view separately.
        tools::Rectangle aDummyBBox;
        std::vector<tools::Rectangle> aPixelRects;
        pGrid->GetPixelRectsFor(mrViewData.GetMarkData() /* ours */, aPixelRects);
        auto aOtherLogicRects = convertPixelToLogical(pOther->GetViewData(), aPixelRects, aDummyBBox);
        SfxLokHelper::notifyOtherView(pViewShell, pOther, LOK_CALLBACK_TEXT_VIEW_SELECTION,
                                      "selection", rectanglesToString(aOtherLogicRects).getStr());
    }
}

/**
 * Fetch the selection ranges for other views into the LibreOfficeKit selection,
 * map them into our view co-ordinates and send to our view.
 */
void ScGridWindow::updateOtherKitSelections() const
{
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    bool bInPrintTwips = comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    for (SfxViewShell* it = SfxViewShell::GetFirst(); it;
         it = SfxViewShell::GetNext(*it))
    {
        auto pOther = dynamic_cast<const ScTabViewShell *>(it);
        if (!pOther)
            return;

        // Fetch pixels & convert for each view separately.
        tools::Rectangle aBoundingBox;
        std::vector<tools::Rectangle> aRects;
        OString aRectsString;
        GetRectsAnyFor(pOther->GetViewData().GetMarkData() /* theirs */, aRects, bInPrintTwips);
        if (bInPrintTwips)
        {
            std::for_each(aRects.begin(), aRects.end(),
                          [&aBoundingBox](const tools::Rectangle& rRect) { aBoundingBox.Union(rRect); });
            aRectsString = rectanglesToString(aRects);
        }
        else
            aRectsString = rectanglesToString(
                    convertPixelToLogical(pViewShell->GetViewData(), aRects, aBoundingBox));

        if (it == pViewShell)
        {
            OString sBoundingBoxString = "EMPTY";
            if (!aBoundingBox.IsEmpty())
                sBoundingBoxString = aBoundingBox.toString();

            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_SELECTION_AREA, sBoundingBoxString.getStr());
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, aRectsString.getStr());
        }
        else
            SfxLokHelper::notifyOtherView(it, pViewShell, LOK_CALLBACK_TEXT_VIEW_SELECTION,
                                          "selection", aRectsString);
    }
}

namespace
{

void updateLibreOfficeKitAutoFill(const ScViewData& rViewData, tools::Rectangle const & rRectangle)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    double nPPTX = rViewData.GetPPTX();
    double nPPTY = rViewData.GetPPTY();

    OString sRectangleString = "EMPTY";
    if (!rRectangle.IsEmpty())
    {
        // selection start handle
        tools::Rectangle aLogicRectangle(
                rRectangle.Left()  / nPPTX, rRectangle.Top() / nPPTY,
                rRectangle.Right() / nPPTX, rRectangle.Bottom() / nPPTY);
        sRectangleString = aLogicRectangle.toString();
    }

    ScTabViewShell* pViewShell = rViewData.GetViewShell();
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_AUTO_FILL_AREA, sRectangleString.getStr());
}

} //end anonymous namespace

void ScGridWindow::UpdateCursorOverlay()
{
    ScDocument& rDoc = mrViewData.GetDocument();

    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    // Existing OverlayObjects may be transformed in later versions.
    // For now, just re-create them.

    DeleteCursorOverlay();

    std::vector<tools::Rectangle> aPixelRects;

    //  determine the cursor rectangles in pixels (moved from ScGridWindow::DrawCursor)

    SCTAB nTab = mrViewData.GetTabNo();
    SCCOL nX = mrViewData.GetCurX();
    SCROW nY = mrViewData.GetCurY();

    const ScPatternAttr* pPattern = rDoc.GetPattern(nX,nY,nTab);

    if (!comphelper::LibreOfficeKit::isActive() && !maVisibleRange.isInside(nX, nY))
    {
        if (maVisibleRange.mnCol2 < nX || maVisibleRange.mnRow2 < nY)
            return;     // no further check needed, nothing visible

        // fdo#87382 Also display the cell cursor for the visible part of
        // merged cells if the view position is part of merged cells.
        const ScMergeAttr& rMerge = pPattern->GetItem(ATTR_MERGE);
        if (rMerge.GetColMerge() <= 1 && rMerge.GetRowMerge() <= 1)
            return;     // not merged and invisible

        SCCOL nX2 = nX + rMerge.GetColMerge() - 1;
        SCROW nY2 = nY + rMerge.GetRowMerge() - 1;
        // Check if the middle or tail of the merged range is visible.
        if (maVisibleRange.mnCol1 > nX2 || maVisibleRange.mnRow1 > nY2)
            return;     // no visible part
    }

    //  don't show the cursor in overlapped cells
    const ScMergeFlagAttr& rMergeFlag = pPattern->GetItem(ATTR_MERGE_FLAG);
    bool bOverlapped = rMergeFlag.IsOverlapped();

    //  left or above of the screen?
    bool bVis = comphelper::LibreOfficeKit::isActive() || ( nX>=mrViewData.GetPosX(eHWhich) && nY>=mrViewData.GetPosY(eVWhich) );
    if (!bVis)
    {
        SCCOL nEndX = nX;
        SCROW nEndY = nY;
        const ScMergeAttr& rMerge = pPattern->GetItem(ATTR_MERGE);
        if (rMerge.GetColMerge() > 1)
            nEndX += rMerge.GetColMerge()-1;
        if (rMerge.GetRowMerge() > 1)
            nEndY += rMerge.GetRowMerge()-1;
        bVis = ( nEndX>=mrViewData.GetPosX(eHWhich) && nEndY>=mrViewData.GetPosY(eVWhich) );
    }

    if ( bVis && !bOverlapped && !mrViewData.HasEditView(eWhich) && mrViewData.IsActive() )
    {
        Point aScrPos = mrViewData.GetScrPos( nX, nY, eWhich, true );
        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

        //  completely right of/below the screen?
        //  (test with logical start position in aScrPos)
        bool bMaybeVisible;
        if ( bLayoutRTL )
            bMaybeVisible = ( aScrPos.X() >= -2 && aScrPos.Y() >= -2 );
        else
        {
            Size aOutSize = GetOutputSizePixel();
            bMaybeVisible = ( aScrPos.X() <= aOutSize.Width() + 2 && aScrPos.Y() <= aOutSize.Height() + 2 );
        }

        // in the tiled rendering case, don't limit to the screen size
        if (bMaybeVisible || comphelper::LibreOfficeKit::isActive())
        {
            tools::Long nSizeXPix;
            tools::Long nSizeYPix;
            mrViewData.GetMergeSizePixel( nX, nY, nSizeXPix, nSizeYPix );

            if (bLayoutRTL)
                aScrPos.AdjustX( -(nSizeXPix - 2) );       // move instead of mirroring

            // show the cursor as 4 (thin) rectangles
            tools::Rectangle aRect(aScrPos, Size(nSizeXPix - 1, nSizeYPix - 1));

            float fScaleFactor = GetDPIScaleFactor();

            tools::Long aCursorWidth = 1 * fScaleFactor;

            tools::Rectangle aLeft = aRect;
            aLeft.AdjustTop( -aCursorWidth );
            aLeft.AdjustBottom(aCursorWidth );
            aLeft.SetRight( aLeft.Left() );
            aLeft.AdjustLeft( -aCursorWidth );

            tools::Rectangle aRight = aRect;
            aRight.AdjustTop( -aCursorWidth );
            aRight.AdjustBottom(aCursorWidth );
            aRight.SetLeft( aRight.Right() );
            aRight.AdjustRight(aCursorWidth );

            tools::Rectangle aTop = aRect;
            aTop.SetBottom( aTop.Top() );
            aTop.AdjustTop( -aCursorWidth );

            tools::Rectangle aBottom = aRect;
            aBottom.SetTop( aBottom.Bottom() );
            aBottom.AdjustBottom(aCursorWidth );

            aPixelRects.push_back(aLeft);
            aPixelRects.push_back(aRight);
            aPixelRects.push_back(aTop);
            aPixelRects.push_back(aBottom);
        }
    }

    if ( !aPixelRects.empty() )
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            mpOOCursors.reset(new sdr::overlay::OverlayObjectList);
            updateKitCellCursor(nullptr);
        }
        else
        {
            // #i70788# get the OverlayManager safely
            rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();

            if (xOverlayManager.is())
            {
                Color aCursorColor = GetSettings().GetStyleSettings().GetHighlightColor();
                if (mrViewData.GetActivePart() != eWhich)
                    // non-active pane uses a different color.
                    aCursorColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::CALCPAGEBREAKAUTOMATIC).nColor;
                std::vector< basegfx::B2DRange > aRanges;
                const basegfx::B2DHomMatrix aTransform(GetOutDev()->GetInverseViewTransformation());

                for(const tools::Rectangle & rRA : aPixelRects)
                {
                    basegfx::B2DRange aRB(rRA.Left(), rRA.Top(), rRA.Right() + 1, rRA.Bottom() + 1);
                    aRB.transform(aTransform);
                    aRanges.push_back(aRB);
                }

                std::unique_ptr<sdr::overlay::OverlayObject> pOverlay(new sdr::overlay::OverlaySelection(
                    sdr::overlay::OverlayType::Solid,
                    aCursorColor,
                    std::move(aRanges),
                    false));

                xOverlayManager->add(*pOverlay);
                mpOOCursors.reset(new sdr::overlay::OverlayObjectList);
                mpOOCursors->append(std::move(pOverlay));
            }
        }
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

void ScGridWindow::GetCellSelection(std::vector<tools::Rectangle>& rLogicRects)
{
    std::vector<tools::Rectangle> aRects;
    if (comphelper::LibreOfficeKit::isActive() &&
            comphelper::LibreOfficeKit::isCompatFlagSet(
                comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
        GetSelectionRectsPrintTwips(aRects);
    else
        GetSelectionRects(aRects);
    UpdateKitSelection(aRects, &rLogicRects);
}

void ScGridWindow::DeleteSelectionOverlay()
{
    mpOOSelection.reset();
}

void ScGridWindow::UpdateSelectionOverlay()
{
    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    DeleteSelectionOverlay();
    std::vector<tools::Rectangle> aRects;
    if (comphelper::LibreOfficeKit::isActive() &&
            comphelper::LibreOfficeKit::isCompatFlagSet(
                comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
        GetSelectionRectsPrintTwips(aRects);
    else
        GetSelectionRects(aRects);

    if (!aRects.empty() && mrViewData.IsActive())
    {
        // #i70788# get the OverlayManager safely
        rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();
        if (comphelper::LibreOfficeKit::isActive())
        {
            // notify the LibreOfficeKit too
            UpdateKitSelection(aRects);
        }
        else if (xOverlayManager.is())
        {
            std::vector< basegfx::B2DRange > aRanges;
            const basegfx::B2DHomMatrix aTransform(GetOutDev()->GetInverseViewTransformation());
            ScDocument& rDoc = mrViewData.GetDocument();
            SCTAB nTab = mrViewData.GetTabNo();
            bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

            for(const tools::Rectangle & rRA : aRects)
            {
                if (bLayoutRTL)
                {
                    basegfx::B2DRange aRB(rRA.Left(), rRA.Top() - 1, rRA.Right() + 1, rRA.Bottom());
                    aRB.transform(aTransform);
                    aRanges.push_back(aRB);
                }
                else
                {
                    basegfx::B2DRange aRB(rRA.Left() - 1, rRA.Top() - 1, rRA.Right(), rRA.Bottom());
                    aRB.transform(aTransform);
                    aRanges.push_back(aRB);
                }
            }

            // get the system's highlight color
            const Color aHighlight(SvtOptionsDrawinglayer::getHilightColor());

            std::unique_ptr<sdr::overlay::OverlayObject> pOverlay(new sdr::overlay::OverlaySelection(
                sdr::overlay::OverlayType::Transparent,
                aHighlight,
                std::move(aRanges),
                true));

            xOverlayManager->add(*pOverlay);
            mpOOSelection.reset(new sdr::overlay::OverlayObjectList);
            mpOOSelection->append(std::move(pOverlay));
        }
    }
    else
    {
        ScTabViewShell* pViewShell = mrViewData.GetViewShell();
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, "EMPTY");
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_SELECTION_AREA, "EMPTY");
        SfxLokHelper::notifyOtherViews(pViewShell, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", "EMPTY");

        ScInputHandler* pViewHdl = SC_MOD()->GetInputHdl(pViewShell);
        if (!pViewHdl || !pViewHdl->IsEditMode())
        {
            std::vector<ReferenceMark> aReferenceMarks;
            ScInputHandler::SendReferenceMarks(pViewShell, aReferenceMarks);
        }
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

void ScGridWindow::DeleteAutoFillOverlay()
{
    mpOOAutoFill.reset();
    mpAutoFillRect.reset();
}

void ScGridWindow::UpdateAutoFillOverlay()
{
    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    DeleteAutoFillOverlay();

    //  get the AutoFill handle rectangle in pixels

    if ( !(bAutoMarkVisible && aAutoMarkPos.Tab() == mrViewData.GetTabNo() &&
         !mrViewData.HasEditView(eWhich) && mrViewData.IsActive()) )
        return;

    SCCOL nX = aAutoMarkPos.Col();
    SCROW nY = aAutoMarkPos.Row();

    if (!maVisibleRange.isInside(nX, nY) && !comphelper::LibreOfficeKit::isActive())
    {
        // Autofill mark is not visible.  Bail out.
        return;
    }

    SCTAB nTab = mrViewData.GetTabNo();
    ScDocument& rDoc = mrViewData.GetDocument();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

    float fScaleFactor = GetDPIScaleFactor();
    // Size should be even
    Size aFillHandleSize(6 * fScaleFactor, 6 * fScaleFactor);

    Point aFillPos = mrViewData.GetScrPos( nX, nY, eWhich, true );
    tools::Long nSizeXPix;
    tools::Long nSizeYPix;
    mrViewData.GetMergeSizePixel( nX, nY, nSizeXPix, nSizeYPix );

    if (bLayoutRTL)
        aFillPos.AdjustX( -(nSizeXPix - 2 + (aFillHandleSize.Width() / 2)) );
    else
        aFillPos.AdjustX(nSizeXPix - (aFillHandleSize.Width() / 2) );

    aFillPos.AdjustY(nSizeYPix );
    aFillPos.AdjustY( -(aFillHandleSize.Height() / 2) );

    tools::Rectangle aFillRect(aFillPos, aFillHandleSize);

    // expand rect to increase hit area
    mpAutoFillRect = aFillRect;
    mpAutoFillRect->expand(fScaleFactor);

    // #i70788# get the OverlayManager safely
    rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();
    if (comphelper::LibreOfficeKit::isActive()) // notify the LibreOfficeKit
    {
        updateLibreOfficeKitAutoFill(mrViewData, aFillRect);
    }
    else if (xOverlayManager.is())
    {
        Color aHandleColor = GetSettings().GetStyleSettings().GetHighlightColor();
        if (mrViewData.GetActivePart() != eWhich)
            // non-active pane uses a different color.
            aHandleColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::CALCPAGEBREAKAUTOMATIC).nColor;
        std::vector< basegfx::B2DRange > aRanges;
        const basegfx::B2DHomMatrix aTransform(GetOutDev()->GetInverseViewTransformation());
        basegfx::B2DRange aRB = vcl::unotools::b2DRectangleFromRectangle(aFillRect);

        aRB.transform(aTransform);
        aRanges.push_back(aRB);

        std::unique_ptr<sdr::overlay::OverlayObject> pOverlay(new sdr::overlay::OverlaySelection(
            sdr::overlay::OverlayType::Solid,
            aHandleColor,
            std::move(aRanges),
            false));

        xOverlayManager->add(*pOverlay);
        mpOOAutoFill.reset(new sdr::overlay::OverlayObjectList);
        mpOOAutoFill->append(std::move(pOverlay));
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

void ScGridWindow::DeleteDragRectOverlay()
{
    mpOODragRect.reset();
}

void ScGridWindow::UpdateDragRectOverlay()
{
    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    DeleteDragRectOverlay();

    //  get the rectangles in pixels (moved from DrawDragRect)

    if ( bDragRect || bPagebreakDrawn )
    {
        std::vector<tools::Rectangle> aPixelRects;

        SCCOL nX1 = bDragRect ? nDragStartX : aPagebreakDrag.aStart.Col();
        SCROW nY1 = bDragRect ? nDragStartY : aPagebreakDrag.aStart.Row();
        SCCOL nX2 = bDragRect ? nDragEndX : aPagebreakDrag.aEnd.Col();
        SCROW nY2 = bDragRect ? nDragEndY : aPagebreakDrag.aEnd.Row();

        SCTAB nTab = mrViewData.GetTabNo();

        SCCOL nPosX = mrViewData.GetPosX(WhichH(eWhich));
        SCROW nPosY = mrViewData.GetPosY(WhichV(eWhich));
        if (nX1 < nPosX) nX1 = nPosX;
        if (nX2 < nPosX) nX2 = nPosX;
        if (nY1 < nPosY) nY1 = nPosY;
        if (nY2 < nPosY) nY2 = nPosY;

        Point aScrPos( mrViewData.GetScrPos( nX1, nY1, eWhich ) );

        tools::Long nSizeXPix=0;
        tools::Long nSizeYPix=0;
        ScDocument& rDoc = mrViewData.GetDocument();
        double nPPTX = mrViewData.GetPPTX();
        double nPPTY = mrViewData.GetPPTY();
        SCCOLROW i;

        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
        tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

        if (rDoc.ValidCol(nX2) && nX2>=nX1)
            for (i=nX1; i<=nX2; i++)
                nSizeXPix += ScViewData::ToPixel( rDoc.GetColWidth( static_cast<SCCOL>(i), nTab ), nPPTX );
        else
        {
            aScrPos.AdjustX( -nLayoutSign );
            nSizeXPix   += 2;
        }

        if (rDoc.ValidRow(nY2) && nY2>=nY1)
            for (i=nY1; i<=nY2; i++)
                nSizeYPix += ScViewData::ToPixel( rDoc.GetRowHeight( i, nTab ), nPPTY );
        else
        {
            aScrPos.AdjustY( -1 );
            nSizeYPix   += 2;
        }

        aScrPos.AdjustX( -(2 * nLayoutSign) );
        aScrPos.AdjustY( -2 );
        tools::Rectangle aRect( aScrPos.X(), aScrPos.Y(),
                         aScrPos.X() + ( nSizeXPix + 2 ) * nLayoutSign, aScrPos.Y() + nSizeYPix + 2 );
        if ( bLayoutRTL )
        {
            aRect.SetLeft( aRect.Right() );   // end position is left
            aRect.SetRight( aScrPos.X() );
        }

        if ( meDragInsertMode == INS_CELLSDOWN )
        {
            aPixelRects.emplace_back( aRect.Left()+1, aRect.Top()+3, aRect.Left()+1, aRect.Bottom()-2 );
            aPixelRects.emplace_back( aRect.Right()-1, aRect.Top()+3, aRect.Right()-1, aRect.Bottom()-2 );
            aPixelRects.emplace_back( aRect.Left()+1, aRect.Top(), aRect.Right()-1, aRect.Top()+2 );
            aPixelRects.emplace_back( aRect.Left()+1, aRect.Bottom()-1, aRect.Right()-1, aRect.Bottom()-1 );
        }
        else if ( meDragInsertMode == INS_CELLSRIGHT )
        {
            aPixelRects.emplace_back( aRect.Left(), aRect.Top()+1, aRect.Left()+2, aRect.Bottom()-1 );
            aPixelRects.emplace_back( aRect.Right()-1, aRect.Top()+1, aRect.Right()-1, aRect.Bottom()-1 );
            aPixelRects.emplace_back( aRect.Left()+3, aRect.Top()+1, aRect.Right()-2, aRect.Top()+1 );
            aPixelRects.emplace_back( aRect.Left()+3, aRect.Bottom()-1, aRect.Right()-2, aRect.Bottom()-1 );
        }
        else
        {
            aPixelRects.emplace_back( aRect.Left(), aRect.Top(), aRect.Left()+2, aRect.Bottom() );
            aPixelRects.emplace_back( aRect.Right()-2, aRect.Top(), aRect.Right(), aRect.Bottom() );
            aPixelRects.emplace_back( aRect.Left()+3, aRect.Top(), aRect.Right()-3, aRect.Top()+2 );
            aPixelRects.emplace_back( aRect.Left()+3, aRect.Bottom()-2, aRect.Right()-3, aRect.Bottom() );
        }

        // #i70788# get the OverlayManager safely
        rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();

        if (xOverlayManager.is() && !comphelper::LibreOfficeKit::isActive())
        {
            std::vector< basegfx::B2DRange > aRanges;
            const basegfx::B2DHomMatrix aTransform(GetOutDev()->GetInverseViewTransformation());

            for(const tools::Rectangle & rRA : aPixelRects)
            {
                basegfx::B2DRange aRB(rRA.Left(), rRA.Top(), rRA.Right() + 1, rRA.Bottom() + 1);
                aRB.transform(aTransform);
                aRanges.push_back(aRB);
            }

            std::unique_ptr<sdr::overlay::OverlayObject> pOverlay(new sdr::overlay::OverlaySelection(
                sdr::overlay::OverlayType::Invert,
                COL_BLACK,
                std::move(aRanges),
                false));

            xOverlayManager->add(*pOverlay);
            mpOODragRect.reset(new sdr::overlay::OverlayObjectList);
            mpOODragRect->append(std::move(pOverlay));
        }
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

void ScGridWindow::DeleteHeaderOverlay()
{
    mpOOHeader.reset();
}

void ScGridWindow::UpdateHeaderOverlay()
{
    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    DeleteHeaderOverlay();

    //  Pixel rectangle is in aInvertRect
    if ( !aInvertRect.IsEmpty() )
    {
        // #i70788# get the OverlayManager safely
        rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();

        if (xOverlayManager.is() && !comphelper::LibreOfficeKit::isActive())
        {
            // Color aHighlight = GetSettings().GetStyleSettings().GetHighlightColor();
            std::vector< basegfx::B2DRange > aRanges;
            const basegfx::B2DHomMatrix aTransform(GetOutDev()->GetInverseViewTransformation());
            basegfx::B2DRange aRB(aInvertRect.Left(), aInvertRect.Top(), aInvertRect.Right() + 1, aInvertRect.Bottom() + 1);

            aRB.transform(aTransform);
            aRanges.push_back(aRB);

            std::unique_ptr<sdr::overlay::OverlayObject> pOverlay(new sdr::overlay::OverlaySelection(
                sdr::overlay::OverlayType::Invert,
                COL_BLACK,
                std::move(aRanges),
                false));

            xOverlayManager->add(*pOverlay);
            mpOOHeader.reset(new sdr::overlay::OverlayObjectList);
            mpOOHeader->append(std::move(pOverlay));
        }
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

void ScGridWindow::DeleteShrinkOverlay()
{
    mpOOShrink.reset();
}

void ScGridWindow::UpdateShrinkOverlay()
{
    MapMode aDrawMode = GetDrawMapMode();
    MapMode aOldMode = GetMapMode();
    if ( aOldMode != aDrawMode )
        SetMapMode( aDrawMode );

    DeleteShrinkOverlay();

    //  get the rectangle in pixels

    tools::Rectangle aPixRect;
    ScRange aRange;
    SCTAB nTab = mrViewData.GetTabNo();
    if ( mrViewData.IsRefMode() && nTab >= mrViewData.GetRefStartZ() && nTab <= mrViewData.GetRefEndZ() &&
         mrViewData.GetDelMark( aRange ) )
    {
        //! limit to visible area
        if ( aRange.aStart.Col() <= aRange.aEnd.Col() &&
             aRange.aStart.Row() <= aRange.aEnd.Row() )
        {
            Point aStart = mrViewData.GetScrPos( aRange.aStart.Col(),
                                                 aRange.aStart.Row(), eWhich );
            Point aEnd = mrViewData.GetScrPos( aRange.aEnd.Col()+1,
                                               aRange.aEnd.Row()+1, eWhich );
            aEnd.AdjustX( -1 );
            aEnd.AdjustY( -1 );

            aPixRect = tools::Rectangle( aStart,aEnd );
        }
    }

    if ( !aPixRect.IsEmpty() )
    {
        // #i70788# get the OverlayManager safely
        rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();

        if (xOverlayManager.is() && !comphelper::LibreOfficeKit::isActive())
        {
            std::vector< basegfx::B2DRange > aRanges;
            const basegfx::B2DHomMatrix aTransform(GetOutDev()->GetInverseViewTransformation());
            basegfx::B2DRange aRB(aPixRect.Left(), aPixRect.Top(), aPixRect.Right() + 1, aPixRect.Bottom() + 1);

            aRB.transform(aTransform);
            aRanges.push_back(aRB);

            std::unique_ptr<sdr::overlay::OverlayObject> pOverlay(new sdr::overlay::OverlaySelection(
                sdr::overlay::OverlayType::Invert,
                COL_BLACK,
                std::move(aRanges),
                false));

            xOverlayManager->add(*pOverlay);
            mpOOShrink.reset(new sdr::overlay::OverlayObjectList);
            mpOOShrink->append(std::move(pOverlay));
        }
    }

    if ( aOldMode != aDrawMode )
        SetMapMode( aOldMode );
}

// #i70788# central method to get the OverlayManager safely
rtl::Reference<sdr::overlay::OverlayManager> ScGridWindow::getOverlayManager() const
{
    SdrPageView* pPV = mrViewData.GetView()->GetScDrawView()->GetSdrPageView();

    if(pPV)
    {
        SdrPageWindow* pPageWin = pPV->FindPageWindow( *GetOutDev() );

        if ( pPageWin )
        {
            return pPageWin->GetOverlayManager();
        }
    }

    return rtl::Reference<sdr::overlay::OverlayManager>();
}

void ScGridWindow::flushOverlayManager()
{
    // #i70788# get the OverlayManager safely
    rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = getOverlayManager();

    if (xOverlayManager.is())
        xOverlayManager->flush();
}

ScViewData& ScGridWindow::getViewData()
{
    return mrViewData;
}

FactoryFunction ScGridWindow::GetUITestFactory() const
{
    return ScGridWinUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
