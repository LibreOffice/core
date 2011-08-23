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

// INCLUDE ---------------------------------------------------------------

#include "xmlconti.hxx"
#include "xmlimprt.hxx"

#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmlnmspe.hxx>
namespace binfilter {

using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLContentContext::ScXMLContentContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ::rtl::OUStringBuffer& sTempValue) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sOUText(),
    sValue(sTempValue)
{
}

ScXMLContentContext::~ScXMLContentContext()
{
}

SvXMLImportContext *ScXMLContentContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && IsXMLToken(rLName, XML_S))
    {
        sal_Int32 nRepeat(0);
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            ::rtl::OUString sValue = xAttrList->getValueByIndex( i );
            ::rtl::OUString aLocalName;
            USHORT nPrfx = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            if ((nPrfx == XML_NAMESPACE_TEXT) && IsXMLToken(aLocalName, XML_C))
                nRepeat = sValue.toInt32();
        }
        if (nRepeat)
            for (sal_Int32 j = 0; j < nRepeat; j++)
                sOUText.append(static_cast<sal_Unicode>(' '));
        else
            sOUText.append(static_cast<sal_Unicode>(' '));
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLContentContext::Characters( const ::rtl::OUString& rChars )
{
    sOUText.append(rChars);
}

void ScXMLContentContext::EndElement()
{
    sValue.append(sOUText);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
