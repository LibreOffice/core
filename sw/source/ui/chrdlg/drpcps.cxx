/*************************************************************************
 *
 *  $RCSfile: drpcps.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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
    String aText;
    BYTE   nLines;
    USHORT nDistance;

    virtual void Paint(const Rectangle &rRect);

public:

     SwDropCapsPict(Window *pParent, const ResId &rResId) : Control(pParent, rResId) {}
    ~SwDropCapsPict();

    void SetText    (const String &rT) {aText     = rT; Invalidate();}
    void SetLines   (      BYTE    nL) {nLines    = nL; Invalidate();}
    void SetDistance(      USHORT  nD) {nDistance = nD; Invalidate();}
};


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
Pict: Paint-Overload
****************************************************************************/

#define LINES  10
#define BORDER  2


void  SwDropCapsPict::Paint(const Rectangle &rRect)
{
    if (!IsVisible())
        return;

    SetMapMode(MapMode(MAP_PIXEL));
    SetLineColor();

    // Weisser Hintergrund
    SetFillColor(Color(COL_WHITE));
    DrawRect(Rectangle(Point(0, 0), GetOutputSizePixel()));
    SetClipRegion(Region(Rectangle(
        Point(BORDER, BORDER),
        Size (GetOutputSizePixel().Width () - 2 * BORDER,
              GetOutputSizePixel().Height() - 2 * BORDER))));

    // Graue Linien
    long nTotLineH = (GetOutputSizePixel().Height() - 2 * BORDER) / LINES,
         nLineH    = nTotLineH - 2;

    ASSERT(nLineH > 0, "So klein lassen wir uns nicht machen");
    long nY0 = (GetOutputSizePixel().Height() - (LINES * nTotLineH)) / 2;
    SetFillColor(Color(COL_LIGHTGRAY));
    for (USHORT i = 0; i < LINES; i++)
        DrawRect(Rectangle(Point(BORDER, nY0 + i * nTotLineH), Size(GetOutputSizePixel().Width() - 2 * BORDER, nLineH)));

    // Text berechnen
    Font aFont;
    {
        SwDropCapsPage *pPage = (SwDropCapsPage *) GetParent();
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

    long nTextH = nLines * nTotLineH;
    aFont.SetSize(Size(0, nTextH));
    aFont.SetTransparent(TRUE);
    SetFont(aFont);
    USHORT nLeading = (USHORT) GetFontMetric().GetLeading();
    aFont.SetSize(Size(0, aFont.GetSize().Height() + nLeading));
    SetFont(aFont);

    // Texthintergrund mit Abstand (240 twips ~ 1 Zeilenhoehe)

    ULONG lDistance = nDistance;
    USHORT nDistW = (USHORT) (ULONG) (((lDistance * 100) / 240) * nTotLineH) / 100;
    SetFillColor(Color(COL_WHITE));
    if(((SwDropCapsPage*)GetParent())->aDropCapsBox.IsChecked())
    {
        DrawRect(Rectangle(
        Point(BORDER, nY0),
        Size (GetTextWidth(aText) + nDistW, nTextH)));

    // Text zeichnen
        DrawText(Point(BORDER, nY0 - nLeading), aText);
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
    aSettingsGroup(this, SW_RES(GRP_SETTINGS)),
    aPictBox      (this, SW_RES(BOX_PICT       )),
    aContentBox   (this, SW_RES(BOX_CONTENT )),
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

    // Preview
    pPict->SetText(aTextEdit.GetText());
    pPict->SetLines((BYTE )aLinesField.GetValue());
    pPict->SetDistance((USHORT)aDistanceField.Denormalize(aDistanceField.GetValue(FUNIT_TWIP)));

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
            nVal = (USHORT)aDropCapsField.GetMax();

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

        if (aWholeWordCB.IsChecked())   // Nur erstes Wort anzeigen
        {
            USHORT nPos = sPreview.Search(' ');

            if (nPos != STRING_NOTFOUND)
                sPreview.Erase(nPos);
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
    pPict->Invalidate();
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


/*--------------------------------------------------
  $Log: not supported by cvs2svn $
  Revision 1.100  2000/09/18 16:05:14  willem.vandorp
  OpenOffice header added.

  Revision 1.99  2000/08/25 10:13:51  os
  style listbox sorted

  Revision 1.98  2000/07/03 10:33:17  os
  #72742# resource warnings corrected

  Revision 1.97  2000/07/03 08:04:44  os
  #72742# resource warnings corrected

  Revision 1.96  2000/04/20 12:54:31  os
  GetName() returns String&

  Revision 1.95  2000/04/11 08:03:22  os
  UNICODE

  Revision 1.94  2000/03/03 15:16:58  os
  StarView remainders removed

  Revision 1.93  2000/02/11 14:43:37  hr
  #70473# changes for unicode ( patched by automated patchtool )

  Revision 1.92  1999/11/16 17:11:59  os
  use FillCharStyleListBox

  Revision 1.91  1998/09/08 14:49:16  OS
  #56134# Metric fuer Text und HTML getrennt


      Rev 1.90   08 Sep 1998 16:49:16   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.89   03 Apr 1998 17:06:10   OM
   #49189 Korrekte Initialenvorschau

      Rev 1.88   03 Apr 1998 16:59:36   OM
   #49189 Korrekte Initialenvorschau

      Rev 1.87   20 Mar 1998 17:43:28   OM
   Ganzes Wort als Initialie

      Rev 1.86   12 Sep 1997 10:35:36   OS
   ITEMID_* definiert

      Rev 1.85   09 Sep 1997 13:33:42   OS
   lokale class

      Rev 1.84   04 Sep 1997 17:16:22   MA
   includes

      Rev 1.83   01 Sep 1997 13:25:16   OS
   DLL-Umstellung

      Rev 1.82   15 Aug 1997 12:12:34   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.81   09 Aug 1997 13:01:26   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.80   08 Aug 1997 17:29:40   OM
   Headerfile-Umstellung

      Rev 1.79   09 Jul 1997 17:20:50   HJS
   includes

      Rev 1.78   24 Feb 1997 17:19:44   OS
   #36936# -> keine PoolId benutzen

      Rev 1.77   19 Feb 1997 19:11:54   MA
   fix: DropCap, Text per Item

      Rev 1.76   11 Feb 1997 16:35:04   OS
   Vorlagen in SvStringsISortDtor sortieren

      Rev 1.75   11 Dec 1996 15:46:50   JP
   neu: holen der HTML-CharVorlagen

      Rev 1.74   11 Nov 1996 09:16:38   MA
   ResMgr

      Rev 1.73   30 Oct 1996 08:12:10   MA
   includes

      Rev 1.72   29 Oct 1996 17:36:42   HJS
   includes

      Rev 1.71   24 Oct 1996 17:07:14   JP
   Optimierung: Find...ByName

      Rev 1.70   23 Oct 1996 13:49:12   JP
   SVMEM -> SVSTDARR Umstellung

      Rev 1.69   02 Oct 1996 08:36:14   MA
   Umstellung Enable/Disable

      Rev 1.68   28 Aug 1996 08:59:10   OS
   includes

      Rev 1.67   25 Jun 1996 13:57:34   MA
   atrpafnchxx entfernt

      Rev 1.66   04 Apr 1996 13:38:54   OS
   Bug 26699: neue Methode FillSet, die keine Aenderungen an der Shell vornimmt

      Rev 1.65   22 Feb 1996 08:28:36   OS
   Text beim Ausschalten zuruecksetzen

      Rev 1.64   21 Feb 1996 12:41:44   OS
   Dialog verschoenert, disable ist nicht mehr gleichbedeutend mit Inhalt loeschen

      Rev 1.63   06 Feb 1996 15:18:40   JP
   Link Umstellung 305

      Rev 1.62   30 Jan 1996 22:52:14   JP
   Bug #24974#: DropCaps-Text nie bei Vorlagen austauschen

      Rev 1.61   23 Jan 1996 11:27:38   OS
   richtige Reaktion auf Standard-Button im Formatdialog

      Rev 1.60   28 Nov 1995 21:15:16   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

--------------------------------------------------*/


