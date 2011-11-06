/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "viewcontext.hxx"
#include "sdxmlimp_impl.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "xmloff/VisAreaContext.hxx"

using namespace com::sun::star;
using namespace rtl;
using ::xmloff::token::IsXMLToken;

using ::xmloff::token::XML_EMBEDDED_VISIBLE_AREA;

//------------------------------------------------------------------

SdXMLViewSettingsContext::SdXMLViewSettingsContext( SdXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference<xml::sax::XAttributeList>& ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SdXMLViewSettingsContext::~SdXMLViewSettingsContext()
{
}

SvXMLImportContext *SdXMLViewSettingsContext::CreateChildContext( sal_uInt16 nPrefix,
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
