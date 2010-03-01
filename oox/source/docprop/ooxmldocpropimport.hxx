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

#ifndef OOX_DOCPROP_OOXMLDOCPROPIMPORT_HXX
#define OOX_DOCPROP_OOXMLDOCPROPIMPORT_HXX

#include <sal/config.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>

namespace oox {
namespace docprop {

class OOXMLDocPropImportImpl:
    public ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::document::XOOXMLDocumentPropertiesImporter>
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    OOXMLDocPropImportImpl(const OOXMLDocPropImportImpl &); // not defined
    OOXMLDocPropImportImpl& operator=(const OOXMLDocPropImportImpl &); // not defined

public:
    explicit OOXMLDocPropImportImpl(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);
    virtual ~OOXMLDocPropImportImpl() {}

    ::com::sun::star::uno::Sequence< ::com::sun::star::xml::sax::InputSource > GetRelatedStreams( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xStorage, const ::rtl::OUString& aStreamType );

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::document::XOOXMLDocumentPropertiesImporter:
    virtual void SAL_CALL importProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > & xSource, const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > & xDocumentProperties) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::Exception);

};

} // namespace docprop
} // namespace oox

#endif

