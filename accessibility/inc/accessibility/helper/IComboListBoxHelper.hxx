/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IComboListBoxHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:22:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX
#define ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

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
