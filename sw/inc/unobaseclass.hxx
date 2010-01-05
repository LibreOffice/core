/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unobaseclass.hxx,v $
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
#ifndef _UNOBASECLASS_HXX
#define _UNOBASECLASS_HXX

#include <calbck.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <cppuhelper/implbase2.hxx>

typedef
cppu::WeakImplHelper2
<
    ::com::sun::star::container::XEnumeration,
    ::com::sun::star::lang::XServiceInfo
>
SwSimpleEnumerationBaseClass;


/* -----------------29.04.98 07:35-------------------
 *
 * --------------------------------------------------*/
enum CursorType
{
    CURSOR_INVALID,
    CURSOR_BODY,
    CURSOR_FRAME,
    CURSOR_TBLTEXT,
    CURSOR_FOOTNOTE,
    CURSOR_HEADER,
    CURSOR_FOOTER,
    CURSOR_REDLINE,
    CURSOR_ALL,         // for Search&Replace
    CURSOR_SELECTION,   // create a paragraph enumeration from
                        // a text range or cursor
    CURSOR_SELECTION_IN_TABLE,
    CURSOR_META,         // meta/meta-field
};

#endif

