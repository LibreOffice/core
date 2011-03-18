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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
#include "XMLCalculationSettingsContext.hxx"
#include "xmlimprt.hxx"
#include "unonames.hxx"
#include "docoptio.hxx"
#include "document.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <comphelper/extract.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLCalculationSettingsContext::ScXMLCalculationSettingsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    fIterationEpsilon(0.001),
    nIterationCount(100),
    nYear2000(1930),
    bIsIterationEnabled(false),
    bCalcAsShown(false),
    bIgnoreCase(false),
    bLookUpLabels(sal_True),
    bMatchWholeCell(sal_True),
    bUseRegularExpressions(sal_True)
{
    aNullDate.Day = 30;
    aNullDate.Month = 12;
    aNullDate.Year = 1899;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_CASE_SENSITIVE))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bIgnoreCase = sal_True;
            }
            else if (IsXMLToken(aLocalName, XML_PRECISION_AS_SHOWN))
            {
                if (IsXMLToken(sValue, XML_TRUE))
                    bCalcAsShown = sal_True;
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
                GetScImport().GetMM100UnitConverter().convertNumber(nTemp, sValue);
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
                                            const ::rtl::OUString& rLName,
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
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_CALCASSHOWN)), uno::makeAny(bCalcAsShown) );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_IGNORECASE)), uno::makeAny(bIgnoreCase) );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LOOKUPLABELS)), uno::makeAny(bLookUpLabels) );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_MATCHWHOLE)), uno::makeAny(bMatchWholeCell) );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_REGEXENABLED)), uno::makeAny(bUseRegularExpressions) );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERENABLED)), uno::makeAny(bIsIterationEnabled) );
            xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERCOUNT)), uno::makeAny(nIterationCount) );
            xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITEREPSILON)), uno::makeAny(fIterationEpsilon) );
            xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NULLDATE)), uno::makeAny(aNullDate) );
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
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLCalculationSettingsContext* pCalcSet) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE && IsXMLToken(aLocalName, XML_DATE_VALUE))
        {
            util::DateTime aDateTime;
            GetScImport().GetMM100UnitConverter().convertDateTime(aDateTime, sValue);
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
                                            const ::rtl::OUString& rLName,
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
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLCalculationSettingsContext* pCalcSet) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_STATUS))
            {
                if (IsXMLToken(sValue, XML_ENABLE))
                    pCalcSet->SetIterationStatus(sal_True);
            }
            else if (IsXMLToken(aLocalName, XML_STEPS))
            {
                sal_Int32 nSteps;
                GetScImport().GetMM100UnitConverter().convertNumber(nSteps, sValue);
                pCalcSet->SetIterationCount(nSteps);
            }
            else if (IsXMLToken(aLocalName, XML_MAXIMUM_DIFFERENCE))
            {
                double fDif;
                GetScImport().GetMM100UnitConverter().convertDouble(fDif, sValue);
                pCalcSet->SetIterationEpsilon(fDif);
            }
        }
    }
}

ScXMLIterationContext::~ScXMLIterationContext()
{
}

SvXMLImportContext *ScXMLIterationContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
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
