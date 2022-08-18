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
#include <xmloff/xmlnamespace.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;


OXMLFunction::OXMLFunction( ORptFilter& _rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XFunctionsSupplier >& _xFunctions
                ,bool _bAddToReport
                ) :
    SvXMLImportContext( _rImport )
    ,m_xFunctions(_xFunctions->getFunctions())
    ,m_bAddToReport(_bAddToReport)
{

    OSL_ENSURE(m_xFunctions.is(),"Functions is NULL!");
    m_xFunction = m_xFunctions->createFunction();

    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        try
        {
            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_NAME):
                    m_xFunction->setName(aIter.toString());
                    break;
                case XML_ELEMENT(REPORT, XML_FORMULA):
                    m_xFunction->setFormula(ORptFilter::convertFormula(aIter.toString()));
                    break;
                case XML_ELEMENT(REPORT, XML_PRE_EVALUATED):
                    m_xFunction->setPreEvaluated(IsXMLToken(aIter, XML_TRUE));
                    break;
                case XML_ELEMENT(REPORT, XML_INITIAL_FORMULA):
                    if ( !aIter.isEmpty() )
                        m_xFunction->setInitialFormula(beans::Optional< OUString>(true,ORptFilter::convertFormula(aIter.toString())));
                    break;
                case XML_ELEMENT(REPORT, XML_DEEP_TRAVERSING):
                    m_xFunction->setDeepTraversing(IsXMLToken(aIter, XML_TRUE));
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("reportdesign", aIter);
                    break;
            }
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "reportdesign", "Exception caught while putting Function props!");
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

void OXMLFunction::endFastElement(sal_Int32 )
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
            m_xFunctions->insertByIndex(m_xFunctions->getCount(),uno::Any(m_xFunction));
            m_xFunction.clear();
        }catch(uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION( "reportdesign", "");
        }
    }
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
