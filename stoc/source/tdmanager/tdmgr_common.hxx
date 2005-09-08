/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdmgr_common.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:13:53 $
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

#ifndef _STOC_TDMGR_COMMON_HXX
#define _STOC_TDMGR_COMMON_HXX

#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif

#include "com/sun/star/reflection/XTypeDescription.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) (sizeof (x) / sizeof *(x))


namespace css = ::com::sun::star;

namespace stoc_tdmgr
{
    extern rtl_StandardModuleCount g_moduleCount;

struct IncompatibleTypeException
{
    ::rtl::OUString m_cause;
    IncompatibleTypeException( ::rtl::OUString const & cause )
        : m_cause( cause ) {}
};

void check(
    css::uno::Reference<css::reflection::XTypeDescription> const & xNewTD,
    css::uno::Reference<css::reflection::XTypeDescription> const & xExistingTD,
    ::rtl::OUString const & context = ::rtl::OUString() );
/* throw (css::uno::RuntimeException, IncompatibleTypeException) */

} // namespace stoc_tdmgr

#endif /* _STOC_TDMGR_COMMON_HXX */
