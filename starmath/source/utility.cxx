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

void SmFontPickList::Clear()
{
    aFontVec.clear();
}

SmFontPickList& SmFontPickList::operator = (const SmFontPickList& rList)
{
    Clear();
    nMaxItems = rList.nMaxItems;
    for (sal_uInt16 nPos = 0; nPos < rList.aFontVec.size(); nPos++)
        aFontVec.push_back( rList.aFontVec[nPos] );

    return *this;
}

Font SmFontPickList::operator [] (sal_uInt16 nPos) const
{
    return aFontVec[nPos];
}

Font SmFontPickList::Get(sal_uInt16 nPos) const
{
    return nPos < aFontVec.size() ? aFontVec[nPos] : Font();
}

bool SmFontPickList::Contains(const Font &rFont) const
{
    return std::find( aFontVec.begin(), aFontVec.end(), rFont ) != aFontVec.end();
}




bool SmFontPickList::CompareItem(const Font & rFirstFont, const Font & rSecondFont) const
{
  return rFirstFont.GetName() == rSecondFont.GetName() &&
        rFirstFont.GetFamily()  == rSecondFont.GetFamily()  &&
        rFirstFont.GetCharSet() == rSecondFont.GetCharSet() &&
        rFirstFont.GetWeight()  == rSecondFont.GetWeight()  &&
        rFirstFont.GetItalic()  == rSecondFont.GetItalic();
}

OUString SmFontPickList::GetStringItem(const Font &rFont)
{
    OUStringBuffer aString(rFont.GetName());

    if (IsItalic( rFont ))
    {
        aString.append(", ");
        aString.append(SM_RESSTR(RID_FONTITALIC));
    }
    if (IsBold( rFont ))
    {
        aString.append(", ");
        aString.append(SM_RESSTR(RID_FONTBOLD));
    }

    return aString.makeStringAndClear();
}

void SmFontPickList::Insert(const Font &rFont)
{
    Remove(rFont);
    aFontVec.push_front( rFont );

    if (aFontVec.size() > nMaxItems)
    {
        aFontVec.pop_back();
    }
}

void SmFontPickList::Update(const Font &rFont, const Font &rNewFont)
{
    for (sal_uInt16 nPos = 0; nPos < aFontVec.size(); nPos++)
        if (CompareItem( aFontVec[nPos], rFont ))
        {
            aFontVec[nPos] = rNewFont;
            break;
        }
}

void SmFontPickList::Remove(const Font &rFont)
{
    for (sal_uInt16 nPos = 0; nPos < aFontVec.size(); nPos++)
        if (CompareItem( aFontVec[nPos], rFont))
        {
            aFontVec.erase( aFontVec.begin() + nPos );
            break;
        }
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

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSmFontPickListBox(Window* pParent, VclBuilder::stringmap &)
{
    return new SmFontPickListBox(pParent, WB_DROPDOWN);
}

SmFontPickListBox::SmFontPickListBox (Window* pParent, WinBits nBits) :
    SmFontPickList(4),
    ListBox(pParent, nBits)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}

IMPL_LINK( SmFontPickListBox, SelectHdl, ListBox *, /*pListBox*/ )
{
    sal_uInt16  nPos;
    OUString aString;

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

SmFontPickListBox& SmFontPickListBox::operator=(const SmFontPickList& rList)
{
    sal_uInt16 nPos;

    *(SmFontPickList *)this = rList;

    for (nPos = 0; nPos < aFontVec.size(); nPos++)
        InsertEntry(GetStringItem(aFontVec[nPos]), nPos);

    if (aFontVec.size() > 0)
        SelectEntry(GetStringItem(aFontVec.front()));

    return *this;
}

void SmFontPickListBox::Insert(const Font &rFont)
{
    SmFontPickList::Insert(rFont);

    RemoveEntry(GetStringItem(aFontVec.front()));
    InsertEntry(GetStringItem(aFontVec.front()), 0);
    SelectEntry(GetStringItem(aFontVec.front()));

    while (GetEntryCount() > nMaxItems)
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
