/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "xmlGroup.hxx"
#include "xmlSection.hxx"
#include "xmlFunction.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlHelper.hxx"
#include "xmlEnums.hxx"
#include <ucbhelper/content.hxx>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/report/KeepTogether.hpp>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;

    static sal_Int16 lcl_getKeepTogetherOption(const OUString& _sValue)
    {
        sal_Int16 nRet = report::KeepTogether::NO;
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetKeepTogetherOptions();
        (void)SvXMLUnitConverter::convertEnum( nRet, _sValue, aXML_EnumMap );
        return nRet;
    }

OXMLGroup::OXMLGroup( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
{

    m_xGroups = _rImport.getReportDefinition()->getGroups();
    OSL_ENSURE(m_xGroups.is(),"Groups is NULL!");
    m_xGroup = m_xGroups->createGroup();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetGroupElemTokenMap();
    m_xGroup->setSortAscending(false);// the default value has to be set
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        const OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

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

                            static const char s_sChanged[] = "rpt:HASCHANGED(\"";
                            sal_Int32 nPos = sValue.indexOf(s_sChanged);
                            if ( nPos == -1 )
                                nPos = 5;
                            else
                            {
                                nPos = strlen(s_sChanged);
                                static const char s_sQuote[] = "\"\"";
                                sal_Int32 nIndex = sValue.indexOf(s_sQuote,nPos);
                                while ( nIndex > -1 )
                                {
                                    sValue = sValue.replaceAt(nIndex,2, "\"");
                                    nIndex = sValue.indexOf(s_sQuote,nIndex+2);
                                }
                                nLen = sValue.getLength() - 1;
                            }
                            sValue = sValue.copy(nPos,nLen-nPos-1);
                            const ORptFilter::TGroupFunctionMap& aFunctions = _rImport.getFunctions();
                            ORptFilter::TGroupFunctionMap::const_iterator aFind = aFunctions.find(sValue);
                            if ( aFind != aFunctions.end() )
                            {
                                const OUString sCompleteFormula = aFind->second->getFormula();
                                OUString sExpression = sCompleteFormula.getToken(1,'[');
                                sExpression = sExpression.getToken(0,']');
                                sal_Int32 nIndex = 0;
                                const OUString sFormula = sCompleteFormula.getToken(0,'(',nIndex);
                                ::sal_Int16 nGroupOn = report::GroupOn::DEFAULT;

                                if ( sFormula == "rpt:LEFT")
                                {
                                    nGroupOn = report::GroupOn::PREFIX_CHARACTERS;
                                    OUString sInterval = sCompleteFormula.getToken(1,';',nIndex);
                                    sInterval = sInterval.getToken(0,')');
                                    m_xGroup->setGroupInterval(sInterval.toInt32());
                                }
                                else if ( sFormula == "rpt:YEAR")
                                    nGroupOn = report::GroupOn::YEAR;
                                else if ( sFormula == "rpt:MONTH")
                                {
                                    nGroupOn = report::GroupOn::MONTH;
                                }
                                else if ( sCompleteFormula.matchIgnoreAsciiCase("rpt:INT((MONTH",0)
                                       && sCompleteFormula.endsWithIgnoreAsciiCase("-1)/3)+1") )
                                {
                                    nGroupOn = report::GroupOn::QUARTAL;
                                }
                                else if ( sFormula == "rpt:WEEK")
                                    nGroupOn = report::GroupOn::WEEK;
                                else if ( sFormula == "rpt:DAY")
                                    nGroupOn = report::GroupOn::DAY;
                                else if ( sFormula == "rpt:HOUR")
                                    nGroupOn = report::GroupOn::HOUR;
                                else if ( sFormula == "rpt:MINUTE")
                                    nGroupOn = report::GroupOn::MINUTE;
                                else if ( sFormula == "rpt:INT")
                                {
                                    nGroupOn = report::GroupOn::INTERVAL;
                                    _rImport.removeFunction(sExpression);
                                    sExpression = sExpression.copy(OUString("INT_count_").getLength());
                                    OUString sInterval = sCompleteFormula.getToken(1,'/');
                                    sInterval = sInterval.getToken(0,')');
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
            OSL_FAIL("Exception caught while putting group props!");
        }
    }
}


OXMLGroup::~OXMLGroup()
{

}

SvXMLImportContextRef OXMLGroup::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
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
                m_xGroup->setHeaderOn(true);
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
                Reference<XReportDefinition> xComponent = rImport.getReportDefinition();
                pContext = new OXMLSection( rImport, nPrefix, rLocalName,xAttrList, xComponent->getDetail());
            }
            break;

        case XML_TOK_GROUP_FOOTER:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                m_xGroup->setFooterOn(true);
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

ORptFilter& OXMLGroup::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

void OXMLGroup::EndElement()
{
    try
    {
        // the group elements end in the reverse order
        m_xGroups->insertByIndex(0,uno::makeAny(m_xGroup));
    }catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
