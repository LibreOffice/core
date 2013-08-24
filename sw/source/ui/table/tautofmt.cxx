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

class AutoFmtPreview : public Window
{
public:
    AutoFmtPreview(Window* pParent);
    ~AutoFmtPreview();

    void NotifyChange( const SwTableAutoFmt& rNewData );

    void DetectRTL(SwWrtShell* pWrtShell);

    virtual void Resize();
protected:
    virtual void Paint( const Rectangle& rRect );

private:
    SwTableAutoFmt          aCurData;
    VirtualDevice           aVD;
    SvtScriptedTextHelper   aScriptedText;
    svx::frame::Array       maArray;            /// Implementation to draw the frame borders.
    sal_Bool                    bFitWidth;
    bool                    mbRTL;
    Size                    aPrvSize;
    long                    nLabelColWidth;
    long                    nDataColWidth1;
    long                    nDataColWidth2;
    long                    nRowHeight;
    const String            aStrJan;
    const String            aStrFeb;
    const String            aStrMar;
    const String            aStrNorth;
    const String            aStrMid;
    const String            aStrSouth;
    const String            aStrSum;
    SvNumberFormatter*      pNumFmt;

    uno::Reference< i18n::XBreakIterator >       m_xBreak;

    void    Init            ();
    void    DoPaint         ( const Rectangle& rRect );
    void    CalcCellArray   ( sal_Bool bFitWidth );
    void    CalcLineMap     ();
    void    PaintCells      ();

    sal_uInt8                GetFormatIndex( size_t nCol, size_t nRow ) const;
    const SvxBoxItem&   GetBoxItem( size_t nCol, size_t nRow ) const;

    void                DrawString( size_t nCol, size_t nRow );
    void                DrawStrings();
    void                DrawBackground();

    void    MakeFonts       ( sal_uInt8 nIndex, Font& rFont, Font& rCJKFont, Font& rCTLFont );
    String  MakeNumberString( String cellString, sal_Bool bAddDec );
};

class SwStringInputDlg : public ModalDialog
{
public:
            SwStringInputDlg(     Window* pParent,
                            const String& rTitle,
                            const String& rEditTitle,
                            const OUString& rDefault );
            ~SwStringInputDlg();

    OUString GetInputString() const;

private:
    Edit*           m_pEdInput;   // Edit obtains the focus.
};


SwStringInputDlg::SwStringInputDlg(Window* pParent, const String& rTitle,
    const String& rEditTitle, const OUString& rDefault)
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
}

// AutoFormat-Dialogue:

SwAutoFormatDlg::SwAutoFormatDlg( Window* pParent, SwWrtShell* pWrtShell,
                    sal_Bool bSetAutoFormat, const SwTableAutoFmt* pSelFmt )
    : SfxModalDialog(pParent, "AutoFormatTableDialog", "modules/swriter/ui/autoformattable.ui")
    , aStrTitle(SW_RES(STR_ADD_AUTOFORMAT_TITLE))
    , aStrLabel(SW_RES(STR_ADD_AUTOFORMAT_LABEL))
    , aStrClose(SW_RES(STR_BTN_AUTOFORMAT_CLOSE))
    , aStrDelTitle(SW_RES(STR_DEL_AUTOFORMAT_TITLE))
    , aStrDelMsg(SW_RES(STR_DEL_AUTOFORMAT_MSG))
    , aStrRenameTitle(SW_RES(STR_RENAME_AUTOFORMAT_TITLE))
    , aStrInvalidFmt(SW_RES(STR_INVALID_AUTOFORMAT_NAME))
    , pShell(pWrtShell)
    , nIndex(0)
    , nDfltStylePos(0)
    , bCoreDataChanged(sal_False)
    , bSetAutoFmt(bSetAutoFormat)
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

    pTableTbl = new SwTableAutoFmtTbl;
    pTableTbl->Load();

    Init(pSelFmt);
}

SwAutoFormatDlg::~SwAutoFormatDlg()
{
    if (bCoreDataChanged)
        pTableTbl->Save();
    delete pTableTbl;
}

void SwAutoFormatDlg::Init( const SwTableAutoFmt* pSelFmt )
{
    Link aLk( LINK( this, SwAutoFormatDlg, CheckHdl ) );
    m_pBtnBorder->SetClickHdl( aLk );
    m_pBtnFont->SetClickHdl( aLk );
    m_pBtnPattern->SetClickHdl( aLk );
    m_pBtnAlignment->SetClickHdl( aLk );
    m_pBtnNumFormat->SetClickHdl( aLk );

    m_pBtnAdd->SetClickHdl ( LINK( this, SwAutoFormatDlg, AddHdl ) );
    m_pBtnRemove->SetClickHdl ( LINK( this, SwAutoFormatDlg, RemoveHdl ) );
    m_pBtnRename->SetClickHdl ( LINK( this, SwAutoFormatDlg, RenameHdl ) );
    m_pBtnOk->SetClickHdl ( LINK( this, SwAutoFormatDlg, OkHdl ) );
    m_pLbFormat->SetSelectHdl( LINK( this, SwAutoFormatDlg, SelFmtHdl ) );

    m_pBtnAdd->Enable( bSetAutoFmt );

    nIndex = 0;
    if( !bSetAutoFmt )
    {
        // Then the list to be expanded by the entry "- none -".
        m_pLbFormat->InsertEntry( ViewShell::GetShellRes()->aStrNone );
        nDfltStylePos = 1;
        nIndex = 255;
    }

    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(pTableTbl->size());
            i < nCount; i++)
    {
        SwTableAutoFmt const& rFmt = (*pTableTbl)[ i ];
        m_pLbFormat->InsertEntry(rFmt.GetName());
        if (pSelFmt && rFmt.GetName() == pSelFmt->GetName())
            nIndex = i;
    }

    m_pLbFormat->SelectEntryPos( 255 != nIndex ? (nDfltStylePos + nIndex) : 0 );
    SelFmtHdl( 0 );
}

void SwAutoFormatDlg::UpdateChecks( const SwTableAutoFmt& rFmt, sal_Bool bEnable )
{
    m_pBtnNumFormat->Enable( bEnable );
    m_pBtnNumFormat->Check( rFmt.IsValueFormat() );

    m_pBtnBorder->Enable( bEnable );
    m_pBtnBorder->Check( rFmt.IsFrame() );

    m_pBtnFont->Enable( bEnable );
    m_pBtnFont->Check( rFmt.IsFont() );

    m_pBtnPattern->Enable( bEnable );
    m_pBtnPattern->Check( rFmt.IsBackground() );

    m_pBtnAlignment->Enable( bEnable );
    m_pBtnAlignment->Check( rFmt.IsJustify() );
}

void SwAutoFormatDlg::FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const
{
    if( 255 != nIndex )
    {
        if( rToFill )
            *rToFill = (*pTableTbl)[ nIndex ];
        else
            rToFill = new SwTableAutoFmt( (*pTableTbl)[ nIndex ] );
    }
    else
        delete rToFill, rToFill = 0;
}


// Handler:

IMPL_LINK( SwAutoFormatDlg, CheckHdl, Button *, pBtn )
{
    SwTableAutoFmt* pData  = &(*pTableTbl)[nIndex];
    sal_Bool bCheck = ((CheckBox*)pBtn)->IsChecked(), bDataChgd = sal_True;

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
        bDataChgd = sal_False;

    if( bDataChgd )
    {
        if( !bCoreDataChanged )
        {
            m_pBtnCancel->SetText( aStrClose );
            bCoreDataChanged = sal_True;
        }

        m_pWndPreview->NotifyChange( *pData );
    }
    return 0;
}

IMPL_LINK_NOARG(SwAutoFormatDlg, AddHdl)
{
    bool bOk = false, bFmtInserted = false;
    while( !bOk )
    {
        SwStringInputDlg* pDlg = new SwStringInputDlg( this,
                                                       aStrTitle,
                                                       aStrLabel,
                                                       OUString() );
        if( RET_OK == pDlg->Execute() )
        {
            const OUString aFormatName( pDlg->GetInputString() );

            if ( !aFormatName.isEmpty() )
            {
                sal_uInt16 n;
                for( n = 0; n < pTableTbl->size(); ++n )
                    if( (*pTableTbl)[n].GetName() == aFormatName )
                        break;

                if( n >= pTableTbl->size() )
                {
                    // Format with the name does not already exist, so take up.
                    SwTableAutoFmt* pNewData = new
                                        SwTableAutoFmt( aFormatName );
                    pShell->GetTableAutoFmt( *pNewData );

                    // Insert sorted!!
                    for( n = 1; n < pTableTbl->size(); ++n )
                        if( (*pTableTbl)[ n ].GetName() > aFormatName )
                            break;

                    pTableTbl->InsertAutoFmt(n, pNewData);
                    m_pLbFormat->InsertEntry( aFormatName, nDfltStylePos + n );
                    m_pLbFormat->SelectEntryPos( nDfltStylePos + n );
                    bFmtInserted = true;
                    m_pBtnAdd->Enable( sal_False );
                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = sal_True;
                    }

                    SelFmtHdl( 0 );
                    bOk = true;
                }
            }

            if( !bFmtInserted )
            {
                bOk = RET_CANCEL == MessageDialog(this, aStrInvalidFmt, VCL_MESSAGE_ERROR, VCL_BUTTONS_OK_CANCEL)
                                    .Execute();
            }
        }
        else
            bOk = true;
        delete pDlg;
    }
    return 0;
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RemoveHdl)
{
    String aMessage = aStrDelMsg ;
    aMessage.AppendAscii("\n\n");
    aMessage += m_pLbFormat->GetSelectEntry() ;
    aMessage += '\n';

    MessBox* pBox = new MessBox( this, WinBits( WB_OK_CANCEL ),
                                    aStrDelTitle, aMessage);

    if ( pBox->Execute() == RET_OK )
    {
        m_pLbFormat->RemoveEntry( nDfltStylePos + nIndex );
        m_pLbFormat->SelectEntryPos( nDfltStylePos + nIndex-1 );

        pTableTbl->EraseAutoFmt(nIndex);
        nIndex--;

        if( !nIndex )
        {
            m_pBtnRemove->Enable(sal_False);
            m_pBtnRename->Enable(sal_False);
        }

        if( !bCoreDataChanged )
        {
            m_pBtnCancel->SetText( aStrClose );
            bCoreDataChanged = sal_True;
        }
    }
    delete pBox;

    SelFmtHdl( 0 );

    return 0;
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RenameHdl)
{
    bool bOk = false;
    while( !bOk )
    {
        SwStringInputDlg* pDlg = new SwStringInputDlg( this,
                                                       aStrRenameTitle,
                                                       m_pLbFormat->GetSelectEntry(),
                                                       OUString() );
        if( pDlg->Execute() == RET_OK )
        {
            bool bFmtRenamed = false;
            const OUString aFormatName( pDlg->GetInputString() );

            if ( !aFormatName.isEmpty() )
            {
                sal_uInt16 n;
                for( n = 0; n < pTableTbl->size(); ++n )
                    if ((*pTableTbl)[n].GetName() == aFormatName)
                        break;

                if( n >= pTableTbl->size() )
                {
                    // no format with this name exists, so rename it
                    m_pLbFormat->RemoveEntry( nDfltStylePos + nIndex );
                    SwTableAutoFmt* p = pTableTbl->ReleaseAutoFmt( nIndex );

                    p->SetName( aFormatName );

                    // keep all arrays sorted!
                    for( n = 1; n < pTableTbl->size(); ++n )
                        if ((*pTableTbl)[n].GetName() > aFormatName)
                        {
                            break;
                        }

                    pTableTbl->InsertAutoFmt( n, p );
                    m_pLbFormat->InsertEntry( aFormatName, nDfltStylePos + n );
                    m_pLbFormat->SelectEntryPos( nDfltStylePos + n );

                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = sal_True;
                    }

                    SelFmtHdl( 0 );
                    bOk = true;
                    bFmtRenamed = true;
                }
            }

            if( !bFmtRenamed )
            {
                bOk = RET_CANCEL == MessageDialog(this, aStrInvalidFmt, VCL_MESSAGE_ERROR, VCL_BUTTONS_OK_CANCEL)
                                    .Execute();
            }
        }
        else
            bOk = true;
        delete pDlg;
    }
    return 0;
}

IMPL_LINK_NOARG(SwAutoFormatDlg, SelFmtHdl)
{
    sal_Bool bBtnEnable = sal_False;
    sal_uInt8 nSelPos = (sal_uInt8) m_pLbFormat->GetSelectEntryPos(), nOldIdx = nIndex;
    if( nSelPos >= nDfltStylePos )
    {
        nIndex = nSelPos - nDfltStylePos;
        m_pWndPreview->NotifyChange( (*pTableTbl)[nIndex] );
        bBtnEnable = 0 != nIndex;
        UpdateChecks( (*pTableTbl)[nIndex], sal_True );
    }
    else
    {
        nIndex = 255;

        SwTableAutoFmt aTmp( ViewShell::GetShellRes()->aStrNone );
        aTmp.SetFont( sal_False );
        aTmp.SetJustify( sal_False );
        aTmp.SetFrame( sal_False );
        aTmp.SetBackground( sal_False );
        aTmp.SetValueFormat( sal_False );
        aTmp.SetWidthHeight( sal_False );

        if( nOldIdx != nIndex )
            m_pWndPreview->NotifyChange( aTmp );
        UpdateChecks( aTmp, sal_False );
    }

    m_pBtnRemove->Enable( bBtnEnable );
    m_pBtnRename->Enable( bBtnEnable );

    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwAutoFormatDlg, OkHdl)
{
    if( bSetAutoFmt )
        pShell->SetTableAutoFmt( (*pTableTbl)[ nIndex ] );
    EndDialog( RET_OK );
    return sal_True;
}
IMPL_LINK_NOARG_INLINE_END(SwAutoFormatDlg, OkHdl)

// AutoFmtPreview

AutoFmtPreview::AutoFmtPreview(Window* pParent) :
        Window          ( pParent ),
        aCurData        ( aEmptyStr ),
        aVD             ( *this ),
        aScriptedText   ( aVD ),
        bFitWidth       ( sal_False ),
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
    pNumFmt = new SvNumberFormatter( xContext, LANGUAGE_SYSTEM );

    Init();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeAutoFmtPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new AutoFmtPreview(pParent);
}

void AutoFmtPreview::Resize()
{
    aPrvSize = Size(GetSizePixel().Width() - 6, GetSizePixel().Height() - 30);
    nLabelColWidth = (aPrvSize.Width() - 4) / 4 - 12;
    nDataColWidth1 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 3;
    nDataColWidth2 = (aPrvSize.Width() - 4 - 2 * nLabelColWidth) / 4;
    nRowHeight = (aPrvSize.Height() - 4) / 5;
    NotifyChange(aCurData);
}

void AutoFmtPreview::DetectRTL(SwWrtShell* pWrtShell)
{
    if (!pWrtShell->IsCrsrInTbl()) // We haven't created the table yet
        mbRTL = Application::GetSettings().GetLayoutRTL();
    else
        mbRTL = pWrtShell->IsTableRightToLeft();
}

AutoFmtPreview::~AutoFmtPreview()
{
    delete pNumFmt;
}

static void lcl_SetFontProperties(
        Font& rFont,
        const SvxFontItem& rFontItem,
        const SvxWeightItem& rWeightItem,
        const SvxPostureItem& rPostureItem )
{
    rFont.SetFamily     ( rFontItem.GetFamily() );
    rFont.SetName       ( rFontItem.GetFamilyName() );
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

void AutoFmtPreview::MakeFonts( sal_uInt8 nIndex, Font& rFont, Font& rCJKFont, Font& rCTLFont )
{
    const SwBoxAutoFmt& rBoxFmt = aCurData.GetBoxFmt( nIndex );

    rFont = rCJKFont = rCTLFont = GetFont();
    Size aFontSize( rFont.GetSize().Width(), 10 );

    lcl_SetFontProperties( rFont, rBoxFmt.GetFont(), rBoxFmt.GetWeight(), rBoxFmt.GetPosture() );
    lcl_SetFontProperties( rCJKFont, rBoxFmt.GetCJKFont(), rBoxFmt.GetCJKWeight(), rBoxFmt.GetCJKPosture() );
    lcl_SetFontProperties( rCTLFont, rBoxFmt.GetCTLFont(), rBoxFmt.GetCTLWeight(), rBoxFmt.GetCTLPosture() );

    SETONALLFONTS( SetUnderline,    (FontUnderline)rBoxFmt.GetUnderline().GetValue() );
    SETONALLFONTS( SetOverline,     (FontUnderline)rBoxFmt.GetOverline().GetValue() );
    SETONALLFONTS( SetStrikeout,    (FontStrikeout)rBoxFmt.GetCrossedOut().GetValue() );
    SETONALLFONTS( SetOutline,      rBoxFmt.GetContour().GetValue() );
    SETONALLFONTS( SetShadow,       rBoxFmt.GetShadowed().GetValue() );
    SETONALLFONTS( SetColor,        rBoxFmt.GetColor().GetValue() );
    SETONALLFONTS( SetSize,         aFontSize );
    SETONALLFONTS( SetTransparent,  sal_True );
}

sal_uInt8 AutoFmtPreview::GetFormatIndex( size_t nCol, size_t nRow ) const
{
    static const sal_uInt8 pnFmtMap[] =
    {
        0,  1,  2,  1,  3,
        4,  5,  6,  5,  7,
        8,  9,  10, 9,  11,
        4,  5,  6,  5,  7,
        12, 13, 14, 13, 15
    };
    return pnFmtMap[ maArray.GetCellIndex( nCol, nRow, mbRTL ) ];
}

const SvxBoxItem& AutoFmtPreview::GetBoxItem( size_t nCol, size_t nRow ) const
{
    return aCurData.GetBoxFmt( GetFormatIndex( nCol, nRow ) ).GetBox();
}

void AutoFmtPreview::DrawString( size_t nCol, size_t nRow )
{
    // Output of the cell text:
    sal_uLong   nNum;
    double  nVal;
    OUString cellString;
    sal_uInt8    nIndex = static_cast< sal_uInt8 >( maArray.GetCellIndex( nCol, nRow, mbRTL ) );

    switch( nIndex )
    {
    case  1: cellString = aStrJan;          break;
    case  2: cellString = aStrFeb;          break;
    case  3: cellString = aStrMar;          break;
    case  5: cellString = aStrNorth;        break;
    case 10: cellString = aStrMid;          break;
    case 15: cellString = aStrSouth;        break;
    case  4:
    case 20: cellString = aStrSum;          break;

    case  6:
    case  8:
    case 16:
    case 18:    nVal = nIndex;
        nNum = 5;
        goto MAKENUMSTR;
    case 17:
    case  7:    nVal = nIndex;
        nNum = 6;
        goto MAKENUMSTR;
    case 11:
    case 12:
    case 13:    nVal = nIndex;
        nNum = 12 == nIndex ? 10 : 9;
        goto MAKENUMSTR;

    case  9:    nVal = 21; nNum = 7;    goto MAKENUMSTR;
    case 14:    nVal = 36; nNum = 11;   goto MAKENUMSTR;
    case 19:    nVal = 51; nNum = 7;    goto MAKENUMSTR;
    case 21:    nVal = 33; nNum = 13;   goto MAKENUMSTR;
    case 22:    nVal = 36; nNum = 14;   goto MAKENUMSTR;
    case 23:    nVal = 39; nNum = 13;   goto MAKENUMSTR;
    case 24:    nVal = 108; nNum = 15;  goto MAKENUMSTR;
MAKENUMSTR:
        if( aCurData.IsValueFormat() )
        {
            OUString sFmt;
            LanguageType eLng, eSys;
            aCurData.GetBoxFmt( (sal_uInt8)nNum ).GetValueFormat( sFmt, eLng, eSys );

            short nType;
            bool bNew;
            sal_Int32 nCheckPos;
            sal_uInt32 nKey = pNumFmt->GetIndexPuttingAndConverting( sFmt, eLng,
                                                                     eSys, nType, bNew, nCheckPos);
            Color* pDummy;
            pNumFmt->GetOutputString( nVal, nKey, cellString, &pDummy );
        }
        else
            cellString = OUString::number((sal_Int32)nVal);
        break;

    }

    if( !cellString.isEmpty() )
    {
        Size                aStrSize;
        sal_uInt8           nFmtIndex       = GetFormatIndex( nCol, nRow );
        Rectangle           cellRect        = maArray.GetCellRect( nCol, nRow );
        Point               aPos            = cellRect.TopLeft();
        sal_uInt16          nRightX         = 0;

        Size theMaxStrSize( cellRect.GetWidth() - FRAME_OFFSET,
                            cellRect.GetHeight() - FRAME_OFFSET );
        if( aCurData.IsFont() )
        {
            Font aFont, aCJKFont, aCTLFont;
            MakeFonts( nFmtIndex, aFont, aCJKFont, aCTLFont );
            aScriptedText.SetFonts( &aFont, &aCJKFont, &aCTLFont );
        }
        else
            aScriptedText.SetDefaultFont();

        aScriptedText.SetText( cellString, m_xBreak );
        aStrSize = aScriptedText.GetTextSize();

        if( aCurData.IsFont() &&
            theMaxStrSize.Height() < aStrSize.Height() )
        {
                // If the string in this font does not
                // fit into the cell, the standard font
                // is taken again:
                aScriptedText.SetDefaultFont();
                aStrSize = aScriptedText.GetTextSize();
        }

        while( theMaxStrSize.Width() <= aStrSize.Width() &&
                cellString.getLength() > 1 )
        {
            cellString = cellString.copy(0, cellString.getLength() - 1 );
            aScriptedText.SetText( cellString, m_xBreak );
            aStrSize = aScriptedText.GetTextSize();
        }

        nRightX  = (sal_uInt16)(  cellRect.GetWidth()
                                - aStrSize.Width()
                                - FRAME_OFFSET );

        // vertical (always centering):
        aPos.Y() += (nRowHeight - (sal_uInt16)aStrSize.Height()) / 2;

        // horizontal
        if( mbRTL )
            aPos.X() += nRightX;
        else if (aCurData.IsJustify())
        {
            sal_uInt16 nHorPos = (sal_uInt16)
                    ((cellRect.GetWidth()-aStrSize.Width())/2);
            const SvxAdjustItem& rAdj = aCurData.GetBoxFmt(nFmtIndex).GetAdjust();
            switch ( rAdj.GetAdjust() )
            {
                case SVX_ADJUST_LEFT:
                    aPos.X() += FRAME_OFFSET;
                    break;
                case SVX_ADJUST_RIGHT:
                    aPos.X() += nRightX;
                    break;
                default:
                    aPos.X() += nHorPos;
                    break;
            }
        }
        else
        {
            // Standard align:
            if ( (nCol == 0) || (nIndex == 4) )
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

        aScriptedText.DrawText( aPos );
    }
}

#undef FRAME_OFFSET

void AutoFmtPreview::DrawStrings()
{
    for( size_t nRow = 0; nRow < 5; ++nRow )
        for( size_t nCol = 0; nCol < 5; ++nCol )
            DrawString( nCol, nRow );
}

void AutoFmtPreview::DrawBackground()
{
    for( size_t nRow = 0; nRow < 5; ++nRow )
    {
        for( size_t nCol = 0; nCol < 5; ++nCol )
        {
            SvxBrushItem aBrushItem( aCurData.GetBoxFmt( GetFormatIndex( nCol, nRow ) ).GetBackground() );

            aVD.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            aVD.SetLineColor();
            aVD.SetFillColor( aBrushItem.GetColor() );
            aVD.DrawRect( maArray.GetCellRect( nCol, nRow ) );
            aVD.Pop();
        }
    }
}

void AutoFmtPreview::PaintCells()
{
    // 1) background
    if ( aCurData.IsBackground() )
        DrawBackground();

    // 2) values
    DrawStrings();

    // 3) border
    if ( aCurData.IsFrame() )
        maArray.DrawArray( aVD );
}

void AutoFmtPreview::Init()
{
    SetBorderStyle( GetBorderStyle() | WINDOW_BORDER_MONO );
    maArray.Initialize( 5, 5 );
    maArray.SetUseDiagDoubleClipping( false );
    CalcCellArray( sal_False );
    CalcLineMap();
}

void AutoFmtPreview::CalcCellArray( sal_Bool _bFitWidth )
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

void AutoFmtPreview::CalcLineMap()
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

void AutoFmtPreview::NotifyChange( const SwTableAutoFmt& rNewData )
{
    aCurData  = rNewData;
    bFitWidth = aCurData.IsJustify();//sal_True;  //???
    CalcCellArray( bFitWidth );
    CalcLineMap();
    DoPaint( Rectangle( Point(0,0), GetSizePixel() ) );
}

void AutoFmtPreview::DoPaint( const Rectangle& /*rRect*/ )
{
    sal_uInt32 nOldDrawMode = aVD.GetDrawMode();
    if( GetSettings().GetStyleSettings().GetHighContrastMode() )
        aVD.SetDrawMode( DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );

    Bitmap  thePreview;
    Point   aCenterPos;
    Size    theWndSize = GetSizePixel();
    Color   oldColor;
    Font    aFont;

    aFont = aVD.GetFont();
    aFont.SetTransparent( sal_True );

    aVD.SetFont          ( aFont );
    aVD.SetLineColor     ();
    const Color& rWinColor = GetSettings().GetStyleSettings().GetWindowColor();
    aVD.SetBackground    ( Wallpaper(rWinColor) );
    aVD.SetFillColor     ( rWinColor );
    aVD.SetOutputSizePixel  ( aPrvSize );

    // Draw cells on virtual device
    // and save the result
    PaintCells();
    thePreview = aVD.GetBitmap( Point(0,0), aPrvSize );

    // Draw the Frame and center the preview:
    // (virtual Device for window output)
    aVD.SetOutputSizePixel( theWndSize );
    oldColor = aVD.GetLineColor();
    aVD.SetLineColor();
    aVD.DrawRect( Rectangle( Point(0,0), theWndSize ) );
    SetLineColor( oldColor );
    aCenterPos  = Point( (theWndSize.Width()  - aPrvSize.Width() ) / 2,
                         (theWndSize.Height() - aPrvSize.Height()) / 2 );
    aVD.DrawBitmap( aCenterPos, thePreview );

    // Output in the preview window:
    DrawBitmap( Point(0,0), aVD.GetBitmap( Point(0,0), theWndSize ) );

    aVD.SetDrawMode( nOldDrawMode );
}

void AutoFmtPreview::Paint( const Rectangle& rRect )
{
    DoPaint( rRect );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
