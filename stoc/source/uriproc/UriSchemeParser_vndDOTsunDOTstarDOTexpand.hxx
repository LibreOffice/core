/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UriSchemeParser_vndDOTsunDOTstarDOTexpand.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-12 10:55:34 $
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

#ifndef \
    INCLUDED_STOC_SOURCE_URIPROC_URISCHEMEPARSER_VNDDOTSUNDOTSTARDOTEXPAND_HXX
#define \
    INCLUDED_STOC_SOURCE_URIPROC_URISCHEMEPARSER_VNDDOTSUNDOTSTARDOTEXPAND_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include "com/sun/star/uno/Exception.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
    class XInterface;
} } } }
namespace rtl { class OUString; }

namespace stoc { namespace uriproc {

namespace UriSchemeParser_vndDOTsunDOTstarDOTexpand {
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL create(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((::com::sun::star::uno::Exception));

    ::rtl::OUString SAL_CALL getImplementationName();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames();
}

} }

#endif
