/*************************************************************************
 *
 *  $RCSfile: insrule.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:36:39 $
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
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
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


SwInsertGrfRulerDlg::SwInsertGrfRulerDlg( Window* pParent ) :
    SfxModalDialog(pParent, SW_RES(DLG_INSERT_RULER)),
    aSelectionFL(this, ResId(FL_SEL     )),
    pExampleVS  (new SwRulerValueSet(this, ResId(VS_EXAMPLE ))),
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
    GalleryExplorer::BeginLocking(GALLERY_THEME_RULERS);
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
    GalleryExplorer::EndLocking(GALLERY_THEME_RULERS);
    delete pExampleVS;
}

/*-----------------14.02.97 13.17-------------------

--------------------------------------------------*/

String SwInsertGrfRulerDlg::GetGraphicName()
{
    String sRet;
    USHORT nSel = nSelPos - 2; //align selection position with ValueSet index
    if(nSel < aGrfNames.Count())
        sRet = URIHelper::SmartRelToAbs(*(String*) aGrfNames.GetObject(nSel));
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
SwRulerValueSet::SwRulerValueSet(   Window* pParent, const ResId& rResId ) :
    SvxBmpNumValueSet(pParent, rResId)
{
    SetStyle(  GetStyle() & ~WB_ITEMBORDER     );
}
/*-----------------14.02.97 14.17-------------------

--------------------------------------------------*/
SwRulerValueSet::~SwRulerValueSet()
{
}
/*-----------------14.02.97 13.42-------------------

--------------------------------------------------*/
void __EXPORT SwRulerValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    USHORT  nItemId = rUDEvt.GetItemId();
    Point aBLPos = aRect.TopLeft();

    // Itemzaehlung beginnt bei 1, und die 1. ist die einfache Linie
    if(nItemId > 1)
    {
        Graphic aGraphic;
        if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_RULERS, nItemId - 2,
                                                            &aGraphic))
        {
            Size aGrfSize = aGraphic.GetPrefSize();
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
                aGraphic.Draw(pDev, aBLPos, aPaintSize);
                pDev->SetClipRegion(aRegion);
            }
        }
        else
        {
            SetGrfNotFound(TRUE);
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

