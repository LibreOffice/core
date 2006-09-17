/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexts.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:17:46 $
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
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLMETAI_HXX
#include "xmlmetai.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
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
            if( nFlags & IMPORT_META )
                pContext = new SfxXMLMetaContext( GetImport(), nPrefix, rLocalName, GetImport().GetModel());
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


