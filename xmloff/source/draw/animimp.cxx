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

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <sax/tools/converter.hxx>

#include <list>
#include <comphelper/extract.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "anim.hxx"
#include "animimp.hxx"

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
using namespace ::xmloff::token;

SvXMLEnumMapEntry aXML_AnimationEffect_EnumMap[] =
{
    { XML_NONE,         EK_none },
    { XML_FADE,         EK_fade },
    { XML_MOVE,         EK_move },
    { XML_STRIPES,      EK_stripes },
    { XML_OPEN,         EK_open },
    { XML_CLOSE,        EK_close },
    { XML_DISSOLVE,     EK_dissolve },
    { XML_WAVYLINE,     EK_wavyline },
    { XML_RANDOM,       EK_random },
    { XML_LINES,        EK_lines },
    { XML_LASER,        EK_laser },
    { XML_APPEAR,       EK_appear },
    { XML_HIDE,         EK_hide },
    { XML_MOVE_SHORT,   EK_move_short },
    { XML_CHECKERBOARD, EK_checkerboard },
    { XML_ROTATE,       EK_rotate },
    { XML_STRETCH,      EK_stretch },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_AnimationDirection_EnumMap[] =
{
    { XML_NONE,             ED_none },
    { XML_FROM_LEFT,        ED_from_left },
    { XML_FROM_TOP,         ED_from_top },
    { XML_FROM_RIGHT,       ED_from_right },
    { XML_FROM_BOTTOM,      ED_from_bottom },
    { XML_FROM_CENTER,      ED_from_center },
    { XML_FROM_UPPER_LEFT,  ED_from_upperleft },
    { XML_FROM_UPPER_RIGHT, ED_from_upperright },
    { XML_FROM_LOWER_LEFT,  ED_from_lowerleft },
    { XML_FROM_LOWER_RIGHT, ED_from_lowerright },
    { XML_TO_LEFT,          ED_to_left },
    { XML_TO_TOP,           ED_to_top },
    { XML_TO_RIGHT,         ED_to_right },
    { XML_TO_BOTTOM,        ED_to_bottom },
    { XML_TO_UPPER_LEFT,    ED_to_upperleft },
    { XML_TO_UPPER_RIGHT,   ED_to_upperright },
    { XML_TO_LOWER_RIGHT,   ED_to_lowerright },
    { XML_TO_LOWER_LEFT,    ED_to_lowerleft },
    { XML_PATH,             ED_path },
    { XML_SPIRAL_INWARD_LEFT, ED_spiral_inward_left },
    { XML_SPIRAL_INWARD_RIGHT,ED_spiral_inward_right },
    { XML_SPIRAL_OUTWARD_LEFT, ED_spiral_outward_left },
    { XML_SPIRAL_OUTWARD_RIGHT, ED_spiral_outward_right },
    { XML_VERTICAL,         ED_vertical },
    { XML_HORIZONTAL,       ED_horizontal },
    { XML_TO_CENTER,        ED_to_center },
    { XML_CLOCKWISE,        ED_clockwise },
    { XML_COUNTER_CLOCKWISE,ED_cclockwise },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_AnimationSpeed_EnumMap[] =
{
    { XML_SLOW,     AnimationSpeed_SLOW },
    { XML_MEDIUM,   AnimationSpeed_MEDIUM },
    { XML_FAST,     AnimationSpeed_FAST },
    { XML_TOKEN_INVALID, 0 }
};

AnimationEffect ImplSdXMLgetEffect( XMLEffect eKind, XMLEffectDirection eDirection, sal_Int16 nStartScale, sal_Bool /*bIn*/ )
{
    switch( eKind )
    {
    case EK_fade:
        switch( eDirection )
        {
        case ED_from_left:          return AnimationEffect_FADE_FROM_LEFT;
        case ED_from_top:           return AnimationEffect_FADE_FROM_TOP;
        case ED_from_right:         return AnimationEffect_FADE_FROM_RIGHT;
        case ED_from_bottom:        return AnimationEffect_FADE_FROM_BOTTOM;
        case ED_from_center:        return AnimationEffect_FADE_FROM_CENTER;
        case ED_from_upperleft:     return AnimationEffect_FADE_FROM_UPPERLEFT;
        case ED_from_upperright:    return AnimationEffect_FADE_FROM_UPPERRIGHT;
        case ED_from_lowerleft:     return AnimationEffect_FADE_FROM_LOWERLEFT;
        case ED_from_lowerright:    return AnimationEffect_FADE_FROM_LOWERRIGHT;
        case ED_to_center:          return AnimationEffect_FADE_TO_CENTER;
        case ED_clockwise:          return AnimationEffect_CLOCKWISE;
        case ED_cclockwise:         return AnimationEffect_COUNTERCLOCKWISE;
        case ED_spiral_inward_left: return AnimationEffect_SPIRALIN_LEFT;
        case ED_spiral_inward_right:return AnimationEffect_SPIRALIN_RIGHT;
        case ED_spiral_outward_left:return AnimationEffect_SPIRALOUT_LEFT;
        case ED_spiral_outward_right:return AnimationEffect_SPIRALOUT_RIGHT;
        default:                    return AnimationEffect_FADE_FROM_LEFT;
        }
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
            case ED_from_left:          return AnimationEffect_ZOOM_IN_FROM_LEFT;
            case ED_from_top:           return AnimationEffect_ZOOM_IN_FROM_TOP;
            case ED_from_right:         return AnimationEffect_ZOOM_IN_FROM_RIGHT;
            case ED_from_bottom:        return AnimationEffect_ZOOM_IN_FROM_BOTTOM;
            case ED_from_upperleft:     return AnimationEffect_ZOOM_IN_FROM_UPPERLEFT;
            case ED_from_upperright:    return AnimationEffect_ZOOM_IN_FROM_UPPERRIGHT;
            case ED_from_lowerleft:     return AnimationEffect_ZOOM_IN_FROM_LOWERLEFT;
            case ED_from_lowerright:    return AnimationEffect_ZOOM_IN_FROM_LOWERRIGHT;
            case ED_from_center:        return AnimationEffect_ZOOM_IN_FROM_CENTER;
            case ED_spiral_inward_left: return AnimationEffect_ZOOM_IN_SPIRAL;
            case ED_to_left:            return AnimationEffect_MOVE_TO_LEFT;
            case ED_to_top:             return AnimationEffect_MOVE_TO_TOP;
            case ED_to_right:           return AnimationEffect_MOVE_TO_RIGHT;
            case ED_to_bottom:          return AnimationEffect_MOVE_TO_BOTTOM;
            case ED_to_upperleft:       return AnimationEffect_MOVE_TO_UPPERLEFT;
            case ED_to_upperright:      return AnimationEffect_MOVE_TO_UPPERRIGHT;
            case ED_to_lowerright:      return AnimationEffect_MOVE_TO_LOWERRIGHT;
            case ED_to_lowerleft:       return AnimationEffect_MOVE_TO_LOWERLEFT;
            default:                    return AnimationEffect_ZOOM_IN;
            }
        }
        else if( nStartScale > 100 )
        {
            switch( eDirection )
            {
            case ED_from_left:          return AnimationEffect_ZOOM_OUT_FROM_LEFT;
            case ED_from_top:           return AnimationEffect_ZOOM_OUT_FROM_TOP;
            case ED_from_right:         return AnimationEffect_ZOOM_OUT_FROM_RIGHT;
            case ED_from_bottom:        return AnimationEffect_ZOOM_OUT_FROM_BOTTOM;
            case ED_from_upperleft:     return AnimationEffect_ZOOM_OUT_FROM_UPPERLEFT;
            case ED_from_upperright:    return AnimationEffect_ZOOM_OUT_FROM_UPPERRIGHT;
            case ED_from_lowerleft:     return AnimationEffect_ZOOM_OUT_FROM_LOWERLEFT;
            case ED_from_lowerright:    return AnimationEffect_ZOOM_OUT_FROM_LOWERRIGHT;
            case ED_from_center:        return AnimationEffect_ZOOM_OUT_FROM_CENTER;
            case ED_spiral_inward_left: return AnimationEffect_ZOOM_OUT_SPIRAL;
            default:                    return AnimationEffect_ZOOM_OUT;
            }
        }
        else
        {
            switch( eDirection )
            {
            case ED_from_left:          return AnimationEffect_MOVE_FROM_LEFT;
            case ED_from_top:           return AnimationEffect_MOVE_FROM_TOP;
            case ED_from_right:         return AnimationEffect_MOVE_FROM_RIGHT;
            case ED_from_bottom:        return AnimationEffect_MOVE_FROM_BOTTOM;
            case ED_from_upperleft:     return AnimationEffect_MOVE_FROM_UPPERLEFT;
            case ED_from_upperright:    return AnimationEffect_MOVE_FROM_UPPERRIGHT;
            case ED_from_lowerleft:     return AnimationEffect_MOVE_FROM_LOWERLEFT;
            case ED_from_lowerright:    return AnimationEffect_MOVE_FROM_LOWERRIGHT;
            case ED_path:               return AnimationEffect_PATH;
            case ED_to_top:             return AnimationEffect_MOVE_TO_TOP;
            case ED_to_right:           return AnimationEffect_MOVE_TO_RIGHT;
            case ED_to_bottom:          return AnimationEffect_MOVE_TO_BOTTOM;
            case ED_to_upperleft:       return AnimationEffect_MOVE_TO_UPPERLEFT;
            case ED_to_upperright:      return AnimationEffect_MOVE_TO_UPPERRIGHT;
            case ED_to_lowerright:      return AnimationEffect_MOVE_TO_LOWERRIGHT;
            case ED_to_lowerleft:       return AnimationEffect_MOVE_TO_LOWERLEFT;
            default:
                break;
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
        case ED_from_left:          return AnimationEffect_WAVYLINE_FROM_LEFT;
        case ED_from_top:           return AnimationEffect_WAVYLINE_FROM_TOP;
        case ED_from_right:         return AnimationEffect_WAVYLINE_FROM_RIGHT;
        case ED_from_bottom:        return AnimationEffect_WAVYLINE_FROM_BOTTOM;
        default:                    return AnimationEffect_WAVYLINE_FROM_LEFT;
        }
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
        case ED_from_left:          return AnimationEffect_LASER_FROM_LEFT;
        case ED_from_top:           return AnimationEffect_LASER_FROM_TOP;
        case ED_from_right:         return AnimationEffect_LASER_FROM_RIGHT;
        case ED_from_bottom:        return AnimationEffect_LASER_FROM_BOTTOM;
        case ED_from_upperleft:     return AnimationEffect_LASER_FROM_UPPERLEFT;
        case ED_from_upperright:    return AnimationEffect_LASER_FROM_UPPERRIGHT;
        case ED_from_lowerleft:     return AnimationEffect_LASER_FROM_LOWERLEFT;
        case ED_from_lowerright:    return AnimationEffect_LASER_FROM_LOWERRIGHT;
        default:                    return AnimationEffect_LASER_FROM_LEFT;
        }
    case EK_appear:
        return AnimationEffect_APPEAR;
    case EK_hide:
        return AnimationEffect_HIDE;
    case EK_move_short:
        switch( eDirection )
        {
        case ED_from_left:          return AnimationEffect_MOVE_SHORT_FROM_LEFT;
        case ED_from_top:           return AnimationEffect_MOVE_SHORT_FROM_TOP;
        case ED_from_right:         return AnimationEffect_MOVE_SHORT_FROM_RIGHT;
        case ED_from_bottom:        return AnimationEffect_MOVE_SHORT_FROM_BOTTOM;
        case ED_from_upperleft:     return AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT;
        case ED_from_upperright:    return AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT;
        case ED_from_lowerleft:     return AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT;
        case ED_from_lowerright:    return AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT;
        case ED_to_left:            return AnimationEffect_MOVE_SHORT_TO_LEFT;
        case ED_to_upperleft:       return AnimationEffect_MOVE_SHORT_TO_UPPERLEFT;
        case ED_to_top:             return AnimationEffect_MOVE_SHORT_TO_TOP;
        case ED_to_upperright:      return AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT;
        case ED_to_right:           return AnimationEffect_MOVE_SHORT_TO_RIGHT;
        case ED_to_lowerright:      return AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT;
        case ED_to_bottom:          return AnimationEffect_MOVE_SHORT_TO_BOTTOM;
        case ED_to_lowerleft:       return AnimationEffect_MOVE_SHORT_TO_LOWERLEFT;
        default:                    return AnimationEffect_MOVE_SHORT_FROM_LEFT;
        }
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
        case ED_from_left:          return AnimationEffect_STRETCH_FROM_LEFT;
        case ED_from_top:           return AnimationEffect_STRETCH_FROM_TOP;
        case ED_from_right:         return AnimationEffect_STRETCH_FROM_RIGHT;
        case ED_from_bottom:        return AnimationEffect_STRETCH_FROM_BOTTOM;
        case ED_from_upperleft:     return AnimationEffect_STRETCH_FROM_UPPERLEFT;
        case ED_from_upperright:    return AnimationEffect_STRETCH_FROM_UPPERRIGHT;
        case ED_from_lowerleft:     return AnimationEffect_STRETCH_FROM_LOWERLEFT;
        case ED_from_lowerright:    return AnimationEffect_STRETCH_FROM_LOWERRIGHT;
        case ED_vertical:           return AnimationEffect_VERTICAL_STRETCH;
        case ED_horizontal:         return AnimationEffect_HORIZONTAL_STRETCH;
        default:
            break;
        }
        return AnimationEffect_STRETCH_FROM_LEFT;
    default:
        return AnimationEffect_NONE;
    }
}

class AnimImpImpl
{
public:
    Reference< XPropertySet > mxLastShape;
    OUString maLastShapeId;

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
    :   msDimColor( "DimColor" ),
        msDimHide( "DimHide" ),
        msDimPrev( "DimPrevious" ),
        msEffect( "Effect" ),
        msPlayFull( "PlayFull" ),
        msPresOrder( "PresentationOrder" ),
        msSound( "Sound" ),
        msSoundOn( "SoundOn" ),
        msSpeed( "Speed" ),
        msTextEffect( "TextEffect" ),
        msPresShapeService( "com.sun.star.presentation.Shape" ),
        msAnimPath( "AnimationPath" ),
        msIsAnimation( "IsAnimation" )
    {}
};

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
    AnimImpImpl*    mpImpl;

    XMLActionKind   meKind;
    sal_Bool        mbTextEffect;
    OUString        maShapeId;

    XMLEffect       meEffect;
    XMLEffectDirection  meDirection;
    sal_Int16       mnStartScale;

    AnimationSpeed  meSpeed;
    sal_Int32       maDimColor;
    OUString        maSoundURL;
    sal_Bool        mbPlayFull;
    OUString        maPathShapeId;

public:
    TYPEINFO();

    XMLAnimationsEffectContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList,
        AnimImpImpl* pImpl);
    virtual ~XMLAnimationsEffectContext();

    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList );
};

class XMLAnimationsSoundContext : public SvXMLImportContext
{
    XMLAnimationsEffectContext* mpParent;

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
:   SvXMLImportContext(rImport, nPrfx, rLocalName),
    mpImpl( pImpl ),
    meKind( XMLE_SHOW ), mbTextEffect( sal_False ),
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
                maShapeId = sValue;
            }
            else if( IsXMLToken( aLocalName, XML_COLOR ) )
            {
                ::sax::Converter::convertColor(maDimColor, sValue);
            }
            break;

        case XML_NAMESPACE_PRESENTATION:
            if( IsXMLToken( aLocalName, XML_EFFECT ) )
            {
                sal_uInt16 eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationEffect_EnumMap ) )
                    meEffect = (XMLEffect)eEnum;
            }
            else if( IsXMLToken(aLocalName, XML_DIRECTION ) )
            {
                sal_uInt16 eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationDirection_EnumMap ) )
                    meDirection = (XMLEffectDirection)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_START_SCALE ) )
            {
                sal_Int32 nScale;
                if (::sax::Converter::convertPercent( nScale, sValue ))
                    mnStartScale = (sal_Int16)nScale;
            }
            else if( IsXMLToken( aLocalName, XML_SPEED ) )
            {
                sal_uInt16 eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationSpeed_EnumMap ) )
                    meSpeed = (AnimationSpeed)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_PATH_ID ) )
            {
                maPathShapeId = sValue;
            }
            break;
        }
    }
}

XMLAnimationsEffectContext::~XMLAnimationsEffectContext()
{
}

SvXMLImportContext * XMLAnimationsEffectContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
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

        if( !maShapeId.isEmpty() )
        {
            Reference< XPropertySet > xSet;
            if( mpImpl->maLastShapeId != maShapeId )
            {
                xSet = Reference< XPropertySet >::query( GetImport().getInterfaceToIdentifierMapper().getReference( maShapeId ) );
                if( xSet.is() )
                {
                    // check for presentation shape service
                    {
                        Reference< XServiceInfo > xServiceInfo( xSet, UNO_QUERY );
                        if( !xServiceInfo.is() || !xServiceInfo->supportsService( mpImpl->msPresShapeService ) )
                            return;
                    }

                    mpImpl->maLastShapeId = maShapeId;
                    mpImpl->mxLastShape = xSet;
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

                    aAny <<= maDimColor;
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

                        xSet->setPropertyValue( mbTextEffect ? mpImpl->msTextEffect : mpImpl->msEffect, makeAny( eEffect ) );
                        xSet->setPropertyValue( mpImpl->msSpeed, makeAny( meSpeed ) );

                        if( eEffect == AnimationEffect_PATH && !maPathShapeId.isEmpty() )
                        {
                            Reference< XShape > xPath( GetImport().getInterfaceToIdentifierMapper().getReference( maPathShapeId ), UNO_QUERY );
                            if( xPath.is() )
                                xSet->setPropertyValue( mpImpl->msAnimPath, makeAny( xPath ) );
                        }
                    }
                }
            }
            if( !maSoundURL.isEmpty() )
            {
                if( xSet.is() )
                {
                    aAny <<= maSoundURL;
                    xSet->setPropertyValue( mpImpl->msSound, aAny );

                    aAny <<= bool2any( mbPlayFull );
                    xSet->setPropertyValue( mpImpl->msPlayFull, aAny );

                    aAny <<= bool2any( sal_True );
                    xSet->setPropertyValue( mpImpl->msSoundOn, aAny );
                }
                else
                {
                    OSL_FAIL("XMLAnimationsEffectContext::EndElement - Sound URL without a XPropertySet!");
                }
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL( "exception caught while importing animation information!" );
    }
}

TYPEINIT1( XMLAnimationsContext, SvXMLImportContext );

XMLAnimationsContext::XMLAnimationsContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& )
: SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    mpImpl = new AnimImpImpl();
}

XMLAnimationsContext::~XMLAnimationsContext()
{
    delete mpImpl;
}

SvXMLImportContext * XMLAnimationsContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return new XMLAnimationsEffectContext( GetImport(), nPrefix, rLocalName,  xAttrList, mpImpl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
