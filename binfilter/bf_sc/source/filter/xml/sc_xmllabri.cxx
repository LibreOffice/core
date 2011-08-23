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

#ifdef _MSC_VER
#pragma hdrstop
#endif

//___________________________________________________________________

#include "xmllabri.hxx"

#include <bf_xmloff/nmspmap.hxx>

#include <com/sun/star/sheet/XLabelRanges.hpp>

#include "XMLConverter.hxx"
#include "unonames.hxx"
#include "xmlimprt.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace xmloff::token;


//___________________________________________________________________

ScXMLLabelRangesContext::ScXMLLabelRangesContext(
        ScXMLImport& rImport,
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ):
    SvXMLImportContext( rImport, nPrefix, rLName )
{
    rImport.LockSolarMutex();
}

ScXMLLabelRangesContext::~ScXMLLabelRangesContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext* ScXMLLabelRangesContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext*		pContext	= NULL;
    const SvXMLTokenMap&	rTokenMap	= GetScImport().GetLabelRangesElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_LABEL_RANGE_ELEM:
            pContext = new ScXMLLabelRangeContext( GetScImport(), nPrefix, rLName, xAttrList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLLabelRangesContext::EndElement()
{
}


//___________________________________________________________________

ScXMLLabelRangeContext::ScXMLLabelRangeContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bColumnOrientation( sal_False )
{
    sal_Int16				nAttrCount		= xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap&	rAttrTokenMap	= GetScImport().GetLabelRangeAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString	sAttrName	= xAttrList->getNameByIndex( nIndex );
        OUString	sValue		= xAttrList->getValueByIndex( nIndex );
        OUString	aLocalName;
        USHORT		nPrefix		= GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE:
                sLabelRangeStr = sValue;
            break;
            case XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE:
                sDataRangeStr = sValue;
            break;
            case XML_TOK_LABEL_RANGE_ATTR_ORIENTATION:
                bColumnOrientation = IsXMLToken(sValue, XML_COLUMN );
            break;
        }
    }
}

ScXMLLabelRangeContext::~ScXMLLabelRangeContext()
{
}

SvXMLImportContext* ScXMLLabelRangeContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLLabelRangeContext::EndElement()
{
    if (GetScImport().GetModel().is())
    {
        uno::Reference< beans::XPropertySet > xPropSet( GetScImport().GetModel(), uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            uno::Any aAny = xPropSet->getPropertyValue( bColumnOrientation ?
                OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLLABELRNG ) ) :
                OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_ROWLABELRNG ) ) );
            uno::Reference< sheet::XLabelRanges > xLabelRanges;
            if( aAny >>= xLabelRanges )
            {
                table::CellRangeAddress aLabelRange;
                table::CellRangeAddress aDataRange;
                sal_Int32 nOffset1(0);
                sal_Int32 nOffset2(0);
                if (ScXMLConverter::GetRangeFromString( aLabelRange, sLabelRangeStr, GetScImport().GetDocument(), nOffset1 ) &&
                    ScXMLConverter::GetRangeFromString( aDataRange, sDataRangeStr, GetScImport().GetDocument(), nOffset2 ))
                    xLabelRanges->addNew( aLabelRange, aDataRange );
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
