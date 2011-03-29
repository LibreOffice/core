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
#include "precompiled_xmloff.hxx"
#include <SchXMLCalculationSettingsContext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SchXMLCalculationSettingsContext::SchXMLCalculationSettingsContext( SvXMLImport& rImport,
                                    sal_uInt16 p_nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
: SvXMLImportContext ( rImport, p_nPrefix, rLocalName )
{
    const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName(sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_TABLE && IsXMLToken( aLocalName, XML_DATE_VALUE ) )
        {
            util::DateTime aNullDate;
            const rtl::OUString sValue = xAttrList->getValueByIndex( i );
            GetImport().GetMM100UnitConverter().convertDateTime(aNullDate, sValue);
            m_aNullDate <<= aNullDate;
        }
    }
}
SvXMLImportContext* SchXMLCalculationSettingsContext::CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    return new SchXMLCalculationSettingsContext(GetImport(),nPrefix,rLocalName,xAttrList);
}

void SchXMLCalculationSettingsContext::EndElement()
{
    if ( m_aNullDate.hasValue() )
    {
        Reference < XPropertySet > xPropSet ( GetImport().GetModel(), UNO_QUERY );
        ::rtl::OUString sNullDate( RTL_CONSTASCII_USTRINGPARAM ( "NullDate" ) );
        xPropSet->setPropertyValue ( sNullDate, m_aNullDate );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
