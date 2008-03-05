/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyperlinkcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:23:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#include "hyperlinkcontext.hxx"

#include <rtl/ustring.hxx>

#include <com/sun/star/xml/sax/XFastContextHandler.hpp>

#include "oox/helper/propertymap.hxx"
#include "oox/core/relations.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/skipcontext.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"
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
    const OUString& sHref = getRelations().getTargetFromRelId( aRelId );
    if( sHref.getLength() > 0 )
    {
        OSL_TRACE("OOX: URI href %s", ::rtl::OUStringToOString (sHref, RTL_TEXTENCODING_UTF8).pData->buffer);
        const OUString sURL( CREATE_OUSTRING( "URL" ) );
        maProperties[ sURL ] <<= getFilter().getAbsoluteUrl( sHref );
        OUString sTooltip = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_tooltip );
        const OUString sRepresentation( CREATE_OUSTRING( "Representation" ) );
        maProperties[ sRepresentation ] <<= sTooltip;

        OUString sFrame = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_tgtFrame );
        if( sFrame.getLength() )
        {
            const OUString sTargetFrame( CREATE_OUSTRING( "TargetFrame" ) );
            maProperties[ sTargetFrame ] <<= sFrame;
        }

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
        xRet.set( new SkipContext( *this ) );
        break;
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

