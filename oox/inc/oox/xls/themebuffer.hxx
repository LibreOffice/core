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

#ifndef OOX_XLS_THEMEBUFFER_HXX
#define OOX_XLS_THEMEBUFFER_HXX

#include "oox/drawingml/theme.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

struct FontModel;

// ============================================================================

class ThemeBuffer : public ::oox::drawingml::Theme, public WorkbookHelper
{
public:
    explicit            ThemeBuffer( const WorkbookHelper& rHelper );
    virtual             ~ThemeBuffer();

    /** Returns the theme color with the specified token identifier. */
    sal_Int32           getColorByToken( sal_Int32 nToken ) const;

    /** Returns the default font data for the current file type. */
    inline const FontModel& getDefaultFontModel() const { return *mxDefFontModel; }

private:
    typedef ::std::auto_ptr< FontModel > FontModelPtr;
    FontModelPtr        mxDefFontModel;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

