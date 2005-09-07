/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: utility.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:15:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#include <tools/stream.hxx>

#include "starmath.hrc"

#include "utility.hxx"
#include "dialog.hxx"
#include "view.hxx"
#include "smdll.hxx"


SmViewShell * SmGetActiveView()
    // return pointer to active SmViewShell, if this is not possible
    // return 0 instead.
{
    SfxViewShell *pView = SfxViewShell::Current();
    return PTR_CAST(SmViewShell, pView);
}


////////////////////////////////////////////////////////////
//
// SmRectCache
//


SmRectCache::Key::Key(const XubString &rText, const Font &rFont)
{
}


BOOL SmRectCache::Key::operator <  (const Key &rKey) const
{
#ifdef never
    BOOL  bRes = FALSE;

    if (aText < rKey.aText)
        bRes = TRUE;
    else if ()

    return      aText       <  rKey.aText
            ||  aText       == rKey.aText        &&  aFontName   < rKey.aFontName
            ||  aFontName   == rKey.aFontname    &&  aFontSize   < rKey.aFontSize
            ||  aFontSize   == rKey.aFontSize    &&  eFontWeight < rKey.eFontWeight
            ||  eFontWeight == rKey.eFontWeight  &&  eFontItalic < rKey.eFontItalic;
#endif
    return FALSE;
}


BOOL SmRectCache::Key::operator == (const Key &rKey) const
{
    return      aText       == rKey.aText
            &&  aFontName   == rKey.aFontName
            &&  aFontSize   == rKey.aFontSize
            &&  eFontWeight == rKey.eFontWeight
            &&  eFontItalic == rKey.eFontItalic;
}


SmRectCache::SmRectCache()
{
    pVirDev = 0;
}


SmRectCache::~SmRectCache()
{
    delete pVirDev;
}


const SmRect * SmRectCache::Search(const Key &rKey) const
{
    return 0;
}


const SmRect * SmRectCache::Add(const Key &rKey, const SmRect &rRect)
{
    return (const SmRect *)-1;
}


VirtualDevice * SmRectCache::GetVirDev()
{
    if (!pVirDev)
    {
        SmViewShell *pView = SmGetActiveView();
        if (pView)
            pVirDev = new VirtualDevice( pView->GetGraphicWindow() );
        else
            pVirDev = new VirtualDevice;
        pVirDev->SetMapMode( MapMode(MAP_100TH_MM) );
    }
    DBG_ASSERT(pVirDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
               "Sm : falscher MapMode");

    return pVirDev;
}


void SmRectCache::Reset()
{
}


////////////////////////////////////////////////////////////


/**************************************************************************/

SmPickList::SmPickList(USHORT nInitSize, USHORT nMaxSize) :
    SfxPtrArr((BYTE) nInitSize, 1)
{
    nSize = nMaxSize;
}


SmPickList::~SmPickList()
{
    Clear();
}


SmPickList& SmPickList::operator=(const SmPickList& rList)
{
    USHORT  nPos;

    Clear();
    nSize = rList.nSize;
    for (nPos = 0; nPos < rList.Count(); nPos++)
        InsertPtr(nPos, CreateItem(rList.Get(nPos)));

    return *this;
}


void SmPickList::Insert(const void *pItem)
{
    Remove(pItem);
    InsertPtr(0, CreateItem(pItem));

    if (Count() > nSize)
    {
        DestroyItem(GetPtr(nSize));
        RemovePtr(nSize, 1);
    }
}


void SmPickList::Update(const void *pItem, const void *pNewItem)
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        if (CompareItem(GetPtr(nPos), pItem))
        {
            DestroyItem(GetPtr(nPos));
            GetPtr(nPos) = CreateItem(pNewItem);
            break;
        }
}

void SmPickList::Remove(const void *pItem)
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        if (CompareItem(GetPtr(nPos), pItem))
        {
            DestroyItem(GetPtr(nPos));
            RemovePtr(nPos, 1);
            break;
        }
}

void SmPickList::SetSize(USHORT nNewSize)
{
    nSize = nNewSize;

    while (Count() > nSize)
    {
        DestroyItem(GetPtr(Count() - 1));
        RemovePtr(Count() - 1, 1);
    }
}


BOOL SmPickList::Contains(const void *pItem) const
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        if (CompareItem(GetPtr(nPos), pItem))
            return TRUE;

    return FALSE;
}


void SmPickList::Clear()
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        DestroyItem(GetPtr(nPos));

    RemovePtr(0, Count());
}


/**************************************************************************/
/**************************************************************************/

void * SmFontPickList::CreateItem(const String& rString)
{
    return new Font();
}

void * SmFontPickList::CreateItem(const void *pItem)
{
    return new Font(*((Font *) pItem));
}

void SmFontPickList::DestroyItem(void *pItem)
{
    delete (Font *)pItem;
}

BOOL SmFontPickList::CompareItem(const void *pFirstItem, const void *pSecondItem) const
{
    Font    *pFirstFont, *pSecondFont;

    pFirstFont  = (Font *)pFirstItem;
    pSecondFont = (Font *)pSecondItem;

    if (pFirstFont->GetName() == pSecondFont->GetName())
        if ((pFirstFont->GetFamily()  == pSecondFont->GetFamily())  &&
            (pFirstFont->GetCharSet() == pSecondFont->GetCharSet()) &&
            (pFirstFont->GetWeight()  == pSecondFont->GetWeight())  &&
            (pFirstFont->GetItalic()  == pSecondFont->GetItalic()))
            return (TRUE);

    return FALSE;
}

String SmFontPickList::GetStringItem(void *pItem)
{
    Font   *pFont;
    String  aString;
    const sal_Char *pDelim = ", ";

    pFont = (Font *)pItem;

    aString = pFont->GetName();

    if (IsItalic( *pFont ))
    {
        aString.AppendAscii( pDelim );
        aString += String(SmResId(RID_FONTITALIC));
    }
    if (IsBold( *pFont ))    // bold?
    {
        aString.AppendAscii( pDelim );
        aString += String(SmResId(RID_FONTBOLD));
    }

    return (aString);
}

void SmFontPickList::Insert(const Font &rFont)
{
    SmPickList::Insert((void *)&rFont);
}

void SmFontPickList::Update(const Font &rFont, const Font &rNewFont)
{
    SmPickList::Update((void *)&rFont, (void *)&rNewFont);
}

void SmFontPickList::Remove(const Font &rFont)
{
    SmPickList::Remove((void *)&rFont);
}


void SmFontPickList::ReadFrom(const SmFontDialog& rDialog)
{
    Insert(rDialog.GetFont());
}

void SmFontPickList::WriteTo(SmFontDialog& rDialog) const
{
    rDialog.SetFont(Get());
}


/**************************************************************************/


/**************************************************************************/

IMPL_LINK( SmFontPickListBox, SelectHdl, ListBox *, pListBox )
{
    USHORT  nPos;
    String  aString;

    nPos = GetSelectEntryPos();

    if (nPos != 0)
    {
        SmFontPickList::Insert(Get(nPos));
        aString = GetEntry(nPos);
        RemoveEntry(nPos);
        InsertEntry(aString, 0);
    }

    SelectEntryPos(0);

    return 0;
}


SmFontPickListBox::SmFontPickListBox(Window* pParent, WinBits nWinStyle, USHORT nMax) :
    SmFontPickList(nMax, nMax),
    ListBox(pParent, nWinStyle)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}


SmFontPickListBox::SmFontPickListBox(Window* pParent, const ResId& rResId, USHORT nMax) :
    SmFontPickList(nMax, nMax),
    ListBox(pParent, rResId)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}


SmFontPickListBox& SmFontPickListBox::operator=(const SmFontPickList& rList)
{
    USHORT nPos;

    *(SmFontPickList *)this = rList;

    for (nPos = 0; nPos < Count(); nPos++)
        InsertEntry(GetStringItem(GetPtr(nPos)), nPos);

    if (Count() > 0)
        SelectEntry(GetStringItem(GetPtr(0)));

    return *this;
}

void SmFontPickListBox::Insert(const Font &rFont)
{
    SmFontPickList::Insert(rFont);

    RemoveEntry(GetStringItem(GetPtr(0)));
    InsertEntry(GetStringItem(GetPtr(0)), 0);
    SelectEntry(GetStringItem(GetPtr(0)));

    while (GetEntryCount() > nSize)
        RemoveEntry(GetEntryCount() - 1);

    return;
}


void SmFontPickListBox::Update(const Font &rFont, const Font &rNewFont)
{
    SmFontPickList::Update(rFont, rNewFont);

    // ********************** hier fehlt noch was

    return;
}


void SmFontPickListBox::Remove(const Font &rFont)
{
    SmFontPickList::Remove(rFont);

    // ********************** hier fehlt noch was

    return;
}

////////////////////////////////////////

BOOL IsItalic( const Font &rFont )
{
    FontItalic eItalic = rFont.GetItalic();
    // the code below leaves only _NONE and _DONTKNOW as not italic
    return eItalic == ITALIC_OBLIQUE  ||  eItalic == ITALIC_NORMAL;
}


BOOL IsBold( const Font &rFont )
{
    FontWeight eWeight = rFont.GetWeight();
    return eWeight != WEIGHT_DONTKNOW && eWeight > WEIGHT_NORMAL;
}


void SmFace::Impl_Init()
{
    SetSize( GetSize() );
    SetTransparent( TRUE );
    SetAlign( ALIGN_BASELINE );
    SetColor( COL_AUTO );
}

void SmFace::SetSize(const Size& rSize)
{
    Size  aSize (rSize);

    // check the requested size against minimum value
    static int __READONLY_DATA  nMinVal = SmPtsTo100th_mm(2);

    if (aSize.Height() < nMinVal)
        aSize.Height() = nMinVal;

    //! we don't force a maximum value here because this may prevent eg the
    //! parentheses in "left ( ... right )" from matching up with large
    //! bodies (eg stack{...} with many entries).
    //! Of course this is holds only if characters are used and not polygons.

    Font::SetSize(aSize);
}


long SmFace::GetBorderWidth() const
{
    if (nBorderWidth < 0)
        return GetDefaultBorderWidth();
    else
        return nBorderWidth;
}

SmFace & SmFace::operator = (const SmFace &rFace)
{
    Font::operator = (rFace);
    nBorderWidth = -1;
    return *this;
}


SmFace & operator *= (SmFace &rFace, const Fraction &rFrac)
    // scales the width and height of 'rFace' by 'rFrac' and returns a
    // reference to 'rFace'.
    // It's main use is to make scaling fonts look easier.
{   const Size &rFaceSize = rFace.GetSize();

    rFace.SetSize(Size(Fraction(rFaceSize.Width())  *= rFrac,
                       Fraction(rFaceSize.Height()) *= rFrac));
    return rFace;
}



