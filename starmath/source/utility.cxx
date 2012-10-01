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


////////////////////////////////////////////////////////////

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

SmPickList::SmPickList(sal_uInt16 nInitSize, sal_uInt16 nMaxSize) :
    SfxPtrArr((sal_uInt8) nInitSize, 1)
{
    nSize = nMaxSize;
}


SmPickList::~SmPickList()
{
    Clear();
}


SmPickList& SmPickList::operator=(const SmPickList& rList)
{
    sal_uInt16  nPos;

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
    sal_uInt16  nPos;

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
    sal_uInt16  nPos;

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
    sal_uInt16  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        DestroyItem(GetPtr(nPos));

    RemovePtr(0, Count());
}


/**************************************************************************/

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
    sal_uInt16  nPos;
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


SmFontPickListBox::SmFontPickListBox(Window* pParent, const ResId& rResId, sal_uInt16 nMax) :
    SmFontPickList(nMax, nMax),
    ListBox(pParent, rResId)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}


SmFontPickListBox& SmFontPickListBox::operator=(const SmFontPickList& rList)
{
    sal_uInt16 nPos;

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
