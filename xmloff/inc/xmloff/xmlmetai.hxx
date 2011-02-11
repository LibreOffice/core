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

#ifndef _XMLOFF_XMLMETAI_HXX
#define _XMLOFF_XMLMETAI_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/xmlictxt.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>


/// handles the top-level office:document-meta element of meta.xml documents
// NB: virtual inheritance is needed so that the context that handles the
//     flat xml file format can multiply inherit properly
class XMLOFF_DLLPUBLIC SvXMLMetaDocumentContext
    : public virtual SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler> mxDocBuilder;

public:
    SvXMLMetaDocumentContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties>& xDocProps,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler>& xDocBuilder);

    virtual ~SvXMLMetaDocumentContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference<
             ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

protected:
    /// initialize DocumentProperties object with DOM and base URL
    void initDocumentProperties();
    // set the BuildId property at the importer
    void setBuildId(const ::rtl::OUString & i_rBuildId);

public:
    static void setBuildId(const ::rtl::OUString & rGenerator,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xImportInfo );
};

#endif // _XMLOFF_XMLMETAI_HXX

