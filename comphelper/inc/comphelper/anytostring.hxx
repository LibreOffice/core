/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anytostring.hxx,v $
 * $Revision: 1.6 $
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

#if ! defined(INCLUDED_COMPHELPER_ANYTOSTRING_HXX)
#define INCLUDED_COMPHELPER_ANYTOSTRING_HXX

#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Any.hxx"
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{

/** Creates a STRING representation out of an ANY value.

    @param value
           ANY value
    @return
           STRING representation of given ANY value
*/
COMPHELPER_DLLPUBLIC ::rtl::OUString anyToString( ::com::sun::star::uno::Any const & value );

}

#endif

