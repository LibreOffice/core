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
#ifndef ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX
#define ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX

#include <tools/string.hxx>
#include <tools/wintypes.hxx>

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
} } } } }

class Window;
namespace accessibility
{
    class SAL_NO_VTABLE IComboListBoxHelper
    {
    public:
        virtual String          GetEntry( sal_uInt16 nPos ) const = 0;
        virtual Rectangle       GetDropDownPosSizePixel( ) const = 0;
        virtual Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const = 0;
        virtual Rectangle       GetWindowExtentsRelative( Window* pRelativeWindow ) = 0;
        virtual sal_Bool            IsActive() const = 0;
        virtual sal_Bool            IsEntryVisible( sal_uInt16 nPos ) const = 0;
        virtual sal_uInt16          GetDisplayLineCount() const = 0;
        virtual void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const = 0;
        virtual WinBits         GetStyle() const = 0;
        virtual sal_Bool            IsMultiSelectionEnabled() const = 0;
        virtual sal_uInt16          GetTopEntry() const = 0;
        virtual sal_Bool            IsEntryPosSelected( sal_uInt16 nPos ) const = 0;
        virtual sal_uInt16          GetEntryCount() const = 0;
        virtual void            Select() = 0;
        virtual void            SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect = sal_True ) = 0;
        virtual sal_uInt16          GetSelectEntryCount() const = 0;
        virtual void            SetNoSelection() = 0;
        virtual sal_uInt16          GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const = 0;
        virtual sal_Bool            IsInDropDown() const = 0;
        virtual Rectangle       GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const = 0;
        virtual long            GetIndexForPoint( const Point& rPoint, sal_uInt16& nPos ) const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >
                                GetClipboard() = 0;
    };
}
#endif // ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
