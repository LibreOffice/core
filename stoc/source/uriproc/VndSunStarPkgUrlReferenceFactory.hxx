/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VndSunStarPkgUrlReferenceFactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:18:23 $
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

#ifndef INCLUDED_stoc_source_uriproc_VndSunStarPkgUrlReferenceFactory_hxx
#define INCLUDED_stoc_source_uriproc_VndSunStarPkgUrlReferenceFactory_hxx

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
    class XInterface;
} } } }
namespace rtl { class OUString; }

namespace stoc { namespace uriproc {

namespace VndSunStarPkgUrlReferenceFactory {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context)
        SAL_THROW((com::sun::star::uno::Exception));

    rtl::OUString SAL_CALL getImplementationName();

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames();
}

} }

#endif
