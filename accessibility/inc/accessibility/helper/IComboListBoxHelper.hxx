/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IComboListBoxHelper.hxx,v $
 * $Revision: 1.3 $
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

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <vcl/wintypes.hxx>
#include <tools/gen.hxx>

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
} } } } }

class Window;
namespace accessibility
{
    class SAL_NO_VTABLE IComboListBoxHelper
    {
    public:
        virtual String          GetEntry( USHORT nPos ) const = 0;
        virtual Rectangle       GetDropDownPosSizePixel( ) const = 0;
        virtual Rectangle       GetBoundingRectangle( USHORT nItem ) const = 0;
        virtual Rectangle       GetWindowExtentsRelative( Window* pRelativeWindow ) = 0;
        virtual BOOL            IsActive() const = 0;
        virtual BOOL            IsEntryVisible( USHORT nPos ) const = 0;
        virtual USHORT          GetDisplayLineCount() const = 0;
        virtual void            GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const = 0;
        virtual WinBits         GetStyle() const = 0;
        virtual BOOL            IsMultiSelectionEnabled() const = 0;
        virtual USHORT          GetTopEntry() const = 0;
        virtual BOOL            IsEntryPosSelected( USHORT nPos ) const = 0;
        virtual USHORT          GetEntryCount() const = 0;
        virtual void            Select() = 0;
        virtual void            SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE ) = 0;
        virtual USHORT          GetSelectEntryCount() const = 0;
        virtual void            SetNoSelection() = 0;
        virtual USHORT          GetSelectEntryPos( USHORT nSelIndex = 0 ) const = 0;
        virtual BOOL            IsInDropDown() const = 0;
        virtual Rectangle       GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const = 0;
        virtual long            GetIndexForPoint( const Point& rPoint, USHORT& nPos ) const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >
                                GetClipboard() = 0;
    };
}
#endif // ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX
