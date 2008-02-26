/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MetaImportComponent.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:29:27 $
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

#ifndef _XMLOFF_METAIMPORTCOMPONENT_HXX
#define _XMLOFF_METAIMPORTCOMPONENT_HXX

#include <xmloff/xmlimp.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>


class XMLMetaImportComponent : public SvXMLImport
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;

public:
    // #110680#
    // XMLMetaImportComponent() throw();
    XMLMetaImportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
        ) throw();

    virtual ~XMLMetaImportComponent() throw();


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

#endif

