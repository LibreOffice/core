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

#include <memory>
#include <vcl/weld.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>
#include <svtools/scriptedtext.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "swmodule.hxx"
#include "swtypes.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "tautofmt.hxx"
#include "shellres.hxx"

using namespace com::sun::star;

#define FRAME_OFFSET 4

class SwStringInputDlg
{
public:
    SwStringInputDlg(Weld::Window* pParent, const OUString& rTitle,
                     const OUString& rEditTitle, const OUString& rDefault);

    OUString GetInputString() const;

    short Execute()
    {
        short nRet = m_xDialog->run();
        m_xDialog->hide();
        return nRet;
    }

private:
    std::unique_ptr<Weld::Builder> m_xBuilder;
    std::unique_ptr<Weld::Dialog> m_xDialog;
    std::unique_ptr<Weld::Label> m_xLabel;
    std::unique_ptr<Weld::Entry> m_xEdInput; // Edit obtains the focus.
};

SwStringInputDlg::SwStringInputDlg(Weld::Window* pParent, const OUString& rTitle,
    const OUString& rEditTitle, const OUString& rDefault)
    : m_xBuilder(Application::CreateBuilder("modules/swriter/ui/stringinput.ui"))
{
    m_xDialog.reset(m_xBuilder->weld_dialog("StringInputDialog"));
    m_xDialog->set_title(rTitle);
    m_xDialog->set_transient_for(pParent);

    m_xLabel.reset(m_xBuilder->weld_label("name"));
    m_xLabel->set_label(rEditTitle);

    m_xEdInput.reset(m_xBuilder->weld_entry("edit"));
    m_xEdInput->set_text(rDefault);
}

OUString SwStringInputDlg::GetInputString() const
{
    return m_xEdInput->get_text();
}

// AutoFormat-Dialogue:
SwAutoFormatDlg::SwAutoFormatDlg(Weld::Window* pParent, SwWrtShell* pWrtShell,
                                 bool bAutoFormat, const SwTableAutoFormat* pSelFormat)
    : m_xBuilder(Application::CreateBuilder("modules/swriter/ui/autoformattable.ui"))
    , m_aWndPreview(m_xBuilder->weld_drawing_area("preview"))
    , aStrTitle(SwResId(STR_ADD_AUTOFORMAT_TITLE))
    , aStrLabel(SwResId(STR_ADD_AUTOFORMAT_LABEL))
    , aStrClose(SwResId(STR_BTN_AUTOFORMAT_CLOSE))
    , aStrDelTitle(SwResId(STR_DEL_AUTOFORMAT_TITLE))
    , aStrDelMsg(SwResId(STR_DEL_AUTOFORMAT_MSG))
    , aStrRenameTitle(SwResId(STR_RENAME_AUTOFORMAT_TITLE))
    , aStrInvalidFormat(SwResId(STR_INVALID_AUTOFORMAT_NAME))
    , pShell(pWrtShell)
    , nIndex(0)
    , nDfltStylePos(0)
    , bCoreDataChanged(false)
    , bSetAutoFormat(bAutoFormat)
{
    m_xDialog.reset(m_xBuilder->weld_dialog("AutoFormatTableDialog"));
    m_xDialog->set_transient_for(pParent);
    m_xLbFormat.reset(m_xBuilder->weld_tree_view("formatlb"));
    m_xBtnNumFormat.reset(m_xBuilder->weld_check_button("numformatcb"));
    m_xBtnBorder.reset(m_xBuilder->weld_check_button("bordercb"));
    m_xBtnFont.reset(m_xBuilder->weld_check_button("fontcb"));
    m_xBtnPattern.reset(m_xBuilder->weld_check_button("patterncb"));
    m_xBtnAlignment.reset(m_xBuilder->weld_check_button("alignmentcb"));
    m_xBtnOk.reset(m_xBuilder->weld_button("ok"));
    m_xBtnCancel.reset(m_xBuilder->weld_button("cancel"));
    m_xBtnAdd.reset(m_xBuilder->weld_button("add"));
    m_xBtnRemove.reset(m_xBuilder->weld_button("remove"));
    m_xBtnRename.reset(m_xBuilder->weld_button("rename"));

    m_aWndPreview.DetectRTL(pWrtShell);

    pTableTable = new SwTableAutoFormatTable;
    pTableTable->Load();

    Init(pSelFormat);
}

SwAutoFormatDlg::~SwAutoFormatDlg()
{
    if (bCoreDataChanged)
        pTableTable->Save();
    delete pTableTable;
}

void SwAutoFormatDlg::Init( const SwTableAutoFormat* pSelFormat )
{
    Link<Weld::ToggleButton&, void> aLk(LINK(this, SwAutoFormatDlg, CheckHdl));
    m_xBtnBorder->connect_toggled(aLk);
    m_xBtnFont->connect_toggled(aLk);
    m_xBtnPattern->connect_toggled(aLk);
    m_xBtnAlignment->connect_toggled(aLk);
    m_xBtnNumFormat->connect_toggled(aLk);

    m_xBtnAdd->connect_clicked(LINK(this, SwAutoFormatDlg, AddHdl));
    m_xBtnRemove->connect_clicked(LINK(this, SwAutoFormatDlg, RemoveHdl));
    m_xBtnRename->connect_clicked(LINK(this, SwAutoFormatDlg, RenameHdl));
    m_xBtnOk->connect_clicked(LINK(this, SwAutoFormatDlg, OkHdl));
    m_xLbFormat->connect_changed(LINK(this, SwAutoFormatDlg, SelFormatHdl));

    m_xBtnAdd->set_sensitive(bSetAutoFormat);

    nIndex = 0;
    if( !bSetAutoFormat )
    {
        // Then the list to be expanded by the entry "- none -".
        m_xLbFormat->append(SwViewShell::GetShellRes()->aStrNone);
        nDfltStylePos = 1;
        nIndex = 255;
    }

    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(pTableTable->size());
            i < nCount; i++)
    {
        SwTableAutoFormat const& rFormat = (*pTableTable)[ i ];
        m_xLbFormat->append(rFormat.GetName());
        if (pSelFormat && rFormat.GetName() == pSelFormat->GetName())
            nIndex = i;
    }

    m_xLbFormat->select(255 != nIndex ? (nDfltStylePos + nIndex) : 0);
    SelFormatHdl(*m_xLbFormat);
}

void SwAutoFormatDlg::UpdateChecks( const SwTableAutoFormat& rFormat, bool bEnable )
{
    m_xBtnNumFormat->set_sensitive(bEnable);
    m_xBtnNumFormat->set_active(rFormat.IsValueFormat());

    m_xBtnBorder->set_sensitive(bEnable);
    m_xBtnBorder->set_active(rFormat.IsFrame());

    m_xBtnFont->set_sensitive(bEnable);
    m_xBtnFont->set_active(rFormat.IsFont());

    m_xBtnPattern->set_sensitive(bEnable);
    m_xBtnPattern->set_active(rFormat.IsBackground());

    m_xBtnAlignment->set_sensitive(bEnable);
    m_xBtnAlignment->set_active(rFormat.IsJustify());
}

SwTableAutoFormat* SwAutoFormatDlg::FillAutoFormatOfIndex() const
{
    if (nIndex == 255)
        return nullptr;
    return new SwTableAutoFormat((*pTableTable)[nIndex]);
}

// Handler:
IMPL_LINK(SwAutoFormatDlg, CheckHdl, Weld::ToggleButton&, rBtn, void)
{
    if (nIndex == 255)
        return;

    SwTableAutoFormat& rData  = (*pTableTable)[nIndex];
    bool bCheck = rBtn.get_active(), bDataChgd = true;

    if (&rBtn == m_xBtnNumFormat.get())
        rData.SetValueFormat( bCheck );
    else if (&rBtn == m_xBtnBorder.get())
        rData.SetFrame( bCheck );
    else if (&rBtn == m_xBtnFont.get())
        rData.SetFont( bCheck );
    else if (&rBtn == m_xBtnPattern.get())
        rData.SetBackground( bCheck );
    else if (&rBtn == m_xBtnAlignment.get())
        rData.SetJustify( bCheck );
    else
        bDataChgd = false;

    if( bDataChgd )
    {
        if( !bCoreDataChanged )
        {
            m_xBtnCancel->set_label(aStrClose);
            bCoreDataChanged = true;
        }

        m_aWndPreview.NotifyChange(rData);
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, AddHdl, Weld::Button&, void)
{
    bool bOk = false, bFormatInserted = false;
    while( !bOk )
    {
        std::unique_ptr<SwStringInputDlg> xDlg(new SwStringInputDlg(m_xDialog.get(), aStrTitle, aStrLabel, OUString()));
        if (RET_OK == xDlg->Execute())
        {
            const OUString aFormatName(xDlg->GetInputString());

            if ( !aFormatName.isEmpty() )
            {
                size_t n;
                for( n = 0; n < pTableTable->size(); ++n )
                    if( (*pTableTable)[n].GetName() == aFormatName )
                        break;

                if( n >= pTableTable->size() )
                {
                    // Format with the name does not already exist, so take up.
                    std::unique_ptr<SwTableAutoFormat> pNewData(
                            new SwTableAutoFormat(aFormatName));
                    bool bGetOk = pShell->GetTableAutoFormat( *pNewData );
                    SAL_WARN_IF(!bGetOk, "sw.ui", "GetTableAutoFormat failed for: " << aFormatName);

                    // Insert sorted!!
                    for( n = 1; n < pTableTable->size(); ++n )
                        if( (*pTableTable)[ n ].GetName() > aFormatName )
                            break;

                    pTableTable->InsertAutoFormat(n, std::move(pNewData));
                    m_xLbFormat->insert(aFormatName, nDfltStylePos + n);
                    m_xLbFormat->select(nDfltStylePos + n);
                    bFormatInserted = true;
                    m_xBtnAdd->set_sensitive(false);
                    if ( !bCoreDataChanged )
                    {
                        m_xBtnCancel->set_label(aStrClose);
                        bCoreDataChanged = true;
                    }

                    SelFormatHdl(*m_xLbFormat);
                    bOk = true;
                }
            }

            if( !bFormatInserted )
            {
                std::unique_ptr<Weld::Dialog> xDialog(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Error,
                                                         VclButtonsType::OkCancel, aStrInvalidFormat));
                bOk = RET_CANCEL == xDialog->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RemoveHdl, Weld::Button&, void)
{
    OUString aMessage = aStrDelMsg;
    aMessage += "\n\n";
    aMessage += m_xLbFormat->get_selected();
    aMessage += "\n";

    std::unique_ptr<Weld::Dialog> xDialog(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Info,
                                             VclButtonsType::OkCancel, aMessage));
    xDialog->set_title(aStrDelTitle);

    if (xDialog->run() == RET_OK)
    {
        m_xLbFormat->remove(nDfltStylePos + nIndex);
        m_xLbFormat->select(nDfltStylePos + nIndex - 1);

        pTableTable->EraseAutoFormat(nIndex);
        nIndex--;

        if( !nIndex )
        {
            m_xBtnRemove->set_sensitive(false);
            m_xBtnRename->set_sensitive(false);
        }

        if( !bCoreDataChanged )
        {
            m_xBtnCancel->set_label(aStrClose);
            bCoreDataChanged = true;
        }
    }
    xDialog.reset();

    SelFormatHdl(*m_xLbFormat);
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RenameHdl, Weld::Button&, void)
{
    bool bOk = false;
    while( !bOk )
    {
        std::unique_ptr<SwStringInputDlg> xDlg(new SwStringInputDlg(m_xDialog.get(), aStrRenameTitle,
                                                                    m_xLbFormat->get_selected(),
                                                                    OUString()));
        if (xDlg->Execute() == RET_OK)
        {
            bool bFormatRenamed = false;
            const OUString aFormatName(xDlg->GetInputString());

            if ( !aFormatName.isEmpty() )
            {
                size_t n;
                for( n = 0; n < pTableTable->size(); ++n )
                    if ((*pTableTable)[n].GetName() == aFormatName)
                        break;

                if( n >= pTableTable->size() )
                {
                    // no format with this name exists, so rename it
                    m_xLbFormat->remove(nDfltStylePos + nIndex);
                    std::unique_ptr<SwTableAutoFormat> p(
                            pTableTable->ReleaseAutoFormat(nIndex));

                    p->SetName( aFormatName );

                    // keep all arrays sorted!
                    for( n = 1; n < pTableTable->size(); ++n )
                        if ((*pTableTable)[n].GetName() > aFormatName)
                        {
                            break;
                        }

                    pTableTable->InsertAutoFormat( n, std::move(p) );
                    m_xLbFormat->insert(aFormatName, nDfltStylePos + n);
                    m_xLbFormat->select(nDfltStylePos + n);

                    if ( !bCoreDataChanged )
                    {
                        m_xBtnCancel->set_label(aStrClose);
                        bCoreDataChanged = true;
                    }

                    SelFormatHdl(*m_xLbFormat);
                    bOk = true;
                    bFormatRenamed = true;
                }
            }

            if( !bFormatRenamed )
            {
                std::unique_ptr<Weld::Dialog> xDialog(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Error,
                                                         VclButtonsType::OkCancel, aStrInvalidFormat));
                bOk = RET_CANCEL == xDialog->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, SelFormatHdl, Weld::TreeView&, void)
{
    bool bBtnEnable = false;
    sal_uInt8 nOldIdx = nIndex;
    int nSelPos = m_xLbFormat->get_selected_index();
    if (nSelPos >= nDfltStylePos)
    {
        nIndex = nSelPos - nDfltStylePos;
        m_aWndPreview.NotifyChange((*pTableTable)[nIndex]);
        bBtnEnable = 0 != nIndex;
        UpdateChecks( (*pTableTable)[nIndex], true );
    }
    else
    {
        nIndex = 255;

        SwTableAutoFormat aTmp( SwViewShell::GetShellRes()->aStrNone );
        aTmp.SetFont( false );
        aTmp.SetJustify( false );
        aTmp.SetFrame( false );
        aTmp.SetBackground( false );
        aTmp.SetValueFormat( false );
        aTmp.SetWidthHeight( false );

        if (nOldIdx != nIndex)
            m_aWndPreview.NotifyChange(aTmp);
        UpdateChecks( aTmp, false );
    }

    m_xBtnRemove->set_sensitive(bBtnEnable);
    m_xBtnRename->set_sensitive(bBtnEnable);
}

IMPL_LINK_NOARG(SwAutoFormatDlg, OkHdl, Weld::Button&, void)
{
    if( bSetAutoFormat )
        pShell->SetTableStyle((*pTableTable)[nIndex]);
    m_xDialog->response(RET_OK);
}

AutoFormatPreview::AutoFormatPreview(Weld::DrawingArea* pDrawingArea)
    : mxDrawingArea(pDrawingArea)
    , aCurData(OUString())
    , bFitWidth(false)
    , mbRTL(false)
    , aStrJan(SwResId(STR_JAN))
    , aStrFeb(SwResId(STR_FEB))
    , aStrMar(SwResId(STR_MAR))
    , aStrNorth(SwResId(STR_NORTH))
    , aStrMid(SwResId(STR_MID))
    , aStrSouth(SwResId(STR_SOUTH))
    , aStrSum(SwResId(STR_SUM))
{
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    m_xBreak = i18n::BreakIterator::create(xContext);
    mxNumFormat.reset(new SvNumberFormatter(xContext, LANGUAGE_SYSTEM));

    Init();

    mxDrawingArea->connect_size_allocate(LINK(this, AutoFormatPreview, DoResize));
    mxDrawingArea->connect_draw(LINK(this, AutoFormatPreview, DoPaint));
}

IMPL_LINK(AutoFormatPreview, DoResize, const Size&, rSize, void)
{
    aPrvSize = Size(rSize.Width() - 6, rSize.Height() - 30);
    nLabelColWidth = (aPrvSize.Width() - 4) / 4 - 12;
    nDataColWidth1 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 3;
    nDataColWidth2 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 4;
    nRowHeight = (aPrvSize.Height() - 4) / 5;
    NotifyChange(aCurData);
}

void AutoFormatPreview::DetectRTL(SwWrtShell const * pWrtShell)
{
    if (!pWrtShell->IsCursorInTable()) // We haven't created the table yet
        mbRTL = AllSettings::GetLayoutRTL();
    else
        mbRTL = pWrtShell->IsTableRightToLeft();
}

static void lcl_SetFontProperties(
        vcl::Font& rFont,
        const SvxFontItem& rFontItem,
        const SvxWeightItem& rWeightItem,
        const SvxPostureItem& rPostureItem )
{
    rFont.SetFamily     ( rFontItem.GetFamily() );
    rFont.SetFamilyName ( rFontItem.GetFamilyName() );
    rFont.SetStyleName  ( rFontItem.GetStyleName() );
    rFont.SetCharSet    ( rFontItem.GetCharSet() );
    rFont.SetPitch      ( rFontItem.GetPitch() );
    rFont.SetWeight     ( rWeightItem.GetValue() );
    rFont.SetItalic     ( rPostureItem.GetValue() );
}

#define SETONALLFONTS( MethodName, Value )                  \
rFont.MethodName( Value );                                  \
rCJKFont.MethodName( Value );                               \
rCTLFont.MethodName( Value );

void AutoFormatPreview::MakeFonts(vcl::RenderContext& rRenderContext, sal_uInt8 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont)
{
    const SwBoxAutoFormat& rBoxFormat = aCurData.GetBoxFormat( nIndex );

    rFont = rCJKFont = rCTLFont = rRenderContext.GetFont();
    Size aFontSize( rFont.GetFontSize().Width(), 10 * rRenderContext.GetDPIScaleFactor() );

    lcl_SetFontProperties( rFont, rBoxFormat.GetFont(), rBoxFormat.GetWeight(), rBoxFormat.GetPosture() );
    lcl_SetFontProperties( rCJKFont, rBoxFormat.GetCJKFont(), rBoxFormat.GetCJKWeight(), rBoxFormat.GetCJKPosture() );
    lcl_SetFontProperties( rCTLFont, rBoxFormat.GetCTLFont(), rBoxFormat.GetCTLWeight(), rBoxFormat.GetCTLPosture() );

    SETONALLFONTS( SetUnderline,        rBoxFormat.GetUnderline().GetValue() );
    SETONALLFONTS( SetOverline,         rBoxFormat.GetOverline().GetValue() );
    SETONALLFONTS( SetStrikeout,        rBoxFormat.GetCrossedOut().GetValue() );
    SETONALLFONTS( SetOutline,          rBoxFormat.GetContour().GetValue() );
    SETONALLFONTS( SetShadow,           rBoxFormat.GetShadowed().GetValue() );
    SETONALLFONTS( SetColor,            rBoxFormat.GetColor().GetValue() );
    SETONALLFONTS( SetFontSize,  aFontSize );
    SETONALLFONTS( SetTransparent,      true );
}

sal_uInt8 AutoFormatPreview::GetFormatIndex( size_t nCol, size_t nRow ) const
{
    static const sal_uInt8 pnFormatMap[] =
    {
        0,  1,  2,  1,  3,
        4,  5,  6,  5,  7,
        8,  9,  10, 9,  11,
        4,  5,  6,  5,  7,
        12, 13, 14, 13, 15
    };
    return pnFormatMap[ maArray.GetCellIndex( nCol, nRow, mbRTL ) ];
}

void AutoFormatPreview::DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow)
{
    // Output of the cell text:
    sal_uLong nNum;
    double nVal;
    OUString cellString;
    sal_uInt8 nIndex = static_cast<sal_uInt8>(maArray.GetCellIndex(nCol, nRow, mbRTL));

    switch(nIndex)
    {
    case 1:
        cellString = aStrJan;
        break;
    case 2:
        cellString = aStrFeb;
        break;
    case 3:
        cellString = aStrMar;
        break;
    case 5:
        cellString = aStrNorth;
        break;
    case 10:
        cellString = aStrMid;
        break;
    case 15:
        cellString = aStrSouth;
        break;
    case  4:
    case 20:
        cellString = aStrSum;
        break;
    case 6:
    case 8:
    case 16:
    case 18:
        nVal = nIndex;
        nNum = 5;
        goto MAKENUMSTR;
    case 17:
    case 7:
        nVal = nIndex;
        nNum = 6;
        goto MAKENUMSTR;
    case 11:
    case 12:
    case 13:
        nVal = nIndex;
        nNum = 12 == nIndex ? 10 : 9;
        goto MAKENUMSTR;
    case 9:
        nVal = 21; nNum = 7;
        goto MAKENUMSTR;
    case 14:
        nVal = 36; nNum = 11;
        goto MAKENUMSTR;
    case 19:
        nVal = 51; nNum = 7;
        goto MAKENUMSTR;
    case 21:
        nVal = 33; nNum = 13;
        goto MAKENUMSTR;
    case 22:
        nVal = 36; nNum = 14;
        goto MAKENUMSTR;
    case 23:
        nVal = 39; nNum = 13;
        goto MAKENUMSTR;
    case 24:
        nVal = 108; nNum = 15;
        goto MAKENUMSTR;

MAKENUMSTR:
        if (aCurData.IsValueFormat())
        {
            OUString sFormat;
            LanguageType eLng, eSys;
            aCurData.GetBoxFormat(sal_uInt8(nNum)).GetValueFormat(sFormat, eLng, eSys);

            short nType;
            bool bNew;
            sal_Int32 nCheckPos;
            sal_uInt32 nKey = mxNumFormat->GetIndexPuttingAndConverting(sFormat, eLng,
                                                                    eSys, nType, bNew, nCheckPos);
            Color* pDummy;
            mxNumFormat->GetOutputString(nVal, nKey, cellString, &pDummy);
        }
        else
            cellString = OUString::number(sal_Int32(nVal));
        break;

    }

    if (!cellString.isEmpty())
    {
        SvtScriptedTextHelper aScriptedText(rRenderContext);
        Size aStrSize;
        sal_uInt8 nFormatIndex = GetFormatIndex( nCol, nRow );
        const basegfx::B2DRange aCellRange(maArray.GetCellRange( nCol, nRow ));
        const tools::Rectangle cellRect(
            basegfx::fround(aCellRange.getMinX()), basegfx::fround(aCellRange.getMinY()),
            basegfx::fround(aCellRange.getMaxX()), basegfx::fround(aCellRange.getMaxY()));
        Point aPos = cellRect.TopLeft();
        long nRightX = 0;

        Size theMaxStrSize(cellRect.GetWidth() - FRAME_OFFSET,
                           cellRect.GetHeight() - FRAME_OFFSET);
        if (aCurData.IsFont())
        {
            vcl::Font aFont, aCJKFont, aCTLFont;
            MakeFonts(rRenderContext, nFormatIndex, aFont, aCJKFont, aCTLFont);
            aScriptedText.SetFonts(&aFont, &aCJKFont, &aCTLFont);
        }
        else
        {
            aScriptedText.SetDefaultFont();
        }

        aScriptedText.SetText(cellString, m_xBreak);
        aStrSize = aScriptedText.GetTextSize();

        if (aCurData.IsFont() &&
            theMaxStrSize.Height() < aStrSize.Height())
        {
                // If the string in this font does not
                // fit into the cell, the standard font
                // is taken again:
                aScriptedText.SetDefaultFont();
                aStrSize = aScriptedText.GetTextSize();
        }

        while (theMaxStrSize.Width() <= aStrSize.Width() &&
               cellString.getLength() > 1)
        {
            cellString = cellString.copy(0, cellString.getLength() - 1);
            aScriptedText.SetText(cellString, m_xBreak);
            aStrSize = aScriptedText.GetTextSize();
        }

        nRightX = cellRect.GetWidth() - aStrSize.Width() - FRAME_OFFSET;

        // vertical (always centering):
        aPos.Y() += (nRowHeight - aStrSize.Height()) / 2;

        // horizontal
        if (mbRTL)
            aPos.X() += nRightX;
        else if (aCurData.IsJustify())
        {
            const SvxAdjustItem& rAdj = aCurData.GetBoxFormat(nFormatIndex).GetAdjust();
            switch (rAdj.GetAdjust())
            {
                case SvxAdjust::Left:
                    aPos.X() += FRAME_OFFSET;
                    break;
                case SvxAdjust::Right:
                    aPos.X() += nRightX;
                    break;
                default:
                    aPos.X() += (cellRect.GetWidth() - aStrSize.Width()) / 2;
                    break;
            }
        }
        else
        {
            // Standard align:
            if (nCol == 0 || nIndex == 4)
            {
                // Text-Label left or sum left aligned
                aPos.X() += FRAME_OFFSET;
            }
            else
            {
                // numbers/dates right aligned
                aPos.X() += nRightX;
            }
        }

        aScriptedText.DrawText(aPos);
    }
}

#undef FRAME_OFFSET

void AutoFormatPreview::DrawBackground(vcl::RenderContext& rRenderContext)
{
    for (size_t nRow = 0; nRow < 5; ++nRow)
    {
        for (size_t nCol = 0; nCol < 5; ++nCol)
        {
            SvxBrushItem aBrushItem(aCurData.GetBoxFormat(GetFormatIndex(nCol, nRow)).GetBackground());

            rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(aBrushItem.GetColor());
            const basegfx::B2DRange aCellRange(maArray.GetCellRange( nCol, nRow ));
            rRenderContext.DrawRect(
                tools::Rectangle(
                    basegfx::fround(aCellRange.getMinX()), basegfx::fround(aCellRange.getMinY()),
                    basegfx::fround(aCellRange.getMaxX()), basegfx::fround(aCellRange.getMaxY())));
            rRenderContext.Pop();
        }
    }
}

void AutoFormatPreview::PaintCells(vcl::RenderContext& rRenderContext)
{
    // 1) background
    if (aCurData.IsBackground())
        DrawBackground(rRenderContext);

    // 2) values
    for (size_t nRow = 0; nRow < 5; ++nRow)
        for (size_t nCol = 0; nCol < 5; ++nCol)
            DrawString(rRenderContext, nCol, nRow);

    // 3) border
    if (aCurData.IsFrame())
    {
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
            drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
                rRenderContext,
                aNewViewInformation2D));

        if (pProcessor2D)
        {
            pProcessor2D->process(maArray.CreateB2DPrimitiveArray());
            pProcessor2D.reset();
        }
    }
}

void AutoFormatPreview::Init()
{
    maArray.Initialize( 5, 5 );
    nLabelColWidth = 0;
    nDataColWidth1 = 0;
    nDataColWidth2 = 0;
    nRowHeight = 0;
    CalcCellArray( false );
    CalcLineMap();
}

void AutoFormatPreview::CalcCellArray( bool _bFitWidth )
{
    maArray.SetAllColWidths( _bFitWidth ? nDataColWidth2 : nDataColWidth1 );
    maArray.SetColWidth( 0, nLabelColWidth );
    maArray.SetColWidth( 4, nLabelColWidth );

    maArray.SetAllRowHeights( nRowHeight );

    aPrvSize.Width() = maArray.GetWidth() + 4;
    aPrvSize.Height() = maArray.GetHeight() + 4;
}

inline void lclSetStyleFromBorder( svx::frame::Style& rStyle, const ::editeng::SvxBorderLine* pBorder )
{
    rStyle.Set( pBorder, 0.05, 5 );
}

void AutoFormatPreview::CalcLineMap()
{
    for( size_t nRow = 0; nRow < 5; ++nRow )
    {
        for( size_t nCol = 0; nCol < 5; ++nCol )
        {
            svx::frame::Style aStyle;

            const SvxBoxItem& rItem = aCurData.GetBoxFormat( GetFormatIndex( nCol, nRow ) ).GetBox();
            lclSetStyleFromBorder( aStyle, rItem.GetLeft() );
            maArray.SetCellStyleLeft( nCol, nRow, aStyle );
            lclSetStyleFromBorder( aStyle, rItem.GetRight() );
            maArray.SetCellStyleRight( nCol, nRow, aStyle );
            lclSetStyleFromBorder( aStyle, rItem.GetTop() );
            maArray.SetCellStyleTop( nCol, nRow, aStyle );
            lclSetStyleFromBorder( aStyle, rItem.GetBottom() );
            maArray.SetCellStyleBottom( nCol, nRow, aStyle );

// FIXME - uncomment to draw diagonal borders
//            lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, true ).GetLine() );
//            maArray.SetCellStyleTLBR( nCol, nRow, aStyle );
//            lclSetStyleFromBorder( aStyle, GetDiagItem( nCol, nRow, false ).GetLine() );
//            maArray.SetCellStyleBLTR( nCol, nRow, aStyle );
        }
    }
}

void AutoFormatPreview::NotifyChange( const SwTableAutoFormat& rNewData )
{
    aCurData  = rNewData;
    bFitWidth = aCurData.IsJustify();  // true;  //???
    CalcCellArray( bFitWidth );
    CalcLineMap();
    mxDrawingArea->queue_draw();
}

IMPL_LINK(AutoFormatPreview, DoPaint, vcl::RenderContext&, rRenderContext, void)
{
    rRenderContext.Push(PushFlags::ALL);

    DrawModeFlags nOldDrawMode = rRenderContext.GetDrawMode();
    if (rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
        rRenderContext.SetDrawMode(DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient);

    Size theWndSize = rRenderContext.GetOutputSizePixel();

    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetTransparent(true);

    rRenderContext.SetFont(aFont);
    rRenderContext.SetLineColor();
    const Color& rWinColor = rRenderContext.GetSettings().GetStyleSettings().GetWindowColor();
    rRenderContext.SetBackground(Wallpaper(rWinColor));
    rRenderContext.SetFillColor(rWinColor);

    // Draw the Frame
    Color oldColor = rRenderContext.GetLineColor();
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(tools::Rectangle(Point(0,0), theWndSize));
    rRenderContext.SetLineColor(oldColor);

    // Center the preview
    maArray.SetXOffset(2 + (theWndSize.Width() - aPrvSize.Width()) / 2);
    maArray.SetYOffset(2 + (theWndSize.Height() - aPrvSize.Height()) / 2);
    // Draw cells on virtual device
    PaintCells(rRenderContext);

    rRenderContext.SetDrawMode(nOldDrawMode);
    rRenderContext.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
