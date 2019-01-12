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
#include "xmlReportElement.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include "xmlComponent.hxx"
#include "xmlCondPrtExpr.hxx"
#include "xmlFormatCondition.hxx"


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;

OXMLReportElement::OXMLReportElement( ORptFilter& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XReportControlModel > & _xComponent) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_xComponent(_xComponent)
{

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetReportElementElemTokenMap();

    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            OUString sLocalName;
            const OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_PRINT_WHEN_GROUP_CHANGE:
                    m_xComponent->setPrintWhenGroupChange(s_sTRUE == sValue);
                    break;
                case XML_TOK_PRINT_REPEATED_VALUES:
                    m_xComponent->setPrintRepeatedValues(sValue == s_sTRUE);
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception caught while filling the report definition props");
    }
}


OXMLReportElement::~OXMLReportElement()
{
}

SvXMLImportContextRef OXMLReportElement::CreateChildContext(
        sal_uInt16 _nPrefix,
        const OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
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

ORptFilter& OXMLReportElement::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
