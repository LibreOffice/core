/*************************************************************************
 *
 *  $RCSfile: fontlb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
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

#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#pragma hdrstop

#include "fontlb.hxx"

DBG_NAME(SvLBoxFontString);

/*************************************************************************
#* Funktionen der in die SvxFontListBox eingefuegten Items
#************************************************************************/


/*************************************************************************
#*  Member:     SvLBoxFontString                            Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvLBoxFontString
#*
#*  Funktion:   Konstruktor der Klasse SvLBoxFontString
#*
#*  Input:      Box- Entry,Flags, Text fuer Anzeige, Schrift
#*
#*  Output:     ---
#*
#************************************************************************/

SvLBoxFontString::SvLBoxFontString( SvLBoxEntry*pEntry,USHORT nFlags,const XubString& rStr,
                                    const Font& aFont)

: SvLBoxString( pEntry, nFlags, rStr )
{
    DBG_CTOR(SvLBoxFontString,0);
    aPrivatFont=aFont;
    SetText( pEntry, rStr );
}

/*************************************************************************
#*  Member:     SvLBoxFontString                            Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvLBoxFontString
#*
#*  Funktion:   Default Konstruktor der Klasse SvLBoxFontString
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

SvLBoxFontString::SvLBoxFontString()
: SvLBoxString()
{
    DBG_CTOR(SvLBoxFontString,0);
}

/*************************************************************************
#*  Member:     ~SvLBoxFontString                           Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvLBoxFontString
#*
#*  Funktion:   Destruktor der Klasse SvLBoxFontString
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

SvLBoxFontString::~SvLBoxFontString()
{
    DBG_DTOR(SvLBoxFontString,0);
}


/*************************************************************************
#*  Member:     SvLBoxFontString                            Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvLBoxFontString
#*
#*  Funktion:   Erzeugt einen neuen SvLBoxFontString
#*
#*  Input:      ---
#*
#*  Output:     SvLBoxFontString
#*
#************************************************************************/

SvLBoxItem* SvLBoxFontString::Create() const
{
    DBG_CHKTHIS(SvLBoxFontString,0);
    return new SvLBoxFontString;
}



/*************************************************************************
#*  Member:     SvLBoxFontString                            Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvLBoxFontString
#*
#*  Funktion:   Zeichenroutine des SvLBoxFontString. Gezeichnet wird
#*              der entsprechende Text mit der eingestellten Schriftart
#*              im Ausgabe- Device.
#*
#*  Input:      Position, Ausgabe- Device, Flag fuer Selection,
#*              Zeiger auf den Eintrag
#*
#*  Output:     ---
#*
#************************************************************************/

void SvLBoxFontString::Paint( const Point& rPos, SvLBox& rDev,
                             USHORT nFlags, SvLBoxEntry* pEntry )
{
    Font aFont=rDev.GetFont();
    Font a2Font=aPrivatFont;
    Color aColor;
    if(nFlags & SVLISTENTRYFLAG_SELECTED)
    {
        aColor=a2Font.GetColor();
        aColor.SetRed(~aColor.GetRed());
        aColor.SetGreen(~aColor.GetGreen());
        aColor.SetBlue(~aColor.GetBlue());
        a2Font.SetColor(aColor);
        rDev.SetFont(a2Font);
    }
    else
    {
        rDev.SetFont(aPrivatFont);
    }
    SvLBoxString::Paint(rPos,rDev,nFlags,pEntry );
    rDev.SetFont(aFont);
}


/*************************************************************************
#*  Member:     SvLBoxFontString                            Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvLBoxFontString
#*
#*  Funktion:   Ermittelt und stellt die Ausgabegroesse in der View ein.
#*
#*  Input:      Zeiger auf die View, Eintrag, Anzeigedaten
#*
#*
#*  Output:     ---
#*
#************************************************************************/

void SvLBoxFontString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry,
    SvViewDataItem* pViewData)
{
    DBG_CHKTHIS(SvLBoxFontString,0);
    Font aFont= pView->GetFont();
    pView->SetFont(aPrivatFont);
    SvLBoxString::InitViewData(pView,pEntry,pViewData);
    pView->SetFont(aFont);
}

/*************************************************************************
#* Listbox mit Schrifteinstellung
#************************************************************************/


/*************************************************************************
#*  Member:     SvxFontListBox                          Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Konstruktor der Klasse SvxFontListBox. Die Klasse dient
#*              der Darstellung von Strings in der entsprechenden
#*              Schriftart.
#*
#*  Input:      Parent-Window, WinBits, min. Winkel, max. Winkel
#*
#*  Output:     ---
#*
#************************************************************************/

SvxFontListBox::SvxFontListBox(Window* pParent,const ResId& aResID)
:SvTabListBox(pParent,aResID)
{
    pPrivatVDev=new VirtualDevice;
    aStandardFont=GetFont();
    Size aWinSize=GetOutputSizePixel();
    bSettingFont=FALSE;
    aStandardFont.SetTransparent(TRUE);
    aStandardFont.SetColor(Color(COL_BLUE));
    if(pPrivatVDev!=NULL)
    {
        pPrivatVDev->SetOutputSizePixel(aWinSize);
        pPrivatVDev->SetFont(aStandardFont);
    }
    Color aCol=GetBackground().GetColor();
    aStandardFont.SetColor(aCol);
    SetFont(aStandardFont);
    aEntryFont=aStandardFont;
}

/*************************************************************************
#*  Member:     SvxFontListBox                              Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Destruktor der Klasse SvxFontListBox. Gibt den
#*              Speicher, der fuer das VDevice angelegt wurde,
#*              wieder frei.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

SvxFontListBox::~SvxFontListBox()
{
    delete pPrivatVDev;
}


/*************************************************************************
#*  Member:     InsertFontEntry                             Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxColorTabListBox
#*
#*  Funktion:   Erzeugt aus dem uebergebenen String eine Bitmap
#*              und fuegt diese Bitmap in die ListBox ein.
#*
#*  Input:      String, Font, Position
#*
#*  Output:     ---
#*
#************************************************************************/

void SvxFontListBox::InsertFontEntry( const String& rString,Font aActorFont)
{
    Point aPos(0,0);
    bSettingFont=TRUE;
    aEntryFont=aActorFont;
    InsertEntry( rString);
}

/*************************************************************************
#*  Member:     SelectEntryPos                              Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Schaltet an der Position nPos, die Selektion
#*              an oder aus.
#*
#*  Input:      Position, Selektion: An/Aus
#*
#*  Output:     ---
#*
#************************************************************************/

void SvxFontListBox::SelectEntryPos( USHORT nPos, BOOL bSelect)
{
    SvLBoxEntry* pEntry=GetEntry(nPos);
    if(pEntry!=NULL)
    {
        Select( pEntry,bSelect);
        ShowEntry(pEntry );
    }
}

/*************************************************************************
#*  Member:     GetSelectEntryPos                           Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Liefert die Position der ersten Selektion
#*              zurück.
#*
#*  Input:      ---
#*
#*  Output:     Position
#*
#************************************************************************/

ULONG  SvxFontListBox::GetSelectEntryPos()
{
    SvLBoxEntry*    pSvLBoxEntry=FirstSelected();
    ULONG nSel=LIST_APPEND;

    if(pSvLBoxEntry!=NULL)
    {
        nSel=GetModel()->GetAbsPos( pSvLBoxEntry );
    }

    return nSel;
}

/*************************************************************************
#*  Member:     GetSelectEntry                              Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Liefert den ersten Eintrag der Selektion
#*              zurück.
#*
#*  Input:      ---
#*
#*  Output:     Position
#*
#************************************************************************/

XubString  SvxFontListBox::GetSelectEntry()
{
    return GetEntryText(GetSelectEntryPos());
}


/*************************************************************************
#*  Member:     SetNoSelection                          Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Hebt eine bestehende Selektierung auf.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void SvxFontListBox::SetNoSelection()
{
    SelectAll( FALSE,TRUE );
}


/*************************************************************************
#*  Member:     SvxFontListBox                              Datum:20.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxFontListBox
#*
#*  Funktion:   Ueberladene Funktion der TreeListBox. Fuegt einen
#*              neuen SvLBoxFontString ein.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void SvxFontListBox::InitEntry( SvLBoxEntry* pEntry, const XubString& aStr,
                               const Image& aCollEntryBmp, const Image& aExpEntryBmp)
{
    if(bSettingFont==TRUE)
    {
        bSettingFont=FALSE;

        SvLBoxButton* pButton;
        SvLBoxFontString* pString;
        SvLBoxContextBmp* pContextBmp;

        if( nTreeFlags & TREEFLAG_CHKBTN )
        {
            pButton= new SvLBoxButton( pEntry,0,pCheckButtonData );
            pEntry->AddItem( pButton );
        }

        pContextBmp= new SvLBoxContextBmp( pEntry,0, aCollEntryBmp,aExpEntryBmp,
                                         SVLISTENTRYFLAG_EXPANDED);
        pEntry->AddItem( pContextBmp );

        pString = new SvLBoxFontString( pEntry, 0, aStr,aEntryFont);
        pEntry->AddItem( pString );
    }
    else
    {
        SvTreeListBox::InitEntry( pEntry, aStr, aCollEntryBmp, aExpEntryBmp);
    }
}


