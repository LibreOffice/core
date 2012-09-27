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
#ifndef ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX
#define ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <tools/string.hxx>
#include <tools/wintypes.hxx>

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
} } } } }

class Point;
class Rectangle;
class Window;
namespace accessibility
{
    class IComboListBoxHelper
    {
    public:
        virtual ~IComboListBoxHelper() = 0;

        virtual String          GetEntry( sal_uInt16 nPos ) const = 0;
        virtual Rectangle       GetDropDownPosSizePixel( ) const = 0;
        virtual Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const = 0;
        virtual Rectangle       GetWindowExtentsRelative( Window* pRelativeWindow ) = 0;
        virtual bool            IsActive() const = 0;
        virtual bool            IsEntryVisible( sal_uInt16 nPos ) const = 0;
        virtual sal_uInt16      GetDisplayLineCount() const = 0;
        virtual void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const = 0;
        virtual WinBits         GetStyle() const = 0;
        virtual bool            IsMultiSelectionEnabled() const = 0;
        virtual sal_uInt16      GetTopEntry() const = 0;
        virtual bool            IsEntryPosSelected( sal_uInt16 nPos ) const = 0;
        virtual sal_uInt16      GetEntryCount() const = 0;
        virtual void            Select() = 0;
        virtual void            SelectEntryPos( sal_uInt16 nPos, bool bSelect = sal_True ) = 0;
        virtual sal_uInt16      GetSelectEntryCount() const = 0;
        virtual void            SetNoSelection() = 0;
        virtual sal_uInt16      GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const = 0;
        virtual bool            IsInDropDown() const = 0;
        virtual Rectangle       GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const = 0;
        virtual long            GetIndexForPoint( const Point& rPoint, sal_uInt16& nPos ) const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >
                                GetClipboard() = 0;
    };
}
#endif // ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

