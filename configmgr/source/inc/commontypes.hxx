/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commontypes.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CONFIGMGR_COMMONTYPES_HXX_
#define _CONFIGMGR_COMMONTYPES_HXX_

#include <rtl/ustring.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/uno/Any.hxx>

#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif

//..........................................................................
namespace configmgr
{
//..........................................................................

//==========================================================================
//= IInterface
//==========================================================================
/** abstract base class for objects which may be aquired by ORef's
    (in opposite to the IReference, classes deriving from this IInterface can be
    derived from XInterface, too)
*/
class IInterface
{
public:
    virtual void SAL_CALL acquire(  ) throw () = 0;
    virtual void SAL_CALL release(  ) throw () = 0;
};

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // _CONFIGMGR_COMMONTYPES_HXX_


