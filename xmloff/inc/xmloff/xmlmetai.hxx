/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlmetai.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:30:56 $
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

#ifndef _XMLOFF_XMLMETAI_HXX
#define _XMLOFF_XMLMETAI_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

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
    SvXMLMetaDocumentContext(SvXMLImport& rImport, USHORT nPrfx,
        const rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties>& xDocProps,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler>& xDocBuilder);

    virtual ~SvXMLMetaDocumentContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
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
};

#endif // _XMLOFF_XMLMETAI_HXX

