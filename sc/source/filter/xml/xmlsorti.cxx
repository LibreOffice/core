/*************************************************************************
 *
 *  $RCSfile: xmlsorti.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: sab $ $Date: 2001-04-03 06:06:30 $
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

#include "xmlsorti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"

#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#define SC_USERLIST "UserList"

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLSortContext::ScXMLSortContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    bEnabledUserList(sal_False),
    bBindFormatsToContent(sal_True),
    bIsCaseSensitive(sal_False),
    bCopyOutputData(sal_False),
    sCountry(),
    sLanguage(),
    sAlgorithm(),
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    nUserListIndex = 0;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSortAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT :
            {
                if (sValue.compareToAscii(sXML_false) == 0)
                    bBindFormatsToContent = sal_False;
            }
            break;
            case XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                sal_Int32 nOffset(0);
                if (ScXMLConverter::GetRangeFromString( aScRange, sValue, GetScImport().GetDocument(), nOffset ))
                {
                    ScUnoConversion::FillApiAddress( aOutputPosition, aScRange.aStart );
                    bCopyOutputData = sal_True;
                }
            }
            break;
            case XML_TOK_SORT_ATTR_CASE_SENSITIVE :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bIsCaseSensitive = sal_True;
            }
            break;
            case XML_TOK_SORT_ATTR_LANGUAGE :
                sLanguage = sValue;
            break;
            case XML_TOK_SORT_ATTR_COUNTRY :
                sCountry = sValue;
            break;
            case XML_TOK_SORT_ATTR_ALGORITHM :
                sCountry = sAlgorithm;
            break;
        }
    }
}

ScXMLSortContext::~ScXMLSortContext()
{
}

SvXMLImportContext *ScXMLSortContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetSortElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SORT_SORT_BY :
        {
            pContext = new ScXMLSortByContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSortContext::EndElement()
{
    sal_Int32 nLangLength(sLanguage.getLength());
    sal_Int32 nCountryLength(sCountry.getLength());
    sal_Int32 nAlgoLength(sAlgorithm.getLength());
    sal_uInt8 i (0);
    if (nLangLength || nCountryLength)
        i++;
    if (nAlgoLength)
        i++;
    uno::Sequence <beans::PropertyValue> aSortDescriptor(7 + i);
    uno::Any aTemp;
    beans::PropertyValue aPropertyValue;
    aTemp = ::cppu::bool2any(bBindFormatsToContent);
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_BINDFMT);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[0] = aPropertyValue;
    aTemp = ::cppu::bool2any(bCopyOutputData);
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_COPYOUT);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[1] = aPropertyValue;
    aTemp = ::cppu::bool2any(bIsCaseSensitive);
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_ISCASE);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[2] = aPropertyValue;
    aTemp = ::cppu::bool2any(bEnabledUserList);
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_ISULIST);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[3] = aPropertyValue;
    aTemp <<= aOutputPosition;
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_OUTPOS);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[4] = aPropertyValue;
    aTemp <<= nUserListIndex;
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_UINDEX);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[5] = aPropertyValue;
    aTemp <<= aSortFields;
    aPropertyValue.Name = rtl::OUString::createFromAscii(SC_UNONAME_SORTFLD);
    aPropertyValue.Value = aTemp;
    aSortDescriptor[6] = aPropertyValue;
    if (nLangLength || nCountryLength)
    {
        lang::Locale aLocale;
        aLocale.Language = sLanguage;
        aLocale.Country = sCountry;
        aTemp <<= aLocale;
        aPropertyValue.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COLLLOC));
        aPropertyValue.Value = aTemp;
        aSortDescriptor[7] = aPropertyValue;
    }
    if (nAlgoLength)
    {
        aTemp <<= sAlgorithm;
        aPropertyValue.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COLLALG));
        aPropertyValue.Value = aTemp;
        aSortDescriptor[6 + i] = aPropertyValue;
    }
    pDatabaseRangeContext->SetSortSequence(aSortDescriptor);
}

void ScXMLSortContext::AddSortField(const rtl::OUString& sFieldNumber, const rtl::OUString& sDataType, const rtl::OUString& sOrder)
{
    util::SortField aSortField;
    aSortField.Field = sFieldNumber.toInt32();
    if (sOrder.compareToAscii(sXML_ascending) == 0)
        aSortField.SortAscending = sal_True;
    else
        aSortField.SortAscending = sal_False;
    if (sDataType.getLength() > 8)
    {
        rtl::OUString sTemp = sDataType.copy(0, 8);
        if (sTemp.compareToAscii(SC_USERLIST) == 0)
        {
            bEnabledUserList = sal_True;
            sTemp = sDataType.copy(8);
            nUserListIndex = static_cast<sal_Int16>(sTemp.toInt32());
        }
        else
        {
            if (sDataType.compareToAscii(sXML_automatic) == 0)
                aSortField.FieldType = util::SortFieldType_AUTOMATIC;
        }
    }
    else
    {
        if (sDataType.compareToAscii(sXML_text) == 0)
            aSortField.FieldType = util::SortFieldType_ALPHANUMERIC;
        else if (sDataType.compareToAscii(sXML_number) == 0)
            aSortField.FieldType = util::SortFieldType_NUMERIC;
    }
    aSortFields.realloc(aSortFields.getLength() + 1);
    aSortFields[aSortFields.getLength() - 1] = aSortField;
}

ScXMLSortByContext::ScXMLSortByContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLSortContext* pTempSortContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sOrder = rtl::OUString::createFromAscii(sXML_ascending);
    sDataType = rtl::OUString::createFromAscii(sXML_automatic);
    pSortContext = pTempSortContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSortSortByAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SORT_BY_ATTR_FIELD_NUMBER :
            {
                sFieldNumber = sValue;
            }
            break;
            case XML_TOK_SORT_BY_ATTR_DATA_TYPE :
            {
                sDataType = sValue;
            }
            break;
            case XML_TOK_SORT_BY_ATTR_ORDER :
            {
                sOrder = sValue;
            }
            break;
        }
    }
}

ScXMLSortByContext::~ScXMLSortByContext()
{
}

SvXMLImportContext *ScXMLSortByContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSortByContext::EndElement()
{
    pSortContext->AddSortField(sFieldNumber, sDataType, sOrder);
}

