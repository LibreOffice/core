/*************************************************************************
 *
 *  $RCSfile: drpcps.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fme $ $Date: 2002-12-10 09:41:42 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#include <svtools/svstdarr.hxx>

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"


#ifndef _METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#include "charatr.hxx"
#include "viewopt.hxx"
#include "drpcps.hxx"
#include "paratr.hxx"
#include "uitool.hxx"
#include "charfmt.hxx"

#include "chrdlg.hrc"
#include "drpcps.hrc"

// Globals ******************************************************************

static USHORT __FAR_DATA aPageRg[] = {
    RES_PARATR_DROP, RES_PARATR_DROP,
    0
};



// class SwDropCapsPict *****************************************************

class SwDropCapsPict : public Control
{
    String  maText;
    Color   maBackColor;
    Color   maTextLineColor;
    BYTE    mnLines;
    long    mnTotLineH;
    long    mnLineH;
    long    mnTextH;
    USHORT  mnDistance;
    USHORT  mnLeading;

    virtual void Paint(const Rectangle &rRect);
public:

     SwDropCapsPict(Window *pParent, const ResId &rResId) : Control(pParent, rResId) {}
    ~SwDropCapsPict();

    void UpdatePaintSettings( void );       // also invalidates control!

    inline void SetText( const String& rT );
    inline void SetLines( BYTE nL );
    inline void SetDistance( USHORT nD );
    inline void SetValues( const String& rText, BYTE nLines, USHORT nDistance );
};

inline void SwDropCapsPict::SetText( const String& rT )
{
    maText = rT;
    UpdatePaintSettings();
}

inline void SwDropCapsPict::SetLines( BYTE nL )
{
    mnLines = nL;
    UpdatePaintSettings();
}

inline void SwDropCapsPict::SetDistance( USHORT nD )
{
    mnDistance = nD;
    UpdatePaintSettings();
}

inline void SwDropCapsPict::SetValues( const String& rText, BYTE nLines, USHORT nDistance )
{
    maText = rText;
    mnLines = nLines;
    mnDistance = nDistance;

    UpdatePaintSettings();
}

/****************************************************************************
Default-String aus Zeichenanzahl erzeugen (A, AB, ABC, ...)
****************************************************************************/


String GetDefaultString(USHORT nChars)
{
    String aStr;
    for (USHORT i = 0; i < nChars; i++)
        aStr += String((char) (i + 65));
    return aStr;
}

/****************************************************************************
Pict: Dtor
****************************************************************************/


 SwDropCapsPict::~SwDropCapsPict()
{
}

/****************************************************************************
Pict: Update Font
****************************************************************************/

#define LINES  10
#define BORDER  2


void SwDropCapsPict::UpdatePaintSettings( void )
{
    maBackColor = GetSettings().GetStyleSettings().GetWindowColor();
    maTextLineColor = Color( COL_LIGHTGRAY );

    // gray lines
    mnTotLineH = (GetOutputSizePixel().Height() - 2 * BORDER) / LINES;
    mnLineH = mnTotLineH - 2;
    mnLeading = (USHORT) GetFontMetric().GetLeading();

    Font aFont;
    {
        SwDropCapsPage* pPage = ( SwDropCapsPage* ) GetParent();
        if (!pPage->aTemplateBox.GetSelectEntryPos())
        {
            // Font an Absatzanfang erfragen
            pPage->rSh.SttCrsrMove();
            pPage->rSh.Push();
            pPage->rSh.ClearMark();
            pPage->rSh.MovePara(fnParaCurr,fnParaStart);

            SfxItemSet aSet(pPage->rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT);
            pPage->rSh.GetAttr(aSet);
            SvxFontItem aFmtFont((SvxFontItem &) aSet.Get(RES_CHRATR_FONT));

            aFont.SetFamily (aFmtFont.GetFamily());
            aFont.SetName   (aFmtFont.GetFamilyName());
            aFont.SetPitch  (aFmtFont.GetPitch());
            aFont.SetCharSet(aFmtFont.GetCharSet());

            pPage->rSh.Pop(FALSE);
            pPage->rSh.EndCrsrMove();
        }
        else
        {
            // Font an Zeichenvorlage erfragen
            SwCharFmt *pFmt = pPage->rSh.GetCharStyle(
                                    pPage->aTemplateBox.GetSelectEntry(),
                                    SwWrtShell::GETSTYLE_CREATEANY );
            ASSERT(pFmt, "Zeichenvorlage existiert nicht!");
            const SvxFontItem &rFmtFont = pFmt->GetFont();

            aFont.SetFamily (rFmtFont.GetFamily());
            aFont.SetName   (rFmtFont.GetFamilyName());
            aFont.SetPitch  (rFmtFont.GetPitch());
            aFont.SetCharSet(rFmtFont.GetCharSet());
        }
    }

    mnTextH = mnLines * mnTotLineH;
    aFont.SetSize(Size(0, mnTextH));
    aFont.SetTransparent(TRUE);
    aFont.SetColor( SwViewOption::GetFontColor() );
    aFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    SetFont(aFont);
    aFont.SetSize(Size(0, aFont.GetSize().Height() + mnLeading));
    SetFont(aFont);

    Invalidate();
}

/****************************************************************************
Pict: Paint-Overload
****************************************************************************/

void  SwDropCapsPict::Paint(const Rectangle &rRect)
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

    ASSERT(mnLineH > 0, "So klein lassen wir uns nicht machen");
    long nY0 = (aOutputSizePixel.Height() - (LINES * mnTotLineH)) / 2;
    SetFillColor( maTextLineColor );
    for (USHORT i = 0; i < LINES; ++i)
        DrawRect(Rectangle(Point(BORDER, nY0 + i * mnTotLineH), Size(aOutputSizePixel.Width() - 2 * BORDER, mnLineH)));

    // Texthintergrund mit Abstand (240 twips ~ 1 Zeilenhoehe)
    ULONG lDistance = mnDistance;
    USHORT nDistW = (USHORT) (ULONG) (((lDistance * 100) / 240) * mnTotLineH) / 100;
    SetFillColor( maBackColor );
    if(((SwDropCapsPage*)GetParent())->aDropCapsBox.IsChecked())
    {
        DrawRect(Rectangle(
        Point(BORDER, nY0),
        Size (GetTextWidth(maText) + nDistW, mnTextH)));

    // Text zeichnen
        DrawText(Point(BORDER, nY0 - mnLeading), maText);
    }

    SetClipRegion();
}

/****************************************************************************
Dlg: Ctor
****************************************************************************/


SwDropCapsDlg::SwDropCapsDlg(Window *pParent, const SfxItemSet &rSet ) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    SwDropCapsPage* pPage = (SwDropCapsPage*) SwDropCapsPage::Create(this, rSet);
    pPage->SetFormat(FALSE);
    SetTabPage(pPage);
}

/****************************************************************************
Dlg: Dtor
****************************************************************************/


 SwDropCapsDlg::~SwDropCapsDlg()
{
}

/****************************************************************************
Page: Ctor
****************************************************************************/


SwDropCapsPage::SwDropCapsPage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage(pParent, SW_RES(TP_DROPCAPS), rSet),

    aDropCapsBox  (this, SW_RES(CB_SWITCH   )),
    aWholeWordCB  (this, SW_RES(CB_WORD     )),
    aSwitchText   (this, SW_RES(FT_DROPCAPS )),
    aDropCapsField(this, SW_RES(FLD_DROPCAPS)),
    aLinesText    (this, SW_RES(TXT_LINES   )),
    aLinesField   (this, SW_RES(FLD_LINES   )),
    aDistanceText (this, SW_RES(TXT_DISTANCE)),
    aDistanceField(this, SW_RES(FLD_DISTANCE)),
    aTextText     (this, SW_RES(TXT_TEXT    )),
    aTextEdit     (this, SW_RES(EDT_TEXT    )),
    aTemplateText (this, SW_RES(TXT_TEMPLATE)),
    aTemplateBox  (this, SW_RES(BOX_TEMPLATE)),
    aSettingsFL   (this, SW_RES(FL_SETTINGS)),
    aContentFL    (this, SW_RES(FL_CONTENT )),
    pPict         (new SwDropCapsPict(this, SW_RES(CNT_PICT))),
    bModified(FALSE),
    bFormat(TRUE),
    rSh(::GetActiveView()->GetWrtShell())
{
    FreeResource();
    SetExchangeSupport();

    USHORT nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? TRUE : FALSE;

    //Im Vorlagendialog kann der Text nicht beeinflusst werden
    aTextText.Enable( !bFormat );
    aTextEdit.Enable( !bFormat );

    // Metriken
    SetMetric( aDistanceField, GetDfltMetric(bHtmlMode) );

    pPict->SetBorderStyle( WINDOW_BORDER_MONO );

    // Handler installieren
    Link aLk = LINK(this, SwDropCapsPage, ModifyHdl);
    aDropCapsField.SetModifyHdl( aLk );
    aLinesField   .SetModifyHdl( aLk );
    aDistanceField.SetModifyHdl( aLk );
    aTextEdit     .SetModifyHdl( aLk );
    aDropCapsBox  .SetClickHdl (LINK(this, SwDropCapsPage, ClickHdl ));
    aTemplateBox  .SetSelectHdl(LINK(this, SwDropCapsPage, SelectHdl));
    aWholeWordCB  .SetClickHdl (LINK(this, SwDropCapsPage, WholeWordHdl ));
}

/****************************************************************************
Page: Dtor
****************************************************************************/


 SwDropCapsPage::~SwDropCapsPage()
{
    delete pPict;
}


int  SwDropCapsPage::DeactivatePage(SfxItemSet * pSet)
{
    if ( pSet )
        FillSet( *pSet );

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


BOOL  SwDropCapsPage::FillItemSet(SfxItemSet &rSet)
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
    // Zeichen, Zeilen, Abstand und Text
    SwFmtDrop aFmtDrop((SwFmtDrop &) rSet.Get(RES_PARATR_DROP));
    if (aFmtDrop.GetLines() > 1)
    {
        aDropCapsField.SetValue(aFmtDrop.GetChars());
        aLinesField   .SetValue(aFmtDrop.GetLines());
        aDistanceField.SetValue(aDistanceField.Normalize(aFmtDrop.GetDistance()), FUNIT_TWIP);
        aWholeWordCB  .Check   (aFmtDrop.GetWholeWord());
    }
    else
    {
        aDropCapsField.SetValue(1);
        aLinesField   .SetValue(3);
        aDistanceField.SetValue(0);
    }

    ::FillCharStyleListBox(aTemplateBox, rSh.GetView().GetDocShell(), TRUE);

    aTemplateBox.InsertEntry(SW_RESSTR(SW_STR_NONE), 0);
    // Vorlage defaulten
    aTemplateBox.SelectEntryPos(0);
    if (aFmtDrop.GetCharFmt())
        aTemplateBox.SelectEntry(aFmtDrop.GetCharFmt()->GetName());

    // Controls enablen
    aDropCapsBox.Check(aFmtDrop.GetLines() > 1);
    const USHORT nVal = USHORT(aDropCapsField.GetValue());
    if (bFormat)
        aTextEdit.SetText(GetDefaultString(nVal));
    else
    {
        aTextEdit.SetText(rSh.GetDropTxt(nVal));
        aTextEdit.Enable();
        aTextText.Enable();
    }

    // Preview
    pPict->SetValues(   aTextEdit.GetText(),
                        BYTE( aLinesField.GetValue() ),
                        USHORT( aDistanceField.Denormalize( aDistanceField.GetValue( FUNIT_TWIP ) ) ) );

    ClickHdl(&aDropCapsBox);
    bModified = FALSE;
}

/****************************************************************************
Page: Click-Handler der CheckBox
****************************************************************************/


IMPL_LINK( SwDropCapsPage, ClickHdl, Button *, EMPTYARG )
{
    BOOL bChecked = aDropCapsBox.IsChecked();

    aWholeWordCB  .Enable( bChecked && !bHtmlMode );

    aSwitchText.Enable( bChecked && !aWholeWordCB.IsChecked() );
    aDropCapsField.Enable( bChecked && !aWholeWordCB.IsChecked() );
    aLinesText   .Enable( bChecked );
    aLinesField   .Enable( bChecked );
    aDistanceText.Enable( bChecked );
    aDistanceField.Enable( bChecked );
    aTemplateText .Enable( bChecked );
    aTemplateBox  .Enable( bChecked );
    aTextEdit     .Enable( bChecked && !bFormat );
    aTextText     .Enable( bChecked && !bFormat );

    if ( bChecked )
    {
        ModifyHdl(&aDropCapsField);
        aDropCapsField.GrabFocus();
    }
    else
        pPict->SetText(aEmptyStr);

    bModified = TRUE;

    return 0;
}

/****************************************************************************
Page: Click-Handler der CheckBox
****************************************************************************/


IMPL_LINK( SwDropCapsPage, WholeWordHdl, CheckBox *, EMPTYARG )
{
    aDropCapsField.Enable( !aWholeWordCB.IsChecked() );

    ModifyHdl(&aDropCapsField);

    bModified = TRUE;

    return 0;
}

/****************************************************************************
Page: Modify-Handler der SpinFields
****************************************************************************/


IMPL_LINK( SwDropCapsPage, ModifyHdl, Edit *, pEdit )
{
    String sPreview;

    // Ggf. Text setzen
    if (pEdit == &aDropCapsField)
    {
        USHORT nVal;
        BOOL bSetText = FALSE;

        if (!aWholeWordCB.IsChecked())
            nVal = (USHORT)aDropCapsField.GetValue();
        else
            nVal = 0;

        if (bFormat || !rSh.GetDropTxt(1).Len())
            sPreview = GetDefaultString(nVal);
        else
        {
            bSetText = TRUE;
            sPreview = rSh.GetDropTxt(nVal);
        }

        String sEdit(aTextEdit.GetText());

        if (sEdit.Len() && sPreview.CompareTo(sEdit, sEdit.Len()) != COMPARE_EQUAL)
        {
            sPreview = sEdit.Copy(0, sPreview.Len());
            bSetText = FALSE;
        }

        if (bSetText)
            aTextEdit.SetText(sPreview);
    }
    else if (pEdit == &aTextEdit)   // Ggf. Anzahl setzen
    {
        USHORT nTmp = aTextEdit.GetText().Len();
        aDropCapsField.SetValue(Max((USHORT)1, nTmp));

        sPreview = aTextEdit.GetText().Copy(0, nTmp);
    }

    // Bild anpassen
    if (pEdit == &aDropCapsField || pEdit == &aTextEdit)
        pPict->SetText (sPreview);
    else if (pEdit == &aLinesField)
        pPict->SetLines((BYTE)aLinesField.GetValue());
    else
        pPict->SetDistance((USHORT)aDistanceField.Denormalize(aDistanceField.GetValue(FUNIT_TWIP)));

    bModified = TRUE;

    return 0;
}

/****************************************************************************
Page: Select-Handler der Template-Box.
*****************************************************************************/


IMPL_LINK_INLINE_START( SwDropCapsPage, SelectHdl, ListBox *, EMPTYARG )
{
    pPict->UpdatePaintSettings();
    bModified = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SwDropCapsPage, SelectHdl, ListBox *, EMPTYARG )


USHORT*  SwDropCapsPage::GetRanges()
{
    return aPageRg;
}


void SwDropCapsPage::FillSet( SfxItemSet &rSet )
{
    if(bModified)
    {
        SwFmtDrop aFmt;

        BOOL bOn = aDropCapsBox.IsChecked();
        if(bOn)
        {
            // Anzahl, Zeilen, Abstand
            aFmt.GetChars()     = (BYTE) aDropCapsField.GetValue();
            aFmt.GetLines()     = (BYTE) aLinesField.GetValue();
            aFmt.GetDistance()  = (USHORT) aDistanceField.Denormalize(aDistanceField.GetValue(FUNIT_TWIP));
            aFmt.GetWholeWord() = aWholeWordCB.IsChecked();

            // Vorlage
            if (aTemplateBox.GetSelectEntryPos())
                aFmt.SetCharFmt(rSh.GetCharStyle(aTemplateBox.GetSelectEntry()));
        }
        else
        {
            aFmt.GetChars()    = 1;
            aFmt.GetLines()    = 1;
            aFmt.GetDistance() = 0;
        }

        // Attribute setzen
        const SfxPoolItem* pOldItem;
        if(0 == (pOldItem = GetOldItem( rSet, FN_FORMAT_DROPCAPS )) ||
                    aFmt != *pOldItem )
            rSet.Put(aFmt);

        // Harte Textformatierung
        // Bug 24974: In Gestalter/Vorlagenkatoplog macht das keinen Sinn!!
        if( !bFormat && aDropCapsBox.IsChecked() )
        {
            String sText(aTextEdit.GetText());

            if (!aWholeWordCB.IsChecked())
                sText.Erase(aDropCapsField.GetValue());

            SfxStringItem aStr(FN_PARAM_1, sText);
            rSet.Put( aStr );
        }
    }
}




