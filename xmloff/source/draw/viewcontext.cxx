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
#include "viewcontext.hxx"
#include "sdxmlimp_impl.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "VisAreaContext.hxx"

using namespace com::sun::star;
using ::xmloff::token::IsXMLToken;

using ::xmloff::token::XML_EMBEDDED_VISIBLE_AREA;

using ::rtl::OUString;

//------------------------------------------------------------------

SdXMLViewSettingsContext::SdXMLViewSettingsContext( SdXMLImport& rImport, USHORT nPrfx, const OUString& rLName, const uno::Reference<xml::sax::XAttributeList>& ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SdXMLViewSettingsContext::~SdXMLViewSettingsContext()
{
}

SvXMLImportContext *SdXMLViewSettingsContext::CreateChildContext( USHORT nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_OFFICE)
    {
        if ( IsXMLToken( rLocalName, XML_EMBEDDED_VISIBLE_AREA ) )
        {
            sal_Int16 nMeasureUnit = 0;

            uno::Reference< beans::XPropertySet > xProps( GetImport().GetModel(), uno::UNO_QUERY );
            if( xProps.is() )
                xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MapUnit" ) ) ) >>= nMeasureUnit;

            pContext = new XMLVisAreaContext(GetImport(), nPrefix, rLocalName, xAttrList, maVisArea, nMeasureUnit);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void SdXMLViewSettingsContext::EndElement()
{
    uno::Reference< beans::XPropertySet > xProps( GetImport().GetModel(), uno::UNO_QUERY );
    if( xProps.is() )
    {
        uno::Any aAny;
        aAny <<= maVisArea;

        xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VisibleArea" ) ), aAny );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
