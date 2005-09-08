/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: getglobalmutex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:28:08 $
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

#if !defined INCLUDED_OSL_GETGLOBALMUTEX_HXX
#define INCLUDED_OSL_GETGLOBALMUTEX_HXX

#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif

namespace osl {

/** A helper functor for the rtl_Instance template.

    See the rtl_Instance template for examples of how this class is used.
 */
class GetGlobalMutex
{
public:
    ::osl::Mutex * operator()()
    {
        return ::osl::Mutex::getGlobalMutex();
    }
};

}

#endif // INCLUDED_OSL_GETGLOBALMUTEX_HXX
