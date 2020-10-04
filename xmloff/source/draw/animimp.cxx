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

#include <sal/config.h>

#include <string_view>

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <sax/tools/converter.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <anim.hxx>
#include <animimp.hxx>

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

const SvXMLEnumMapEntry<XMLEffect> aXML_AnimationEffect_EnumMap[] =
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
    { XML_TOKEN_INVALID, XMLEffect(0) }
};

const SvXMLEnumMapEntry<XMLEffectDirection> aXML_AnimationDirection_EnumMap[] =
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
    { XML_TOKEN_INVALID, XMLEffectDirection(0) }
};

const SvXMLEnumMapEntry<AnimationSpeed> aXML_AnimationSpeed_EnumMap[] =
{
    { XML_SLOW,     AnimationSpeed_SLOW },
    { XML_MEDIUM,   AnimationSpeed_MEDIUM },
    { XML_FAST,     AnimationSpeed_FAST },
    { XML_TOKEN_INVALID, AnimationSpeed(0) }
};

AnimationEffect ImplSdXMLgetEffect( XMLEffect eKind, XMLEffectDirection eDirection, sal_Int16 nStartScale, bool /*bIn*/ )
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

namespace
{
    constexpr OUStringLiteral gsDimColor = u"DimColor";
    constexpr OUStringLiteral gsDimHide = u"DimHide";
    constexpr OUStringLiteral gsDimPrev = u"DimPrevious";
    constexpr std::u16string_view gsEffect = u"Effect";
    constexpr OUStringLiteral gsPlayFull = u"PlayFull";
    constexpr OUStringLiteral gsSound = u"Sound";
    constexpr OUStringLiteral gsSoundOn = u"SoundOn";
    constexpr OUStringLiteral gsSpeed = u"Speed";
    constexpr std::u16string_view gsTextEffect = u"TextEffect";
    constexpr OUStringLiteral gsPresShapeService = u"com.sun.star.presentation.Shape";
    constexpr OUStringLiteral gsAnimPath = u"AnimationPath";
    constexpr OUStringLiteral gsIsAnimation = u"IsAnimation";
};

namespace {

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
    rtl::Reference<XMLAnimationsContext> mxAnimationsContext;

    XMLActionKind   meKind;
    bool        mbTextEffect;
    OUString        maShapeId;

    XMLEffect       meEffect;
    XMLEffectDirection  meDirection;
    sal_Int16       mnStartScale;

    AnimationSpeed  meSpeed;
    sal_Int32       maDimColor;
    OUString        maSoundURL;
    bool        mbPlayFull;
    OUString        maPathShapeId;

public:

    XMLAnimationsEffectContext( SvXMLImport& rImport,
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& xAttrList,
        XMLAnimationsContext& rAnimationsContext);

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override {}

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class XMLAnimationsSoundContext : public SvXMLImportContext
{
public:

    XMLAnimationsSoundContext( SvXMLImport& rImport, sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList, XMLAnimationsEffectContext* pParent );

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override {}
};

}

XMLAnimationsSoundContext::XMLAnimationsSoundContext( SvXMLImport& rImport, sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList, XMLAnimationsEffectContext* pParent )
: SvXMLImportContext( rImport )
{
    if( !pParent || nElement != XML_ELEMENT(PRESENTATION, XML_SOUND) )
        return;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString sValue = aIter.toString();
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(XLINK, XML_HREF):
                pParent->maSoundURL = rImport.GetAbsoluteReference(sValue);
                break;
            case XML_ELEMENT(PRESENTATION, XML_PLAY_FULL):
                pParent->mbPlayFull = IsXMLToken( sValue, XML_TRUE );
                break;
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

XMLAnimationsEffectContext::XMLAnimationsEffectContext( SvXMLImport& rImport,
    sal_Int32 nElement,
    const Reference< XFastAttributeList >& xAttrList,
    XMLAnimationsContext& rAnimationsContext )
:   SvXMLImportContext(rImport),
    mxAnimationsContext( &rAnimationsContext ),
    meKind( XMLE_SHOW ), mbTextEffect( false ),
    meEffect( EK_none ), meDirection( ED_none ), mnStartScale( 100 ),
    meSpeed( AnimationSpeed_MEDIUM ), maDimColor(0), mbPlayFull( false )
{
    switch(nElement & TOKEN_MASK)
    {
        case XML_SHOW_SHAPE:
            meKind = XMLE_SHOW;
            break;
        case XML_SHOW_TEXT:
            meKind = XMLE_SHOW;
            mbTextEffect = true;
            break;
        case XML_HIDE_SHAPE:
            meKind = XMLE_HIDE;
            break;
        case XML_HIDE_TEXT:
            meKind = XMLE_HIDE;
            mbTextEffect = true;
            break;
        case XML_DIM:
            meKind = XMLE_DIM;
            break;
        case XML_PLAY:
            meKind = XMLE_PLAY;
            break;
        default:
            SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
            // unknown action, overread
            return;
    }

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString sValue = aIter.toString();
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(DRAW, XML_SHAPE_ID):
                maShapeId = sValue;
                break;
            case XML_ELEMENT(DRAW, XML_COLOR):
                ::sax::Converter::convertColor(maDimColor, sValue);
                break;

            case XML_ELEMENT(PRESENTATION, XML_EFFECT):
                SvXMLUnitConverter::convertEnum( meEffect, sValue, aXML_AnimationEffect_EnumMap );
                break;
            case XML_ELEMENT(PRESENTATION, XML_DIRECTION):
                SvXMLUnitConverter::convertEnum( meDirection, sValue, aXML_AnimationDirection_EnumMap );
                break;
            case XML_ELEMENT(PRESENTATION, XML_START_SCALE):
            {
                sal_Int32 nScale;
                if (::sax::Converter::convertPercent( nScale, sValue ))
                    mnStartScale = static_cast<sal_Int16>(nScale);
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_SPEED):
                SvXMLUnitConverter::convertEnum( meSpeed, sValue, aXML_AnimationSpeed_EnumMap );
                break;
            case XML_ELEMENT(PRESENTATION, XML_PATH_ID):
                maPathShapeId = sValue;
                break;
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLAnimationsEffectContext::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return new XMLAnimationsSoundContext( GetImport(), nElement, xAttrList, this );
}

void XMLAnimationsEffectContext::endFastElement(sal_Int32 )
{
    // set effect on shape

    try
    {
        if( !maShapeId.isEmpty() )
        {
            Reference< XPropertySet > xSet;
            if( mxAnimationsContext->maLastShapeId != maShapeId )
            {
                xSet.set( GetImport().getInterfaceToIdentifierMapper().getReference( maShapeId ), UNO_QUERY );
                if( xSet.is() )
                {
                    // check for presentation shape service
                    {
                        Reference< XServiceInfo > xServiceInfo( xSet, UNO_QUERY );
                        if( !xServiceInfo.is() || !xServiceInfo->supportsService( gsPresShapeService ) )
                            return;
                    }

                    mxAnimationsContext->maLastShapeId = maShapeId;
                    mxAnimationsContext->mxLastShape = xSet;
                }
            }
            else
            {
                xSet = mxAnimationsContext->mxLastShape;
            }

            if( xSet.is() )
            {
                if( meKind == XMLE_DIM )
                {
                    xSet->setPropertyValue( gsDimPrev, Any(true) );

                    xSet->setPropertyValue( gsDimColor, Any(maDimColor) );
                }
                else if( meKind == XMLE_PLAY )
                {
                    xSet->setPropertyValue( gsIsAnimation, Any(true) );

                    // #i42894# speed is not supported for the old group animation fallback, so no need to set it
                    // aAny <<= meSpeed;
                    // xSet->setPropertyValue( mpImpl->msSpeed, aAny );
                }
                else
                {
                    if( meKind == XMLE_HIDE && !mbTextEffect && meEffect == EK_none )
                    {
                        xSet->setPropertyValue( gsDimHide, Any(true) );
                    }
                    else
                    {
                        const AnimationEffect eEffect = ImplSdXMLgetEffect( meEffect, meDirection, mnStartScale, meKind == XMLE_SHOW );

                        auto const s = mbTextEffect ? gsTextEffect : gsEffect;
                        xSet->setPropertyValue( s, makeAny( eEffect ) );
                        xSet->setPropertyValue( gsSpeed, makeAny( meSpeed ) );

                        if( eEffect == AnimationEffect_PATH && !maPathShapeId.isEmpty() )
                        {
                            Reference< XShape > xPath( GetImport().getInterfaceToIdentifierMapper().getReference( maPathShapeId ), UNO_QUERY );
                            if( xPath.is() )
                                xSet->setPropertyValue( gsAnimPath, makeAny( xPath ) );
                        }
                    }
                }
            }
            if( !maSoundURL.isEmpty() )
            {
                if( xSet.is() )
                {
                    xSet->setPropertyValue( gsSound, Any(maSoundURL) );
                    xSet->setPropertyValue( gsPlayFull, Any(mbPlayFull) );
                    xSet->setPropertyValue( gsSoundOn, Any(true) );
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
        TOOLS_WARN_EXCEPTION("xmloff.draw", "exception caught while importing animation information!");
    }
}


XMLAnimationsContext::XMLAnimationsContext( SvXMLImport& rImport )
: SvXMLImportContext(rImport)
{
}

void XMLAnimationsContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLAnimationsContext::createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return new XMLAnimationsEffectContext( GetImport(), nElement, xAttrList, *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
