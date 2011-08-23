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

#include <XMLCalculationSettingsContext.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include "xmlimp.hxx"
#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "xmluconv.hxx"
namespace binfilter {


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;

XMLCalculationSettingsContext::XMLCalculationSettingsContext( SvXMLImport& rImport, 
                                    sal_uInt16 nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
: nYear( 1930 )
, SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if ( IsXMLToken( aLocalName, XML_NULL_YEAR ) )
            {
                sal_Int32 nTemp;
                GetImport().GetMM100UnitConverter().convertNumber(nTemp, sValue);
                nYear= static_cast <sal_Int16> (nTemp);
            }
        }
    }
}

XMLCalculationSettingsContext::~XMLCalculationSettingsContext()
{
}
void XMLCalculationSettingsContext::EndElement()
{
    if (nYear != 1930 )
    {
        Reference < XTextDocument > xTextDoc ( GetImport().GetModel(), UNO_QUERY);
        if (xTextDoc.is())
        {
            Reference < XPropertySet > xPropSet ( xTextDoc, UNO_QUERY );
            OUString sTwoDigitYear ( RTL_CONSTASCII_USTRINGPARAM ( "TwoDigitYear" ) );
            Any aAny;
            aAny <<= nYear;
            xPropSet->setPropertyValue ( sTwoDigitYear, aAny );
        }
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
