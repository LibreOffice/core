/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <tools/debug.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>

#include <sax/tools/converter.hxx>

#include <list>
#include <comphelper/extract.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/shapeexport.hxx>
#include "anim.hxx"


using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::xmloff::token;


const struct Effect
{
    XMLEffect meKind;
    XMLEffectDirection meDirection;
    sal_Int16 mnStartScale;
    bool mbIn;
}
    AnimationEffectMap[] =
{
    { EK_none, ED_none,             -1, true }, 
    { EK_fade, ED_from_left,        -1, true }, 
    { EK_fade, ED_from_top,         -1, true }, 
    { EK_fade, ED_from_right,       -1, true }, 
    { EK_fade, ED_from_bottom,      -1, true }, 
    { EK_fade, ED_to_center,        -1, true }, 
    { EK_fade, ED_from_center,      -1, true }, 
    { EK_move, ED_from_left,        -1, true }, 
    { EK_move, ED_from_top,         -1, true }, 
    { EK_move, ED_from_right,       -1, true }, 
    { EK_move, ED_from_bottom,      -1, true }, 
    { EK_stripes, ED_vertical,      -1, true }, 
    { EK_stripes, ED_horizontal,    -1, true }, 
    { EK_fade, ED_clockwise,        -1, true }, 
    { EK_fade, ED_cclockwise,       -1, true }, 
    { EK_fade, ED_from_upperleft,   -1, true }, 
    { EK_fade, ED_from_upperright,  -1, true }, 
    { EK_fade, ED_from_lowerleft,   -1, true }, 
    { EK_fade, ED_from_lowerright,  -1, true }, 
    { EK_close,ED_vertical,         -1, true }, 
    { EK_close,ED_horizontal,       -1, true }, 
    { EK_open, ED_vertical,         -1, true }, 
    { EK_open, ED_horizontal,       -1, true }, 
    { EK_move, ED_path,             -1, true }, 
    { EK_move, ED_to_left,          -1, false },
    { EK_move, ED_to_top,           -1, false },
    { EK_move, ED_to_right,         -1, false },
    { EK_move, ED_to_bottom,        -1, false },
    { EK_fade, ED_spiral_inward_left,   -1, true }, 
    { EK_fade, ED_spiral_inward_right,  -1, true }, 
    { EK_fade, ED_spiral_outward_left,  -1, true }, 
    { EK_fade, ED_spiral_outward_right, -1, true }, 
    { EK_dissolve, ED_none,         -1, true }, 
    { EK_wavyline, ED_from_left,    -1, true }, 
    { EK_wavyline, ED_from_top,     -1, true }, 
    { EK_wavyline, ED_from_right,   -1, true }, 
    { EK_wavyline, ED_from_bottom,  -1, true }, 
    { EK_random, ED_none,           -1, true }, 
    { EK_lines, ED_vertical,        -1, true }, 
    { EK_lines, ED_horizontal,      -1, true }, 
    { EK_laser, ED_from_left,       -1, true }, 
    { EK_laser, ED_from_top,        -1, true }, 
    { EK_laser, ED_from_right,      -1, true }, 
    { EK_laser, ED_from_bottom,     -1, true }, 
    { EK_laser, ED_from_upperleft,  -1, true }, 
    { EK_laser, ED_from_upperright, -1, true }, 
    { EK_laser, ED_from_lowerleft,  -1, true }, 
    { EK_laser, ED_from_lowerright, -1, true }, 
    { EK_appear,ED_none,            -1, true }, 
    { EK_hide,  ED_none,            -1, false },
    { EK_move,  ED_from_upperleft,  -1, true }, 
    { EK_move,  ED_from_upperright, -1, true }, 
    { EK_move,  ED_from_lowerright, -1, true }, 
    { EK_move,  ED_from_lowerleft,  -1, true }, 
    { EK_move,  ED_to_upperleft,    -1, false },
    { EK_move,  ED_to_upperright,   -1, false },
    { EK_move,  ED_to_lowerright,   -1, false },
    { EK_move,  ED_to_lowerleft,    -1, false },
    { EK_move_short, ED_from_left,  -1, true }, 
    { EK_move_short, ED_from_upperleft, -1, true }, 
    { EK_move_short, ED_from_top,       -1, true }, 
    { EK_move_short, ED_from_upperright,-1, true }, 
    { EK_move_short, ED_from_right,     -1, true }, 
    { EK_move_short, ED_from_lowerright,-1, true }, 
    { EK_move_short, ED_from_bottom,    -1, true }, 
    { EK_move_short, ED_from_lowerleft, -1, true }, 
    { EK_move_short, ED_to_left,        -1, false },
    { EK_move_short, ED_to_upperleft,   -1, false },
    { EK_move_short, ED_to_top,         -1, false },
    { EK_move_short, ED_to_upperright,  -1, false },
    { EK_move_short, ED_to_right,       -1, false },
    { EK_move_short, ED_to_lowerright,  -1, false },
    { EK_move_short, ED_to_bottom,      -1, false },
    { EK_move_short, ED_to_lowerleft,   -1, false },
    { EK_checkerboard, ED_vertical,     -1, true }, 
    { EK_checkerboard, ED_horizontal,   -1, true }, 
    { EK_rotate, ED_horizontal,     -1, true }, 
    { EK_rotate, ED_vertical,       -1, true }, 
    { EK_stretch,ED_horizontal,     -1, true }, 
    { EK_stretch,ED_vertical,       -1, true }, 
    { EK_stretch,ED_from_left,      -1, true }, 
    { EK_stretch,ED_from_upperleft, -1, true }, 
    { EK_stretch,ED_from_top,       -1, true }, 
    { EK_stretch,ED_from_upperright,-1, true }, 
    { EK_stretch,ED_from_right,     -1, true }, 
    { EK_stretch,ED_from_lowerright,-1, true }, 
    { EK_stretch,ED_from_bottom,    -1, true }, 
    { EK_stretch,ED_from_lowerleft, -1, true }, 
    { EK_move,  ED_none,             0, true }, 
    { EK_move,  ED_none,            50, true }, 
    { EK_move,  ED_spiral_inward_left,   0, true }, 
    { EK_move,  ED_none,           400, true }, 
    { EK_move,  ED_none,           200, true }, 
    { EK_move,  ED_spiral_inward_left, 400, true }, 
    { EK_move,  ED_from_left,        0, true }, 
    { EK_move,  ED_from_upperleft,   0, true }, 
    { EK_move,  ED_from_top,         0, true }, 
    { EK_move,  ED_from_upperright,  0, true }, 
    { EK_move,  ED_from_right,       0, true }, 
    { EK_move,  ED_from_lowerright,  0, true }, 
    { EK_move,  ED_from_bottom,      0, true }, 
    { EK_move,  ED_from_lowerleft,   0, true }, 
    { EK_move,  ED_from_center,      0, true }, 
    { EK_move,  ED_from_left,      400, true }, 
    { EK_move,  ED_from_upperleft, 400, true }, 
    { EK_move,  ED_from_top,       400, true }, 
    { EK_move,  ED_from_upperright,400, true }, 
    { EK_move,  ED_from_right,     400, true }, 
    { EK_move,  ED_from_lowerright,400, true }, 
    { EK_move,  ED_from_bottom,    400, true }, 
    { EK_move,  ED_from_lowerleft, 400, true }, 
    { EK_move,  ED_from_center,    400, true }  
};

void SdXMLImplSetEffect( AnimationEffect eEffect, XMLEffect& eKind, XMLEffectDirection& eDirection, sal_Int16& nStartScale, bool& bIn )
{
    if( eEffect < AnimationEffect_NONE || eEffect > AnimationEffect_ZOOM_OUT_FROM_CENTER )
    {
        OSL_FAIL( "unknown animation effect!" );
        eEffect = AnimationEffect_NONE;
    }

    const Effect& rEffect = AnimationEffectMap[eEffect];
    eKind = rEffect.meKind;
    eDirection = rEffect.meDirection;
    nStartScale = rEffect.mnStartScale;
    bIn = rEffect.mbIn;
}

enum XMLActionKind
{
    XMLE_SHOW,
    XMLE_HIDE,
    XMLE_DIM,
    XMLE_PLAY
};

struct XMLEffectHint
{
    XMLActionKind   meKind;
    sal_Bool        mbTextEffect;
    Reference<XShape> mxShape;

    XMLEffect       meEffect;
    XMLEffectDirection  meDirection;
    sal_Int16       mnStartScale;

    AnimationSpeed  meSpeed;
	sal_Int32		maDimColor;
    OUString        maSoundURL;
    sal_Bool        mbPlayFull;
    sal_Int32       mnPresId;
    sal_Int32       mnPathShapeId;

    bool operator<(const XMLEffectHint& rComp) const { return mnPresId < rComp.mnPresId; }

    XMLEffectHint()
    :   meKind( XMLE_SHOW ), mbTextEffect( sal_False ),
        meEffect( EK_none ), meDirection( ED_none ), mnStartScale( -1 ),
        meSpeed( AnimationSpeed_SLOW ), maDimColor(0), mbPlayFull( sal_False ),
        mnPresId( 0 ), mnPathShapeId( -1 )
        {}
};

class AnimExpImpl
{
public:
    list<XMLEffectHint> maEffects;
    UniReference< XMLShapeExport > mxShapeExp;

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
    OUString msIsAnimation;
    OUString msAnimPath;

    AnimExpImpl()
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
        msIsAnimation( "IsAnimation" ),
        msAnimPath( "AnimationPath" )
    {}
};

XMLAnimationsExporter::XMLAnimationsExporter( XMLShapeExport* pShapeExp )
{
    mpImpl = new AnimExpImpl;
    mpImpl->mxShapeExp = pShapeExp;
}

XMLAnimationsExporter::~XMLAnimationsExporter()
{
    delete mpImpl;
    mpImpl = NULL;
}

void XMLAnimationsExporter::prepare( Reference< XShape > xShape, SvXMLExport& )
{
    try
    {
        
        {
            Reference< XServiceInfo > xServiceInfo( xShape, UNO_QUERY );
            if( !xServiceInfo.is() || !xServiceInfo->supportsService("com.sun.star.presentation.Shape") )
                return;
        }

        Reference< XPropertySet > xProps( xShape, UNO_QUERY );
        if( xProps.is() )
        {
            AnimationEffect eEffect;
            xProps->getPropertyValue( mpImpl->msEffect ) >>= eEffect;
            if( eEffect == AnimationEffect_PATH )
            {
                Reference< XShape > xPath;
                xProps->getPropertyValue( mpImpl->msAnimPath ) >>= xPath;
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("exception caught while collection animation information!");
    }
}

void XMLAnimationsExporter::collect( Reference< XShape > xShape, SvXMLExport& rExport )
{
    try
    {
        
        {
            Reference< XServiceInfo > xServiceInfo( xShape, UNO_QUERY );
            if( !xServiceInfo.is() || !xServiceInfo->supportsService("com.sun.star.presentation.Shape") )
                return;
        }

        Reference< XPropertySet > xProps( xShape, UNO_QUERY );
        if( xProps.is() )
        {
            const OUString aEmptyStr;

            Reference< XPropertySetInfo > xInfo( xProps->getPropertySetInfo() );
            AnimationEffect eEffect;
            XMLEffectHint aEffect;

            if( any2bool( xProps->getPropertyValue( mpImpl->msSoundOn ) ) )
            {
                xProps->getPropertyValue( mpImpl->msSound ) >>= aEffect.maSoundURL;
                xProps->getPropertyValue( mpImpl->msPlayFull ) >>= aEffect.mbPlayFull;
            }

            xProps->getPropertyValue( mpImpl->msPresOrder ) >>= aEffect.mnPresId;
            xProps->getPropertyValue( mpImpl->msSpeed ) >>= aEffect.meSpeed;


            sal_Bool bIsAnimation = false;
            xProps->getPropertyValue( mpImpl->msIsAnimation ) >>= bIsAnimation;
            if( bIsAnimation )
            {
                aEffect.meKind = XMLE_PLAY;

                if( !aEffect.mxShape.is() )
                {
                    rExport.getInterfaceToIdentifierMapper().registerReference( xShape );
                    aEffect.mxShape = xShape;
                }

                mpImpl->maEffects.push_back( aEffect );
            }

            {

                xProps->getPropertyValue( mpImpl->msEffect ) >>= eEffect;
                if( eEffect != AnimationEffect_NONE )
                {
                    bool bIn = true;
                    SdXMLImplSetEffect( eEffect, aEffect.meEffect, aEffect.meDirection, aEffect.mnStartScale, bIn );

                    aEffect.meKind = bIn ? XMLE_SHOW : XMLE_HIDE;

                    if( !aEffect.mxShape.is() )
                    {
                        rExport.getInterfaceToIdentifierMapper().registerReference( xShape );
                        aEffect.mxShape = xShape;
                    }

                    if( eEffect == AnimationEffect_PATH )
                    {
                        Reference< XShape > xPath;
                        xProps->getPropertyValue( mpImpl->msAnimPath ) >>= xPath;
                        if( xPath.is() )
                        {


                        }
                    }
                    mpImpl->maEffects.push_back( aEffect );

                    aEffect.mnPathShapeId = -1;
                    aEffect.maSoundURL = aEmptyStr;
                }

                xProps->getPropertyValue( mpImpl->msTextEffect ) >>= eEffect;
                if( eEffect != AnimationEffect_NONE )
                {
                    bool bIn = true;
                    SdXMLImplSetEffect( eEffect, aEffect.meEffect, aEffect.meDirection, aEffect.mnStartScale, bIn );
                    aEffect.meKind = bIn ? XMLE_SHOW : XMLE_HIDE;
                    aEffect.mbTextEffect = sal_True;

                    if( !aEffect.mxShape.is() )
                    {
                        rExport.getInterfaceToIdentifierMapper().registerReference( xShape );
                        aEffect.mxShape = xShape;
                    }

                    mpImpl->maEffects.push_back( aEffect );
                    aEffect.mbTextEffect = sal_False;
                    aEffect.maSoundURL = aEmptyStr;
                }

                sal_Bool bDimPrev = false;
                sal_Bool bDimHide = false;
                xProps->getPropertyValue( mpImpl->msDimPrev ) >>= bDimPrev;
                xProps->getPropertyValue( mpImpl->msDimHide ) >>= bDimHide;
                if( bDimPrev || bDimHide )
                {
                    aEffect.meKind = bDimPrev ? XMLE_DIM : XMLE_HIDE;
                    aEffect.meEffect = EK_none;
                    aEffect.meDirection = ED_none;
                    aEffect.meSpeed = AnimationSpeed_MEDIUM;
                    if( bDimPrev )
                    {
                        xProps->getPropertyValue( mpImpl->msDimColor )
                            >>= aEffect.maDimColor;
                    }

                    if( !aEffect.mxShape.is() )
                    {
                        rExport.getInterfaceToIdentifierMapper().registerReference( xShape );
                        aEffect.mxShape = xShape;
                    }

                    mpImpl->maEffects.push_back( aEffect );
                    aEffect.maSoundURL = aEmptyStr;
                }
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("exception caught while collection animation information!");
    }
}

void XMLAnimationsExporter::exportAnimations( SvXMLExport& rExport )
{
    mpImpl->maEffects.sort();

    list<XMLEffectHint>::iterator aIter = mpImpl->maEffects.begin();
    const list<XMLEffectHint>::iterator aEnd = mpImpl->maEffects.end();

    OUStringBuffer sTmp;

    if( aIter != aEnd )
    {
        SvXMLElementExport aElement( rExport, XML_NAMESPACE_PRESENTATION, XML_ANIMATIONS, sal_True, sal_True );

        do
        {
            XMLEffectHint& rEffect = *aIter;

            DBG_ASSERT( rEffect.mxShape.is(), "shape id creation failed for animation effect?" );

            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_SHAPE_ID, rExport.getInterfaceToIdentifierMapper().getIdentifier( rEffect.mxShape ) );

            if( rEffect.meKind == XMLE_DIM )
            {
                

                ::sax::Converter::convertColor( sTmp, rEffect.maDimColor );
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COLOR, sTmp.makeStringAndClear() );

                SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, XML_DIM, sal_True, sal_True );
            }
            else if( rEffect.meKind == XMLE_PLAY )
            {
                if( rEffect.meSpeed != AnimationSpeed_MEDIUM )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meSpeed, aXML_AnimationSpeed_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SPEED, sTmp.makeStringAndClear() );
                }

                SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, XML_PLAY, sal_True, sal_True );
            }
            else
            {

                if( rEffect.meEffect != EK_none )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meEffect, aXML_AnimationEffect_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_EFFECT, sTmp.makeStringAndClear() );
                }

                if( rEffect.meDirection != ED_none )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meDirection, aXML_AnimationDirection_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_DIRECTION, sTmp.makeStringAndClear() );
                }

                if( rEffect.mnStartScale != -1 )
                {
                    ::sax::Converter::convertPercent(sTmp, rEffect.mnStartScale);
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_START_SCALE, sTmp.makeStringAndClear() );
                }

                if( rEffect.meSpeed != AnimationSpeed_MEDIUM )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meSpeed, aXML_AnimationSpeed_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SPEED, sTmp.makeStringAndClear() );
                }

                if( rEffect.mnPathShapeId != -1 )
                {
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PATH_ID, OUString::number( rEffect.mnPathShapeId ) );
                }

                enum XMLTokenEnum eLocalName;
                if( rEffect.meKind == XMLE_SHOW )
                {
                    if( rEffect.mbTextEffect )
                        eLocalName = XML_SHOW_TEXT;
                    else
                        eLocalName = XML_SHOW_SHAPE;
                }
                else
                {
                    if( rEffect.mbTextEffect )
                        eLocalName = XML_HIDE_TEXT;
                    else
                        eLocalName = XML_HIDE_SHAPE;
                }

                SvXMLElementExport aEle( rExport, XML_NAMESPACE_PRESENTATION, eLocalName, sal_True, sal_True );
                if( !rEffect.maSoundURL.isEmpty() )
                {
                    rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, rExport.GetRelativeReference(rEffect.maSoundURL) );
                    rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_NEW );
                    rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                    if( rEffect.mbPlayFull )
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PLAY_FULL, XML_TRUE );

                    SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, XML_SOUND, sal_True, sal_True );
                }
            }

            ++aIter;
        }
        while( aIter != aEnd );
    }

    mpImpl->maEffects.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
