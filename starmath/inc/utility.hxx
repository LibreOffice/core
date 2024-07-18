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

#pragma once

#include <sal/config.h>
#include "smdllapi.hxx"

#include <o3tl/unit_conversion.hxx>
#include <sal/log.hxx>
#include <vcl/font.hxx>
#include <vcl/weld.hxx>
#include <tools/fract.hxx>
#include <tools/mapunit.hxx>
#include <deque>


class SmViewShell;
SmViewShell * SmGetActiveView();


// SmFace


bool    IsItalic( const vcl::Font &rFont );
bool    IsBold( const vcl::Font &rFont );

class SM_DLLPUBLIC SmFace final : public vcl::Font
{
    tools::Long    nBorderWidth;

    void    Impl_Init();

public:
    SmFace() :
        Font(), nBorderWidth(-1) { Impl_Init(); }
    explicit SmFace(const Font& rFont) :
        Font(rFont), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const OUString& rName, const Size& rSize) :
        Font(rName, rSize), nBorderWidth(-1) { Impl_Init(); }

    SmFace(const SmFace &rFace) :
        Font(rFace), nBorderWidth(-1) { Impl_Init(); }

    // overloaded version in order to supply a min value
    // for font size (height). (Also used in ctor's to do so.)
    void    SetSize(const Size& rSize);

    void    SetBorderWidth(tools::Long nWidth)     { nBorderWidth = nWidth; }
    tools::Long    GetBorderWidth() const;
    tools::Long    GetDefaultBorderWidth() const   { return GetFontSize().Height() / 20 ; }
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

public:
    explicit SmFontPickList(sal_uInt16 nMax = 5) : nMaxItems(nMax) {}
    virtual ~SmFontPickList() { Clear(); }

    virtual void    Insert(const vcl::Font &rFont);

    void            Clear();
    vcl::Font       Get(sal_uInt16 nPos = 0) const;

    SmFontPickList& operator = (const SmFontPickList& rList);

    void            ReadFrom(const SmFontDialog& rDialog);
    void            WriteTo(SmFontDialog& rDialog) const;
};


//  SmFontPickListBox


class SmFontPickListBox final : public SmFontPickList
{
private:
    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);

public:
    SmFontPickListBox(std::unique_ptr<weld::ComboBox> pWidget);
    SmFontPickListBox& operator = (const SmFontPickList& rList);
    virtual void    Insert(const vcl::Font &rFont) override;
};

// Math used to use 100ths of MM by default, but now uses twips
// which seems to improve (subpixel) positioning.
inline MapUnit SmMapUnit()
{
    return MapUnit::MapTwip;
}
inline o3tl::Length SmO3tlLengthUnit()
{
    return o3tl::Length::twip;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
