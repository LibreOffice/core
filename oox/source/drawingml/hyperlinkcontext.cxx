/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hyperlinkcontext.cxx,v $
 * $Revision: 1.4 $
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

#include "hyperlinkcontext.hxx"

#include <rtl/ustring.hxx>

#include <com/sun/star/xml/sax/XFastContextHandler.hpp>

#include "oox/helper/propertymap.hxx"
#include "oox/core/relations.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace drawingml {

HyperLinkContext::HyperLinkContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& xAttributes, PropertyMap& aProperties )
    : ContextHandler( rParent )
    , maProperties(aProperties)
{
    OUString aRelId = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_id );
    OSL_TRACE("OOX: URI rId %s", ::rtl::OUStringToOString (aRelId, RTL_TEXTENCODING_UTF8).pData->buffer);
    const OUString& sHref = getRelations().getExternalTargetFromRelId( aRelId );
    if( sHref.getLength() > 0 )
    {
        OSL_TRACE("OOX: URI href %s", ::rtl::OUStringToOString (sHref, RTL_TEXTENCODING_UTF8).pData->buffer);
        maProperties[ PROP_URL ] <<= getFilter().getAbsoluteUrl( sHref );
        OUString sTooltip = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_tooltip );
        maProperties[ PROP_Representation ] <<= sTooltip;

        OUString sFrame = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_tgtFrame );
        if( sFrame.getLength() )
            maProperties[ PROP_TargetFrame ] <<= sFrame;

//              sValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) );
//              const rtl::OUString sUnvisitedCharStyleName( CREATE_OUSTRING( "UnvisitedCharStyleName" ) );
//              maProperties[ sUnvisitedCharStyleName ] <<= sValue;
//              const rtl::OUString sVisitedCharStyleName( CREATE_OUSTRING( "VisitedCharStyleName" ) );
//              maProperties[ sVisitedCharStyleName ] <<= sValue;

    }
    // TODO unhandled
    // XML_invalidUrl
    // XML_history
    // XML_highlightClick
    // XML_endSnd
    // XML_action
}

HyperLinkContext::~HyperLinkContext()
{
}

Reference< XFastContextHandler > HyperLinkContext::createFastChildContext(
        ::sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElement )
    {
    case NMSP_DRAWINGML|XML_extLst:
        return xRet;
    case NMSP_DRAWINGML|XML_snd:
        EmbeddedWAVAudioFile aAudio;
        getEmbeddedWAVAudioFile( getRelations(), xAttribs, aAudio );
        break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

} // namespace drawingml
} // namespace oox

