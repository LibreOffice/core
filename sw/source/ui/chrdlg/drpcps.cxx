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

#include <hintids.hxx>

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"

#include <vcl/metric.hxx>

#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/svxfont.hxx>
#include <vcl/print.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/scripttypeitem.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>

#include "charatr.hxx"
#include "viewopt.hxx"
#include "drpcps.hxx"
#include "paratr.hxx"
#include "uitool.hxx"
#include "charfmt.hxx"

#include "chrdlg.hrc"


using namespace css;
using namespace css::uno;
using namespace css::lang;

// Globals ******************************************************************

static sal_uInt16 aPageRg[] = {
    RES_PARATR_DROP, RES_PARATR_DROP,
    0
};

class SwDropCapsPict : public Control
{
    SwDropCapsPage* mpPage;
    String          maText;
    String          maScriptText;
    Color           maBackColor;
    Color           maTextLineColor;
    sal_uInt8       mnLines;
    long            mnTotLineH;
    long            mnLineH;
    long            mnTextH;
    sal_uInt16      mnDistance;
    Printer*        mpPrinter;
    bool            mbDelPrinter;
    /// The _ScriptInfo structure holds information on where we change from one
    /// script to another.
    struct _ScriptInfo
    {
        sal_uLong  textWidth;   ///< Physical width of this segment.
        sal_uInt16 scriptType;  ///< Script type (e.g. Latin, Asian, Complex)
        xub_StrLen changePos;   ///< Character position where the script changes.
        _ScriptInfo(sal_uLong txtWidth, sal_uInt16 scrptType, xub_StrLen position)
            : textWidth(txtWidth), scriptType(scrptType), changePos(position) {}
        bool operator<(_ScriptInfo other) { return changePos < other.changePos; }
    };
    std::vector<_ScriptInfo> maScriptChanges;
    SvxFont         maFont;
    SvxFont         maCJKFont;
    SvxFont         maCTLFont;
    Size            maTextSize;
    Reference< css::i18n::XBreakIterator >   xBreak;

    virtual void    Paint(const Rectangle &rRect);
    void            CheckScript( void );
    Size            CalcTextSize( void );
    inline void     InitPrinter( void );
    void            _InitPrinter( void );
    void            GetFontSettings( const SwDropCapsPage& _rPage, Font& _rFont, sal_uInt16 _nWhich );
    void            GetFirstScriptSegment(xub_StrLen &start, xub_StrLen &end, sal_uInt16 &scriptType);
    bool            GetNextScriptSegment(size_t &nIdx, xub_StrLen &start, xub_StrLen &end, sal_uInt16 &scriptType);

public:

    SwDropCapsPict(Window *pParent, WinBits nBits)
        : Control(pParent, nBits)
        , mpPage(NULL)
        , mnTotLineH(0)
        , mnLineH(0)
        , mnTextH(0)
        , mpPrinter( NULL )
        , mbDelPrinter( false )
    {}

    void SetDropCapsPage(SwDropCapsPage* pPage) { mpPage = pPage; }

    ~SwDropCapsPict();

    void UpdatePaintSettings( void );       // also invalidates control!

    virtual void Resize();
    virtual Size GetOptimalSize() const;

    void SetText( const OUString& rT );
    void SetLines( sal_uInt8 nL );
    void SetDistance( sal_uInt16 nD );
    void SetValues( const String& rText, sal_uInt8 nLines, sal_uInt16 nDistance );

    void        DrawPrev( const Point& rPt );
};

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwDropCapsPict(Window *pParent, VclBuilder::stringmap &)
{
    return new SwDropCapsPict(pParent, WB_BORDER);
}

void SwDropCapsPict::SetText( const OUString& rT )
{
    maText = rT;
    UpdatePaintSettings();
}

Size SwDropCapsPict::GetOptimalSize() const
{
    return getParagraphPreviewOptimalSize(this);
}

void SwDropCapsPict::Resize()
{
    Control::Resize();
    UpdatePaintSettings();
}

void SwDropCapsPict::SetLines( sal_uInt8 nL )
{
    mnLines = nL;
    UpdatePaintSettings();
}

void SwDropCapsPict::SetDistance( sal_uInt16 nD )
{
    mnDistance = nD;
    UpdatePaintSettings();
}

void SwDropCapsPict::SetValues( const String& rText, sal_uInt8 nLines, sal_uInt16 nDistance )
{
    maText = rText;
    mnLines = nLines;
    mnDistance = nDistance;

    UpdatePaintSettings();
}

void SwDropCapsPict::InitPrinter( void )
{
    if( !mpPrinter )
        _InitPrinter();
}

/****************************************************************************
 Create Default-String from character-count (A, AB, ABC, ...)
****************************************************************************/


String GetDefaultString(sal_uInt16 nChars)
{
    String aStr;
    for (sal_uInt16 i = 0; i < nChars; i++)
        aStr += OUString((char) (i + 65));
    return aStr;
}

static void calcFontHeightAnyAscent( OutputDevice* _pWin, Font& _rFont, long& _nHeight, long& _nAscent )
{
    if ( !_nHeight )
    {
        _pWin->SetFont( _rFont );
        FontMetric aMetric( _pWin->GetFontMetric() );
        _nHeight = aMetric.GetLineHeight();
        _nAscent = aMetric.GetAscent();
    }
}

SwDropCapsPict::~SwDropCapsPict()
{
     if( mbDelPrinter )
         delete mpPrinter;
}

/// Get the details of the first script change.
/// @param[out] start      The character position of the start of the segment.
/// @param[out] end        The character position of the end of the segment.
/// @param[out] scriptType The script type (Latin, Asian, Complex etc.)
void SwDropCapsPict::GetFirstScriptSegment(xub_StrLen &start, xub_StrLen &end, sal_uInt16 &scriptType)
{
    start = 0;
    if( maScriptChanges.empty() )
    {
        end = maText.Len();
        scriptType = css::i18n::ScriptType::LATIN;
    }
    else
    {
        end = maScriptChanges[ 0 ].changePos;
        scriptType = maScriptChanges[ 0 ].scriptType;
    }
}

/// Get the details of the first script change.
/// @param[in,out] nIdx       Index of the current script change.
/// @param[out]    start      The character position of the start of the segment.
/// @param[in,out] end        The character position of the end of the segment.
/// @param[out]    scriptType The script type (Latin, Asian, Complex etc.)
/// @returns True if there was a next segment, false if not.
bool SwDropCapsPict::GetNextScriptSegment(size_t &nIdx, xub_StrLen &start, xub_StrLen &end, sal_uInt16 &scriptType)
{
    if (maScriptChanges.empty() || nIdx >= maScriptChanges.size() - 1 || end >= maText.Len())
        return false;
    start = maScriptChanges[nIdx++].changePos;
    end = maScriptChanges[ nIdx ].changePos;
    scriptType = maScriptChanges[ nIdx ].scriptType;
    return true;
}

#define LINES  10
#define BORDER  2

void SwDropCapsPict::GetFontSettings( const SwDropCapsPage& _rPage, Font& _rFont, sal_uInt16 _nWhich )
{
    SfxItemSet aSet( _rPage.rSh.GetAttrPool(), _nWhich, _nWhich);
    _rPage.rSh.GetCurAttr(aSet);
    SvxFontItem aFmtFont((SvxFontItem &) aSet.Get(_nWhich));

    _rFont.SetFamily (aFmtFont.GetFamily());
    _rFont.SetName   (aFmtFont.GetFamilyName());
    _rFont.SetPitch  (aFmtFont.GetPitch());
    _rFont.SetCharSet(aFmtFont.GetCharSet());
}

void SwDropCapsPict::UpdatePaintSettings( void )
{
    maBackColor = GetSettings().GetStyleSettings().GetWindowColor();
    maTextLineColor = Color( COL_LIGHTGRAY );

    // gray lines
    mnTotLineH = (GetOutputSizePixel().Height() - 2 * BORDER) / LINES;
    mnLineH = mnTotLineH - 2;

    Font aFont;
    if (mpPage)
    {
        if (!mpPage->m_pTemplateBox->GetSelectEntryPos())
        {
            // query the Font at paragraph's beginning
            mpPage->rSh.SttCrsrMove();
            mpPage->rSh.Push();
            mpPage->rSh.ClearMark();
            SwWhichPara pSwuifnParaCurr = GetfnParaCurr();
            SwPosPara pSwuifnParaStart = GetfnParaStart();
            mpPage->rSh.MovePara(pSwuifnParaCurr,pSwuifnParaStart);
            // normal
            GetFontSettings( *mpPage, aFont, RES_CHRATR_FONT );

            // CJK
            GetFontSettings( *mpPage, maCJKFont, RES_CHRATR_CJK_FONT );

            // CTL
            GetFontSettings( *mpPage, maCTLFont, RES_CHRATR_CTL_FONT );

            mpPage->rSh.Pop(sal_False);
            mpPage->rSh.EndCrsrMove();
        }
        else
        {
            // query Font at character template
            SwCharFmt *pFmt = mpPage->rSh.GetCharStyle(
                                    mpPage->m_pTemplateBox->GetSelectEntry(),
                                    SwWrtShell::GETSTYLE_CREATEANY );
            OSL_ENSURE(pFmt, "character style doesn't exist!");
            const SvxFontItem &rFmtFont = pFmt->GetFont();

            aFont.SetFamily (rFmtFont.GetFamily());
            aFont.SetName   (rFmtFont.GetFamilyName());
            aFont.SetPitch  (rFmtFont.GetPitch());
            aFont.SetCharSet(rFmtFont.GetCharSet());
        }
    }

    mnTextH = mnLines * mnTotLineH;
    aFont.SetSize(Size(0, mnTextH));
    maCJKFont.SetSize(Size(0, mnTextH));
    maCTLFont.SetSize(Size(0, mnTextH));

    aFont.SetTransparent(sal_True);
    maCJKFont.SetTransparent(sal_True);
    maCTLFont.SetTransparent(sal_True);

    aFont.SetColor( SwViewOption::GetFontColor() );
    maCJKFont.SetColor( SwViewOption::GetFontColor() );
    maCTLFont.SetColor( SwViewOption::GetFontColor() );

    aFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    maCJKFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    maCTLFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());

    maCJKFont.SetSize(Size(0, maCJKFont.GetSize().Height()));
    maCTLFont.SetSize(Size(0, maCTLFont.GetSize().Height()));

    SetFont(aFont);
    aFont.SetSize(Size(0, aFont.GetSize().Height()));
    SetFont(aFont);
    maFont = aFont;

    CheckScript();

    maTextSize = CalcTextSize();

    Invalidate();
}

/****************************************************************************
Pict: Paint-Overload
****************************************************************************/

void  SwDropCapsPict::Paint(const Rectangle &/*rRect*/)
{
    if (!IsVisible())
        return;

    SetMapMode(MapMode(MAP_PIXEL));
    SetLineColor();

    SetFillColor( maBackColor );

    Size aOutputSizePixel( GetOutputSizePixel() );

    DrawRect(Rectangle(Point(0, 0), aOutputSizePixel ));
    SetClipRegion(Region(Rectangle(
        Point(BORDER, BORDER),
        Size (aOutputSizePixel.Width () - 2 * BORDER,
              aOutputSizePixel.Height() - 2 * BORDER))));

    OSL_ENSURE(mnLineH > 0, "We cannot make it that small");
    long nY0 = (aOutputSizePixel.Height() - (LINES * mnTotLineH)) / 2;
    SetFillColor( maTextLineColor );
    for (sal_uInt16 i = 0; i < LINES; ++i)
        DrawRect(Rectangle(Point(BORDER, nY0 + i * mnTotLineH), Size(aOutputSizePixel.Width() - 2 * BORDER, mnLineH)));

    // Text background with gap (240 twips ~ 1 line height)
    sal_uLong lDistance = mnDistance;
    sal_uInt16 nDistW = (sal_uInt16) (sal_uLong) (((lDistance * 100) / 240) * mnTotLineH) / 100;
    SetFillColor( maBackColor );
    if (mpPage && mpPage->m_pDropCapsBox->IsChecked())
    {
        Size    aTextSize( maTextSize );
        aTextSize.Width() += nDistW;
        DrawRect( Rectangle( Point( BORDER, nY0 ), aTextSize ) );

        // draw Text
        DrawPrev( Point( BORDER, nY0 ) );
    }

    SetClipRegion();
}

void SwDropCapsPict::DrawPrev( const Point& rPt )
{
    Point aPt(rPt);
    InitPrinter();

    Font        aOldFont = mpPrinter->GetFont();
    sal_uInt16      nScript;
    size_t      nIdx = 0;
    xub_StrLen  nStart;
    xub_StrLen  nEnd;
    GetFirstScriptSegment(nStart, nEnd, nScript);
    do
    {
        SvxFont&    rFnt = (nScript==css::i18n::ScriptType::ASIAN) ? maCJKFont : ((nScript==css::i18n::ScriptType::COMPLEX) ? maCTLFont : maFont);
        mpPrinter->SetFont( rFnt );

        rFnt.DrawPrev( this, mpPrinter, aPt, maText, nStart, nEnd - nStart );

        aPt.X() += maScriptChanges[ nIdx ].textWidth;
        if ( !GetNextScriptSegment(nIdx, nStart, nEnd, nScript) )
            break;
    }
    while( true );
    mpPrinter->SetFont( aOldFont );
}

void SwDropCapsPict::CheckScript( void )
{
    if( maScriptText == maText )
        return;

    maScriptText = maText;
    maScriptChanges.clear();
    if( !xBreak.is() )
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        xBreak = css::i18n::BreakIterator::create(xContext);
    }
    sal_uInt16 nScript = xBreak->getScriptType( maText, 0 );
    sal_uInt16 nChg = 0;
    if( css::i18n::ScriptType::WEAK == nScript )
    {
        nChg = (xub_StrLen)xBreak->endOfScript( maText, nChg, nScript );
        if( nChg < maText.Len() )
            nScript = xBreak->getScriptType( maText, nChg );
        else
            nScript = css::i18n::ScriptType::LATIN;
    }

    do
    {
        nChg = (xub_StrLen)xBreak->endOfScript( maText, nChg, nScript );
        maScriptChanges.push_back( _ScriptInfo(0, nScript, nChg) );

        if( nChg < maText.Len() )
            nScript = xBreak->getScriptType( maText, nChg );
        else
            break;
    } while( true );
}

Size SwDropCapsPict::CalcTextSize( void )
{
    InitPrinter();

    sal_uInt16      nScript;
    size_t      nIdx = 0;
    xub_StrLen  nStart;
    xub_StrLen  nEnd;
    GetFirstScriptSegment(nStart, nEnd, nScript);
    long        nTxtWidth = 0;
    long        nCJKHeight = 0;
    long        nCTLHeight = 0;
    long        nHeight = 0;
    long        nAscent = 0;
    long        nCJKAscent = 0;
    long        nCTLAscent = 0;
    do
    {
        SvxFont&    rFnt = ( nScript == css::i18n::ScriptType::ASIAN )? maCJKFont :
                                ( ( nScript == css::i18n::ScriptType::COMPLEX )? maCTLFont : maFont );
        sal_uLong       nWidth = rFnt.GetTxtSize( mpPrinter, maText, nStart, nEnd-nStart ).Width();

        if( nIdx < maScriptChanges.size() )
            maScriptChanges[ nIdx ].textWidth = nWidth;
        nTxtWidth += nWidth;
        switch(nScript)
        {
            case css::i18n::ScriptType::ASIAN:
                calcFontHeightAnyAscent( this, maCJKFont, nCJKHeight, nCJKAscent );
                break;
            case css::i18n::ScriptType::COMPLEX:
                calcFontHeightAnyAscent( this, maCTLFont, nCTLHeight, nCTLAscent );
                break;
            default:
                calcFontHeightAnyAscent( this, maFont, nHeight, nAscent );
        }

        if ( !GetNextScriptSegment(nIdx, nStart, nEnd, nScript) )
            break;
    }
    while( true );
    nHeight -= nAscent;
    nCJKHeight -= nCJKAscent;
    nCTLHeight -= nCTLAscent;
    if( nHeight < nCJKHeight )
        nHeight = nCJKHeight;
    if( nAscent < nCJKAscent )
        nAscent = nCJKAscent;
    if( nHeight < nCTLHeight )
        nHeight = nCTLHeight;
    if( nAscent < nCTLAscent )
        nAscent = nCTLAscent;
    nHeight += nAscent;

    Size aTxtSize( nTxtWidth, nHeight );
    return aTxtSize;
}

void SwDropCapsPict::_InitPrinter()
{
    SfxViewShell*   pSh = SfxViewShell::Current();

    if ( pSh )
        mpPrinter = pSh->GetPrinter();

    if ( !mpPrinter )
    {
        mpPrinter = new Printer;
        mbDelPrinter = true;
    }
}

SwDropCapsDlg::SwDropCapsDlg(Window *pParent, const SfxItemSet &rSet )
    : SfxSingleTabDialog(pParent, rSet)
{
    SwDropCapsPage* pNewPage = (SwDropCapsPage*) SwDropCapsPage::Create(get_content_area(), rSet);
    pNewPage->SetFormat(false);
    setTabPage(pNewPage);
}

SwDropCapsPage::SwDropCapsPage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "DropCapPage","modules/swriter/ui/dropcapspage.ui", rSet)
    , bModified(false)
    , bFormat(true)
    , rSh(::GetActiveView()->GetWrtShell())
{
    get(m_pDropCapsBox,"checkCB_SWITCH");
    get(m_pWholeWordCB,"checkCB_WORD");
    get(m_pDropCapsField,"spinFLD_DROPCAPS");
    get(m_pLinesField,"spinFLD_LINES");
    get(m_pDistanceField,"spinFLD_DISTANCE");
    get(m_pSwitchText,"labelFT_DROPCAPS");
    get(m_pLinesText,"labelTXT_LINES");
    get(m_pDistanceText,"labelTXT_DISTANCE");
    get(m_pTemplateText,"labelTXT_TEMPLATE");
    get(m_pTextText,"labelTXT_TEXT");
    get(m_pTextEdit,"entryEDT_TEXT");
    get(m_pTemplateBox,"comboBOX_TEMPLATE");
    get(m_pPict,"drawingareaWN_EXAMPLE");

    m_pPict->SetDropCapsPage(this);

    SetExchangeSupport();

    sal_uInt16 nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? sal_True : sal_False;

    // In the template dialog the text is not influenceable
    m_pTextText->Enable( !bFormat );
    m_pTextEdit->Enable( !bFormat );

    // Metrics
    SetMetric( *m_pDistanceField, GetDfltMetric(bHtmlMode) );

    m_pPict->SetBorderStyle( WINDOW_BORDER_MONO );

    // Install handler
    Link aLk = LINK(this, SwDropCapsPage, ModifyHdl);
    m_pDropCapsField->SetModifyHdl( aLk );
    m_pLinesField->SetModifyHdl( aLk );
    m_pDistanceField->SetModifyHdl( aLk );
    m_pTextEdit->SetModifyHdl( aLk );
    m_pDropCapsBox->SetClickHdl (LINK(this, SwDropCapsPage, ClickHdl ));
    m_pTemplateBox->SetSelectHdl(LINK(this, SwDropCapsPage, SelectHdl));
    m_pWholeWordCB->SetClickHdl (LINK(this, SwDropCapsPage, WholeWordHdl ));

    setPreviewsToSamePlace(pParent, this);
}

SwDropCapsPage::~SwDropCapsPage()
{
}

int  SwDropCapsPage::DeactivatePage(SfxItemSet * _pSet)
{
    if ( _pSet )
        FillSet( *_pSet );

    return LEAVE_PAGE;
}

/****************************************************************************
Page: Factory
****************************************************************************/

SfxTabPage*  SwDropCapsPage::Create(Window *pParent,
    const SfxItemSet &rSet)
{
    return new SwDropCapsPage(pParent, rSet);
}

/****************************************************************************
Page: FillItemSet-Overload
****************************************************************************/

sal_Bool  SwDropCapsPage::FillItemSet(SfxItemSet &rSet)
{
    if(bModified)
        FillSet(rSet);
    return bModified;
}

/****************************************************************************
Page: Reset-Overload
****************************************************************************/

void  SwDropCapsPage::Reset(const SfxItemSet &rSet)
{
    // Characters, lines, gap and text
    SwFmtDrop aFmtDrop((SwFmtDrop &) rSet.Get(RES_PARATR_DROP));
    if (aFmtDrop.GetLines() > 1)
    {
        m_pDropCapsField->SetValue(aFmtDrop.GetChars());
        m_pLinesField->SetValue(aFmtDrop.GetLines());
        m_pDistanceField->SetValue(m_pDistanceField->Normalize(aFmtDrop.GetDistance()), FUNIT_TWIP);
        m_pWholeWordCB->Check(aFmtDrop.GetWholeWord());
    }
    else
    {
        m_pDropCapsField->SetValue(1);
        m_pLinesField->SetValue(3);
        m_pDistanceField->SetValue(0);
    }

    ::FillCharStyleListBox(*m_pTemplateBox, rSh.GetView().GetDocShell(), true);

    m_pTemplateBox->InsertEntry(SW_RESSTR(SW_STR_NONE), 0);

    // Reset format
    m_pTemplateBox->SelectEntryPos(0);
    if (aFmtDrop.GetCharFmt())
        m_pTemplateBox->SelectEntry(aFmtDrop.GetCharFmt()->GetName());

    // Enable controls
    m_pDropCapsBox->Check(aFmtDrop.GetLines() > 1);
    const sal_uInt16 nVal = sal_uInt16(m_pDropCapsField->GetValue());
    if (bFormat)
        m_pTextEdit->SetText(GetDefaultString(nVal));
    else
    {
        m_pTextEdit->SetText(rSh.GetDropTxt(nVal));
        m_pTextEdit->Enable();
        m_pTextText->Enable();
    }

    // Preview
    m_pPict->SetValues( m_pTextEdit->GetText(),
                        sal_uInt8( m_pLinesField->GetValue() ),
                        sal_uInt16( m_pDistanceField->Denormalize( m_pDistanceField->GetValue( FUNIT_TWIP ) ) ) );

    ClickHdl(m_pDropCapsBox);
    bModified = sal_False;
}

/****************************************************************************
Page: CheckBox's Click-Handler
****************************************************************************/


IMPL_LINK_NOARG(SwDropCapsPage, ClickHdl)
{
    sal_Bool bChecked = m_pDropCapsBox->IsChecked();

    m_pWholeWordCB->Enable( bChecked && !bHtmlMode );

    m_pSwitchText->Enable( bChecked && !m_pWholeWordCB->IsChecked() );
    m_pDropCapsField->Enable( bChecked && !m_pWholeWordCB->IsChecked() );
    m_pLinesText->Enable( bChecked );
    m_pLinesField->Enable( bChecked );
    m_pDistanceText->Enable( bChecked );
    m_pDistanceField->Enable( bChecked );
    m_pTemplateText->Enable( bChecked );
    m_pTemplateBox->Enable( bChecked );
    m_pTextEdit->Enable( bChecked && !bFormat );
    m_pTextText->Enable( bChecked && !bFormat );

    if ( bChecked )
    {
        ModifyHdl(m_pDropCapsField);
        m_pDropCapsField->GrabFocus();
    }
    else
        m_pPict->SetText("");

    bModified = sal_True;

    return 0;
}

/****************************************************************************
Page: CheckBox's Click-Handler
****************************************************************************/


IMPL_LINK_NOARG(SwDropCapsPage, WholeWordHdl)
{
    m_pDropCapsField->Enable( !m_pWholeWordCB->IsChecked() );
    m_pSwitchText->Enable(!m_pWholeWordCB->IsChecked());

    ModifyHdl(m_pDropCapsField);

    bModified = sal_True;

    return 0;
}

/****************************************************************************
Page: SpinFields' Modify-Handler
****************************************************************************/


IMPL_LINK( SwDropCapsPage, ModifyHdl, Edit *, pEdit )
{
    String sPreview;

    // set text if applicable
    if (pEdit == m_pDropCapsField)
    {
        sal_uInt16 nVal;
        bool bSetText = false;

        if (!m_pWholeWordCB->IsChecked())
            nVal = (sal_uInt16)m_pDropCapsField->GetValue();
        else
            nVal = 0;

        if (bFormat || !rSh.GetDropTxt(1).Len())
            sPreview = GetDefaultString(nVal);
        else
        {
            bSetText = true;
            sPreview = rSh.GetDropTxt(nVal);
        }

        String sEdit(m_pTextEdit->GetText());

        if (sEdit.Len() && sPreview.CompareTo(sEdit, sEdit.Len()) != COMPARE_EQUAL)
        {
            sPreview = sEdit.Copy(0, sPreview.Len());
            bSetText = false;
        }

        if (bSetText)
            m_pTextEdit->SetText(sPreview);
    }
    else if (pEdit == m_pTextEdit)   // set quantity if applicable
    {
        sal_Int32 nTmp = m_pTextEdit->GetText().getLength();
        m_pDropCapsField->SetValue(std::max((sal_uInt16)1, (sal_uInt16)nTmp));

        sPreview = m_pTextEdit->GetText().copy(0, nTmp);
    }

    // adjust image
    if (pEdit == m_pDropCapsField || pEdit == m_pTextEdit)
        m_pPict->SetText (sPreview);
    else if (pEdit == m_pLinesField)
        m_pPict->SetLines((sal_uInt8)m_pLinesField->GetValue());
    else
        m_pPict->SetDistance((sal_uInt16)m_pDistanceField->Denormalize(m_pDistanceField->GetValue(FUNIT_TWIP)));

    bModified = sal_True;

    return 0;
}

/****************************************************************************
Page: Template-Box' Select-Handler.
*****************************************************************************/


IMPL_LINK_NOARG_INLINE_START(SwDropCapsPage, SelectHdl)
{
    m_pPict->UpdatePaintSettings();
    bModified = sal_True;
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwDropCapsPage, SelectHdl)

sal_uInt16*  SwDropCapsPage::GetRanges()
{
    return aPageRg;
}

void SwDropCapsPage::FillSet( SfxItemSet &rSet )
{
    if(bModified)
    {
        SwFmtDrop aFmt;

        sal_Bool bOn = m_pDropCapsBox->IsChecked();
        if(bOn)
        {
            // quantity, lines, gap
            aFmt.GetChars()     = (sal_uInt8) m_pDropCapsField->GetValue();
            aFmt.GetLines()     = (sal_uInt8) m_pLinesField->GetValue();
            aFmt.GetDistance()  = (sal_uInt16) m_pDistanceField->Denormalize(m_pDistanceField->GetValue(FUNIT_TWIP));
            aFmt.GetWholeWord() = m_pWholeWordCB->IsChecked();

            // template
            if (m_pTemplateBox->GetSelectEntryPos())
                aFmt.SetCharFmt(rSh.GetCharStyle(m_pTemplateBox->GetSelectEntry()));
        }
        else
        {
            aFmt.GetChars()    = 1;
            aFmt.GetLines()    = 1;
            aFmt.GetDistance() = 0;
        }

        // set attributes
        const SfxPoolItem* pOldItem;
        if(0 == (pOldItem = GetOldItem( rSet, FN_FORMAT_DROPCAPS )) ||
                    aFmt != *pOldItem )
            rSet.Put(aFmt);

        // hard text formatting
        // Bug 24974: in designer/template catalog this doesn't make sense!!
        if( !bFormat && m_pDropCapsBox->IsChecked() )
        {
            String sText(m_pTextEdit->GetText());

            if (!m_pWholeWordCB->IsChecked())
                sText.Erase( static_cast< xub_StrLen >(m_pDropCapsField->GetValue()));

            SfxStringItem aStr(FN_PARAM_1, sText);
            rSet.Put( aStr );
        }
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
