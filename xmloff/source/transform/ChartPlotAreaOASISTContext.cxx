/*************************************************************************
 *
 *  $RCSfile: ChartPlotAreaOASISTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:44:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartPlotAreaOASISTContext.hxx"

#ifndef _XMLOFF_TRANSFORMER_BASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_DEEPTCONTEXT_HXX
#include "DeepTContext.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOASIS_HXX
#include "ActionMapTypesOASIS.hxx"
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

class XMLAxisOASISContext : public XMLTransformerContext
{
public:
    TYPEINFO();

    XMLAxisOASISContext( XMLTransformerBase& rTransformer,
                         const ::rtl::OUString& rQName,
                         ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext );
    ~XMLAxisOASISContext();

    virtual XMLTransformerContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::rtl::OUString& rQName,
        const Reference< xml::sax::XAttributeList >& xAttrList );

private:
    ::rtl::Reference< XMLPersAttrListTContext > & m_rCategoriesContext;
};

TYPEINIT1( XMLAxisOASISContext, XMLTransformerContext );

XMLAxisOASISContext::XMLAxisOASISContext(
    XMLTransformerBase& rTransformer,
    const ::rtl::OUString& rQName,
    ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext ) :
        XMLTransformerContext( rTransformer, rQName ),
        m_rCategoriesContext( rOutCategoriesContext )
{}

XMLAxisOASISContext::~XMLAxisOASISContext()
{}

XMLTransformerContext * XMLAxisOASISContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rQName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    XMLTransformerContext * pContext = 0;

    if( XML_NAMESPACE_CHART == nPrefix &&
        ::xmloff::token::IsXMLToken( rLocalName, ::xmloff::token::XML_CATEGORIES ) )
    {
        // store categories element at parent
        m_rCategoriesContext.set( new XMLPersAttrListTContext( GetTransformer(), rQName ));
        pContext = m_rCategoriesContext.get();
    }
    else
    {
        pContext =  XMLTransformerContext::CreateChildContext(
            nPrefix, rLocalName, rQName, xAttrList );
    }

    return pContext;
}

TYPEINIT1( XMLChartPlotAreaOASISTContext, XMLProcAttrTransformerContext );

XMLChartPlotAreaOASISTContext::XMLChartPlotAreaOASISTContext(
    XMLTransformerBase & rTransformer, const ::rtl::OUString & rQName ) :
        XMLProcAttrTransformerContext( rTransformer, rQName, OASIS_SHAPE_ACTIONS )
{
}

XMLChartPlotAreaOASISTContext::~XMLChartPlotAreaOASISTContext()
{}

XMLTransformerContext * XMLChartPlotAreaOASISTContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rQName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( XML_NAMESPACE_CHART == nPrefix &&
        ::xmloff::token::IsXMLToken( rLocalName, ::xmloff::token::XML_AXIS ) )
    {
        pContext = new XMLAxisOASISContext( GetTransformer(), rQName, m_rCategoriesContext );
    }
    else
    {
        // export (and forget) categories if found in an axis-element
        // otherwise export regularly
        ExportCategories();
        pContext =  XMLProcAttrTransformerContext::CreateChildContext(
                nPrefix, rLocalName, rQName, xAttrList );
    }

    return pContext;
}

void XMLChartPlotAreaOASISTContext::EndElement()
{
    ExportCategories();
    XMLProcAttrTransformerContext::EndElement();
}

void XMLChartPlotAreaOASISTContext::ExportCategories()
{
    if( m_rCategoriesContext.is())
    {
        m_rCategoriesContext->Export();
        m_rCategoriesContext.clear();
    }
}
