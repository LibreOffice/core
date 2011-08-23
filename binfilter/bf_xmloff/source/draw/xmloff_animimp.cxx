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

#include <tools/debug.hxx>

#include <com/sun/star/presentation/AnimationSpeed.hpp>

#include <list>


#include <comphelper/extract.hxx>


#include "xmlimp.hxx"

#include "xmlnmspe.hxx"

#include "xmluconv.hxx"
/*
*/

#include "nmspmap.hxx"

#include "anim.hxx"

#include "animimp.hxx"
namespace binfilter {

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;

SvXMLEnumMapEntry aXML_AnimationEffect_EnumMap[] =
{
    { XML_NONE,		    EK_none },
    { XML_FADE,		    EK_fade },
    { XML_MOVE,		    EK_move },
    { XML_STRIPES,		EK_stripes },
    { XML_OPEN,		    EK_open },
    { XML_CLOSE,		EK_close },
    { XML_DISSOLVE,	    EK_dissolve },
    { XML_WAVYLINE,	    EK_wavyline },
    { XML_RANDOM,		EK_random },
    { XML_LINES,		EK_lines },
    { XML_LASER,		EK_laser },
    { XML_APPEAR,		EK_appear },
    { XML_HIDE,		    EK_hide },
    { XML_MOVE_SHORT,	EK_move_short },
    { XML_CHECKERBOARD, EK_checkerboard },
    { XML_ROTATE,		EK_rotate },
    { XML_STRETCH,		EK_stretch },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_AnimationDirection_EnumMap[] =
{
    { XML_NONE,			    ED_none },
    { XML_FROM_LEFT,		ED_from_left },
    { XML_FROM_TOP,		    ED_from_top },
    { XML_FROM_RIGHT,		ED_from_right },
    { XML_FROM_BOTTOM,		ED_from_bottom },
    { XML_FROM_CENTER,		ED_from_center },
    { XML_FROM_UPPER_LEFT,	ED_from_upperleft },
    { XML_FROM_UPPER_RIGHT, ED_from_upperright },
    { XML_FROM_LOWER_LEFT,	ED_from_lowerleft },
    { XML_FROM_LOWER_RIGHT, ED_from_lowerright },
    { XML_TO_LEFT,			ED_to_left },
    { XML_TO_TOP,			ED_to_top },
    { XML_TO_RIGHT,		    ED_to_right },
    { XML_TO_BOTTOM,		ED_to_bottom },
    { XML_TO_UPPER_LEFT,	ED_to_upperleft },
    { XML_TO_UPPER_RIGHT,	ED_to_upperright },
    { XML_TO_LOWER_RIGHT,	ED_to_lowerright },
    { XML_TO_LOWER_LEFT,	ED_to_lowerleft },
    { XML_PATH,			    ED_path },
    { XML_SPIRAL_INWARD_LEFT, ED_spiral_inward_left },
    { XML_SPIRAL_INWARD_RIGHT,ED_spiral_inward_right },
    { XML_SPIRAL_OUTWARD_LEFT, ED_spiral_outward_left },
    { XML_SPIRAL_OUTWARD_RIGHT, ED_spiral_outward_right },
    { XML_VERTICAL,		    ED_vertical },
    { XML_HORIZONTAL,		ED_horizontal },
    { XML_TO_CENTER,		ED_to_center },
    { XML_CLOCKWISE,		ED_clockwise },
    { XML_COUNTER_CLOCKWISE,ED_cclockwise },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_AnimationSpeed_EnumMap[] =
{
    { XML_SLOW,	    AnimationSpeed_SLOW },
    { XML_MEDIUM,	AnimationSpeed_MEDIUM },
    { XML_FAST,	    AnimationSpeed_FAST },
    { XML_TOKEN_INVALID, 0 }
};

AnimationEffect ImplSdXMLgetEffect( XMLEffect eKind, XMLEffectDirection eDirection, sal_Int16 nStartScale, sal_Bool bIn )
{
    switch( eKind )
    {
    case EK_fade:
        switch( eDirection )
        {
        case ED_from_left:			return AnimationEffect_FADE_FROM_LEFT;
        case ED_from_top:			return AnimationEffect_FADE_FROM_TOP;
        case ED_from_right:			return AnimationEffect_FADE_FROM_RIGHT;
        case ED_from_bottom:		return AnimationEffect_FADE_FROM_BOTTOM;
        case ED_from_center:		return AnimationEffect_FADE_FROM_CENTER;
        case ED_from_upperleft:		return AnimationEffect_FADE_FROM_UPPERLEFT;
        case ED_from_upperright:	return AnimationEffect_FADE_FROM_UPPERRIGHT;
        case ED_from_lowerleft:		return AnimationEffect_FADE_FROM_LOWERLEFT;
        case ED_from_lowerright:	return AnimationEffect_FADE_FROM_LOWERRIGHT;
        case ED_to_center:			return AnimationEffect_FADE_TO_CENTER;
        case ED_clockwise:			return AnimationEffect_CLOCKWISE;
        case ED_cclockwise:			return AnimationEffect_COUNTERCLOCKWISE;
        case ED_spiral_inward_left:	return AnimationEffect_SPIRALIN_LEFT;
        case ED_spiral_inward_right:return AnimationEffect_SPIRALIN_RIGHT;
        case ED_spiral_outward_left:return AnimationEffect_SPIRALOUT_LEFT;
        case ED_spiral_outward_right:return AnimationEffect_SPIRALOUT_RIGHT;
        }
        return AnimationEffect_FADE_FROM_LEFT;
    case EK_move:
        if( nStartScale == 200 )
        {
            return AnimationEffect_ZOOM_OUT_SMALL;
        }
        else if( nStartScale == 50 )
        {
            return AnimationEffect_ZOOM_IN_SMALL;
        }
        else if( nStartScale < 100 )
        {
            switch( eDirection )
            {
            case ED_from_left:			return AnimationEffect_ZOOM_IN_FROM_LEFT;
            case ED_from_top:			return AnimationEffect_ZOOM_IN_FROM_TOP;
            case ED_from_right:			return AnimationEffect_ZOOM_IN_FROM_RIGHT;
            case ED_from_bottom:		return AnimationEffect_ZOOM_IN_FROM_BOTTOM;
            case ED_from_upperleft:		return AnimationEffect_ZOOM_IN_FROM_UPPERLEFT;
            case ED_from_upperright:	return AnimationEffect_ZOOM_IN_FROM_UPPERRIGHT;
            case ED_from_lowerleft:		return AnimationEffect_ZOOM_IN_FROM_LOWERLEFT;
            case ED_from_lowerright:	return AnimationEffect_ZOOM_IN_FROM_LOWERRIGHT;
            case ED_from_center:		return AnimationEffect_ZOOM_IN_FROM_CENTER;
            case ED_spiral_inward_left:	return AnimationEffect_ZOOM_IN_SPIRAL;
            case ED_to_left:			return AnimationEffect_MOVE_TO_LEFT;
            case ED_to_top:				return AnimationEffect_MOVE_TO_TOP;
            case ED_to_right:			return AnimationEffect_MOVE_TO_RIGHT;
            case ED_to_bottom:			return AnimationEffect_MOVE_TO_BOTTOM;
            case ED_to_upperleft:		return AnimationEffect_MOVE_TO_UPPERLEFT;
            case ED_to_upperright:		return AnimationEffect_MOVE_TO_UPPERRIGHT;
            case ED_to_lowerright:		return AnimationEffect_MOVE_TO_LOWERRIGHT;
            case ED_to_lowerleft:		return AnimationEffect_MOVE_TO_LOWERLEFT;
            }
            return AnimationEffect_ZOOM_IN;
        }
        else if( nStartScale > 100 )
        {
            switch( eDirection )
            {
            case ED_from_left:			return AnimationEffect_ZOOM_OUT_FROM_LEFT;
            case ED_from_top:			return AnimationEffect_ZOOM_OUT_FROM_TOP;
            case ED_from_right:			return AnimationEffect_ZOOM_OUT_FROM_RIGHT;
            case ED_from_bottom:		return AnimationEffect_ZOOM_OUT_FROM_BOTTOM;
            case ED_from_upperleft:		return AnimationEffect_ZOOM_OUT_FROM_UPPERLEFT;
            case ED_from_upperright:	return AnimationEffect_ZOOM_OUT_FROM_UPPERRIGHT;
            case ED_from_lowerleft:		return AnimationEffect_ZOOM_OUT_FROM_LOWERLEFT;
            case ED_from_lowerright:	return AnimationEffect_ZOOM_OUT_FROM_LOWERRIGHT;
            case ED_from_center:		return AnimationEffect_ZOOM_OUT_FROM_CENTER;
            case ED_spiral_inward_left:	return AnimationEffect_ZOOM_OUT_SPIRAL;
            }
            return AnimationEffect_ZOOM_OUT;
        }
        else
        {
            switch( eDirection )
            {
            case ED_from_left:			return AnimationEffect_MOVE_FROM_LEFT;
            case ED_from_top:			return AnimationEffect_MOVE_FROM_TOP;
            case ED_from_right:			return AnimationEffect_MOVE_FROM_RIGHT;
            case ED_from_bottom:		return AnimationEffect_MOVE_FROM_BOTTOM;
            case ED_from_upperleft:		return AnimationEffect_MOVE_FROM_UPPERLEFT;
            case ED_from_upperright:	return AnimationEffect_MOVE_FROM_UPPERRIGHT;
            case ED_from_lowerleft:		return AnimationEffect_MOVE_FROM_LOWERLEFT;
            case ED_from_lowerright:	return AnimationEffect_MOVE_FROM_LOWERRIGHT;
            case ED_path:				return AnimationEffect_PATH;
            case ED_to_top:				return AnimationEffect_MOVE_TO_TOP;
            case ED_to_right:			return AnimationEffect_MOVE_TO_RIGHT;
            case ED_to_bottom:			return AnimationEffect_MOVE_TO_BOTTOM;
            case ED_to_upperleft:		return AnimationEffect_MOVE_TO_UPPERLEFT;
            case ED_to_upperright:		return AnimationEffect_MOVE_TO_UPPERRIGHT;
            case ED_to_lowerright:		return AnimationEffect_MOVE_TO_LOWERRIGHT;
            case ED_to_lowerleft:		return AnimationEffect_MOVE_TO_LOWERLEFT;
            }
        }
        return AnimationEffect_MOVE_FROM_LEFT;
    case EK_stripes:
        if( eDirection == ED_vertical )
            return AnimationEffect_VERTICAL_STRIPES;
        else
            return AnimationEffect_HORIZONTAL_STRIPES;
    case EK_open:
        if( eDirection == ED_vertical )
            return AnimationEffect_OPEN_VERTICAL;
        else
            return AnimationEffect_OPEN_HORIZONTAL;
    case EK_close:
        if( eDirection == ED_vertical )
            return AnimationEffect_CLOSE_VERTICAL;
        else
            return AnimationEffect_CLOSE_HORIZONTAL;
    case EK_dissolve:
        return AnimationEffect_DISSOLVE;
    case EK_wavyline:
        switch( eDirection )
        {
        case ED_from_left:			return AnimationEffect_WAVYLINE_FROM_LEFT;
        case ED_from_top:			return AnimationEffect_WAVYLINE_FROM_TOP;
        case ED_from_right:			return AnimationEffect_WAVYLINE_FROM_RIGHT;
        case ED_from_bottom:		return AnimationEffect_WAVYLINE_FROM_BOTTOM;
        }
        return AnimationEffect_WAVYLINE_FROM_LEFT;
    case EK_random:
        return AnimationEffect_RANDOM;
    case EK_lines:
        if( eDirection == ED_vertical )
            return AnimationEffect_VERTICAL_LINES;
        else
            return AnimationEffect_HORIZONTAL_LINES;
    case EK_laser:
        switch( eDirection )
        {
        case ED_from_left:			return AnimationEffect_LASER_FROM_LEFT;
        case ED_from_top:			return AnimationEffect_LASER_FROM_TOP;
        case ED_from_right:			return AnimationEffect_LASER_FROM_RIGHT;
        case ED_from_bottom:		return AnimationEffect_LASER_FROM_BOTTOM;
        case ED_from_upperleft:		return AnimationEffect_LASER_FROM_UPPERLEFT;
        case ED_from_upperright:	return AnimationEffect_LASER_FROM_UPPERRIGHT;
        case ED_from_lowerleft:		return AnimationEffect_LASER_FROM_LOWERLEFT;
        case ED_from_lowerright:	return AnimationEffect_LASER_FROM_LOWERRIGHT;
        }
        return AnimationEffect_LASER_FROM_LEFT;
    case EK_appear:
        return AnimationEffect_APPEAR;
    case EK_hide:
        return AnimationEffect_HIDE;
    case EK_move_short:
        switch( eDirection )
        {
        case ED_from_left:			return AnimationEffect_MOVE_SHORT_FROM_LEFT;
        case ED_from_top:			return AnimationEffect_MOVE_SHORT_FROM_TOP;
        case ED_from_right:			return AnimationEffect_MOVE_SHORT_FROM_RIGHT;
        case ED_from_bottom:		return AnimationEffect_MOVE_SHORT_FROM_BOTTOM;
        case ED_from_upperleft:		return AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT;
        case ED_from_upperright:	return AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT;
        case ED_from_lowerleft:		return AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT;
        case ED_from_lowerright:	return AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT;
        case ED_to_left:			return AnimationEffect_MOVE_SHORT_TO_LEFT;
        case ED_to_upperleft:		return AnimationEffect_MOVE_SHORT_TO_UPPERLEFT;
        case ED_to_top:				return AnimationEffect_MOVE_SHORT_TO_TOP;
        case ED_to_upperright:		return AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT;
        case ED_to_right:			return AnimationEffect_MOVE_SHORT_TO_RIGHT;
        case ED_to_lowerright:		return AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT;
        case ED_to_bottom:			return AnimationEffect_MOVE_SHORT_TO_BOTTOM;
        case ED_to_lowerleft:		return AnimationEffect_MOVE_SHORT_TO_LOWERLEFT;
        }
        return AnimationEffect_MOVE_SHORT_FROM_LEFT;
    case EK_checkerboard:
        if( eDirection == ED_vertical )
            return AnimationEffect_VERTICAL_CHECKERBOARD;
        else
            return AnimationEffect_HORIZONTAL_CHECKERBOARD;
    case EK_rotate:
        if( eDirection == ED_vertical )
            return AnimationEffect_VERTICAL_ROTATE;
        else
            return AnimationEffect_HORIZONTAL_ROTATE;
    case EK_stretch:
        switch( eDirection )
        {
        case ED_from_left:			return AnimationEffect_STRETCH_FROM_LEFT;
        case ED_from_top:			return AnimationEffect_STRETCH_FROM_TOP;
        case ED_from_right:			return AnimationEffect_STRETCH_FROM_RIGHT;
        case ED_from_bottom:		return AnimationEffect_STRETCH_FROM_BOTTOM;
        case ED_from_upperleft:		return AnimationEffect_STRETCH_FROM_UPPERLEFT;
        case ED_from_upperright:	return AnimationEffect_STRETCH_FROM_UPPERRIGHT;
        case ED_from_lowerleft:		return AnimationEffect_STRETCH_FROM_LOWERLEFT;
        case ED_from_lowerright:	return AnimationEffect_STRETCH_FROM_LOWERRIGHT;
        case ED_vertical:			return AnimationEffect_VERTICAL_STRETCH;
        case ED_horizontal:			return AnimationEffect_HORIZONTAL_STRETCH;
        }
        return AnimationEffect_STRETCH_FROM_LEFT;
    }

    return AnimationEffect_NONE;
}

///////////////////////////////////////////////////////////////////////

class AnimImpImpl
{
public:
    Reference< XPropertySet > mxLastShape;
    sal_Int32 mnPresOrder;
    sal_Int32 mnLastShapeId;

    OUString msDimColor;
    OUString msDimHide;
    OUString msDimPrev;
    OUString msEffect;
    OUString msPlayFull;
    OUString msPresOrder;
    OUString msSound;
    OUString msSoundOn;
    OUString msSpeed;
    OUString msTextEffect;
    OUString msPresShapeService;
    OUString msAnimPath;
    OUString msIsAnimation;

    AnimImpImpl()
    :	mnPresOrder( 0 ),
        mnLastShapeId( -1 ),
        msDimColor( RTL_CONSTASCII_USTRINGPARAM( "DimColor" ) ),
        msDimHide( RTL_CONSTASCII_USTRINGPARAM( "DimHide" ) ),
        msDimPrev( RTL_CONSTASCII_USTRINGPARAM( "DimPrevious" ) ),
        msEffect( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ),
        msPlayFull( RTL_CONSTASCII_USTRINGPARAM( "PlayFull" ) ),
        msPresOrder( RTL_CONSTASCII_USTRINGPARAM( "PresentationOrder" ) ),
        msSound( RTL_CONSTASCII_USTRINGPARAM( "Sound" ) ),
        msSoundOn( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ),
        msSpeed( RTL_CONSTASCII_USTRINGPARAM( "Speed" ) ),
        msTextEffect( RTL_CONSTASCII_USTRINGPARAM( "TextEffect" ) ),
        msPresShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.Shape" ) ),
        msAnimPath( RTL_CONSTASCII_USTRINGPARAM( "AnimationPath" ) ),
        msIsAnimation( RTL_CONSTASCII_USTRINGPARAM( "IsAnimation" ) )
    {}
};

///////////////////////////////////////////////////////////////////////

enum XMLActionKind
{
    XMLE_SHOW,
    XMLE_HIDE,
    XMLE_DIM,
    XMLE_PLAY
};

class XMLAnimationsEffectContext : public SvXMLImportContext
{
public:
    AnimImpImpl*	mpImpl;

    XMLActionKind	meKind;
    sal_Bool		mbTextEffect;
    sal_Int32		mnShapeId;

    XMLEffect		meEffect;
    XMLEffectDirection	meDirection;
    sal_Int16		mnStartScale;

    AnimationSpeed	meSpeed;
    Color			maDimColor;
    OUString		maSoundURL;
    sal_Bool		mbPlayFull;
    sal_Int32		mnPathShapeId;

public:
    TYPEINFO();

    XMLAnimationsEffectContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList,
        AnimImpImpl* pImpl);
    virtual ~XMLAnimationsEffectContext();

    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList );
};

class XMLAnimationsSoundContext : public SvXMLImportContext
{
    XMLAnimationsEffectContext*	mpParent;

public:
    TYPEINFO();

    XMLAnimationsSoundContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, XMLAnimationsEffectContext* pParent );
    virtual ~XMLAnimationsSoundContext();
};

TYPEINIT1( XMLAnimationsSoundContext, SvXMLImportContext );

XMLAnimationsSoundContext::XMLAnimationsSoundContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, XMLAnimationsEffectContext* pParent )
: SvXMLImportContext( rImport, nPrfx, rLocalName ), mpParent( pParent )
{
    if( mpParent && nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_SOUND ) )
    {
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nPrefix )
            {
            case XML_NAMESPACE_XLINK:
                if( IsXMLToken( aLocalName, XML_HREF ) )
                {
                    mpParent->maSoundURL = rImport.GetAbsoluteReference(sValue);
                }
                break;
            case XML_NAMESPACE_PRESENTATION:
                if( IsXMLToken( aLocalName, XML_PLAY_FULL ) )
                {
                    mpParent->mbPlayFull = IsXMLToken( sValue, XML_TRUE );
                }
            }
        }
    }
}

XMLAnimationsSoundContext::~XMLAnimationsSoundContext()
{
}


TYPEINIT1( XMLAnimationsEffectContext, SvXMLImportContext );

XMLAnimationsEffectContext::XMLAnimationsEffectContext( SvXMLImport& rImport,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList, AnimImpImpl* pImpl )
:	SvXMLImportContext(rImport, nPrfx, rLocalName),
    mpImpl( pImpl ),
    meKind( XMLE_SHOW ), mbTextEffect( sal_False ), mnShapeId( -1 ), mnPathShapeId( -1 ),
    meEffect( EK_none ), meDirection( ED_none ), mnStartScale( 100 ),
    meSpeed( AnimationSpeed_MEDIUM ), maDimColor(0), mbPlayFull( sal_False )
{
    if( IsXMLToken( rLocalName, XML_SHOW_SHAPE ) )
    {
        meKind = XMLE_SHOW;
    }
    else if( IsXMLToken( rLocalName, XML_SHOW_TEXT ) )
    {
        meKind = XMLE_SHOW;
        mbTextEffect = sal_True;
    }
    else if( IsXMLToken( rLocalName, XML_HIDE_SHAPE ) )
    {
        meKind = XMLE_HIDE;
    }
    else if( IsXMLToken( rLocalName, XML_HIDE_TEXT ) )
    {
        meKind = XMLE_HIDE;
        mbTextEffect = sal_True;
    }
    else if( IsXMLToken( rLocalName, XML_DIM ) )
    {
        meKind = XMLE_DIM;
    }
    else if( IsXMLToken( rLocalName, XML_PLAY ) )
    {
        meKind = XMLE_PLAY;
    }
    else
    {
        // unknown action, overread
        return;
    }

    // read attributes
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        switch( nPrefix )
        {
        case XML_NAMESPACE_DRAW:
            if( IsXMLToken( aLocalName, XML_SHAPE_ID ) )
            {
                SvXMLUnitConverter::convertNumber(mnShapeId, sValue);
            }
            else if( IsXMLToken( aLocalName, XML_COLOR ) )
            {
                SvXMLUnitConverter::convertColor(maDimColor, sValue);
            }
            break;

        case XML_NAMESPACE_PRESENTATION:
            if( IsXMLToken( aLocalName, XML_EFFECT ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationEffect_EnumMap ) )
                    meEffect = (XMLEffect)eEnum;
            }
            else if( IsXMLToken(aLocalName, XML_DIRECTION ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationDirection_EnumMap ) )
                    meDirection = (XMLEffectDirection)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_START_SCALE ) )
            {
                sal_Int32 nScale;
                if( SvXMLUnitConverter::convertPercent( nScale, sValue ) )
                    mnStartScale = (sal_Int16)nScale;
            }
            else if( IsXMLToken( aLocalName, XML_SPEED ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationSpeed_EnumMap ) )
                    meSpeed = (AnimationSpeed)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_PATH_ID ) )
            {
                SvXMLUnitConverter::convertNumber(mnPathShapeId, sValue);
            }
            break;
        }
    }
}

XMLAnimationsEffectContext::~XMLAnimationsEffectContext()
{
}

SvXMLImportContext * XMLAnimationsEffectContext::CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
{
    return new XMLAnimationsSoundContext( GetImport(), nPrefix, rLocalName, xAttrList, this );
}

void XMLAnimationsEffectContext::EndElement()
{
    // set effect on shape

    try
    {
        UniReference< XMLShapeImportHelper > xShapeImport( GetImport().GetShapeImport() );
        Any aAny;

        if( mnShapeId != -1 )
        {
            Reference< XPropertySet > xSet;
            if( mpImpl->mnLastShapeId != mnShapeId )
            {
                xSet = Reference< XPropertySet >::query( xShapeImport->getShapeFromId( mnShapeId ) );
                if( xSet.is() )
                {
                    // check for presentation shape service
                    {
                        Reference< XServiceInfo > xServiceInfo( xSet, UNO_QUERY );
                        if( !xServiceInfo.is() || !xServiceInfo->supportsService( mpImpl->msPresShapeService ) )
                            return;
                    }

                    mpImpl->mnLastShapeId = mnShapeId;
                    mpImpl->mxLastShape = xSet;

                    aAny <<= mpImpl->mnPresOrder++;
                    xSet->setPropertyValue( mpImpl->msPresOrder, aAny );
                }
            }
            else
            {
                xSet = mpImpl->mxLastShape;
            }

            if( xSet.is() )
            {
                if( meKind == XMLE_DIM )
                {
                    aAny <<= (sal_Bool)sal_True;
                    xSet->setPropertyValue( mpImpl->msDimPrev, aAny );

                    aAny <<= (sal_Int32)maDimColor.GetColor();
                    xSet->setPropertyValue( mpImpl->msDimColor, aAny );
                }
                else if( meKind == XMLE_PLAY )
                {
                    aAny <<= (sal_Bool)sal_True;
                    xSet->setPropertyValue( mpImpl->msIsAnimation, aAny );

                    aAny <<= meSpeed;
                    xSet->setPropertyValue( mpImpl->msSpeed, aAny );
                }
                else
                {
                    if( meKind == XMLE_HIDE && !mbTextEffect && meEffect == EK_none )
                    {
                        aAny = bool2any( sal_True );
                        xSet->setPropertyValue( mpImpl->msDimHide, aAny );
                    }
                    else
                    {
                        const AnimationEffect eEffect = ImplSdXMLgetEffect( meEffect, meDirection, mnStartScale, meKind == XMLE_SHOW );

                        aAny <<= eEffect;
                        xSet->setPropertyValue( mbTextEffect ? mpImpl->msTextEffect : mpImpl->msEffect, aAny );

                        aAny <<= meSpeed;
                        xSet->setPropertyValue( mpImpl->msSpeed, aAny );

                        if( eEffect == AnimationEffect_PATH && mnPathShapeId != -1 )
                        {
                            Reference< XShape > xPath( xShapeImport->getShapeFromId( mnPathShapeId ) );
                            if( xPath.is() )
                            {
                                aAny <<= xPath;
                                xSet->setPropertyValue( mpImpl->msAnimPath, aAny );
                            }
                        }
                    }
                }
            }

            if( maSoundURL.getLength() != 0 )
            {
                aAny <<= maSoundURL;
                xSet->setPropertyValue( mpImpl->msSound, aAny );

                aAny <<= bool2any( mbPlayFull );
                xSet->setPropertyValue( mpImpl->msPlayFull, aAny );

                aAny <<= bool2any( sal_True );
                xSet->setPropertyValue( mpImpl->msSoundOn, aAny );
            }
        }
    }
    catch( Exception e )
    {
        DBG_ERROR( "exception catched while importing animation information!" );
    }
}

///////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLAnimationsContext, SvXMLImportContext );

XMLAnimationsContext::XMLAnimationsContext( SvXMLImport& rImport, sal_uInt16 nPrfx,	const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList)
: SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    mpImpl = new AnimImpImpl();
}

XMLAnimationsContext::~XMLAnimationsContext()
{
    delete mpImpl;
}

SvXMLImportContext * XMLAnimationsContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return new XMLAnimationsEffectContext( GetImport(), nPrefix, rLocalName,  xAttrList, mpImpl );
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
