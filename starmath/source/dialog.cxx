/*************************************************************************
 *
 *  $RCSfile: dialog.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define SMDLL 1

#ifndef _TOOLS_RCID_H
#include "tools/rcid.h"
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SV_SNDSTYLE_HXX //autogen
#include <vcl/sndstyle.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_HRC //autogen
#include <sfx2/sfx.hrc>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif


#ifndef CONFIG_HXX
#include "config.hxx"
#endif
#ifndef DIALOG_HXX
#include "dialog.hxx"
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif
#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
#ifndef SYMBOL_HXX
#include "symbol.hxx"
#endif
#ifndef VIEW_HXX
#include "view.hxx"
#endif
#ifndef DOCUMENT_HXX
#include "document.hxx"
#endif

////////////////////////////////////////
//
// Da der FontStyle besser über die Attribute gesetzt/abgefragt wird als über
// den StyleName bauen wir uns hier unsere eigene Übersetzung
// Attribute <-> StyleName
// Die Bits des Index stellen die Attribute dar:
// Bit 0 : italic
// Bit 1 : bold
//

static XubString __READONLY_DATA  aStyleName[4] =
{
    C2S("normal"),  C2S("italic"),
    C2S("bold"),    C2S("bold italic")
};

USHORT aStyleNameCount = sizeof(aStyleName) / sizeof(aStyleName[0]);


const XubString GetFontStyleName(const Font &rFont)
{
    USHORT  nIndex =  2 * (rFont.GetWeight() == WEIGHT_BOLD)
                    + 1 * (rFont.GetItalic() == ITALIC_NORMAL);
    return aStyleName[nIndex];
}


void SetFontStyle(const XubString &rStyleName, Font &rFont)
{
    // finden des Index passend zum StyleName fuer den leeren StyleName wird
    // 0 (nicht bold nicht italic) angenommen.
    USHORT  nIndex = 0;
    if (rStyleName.Len())
    {
        USHORT i;
        for (i = 0;  i < aStyleNameCount;  i++)
            if (rStyleName.CompareTo(aStyleName[i]) == COMPARE_EQUAL)
                break;
        DBG_ASSERT(i < aStyleNameCount, "Sm : StyleName unbekannt");
        nIndex = i;
    }

    rFont.SetItalic((nIndex & 0x1) ? ITALIC_NORMAL : ITALIC_NONE);
    rFont.SetWeight((nIndex & 0x2) ? WEIGHT_BOLD : WEIGHT_NORMAL);
}


/**************************************************************************/

SmAboutDialog::SmAboutDialog(Window *pParent, BOOL bFreeRes) :
    ModalDialog  (pParent, SmResId(RID_DEFAULTABOUT)),
    aFixedText1  (this, ResId(1)),
    aFixedText2  (this, ResId(2)),
    aFixedText3  (this, ResId(3)),
    aFixedText4  (this, ResId(4)),
    aFixedText5  (this, ResId(5)),
    aFixedText6  (this, ResId(6)),
    aReleaseText (this, ResId(7)),
    aFixedBitmap1(this, ResId(1)),
    aOKButton1   (this, ResId(1))
{
    if (bFreeRes)
        FreeResource();

#ifndef PRODUCT
    aReleaseText.Show();
#else
    aReleaseText.Hide();
#endif

}

/**************************************************************************/


IMPL_LINK_INLINE_START( SmPrintOptionsTabPage, SizeButtonClickHdl, Button *, pButton )
{
    aZoom.Enable(aSizeZoomed.IsChecked());
    return 0;
}
IMPL_LINK_INLINE_END( SmPrintOptionsTabPage, SizeButtonClickHdl, Button *, pButton )


SmPrintOptionsTabPage::SmPrintOptionsTabPage(Window *pParent, const SfxItemSet &rOptions)
    : SfxTabPage(pParent, SmResId(RID_PRINTOPTIONPAGE), rOptions),
    aGroupBox1      (this, ResId(1)),
    aTitle          (this, ResId(1)),
    aText           (this, ResId(2)),
    aFrame          (this, ResId(3)),
    aGroupBox3      (this, ResId(3)),
    aNoRightSpaces  (this, ResId(4)),
    aGroupBox2      (this, ResId(2)),
    aSizeNormal     (this, ResId(1)),
    aSizeScaled     (this, ResId(2)),
    aSizeZoomed     (this, ResId(3)),
    aZoom           (this, ResId(1))
{
    FreeResource();

    aSizeNormal.SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    aSizeScaled.SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    aSizeZoomed.SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));

    Reset(rOptions);
}


BOOL SmPrintOptionsTabPage::FillItemSet(SfxItemSet& rSet)
{
    UINT16  nPrintSize;
    if (aSizeNormal.IsChecked())
        nPrintSize = PRINT_SIZE_NORMAL;
    else if (aSizeScaled.IsChecked())
        nPrintSize = PRINT_SIZE_SCALED;
    else
        nPrintSize = PRINT_SIZE_ZOOMED;

    rSet.Put(SfxUInt16Item(GetWhich(SID_PRINTSIZE), (UINT16) nPrintSize));
    rSet.Put(SfxUInt16Item(GetWhich(SID_PRINTZOOM), (UINT16) aZoom.GetValue()));
    rSet.Put(SfxBoolItem(GetWhich(SID_PRINTTITLE), aTitle.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_PRINTTEXT), aText.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_PRINTFRAME), aFrame.IsChecked()));
    rSet.Put(SfxBoolItem(GetWhich(SID_NO_RIGHT_SPACES), aNoRightSpaces.IsChecked()));

    return TRUE;
}


void SmPrintOptionsTabPage::Reset(const SfxItemSet& rSet)
{
    SmPrintSize ePrintSize = (SmPrintSize)((const SfxUInt16Item &)rSet.Get(GetWhich(SID_PRINTSIZE))).GetValue();

    aSizeNormal.Check(ePrintSize == PRINT_SIZE_NORMAL);
    aSizeScaled.Check(ePrintSize == PRINT_SIZE_SCALED);
    aSizeZoomed.Check(ePrintSize == PRINT_SIZE_ZOOMED);

    aZoom.Enable(aSizeZoomed.IsChecked());

    aZoom.SetValue(((const SfxUInt16Item &)rSet.Get(GetWhich(SID_PRINTZOOM))).GetValue());

    aTitle.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_PRINTTITLE))).GetValue());
    aText.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_PRINTTEXT))).GetValue());
    aFrame.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_PRINTFRAME))).GetValue());
    aNoRightSpaces.Check(((const SfxBoolItem &)rSet.Get(GetWhich(SID_NO_RIGHT_SPACES))).GetValue());
}


SfxTabPage* SmPrintOptionsTabPage::Create(Window* pWindow, const SfxItemSet& rSet)
{
    return (new SmPrintOptionsTabPage(pWindow, rSet));
}

/**************************************************************************/


#ifdef NEVER
SmExtraOptionsTabPage::SmExtraOptionsTabPage(const SfxItemSet& rInSet, Window * pParent, BOOL bFreeRes)
    : SfxTabPage(pParent, SmResId(RID_EXTRAOPTIONPAGE), rInSet),
    aAutoRedraw   (this, ResId(1)),
    aFixedText1   (this, ResId(1)),
    aSymbolFile   (this, ResId(2), 32),
    aBrowseButton (this, ResId(1))
{
    if (bFreeRes)
        FreeResource();

    aBrowseButton.SetClickHdl(LINK(this, SmExtraOptionsTabPage, SymClickHdl));
}


IMPL_LINK( SmExtraOptionsTabPage, SymClickHdl, PushButton *, pPushButton )
{
    SfxSimpleFileDialog  *pFileDialog =
            new SfxSimpleFileDialog(this, WinBits(WB_OPEN | WB_3DLOOK));

#ifdef MAC
    pFileDialog->AddFilter(SmResId(RID_SYMBOLFILESSTR), SmResId(RID_FILESYMTYP));
    pFileDialog->AddFilter(SmResId(RID_ALLFILESSTR), "****");

    pFileDialog->SetCurFilter(SmResId(RID_SYMBOLFILESSTR));
#else
    pFileDialog->AddFilter(SmResId(RID_SYMBOLFILESSTR), C2S("*.sms"));
    pFileDialog->AddFilter(SmResId(RID_ALLFILESSTR), C2S("*.*"));

    pFileDialog->SetCurFilter(SmResId(RID_SYMBOLFILESSTR));

    pFileDialog->SetDefaultExt(C2S("*.sms"));
#endif

    pFileDialog->SetPath(aSymbolFile.GetText());

    if (pFileDialog->Execute() == RET_OK)
        aSymbolFile.SetText(pFileDialog->GetPath());

    delete pFileDialog;
    return 0;
}


BOOL SmExtraOptionsTabPage::FillItemSet(SfxItemSet& rOutSet)
{
    rOutSet.Put(SfxBoolItem(GetWhich(SID_AUTOREDRAW), aAutoRedraw.IsChecked()));
    rOutSet.Put(SfxStringItem(GetWhich(SID_SYMBOLFILE), aSymbolFile.GetText()));

    return (TRUE);
}


void SmExtraOptionsTabPage::Reset(const SfxItemSet& rOutSet)
{
    aAutoRedraw.Check(((const SfxBoolItem&)rOutSet.Get(GetWhich(SID_AUTOREDRAW))).GetValue());
    aSymbolFile.SetText(((const SfxStringItem&)rOutSet.Get(GetWhich(SID_SYMBOLFILE))).GetValue());
}


SfxTabPage* SmExtraOptionsTabPage::Create(Window* pWindow, const SfxItemSet& rSet)
{
    return (new SmExtraOptionsTabPage(rSet, pWindow));
}
#endif //NEVER

/**************************************************************************/


void SmShowFont::Paint(const Rectangle&)
{
    XubString   Text (GetFont().GetName());
    Size    TextSize(GetTextWidth(Text), GetTextHeight());

    DrawText(Point((GetOutputSize().Width()  - TextSize.Width())  / 2,
                   (GetOutputSize().Height() - TextSize.Height()) / 2), Text);
}


void SmShowFont::SetFont(const Font& rFont)
{
    Font aFont (rFont);

    Invalidate();
    aFont.SetSize(Size(0, 24));
    aFont.SetAlign(ALIGN_TOP);
    Control::SetFont(aFont);
}


IMPL_LINK_INLINE_START( SmFontDialog, FontChangeHdl, ComboBox *, pComboBox )
{
    Face.SetName(pComboBox->GetText());
    aShowFont.SetFont(Face);
    return 0;
}
IMPL_LINK_INLINE_END( SmFontDialog, FontChangeHdl, ComboBox *, pComboBox )


IMPL_LINK( SmFontDialog, AttrChangeHdl, CheckBox *, pCheckBox )
{
    if (aBoldCheckBox.IsChecked())
        Face.SetWeight(FontWeight(WEIGHT_BOLD));
    else
        Face.SetWeight(FontWeight(WEIGHT_NORMAL));

    if (aItalicCheckBox.IsChecked())
        Face.SetItalic(ITALIC_NORMAL);
    else
        Face.SetItalic(ITALIC_NONE);

    aShowFont.SetFont(Face);
    return 0;
}


void SmFontDialog::SetFont(const Font &rFont)
{
    Face = rFont;

    aFontBox.SetText(Face.GetName());
    aBoldCheckBox.Check(Face.GetWeight() == WEIGHT_BOLD);
    aItalicCheckBox.Check(Face.GetItalic() != ITALIC_NONE);

    aShowFont.SetFont(Face);
}


SmFontDialog::SmFontDialog(Window * pParent, BOOL bFreeRes)
    : ModalDialog(pParent,SmResId(RID_FONTDIALOG)),
    aFixedText1     (this, ResId(1)),
    aFontBox        (this, ResId(1)),
    aBoldCheckBox   (this, ResId(1)),
    aItalicCheckBox (this, ResId(2)),
    aOKButton1      (this, ResId(1)),
    aCancelButton1  (this, ResId(1)),
    aShowFont       (this, ResId(1)),
    aGroupBox1      (this, ResId(1)),
    aGroupBox2      (this, ResId(2))
{
    if (bFreeRes)
        FreeResource();

    {
        WaitObject( this );
        //Application::EnterWait();

        // get FontList from printer (if possible), otherwise from application window
        SmViewShell *pView = SmGetActiveView();
        DBG_ASSERT(pView, "Sm : NULL pointer");
        OutputDevice *pDev = pView->GetDoc()->GetPrinter();
        if (!pDev || pDev->GetDevFontCount() == 0)
            pDev = &pView->GetGraphicWindow();
        FontList aFontList(pDev);

        USHORT  nCount = aFontList.GetFontNameCount();
        for (USHORT i = 0;  i < nCount;  i++)
            aFontBox.InsertEntry( aFontList.GetFontName(i).GetName() );

        Face.SetSize(Size(0, 24));
        Face.SetWeight(WEIGHT_NORMAL);
        Face.SetItalic(ITALIC_NONE);
        Face.SetFamily(FAMILY_DONTKNOW);
        Face.SetPitch(PITCH_DONTKNOW);
        Face.SetCharSet(RTL_TEXTENCODING_DONTKNOW);
        Face.SetTransparent(TRUE);

        aShowFont.SetFillColor( Color(COL_LIGHTGRAY) );

        //Application::LeaveWait();
    }

    aFontBox.SetSelectHdl(LINK(this, SmFontDialog, FontChangeHdl));
    aBoldCheckBox.SetClickHdl(LINK(this, SmFontDialog, AttrChangeHdl));
    aItalicCheckBox.SetClickHdl(LINK(this, SmFontDialog, AttrChangeHdl));
}

/**************************************************************************/


IMPL_LINK( SmFontSizeDialog, DefaultButtonClickHdl, Button *, pButton )
{
    QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));

    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD1();
        WriteTo(pp->GetConfig()->GetFormat());
    }

    delete pQueryBox;
    return 0;
}


SmFontSizeDialog::SmFontSizeDialog(Window * pParent, BOOL bFreeRes)
    : ModalDialog(pParent, SmResId(RID_FONTSIZEDIALOG)),
    aFixedText1(this, ResId(1)),
    aBaseSize(this, ResId(1)),
    aFixedText4(this, ResId(4)),
    aTextSize(this, ResId(4)),
    aFixedText5(this, ResId(5)),
    aIndexSize(this, ResId(5)),
    aFixedText6(this, ResId(6)),
    aFunctionSize(this, ResId(6)),
    aFixedText7(this, ResId(7)),
    aOperatorSize(this, ResId(7)),
    aFixedText8(this, ResId(8)),
    aBorderSize(this, ResId(8)),
    aGroupBox1(this, ResId(1)),
    aOKButton1(this, ResId(1)),
    aCancelButton1(this, ResId(1)),
    aDefaultButton(this, ResId(1))
{
    if (bFreeRes)
        FreeResource();

    aDefaultButton.SetClickHdl(LINK(this, SmFontSizeDialog, DefaultButtonClickHdl));
}


void SmFontSizeDialog::ReadFrom(const SmFormat &rFormat)
{
    //! aufpassen: richtig runden!
    aBaseSize.SetValue( SmRoundFraction(
        Sm100th_mmToPts( rFormat.GetBaseSize().Height() ) ) );

    aTextSize    .SetValue( rFormat.GetRelSize(SIZ_TEXT) );
    aIndexSize   .SetValue( rFormat.GetRelSize(SIZ_INDEX) );
    aFunctionSize.SetValue( rFormat.GetRelSize(SIZ_FUNCTION) );
    aOperatorSize.SetValue( rFormat.GetRelSize(SIZ_OPERATOR) );
    aBorderSize  .SetValue( rFormat.GetRelSize(SIZ_LIMITS) );
}


void SmFontSizeDialog::WriteTo(SmFormat &rFormat) const
{
    rFormat.SetBaseSize( Size(0, SmPtsTo100th_mm(aBaseSize.GetValue())) );

    rFormat.SetRelSize(SIZ_TEXT,     (USHORT) aTextSize    .GetValue());
    rFormat.SetRelSize(SIZ_INDEX,    (USHORT) aIndexSize   .GetValue());
    rFormat.SetRelSize(SIZ_FUNCTION, (USHORT) aFunctionSize.GetValue());
    rFormat.SetRelSize(SIZ_OPERATOR, (USHORT) aOperatorSize.GetValue());
    rFormat.SetRelSize(SIZ_LIMITS,   (USHORT) aBorderSize  .GetValue());

    const Size aTmp (rFormat.GetBaseSize());
    for (USHORT  i = FNT_BEGIN;  i <= FNT_END;  i++)
        rFormat.Font(i).SetSize(aTmp);

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


IMPL_LINK( SmFontTypeDialog, MenuSelectHdl, Menu *, pMenu )
{
    SmFontPickListBox *pActiveListBox;

    switch (pMenu->GetCurItemId())
    {
        case 1: pActiveListBox = &aVariableFont; break;
        case 2: pActiveListBox = &aFunctionFont; break;
        case 3: pActiveListBox = &aNumberFont;   break;
        case 4: pActiveListBox = &aTextFont;     break;
        case 5: pActiveListBox = &aSerifFont;    break;
        case 6: pActiveListBox = &aSansFont;     break;
        case 7: pActiveListBox = &aFixedFont;    break;
        default:pActiveListBox = NULL;
    }

    if (pActiveListBox)
    {
        SmFontDialog *pFontDialog = new SmFontDialog(this);

        pActiveListBox->WriteTo(*pFontDialog);
        if (pFontDialog->Execute() == RET_OK)
            pActiveListBox->ReadFrom(*pFontDialog);
        delete pFontDialog;
    }
    return 0;
}


IMPL_LINK_INLINE_START( SmFontTypeDialog, DefaultButtonClickHdl, Button *, pButton )
{
    QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));
    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD1();
        WriteTo(pp->GetConfig()->GetFormat());
    }

    delete pQueryBox;
    return 0;
}
IMPL_LINK_INLINE_END( SmFontTypeDialog, DefaultButtonClickHdl, Button *, pButton )


SmFontTypeDialog::SmFontTypeDialog(Window * pParent, BOOL bFreeRes)
    : ModalDialog(pParent, SmResId(RID_FONTTYPEDIALOG)),
    aFixedText1    (this, ResId(1)),
    aVariableFont  (this, ResId(1)),
    aFixedText2    (this, ResId(2)),
    aFunctionFont  (this, ResId(2)),
    aFixedText3    (this, ResId(3)),
    aNumberFont    (this, ResId(3)),
    aFixedText4    (this, ResId(4)),
    aTextFont      (this, ResId(4)),
    aFixedText5    (this, ResId(5)),
    aSerifFont     (this, ResId(5)),
    aFixedText6    (this, ResId(6)),
    aSansFont      (this, ResId(6)),
    aFixedText7    (this, ResId(7)),
    aFixedFont     (this, ResId(7)),
    aGroupBox1     (this, ResId(1)),
    aGroupBox2     (this, ResId(2)),
    aOKButton1     (this, ResId(1)),
    aCancelButton1 (this, ResId(1)),
    aMenuButton    (this, ResId(1)),
    aDefaultButton (this, ResId(2))
{
    if (bFreeRes)
        FreeResource();

    aDefaultButton.SetClickHdl(LINK(this, SmFontTypeDialog, DefaultButtonClickHdl));

    aMenuButton.GetPopupMenu()->SetSelectHdl(LINK(this, SmFontTypeDialog, MenuSelectHdl));
}

void SmFontTypeDialog::ReadFrom(const SmFormat &rFormat)
{
    SmModule *pp = SM_MOD1();

    aVariableFont = pp->GetConfig()->GetFontPickList(FNT_VARIABLE);
    aFunctionFont = pp->GetConfig()->GetFontPickList(FNT_FUNCTION);
    aNumberFont   = pp->GetConfig()->GetFontPickList(FNT_NUMBER);
    aTextFont     = pp->GetConfig()->GetFontPickList(FNT_TEXT);
    aSerifFont    = pp->GetConfig()->GetFontPickList(FNT_SERIF);
    aSansFont     = pp->GetConfig()->GetFontPickList(FNT_SANS);
    aFixedFont    = pp->GetConfig()->GetFontPickList(FNT_FIXED);

    aVariableFont.Insert( rFormat.GetFont(FNT_VARIABLE) );
    aFunctionFont.Insert( rFormat.GetFont(FNT_FUNCTION) );
    aNumberFont  .Insert( rFormat.GetFont(FNT_NUMBER) );
    aTextFont    .Insert( rFormat.GetFont(FNT_TEXT) );
    aSerifFont   .Insert( rFormat.GetFont(FNT_SERIF) );
    aSansFont    .Insert( rFormat.GetFont(FNT_SANS) );
    aFixedFont   .Insert( rFormat.GetFont(FNT_FIXED) );
}


void SmFontTypeDialog::WriteTo(SmFormat &rFormat) const
{
    SmModule *pp = SM_MOD1();

    pp->GetConfig()->GetFontPickList(FNT_VARIABLE) = aVariableFont;
    pp->GetConfig()->GetFontPickList(FNT_FUNCTION) = aFunctionFont;
    pp->GetConfig()->GetFontPickList(FNT_NUMBER)   = aNumberFont;
    pp->GetConfig()->GetFontPickList(FNT_TEXT)     = aTextFont;
    pp->GetConfig()->GetFontPickList(FNT_SERIF)    = aSerifFont;
    pp->GetConfig()->GetFontPickList(FNT_SANS)     = aSansFont;
    pp->GetConfig()->GetFontPickList(FNT_FIXED)    = aFixedFont;

    rFormat.SetFont( FNT_VARIABLE, aVariableFont.Get(0) );
    rFormat.SetFont( FNT_FUNCTION, aFunctionFont.Get(0) );
    rFormat.SetFont( FNT_NUMBER,   aNumberFont  .Get(0) );
    rFormat.SetFont( FNT_TEXT,     aTextFont    .Get(0) );
    rFormat.SetFont( FNT_SERIF,    aSerifFont   .Get(0) );
    rFormat.SetFont( FNT_SANS,     aSansFont    .Get(0) );
    rFormat.SetFont( FNT_FIXED,    aFixedFont   .Get(0) );

    for (USHORT i = FNT_BEGIN;  i <= FNT_FIXED;  i++)
        rFormat.Font(i).SetTransparent(TRUE);

    rFormat.RequestApplyChanges();
}

/**************************************************************************/

struct FieldMinMax
{
    USHORT nMin, nMax;
};

// Data for min and max values of the 4 metric fields
// for each of the 10 categories
static const FieldMinMax pMinMaxData[10][4] =
{
    // 0
    {{ 0, 200 },    { 0, 200 },     { 0, 100 },     { 0, 0 }},
    // 1
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 2
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 3
    {{ 0, 100 },    { 1, 100 },     { 0, 0 },       { 0, 0 }},
    // 4
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 5
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 100 }},
    // 6
    {{ 0, 300 },    { 0, 300 },     { 0, 0 },       { 0, 0 }},
    // 7
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 8
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 9
    {{ 0, 10000 },  { 0, 10000 },   { 0, 10000 },   { 0, 10000 }}
};

SmCategoryDesc::SmCategoryDesc(const ResId& rResId, USHORT nCategoryIdx) :
    Resource(rResId)
{
    if (IsAvailableRes(ResId(1).SetRT(RSC_STRING)))
    {
        Name = XubString(ResId(1));

        int i;
        for (i = 0; i < 4; i++)
        {
            int nI2 = i + 2;

            if (IsAvailableRes(ResId(nI2).SetRT(RSC_STRING)))
            {
                Strings[i]  = new XubString(ResId(nI2));
                Graphics[i] = new Bitmap(ResId(nI2));
            }
            else
            {
                Strings[i]  = 0;
                Graphics[i] = 0;
            }
        }

        for (i = 0; i < 4; i++)
        {
            const FieldMinMax &rMinMax = pMinMaxData[ nCategoryIdx ][i];
            Value[i] = Minimum[i] = rMinMax.nMin;
            Maximum[i] = rMinMax.nMax;
        }
    }

    FreeResource();
}


SmCategoryDesc::~SmCategoryDesc()
{
    for (int i = 0; i < 4; i++)
    {
        delete Strings[i];
        delete Graphics[i];
    }
}

/**************************************************************************/

IMPL_LINK( SmDistanceDialog, GetFocusHdl, Control *, pControl )
{
    if (Categories[nActiveCategory])
    {
        USHORT  i;

        if (pControl == &aMetricField1)
            i = 0;
        else if (pControl == &aMetricField2)
            i = 1;
        else if (pControl == &aMetricField3)
            i = 2;
        else if (pControl == &aMetricField4)
            i = 3;
        else
            return 0;
        aBitmap.SetBitmap(*(Categories[nActiveCategory]->GetGraphic(i)));
    }
    return 0;
}

IMPL_LINK( SmDistanceDialog, MenuSelectHdl, Menu *, pMenu )
{
    SetCategory(pMenu->GetCurItemId() - 1);
    return 0;
}


IMPL_LINK( SmDistanceDialog, DefaultButtonClickHdl, Button *, pButton )
{
    QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));

    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD1();
        WriteTo(pp->GetConfig()->GetFormat());
    }
    delete pQueryBox;
    return 0;
}


IMPL_LINK( SmDistanceDialog, CheckBoxClickHdl, CheckBox *, pCheckBox )
{
    if (pCheckBox == &aCheckBox1)
    {
        aCheckBox1.Toggle();

        BOOL bChecked = aCheckBox1.IsChecked();
        aFixedText4  .Enable( bChecked );
        aMetricField4.Enable( bChecked );
    }
    return 0;
}


void SmDistanceDialog::SetHelpId(MetricField &rField, ULONG nHelpId)
{
    //! HelpID's die auf diese Weise explizit gesetzt werden, müssen im
    //! util Verzeichnis im File "hidother.src" mit Hilfe von "hidspecial"
    //! definiert werden!

    const XubString aEmptyText;
    DBG_ASSERT(aEmptyText.Len() == 0, "Sm: Ooops...");

    rField.SetHelpId(nHelpId);
    rField.SetHelpText(aEmptyText);

    // since MetricField inherits from SpinField which has a sub Edit field
    // (which is actually the one we modify) we have to set the help-id
    // for it too.
    Edit *pSubEdit = rField.GetSubEdit();
    if (pSubEdit)
    {   pSubEdit->SetHelpId(nHelpId);
        pSubEdit->SetHelpText(aEmptyText);
    }
}


void SmDistanceDialog::SetCategory(USHORT nCategory)
{
    DBG_ASSERT(0 <= nCategory  &&  nCategory < NOCATEGORIES,
        "Sm: falsche Kategorienummer in SmDistanceDialog");

    // array to convert category- and metricfield-number in help ids.
    // 0 is used in case of unused combinations.
    DBG_ASSERT(NOCATEGORIES == 10, "Sm : Array paßt nicht zu Anzahl der Kategorien");
    ULONG __READONLY_DATA  aCatMf2Hid[10][4] =
    {
        HID_SMA_DEFAULT_DIST,       HID_SMA_LINE_DIST,          HID_SMA_ROOT_DIST, 0,
        HID_SMA_SUP_DIST,           HID_SMA_SUB_DIST ,          0, 0,
        HID_SMA_NUMERATOR_DIST,     HID_SMA_DENOMINATOR_DIST,   0, 0,
        HID_SMA_FRACLINE_EXCWIDTH,  HID_SMA_FRACLINE_LINEWIDTH, 0, 0,
        HID_SMA_UPPERLIMIT_DIST,    HID_SMA_LOWERLIMIT_DIST,    0, 0,
        HID_SMA_BRACKET_EXCHEIGHT,  HID_SMA_BRACKET_DIST,       0, HID_SMA_BRACKET_EXCHEIGHT2,
        HID_SMA_MATRIXROW_DIST,     HID_SMA_MATRIXCOL_DIST,     0, 0,
        HID_SMA_ATTRIBUT_DIST,      HID_SMA_INTERATTRIBUT_DIST, 0, 0,
        HID_SMA_OPERATOR_EXCHEIGHT, HID_SMA_OPERATOR_DIST,      0, 0,
        HID_SMA_LEFTBORDER_DIST,    HID_SMA_RIGHTBORDER_DIST,   HID_SMA_UPPERBORDER_DIST, HID_SMA_LOWERBORDER_DIST
    };

    // array to help iterate over the controls
    Window * __READONLY_DATA  aWin[4][2] =
    {
        &aFixedText1,   &aMetricField1,
        &aFixedText2,   &aMetricField2,
        &aFixedText3,   &aMetricField3,
        &aFixedText4,   &aMetricField4
    };

    SmCategoryDesc *pCat;

    // merken der (evtl neuen) Einstellungen der aktiven SmCategoryDesc
    // bevor zu der neuen gewechselt wird.
    if (nActiveCategory != CATEGORY_NONE)
    {
        pCat = Categories[nActiveCategory];
        pCat->SetValue(0, (USHORT) aMetricField1.GetValue());
        pCat->SetValue(1, (USHORT) aMetricField2.GetValue());
        pCat->SetValue(2, (USHORT) aMetricField3.GetValue());
        pCat->SetValue(3, (USHORT) aMetricField4.GetValue());

        if (nActiveCategory == 5)
            bScaleAllBrackets = aCheckBox1.IsChecked();

        aMenuButton.GetPopupMenu()->CheckItem(nActiveCategory + 1, FALSE);
    }

    // aktivieren/deaktivieren der zugehörigen Controls in Abhängigkeit von der
    // gewählten Kategorie.
    BOOL  bActive;
    for (int i = 0;  i < 4;  i++)
    {
        FixedText   *pFT = (FixedText * const)   aWin[i][0];
        MetricField *pMF = (MetricField * const) aWin[i][1];

        // Um feststellen welche Controls aktiv sein sollen wird das
        // vorhandensein einer zugehörigen HelpID überprüft.
        bActive = aCatMf2Hid[nCategory][i] != 0;

        pFT->Show(bActive);
        pFT->Enable(bActive);
        pMF->Show(bActive);
        pMF->Enable(bActive);

        // setzen von Maßeinheit und Anzahl der Nachkommastellen
        FieldUnit  eUnit;
        USHORT     nDigits;
        if (nCategory < 9)
        {
            eUnit   = FUNIT_CUSTOM;
            nDigits = 0;
            pMF->SetCustomUnitText( '%' );
        }
        else
        {
            eUnit   = FUNIT_100TH_MM;
            nDigits = 2;
        }
        pMF->SetUnit(eUnit);            //! verändert den Wert
        pMF->SetDecimalDigits(nDigits);

        if (bActive)
        {
            pCat = Categories[nCategory];
            pFT->SetText(*pCat->GetString(i));

            pMF->SetMin(pCat->GetMinimum(i));
            pMF->SetMax(pCat->GetMaximum(i));
            pMF->SetValue(pCat->GetValue(i));

            SetHelpId(*pMF, aCatMf2Hid[nCategory][i]);
        }
    }
    // nun noch die CheckBox und das zugehörige MetricField genau dann aktivieren,
    // falls es sich um das Klammer Menu handelt.
    bActive = nCategory == 5;
    aCheckBox1.Show(bActive);
    aCheckBox1.Enable(bActive);
    if (bActive)
    {
        aCheckBox1.Check( bScaleAllBrackets );

        BOOL bChecked = aCheckBox1.IsChecked();
        aFixedText4  .Enable( bChecked );
        aMetricField4.Enable( bChecked );
    }

    aMenuButton.GetPopupMenu()->CheckItem(nCategory + 1, TRUE);
    aGroupBox.SetText(Categories[nCategory]->GetName());

    nActiveCategory = nCategory;

    aMetricField1.GrabFocus();
    Invalidate();
    Update();
}


SmDistanceDialog::SmDistanceDialog(Window *pParent, BOOL bFreeRes)
    : ModalDialog(pParent, SmResId(RID_DISTANCEDIALOG)),
    aFixedText1    (this, ResId(1)),
    aFixedText2    (this, ResId(2)),
    aFixedText3    (this, ResId(3)),
    aFixedText4    (this, ResId(4)),
    aMetricField1  (this, ResId(1)),
    aMetricField2  (this, ResId(2)),
    aMetricField3  (this, ResId(3)),
    aMetricField4  (this, ResId(4)),
    aOKButton1     (this, ResId(1)),
    aCancelButton1 (this, ResId(1)),
    aMenuButton    (this, ResId(1)),
    aDefaultButton (this, ResId(1)),
    aCheckBox1     (this, ResId(1)),
    aBitmap        (this, ResId(1)),
    aGroupBox      (this, ResId(1))
{
    for (int i = 0; i < NOCATEGORIES; i++)
        Categories[i] = new SmCategoryDesc(SmResId(i + 1), i);
    nActiveCategory   = CATEGORY_NONE;
    bScaleAllBrackets = FALSE;

    if (bFreeRes)
        FreeResource();

    aMetricField1.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aMetricField2.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aMetricField3.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aMetricField4.SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    aCheckBox1.SetClickHdl(LINK(this, SmDistanceDialog, CheckBoxClickHdl));

    aMenuButton.GetPopupMenu()->SetSelectHdl(LINK(this, SmDistanceDialog, MenuSelectHdl));

    aDefaultButton.SetClickHdl(LINK(this, SmDistanceDialog, DefaultButtonClickHdl));
                       }


SmDistanceDialog::~SmDistanceDialog()
{
    for (int i = 0; i < NOCATEGORIES; i++)
        DELETEZ(Categories[i]);
}


void SmDistanceDialog::ReadFrom(const SmFormat &rFormat)
{
    Categories[0]->SetValue(0, rFormat.GetDistance(DIS_HORIZONTAL));
    Categories[0]->SetValue(1, rFormat.GetDistance(DIS_VERTICAL));
    Categories[0]->SetValue(2, rFormat.GetDistance(DIS_ROOT));
    Categories[1]->SetValue(0, rFormat.GetDistance(DIS_SUPERSCRIPT));
    Categories[1]->SetValue(1, rFormat.GetDistance(DIS_SUBSCRIPT));
    Categories[2]->SetValue(0, rFormat.GetDistance(DIS_NUMERATOR));
    Categories[2]->SetValue(1, rFormat.GetDistance(DIS_DENOMINATOR));
    Categories[3]->SetValue(0, rFormat.GetDistance(DIS_FRACTION));
    Categories[3]->SetValue(1, rFormat.GetDistance(DIS_STROKEWIDTH));
    Categories[4]->SetValue(0, rFormat.GetDistance(DIS_UPPERLIMIT));
    Categories[4]->SetValue(1, rFormat.GetDistance(DIS_LOWERLIMIT));
    Categories[5]->SetValue(0, rFormat.GetDistance(DIS_BRACKETSIZE));
    Categories[5]->SetValue(1, rFormat.GetDistance(DIS_BRACKETSPACE));
    Categories[5]->SetValue(3, rFormat.GetDistance(DIS_NORMALBRACKETSIZE));
    Categories[6]->SetValue(0, rFormat.GetDistance(DIS_MATRIXROW));
    Categories[6]->SetValue(1, rFormat.GetDistance(DIS_MATRIXCOL));
    Categories[7]->SetValue(0, rFormat.GetDistance(DIS_ORNAMENTSIZE));
    Categories[7]->SetValue(1, rFormat.GetDistance(DIS_ORNAMENTSPACE));
    Categories[8]->SetValue(0, rFormat.GetDistance(DIS_OPERATORSIZE));
    Categories[8]->SetValue(1, rFormat.GetDistance(DIS_OPERATORSPACE));
    Categories[9]->SetValue(0, rFormat.GetDistance(DIS_LEFTSPACE));
    Categories[9]->SetValue(1, rFormat.GetDistance(DIS_RIGHTSPACE));
    Categories[9]->SetValue(2, rFormat.GetDistance(DIS_TOPSPACE));
    Categories[9]->SetValue(3, rFormat.GetDistance(DIS_BOTTOMSPACE));

    bScaleAllBrackets = rFormat.IsScaleNormalBrackets();

    // force update (even of category 0) by setting nActiveCategory to a
    // non-existent category number
    nActiveCategory = CATEGORY_NONE;
    SetCategory(0);
}


void SmDistanceDialog::WriteTo(SmFormat &rFormat) /*const*/
{
    // hmm... k”nnen die tats„chlich unterschiedlich sein?
    // wenn nicht kann oben n„mlich das const stehen!
    SetCategory(nActiveCategory);

    rFormat.SetDistance( DIS_HORIZONTAL,        Categories[0]->GetValue(0) );
    rFormat.SetDistance( DIS_VERTICAL,          Categories[0]->GetValue(1) );
    rFormat.SetDistance( DIS_ROOT,              Categories[0]->GetValue(2) );
    rFormat.SetDistance( DIS_SUPERSCRIPT,       Categories[1]->GetValue(0) );
    rFormat.SetDistance( DIS_SUBSCRIPT,         Categories[1]->GetValue(1) );
    rFormat.SetDistance( DIS_NUMERATOR,         Categories[2]->GetValue(0) );
    rFormat.SetDistance( DIS_DENOMINATOR,       Categories[2]->GetValue(1) );
    rFormat.SetDistance( DIS_FRACTION,          Categories[3]->GetValue(0) );
    rFormat.SetDistance( DIS_STROKEWIDTH,       Categories[3]->GetValue(1) );
    rFormat.SetDistance( DIS_UPPERLIMIT,        Categories[4]->GetValue(0) );
    rFormat.SetDistance( DIS_LOWERLIMIT,        Categories[4]->GetValue(1) );
    rFormat.SetDistance( DIS_BRACKETSIZE,       Categories[5]->GetValue(0) );
    rFormat.SetDistance( DIS_BRACKETSPACE,      Categories[5]->GetValue(1) );
    rFormat.SetDistance( DIS_MATRIXROW,         Categories[6]->GetValue(0) );
    rFormat.SetDistance( DIS_MATRIXCOL,         Categories[6]->GetValue(1) );
    rFormat.SetDistance( DIS_ORNAMENTSIZE,      Categories[7]->GetValue(0) );
    rFormat.SetDistance( DIS_ORNAMENTSPACE,     Categories[7]->GetValue(1) );
    rFormat.SetDistance( DIS_OPERATORSIZE,      Categories[8]->GetValue(0) );
    rFormat.SetDistance( DIS_OPERATORSPACE,     Categories[8]->GetValue(1) );
    rFormat.SetDistance( DIS_LEFTSPACE,         Categories[9]->GetValue(0) );
    rFormat.SetDistance( DIS_RIGHTSPACE,        Categories[9]->GetValue(1) );
    rFormat.SetDistance( DIS_TOPSPACE,          Categories[9]->GetValue(2) );
    rFormat.SetDistance( DIS_BOTTOMSPACE,       Categories[9]->GetValue(3) );
    rFormat.SetDistance( DIS_NORMALBRACKETSIZE, Categories[5]->GetValue(3) );

    rFormat.SetScaleNormalBrackets( bScaleAllBrackets );

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


IMPL_LINK( SmAlignDialog, DefaultButtonClickHdl, Button *, pButton )
{
   QueryBox *pQueryBox = new QueryBox(this, SmResId(RID_DEFAULTSAVEQUERY));

    if (pQueryBox->Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD1();
        WriteTo(pp->GetConfig()->GetFormat());
    }

    delete pQueryBox;
    return 0;
}


SmAlignDialog::SmAlignDialog(Window * pParent, BOOL bFreeRes)
    : ModalDialog(pParent, SmResId(RID_ALIGNDIALOG)),
    aLeft          (this, ResId(1)),
    aCenter        (this, ResId(2)),
    aRight         (this, ResId(3)),
    aGroupBox1     (this, ResId(1)),
    aOKButton1     (this, ResId(1)),
    aCancelButton1 (this, ResId(1)),
    aDefaultButton (this, ResId(1))
{
    if (bFreeRes)
        FreeResource();

    aDefaultButton.SetClickHdl(LINK(this, SmAlignDialog, DefaultButtonClickHdl));
}


void SmAlignDialog::ReadFrom(const SmFormat &rFormat)
{
    switch (rFormat.GetHorAlign())
    {
        case AlignLeft:
            aLeft  .Check(TRUE);
            aCenter.Check(FALSE);
            aRight .Check(FALSE);
            break;

        case AlignCenter:
            aLeft  .Check(FALSE);
            aCenter.Check(TRUE);
            aRight .Check(FALSE);
            break;

        case AlignRight:
            aLeft  .Check(FALSE);
            aCenter.Check(FALSE);
            aRight .Check(TRUE);
            break;
    }
}


void SmAlignDialog::WriteTo(SmFormat &rFormat) const
{
    if (aLeft.IsChecked())
        rFormat.SetHorAlign(AlignLeft);
    else if (aRight.IsChecked())
        rFormat.SetHorAlign(AlignRight);
    else
        rFormat.SetHorAlign(AlignCenter);

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


void SmShowSymbolSet::Paint(const Rectangle&)
{
    Push(PUSH_MAPMODE);

    // MapUnit einstellen für die 'nLen' berechnet wurde
    SetMapMode(MapMode(MAP_PIXEL));

    int v        = (int) (aVScrollBar.GetThumbPos() * nColumns);
    int nSymbols = (int) aSymbolSet.GetCount();

    for (int i = v; i < nSymbols ; i++)
    {
        SmSym    aSymbol (aSymbolSet.GetSymbol(i));
        Font     aFont   (aSymbol.GetFace());

        // etwas kleinere FontSize nehmen (als nLen) um etwas Luft zu haben
        // (hoffentlich auch genug für links und rechts!)
        aFont.SetSize(Size(0, nLen - (nLen / 3)));
        SetFont(aFont);

        int   nIV   = i - v;
        Size  aSize(GetTextWidth(aSymbol.GetCharacter()), GetTextHeight());

        DrawText(Point((nIV % nColumns) * nLen + (nLen - aSize.Width()) / 2,
                       (nIV / nColumns) * nLen + (nLen - aSize.Height()) / 2),
                 aSymbol.GetCharacter());
    }

    if (nSelectSymbol != SYMBOL_NONE)
    {
        Invert(Rectangle(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                 ((nSelectSymbol - v) / nColumns) * nLen),
                           Size(nLen, nLen)));
    }

    Pop();
}


void SmShowSymbolSet::MouseButtonDown(const MouseEvent& rMEvt)
{
    GrabFocus();

    if (rMEvt.IsLeft() && Rectangle(Point(0, 0), aOutputSize).IsInside(rMEvt.GetPosPixel()))
    {
        SelectSymbol ((rMEvt.GetPosPixel().Y() / nLen) * nColumns + (rMEvt.GetPosPixel().X() / nLen) +
                      aVScrollBar.GetThumbPos() * nColumns);

        aSelectHdlLink.Call(this);

        if (rMEvt.GetClicks() > 1) aDblClickHdlLink.Call(this);
    }
    else Control::MouseButtonDown (rMEvt);
}


void SmShowSymbolSet::KeyInput(const KeyEvent& rKEvt)
{
    USHORT n = nSelectSymbol;

    if (n != SYMBOL_NONE)
    {
        switch (rKEvt.GetKeyCode().GetCode())
        {
            case KEY_DOWN:      n += nColumns;  break;
            case KEY_UP:        n -= nColumns;  break;
            case KEY_LEFT:      n -= 1; break;
            case KEY_RIGHT:     n += 1; break;
            case KEY_HOME:      n  = 0; break;
            case KEY_END:       n  = aSymbolSet.GetCount() - 1; break;
            case KEY_PAGEUP:    n -= nColumns * nRows;  break;
            case KEY_PAGEDOWN:  n += nColumns * nRows;  break;

            default:
                Control::KeyInput(rKEvt);
                return;
        }
    }
    else
        n = 0;

    if (n >= aSymbolSet.GetCount())
        n = nSelectSymbol;

    // adjust scrollbar
    if ((n < (USHORT) (aVScrollBar.GetThumbPos() * nColumns)) ||
        (n >= (USHORT) ((aVScrollBar.GetThumbPos() + nRows) * nColumns)))
    {
        aVScrollBar.SetThumbPos(n / nColumns);
        Invalidate();
        Update();
    }

    SelectSymbol(n);
    aSelectHdlLink.Call(this);
}


SmShowSymbolSet::SmShowSymbolSet(Window *pParent, const ResId& rResId) :
    Control(pParent, rResId),
    aVScrollBar(this, WinBits(WB_VSCROLL))
{
    nSelectSymbol = SYMBOL_NONE;

    aOutputSize = GetOutputSizePixel();
    long nScrollBarWidth = aVScrollBar.GetSizePixel().Width(),
         nUseableWidth   = aOutputSize.Width() - nScrollBarWidth;

    // Höhe von 16pt in Pixeln (passend zu 'aOutputSize')
    nLen = (USHORT) LogicToPixel(Size(0, 16), MapMode(MAP_POINT)).Height();

    nColumns = nUseableWidth / nLen;
    if (nColumns > 2  && nColumns % 2 != 0)
        nColumns--;
    nRows    = aOutputSize.Height() / nLen;
    DBG_ASSERT(nColumns > 0, "Sm : keine Spalten");
    DBG_ASSERT(nRows > 0, "Sm : keine Zeilen");

    // genau passend machen
    aOutputSize.Width()  = nColumns * nLen;
    aOutputSize.Height() = nRows * nLen;

    aVScrollBar.SetPosSizePixel(Point(aOutputSize.Width() + 1, -1),
                                Size(nScrollBarWidth, aOutputSize.Height() + 2));
    aVScrollBar.Enable(FALSE);
    aVScrollBar.Show();
    aVScrollBar.SetScrollHdl(LINK(this, SmShowSymbolSet, ScrollHdl));

    Size WindowSize (aOutputSize);
    WindowSize.Width() += nScrollBarWidth;
    SetOutputSizePixel(WindowSize);

}


void SmShowSymbolSet::SetSymbolSet(const SmSymSet& rSymbolSet)
{
    aSymbolSet = rSymbolSet;

    if (aSymbolSet.GetCount() > (nColumns * nRows))
    {
        aVScrollBar.SetRange(Range(0, ((aSymbolSet.GetCount() + (nColumns - 1)) / nColumns) - nRows));
        aVScrollBar.Enable(TRUE);
    }
    else
    {
        aVScrollBar.SetRange(Range(0,0));
        aVScrollBar.Enable (FALSE);
    }

    Invalidate();
}


void SmShowSymbolSet::SelectSymbol(USHORT nSymbol)
{
    int v = (int) (aVScrollBar.GetThumbPos() * nColumns);

    if (nSelectSymbol != SYMBOL_NONE)
        Invalidate(Rectangle(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                   ((nSelectSymbol - v) / nColumns) * nLen),
                             Size(nLen, nLen)));

    if (nSymbol < aSymbolSet.GetCount())
        nSelectSymbol = nSymbol;

    if (aSymbolSet.GetCount() == 0)
        nSelectSymbol = SYMBOL_NONE;

    if (nSelectSymbol != SYMBOL_NONE)
        Invalidate(Rectangle(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                   ((nSelectSymbol - v) / nColumns) * nLen),
                             Size(nLen, nLen)));

    Update();
}

IMPL_LINK( SmShowSymbolSet, ScrollHdl, ScrollBar*, pScrollBar)
{
    Invalidate();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void SmShowSymbol::Paint(const Rectangle&)
{
    const XubString &rText = GetText();
    Size            aTextSize(GetTextWidth(rText), GetTextHeight());

    DrawText(Point((GetOutputSize().Width()  - aTextSize.Width())  / 2,
                   (GetOutputSize().Height() - aTextSize.Height()) / 2), rText);
}


void SmShowSymbol::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.GetClicks() > 1)
        aDblClickHdlLink.Call(this);
    else
        Control::MouseButtonDown (rMEvt);
}


void SmShowSymbol::SetSymbol(const SmSym *pSymbol)
{
    if (pSymbol)
    {
        Font aFont (pSymbol->GetFace());
        aFont.SetSize(Size(0, GetOutputSize().Height() - GetOutputSize().Height() / 3));
        SetFont(aFont);

        SetText(XubString(pSymbol->GetCharacter()));
    }

    // 'Invalidate' füllt den background mit der background-Farbe.
    // Falls der NULL pointer übergeben wurde reicht dies also zum löschen
    // der Anzeige
    Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void SmSymbolDialog::FillSymbolSets(BOOL bDeleteText)
    // füllt die Einträge der möglichen 'SymbolsSet's im Dialog mit den
    // aktuellen Werten des SymbolSet Managers, selektiert aber keinen.
{
    aSymbolSets.Clear();
    if (bDeleteText)
        aSymbolSets.SetNoSelection();

    USHORT nNumSymSets = rSymSetMgr.GetCount();
    for (USHORT i = 0;  i < nNumSymSets;  i++)
        aSymbolSets.InsertEntry(rSymSetMgr.GetSymbolSet(i)->GetName());
}


IMPL_LINK( SmSymbolDialog, SymbolSetChangeHdl, ListBox *, pListBox )
{
    DBG_ASSERT(pListBox == &aSymbolSets, "Sm : falsches Argument");

    SelectSymbolSet(aSymbolSets.GetSelectEntry());
    return 0;
}


IMPL_LINK( SmSymbolDialog, SymbolChangeHdl, SmShowSymbolSet *, pShowSymbolSet )
{
    DBG_ASSERT(pShowSymbolSet == &aSymbolSetDisplay, "Sm : falsches Argument");

    SelectSymbol(aSymbolSetDisplay.GetSelectSymbol());
    return 0;
}

IMPL_LINK( SmSymbolDialog, EditClickHdl, Button *, pButton )
{
    DBG_ASSERT(pButton == &aEditBtn, "Sm : falsches Argument");

    SmSymDefineDialog *pDialog = new SmSymDefineDialog(this, rSymSetMgr);

    // aktuelles Symbol und SymbolSet am neuen Dialog setzen
    const XubString  aSymSetName (aSymbolSets.GetSelectEntry()),
                    aSymName    (aSymbolName.GetText());
    pDialog->SelectOldSymbolSet(aSymSetName);
    pDialog->SelectOldSymbol(aSymName);
    pDialog->SelectSymbolSet(aSymSetName);
    pDialog->SelectSymbol(aSymName);

    // altes SymbolSet merken
    XubString  aOldSymbolSet (aSymbolSets.GetSelectEntry());

    // Dialog an evtl geänderte Daten des SymbolSet Manager anpassen
    if (pDialog->Execute() == RET_OK  &&  rSymSetMgr.IsModified())
        FillSymbolSets();

    // wenn das alte SymbolSet nicht mehr existiert zum ersten gehen
    // (soweit eines vorhanden ist)
    if (!SelectSymbolSet(aOldSymbolSet)  &&  aSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aSymbolSets.GetEntry(0));

    delete pDialog;
    return 0;
}


IMPL_LINK( SmSymbolDialog, SymbolDblClickHdl, SmShowSymbolSet *, pShowSymbolSet )
{
    DBG_ASSERT(pShowSymbolSet == &aSymbolSetDisplay, "Sm : falsches Argument");

    GetClickHdl(&aGetBtn);
    EndDialog(RET_OK);
    return 0;
}


IMPL_LINK( SmSymbolDialog, GetClickHdl, Button *, pButton )
{
    DBG_ASSERT(pButton == &aGetBtn, "Sm : falscher Button");

    const SmSym *pSym = GetSymbol();
    if (pSym)
    {
        XubString   aText ('%');
        aText += pSym->GetName();

        SmViewShell *pViewSh = SmGetActiveView();
        if (pViewSh)
            pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                    SID_INSERTTEXT, SFX_CALLMODE_STANDARD,
                    new SfxStringItem(SID_INSERTTEXT, aText), 0L);
    }

    return 0;
}


IMPL_LINK_INLINE_START( SmSymbolDialog, CloseClickHdl, Button *, pButton )
{
    DBG_ASSERT(pButton == &aCloseBtn, "Sm : falscher Button");

    EndDialog(TRUE);
    return 0;
}
IMPL_LINK_INLINE_END( SmSymbolDialog, CloseClickHdl, Button *, pButton )


SmSymbolDialog::SmSymbolDialog(Window *pParent, SmSymSetManager &rMgr, BOOL bFreeRes) :
    ModalDialog         (pParent, SmResId(RID_SYMBOLDIALOG)),
    aSymbolSetText      (this, ResId(1)),
    aSymbolSets         (this, ResId(1)),
    aSymbolSetDisplay   (this, ResId(1)),
    aGrpBox             (this, ResId(1)),
    aSymbolName         (this, ResId(2)),
    aSymbolDisplay      (this, ResId(2)),
    aCloseBtn           (this, ResId(3)),
    aEditBtn            (this, ResId(1)),
    aGetBtn             (this, ResId(2)),
    rSymSetMgr          (rMgr)
{
    if (bFreeRes)
        FreeResource();

    pSymSet = NULL;
    FillSymbolSets();
    if (aSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aSymbolSets.GetEntry(0));

    // set background color to white
    Wallpaper aWhiteWall( (Color) Color(COL_WHITE) );
    aSymbolDisplay   .SetBackground( aWhiteWall );
    aSymbolSetDisplay.SetBackground( aWhiteWall );

    aSymbolSets      .SetSelectHdl  (LINK(this, SmSymbolDialog, SymbolSetChangeHdl));
    aSymbolSetDisplay.SetSelectHdl  (LINK(this, SmSymbolDialog, SymbolChangeHdl));
    aSymbolSetDisplay.SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    aSymbolDisplay   .SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    aCloseBtn        .SetClickHdl   (LINK(this, SmSymbolDialog, CloseClickHdl));
    aEditBtn         .SetClickHdl   (LINK(this, SmSymbolDialog, EditClickHdl));
    aGetBtn          .SetClickHdl   (LINK(this, SmSymbolDialog, GetClickHdl));
}


BOOL SmSymbolDialog::SelectSymbolSet(const XubString &rSymbolSetName)
{
    BOOL    bRet = FALSE;
    USHORT  nPos = aSymbolSets.GetEntryPos(rSymbolSetName);

    pSymSet = NULL;
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        aSymbolSets.SelectEntryPos(nPos);
        USHORT nSymbolSetNo = rSymSetMgr.GetSymbolSetPos(aSymbolSets.GetSelectEntry());
        pSymSet = rSymSetMgr.GetSymbolSet(nSymbolSetNo);
        DBG_ASSERT(pSymSet, "Sm : NULL pointer");

        aSymbolSetDisplay.SetSymbolSet(*pSymSet);
        if (pSymSet->GetCount() > 0)
            SelectSymbol(0);

        bRet = TRUE;
    }
    else
        aSymbolSets.SetNoSelection();

    return bRet;
}


void SmSymbolDialog::SelectSymbol(USHORT nSymbolNo)
{
    const SmSym *pSym = NULL;
    if (pSymSet  &&  nSymbolNo < pSymSet->GetCount())
        pSym = &pSymSet->GetSymbol(nSymbolNo);

    aSymbolSetDisplay.SelectSymbol(nSymbolNo);
    aSymbolDisplay.SetSymbol(pSym);
    aSymbolName.SetText(pSym ? pSym->GetName() : XubString());
}


const SmSym * SmSymbolDialog::GetSymbol() const
{
    USHORT nSymbolNo = aSymbolSetDisplay.GetSelectSymbol();
    return pSymSet == NULL ? NULL : &pSymSet->GetSymbol(nSymbolNo);
}


/**************************************************************************/


void SmShowCharset::Paint(const Rectangle&)
{
    Size OutputSize = GetOutputSizePixel();

    int i;
    for (i = 1; i < (int) nColumns; i++)
    {
        int nLenI = nLen * i;

        DrawLine(Point(nLenI, 0), Point(nLenI, OutputSize.Height()));
    }

    for (i = 1; i < (int) nRows; i++)
    {
        int nLenI = nLen * i;

        DrawLine(Point(0, nLenI), Point(OutputSize.Width(), nLenI));
    }

    for (i = 1; i < 256; i++)
    {
        int x = (i % nColumns) * nLen;
        int y = (i / nColumns) * nLen;

        Size aTextSize(GetTextWidth(aChar), GetTextHeight());

        XubString aChar((xub_Unicode) i);
        DrawText(Point(x + (nLen - aTextSize.Width())  / 2,
                       y + (nLen - aTextSize.Height()) / 2), aChar);
    }
}


void SmShowCharset::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        GrabFocus();

        USHORT n = (USHORT) ((rMEvt.GetPosPixel().Y() / nLen) * nColumns +
                             (rMEvt.GetPosPixel().X() / nLen));
        SelectChar((xub_Unicode)Min((USHORT) n, (USHORT) 255));

        aSelectHdlLink.Call(this);

        if (rMEvt.GetClicks() > 1)
            aDblClickHdlLink.Call(this);
    }
    else Control::MouseButtonDown (rMEvt);
}


void SmShowCharset::KeyInput(const KeyEvent& rKEvt)
{
    xub_Unicode n = aChar;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_DOWN:      n += (xub_Unicode) nColumns;    break;
        case KEY_UP:        n -= (xub_Unicode) nColumns;    break;
        case KEY_LEFT:      n -= 1;                 break;
        case KEY_RIGHT:     n += 1;                 break;
        case KEY_HOME:      n  = 0;                 break;
        case KEY_END:       n  = (xub_Unicode) 255;     break;
        case KEY_PAGEUP:    n -= (xub_Unicode) nColumns;    break;
        case KEY_PAGEDOWN:  n += (xub_Unicode) nColumns;    break;

        default:
            Control::KeyInput(rKEvt);
            return;
    }

    SelectChar(n);
    aSelectHdlLink.Call(this);
}


SmShowCharset::SmShowCharset(Window *pParent, const ResId& rResId) :
    Control(pParent, rResId)
{
    aChar = xub_Unicode('\0');

    Size    aOutputSize (GetOutputSizePixel());

    nColumns = 32;
    nRows    = 8;

    // FontSize passend wählen
    nLen = Min(aOutputSize.Width() / nColumns, aOutputSize.Height() / nRows);

    // Fenster genau passend machen (wird höchstens kleiner!)
    aOutputSize.Width()  = nColumns * nLen;
    aOutputSize.Height() = nRows * nLen;

    SetOutputSizePixel(aOutputSize);
}


void SmShowCharset::SetFont(const Font &rFont)
{
    Font  aFont (rFont);

    // etwas kleinere FontSize nehmen (als nLen) um etwas Luft zu haben
    // (hofentlich auch genug für links und rechts)
    aFont.SetSize(Size(0, nLen - (nLen / 3)));
    aFont.SetTransparent(TRUE);
    Control::SetFont(aFont);

    Invalidate();
}


void SmShowCharset::SelectChar(xub_Unicode aCharP)
{
    int c = aChar & 0xFF;
    Size aNSize (nLen, nLen);

    Invalidate(Rectangle(Point((c % nColumns) * nLen, (c / nColumns) * nLen), aNSize));

    aChar = aCharP;
    c     = aChar & 0xFF;
    Invalidate(Rectangle(Point((c % nColumns) * nLen, (c / nColumns) * nLen), aNSize));

    Update();
}


////////////////////////////////////////////////////////////////////////////////


void SmShowChar::Paint(const Rectangle&)
{
    XubString Text (GetText ());

    if (Text.Len() > 0)
    {
        Size aTextSize(GetTextWidth(Text), GetTextHeight());

        DrawText(Point((GetOutputSize().Width()  - aTextSize.Width())  / 2,
                       (GetOutputSize().Height() - aTextSize.Height()) / 2), Text);
    }
}


void SmShowChar::SetChar(xub_Unicode aChar)
{
    SetText(XubString(aChar));
    Invalidate();
}


void SmShowChar::SetFont(const Font &rFont)
{
    Font  aFont (rFont);
    Size  aSize (Size(0, GetOutputSize().Height() - GetOutputSize().Height() / 3));

    aFont.SetSize(aSize);
    aFont.SetTransparent(TRUE);
    Control::SetFont(aFont);

    Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void SmSymDefineDialog::FillSymbols(ComboBox &rComboBox, BOOL bDeleteText)
{
    DBG_ASSERT(&rComboBox == &aOldSymbols  ||  &rComboBox == &aSymbols,
        "Sm : falsche ComboBox");

    rComboBox.Clear();
    if (bDeleteText)
        rComboBox.SetText(XubString());

    ComboBox &rSymbolSets = &rComboBox == &aOldSymbols ?
                                    aOldSymbolSets : aSymbolSets;
    const SmSymSet *pSymSet = GetSymbolSet(rSymbolSets);
    if (pSymSet)
    {   USHORT nNumSymbols = pSymSet->GetCount();
        for (USHORT i = 0;  i < nNumSymbols;  i++)
            rComboBox.InsertEntry(pSymSet->GetSymbol(i).GetName());
    }

}


void SmSymDefineDialog::FillSymbolSets(ComboBox &rComboBox, BOOL bDeleteText)
{
    DBG_ASSERT(&rComboBox == &aOldSymbolSets  ||  &rComboBox == &aSymbolSets,
        "Sm : falsche ComboBox");

    rComboBox.Clear();
    if (bDeleteText)
        rComboBox.SetText(XubString());

    USHORT nNumSymSets = aSymSetMgrCopy.GetCount();
    for (USHORT i = 0;  i < nNumSymSets;  i++)
        rComboBox.InsertEntry(aSymSetMgrCopy.GetSymbolSet(i)->GetName());
}


void SmSymDefineDialog::FillFonts(BOOL bDelete)
{
    aFonts.Clear();
    if (bDelete)
        aFonts.SetNoSelection();

    // alle Fonts der 'FontList' in die Fontliste aufnehmen
    // von denen mit gleichen Namen jedoch nur einen (denn der Style wird
    // über die 'FontStyleBox' gewählt und nicht auch noch hier)
    DBG_ASSERT(pFontList, "Sm : NULL pointer");
    USHORT  nCount = pFontList->GetFontNameCount();
    for (USHORT i = 0;  i < nCount;  i++)
        aFonts.InsertEntry( pFontList->GetFontName(i).GetName() );
}


void SmSymDefineDialog::FillStyles(BOOL bDeleteText)
{
    aStyles.Clear();
    if (bDeleteText)
        aStyles.SetText(XubString());

    XubString aText (aFonts.GetSelectEntry());
    if (aText.Len() != 0)
    {
        //aStyles.Fill(aText, &aFontList);
        // eigene StyleName's verwenden
        for (USHORT i = 0;  i < aStyleNameCount;  i++)
            aStyles.InsertEntry( aStyleName[i] );

        DBG_ASSERT(aStyles.GetEntryCount() > 0, "Sm : keine Styles vorhanden");
        aStyles.SetText( aStyles.GetEntry(0) );
    }
}


SmSymSet * SmSymDefineDialog::GetSymbolSet(const ComboBox &rComboBox)
{
    DBG_ASSERT(&rComboBox == &aOldSymbolSets  ||  &rComboBox == &aSymbolSets,
        "Sm : falsche ComboBox");

    USHORT nSymbolSetNo = aSymSetMgrCopy.GetSymbolSetPos(rComboBox.GetText());

    return nSymbolSetNo == SYMBOLSET_NONE ?
        NULL : aSymSetMgrCopy.GetSymbolSet(nSymbolSetNo);
}


SmSym * SmSymDefineDialog::GetSymbol(const ComboBox &rComboBox)
{
    DBG_ASSERT(&rComboBox == &aOldSymbols  ||  &rComboBox == &aSymbols,
        "Sm : falsche ComboBox");

    return aSymSetMgrCopy.GetSymbol(rComboBox.GetText());
}


IMPL_LINK( SmSymDefineDialog, OldSymbolChangeHdl, ComboBox *, pComboBox )
{
    DBG_ASSERT(pComboBox == &aOldSymbols, "Sm : falsches Argument");
    SelectSymbol(aOldSymbols, aOldSymbols.GetText(), FALSE);
    return 0;
}


IMPL_LINK( SmSymDefineDialog, OldSymbolSetChangeHdl, ComboBox *, pComboBox )
{
    DBG_ASSERT(pComboBox == &aOldSymbolSets, "Sm : falsches Argument");
    SelectSymbolSet(aOldSymbolSets, aOldSymbolSets.GetText(), FALSE);
    return 0;
}


IMPL_LINK( SmSymDefineDialog, ModifyHdl, ComboBox *, pComboBox )
{
    // merken der Cursorposition zum wiederherstellen derselben
    Selection  aSelection (pComboBox->GetSelection());

    if (pComboBox == &aSymbols)
        SelectSymbol(aSymbols, aSymbols.GetText(), FALSE);
    else if (pComboBox == &aSymbolSets)
        SelectSymbolSet(aSymbolSets, aSymbolSets.GetText(), FALSE);
    else if (pComboBox == &aOldSymbols)
        // nur Namen aus der Liste erlauben
        SelectSymbol(aOldSymbols, aOldSymbols.GetText(), TRUE);
    else if (pComboBox == &aOldSymbolSets)
        // nur Namen aus der Liste erlauben
        SelectSymbolSet(aOldSymbolSets, aOldSymbolSets.GetText(), TRUE);
    else if (pComboBox == &aStyles)
        // nur Namen aus der Liste erlauben (ist hier eh immer der Fall)
        SelectStyle(aStyles.GetText(), TRUE);
    else
        DBG_ASSERT(0, "Sm : falsche ComboBox Argument");

    pComboBox->SetSelection(aSelection);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, FontChangeHdl, ListBox *, pListBox )
{
    DBG_ASSERT(pListBox == &aFonts, "Sm : falsches Argument");

    SelectFont(aFonts.GetSelectEntry());
    return 0;
}


IMPL_LINK( SmSymDefineDialog, StyleChangeHdl, ComboBox *, pComboBox )
{
    DBG_ASSERT(pComboBox == &aStyles, "Sm : falsches Argument");

    SelectStyle(aStyles.GetText());
    return 0;
}


IMPL_LINK_INLINE_START( SmSymDefineDialog, CharSelectHdl, SmShowCharset *, pShowCharset )
{
    DBG_ASSERT(pShowCharset == &aCharsetDisplay, "Sm : falsches Argument");

    SelectChar(aCharsetDisplay.GetSelectChar());
    return 0;
}
IMPL_LINK_INLINE_END( SmSymDefineDialog, CharSelectHdl, SmShowCharset *, pShowCharset )


IMPL_LINK( SmSymDefineDialog, AddClickHdl, Button *, pButton )
{
    DBG_ASSERT(pButton == &aAddBtn, "Sm : falsches Argument");
    DBG_ASSERT(aAddBtn.IsEnabled(), "Sm : Voraussetzungen erfüllt ??");

    SmSymSet *pSymSet = GetSymbolSet(aSymbolSets);

    // SymbolSet einfügen falls es noch nicht existiert
    if (!pSymSet)
    {
        pSymSet = new SmSymSet(aSymbolSets.GetText());
        aSymSetMgrCopy.AddSymbolSet(pSymSet);
        FillSymbolSets(aOldSymbolSets, FALSE);
        FillSymbolSets(aSymbolSets,    FALSE);
    }
    DBG_ASSERT(pSymSet, "Sm : NULL pointer");

    // Symbol ins SymbolSet einfügen
    SmSym *pSym = new SmSym(aSymbols.GetText(), aCharsetDisplay.GetFont(),
                            aCharsetDisplay.GetSelectChar());
    pSymSet->AddSymbol(pSym);

    // update der Hash Tabelle erzwingen (damit aAddBtn disabled wird).
    // (wird später nach Überarbeitung von symbol.cxx überflüssig werden).
    aSymSetMgrCopy.ChangeSymbolSet((SmSymSet *)1);

    // Symbolliste aktualiseren
    FillSymbols(aOldSymbols ,FALSE);
    FillSymbols(aSymbols    ,FALSE);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, ChangeClickHdl, Button *, pButton )
{
    DBG_ASSERT(pButton == &aChangeBtn, "Sm : falsches Argument");
    DBG_ASSERT(aChangeBtn.IsEnabled(), "Sm : Voraussetzungen erfüllt ??");

    // finden des SymbolSets zum alten Symbol
    SmSymSet *pOldSymSet = GetSymbolSet(aOldSymbolSets);
    DBG_ASSERT(pOldSymSet, "Sm : NULL pointer");

    // suchen des neuen SymbolSets
    SmSymSet *pNewSymSet = GetSymbolSet(aSymbolSets);
    // SymbolSet einfügen falls es noch nicht existiert
    if (!pNewSymSet)
    {
        pNewSymSet = new SmSymSet(aSymbolSets.GetText());
        aSymSetMgrCopy.AddSymbolSet(pNewSymSet);
        FillSymbolSets(aOldSymbolSets, FALSE);
        FillSymbolSets(aSymbolSets,    FALSE);
    }

    // das (alte) Symbol besorgen
    USHORT nSymbol = pOldSymSet->GetSymbolPos(aOldSymbols.GetText());
    SmSym *pSym    = (SmSym *) &pOldSymSet->GetSymbol(nSymbol);
    DBG_ASSERT(pSym, "Sm : NULL pointer");

    // Änderungen durchführen;
    pSym->GetName() = aSymbols.GetText();
    pSym->GetFace() = aCharsetDisplay.GetFont();
    pSym->GetCharacter() = aCharsetDisplay.GetSelectChar();

    // das SymbolSet wechseln wenn nötig
    if (pOldSymSet != pNewSymSet)
    {
        pOldSymSet->RemoveSymbol(nSymbol);
        pNewSymSet->AddSymbol(pSym);
    }

    //!! den SymbolSet Manger dazu zwingen seinen HashTable zu aktualisieren,
    //!! um mit möglich neuen bzw fehlenden alten Symbol Namen konform zu sein.
    aSymSetMgrCopy.ChangeSymbolSet((SmSymSet *)1);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, DeleteClickHdl, Button *, pButton )
{
    DBG_ASSERT(pButton == &aDeleteBtn, "Sm : falsches Argument");
    DBG_ASSERT(aDeleteBtn.IsEnabled(), "Sm : Voraussetzungen erfüllt ??");

    if (pOrigSymbol)
    {
        // löschen des Symbols:
        //
        // zugehöriges SymbolSet finden
        SmSymSet *pSymSet = GetSymbolSet(aOldSymbolSets);
        DBG_ASSERT(pSymSet, "Sm : NULL pointer");
        // finden des Index
        XubString  aOldSymbolName (pOrigSymbol->GetName());
        USHORT    nSymbolNo   = pSymSet->GetSymbolPos(aOldSymbolName);
        DBG_ASSERT(nSymbolNo != SYMBOL_NONE, "Sm : kein Symbol");
        // Bezüge auf das Symbols löschen
        DBG_ASSERT(pOrigSymbol == &pSymSet->GetSymbol(nSymbolNo),
            "Sm : Fehler beim löschen des Symbols");
        SetOrigSymbol(NULL, XubString());
        // und weg mit dem Symbol
        pSymSet->DeleteSymbol(nSymbolNo);

        //!! den SymbolSet Manger dazu zwingen seinen HashTable zu aktualisieren,
        //!! was er nämlich nicht tut, wenn in einem seiner SymbolSets geändert/
        //!! gelöscht wird, was wiederum zu einem Absturz führen kann (wenn er
        //!! über ein nicht mehr existentes aber nicht entferntes Symbol iteriert).
        aSymSetMgrCopy.ChangeSymbolSet((SmSymSet *)1);

        // aktualisieren der Symboleinträge des Dialogs
        aOldSymbols.SetText(XubString());
        aOldSymbols.RemoveEntry(aOldSymbolName);
        if (aSymbolSets.GetText() == aOldSymbolSets.GetText())
            aSymbols.RemoveEntry(aOldSymbolName);
    }

    UpdateButtons();

    return 0;
}


void SmSymDefineDialog::UpdateButtons()
{
    BOOL  bAdd    = FALSE,
          bChange = FALSE,
          bDelete = FALSE,
          bEqual;
    XubString  aSymbolName    (aSymbols.GetText()),
              aSymbolSetName (aSymbolSets.GetText());

    if (aSymbolName.Len() > 0  &&  aSymbolSetName.Len() > 0)
    {
        // alle Einstellungen gleich?
        //! (Font-, Style- und SymbolSet Name werden nicht case sensitiv verglichen)
        bEqual = pOrigSymbol
                    && aSymbolSetName.EqualsIgnoreCaseAscii(aOldSymbolSetName.GetText())
                    && aSymbolName.Equals(pOrigSymbol->GetName())
                    && aFonts.GetSelectEntry().EqualsIgnoreCaseAscii(pOrigSymbol->GetFace().GetName())
                    && aStyles.GetText().EqualsIgnoreCaseAscii(GetFontStyleName(pOrigSymbol->GetFace()))
                    && aCharsetDisplay.GetSelectChar() == pOrigSymbol->GetCharacter();

        // hinzufügen nur wenn es noch kein Symbol desgleichen Namens gibt
        bAdd    = aSymSetMgrCopy.GetSymbol(aSymbolName) == NULL;

        // löschen nur wenn alle Einstellungen gleich sind
        bDelete = pOrigSymbol != NULL;

        // ändern wenn bei gleichem Namen mindestens eine Einstellung anders ist
        // oder wenn es noch kein Symbol des neuen Namens gibt (würde implizites
        // löschen des bereits vorhandenen Symbols erfordern)
        BOOL  bEqualName = pOrigSymbol && aSymbolName == pOrigSymbol->GetName();
        bChange = pOrigSymbol && (bEqualName && !bEqual || !bEqualName && bAdd);
    }

    aAddBtn   .Enable(bAdd);
    aChangeBtn.Enable(bChange);
    aDeleteBtn.Enable(bDelete);
}


SmSymDefineDialog::SmSymDefineDialog(Window * pParent, SmSymSetManager &rMgr, BOOL bFreeRes) :
    ModalDialog         (pParent, SmResId(RID_SYMDEFINEDIALOG)),
    aOldSymbolText      (this, ResId(1)),
    aOldSymbols         (this, ResId(1)),
    aOldSymbolSetText   (this, ResId(2)),
    aOldSymbolSets      (this, ResId(2)),
    aCharsetDisplay     (this, ResId(1)),
    aSymbolText         (this, ResId(9)),
    aSymbols            (this, ResId(4)),
    aSymbolSetText      (this, ResId(10)),
    aSymbolSets         (this, ResId(5)),
    aFontText           (this, ResId(3)),
    aFonts              (this, ResId(1)),
    aStyleText          (this, ResId(4)),
    aStyles             (this, ResId(3)),
    aOldSymbolName      (this, ResId(7)),
    aOldSymbolDisplay   (this, ResId(3)),
    aOldSymbolSetName   (this, ResId(8)),
    aSymbolName         (this, ResId(5)),
    aSymbolDisplay      (this, ResId(2)),
    aSymbolSetName      (this, ResId(6)),
    aAddBtn             (this, ResId(1)),
    aChangeBtn          (this, ResId(2)),
    aDeleteBtn          (this, ResId(3)),
    aOkBtn              (this, ResId(1)),
    aCancelBtn          (this, ResId(1)),
    aRightArrow         (this, ResId(1)),
    pFontList           (NULL),
    rSymSetMgr          (rMgr)
{
    if (bFreeRes)
        FreeResource();

    // get FontList from printer (if possible), otherwise from application window
    SmViewShell *pView = SmGetActiveView();
    DBG_ASSERT(pView, "Sm : NULL pointer");
    OutputDevice *pDev = pView->GetDoc()->GetPrinter();
    if (!pDev || pDev->GetDevFontCount() == 0)
        pDev = &pView->GetGraphicWindow();
    pFontList = new FontList(pDev);


    pOrigSymbol = 0;

    // make autocompletion for symbols case-sensitive
    aOldSymbols.EnableAutocomplete(TRUE, TRUE);
    aSymbols   .EnableAutocomplete(TRUE, TRUE);

    FillFonts();
    if (aFonts.GetEntryCount() > 0)
        SelectFont(aFonts.GetEntry(0));

    // set background color to white
    Wallpaper aWhiteWall( (Color) Color(COL_WHITE) );
    aCharsetDisplay  .SetBackground( aWhiteWall );
    aOldSymbolDisplay.SetBackground( aWhiteWall );
    aOldSymbolDisplay.SetBackground( aWhiteWall );

    SetSymbolSetManager(rSymSetMgr);

    aOldSymbols    .SetSelectHdl(LINK(this, SmSymDefineDialog, OldSymbolChangeHdl));
    aOldSymbolSets .SetSelectHdl(LINK(this, SmSymDefineDialog, OldSymbolSetChangeHdl));
    aSymbolSets    .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aOldSymbolSets .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aSymbols       .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aOldSymbols    .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aStyles        .SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    aFonts         .SetSelectHdl(LINK(this, SmSymDefineDialog, FontChangeHdl));
    aStyles        .SetSelectHdl(LINK(this, SmSymDefineDialog, StyleChangeHdl));
    aAddBtn        .SetClickHdl (LINK(this, SmSymDefineDialog, AddClickHdl));
    aChangeBtn     .SetClickHdl (LINK(this, SmSymDefineDialog, ChangeClickHdl));
    aDeleteBtn     .SetClickHdl (LINK(this, SmSymDefineDialog, DeleteClickHdl));
    aCharsetDisplay.SetSelectHdl(LINK(this, SmSymDefineDialog, CharSelectHdl));
}


SmSymDefineDialog::~SmSymDefineDialog()
{
    delete pFontList;
}


short SmSymDefineDialog::Execute()
{
    short nResult = ModalDialog::Execute();

    // Änderungen übernehmen falls Dialog mit OK beendet wurde
    if (aSymSetMgrCopy.IsModified()  &&  nResult == RET_OK)
    {
        // leere SymbolSets aus dem Ergebnis entfernen.
        // Dabei von hinten durch das array iterieren, da beim löschen die
        // Elemente aufrücken.
        USHORT  nSymbolSets = aSymSetMgrCopy.GetCount();
        for (int i = nSymbolSets - 1;  i >= 0;  i--)
            if (aSymSetMgrCopy.GetSymbolSet(i)->GetCount() == 0)
                aSymSetMgrCopy.DeleteSymbolSet(i);

        rSymSetMgr = aSymSetMgrCopy;
    }

    return nResult;
}


void SmSymDefineDialog::SetSymbolSetManager(const SmSymSetManager &rMgr)
{
    aSymSetMgrCopy = rMgr;

    // Das modified Flag der Kopie auf FALSE setzen, damit man später damit
    // testen kann ob sich was geändert hat.
    aSymSetMgrCopy.SetModified(FALSE);

    FillSymbolSets(aOldSymbolSets);
    if (aOldSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aOldSymbolSets.GetEntry(0));
    FillSymbolSets(aSymbolSets);
    if (aSymbolSets.GetEntryCount() > 0)
        SelectSymbolSet(aSymbolSets.GetEntry(0));
    FillSymbols(aOldSymbols);
    if (aOldSymbols.GetEntryCount() > 0)
        SelectSymbol(aOldSymbols.GetEntry(0));
    FillSymbols(aSymbols);
    if (aSymbols.GetEntryCount() > 0)
        SelectSymbol(aSymbols.GetEntry(0));

    UpdateButtons();
}


BOOL SmSymDefineDialog::SelectSymbolSet(ComboBox &rComboBox,
        const XubString &rSymbolSetName, BOOL bDeleteText)
{
    DBG_ASSERT(&rComboBox == &aOldSymbolSets  ||  &rComboBox == &aSymbolSets,
        "Sm : falsche ComboBox");

    // 'Normalisieren' des SymbolNamens (ohne leading und trailing Leerzeichen)
    XubString  aNormName (rSymbolSetName);
    aNormName.EraseLeadingChars(' ');
    aNormName.EraseTrailingChars(' ');
    // und evtl Abweichungen in der Eingabe beseitigen
    rComboBox.SetText(aNormName);

    BOOL   bRet = FALSE;
    USHORT nPos = rComboBox.GetEntryPos(aNormName);

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        rComboBox.SetText(rComboBox.GetEntry(nPos));
        bRet = TRUE;
    }
    else if (bDeleteText)
        rComboBox.SetText(XubString());

    BOOL  bIsOld = &rComboBox == &aOldSymbolSets;

    // setzen des SymbolSet Namens an der zugehörigen Darstellung
    FixedText &rFT = bIsOld ? aOldSymbolSetName : aSymbolSetName;
    rFT.SetText(rComboBox.GetText());

    // setzen der zum SymbolSet gehörenden Symbol Namen an der zugehörigen
    // Auswahbox
    ComboBox  &rCB = bIsOld ? aOldSymbols : aSymbols;
    FillSymbols(rCB, FALSE);

    // bei Wechsel des SymbolSets für das alte Zeichen ein gültiges
    // Symbol bzw keins zur Anzeige bringen
    if (bIsOld)
    {
        XubString  aOldSymbolName;
        if (aOldSymbols.GetEntryCount() > 0)
            aOldSymbolName = aOldSymbols.GetEntry(0);
        SelectSymbol(aOldSymbols, aOldSymbolName, TRUE);
    }

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetOrigSymbol(const SmSym *pSymbol,
                                      const XubString &rSymbolSetName)
{
    pOrigSymbol = pSymbol;

    XubString  aSymName,
              aSymSetName;
    if (pSymbol)
    {
        aSymName    = pSymbol->GetName();
        aSymSetName = rSymbolSetName;
        aOldSymbolDisplay.SetFont(pSymbol->GetFace());
        aOldSymbolDisplay.SetChar(pSymbol->GetCharacter());
    }
    else
    {   // löschen des angezeigten Symbols
        aOldSymbolDisplay.SetText(XubString());
        aOldSymbolDisplay.Invalidate();
    }
    aOldSymbolName   .SetText(aSymName);
    aOldSymbolSetName.SetText(aSymSetName);
}


BOOL SmSymDefineDialog::SelectSymbol(ComboBox &rComboBox,
        const XubString &rSymbolName, BOOL bDeleteText)
{
    DBG_ASSERT(&rComboBox == &aOldSymbols  ||  &rComboBox == &aSymbols,
        "Sm : falsche ComboBox");

    // 'Normalisieren' des SymbolNamens (ohne Leerzeichen)
    XubString  aNormName (rSymbolName);
    aNormName.EraseAllChars(' ');
    // und evtl Abweichungen in der Eingabe beseitigen
    rComboBox.SetText(aNormName);

    BOOL   bRet = FALSE;
    USHORT nPos = rComboBox.GetEntryPos(aNormName);

    BOOL  bIsOld = &rComboBox == &aOldSymbols;

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        rComboBox.SetText(rComboBox.GetEntry(nPos));

        if (!bIsOld)
        {
            const SmSym *pSymbol = GetSymbol(aSymbols);
            if (pSymbol)
            {
                // Font und Style entsprechend wählen
                const Font &rFont = pSymbol->GetFace();
                SelectFont(rFont.GetName(), FALSE);
                SelectStyle(GetFontStyleName(rFont), FALSE);

                // da das setzen des Fonts über den Style Namen des SymbolsFonts nicht
                // so gut klappt (er kann zB leer sein obwohl der Font selbst 'bold' und
                // 'italic' ist!). Setzen wir hier den Font wie er zum Symbol gehört
                // zu Fuß.
                aCharsetDisplay.SetFont(rFont);
                aSymbolDisplay.SetFont(rFont);

                // das zugehörige Zeichen auswählen
                SelectChar(pSymbol->GetCharacter());
            }
        }

        bRet = TRUE;
    }
    else if (bDeleteText)
        rComboBox.SetText(XubString());

    if (bIsOld)
    {
        // bei Wechsel des alten Symbols nur vorhandene anzeigen sonst keins
        const SmSym *pOldSymbol = NULL;
        XubString     aOldSymbolSetName;
        if (nPos != COMBOBOX_ENTRY_NOTFOUND)
        {
            pOldSymbol        = aSymSetMgrCopy.GetSymbol(aNormName);
            aOldSymbolSetName = aOldSymbolSets.GetText();
        }
        SetOrigSymbol(pOldSymbol, aOldSymbolSetName);
    }
    else
        aSymbolName.SetText(rComboBox.GetText());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetFont(const XubString &rFontName, const XubString &rStyleName)
{
    // Font (FontInfo) passend zu Namen und Style holen
    DBG_ASSERT(pFontList, "Sm : NULL pointer");
    FontInfo  aFI( pFontList->Get(rFontName, WEIGHT_NORMAL, ITALIC_NONE) );
    SetFontStyle(rStyleName, aFI);

    aCharsetDisplay.SetFont(aFI);
    aSymbolDisplay.SetFont(aFI);
}


BOOL SmSymDefineDialog::SelectFont(const XubString &rFontName, BOOL bApplyFont)
{
    BOOL   bRet = FALSE;
    USHORT nPos = aFonts.GetEntryPos(rFontName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        aFonts.SelectEntryPos(nPos);
        if (aStyles.GetEntryCount() > 0)
            SelectStyle(aStyles.GetEntry(0));
        if (bApplyFont)
            SetFont(aFonts.GetSelectEntry(), aStyles.GetText());
        bRet = TRUE;
    }
    else
        aFonts.SetNoSelection();
    FillStyles();

    UpdateButtons();

    return bRet;
}


BOOL SmSymDefineDialog::SelectStyle(const XubString &rStyleName, BOOL bApplyFont)
{
    BOOL   bRet = FALSE;
    USHORT nPos = aStyles.GetEntryPos(rStyleName);

    // falls der Style nicht zur Auswahl steht nehmen wir den erst möglichen
    // (sofern vorhanden)
    if (nPos == COMBOBOX_ENTRY_NOTFOUND  &&  aStyles.GetEntryCount() > 0)
        nPos = 0;

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        aStyles.SetText(aStyles.GetEntry(nPos));
        if (bApplyFont)
            SetFont(aFonts.GetSelectEntry(), aStyles.GetText());
        bRet = TRUE;
    }
    else
        aStyles.SetText(XubString());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SelectChar(xub_Unicode cChar)
{
    aCharsetDisplay.SelectChar(cChar);
    aSymbolDisplay.SetChar(cChar);

    UpdateButtons();
}


/**************************************************************************/

#ifdef NEVER
SfxTabPage* SmGeneralTabPage::Create(Window* pWindow, const SfxItemSet& rSet)
{
    return new SmGeneralTabPage(pWindow, rSet);
}
#endif NEVER



