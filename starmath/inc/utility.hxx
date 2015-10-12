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
#ifndef INCLUDED_STARMATH_INC_UTILITY_HXX
#define INCLUDED_STARMATH_INC_UTILITY_HXX

#include <sal/config.h>

#include <sal/log.hxx>
#include <vcl/font.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/fract.hxx>
#include <deque>


inline long SmPtsTo100th_mm(long nNumPts)
    // returns the length (in 100th of mm) that corresponds to the length
    // 'nNumPts' (in units points).
    // 72.27 [pt] = 1 [inch] = 2,54 [cm] = 2540 [100th of mm].
    // result is being rounded to the nearest integer.
{
    SAL_WARN_IF( nNumPts < 0, "starmath", "Ooops..." );
    // broken into multiple and fraction of 'nNumPts' to reduce chance
    // of overflow
    // (7227 / 2) is added in order to round to the nearest integer
    return 35 * nNumPts + (nNumPts * 1055L + (7227 / 2)) / 7227L;
}


inline Fraction Sm100th_mmToPts(long nNum100th_mm)
    // returns the length (in points) that corresponds to the length
    // 'nNum100th_mm' (in 100th of mm).
{
    SAL_WARN_IF( nNum100th_mm < 0, "starmath", "Ooops..." );
    Fraction  aTmp (7227L, 254000L);
    return aTmp *= Fraction(nNum100th_mm);
}


inline long SmRoundFraction(const Fraction &rFrac)
{
    SAL_WARN_IF( rFrac <= Fraction(), "starmath", "Ooops..." );
    return (rFrac.GetNumerator() + rFrac.GetDenominator() / 2) / rFrac.GetDenominator();
}


class SmViewShell;
SmViewShell * SmGetActiveView();




// SmFace


bool    IsItalic( const vcl::Font &rFont );
bool    IsBold( const vcl::Font &rFont );

class SmFace : public vcl::Font
{
    long    nBorderWidth;

    void    Impl_Init();

public:
    SmFace() :
        Font(), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const Font& rFont) :
        Font(rFont), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const OUString& rName, const Size& rSize) :
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




// SmFontPickList


class SmFontDialog;

class SmFontPickList
{
protected:
    sal_uInt16 nMaxItems;
    std::deque<vcl::Font> aFontVec;

    static bool     CompareItem(const vcl::Font & rFirstFont, const vcl::Font & rSecondFont);
    static OUString GetStringItem(const vcl::Font &rItem);

public:
    SmFontPickList(sal_uInt16 nMax = 5) : nMaxItems(nMax) {}
    virtual ~SmFontPickList() { Clear(); }

    virtual void    Insert(const vcl::Font &rFont);
    virtual void    Update(const vcl::Font &rFont, const vcl::Font &rNewFont);
    virtual void    Remove(const vcl::Font &rFont);

    void            Clear();
    vcl::Font       Get(sal_uInt16 nPos = 0) const;

    SmFontPickList& operator = (const SmFontPickList& rList);
    vcl::Font       operator [] (sal_uInt16 nPos) const;

    void            ReadFrom(const SmFontDialog& rDialog);
    void            WriteTo(SmFontDialog& rDialog) const;
};



//  SmFontPickListBox


class SmFontPickListBox : public SmFontPickList, public ListBox
{
protected:
    DECL_LINK_TYPED(SelectHdl, ListBox&, void);

public:
    SmFontPickListBox(vcl::Window* pParent, WinBits nBits);

    SmFontPickListBox& operator = (const SmFontPickList& rList);

    virtual void    Insert(const vcl::Font &rFont) override;
    using   Window::Update;
    virtual void    Update(const vcl::Font &rFont, const vcl::Font &rNewFont) override;
    virtual void    Remove(const vcl::Font &rFont) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
