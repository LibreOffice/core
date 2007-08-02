/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlGroup.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:33:34 $
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

#ifndef RPT_XMLGROUP_HXX
#include "xmlGroup.hxx"
#endif
#ifndef RPT_XMLSECTION_HXX
#include "xmlSection.hxx"
#endif
#ifndef RPT_XMLFUNCTION_HXX
#include "xmlFunction.hxx"
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
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_GROUPON_HPP_
#include <com/sun/star/report/GroupOn.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_KEEPTOGETHER_HPP_
#include <com/sun/star/report/KeepTogether.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;

    sal_uInt16 lcl_getKeepTogetherOption(const ::rtl::OUString& _sValue)
    {
        sal_uInt16 nRet = report::KeepTogether::NO;
        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetKeepTogetherOptions();
        SvXMLUnitConverter::convertEnum( nRet, _sValue, aXML_EnumMap );
        return nRet;
    }
DBG_NAME( rpt_OXMLGroup )

OXMLGroup::OXMLGroup( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
{
    DBG_CTOR( rpt_OXMLGroup,NULL);

    m_xGroups = _rImport.getReportDefinition()->getGroups();
    OSL_ENSURE(m_xGroups.is(),"Groups is NULL!");
    m_xGroup = m_xGroups->createGroup();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetGroupElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        const ::rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        ::rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        try
        {
            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_START_NEW_COLUMN:
                    m_xGroup->setStartNewColumn(sValue == s_sTRUE);
                    break;
                case XML_TOK_RESET_PAGE_NUMBER:
                    m_xGroup->setResetPageNumber(sValue == s_sTRUE);
                    break;
                case XML_TOK_SORT_ASCENDING:
                    m_xGroup->setSortAscending(sValue == s_sTRUE);
                    break;
                case XML_TOK_GROUP_EXPRESSION:
                    {
                        sal_Int32 nLen = sValue.getLength();
                        if ( nLen )
                        {
                            const static ::rtl::OUString s_sChanged(RTL_CONSTASCII_USTRINGPARAM("rpt:HASCHANGED(\""));
                            sal_Int32 nPos = sValue.indexOf(s_sChanged);
                            if ( nPos == -1 )
                                nPos = 5;
                            else
                            {
                                nPos = s_sChanged.getLength();
                                --nLen;
                            }
                            sValue = sValue.copy(nPos,nLen-nPos-1);
                            const ORptFilter::TGroupFunctionMap& aFunctions = _rImport.getFunctions();
                            ORptFilter::TGroupFunctionMap::const_iterator aFind = aFunctions.find(sValue);
                            if ( aFind != aFunctions.end() )
                            {
                                sal_Int32 nIndex = 0;
                                const ::rtl::OUString sCompleteFormula = aFind->second->getFormula();
                                ::rtl::OUString sExpression = sCompleteFormula.getToken(1,'[',nIndex);
                                nIndex = 0;
                                sExpression = sExpression.getToken(0,']',nIndex);
                                nIndex = 0;
                                const ::rtl::OUString sFormula = sCompleteFormula.getToken(0,'(',nIndex);
                                ::sal_Int16 nGroupOn = report::GroupOn::DEFAULT;

                                if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:LEFT")))
                                {
                                    nGroupOn = report::GroupOn::PREFIX_CHARACTERS;
                                    ::rtl::OUString sInterval = sCompleteFormula.getToken(1,';',nIndex);
                                    nIndex = 0;
                                    sInterval = sInterval.getToken(0,')',nIndex);
                                    m_xGroup->setGroupInterval(sInterval.toInt32());
                                }
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:YEAR")))
                                    nGroupOn = report::GroupOn::YEAR;
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:MONTH")))
                                {
                                    if ( sCompleteFormula.endsWithIgnoreAsciiCaseAsciiL("\4",2) )
                                        nGroupOn = report::GroupOn::QUARTAL;
                                    else
                                        nGroupOn = report::GroupOn::MONTH;
                                }
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:WEEK")))
                                    nGroupOn = report::GroupOn::WEEK;
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:DAY")))
                                    nGroupOn = report::GroupOn::DAY;
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:HOUR")))
                                    nGroupOn = report::GroupOn::HOUR;
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:MINUTE")))
                                    nGroupOn = report::GroupOn::MINUTE;
                                else if ( sFormula ==::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:INT")))
                                {
                                    nGroupOn = report::GroupOn::INTERVAL;
                                    _rImport.removeFunction(sExpression);
                                    sExpression = sExpression.copy(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INT_count_")).getLength());

                                    nIndex = 0;
                                    ::rtl::OUString sInterval = sCompleteFormula.getToken(1,'/',nIndex);
                                    nIndex = 0;
                                    sInterval = sInterval.getToken(0,')',nIndex);
                                    m_xGroup->setGroupInterval(sInterval.toInt32());
                                }

                                m_xGroup->setGroupOn(nGroupOn);

                                _rImport.removeFunction(sValue);
                                sValue = sExpression;
                            }
                            m_xGroup->setExpression(sValue);
                        }
                    }
                    break;
                case XML_TOK_GROUP_KEEP_TOGETHER:
                    m_xGroup->setKeepTogether(lcl_getKeepTogetherOption(sValue));
                    break;
                default:
                    break;
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exception catched while putting group props!");
        }
    }
}
// -----------------------------------------------------------------------------

OXMLGroup::~OXMLGroup()
{

    DBG_DTOR( rpt_OXMLGroup,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLGroup::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetGroupElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_GROUP_FUNCTION:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFunction( rImport, nPrefix, rLocalName,xAttrList,m_xGroup.get());
            }
            break;
        case XML_TOK_GROUP_HEADER:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xGroup->setHeaderOn(sal_True);
                pContext = new OXMLSection( rImport, nPrefix, rLocalName,xAttrList,m_xGroup->getHeader());
            }
            break;
        case XML_TOK_GROUP_GROUP:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLGroup( rImport, nPrefix, rLocalName,xAttrList);
            break;
        case XML_TOK_GROUP_DETAIL:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference<XReportDefinition> m_xComponent = rImport.getReportDefinition();
                pContext = new OXMLSection( rImport, nPrefix, rLocalName,xAttrList ,m_xComponent->getDetail());
            }
            break;

        case XML_TOK_GROUP_FOOTER:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xGroup->setFooterOn(sal_True);
                pContext = new OXMLSection( rImport, nPrefix, rLocalName,xAttrList,m_xGroup->getFooter());
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLGroup::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLGroup::EndElement()
{
    try
    {
        // the group elements end in the reverse order
        m_xGroups->insertByIndex(0,uno::makeAny(m_xGroup));
    }catch(uno::Exception&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
