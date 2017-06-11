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

#include <vcl/builderfactory.hxx>

#include "strings.hrc"

#include "utility.hxx"
#include "dialog.hxx"
#include "view.hxx"


// return pointer to active SmViewShell, if this is not possible
// return 0 instead.
//!! Since this method is based on the current focus it is somewhat
//!! unreliable and may return unexpected 0 pointers!
SmViewShell * SmGetActiveView()
{
    SfxViewShell *pView = SfxViewShell::Current();
    return  dynamic_cast<SmViewShell*>( pView);
}


/**************************************************************************/

void SmFontPickList::Clear()
{
    aFontVec.clear();
}

SmFontPickList& SmFontPickList::operator = (const SmFontPickList& rList)
{
    Clear();
    nMaxItems = rList.nMaxItems;
    for (const auto & nPos : rList.aFontVec)
        aFontVec.push_back( nPos );

    return *this;
}

vcl::Font SmFontPickList::Get(sal_uInt16 nPos) const
{
    return nPos < aFontVec.size() ? aFontVec[nPos] : vcl::Font();
}

namespace {

bool lcl_CompareItem(const vcl::Font & rFirstFont, const vcl::Font & rSecondFont)
{
  return rFirstFont.GetFamilyName() == rSecondFont.GetFamilyName() &&
         rFirstFont.GetFamilyType() == rSecondFont.GetFamilyType() &&
         rFirstFont.GetCharSet()    == rSecondFont.GetCharSet()    &&
         rFirstFont.GetWeight()     == rSecondFont.GetWeight()     &&
         rFirstFont.GetItalic()     == rSecondFont.GetItalic();
}

OUString lcl_GetStringItem(const vcl::Font &rFont)
{
    OUStringBuffer aString(rFont.GetFamilyName());

    if (IsItalic( rFont ))
    {
        aString.append(", ");
        aString.append(SmResId(RID_FONTITALIC));
    }
    if (IsBold( rFont ))
    {
        aString.append(", ");
        aString.append(SmResId(RID_FONTBOLD));
    }

    return aString.makeStringAndClear();
}

}

void SmFontPickList::Insert(const vcl::Font &rFont)
{
    for (size_t nPos = 0; nPos < aFontVec.size(); nPos++)
        if (lcl_CompareItem( aFontVec[nPos], rFont))
        {
            aFontVec.erase( aFontVec.begin() + nPos );
            break;
        }

    aFontVec.push_front( rFont );

    if (aFontVec.size() > nMaxItems)
    {
        aFontVec.pop_back();
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

VCL_BUILDER_FACTORY_ARGS(SmFontPickListBox, WB_DROPDOWN)

SmFontPickListBox::SmFontPickListBox (vcl::Window* pParent, WinBits nBits) :
    SmFontPickList(4),
    ListBox(pParent, nBits)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}

IMPL_LINK_NOARG( SmFontPickListBox, SelectHdl, ListBox&, void )
{
    OUString aString;

    const sal_Int32 nPos = GetSelectEntryPos();

    if (nPos != 0)
    {
        SmFontPickList::Insert(Get(nPos));
        aString = GetEntry(nPos);
        RemoveEntry(nPos);
        InsertEntry(aString, 0);
    }

    SelectEntryPos(0);
}

SmFontPickListBox& SmFontPickListBox::operator=(const SmFontPickList& rList)
{
    *static_cast<SmFontPickList *>(this) = rList;

    for (decltype(aFontVec)::size_type nPos = 0; nPos < aFontVec.size(); nPos++)
        InsertEntry(lcl_GetStringItem(aFontVec[nPos]), nPos);

    if (!aFontVec.empty())
        SelectEntry(lcl_GetStringItem(aFontVec.front()));

    return *this;
}

void SmFontPickListBox::Insert(const vcl::Font &rFont)
{
    SmFontPickList::Insert(rFont);

    RemoveEntry(lcl_GetStringItem(aFontVec.front()));
    InsertEntry(lcl_GetStringItem(aFontVec.front()), 0);
    SelectEntry(lcl_GetStringItem(aFontVec.front()));

    while (GetEntryCount() > nMaxItems)
        RemoveEntry(GetEntryCount() - 1);
}


bool IsItalic( const vcl::Font &rFont )
{
    FontItalic eItalic = rFont.GetItalic();
    // the code below leaves only _NONE and _DONTKNOW as not italic
    return eItalic == ITALIC_OBLIQUE  ||  eItalic == ITALIC_NORMAL;
}


bool IsBold( const vcl::Font &rFont )
{
    FontWeight eWeight = rFont.GetWeight();
    return eWeight != WEIGHT_DONTKNOW && eWeight > WEIGHT_NORMAL;
}


void SmFace::Impl_Init()
{
    SetSize( GetFontSize() );
    SetTransparent( true );
    SetAlignment( ALIGN_BASELINE );
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

    Font::SetFontSize(aSize);
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
{   const Size &rFaceSize = rFace.GetFontSize();

    rFace.SetSize(Size(Fraction(rFaceSize.Width())  *= rFrac,
                       Fraction(rFaceSize.Height()) *= rFrac));
    return rFace;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
