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
#ifndef UTILITY_HXX
#define UTILITY_HXX

#include <sfx2/minarray.hxx>
#include <vcl/font.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/fract.hxx>


inline long SmPtsTo100th_mm(long nNumPts)
    // returns the length (in 100th of mm) that corresponds to the length
    // 'nNumPts' (in units points).
    // 72.27 [pt] = 1 [inch] = 2,54 [cm] = 2540 [100th of mm].
    // result is being rounded to the nearest integer.
{
    OSL_ENSURE(nNumPts >= 0, "Sm : Ooops...");
    // broken into multiple and fraction of 'nNumPts' to reduce chance
    // of overflow
    // (7227 / 2) is added in order to round to the nearest integer
    return 35 * nNumPts + (nNumPts * 1055L + (7227 / 2)) / 7227L;
}


inline long SmPtsTo100th_mm(const Fraction &rNumPts)
    // as above but with argument 'rNumPts' as 'Fraction'
{
    Fraction  aTmp (254000L, 7227L);
    return aTmp *= rNumPts;
}


inline Fraction Sm100th_mmToPts(long nNum100th_mm)
    // returns the length (in points) that corresponds to the length
    // 'nNum100th_mm' (in 100th of mm).
{
    OSL_ENSURE(nNum100th_mm >= 0, "Sm : Ooops...");
    Fraction  aTmp (7227L, 254000L);
    return aTmp *= Fraction(nNum100th_mm);
}


inline long SmRoundFraction(const Fraction &rFrac)
{
    OSL_ENSURE(rFrac > Fraction(), "Sm : Ooops...");
    return (rFrac.GetNumerator() + rFrac.GetDenominator() / 2) / rFrac.GetDenominator();
}


class SmViewShell;
SmViewShell * SmGetActiveView();


////////////////////////////////////////////////////////////
//
// SmFace
//

bool    IsItalic( const Font &rFont );
bool    IsBold( const Font &rFont );

class SmFace : public Font
{
    long    nBorderWidth;

    void    Impl_Init();

public:
    SmFace() :
        Font(), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const Font& rFont) :
        Font(rFont), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const rtl::OUString& rName, const Size& rSize) :
        Font(rName, rSize), nBorderWidth(-1) { Impl_Init(); }
    SmFace( FontFamily eFamily, const Size& rSize) :
        Font(eFamily, rSize), nBorderWidth(-1) { Impl_Init(); }

    SmFace(const SmFace &rFace) :
        Font(rFace), nBorderWidth(-1) { Impl_Init(); }

    // overloaded version in order to supply a min value
    // for font size (height). (Also used in ctor's to do so.)
    void    SetSize(const Size& rSize);

    void    SetBorderWidth(long nWidth)     { nBorderWidth = nWidth; }
    long    GetBorderWidth() const;
    long    GetDefaultBorderWidth() const   { return GetSize().Height() / 20 ; }
    void    FreezeBorderWidth()     { nBorderWidth = GetDefaultBorderWidth(); }

    SmFace & operator = (const SmFace &rFace);
};

SmFace & operator *= (SmFace &rFace, const Fraction &rFrac);


////////////////////////////////////////////////////////////
//
// SmPickList
//

class SmPickList : public SfxPtrArr
{
protected:
    sal_uInt16  nSize;

    virtual void   *CreateItem(const void *pItem) = 0;
    virtual void    DestroyItem(void *pItem) = 0;

    virtual bool    CompareItem(const void *pFirstItem, const void *pSecondItem) const = 0;

    virtual String  GetStringItem(void *pItem) = 0;

    void       *GetPtr(sal_uInt16 nPos) const { return SfxPtrArr::GetObject(nPos); }
    void      *&GetPtr(sal_uInt16 nPos) { return SfxPtrArr::GetObject(nPos); }
    void        InsertPtr(sal_uInt16 nPos, void *pItem) { SfxPtrArr::Insert(nPos, pItem); }
    void        RemovePtr(sal_uInt16 nPos, sal_uInt16 nCount = 1) { SfxPtrArr::Remove(nPos, nCount); }

public:
    SmPickList(sal_uInt16 nInitSize = 0, sal_uInt16 nMaxSize = 5);
    virtual ~SmPickList();

    SmPickList&   operator = (const SmPickList& rList);

    void       *Get(sal_uInt16 nPos = 0) const { return GetPtr(nPos); }
    using   SfxPtrArr::Insert;
    void        Insert(const void* pItem);
    void        Update(const void* pItem, const void *pNewItem);
    using   SfxPtrArr::Remove;
    void        Remove(const void* pItem);

    using   SfxPtrArr::operator [];
    void       *operator [] (sal_uInt16 nPos) const { return GetPtr(nPos); }

    sal_uInt16      GetSize() const { return nSize; }
    sal_uInt16      Count() const { return SfxPtrArr::Count(); }

    void        Clear();
};


////////////////////////////////////////////////////////////
//
// SmFontPickList
//

class SmFontDialog;

class SmFontPickList : public SmPickList
{
protected:
    virtual void   *CreateItem(const void *pItem);
    virtual void    DestroyItem(void *pItem);

    virtual bool    CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String  GetStringItem(void *pItem);

public:
    SmFontPickList()
        : SmPickList(0, 5) {}
    SmFontPickList(sal_uInt16 nInitSize, sal_uInt16 nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmFontPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    virtual ~SmFontPickList() { Clear(); }

    using   SfxPtrArr::Insert;
    virtual void    Insert(const Font &rFont);
    using   SmPickList::Update;
    virtual void    Update(const Font &rFont, const Font &rNewFont);
    using   SfxPtrArr::Remove;
    virtual void    Remove(const Font &rFont);

    using   SmPickList::Contains;
    inline bool     Contains(const Font &rFont) const;
    inline Font     Get(sal_uInt16 nPos = 0) const;

    inline SmFontPickList&  operator = (const SmFontPickList& rList);
    using   SfxPtrArr::operator [];
    inline Font             operator [] (sal_uInt16 nPos) const;

    void            ReadFrom(const SmFontDialog& rDialog);
    void            WriteTo(SmFontDialog& rDialog) const;
};

inline SmFontPickList& SmFontPickList::operator = (const SmFontPickList& rList)
{
    *(SmPickList *)this = *(SmPickList *)&rList; return *this;
}

inline Font SmFontPickList::operator [] (sal_uInt16 nPos) const
{
    return *((Font *)SmPickList::operator[](nPos));
}

inline Font SmFontPickList::Get(sal_uInt16 nPos) const
{
    return nPos < Count() ? *((Font *)SmPickList::Get(nPos)) : Font();
}

inline bool SmFontPickList::Contains(const Font &rFont) const
{
    return SmPickList::Contains((void *)&rFont);
}


////////////////////////////////////////////////////////////
//
//  SmFontPickListBox
//

class SmFontPickListBox : public SmFontPickList, public ListBox
{
protected:
    DECL_LINK(SelectHdl, ListBox *);

public:
    SmFontPickListBox(Window* pParent, const ResId& rResId, sal_uInt16 nMax = 4);

    SmFontPickListBox& operator = (const SmFontPickList& rList);

    using   SfxPtrArr::Insert;
    virtual void    Insert(const Font &rFont);
    using   Window::Update;
    virtual void    Update(const Font &rFont, const Font &rNewFont);
    using   SfxPtrArr::Remove;
    virtual void    Remove(const Font &rFont);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
