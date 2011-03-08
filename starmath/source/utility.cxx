/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <sfx2/app.hxx>
#include <vcl/virdev.hxx>
#include <tools/string.hxx>
#include <tools/tenccvt.hxx>
#include <osl/thread.h>

#include <tools/stream.hxx>

#include "starmath.hrc"

#include "utility.hxx"
#include "dialog.hxx"
#include "view.hxx"
#include "smdll.hxx"


// return pointer to active SmViewShell, if this is not possible
// return 0 instead.
//!! Since this method is based on the current focus it is somewhat
//!! unreliable and may return unexpected 0 pointers!
SmViewShell * SmGetActiveView()
{
    SfxViewShell *pView = SfxViewShell::Current();
    return PTR_CAST(SmViewShell, pView);
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

void SmPickList::Clear()
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        DestroyItem(GetPtr(nPos));

    RemovePtr(0, Count());
}


/**************************************************************************/

void * SmFontPickList::CreateItem(const String& /*rString*/)
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

bool SmFontPickList::CompareItem(const void *pFirstItem, const void *pSecondItem) const
{
    Font    *pFirstFont, *pSecondFont;

    pFirstFont  = (Font *)pFirstItem;
    pSecondFont = (Font *)pSecondItem;

    if (pFirstFont->GetName() == pSecondFont->GetName())
        if ((pFirstFont->GetFamily()  == pSecondFont->GetFamily())  &&
            (pFirstFont->GetCharSet() == pSecondFont->GetCharSet()) &&
            (pFirstFont->GetWeight()  == pSecondFont->GetWeight())  &&
            (pFirstFont->GetItalic()  == pSecondFont->GetItalic()))
            return (true);

    return false;
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
    if (IsBold( *pFont ))
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


IMPL_LINK( SmFontPickListBox, SelectHdl, ListBox *, /*pListBox*/ )
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

    return;
}


void SmFontPickListBox::Remove(const Font &rFont)
{
    SmFontPickList::Remove(rFont);

    return;
}

////////////////////////////////////////

bool IsItalic( const Font &rFont )
{
    FontItalic eItalic = rFont.GetItalic();
    // the code below leaves only _NONE and _DONTKNOW as not italic
    return eItalic == ITALIC_OBLIQUE  ||  eItalic == ITALIC_NORMAL;
}


bool IsBold( const Font &rFont )
{
    FontWeight eWeight = rFont.GetWeight();
    return eWeight != WEIGHT_DONTKNOW && eWeight > WEIGHT_NORMAL;
}


void SmFace::Impl_Init()
{
    SetSize( GetSize() );
    SetTransparent( true );
    SetAlign( ALIGN_BASELINE );
    SetColor( COL_AUTO );
}

void SmFace::SetSize(const Size& rSize)
{
    Size  aSize (rSize);

    // check the requested size against minimum value
    static int const    nMinVal = SmPtsTo100th_mm(2);

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
