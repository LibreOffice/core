/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlReportElementBase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:23:33 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_XMLREPORTELEMENTBASE_HXX
#include "xmlReportElementBase.hxx"
#endif
#ifndef RPT_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef RPT_XMLCONTROLPROPERTY_HXX
#include "xmlControlProperty.hxx"
#endif
#ifndef RPT_XMLREPORTELEMENT_HXX
#include "xmlReportElement.hxx"
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef RPT_XMLTABLE_HXX
#include "xmlTable.hxx"
#endif

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLReportElementBase::OXMLReportElementBase( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                , const ::rtl::OUString& rLName
                ,const Reference< XReportComponent > & _xComponent
                ,OXMLTable* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_rImport(rImport)
,m_pContainer(_pContainer)
,m_xComponent(_xComponent)
{
}
// -----------------------------------------------------------------------------

OXMLReportElementBase::~OXMLReportElementBase()
{
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLReportElementBase::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = _CreateChildContext(nPrefix,rLocalName,xAttrList);
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLReportElementBase::_CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = m_rImport.GetControlElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_REPORT_ELEMENT:
            {
                uno::Reference<report::XReportControlModel> xReportModel(m_xComponent,uno::UNO_QUERY);
                if ( xReportModel.is() )
                {
                    m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                    pContext = new OXMLReportElement( m_rImport, nPrefix, rLocalName,xAttrList,xReportModel);
                }
            }
            break;
        case XML_TOK_PROPERTIES:
            m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLControlProperty( m_rImport, nPrefix, rLocalName,xAttrList,m_xComponent.get());
            break;
        default:
            break;
    }

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLReportElementBase::EndElement()
{
    try
    {
        if ( m_pContainer && m_pContainer->getSection().is() && m_xComponent.is() )
            m_pContainer->getSection()->add(m_xComponent.get());
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception caught while inserting a new control!");
    }
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
