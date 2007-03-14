/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mergekeys.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-14 08:27:21 $
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

#ifndef INCLUDED_STOC_SOURCE_IMPLEMENTATIONREGISTRATION_MERGEKEYS_HXX
#define INCLUDED_STOC_SOURCE_IMPLEMENTATIONREGISTRATION_MERGEKEYS_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"

namespace com { namespace sun { namespace star { namespace registry {
    class XRegistryKey;
} } } }

namespace stoc_impreg {

// throws ::com::sun::star::registry::InvalidRegistryException,
// ::com::sun::star::registry::MergeConflictException,
// ::com::sun::star::uno::RuntimeException:
void mergeKeys(
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >
        const & xDest,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >
        const & xSource);

}

#endif
