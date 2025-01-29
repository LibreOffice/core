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

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ustring.hxx>
#include <vcl/wintypes.hxx>
#include <tools/long.hxx>

namespace com::sun::star::datatransfer::clipboard {
    class XClipboard;
}

class AbsoluteScreenPixelRectangle;
class Point;
namespace tools { class Rectangle; }
namespace vcl { class Window; }

class IComboListBoxHelper
{
public:
    virtual ~IComboListBoxHelper() = 0;

    virtual OUString        GetEntry( sal_Int32  nPos ) const = 0;
    virtual tools::Rectangle       GetDropDownPosSizePixel( ) const = 0;
    virtual tools::Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const = 0;
    virtual AbsoluteScreenPixelRectangle GetWindowExtentsAbsolute() = 0;
    virtual bool            IsEnabled() const = 0;
    virtual bool            IsEntryVisible( sal_Int32  nPos ) const = 0;
    virtual sal_uInt16      GetDisplayLineCount() const = 0;
    virtual void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const = 0;
    virtual WinBits         GetStyle() const = 0;
    virtual bool            IsMultiSelectionEnabled() const = 0;
    virtual sal_Int32       GetTopEntry() const = 0;
    virtual bool            IsEntryPosSelected( sal_Int32  nPos ) const = 0;
    virtual sal_Int32       GetEntryCount() const = 0;
    virtual void            Select() = 0;
    virtual void            SelectEntryPos( sal_Int32  nPos, bool bSelect = true ) = 0;
    virtual sal_Int32       GetSelectedEntryCount() const = 0;
    virtual void            SetNoSelection() = 0;
    virtual sal_Int32       GetSelectedEntryPos( sal_Int32  nSelIndex ) const = 0;
    virtual bool            IsInDropDown() const = 0;
    virtual tools::Rectangle       GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const = 0;
    virtual tools::Long            GetIndexForPoint( const Point& rPoint, sal_Int32 & nPos ) const = 0;
    virtual css::uno::Reference< css::datatransfer::clipboard::XClipboard >
                            GetClipboard() = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

