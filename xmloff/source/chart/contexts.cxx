/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexts.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:31:51 $
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
#include "precompiled_xmloff.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLMETAI_HXX
#include <xmloff/xmlmetai.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#ifndef SCH_XMLIMPORT_HXX_
#include "SchXMLImport.hxx"
#endif

//  #ifndef _XMLOFF_XMLCHARTSTYLECONTEXT_HXX_
//  #include "XMLChartStyleContext.hxx"
//  #endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif

#include "contexts.hxx"
#include "SchXMLChartContext.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

// ==================================================

class SchXMLBodyContext_Impl : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:

    SchXMLBodyContext_Impl( SchXMLImportHelper& rImpHelper,
                SvXMLImport& rImport, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName );
    virtual ~SchXMLBodyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
                const uno::Reference< xml::sax::XAttributeList > & xAttrList );
};

SchXMLBodyContext_Impl::SchXMLBodyContext_Impl(
        SchXMLImportHelper& rImpHelper, SvXMLImport& rImport,
        sal_uInt16 nPrfx, const ::rtl::OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrImportHelper( rImpHelper )
{
}

SchXMLBodyContext_Impl::~SchXMLBodyContext_Impl()
{
}

SvXMLImportContext *SchXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & )
{
    return new SchXMLBodyContext( mrImportHelper, GetImport(), nPrefix,
                                  rLocalName );
}

// ==================================================

SchXMLDocContext::SchXMLDocContext( SchXMLImportHelper& rImpHelper,
                                    SvXMLImport& rImport,
                                    USHORT nPrefix,
                                    const rtl::OUString& rLName ) :
        SvXMLImportContext( rImport, nPrefix, rLName ),
        mrImportHelper( rImpHelper )
{
    DBG_ASSERT( XML_NAMESPACE_OFFICE == nPrefix &&
        ( IsXMLToken( rLName, XML_DOCUMENT ) ||
          IsXMLToken( rLName, XML_DOCUMENT_META) ||
          IsXMLToken( rLName, XML_DOCUMENT_STYLES) ||
          IsXMLToken( rLName, XML_DOCUMENT_CONTENT) ),
                "SchXMLDocContext instanciated with no <office:document> element" );
}

SchXMLDocContext::~SchXMLDocContext()
{}

TYPEINIT1( SchXMLDocContext, SvXMLImportContext );

SvXMLImportContext* SchXMLDocContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetDocElemTokenMap();
    sal_uInt16 nFlags = GetImport().getImportFlags();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_DOC_AUTOSTYLES:
            if( nFlags & IMPORT_AUTOSTYLES )
                // not nice, but this is safe, as the SchXMLDocContext class can only by
                // instantiated by the chart import class SchXMLImport (header is not exported)
                pContext =
                    static_cast< SchXMLImport& >( GetImport() ).CreateStylesContext( rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_STYLES:
            // for draw styles containing gradients/hatches/markers and dashes
            if( nFlags & IMPORT_STYLES )
                pContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_META:
          // we come here in the flat ODF file format,
          // if XDocumentPropertiesSupplier is not supported at the model
//        DBG_WARNING("XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
            pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
            break;
        case XML_TOK_DOC_BODY:
            if( nFlags & IMPORT_CONTENT )
                pContext = new SchXMLBodyContext_Impl( mrImportHelper, GetImport(), nPrefix, rLocalName );
            break;
    }

    // call parent when no own context was created
    if( ! pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

// ==================================================

SchXMLFlatDocContext_Impl::SchXMLFlatDocContext_Impl(
        SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        USHORT i_nPrefix, const ::rtl::OUString & i_rLName,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps,
        const uno::Reference<xml::sax::XDocumentHandler>& i_xDocBuilder) :
    SvXMLImportContext(i_rImport, i_nPrefix, i_rLName),
    SchXMLDocContext(i_rImpHelper, i_rImport, i_nPrefix, i_rLName),
    SvXMLMetaDocumentContext(i_rImport, i_nPrefix, i_rLName,
        i_xDocProps, i_xDocBuilder)
{
}

SchXMLFlatDocContext_Impl::~SchXMLFlatDocContext_Impl() { }


SvXMLImportContext *SchXMLFlatDocContext_Impl::CreateChildContext(
    USHORT i_nPrefix, const ::rtl::OUString& i_rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& i_xAttrList)
{
    // behave like meta base class iff we encounter office:meta
    const SvXMLTokenMap& rTokenMap =
        mrImportHelper.GetDocElemTokenMap();
    if ( XML_TOK_DOC_META == rTokenMap.Get( i_nPrefix, i_rLocalName ) ) {
        return SvXMLMetaDocumentContext::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    } else {
        return SchXMLDocContext::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    }
}

// ----------------------------------------

SchXMLBodyContext::SchXMLBodyContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport,
                                      USHORT nPrefix,
                                      const rtl::OUString& rLName ) :
        SvXMLImportContext( rImport, nPrefix, rLName ),
        mrImportHelper( rImpHelper )
{
    DBG_ASSERT( XML_NAMESPACE_OFFICE == nPrefix &&
                IsXMLToken( rLName, XML_CHART ),
                "SchXMLBodyContext instanciated with no <office:chart> element" );
}

SchXMLBodyContext::~SchXMLBodyContext()
{}

void SchXMLBodyContext::EndElement()
{
}

SvXMLImportContext* SchXMLBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    // <chart:chart> element
    if( nPrefix == XML_NAMESPACE_CHART &&
        IsXMLToken( rLocalName, XML_CHART ) )
    {
        pContext = mrImportHelper.CreateChartContext( GetImport(),
                                                      nPrefix, rLocalName,
                                                      GetImport().GetModel(),
                                                      xAttrList );
    }
    else
    {
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;
}


