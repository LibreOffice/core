/*************************************************************************
 *
 *  $RCSfile: XMLCalculationSettingsContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-23 14:58:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLCALCULATIONSETTINGSCONTEXT_HXX
#include "XMLCalculationSettingsContext.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLCalculationSettingsContext::ScXMLCalculationSettingsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    fIterationEpsilon(0.001),
    nIterationCount(100),
    bIsIterationEnabled(sal_False),
    bCalcAsShown(sal_False),
    bIgnoreCase(sal_True),
    bLookUpLabels(sal_True),
    bMatchWholeCell(sal_True)
{
    aNullDate.Day = 30;
    aNullDate.Month = 12;
    aNullDate.Year = 1899;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableRowCellAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_case_sensitive) == 0)
            {
                if (sValue.compareToAscii(sXML_false) == 0)
                    bIgnoreCase = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_precision_as_shown) == 0)
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bCalcAsShown = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_search_criteria_must_apply_to_whole_cell) == 0)
            {
                if (sValue.compareToAscii(sXML_false) == 0)
                    bMatchWholeCell = sal_False;
            }
            else if (aLocalName.compareToAscii(sXML_automatic_find_labels) == 0)
            {
                if (sValue.compareToAscii(sXML_false) == 0)
                    bLookUpLabels = sal_False;
            }
        }
    }
}

ScXMLCalculationSettingsContext::~ScXMLCalculationSettingsContext()
{
}

SvXMLImportContext *ScXMLCalculationSettingsContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (rLName.compareToAscii(sXML_null_date) == 0)
            pContext = new ScXMLNullDateContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        else if (rLName.compareToAscii(sXML_iteration) == 0)
            pContext = new ScXMLIterationContext(GetScImport(), nPrefix, rLName, xAttrList, this);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLCalculationSettingsContext::EndElement()
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc (GetScImport().GetModel(), uno::UNO_QUERY);
    if (xSpreadDoc.is())
    {
        uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aAny = ::cppu::bool2any( bCalcAsShown );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_CALCASSHOWN)), aAny );
            aAny = ::cppu::bool2any( bIgnoreCase );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_IGNORECASE)), aAny );
            aAny = ::cppu::bool2any( bLookUpLabels );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LOOKUPLABELS)), aAny );
            aAny = ::cppu::bool2any( bMatchWholeCell );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_MATCHWHOLE)), aAny );
            aAny = ::cppu::bool2any( bIsIterationEnabled );
            xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERENABLED)), aAny );
            aAny <<= nIterationCount;
            xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERCOUNT)), aAny);
            aAny <<= fIterationEpsilon;
            xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITEREPSILON)), aAny);
            aAny <<= aNullDate;
            xPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NULLDATE)), aAny);
        }
    }
}

ScXMLNullDateContext::ScXMLNullDateContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLCalculationSettingsContext* pCalcSet) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableRowCellAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE && aLocalName.compareToAscii(sXML_date_value) == 0)
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

SvXMLImportContext *ScXMLNullDateContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLNullDateContext::EndElement()
{
}

ScXMLIterationContext::ScXMLIterationContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLCalculationSettingsContext* pCalcSet) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableRowCellAttrTokenMap();

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (aLocalName.compareToAscii(sXML_status) == 0)
            {
                if (sValue.compareToAscii("enable") == 0)
                    pCalcSet->SetIterationStatus(sal_True);
            }
            else if (aLocalName.compareToAscii(sXML_steps) == 0)
            {
                sal_Int32 nSteps;
                GetScImport().GetMM100UnitConverter().convertNumber(nSteps, sValue);
                pCalcSet->SetIterationCount(nSteps);
            }
            else if (aLocalName.compareToAscii(sXML_maximum_difference) == 0)
            {
                double fDif;
                GetScImport().GetMM100UnitConverter().convertNumber(fDif, sValue);
                pCalcSet->SetIterationEpsilon(fDif);
            }
        }
    }
}

ScXMLIterationContext::~ScXMLIterationContext()
{
}

SvXMLImportContext *ScXMLIterationContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLIterationContext::EndElement()
{
}
