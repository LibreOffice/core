/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stocservices.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:55:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
    class XComponentContext;
} } } }

namespace stoc_services
{
    // typeconv
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TypeConverter_Impl_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & )
    throw( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > tcv_getSupportedServiceNames();
    ::rtl::OUString tcv_getImplementationName();

    namespace ExternalUriReferenceTranslator
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriReferenceFactory
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >  const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTexpand
    {
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((::com::sun::star::uno::Exception));
    ::rtl::OUString SAL_CALL getImplementationName();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTscript
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace VndSunStarPkgUrlReferenceFactory
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }
} // namespace
