/*************************************************************************
 *
 *  $RCSfile: insrule.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:45 $
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

#include "uiparam.hxx"
#include "hintids.hxx"

#ifndef _GALLERY_HXX_ //autogen
#include <svx/gallery.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#include "swtypes.hxx"
#include "docsh.hxx"
#include "insrule.hxx"
#include "swvset.hxx"
#include "insrule.hrc"
#include "misc.hrc"
#include "helpid.h"

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


SwInsertGrfRulerDlg::SwInsertGrfRulerDlg( Window* pParent, SwDocShell* pDocSh ) :
    SfxModalDialog(pParent, SW_RES(DLG_INSERT_RULER)),
    aSelectionGB(this, ResId(GB_SEL     )),
    pExampleVS  (new SwRulerValueSet(this, ResId(VS_EXAMPLE ), aGrfNames, pDocSh)),
    aOkPB       (this, ResId(PB_OK      )),
    aCancelPB   (this, ResId(PB_CANCEL  )),
    aHelpPB     (this, ResId(PB_HELP    )),
    sSimple (ResId(ST_SIMPLE)),
    nSelPos(USHRT_MAX)
{
    FreeResource();
    pExampleVS->SetLineCount(6);
    pExampleVS->SetColCount(1);
    pExampleVS->SetSelectHdl(LINK(this, SwInsertGrfRulerDlg, SelectHdl));
    pExampleVS->SetDoubleClickHdl(LINK(this, SwInsertGrfRulerDlg, DoubleClickHdl));
    pExampleVS->GrabFocus();

    // Grafiknamen ermitteln
    GalleryExplorer::FillObjList( GALLERY_THEME_RULERS, aGrfNames );
    pExampleVS->SetHelpId(HID_VS_RULER);
    Color aColor(COL_WHITE);
    pExampleVS->InsertItem( 1, 1);
    pExampleVS->SetItemText( 1, sSimple);

    for(USHORT i = 1; i <= aGrfNames.Count(); i++)
    {
        pExampleVS->InsertItem( i + 1, i);
        pExampleVS->SetItemText( i + 1, *((String*)aGrfNames.GetObject(i-1)));
    }
    pExampleVS->Show();

}
/*-----------------14.02.97 13.18-------------------

--------------------------------------------------*/

SwInsertGrfRulerDlg::~SwInsertGrfRulerDlg()
{
    delete pExampleVS;
}

/*-----------------14.02.97 13.17-------------------

--------------------------------------------------*/

String SwInsertGrfRulerDlg::GetGraphicName()
{
    String sRet;
    if(nSelPos != USHRT_MAX)
    {
        SvxBmpItemInfo* pInfo = pExampleVS->FindInfo(nSelPos);
        if(pInfo)
            sRet = *pInfo->pBrushItem->GetGraphicLink();
    }
    return sRet;
}

/*-----------------14.02.97 13.20-------------------

--------------------------------------------------*/

IMPL_LINK(SwInsertGrfRulerDlg, SelectHdl, ValueSet*, pVS)
{
    nSelPos = pVS->GetSelectItemId();
    aOkPB.Enable();
    return 0;
}

/*-----------------14.02.97 14.17-------------------

--------------------------------------------------*/

SwRulerValueSet::SwRulerValueSet(   Window* pParent,
                                    const ResId& rResId,
                                    const List& rStrNames,
                                    SwDocShell* pDocSh) :
    SvxBmpNumValueSet(pParent, rResId, rStrNames),
    pSwDocSh(pDocSh)
{

    SetStyle(  GetStyle()
                    & ~WB_ITEMBORDER     );

}

/*-----------------14.02.97 14.17-------------------

--------------------------------------------------*/

SwRulerValueSet::~SwRulerValueSet()
{
}


/*-----------------13.02.97 09.41-------------------

--------------------------------------------------*/

IMPL_STATIC_LINK(SwRulerValueSet, GraphicArrivedHdl_Impl, SvxBrushItem*, pItem)
{
    // Ueber Timer wird ein Format angeworfen
    pThis->GetFormatTimer().Start();
    return 0;
}

/*-----------------14.02.97 13.42-------------------

--------------------------------------------------*/

void __EXPORT SwRulerValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    USHORT  nItemId = rUDEvt.GetItemId();
    Point aBLPos = aRect.TopLeft();

    const List& rStrList = GetStringList();
    // Itemzaehlung beginnt bei 1, und die 1. ist die einfache Linie
    if(nItemId > 1)
    {
        if(rStrList.Count() > nItemId - 2)
        {
            String* pGrfName = (String*)rStrList.GetObject(nItemId - 2);
            SvxBmpItemInfo* pInfo;
            if(0 == (pInfo = FindInfo(nItemId)))
            {
                pInfo = new SvxBmpItemInfo();
                pInfo->nItemId = nItemId;
                pInfo->pBrushItem = new SvxBrushItem(*pGrfName, aEmptyStr, GPOS_AREA);
                pInfo->pBrushItem->SetDoneLink(STATIC_LINK(
                                        this, SwRulerValueSet, GraphicArrivedHdl_Impl));
                GetGrfBrushItems().Insert(pInfo);
            }

            const Graphic* pGraphic = pInfo->pBrushItem->GetGraphic(pSwDocSh);

            if(pGraphic)
            {
                Size aGrfSize = pGraphic->GetPrefSize();
                if(aGrfSize.Width() && aGrfSize.Height())
                {
                    int nRelGrf = aGrfSize.Height() * 100 / aGrfSize.Width();
                    Size aWinSize = aRect.GetSize();
                    Size aPaintSize = aWinSize;
                    int nRelWin = aWinSize.Height() * 100 / aWinSize.Width();
                    if(nRelGrf > nRelWin)
                    {
                        aPaintSize.Width() = aWinSize.Height() * 100 / nRelGrf;
                        aBLPos.X() += (aWinSize.Width() - aPaintSize.Width()) /2;
                    }
                    else
                    {
                        aPaintSize.Height() = aWinSize.Width() * nRelGrf/100;
                        aBLPos.Y() += (aWinSize.Height() - aPaintSize.Height()) /2;
                    }
                    aBLPos.X() -= aPaintSize.Width() /2;
                    aBLPos.Y() -= aPaintSize.Height() /2;

                    aPaintSize.Width() *= 2;
                    aPaintSize.Height() *= 2;
                    if(aPaintSize.Height() < 2)
                        aPaintSize.Height() = 2;
                    Region aRegion = pDev->GetClipRegion();
                    pDev->SetClipRegion(aRect);
                    pGraphic->Draw(pDev, aBLPos, aPaintSize);
                    pDev->SetClipRegion(aRegion);
                }
            }
            else
            {
                SetGrfNotFound(TRUE);
            }
        }
    }
    else
    {
        // Text fuer einfache Linie painten
        Font aOldFont = pDev->GetFont();
        Font aFont = pDev->GetFont();
        Size aSize = aFont.GetSize();
        int nRectHeight = aRect.GetHeight();
        aSize.Height() = nRectHeight * 2 / 3;
        aFont.SetSize(aSize);
        pDev->SetFont(aFont);
        String aText(GetItemText(nItemId));
        aSize.Width() = pDev->GetTextWidth(aText);
        aSize.Height() = pDev->GetTextHeight();
        Point aPos(aBLPos);
        aPos.Y() += (nRectHeight - aSize.Height()) / 2;
        aPos.X() += (aRect.GetWidth() - aSize.Width()) / 2;
        pDev->DrawText(aPos, aText);

        pDev->SetFont(aOldFont);
    }
}

/*-----------------15.02.97 10.03-------------------

--------------------------------------------------*/

IMPL_LINK(SwInsertGrfRulerDlg, DoubleClickHdl, ValueSet*, pVS)
{
    EndDialog(RET_OK);
    return 0;
}

/*--------------------------------------------------------------------

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.22  2000/09/18 16:05:57  willem.vandorp
      OpenOffice header added.

      Revision 1.21  2000/09/05 10:40:43  kz
      chg. SetWinStyle/GetWinStyle to SetStyle/GetStyle

      Revision 1.20  2000/04/18 15:08:17  os
      UNICODE

      Revision 1.19  2000/02/11 14:56:03  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.18  1999/03/16 11:43:36  MA
      #62507# Ruler uber ID besorgen


      Rev 1.17   16 Mar 1999 12:43:36   MA
   #62507# Ruler uber ID besorgen

      Rev 1.16   18 Nov 1998 08:31:54   OS
   #58263# Numerierung in den Svx - Reste

      Rev 1.15   17 Nov 1998 13:01:56   OS
   #58263# alte Methoden verschoben

      Rev 1.14   17 Mar 1998 17:04:24   OS
   Focus initial ins ValueSet#48169#

      Rev 1.13   24 Nov 1997 16:47:42   MA
   includes

      Rev 1.12   22 Sep 1997 15:14:42   OS
   MessageBox nicht im Dlg-Ctor aufrufen #44095#

      Rev 1.11   09 Sep 1997 13:34:28   OS
   Header aufgeteilt

      Rev 1.10   09 Aug 1997 13:31:34   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.9   08 Aug 1997 17:28:26   OM
   Headerfile-Umstellung

      Rev 1.8   26 May 1997 10:32:18   TRI
   __EXPORT benutzt

      Rev 1.7   23 May 1997 10:09:36   OS
   Ableitung von SfxModalDialog

      Rev 1.6   10 Mar 1997 17:28:22   OS
   fuer NULL-Size nicht painten (DrawObjekte)

      Rev 1.5   25 Feb 1997 12:21:08   OS
   Mindesthoehe fuer Ruler: 2 Pixel

      Rev 1.4   22 Feb 1997 20:21:42   OS
   doppelte Groesse, nur noch 6 Zeilen

      Rev 1.3   20 Feb 1997 17:50:22   OS
   Itemborder abgeschaltet

      Rev 1.2   19 Feb 1997 16:22:22   OS
   Dialogtitel jetzt >Horizontale Linie einfuegen<

      Rev 1.1   17 Feb 1997 16:38:08   OS
   Offset korrigiert

      Rev 1.0   15 Feb 1997 15:15:36   OS
   Initial revision.

 --------------------------------------------------------------------*/


