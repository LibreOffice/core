/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlSection.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:04:03 $
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

#ifndef RPT_XMLSECTION_HXX
#include "xmlSection.hxx"
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
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef RPT_XMLCOLUMN_HXX
#include "xmlColumn.hxx"
#endif
#ifndef RPT_XMLCondPrtExpr_HXX
#include "xmlCondPrtExpr.hxx"
#endif
#ifndef RPT_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_REPORTPRINTOPTION_HPP_
#include <com/sun/star/report/ReportPrintOption.hpp>
#endif
#ifndef REPORTDESIGN_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef RPT_XMLTABLE_HXX
#include "xmlTable.hxx"
#endif


namespace rptxml
{
    using namespace ::xmloff;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

      sal_uInt16 lcl_getReportPrintOption(const ::rtl::OUString& _sValue)
    {
        sal_uInt16 nRet = report::ReportPrintOption::ALL_PAGES;
        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
        SvXMLUnitConverter::convertEnum( nRet, _sValue, aXML_EnumMap );
        return nRet;
    }


DBG_NAME( rpt_OXMLSection )

OXMLSection::OXMLSection( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName,
                const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const uno::Reference< report::XSection >& _xSection
                ,sal_Bool _bPageHeader)
:SvXMLImportContext( rImport, nPrfx, _sLocalName )
,m_xSection(_xSection)
,m_bPageHeader(_bPageHeader)
{
    DBG_CTOR( rpt_OXMLSection,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetSectionElemTokenMap();

    const sal_Int16 nLength = (m_xSection.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {

                case XML_TOK_PAGE_PRINT_OPTION:
                    if ( _bPageHeader )
                        m_xSection->getReportDefinition()->setPageHeaderOption(lcl_getReportPrintOption(sValue));
                    else
                        m_xSection->getReportDefinition()->setPageFooterOption(lcl_getReportPrintOption(sValue));
                    break;
                case XML_TOK_REPEAT_SECTION:
                    m_xSection->setRepeatSection(sValue == s_sTRUE );
                    break;

                default:
                    OSL_ENSURE(0,"OXMLSection: Unknown attribute!");
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the section props");
    }
}
// -----------------------------------------------------------------------------
OXMLSection::~OXMLSection()
{
    DBG_DTOR( rpt_OXMLSection,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLSection::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetSectionElemTokenMap();
    uno::Reference<lang::XMultiServiceFactory> xFactor = rImport.getServiceFactory();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_TABLE:
            pContext = new OXMLTable( rImport, _nPrefix, _rLocalName,xAttrList,m_xSection);
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( rImport, _nPrefix, _rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLSection::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
