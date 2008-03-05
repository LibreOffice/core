/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: timeanimvaluecontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:51:27 $
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

#include "timeanimvaluecontext.hxx"

#include "oox/core/namespaces.hxx"
#include "animvariantcontext.hxx"

#include "tokens.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    TimeAnimValueListContext::TimeAnimValueListContext( ContextHandler& rParent,
                const Reference< XFastAttributeList >& /*xAttribs*/,
                TimeAnimationValueList & aTavList )
        : ContextHandler( rParent )
            , maTavList( aTavList )
            , mbInValue( false )
    {
    }


    TimeAnimValueListContext::~TimeAnimValueListContext( )
    {
    }


    void SAL_CALL TimeAnimValueListContext::endFastElement( sal_Int32 aElement )
        throw ( SAXException, RuntimeException)
    {
        if( aElement == ( NMSP_PPT|XML_tav ) )
        {
            mbInValue = false;
        }
    }


    Reference< XFastContextHandler > SAL_CALL TimeAnimValueListContext::createFastChildContext( ::sal_Int32 aElementToken,
                                                                                                                                                                                            const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;

        switch ( aElementToken )
        {
        case NMSP_PPT|XML_tav:
        {
            mbInValue = true;
            TimeAnimationValue val;
            val.msFormula = xAttribs->getOptionalValue( XML_fmla );
            val.msTime =  xAttribs->getOptionalValue( XML_tm );
            maTavList.push_back( val );
            break;
        }
        case NMSP_PPT|XML_val:
            if( mbInValue )
            {
                // CT_TLAnimVariant
                xRet.set( new AnimVariantContext( *this, aElementToken, maTavList.back().maValue ) );
            }
            break;
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }


} }
