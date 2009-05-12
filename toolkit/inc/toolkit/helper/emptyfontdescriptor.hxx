/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: emptyfontdescriptor.hxx,v $
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

#ifndef _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_
#define _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>

//  ----------------------------------------------------
//  class EmptyFontDescriptor
//  ----------------------------------------------------
class EmptyFontDescriptor : public ::com::sun::star::awt::FontDescriptor
{
public:
    EmptyFontDescriptor()
    {
        // Not all enums are initialized correctly in FontDescriptor-CTOR because
        // they are set to the first enum value, this is not allways the default value.
        Slant = ::com::sun::star::awt::FontSlant_DONTKNOW;
        Underline = ::com::sun::star::awt::FontUnderline::DONTKNOW;
        Strikeout = ::com::sun::star::awt::FontStrikeout::DONTKNOW;
    }
};




#endif // _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_

