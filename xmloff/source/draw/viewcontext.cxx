/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:30:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _SD_XMLVIEWSETTINGSCONTEXT_HXX
#include "viewcontext.hxx"
#endif
#ifndef _SDXMLIMP_IMPL_HXX
#include "sdxmlimp_impl.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_VISAREACONTEXT_HXX
#include "VisAreaContext.hxx"
#endif

using namespace com::sun::star;
using namespace rtl;
using ::xmloff::token::IsXMLToken;

using ::xmloff::token::XML_EMBEDDED_VISIBLE_AREA;

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
