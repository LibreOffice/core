/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlReportElement.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:03:32 $
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

#ifndef RPT_XMLREPORTELEMENT_HXX
#include "xmlReportElement.hxx"
#endif
#ifndef RPT_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef RPT_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
#endif
#ifndef RPT_XMLCONDPRTEXPR_HXX
#include "xmlCondPrtExpr.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef RPT_XMLFORMATCONDITION_HXX
#include "xmlFormatCondition.hxx"
#endif


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLReportElement )

OXMLReportElement::OXMLReportElement( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XReportControlModel > & _xComponent) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLReportElement,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetReportElementElemTokenMap();

    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
         ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_PRINT_ONLY_WHEN_GROUP_CHANGE:
                    m_xComponent->setPrintWhenGroupChange(s_sTRUE == sValue);
                    break;
                   case XML_TOK_PRINT_REPEATED_VALUES:
                    m_xComponent->setPrintRepeatedValues(sValue == s_sTRUE ? sal_True : sal_False);
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------

OXMLReportElement::~OXMLReportElement()
{

    DBG_DTOR( rpt_OXMLReportElement,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLReportElement::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetReportElementElemTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_COMPONENT:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( rImport, _nPrefix, _rLocalName,xAttrList,m_xComponent.get());
            break;
        case XML_TOK_REP_CONDITIONAL_PRINT_EXPRESSION:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLCondPrtExpr( rImport, _nPrefix, _rLocalName,xAttrList,m_xComponent.get());
            break;
        case XML_TOK_FORMATCONDITION:
            {
                uno::Reference< report::XFormatCondition > xNewCond = m_xComponent->createFormatCondition();
                m_xComponent->insertByIndex(m_xComponent->getCount(),uno::makeAny(xNewCond));
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFormatCondition( rImport, _nPrefix, _rLocalName,xAttrList,xNewCond);
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );


    return pContext;
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLReportElement::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
