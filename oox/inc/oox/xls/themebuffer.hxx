/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: themebuffer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:08:57 $
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

#ifndef OOX_XLS_THEMEBUFFER_HXX
#define OOX_XLS_THEMEBUFFER_HXX

#include "oox/drawingml/theme.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

struct OoxFontData;

// ============================================================================

class ThemeBuffer : public WorkbookHelper
{
public:
    explicit            ThemeBuffer( const WorkbookHelper& rHelper );
    virtual             ~ThemeBuffer();

    /** Returns the pointer to the core drawingml theme. */
    inline const ::oox::drawingml::ThemePtr& getCoreThemePtr() const { return mxTheme; }

    /** Returns the core drawingml theme needed by the theme fragment importer. */
    ::oox::drawingml::Theme& getOrCreateCoreTheme();

    /** Returns the theme color with the specified token identifier. */
    sal_Int32           getColorByToken( sal_Int32 nToken ) const;
    /** Returns the theme color with the specified index. */
    sal_Int32           getColorByIndex( sal_Int32 nIndex ) const;

    /** Returns the system color referred by the specified token. */
    static sal_Int32    getSystemColor( sal_Int32 nElement, sal_Int32 nDefaultColor );
    /** Returns the system background color (window color). */
    static sal_Int32    getSystemWindowColor();
    /** Returns the system text color (window text color). */
    static sal_Int32    getSystemWindowTextColor();

    /** Returns the default font data for the current file type. */
    inline const OoxFontData& getDefaultFontData() const { return *mxDefFontData; }

private:
    typedef ::std::auto_ptr< OoxFontData > OoxFontDataPtr;

    ::oox::drawingml::ThemePtr mxTheme;
    OoxFontDataPtr      mxDefFontData;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

