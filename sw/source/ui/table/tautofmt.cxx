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
#include <svx/framelinkarray.hxx>
#include "app.hrc"
#include "swmodule.hxx"
#include "swtypes.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "tblafmt.hxx"
#include "tautofmt.hxx"
#include "shellres.hxx"

using namespace com::sun::star;

#define FRAME_OFFSET 4

class AutoFormatPreview : public vcl::Window
{
public:
    AutoFormatPreview(vcl::Window* pParent, WinBits nStyle);
    virtual ~AutoFormatPreview();
    virtual void dispose() override;

    void NotifyChange( const SwTableAutoFormat& rNewData );

    void DetectRTL(SwWrtShell* pWrtShell);

    virtual void Resize() override;
protected:
    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

private:
    SwTableAutoFormat          aCurData;
    ScopedVclPtr<VirtualDevice> aVD;
    svx::frame::Array       maArray;            /// Implementation to draw the frame borders.
    bool                    bFitWidth;
    bool                    mbRTL;
    Size                    aPrvSize;
    long                    nLabelColWidth;
    long                    nDataColWidth1;
    long                    nDataColWidth2;
    long                    nRowHeight;
    const OUString          aStrJan;
    const OUString          aStrFeb;
    const OUString          aStrMar;
    const OUString          aStrNorth;
    const OUString          aStrMid;
    const OUString          aStrSouth;
    const OUString          aStrSum;
    SvNumberFormatter*      pNumFormat;

    uno::Reference<i18n::XBreakIterator> m_xBreak;

    void    Init();
    void    DoPaint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    void    CalcCellArray(bool bFitWidth);
    void    CalcLineMap();
    void    PaintCells(vcl::RenderContext& rRenderContext);

    sal_uInt8           GetFormatIndex( size_t nCol, size_t nRow ) const;
    const SvxBoxItem&   GetBoxItem( size_t nCol, size_t nRow ) const;

    void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    void DrawStrings(vcl::RenderContext& rRenderContext);
    void DrawBackground(vcl::RenderContext& rRenderContext);

    void MakeFonts(sal_uInt8 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont);
};

class SwStringInputDlg : public ModalDialog
{
public:
    SwStringInputDlg(vcl::Window* pParent, const OUString& rTitle,
                     const OUString& rEditTitle, const OUString& rDefault );
    virtual ~SwStringInputDlg();
    virtual void dispose() override;

    OUString GetInputString() const;

private:
    VclPtr<Edit> m_pEdInput; // Edit obtains the focus.
};

SwStringInputDlg::SwStringInputDlg(vcl::Window* pParent, const OUString& rTitle,
    const OUString& rEditTitle, const OUString& rDefault)
    : ModalDialog(pParent, "StringInputDialog", "modules/swriter/ui/stringinput.ui")
{
    get<FixedText>("name")->SetText(rEditTitle);
    get(m_pEdInput, "edit");

    SetText(rTitle);
    m_pEdInput->SetText(rDefault);
}

OUString SwStringInputDlg::GetInputString() const
{
    return m_pEdInput->GetText();
}

SwStringInputDlg::~SwStringInputDlg()
{
    disposeOnce();
}

void SwStringInputDlg::dispose()
{
    m_pEdInput.clear();
    ModalDialog::dispose();
}

// AutoFormat-Dialogue:

SwAutoFormatDlg::SwAutoFormatDlg( vcl::Window* pParent, SwWrtShell* pWrtShell,
                    bool bAutoFormat, const SwTableAutoFormat* pSelFormat )
    : SfxModalDialog(pParent, "AutoFormatTableDialog", "modules/swriter/ui/autoformattable.ui")
    , aStrTitle(SW_RES(STR_ADD_AUTOFORMAT_TITLE))
    , aStrLabel(SW_RES(STR_ADD_AUTOFORMAT_LABEL))
    , aStrClose(SW_RES(STR_BTN_AUTOFORMAT_CLOSE))
    , aStrDelTitle(SW_RES(STR_DEL_AUTOFORMAT_TITLE))
    , aStrDelMsg(SW_RES(STR_DEL_AUTOFORMAT_MSG))
    , aStrRenameTitle(SW_RES(STR_RENAME_AUTOFORMAT_TITLE))
    , aStrInvalidFormat(SW_RES(STR_INVALID_AUTOFORMAT_NAME))
    , pShell(pWrtShell)
    , nIndex(0)
    , nDfltStylePos(0)
    , bCoreDataChanged(false)
    , bSetAutoFormat(bAutoFormat)
{
    get(m_pLbFormat, "formatlb");
    get(m_pFormatting, "formatting");
    get(m_pBtnNumFormat, "numformatcb");
    get(m_pBtnBorder, "bordercb");
    get(m_pBtnFont, "fontcb");
    get(m_pBtnPattern, "patterncb");
    get(m_pBtnAlignment, "alignmentcb");
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnRename, "rename");
    get(m_pWndPreview, "preview");

    m_pWndPreview->DetectRTL(pWrtShell);

    pTableTable = new SwTableAutoFormatTable;
    pTableTable->Load();

    Init(pSelFormat);
}

SwAutoFormatDlg::~SwAutoFormatDlg()
{
    disposeOnce();
}

void SwAutoFormatDlg::dispose()
{
    if (bCoreDataChanged)
        pTableTable->Save();
    delete pTableTable;
    m_pLbFormat.clear();
    m_pFormatting.clear();
    m_pBtnNumFormat.clear();
    m_pBtnBorder.clear();
    m_pBtnFont.clear();
    m_pBtnPattern.clear();
    m_pBtnAlignment.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    m_pBtnRename.clear();
    m_pWndPreview.clear();
    SfxModalDialog::dispose();
}

void SwAutoFormatDlg::Init( const SwTableAutoFormat* pSelFormat )
{
    Link<Button*,void> aLk( LINK( this, SwAutoFormatDlg, CheckHdl ) );
    m_pBtnBorder->SetClickHdl( aLk );
    m_pBtnFont->SetClickHdl( aLk );
    m_pBtnPattern->SetClickHdl( aLk );
    m_pBtnAlignment->SetClickHdl( aLk );
    m_pBtnNumFormat->SetClickHdl( aLk );

    m_pBtnAdd->SetClickHdl ( LINK( this, SwAutoFormatDlg, AddHdl ) );
    m_pBtnRemove->SetClickHdl ( LINK( this, SwAutoFormatDlg, RemoveHdl ) );
    m_pBtnRename->SetClickHdl ( LINK( this, SwAutoFormatDlg, RenameHdl ) );
    m_pBtnOk->SetClickHdl ( LINK( this, SwAutoFormatDlg, OkHdl ) );
    m_pLbFormat->SetSelectHdl( LINK( this, SwAutoFormatDlg, SelFormatHdl ) );

    m_pBtnAdd->Enable( bSetAutoFormat );

    nIndex = 0;
    if( !bSetAutoFormat )
    {
        // Then the list to be expanded by the entry "- none -".
        m_pLbFormat->InsertEntry( SwViewShell::GetShellRes()->aStrNone );
        nDfltStylePos = 1;
        nIndex = 255;
    }

    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(pTableTable->size());
            i < nCount; i++)
    {
        SwTableAutoFormat const& rFormat = (*pTableTable)[ i ];
        m_pLbFormat->InsertEntry(rFormat.GetName());
        if (pSelFormat && rFormat.GetName() == pSelFormat->GetName())
            nIndex = i;
    }

    m_pLbFormat->SelectEntryPos( 255 != nIndex ? (nDfltStylePos + nIndex) : 0 );
    SelFormatHdl( *m_pLbFormat );
}

void SwAutoFormatDlg::UpdateChecks( const SwTableAutoFormat& rFormat, bool bEnable )
{
    m_pBtnNumFormat->Enable( bEnable );
    m_pBtnNumFormat->Check( rFormat.IsValueFormat() );

    m_pBtnBorder->Enable( bEnable );
    m_pBtnBorder->Check( rFormat.IsFrame() );

    m_pBtnFont->Enable( bEnable );
    m_pBtnFont->Check( rFormat.IsFont() );

    m_pBtnPattern->Enable( bEnable );
    m_pBtnPattern->Check( rFormat.IsBackground() );

    m_pBtnAlignment->Enable( bEnable );
    m_pBtnAlignment->Check( rFormat.IsJustify() );
}

void SwAutoFormatDlg::FillAutoFormatOfIndex( SwTableAutoFormat*& rToFill ) const
{
    if( 255 != nIndex )
    {
        if( rToFill )
            *rToFill = (*pTableTable)[ nIndex ];
        else
            rToFill = new SwTableAutoFormat( (*pTableTable)[ nIndex ] );
    }
    else
    {
        delete rToFill;
        rToFill = nullptr;
    }
}

// Handler:

IMPL_LINK_TYPED( SwAutoFormatDlg, CheckHdl, Button *, pBtn, void )
{
    SwTableAutoFormat* pData  = &(*pTableTable)[nIndex];
    bool bCheck = static_cast<CheckBox*>(pBtn)->IsChecked(), bDataChgd = true;

    if( pBtn == m_pBtnNumFormat )
        pData->SetValueFormat( bCheck );
    else if ( pBtn == m_pBtnBorder )
        pData->SetFrame( bCheck );
    else if ( pBtn == m_pBtnFont )
        pData->SetFont( bCheck );
    else if ( pBtn == m_pBtnPattern )
        pData->SetBackground( bCheck );
    else if ( pBtn == m_pBtnAlignment )
        pData->SetJustify( bCheck );
    else
        bDataChgd = false;

    if( bDataChgd )
    {
        if( !bCoreDataChanged )
        {
            m_pBtnCancel->SetText( aStrClose );
            bCoreDataChanged = true;
        }

        m_pWndPreview->NotifyChange( *pData );
    }
}

IMPL_LINK_NOARG_TYPED(SwAutoFormatDlg, AddHdl, Button*, void)
{
    bool bOk = false, bFormatInserted = false;
    while( !bOk )
    {
        VclPtrInstance<SwStringInputDlg> pDlg( this, aStrTitle,
                                               aStrLabel, OUString() );
        if( RET_OK == pDlg->Execute() )
        {
            const OUString aFormatName( pDlg->GetInputString() );

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
                    pShell->GetTableAutoFormat( *pNewData );

                    // Insert sorted!!
                    for( n = 1; n < pTableTable->size(); ++n )
                        if( (*pTableTable)[ n ].GetName() > aFormatName )
                            break;

                    pTableTable->InsertAutoFormat(n, std::move(pNewData));
                    m_pLbFormat->InsertEntry( aFormatName, nDfltStylePos + n );
                    m_pLbFormat->SelectEntryPos( nDfltStylePos + n );
                    bFormatInserted = true;
                    m_pBtnAdd->Enable( false );
                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = true;
                    }

                    SelFormatHdl( *m_pLbFormat );
                    bOk = true;
                }
            }

            if( !bFormatInserted )
            {
                bOk = RET_CANCEL == ScopedVclPtrInstance<MessageDialog>(this, aStrInvalidFormat, VclMessageType::Error, VCL_BUTTONS_OK_CANCEL)
                                    ->Execute();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG_TYPED(SwAutoFormatDlg, RemoveHdl, Button*, void)
{
    OUString aMessage = aStrDelMsg;
    aMessage += "\n\n";
    aMessage += m_pLbFormat->GetSelectEntry();
    aMessage += "\n";

    VclPtrInstance<MessBox> pBox( this, WinBits( WB_OK_CANCEL ),
                                  aStrDelTitle, aMessage );

    if ( pBox->Execute() == RET_OK )
    {
        m_pLbFormat->RemoveEntry( nDfltStylePos + nIndex );
        m_pLbFormat->SelectEntryPos( nDfltStylePos + nIndex-1 );

        pTableTable->EraseAutoFormat(nIndex);
        nIndex--;

        if( !nIndex )
        {
            m_pBtnRemove->Enable(false);
            m_pBtnRename->Enable(false);
        }

        if( !bCoreDataChanged )
        {
            m_pBtnCancel->SetText( aStrClose );
            bCoreDataChanged = true;
        }
    }
    pBox.reset();

    SelFormatHdl( *m_pLbFormat );
}

IMPL_LINK_NOARG_TYPED(SwAutoFormatDlg, RenameHdl, Button*, void)
{
    bool bOk = false;
    while( !bOk )
    {
        VclPtrInstance<SwStringInputDlg> pDlg( this, aStrRenameTitle,
                                               m_pLbFormat->GetSelectEntry(),
                                               OUString() );
        if( pDlg->Execute() == RET_OK )
        {
            bool bFormatRenamed = false;
            const OUString aFormatName( pDlg->GetInputString() );

            if ( !aFormatName.isEmpty() )
            {
                size_t n;
                for( n = 0; n < pTableTable->size(); ++n )
                    if ((*pTableTable)[n].GetName() == aFormatName)
                        break;

                if( n >= pTableTable->size() )
                {
                    // no format with this name exists, so rename it
                    m_pLbFormat->RemoveEntry( nDfltStylePos + nIndex );
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
                    m_pLbFormat->InsertEntry( aFormatName, nDfltStylePos + n );
                    m_pLbFormat->SelectEntryPos( nDfltStylePos + n );

                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = true;
                    }

                    SelFormatHdl( *m_pLbFormat );
                    bOk = true;
                    bFormatRenamed = true;
                }
            }

            if( !bFormatRenamed )
            {
                bOk = RET_CANCEL == ScopedVclPtrInstance<MessageDialog>(this, aStrInvalidFormat, VclMessageType::Error, VCL_BUTTONS_OK_CANCEL)
                                    ->Execute();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG_TYPED(SwAutoFormatDlg, SelFormatHdl, ListBox&, void)
{
    bool bBtnEnable = false;
    sal_uInt8 nSelPos = (sal_uInt8) m_pLbFormat->GetSelectEntryPos(), nOldIdx = nIndex;
    if( nSelPos >= nDfltStylePos )
    {
        nIndex = nSelPos - nDfltStylePos;
        m_pWndPreview->NotifyChange( (*pTableTable)[nIndex] );
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

        if( nOldIdx != nIndex )
            m_pWndPreview->NotifyChange( aTmp );
        UpdateChecks( aTmp, false );
    }

    m_pBtnRemove->Enable( bBtnEnable );
    m_pBtnRename->Enable( bBtnEnable );
}

IMPL_LINK_NOARG_TYPED(SwAutoFormatDlg, OkHdl, Button*, void)
{
    if( bSetAutoFormat )
        pShell->SetTableStyle((*pTableTable)[nIndex]);
    EndDialog( RET_OK );
}

AutoFormatPreview::AutoFormatPreview(vcl::Window* pParent, WinBits nStyle) :
        Window          ( pParent, nStyle ),
        aCurData        ( OUString() ),
        aVD             ( VclPtr<VirtualDevice>::Create(*this) ),
        bFitWidth       ( false ),
        mbRTL           ( false ),
        aStrJan         ( SW_RES( STR_JAN ) ),
        aStrFeb         ( SW_RES( STR_FEB ) ),
        aStrMar         ( SW_RES( STR_MAR ) ),
        aStrNorth       ( SW_RES( STR_NORTH ) ),
        aStrMid         ( SW_RES( STR_MID ) ),
        aStrSouth       ( SW_RES( STR_SOUTH ) ),
        aStrSum         ( SW_RES( STR_SUM ) )
{
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    m_xBreak = i18n::BreakIterator::create(xContext);
    pNumFormat = new SvNumberFormatter( xContext, LANGUAGE_SYSTEM );

    Init();
}

VCL_BUILDER_DECL_FACTORY(AutoFormatPreview)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<AutoFormatPreview>::Create(pParent, nWinStyle);
}

void AutoFormatPreview::Resize()
{
    aPrvSize = Size(GetSizePixel().Width() - 6, GetSizePixel().Height() - 30);
    nLabelColWidth = (aPrvSize.Width() - 4) / 4 - 12;
    nDataColWidth1 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 3;
    nDataColWidth2 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 4;
    nRowHeight = (aPrvSize.Height() - 4) / 5;
    NotifyChange(aCurData);
}

void AutoFormatPreview::DetectRTL(SwWrtShell* pWrtShell)
{
    if (!pWrtShell->IsCursorInTable()) // We haven't created the table yet
        mbRTL = AllSettings::GetLayoutRTL();
    else
        mbRTL = pWrtShell->IsTableRightToLeft();
}

AutoFormatPreview::~AutoFormatPreview()
{
    disposeOnce();
}

void AutoFormatPreview::dispose()
{
    delete pNumFormat;
    vcl::Window::dispose();
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
    rFont.SetWeight     ( (FontWeight)rWeightItem.GetValue() );
    rFont.SetItalic     ( (FontItalic)rPostureItem.GetValue() );
}

#define SETONALLFONTS( MethodName, Value )                  \
rFont.MethodName( Value );                                  \
rCJKFont.MethodName( Value );                               \
rCTLFont.MethodName( Value );

void AutoFormatPreview::MakeFonts( sal_uInt8 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont )
{
    const SwBoxAutoFormat& rBoxFormat = aCurData.GetBoxFormat( nIndex );

    rFont = rCJKFont = rCTLFont = GetFont();
    Size aFontSize( rFont.GetFontSize().Width(), 10 * GetDPIScaleFactor() );

    lcl_SetFontProperties( rFont, rBoxFormat.GetFont(), rBoxFormat.GetWeight(), rBoxFormat.GetPosture() );
    lcl_SetFontProperties( rCJKFont, rBoxFormat.GetCJKFont(), rBoxFormat.GetCJKWeight(), rBoxFormat.GetCJKPosture() );
    lcl_SetFontProperties( rCTLFont, rBoxFormat.GetCTLFont(), rBoxFormat.GetCTLWeight(), rBoxFormat.GetCTLPosture() );

    SETONALLFONTS( SetUnderline,        (FontLineStyle)rBoxFormat.GetUnderline().GetValue() );
    SETONALLFONTS( SetOverline,         (FontLineStyle)rBoxFormat.GetOverline().GetValue() );
    SETONALLFONTS( SetStrikeout,        (FontStrikeout)rBoxFormat.GetCrossedOut().GetValue() );
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

const SvxBoxItem& AutoFormatPreview::GetBoxItem( size_t nCol, size_t nRow ) const
{
    return aCurData.GetBoxFormat( GetFormatIndex( nCol, nRow ) ).GetBox();
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
            sal_uInt32 nKey = pNumFormat->GetIndexPuttingAndConverting(sFormat, eLng,
                                                                    eSys, nType, bNew, nCheckPos);
            Color* pDummy;
            pNumFormat->GetOutputString(nVal, nKey, cellString, &pDummy);
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
        Rectangle cellRect = maArray.GetCellRect( nCol, nRow );
        Point aPos = cellRect.TopLeft();
        long nRightX = 0;

        Size theMaxStrSize(cellRect.GetWidth() - FRAME_OFFSET,
                           cellRect.GetHeight() - FRAME_OFFSET);
        if (aCurData.IsFont())
        {
            vcl::Font aFont, aCJKFont, aCTLFont;
            MakeFonts(nFormatIndex, aFont, aCJKFont, aCTLFont);
            aScriptedText.SetFonts(&aFont, &aCJKFont, &aCTLFont);
        }
        else
            aScriptedText.SetDefaultFont();

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
                case SVX_ADJUST_LEFT:
                    aPos.X() += FRAME_OFFSET;
                    break;
                case SVX_ADJUST_RIGHT:
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

void AutoFormatPreview::DrawStrings(vcl::RenderContext& rRenderContext)
{
    for (size_t nRow = 0; nRow < 5; ++nRow)
        for (size_t nCol = 0; nCol < 5; ++nCol)
            DrawString(rRenderContext, nCol, nRow);
}

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
            rRenderContext.DrawRect(maArray.GetCellRect(nCol, nRow));
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
    DrawStrings(rRenderContext);

    // 3) border
    if (aCurData.IsFrame())
        maArray.DrawArray(rRenderContext);
}

void AutoFormatPreview::Init()
{
    SetBorderStyle( GetBorderStyle() | WindowBorderStyle::MONO );
    maArray.Initialize( 5, 5 );
    maArray.SetUseDiagDoubleClipping( false );
    CalcCellArray( false );
    CalcLineMap();
}

void AutoFormatPreview::CalcCellArray( bool _bFitWidth )
{
    maArray.SetXOffset( 2 );
    maArray.SetAllColWidths( _bFitWidth ? nDataColWidth2 : nDataColWidth1 );
    maArray.SetColWidth( 0, nLabelColWidth );
    maArray.SetColWidth( 4, nLabelColWidth );

    maArray.SetYOffset( 2 );
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

            const SvxBoxItem& rItem = GetBoxItem( nCol, nRow );
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
    Invalidate(Rectangle(Point(0,0), GetSizePixel()));
}

void AutoFormatPreview::DoPaint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    DrawModeFlags nOldDrawMode = aVD->GetDrawMode();
    if (rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
        aVD->SetDrawMode(DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient);

    Bitmap thePreview;
    Point aCenterPos;
    Size theWndSize = GetSizePixel();
    Color oldColor;
    vcl::Font aFont;

    aFont = aVD->GetFont();
    aFont.SetTransparent( true );

    aVD->SetFont(aFont);
    aVD->SetLineColor();
    const Color& rWinColor = rRenderContext.GetSettings().GetStyleSettings().GetWindowColor();
    aVD->SetBackground(Wallpaper(rWinColor));
    aVD->SetFillColor(rWinColor);
    aVD->SetOutputSizePixel(aPrvSize);

    // Draw cells on virtual device
    // and save the result
    PaintCells(*aVD.get());
    thePreview = aVD->GetBitmap(Point(0,0), aPrvSize);

    // Draw the Frame and center the preview:
    // (virtual Device for window output)
    aVD->SetOutputSizePixel(theWndSize);
    oldColor = aVD->GetLineColor();
    aVD->SetLineColor();
    aVD->DrawRect(Rectangle(Point(0,0), theWndSize));

    rRenderContext.SetLineColor(oldColor);

    aCenterPos = Point((theWndSize.Width()  - aPrvSize.Width())  / 2,
                       (theWndSize.Height() - aPrvSize.Height()) / 2);
    aVD->DrawBitmap(aCenterPos, thePreview);

    // Output in the preview window:
    rRenderContext.DrawBitmap(Point(0, 0), aVD->GetBitmap(Point(0,0), theWndSize));

    aVD->SetDrawMode(nOldDrawMode);
}

void AutoFormatPreview::Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect )
{
    DoPaint(rRenderContext, rRect);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
