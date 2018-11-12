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
#include "xmlFunction.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlHelper.hxx"
#include "xmlEnums.hxx"
#include <strings.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;


OXMLFunction::OXMLFunction( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XFunctionsSupplier >& _xFunctions
                ,bool _bAddToReport
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xFunctions(_xFunctions->getFunctions())
    ,m_bAddToReport(_bAddToReport)
{

    OSL_ENSURE(m_xFunctions.is(),"Functions is NULL!");
    m_xFunction = m_xFunctions->createFunction();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetFunctionElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        const OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const OUString sValue = _xAttrList->getValueByIndex( i );

        try
        {
            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_FUNCTION_NAME:
                    m_xFunction->setName(sValue);
                    break;
                case XML_TOK_FUNCTION_FORMULA:
                    m_xFunction->setFormula(ORptFilter::convertFormula(sValue));
                    break;
                case XML_TOK_PRE_EVALUATED:
                    m_xFunction->setPreEvaluated(sValue == s_sTRUE);
                    break;
                case XML_TOK_INITIAL_FORMULA:
                    if ( !sValue.isEmpty() )
                        m_xFunction->setInitialFormula(beans::Optional< OUString>(true,ORptFilter::convertFormula(sValue)));
                    break;
                case XML_TOK_DEEP_TRAVERSING:
                    m_xFunction->setDeepTraversing(sValue == s_sTRUE);
                    break;
                default:
                    break;
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("Exception caught while putting Function props!");
        }
    }
}


OXMLFunction::~OXMLFunction()
{
}

ORptFilter& OXMLFunction::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

void OXMLFunction::EndElement()
{
    if ( m_bAddToReport )
    {
        GetOwnImport().insertFunction(m_xFunction);
        m_xFunction.clear();
    }
    else
    {
        try
        {
            m_xFunctions->insertByIndex(m_xFunctions->getCount(),uno::makeAny(m_xFunction));
            m_xFunction.clear();
        }catch(uno::Exception&)
        {
            OSL_FAIL("Exception caught!");
        }
    }
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
