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

#ifndef _ACCESSIBLESTRINGWRAP_HXX
#define _ACCESSIBLESTRINGWRAP_HXX

#include <sal/types.h>
#include <tools/string.hxx>

#include <editeng/editengdllapi.h>

class OutputDevice;
class SvxFont;
class Rectangle;
class Point;

//------------------------------------------------------------------------
//
// AccessibleStringWrap declaration
//
//------------------------------------------------------------------------

class AccessibleStringWrap
{
public:

    EDITENG_DLLPUBLIC AccessibleStringWrap( OutputDevice& rDev, SvxFont& rFont, const String& rText );

    EDITENG_DLLPUBLIC sal_Bool GetCharacterBounds( sal_Int32 nIndex, Rectangle& rRect );
    EDITENG_DLLPUBLIC sal_Int32 GetIndexAtPoint( const Point& rPoint );

private:

    OutputDevice&       mrDev;
    SvxFont&            mrFont;
    String              maText;
};

#endif /* _ACCESSIBLESTRINGWRAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
