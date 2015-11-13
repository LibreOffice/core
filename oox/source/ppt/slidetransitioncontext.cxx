/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

SlideTransitionContext::SlideTransitionContext( FragmentHandler2& rParent, const AttributeList& rAttribs, PropertyMap & aProperties ) throw()
: FragmentHandler2( rParent )
, maSlideProperties( aProperties )
, mbHasTransition( false )
{
    // ST_TransitionSpeed
    maTransition.setOoxTransitionSpeed( rAttribs.getToken( XML_spd, XML_fast ) );

    // TODO
    rAttribs.getBool( XML_advClick, true );

    // careful. if missing, no auto advance... 0 looks like a valid value
    // for auto advance
    if(rAttribs.hasAttribute( XML_advTm ))
        maTransition.setOoxAdvanceTime( rAttribs.getInteger( XML_advTm, -1 ) );
}

SlideTransitionContext::~SlideTransitionContext() throw()
{

}

::oox::core::ContextHandlerRef SlideTransitionContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case PPT_TOKEN( blinds ):
    case PPT_TOKEN( checker ):
    case PPT_TOKEN( comb ):
    case PPT_TOKEN( randomBar ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, rAttribs.getToken( XML_dir, XML_horz ), 0);
        }
        return this;
    case PPT_TOKEN( cover ):
    case PPT_TOKEN( pull ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, rAttribs.getToken( XML_dir, XML_l ), 0 );
        }
        return this;
    case PPT_TOKEN( cut ):
    case PPT_TOKEN( fade ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, sal_Int32(rAttribs.getBool( XML_thruBlk, false )), 0);
        }
        return this;
    case PPT_TOKEN( push ):
    case PPT_TOKEN( wipe ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, rAttribs.getToken( XML_dir, XML_l ), 0 );
        }
        return this;
    case PPT_TOKEN( split ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, rAttribs.getToken( XML_orient, XML_horz ),    rAttribs.getToken( XML_dir, XML_out ) );
        }
        return this;
    case PPT_TOKEN( zoom ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, rAttribs.getToken( XML_dir, XML_out ), 0 );
        }
        return this;
    case PPT_TOKEN( wheel ):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, rAttribs.getUnsigned( XML_spokes, 4 ), 0 );
            // unsignedInt
        }
        return this;
    case PPT_TOKEN( circle ):
    case PPT_TOKEN( diamond ):
    case PPT_TOKEN( dissolve ):
    case PPT_TOKEN( newsflash ):
    case PPT_TOKEN( plus ):
    case PPT_TOKEN( random ):
    case PPT_TOKEN( wedge ):
    case P14_TOKEN( vortex ):
    case P14_TOKEN( ripple ):
    case P14_TOKEN( glitter ):
        // CT_Empty
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType( aElementToken, 0, 0 );
        }
        return this;

    case PPT_TOKEN( sndAc ): // CT_TransitionSoundAction
        //"Sound"
        return new SoundActionContext ( *this, maSlideProperties );
    case PPT_TOKEN( extLst ): // CT_OfficeArtExtensionList
        return this;

    case P14_TOKEN(prism):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setOoxTransitionType(aElementToken, sal_Int32(rAttribs.getBool(XML_isInverted, false)), 0);
        }
        return this;
    case P15_TOKEN(prstTrans):
        if (!mbHasTransition)
        {
            mbHasTransition = true;
            maTransition.setPresetTransition(rAttribs.getString(XML_prst, ""));
        }
        return this;


    default:
        break;
    }

    return this;
}

void SlideTransitionContext::onEndElement()
{
    if( isCurrentElement(PPT_TOKEN( transition )) )
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
