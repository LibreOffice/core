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

#include "oox/ppt/slidetransitioncontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <oox/ppt/backgroundproperties.hxx>
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/ppt/soundactioncontext.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"

#include "tokens.hxx"

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {


SlideTransitionContext::SlideTransitionContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, PropertyMap & aProperties ) throw()
: ContextHandler( rParent )
, maSlideProperties( aProperties )
, mbHasTransition( sal_False )
{
    AttributeList attribs(xAttribs);

    // ST_TransitionSpeed
    maTransition.setOoxTransitionSpeed( xAttribs->getOptionalValueToken( XML_spd, XML_fast ) );

    // TODO
    attribs.getBool( XML_advClick, true );

    // careful. if missing, no auto advance... 0 looks like a valid value
    // for auto advance
    if(attribs.hasAttribute( XML_advTm ))
        maTransition.setOoxAdvanceTime( attribs.getInteger( XML_advTm, -1 ) );
}

SlideTransitionContext::~SlideTransitionContext() throw()
{

}

Reference< XFastContextHandler > SlideTransitionContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_PPT|XML_blinds:
    case NMSP_PPT|XML_checker:
    case NMSP_PPT|XML_comb:
    case NMSP_PPT|XML_randomBar:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, xAttribs->getOptionalValueToken( XML_dir, XML_horz ), 0);
            // ST_Direction { XML_horz, XML_vert }
        }
        break;
    case NMSP_PPT|XML_cover:
    case NMSP_PPT|XML_pull:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, xAttribs->getOptionalValueToken( XML_dir, XML_l ), 0 );
            // ST_TransitionEightDirectionType { ST_TransitionSideDirectionType {
            //                                   XML_d, XML_d, XML_r, XML_u },
            //                                   ST_TransitionCornerDirectionType {
            //                                   XML_ld, XML_lu, XML_rd, XML_ru }
        }
        break;
    case NMSP_PPT|XML_cut:
    case NMSP_PPT|XML_fade:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            AttributeList attribs(xAttribs);
            // CT_OptionalBlackTransition xdb:bool
            maTransition.setOoxTransitionType( aElementToken, attribs.getBool( XML_thruBlk, false ), 0);
        }
        break;
    case NMSP_PPT|XML_push:
    case NMSP_PPT|XML_wipe:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, xAttribs->getOptionalValueToken( XML_dir, XML_l ), 0 );
            // ST_TransitionSideDirectionType { XML_d, XML_l, XML_r, XML_u }
        }
        break;
    case NMSP_PPT|XML_split:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, xAttribs->getOptionalValueToken( XML_orient, XML_horz ),	xAttribs->getOptionalValueToken( XML_dir, XML_out ) );
            // ST_Direction { XML_horz, XML_vert }
            // ST_TransitionInOutDirectionType { XML_out, XML_in }
        }
        break;
    case NMSP_PPT|XML_zoom:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, xAttribs->getOptionalValueToken( XML_dir, XML_out ), 0 );
            // ST_TransitionInOutDirectionType { XML_out, XML_in }
        }
        break;
    case NMSP_PPT|XML_wheel:
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            AttributeList attribs(xAttribs);
            maTransition.setOoxTransitionType( aElementToken, attribs.getUnsigned( XML_spokes, 4 ), 0 );
            // unsignedInt
        }
        break;
    case NMSP_PPT|XML_circle:
    case NMSP_PPT|XML_diamond:
    case NMSP_PPT|XML_dissolve:
    case NMSP_PPT|XML_newsflash:
    case NMSP_PPT|XML_plus:
    case NMSP_PPT|XML_random:
    case NMSP_PPT|XML_wedge:
        // CT_Empty
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, 0, 0 );
        }
        break;


    case NMSP_PPT|XML_sndAc: // CT_TransitionSoundAction
        //"Sound"
        xRet.set( new SoundActionContext ( *this, maSlideProperties ) );
        break;
    case NMSP_PPT|XML_extLst: // CT_OfficeArtExtensionList
        return xRet;
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set(this);

    return xRet;
}

void SlideTransitionContext::endFastElement( sal_Int32 aElement ) throw (::com::sun::star::xml::sax::SAXException, RuntimeException)
{
    if( aElement == (NMSP_PPT|XML_transition) )
    {
        if( mbHasTransition )
        {
            maTransition.setSlideProperties( maSlideProperties );
            mbHasTransition = false;
        }
    }
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
