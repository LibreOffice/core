/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textspacingcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:31:07 $
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

#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/textspacing.hxx"
#include "oox/core/namespaces.hxx"
#include "textspacingcontext.hxx"
#include "tokens.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;


namespace oox { namespace drawingml {

    TextSpacingContext::TextSpacingContext( ContextHandler& rParent, TextSpacing & aSpacing )
        : ContextHandler( rParent )
        , maSpacing( aSpacing )
    {
        maSpacing.bHasValue = sal_True;
    }

    void TextSpacingContext::endFastElement( sal_Int32 /*nElement*/ )
        throw ( SAXException, RuntimeException )
    {
    }

    Reference< XFastContextHandler > TextSpacingContext::createFastChildContext( ::sal_Int32 aElement,
                                                                                                                                                        const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;
        switch( aElement )
        {
        case NMSP_DRAWINGML|XML_spcPct:
            maSpacing.nUnit = TextSpacing::PERCENT;
            maSpacing.nValue = GetPercent( xAttribs->getValue( XML_val ) );
            break;
        case NMSP_DRAWINGML|XML_spcPts:
            maSpacing.nUnit = TextSpacing::POINTS;
            maSpacing.nValue = GetTextSpacingPoint( xAttribs->getValue( XML_val ) );
            break;
        default:
            break;
        }
        if ( !xRet.is() )
            xRet.set( this );
        return xRet;
    }


} }
