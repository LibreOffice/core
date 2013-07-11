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

#include "XMLCalculationSettingsContext.hxx"
#include "xmlimprt.hxx"
#include "unonames.hxx"
#include "docoptio.hxx"
#include "document.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <comphelper/extract.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLCalculationSettingsContext::ScXMLCalculationSettingsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    fIterationEpsilon(0.001),
    nIterationCount(100),
    nYear2000(1930),
    bIsIterationEnabled(false),
    bCalcAsShown(false),
    bIgnoreCase(false),
    bLookUpLabels(true),
    bMatchWholeCell(true),
    bUseRegularExpressions(true)
{
    aNullDate.Day = 30;
    aNullDate.Month = 12;
    aNullDate.Year = 1899;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_CASE_SENSITIVE))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bIgnoreCase = true;
            }
            else if (IsXMLToken(aLocalName, XML_PRECISION_AS_SHOWN))
            {
                if (IsXMLToken(sValue, XML_TRUE))
                    bCalcAsShown = true;
            }
            else if (IsXMLToken(aLocalName, XML_SEARCH_CRITERIA_MUST_APPLY_TO_WHOLE_CELL))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bMatchWholeCell = false;
            }
            else if (IsXMLToken(aLocalName, XML_AUTOMATIC_FIND_LABELS))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bLookUpLabels = false;
            }
            else if (IsXMLToken(aLocalName, XML_NULL_YEAR))
            {
                sal_Int32 nTemp;
                ::sax::Converter::convertNumber(nTemp, sValue);
                nYear2000 = static_cast<sal_uInt16>(nTemp);
            }
            else if (IsXMLToken(aLocalName, XML_USE_REGULAR_EXPRESSIONS))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bUseRegularExpressions = false;
            }
        }
    }
}

ScXMLCalculationSettingsContext::~ScXMLCalculationSettingsContext()
{
}

SvXMLImportContext *ScXMLCalculationSettingsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLName, XML_NULL_DATE))
            pContext = new ScXMLNullDateContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        else if (IsXMLToken(rLName, XML_ITERATION))
            pContext = new ScXMLIterationContext(GetScImport(), nPrefix, rLName, xAttrList, this);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLCalculationSettingsContext::EndElement()
{
    if (GetScImport().GetModel().is())
    {
        uno::Reference <beans::XPropertySet> xPropertySet (GetScImport().GetModel(), uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            xPropertySet->setPropertyValue(OUString(SC_UNO_CALCASSHOWN), uno::makeAny(bCalcAsShown) );
            xPropertySet->setPropertyValue(OUString(SC_UNO_IGNORECASE), uno::makeAny(bIgnoreCase) );
            xPropertySet->setPropertyValue(OUString(SC_UNO_LOOKUPLABELS), uno::makeAny(bLookUpLabels) );
            xPropertySet->setPropertyValue(OUString(SC_UNO_MATCHWHOLE), uno::makeAny(bMatchWholeCell) );
            xPropertySet->setPropertyValue(OUString(SC_UNO_REGEXENABLED), uno::makeAny(bUseRegularExpressions) );
            xPropertySet->setPropertyValue(OUString(SC_UNO_ITERENABLED), uno::makeAny(bIsIterationEnabled) );
            xPropertySet->setPropertyValue( OUString(SC_UNO_ITERCOUNT), uno::makeAny(nIterationCount) );
            xPropertySet->setPropertyValue( OUString(SC_UNO_ITEREPSILON), uno::makeAny(fIterationEpsilon) );
            xPropertySet->setPropertyValue( OUString(SC_UNO_NULLDATE), uno::makeAny(aNullDate) );
            if (GetScImport().GetDocument())
            {
                ScXMLImport::MutexGuard aGuard(GetScImport());
                ScDocOptions aDocOptions (GetScImport().GetDocument()->GetDocOptions());
                aDocOptions.SetYear2000(nYear2000);
                GetScImport().GetDocument()->SetDocOptions(aDocOptions);
            }
        }
    }
}

ScXMLNullDateContext::ScXMLNullDateContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLCalculationSettingsContext* pCalcSet) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE && IsXMLToken(aLocalName, XML_DATE_VALUE))
        {
            util::DateTime aDateTime;
            ::sax::Converter::parseDateTime(aDateTime, 0, sValue);
            util::Date aDate;
            aDate.Day = aDateTime.Day;
            aDate.Month = aDateTime.Month;
            aDate.Year = aDateTime.Year;
            pCalcSet->SetNullDate(aDate);
        }
    }
}

ScXMLNullDateContext::~ScXMLNullDateContext()
{
}

SvXMLImportContext *ScXMLNullDateContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLNullDateContext::EndElement()
{
}

ScXMLIterationContext::ScXMLIterationContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLCalculationSettingsContext* pCalcSet) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_STATUS))
            {
                if (IsXMLToken(sValue, XML_ENABLE))
                    pCalcSet->SetIterationStatus(true);
            }
            else if (IsXMLToken(aLocalName, XML_STEPS))
            {
                sal_Int32 nSteps;
                ::sax::Converter::convertNumber(nSteps, sValue);
                pCalcSet->SetIterationCount(nSteps);
            }
            else if (IsXMLToken(aLocalName, XML_MAXIMUM_DIFFERENCE))
            {
                double fDif;
                ::sax::Converter::convertDouble(fDif, sValue);
                pCalcSet->SetIterationEpsilon(fDif);
            }
        }
    }
}

ScXMLIterationContext::~ScXMLIterationContext()
{
}

SvXMLImportContext *ScXMLIterationContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLIterationContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
