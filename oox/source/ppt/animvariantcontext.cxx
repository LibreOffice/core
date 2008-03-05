/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animvariantcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:43:44 $
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

#include "animvariantcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "pptfilterhelpers.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    AnimVariantContext::AnimVariantContext( ContextHandler& rParent, sal_Int32 aElement, Any & aValue )
        : ContextHandler( rParent )
            , mnElement( aElement )
            , maValue( aValue )
    {
    }

    AnimVariantContext::~AnimVariantContext( ) throw( )
    {
    }

    void SAL_CALL AnimVariantContext::endFastElement( sal_Int32 aElement )
        throw ( SAXException, RuntimeException)
    {
        if( ( aElement == mnElement ) && maColor.isUsed() )
        {
            maValue = makeAny( maColor.getColor( getFilter() ) );
        }
    }


    Reference< XFastContextHandler >
    SAL_CALL AnimVariantContext::createFastChildContext( ::sal_Int32 aElementToken,
                             const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;
        AttributeList attribs(xAttribs);

        switch( aElementToken )
        {
        case NMSP_PPT|XML_boolVal:
        {
            bool val = attribs.getBool( XML_val, false );
            maValue = makeAny( val );
            break;
        }
        case NMSP_PPT|XML_clrVal:
            xRet.set( new ::oox::drawingml::colorChoiceContext( *this, maColor ) );
            // we'll defer setting the Any until the end.
            break;
        case NMSP_PPT|XML_fltVal:
        {
            double val = attribs.getDouble( XML_val, 0.0 );
            maValue = makeAny( val );
            break;
        }
        case NMSP_PPT|XML_intVal:
        {
            sal_Int32 val = attribs.getInteger( XML_val, 0 );
            maValue = makeAny( val );
            break;
        }
        case NMSP_PPT|XML_strVal:
        {
            OUString val = attribs.getString( XML_val );
            convertMeasure( val ); // ignore success or failure if it fails, use as is
            maValue = makeAny( val );
            break;
        }
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }



} }
