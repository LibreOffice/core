/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillpropertiesgroupcontext.cxx,v $
 * $Revision: 1.8.6.1 $
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

#include "oox/drawingml/scene3dcontext.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandler;
using ::oox::core::XmlFilterBase;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

Scene3DContext::Scene3DContext( ContextHandler& rParent, Shape3DProperties& r3DProperties ) throw()
: ContextHandler( rParent )
, mr3DProperties( r3DProperties )
{
}

Reference< XFastContextHandler > Scene3DContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw ( SAXException, RuntimeException )
{
    AttributeList aAttribs( xAttribs );
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_camera:
        mr3DProperties.mfFieldOfVision = aAttribs.getInteger( XML_fov, 0 ) / 36000000.0;
        mr3DProperties.mfZoom = aAttribs.getInteger( XML_zoom, 100000 ) / 100000.0;
        mr3DProperties.mnPreset = aAttribs.getToken( XML_prst, XML_none );

legacyObliqueTopLeft
legacyObliqueTop
legacyObliqueTopRight
legacyObliqueLeft
legacyObliqueFront
legacyObliqueRight
legacyObliqueBottomLeft
legacyObliqueBottom
legacyObliqueBottomRight
legacyPerspectiveTopLeft
legacyPerspectiveTop
legacyPerspectiveTopRight
legacyPerspectiveLeft
legacyPerspectiveFront
legacyPerspectiveRight
legacyPerspectiveBottomLeft
legacyPerspectiveBottom
legacyPerspectiveBottomRight
orthographicFront
isometricTopUp
isometricTopDown
isometricBottomUp
isometricBottomDown
isometricLeftUp
isometricLeftDown
isometricRightUp
isometricRightDown
isometricOffAxis1Left
isometricOffAxis1Right
isometricOffAxis1Top
isometricOffAxis2Left
isometricOffAxis2Right
isometricOffAxis2Top
isometricOffAxis3Left
isometricOffAxis3Right
isometricOffAxis3Bottom
isometricOffAxis4Left
isometricOffAxis4Right
isometricOffAxis4Bottom
obliqueTopLeft
obliqueTop
obliqueTopRight
obliqueLeft
obliqueRight
obliqueBottomLeft
obliqueBottom
obliqueBottomRight
perspectiveFront
perspectiveLeft
perspectiveRight
perspectiveAbove
perspectiveBelow
perspectiveAboveLeftFacing
perspectiveAboveRightFacing
perspectiveContrastingLeftFacing
perspectiveContrastingRightFacing
perspectiveHeroicLeftFacing
perspectiveHeroicRightFacing
perspectiveHeroicExtremeLeftFacing
perspectiveHeroicExtremeRightFacing
perspectiveRelaxed
perspectiveRelaxedModerately


        // TODO: nested element XML_rot
        break;
    case NMSP_DRAWINGML|XML_lightRig:
        mr3DProperties.mnLightRigDirection = aAttribs.getToken( XML_dir, XML_none );

XML_tl
XML_t
XML_tr
XML_l
XML_r
XML_bl
XML_b
XML_br


        mr3DProperties.mnLightRigType = aAttribs.getToken( XML_rig, XML_none );

XML_legacyFlat1
XML_legacyFlat2
XML_legacyFlat3
XML_legacyFlat4
XML_legacyNormal1
XML_legacyNormal2
XML_legacyNormal3
XML_legacyNormal4
XML_legacyHarsh1
XML_legacyHarsh2
XML_legacyHarsh3
XML_legacyHarsh4
XML_threePt
XML_balanced
XML_soft
XML_harsh
XML_flood
XML_contrasting
XML_morning
XML_sunrise
XML_sunset
XML_chilly
XML_freezing
XML_flat
XML_twoPt
XML_glow
XML_brightRoom

        // TODO: nested element XML_rot
        break;
    case NMSP_DRAWINGML|XML_backdrop:
    case NMSP_DRAWINGML|XML_extLst:
        return xRet; // TODO: later (backdrop is not supported by core anyway)
    }
    return xRet;
}

} }
