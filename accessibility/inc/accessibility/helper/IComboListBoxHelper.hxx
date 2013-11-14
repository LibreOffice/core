/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX
#define ACCESSIBILITY_HELPER_COMBOLISTBOXHELPER_HXX

#include <tools/gen.hxx>
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
        virtual sal_Bool        IsEnabled() const = 0;
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
