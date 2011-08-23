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

#ifndef _XMLOFF_METAIMPORTCOMPONENT_HXX
#define _XMLOFF_METAIMPORTCOMPONENT_HXX

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif
namespace binfilter {

class XMLMetaImportComponent : public SvXMLImport
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentInfo>	xDocInfo;

public:
    // #110680#
    // XMLMetaImportComponent() throw();
    XMLMetaImportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
        ) throw();

    ~XMLMetaImportComponent() throw();


protected:

    virtual SvXMLImportContext* CreateContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};


// global functions to support the component

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
    throw();

::rtl::OUString SAL_CALL XMLMetaImportComponent_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLMetaImportComponent_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

}//end of namespace binfilter
#endif

