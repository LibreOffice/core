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

#ifndef _ACCESSIBLESTRINGWRAP_HXX
#define _ACCESSIBLESTRINGWRAP_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
class OutputDevice;
class Rectangle;
class Point;
namespace binfilter {


class SvxFont;

//------------------------------------------------------------------------
//
// AccessibleStringWrap declaration
//
//------------------------------------------------------------------------

class AccessibleStringWrap
{
public:

    AccessibleStringWrap( OutputDevice& rDev, SvxFont& rFont, const String& rText ){DBG_BF_ASSERT(0, "STRIP");}//STRIP001 AccessibleStringWrap( OutputDevice& rDev, SvxFont& rFont, const String& rText );

    sal_Bool GetCharacterBounds( sal_Int32 nIndex, Rectangle& rRect ){DBG_BF_ASSERT(0, "STRIP"); return false;}//STRIP001 sal_Bool GetCharacterBounds( sal_Int32 nIndex, Rectangle& rRect );
    sal_Int32 GetIndexAtPoint( const Point& rPoint ){DBG_BF_ASSERT(0, "STRIP"); return 0;}//STRIP001 sal_Int32 GetIndexAtPoint( const Point& rPoint );

private:
    
};

}//end of namespace binfilter
#endif /* _ACCESSIBLESTRINGWRAP_HXX */
