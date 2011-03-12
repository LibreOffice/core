/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"
#include "xmlFunction.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlHelper.hxx"
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME( rpt_OXMLFunction )

OXMLFunction::OXMLFunction( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XFunctionsSupplier >& _xFunctions
                ,bool _bAddToReport
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xFunctions(_xFunctions->getFunctions())
    ,m_bAddToReport(_bAddToReport)
{
    DBG_CTOR( rpt_OXMLFunction,NULL);

    OSL_ENSURE(m_xFunctions.is(),"Functions is NULL!");
    m_xFunction = m_xFunctions->createFunction();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetFunctionElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
     ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

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
                    if ( sValue.getLength() )
                        m_xFunction->setInitialFormula(beans::Optional< ::rtl::OUString>(sal_True,ORptFilter::convertFormula(sValue)));
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
            OSL_FAIL("Exception catched while putting Function props!");
        }
    }
}
// -----------------------------------------------------------------------------

OXMLFunction::~OXMLFunction()
{
    DBG_DTOR( rpt_OXMLFunction,NULL);
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLFunction::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
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
            OSL_FAIL("Exception catched!");
        }
    }
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
