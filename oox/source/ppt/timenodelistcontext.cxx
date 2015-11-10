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

#include "oox/ppt/timenodelistcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>
#include <rtl/math.hxx>

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include "oox/helper/attributelist.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "drawingml/colorchoicecontext.hxx"
#include "oox/ppt/slidetransition.hxx"

#include "animvariantcontext.hxx"
#include "commonbehaviorcontext.hxx"
#include "conditioncontext.hxx"
#include "commontimenodecontext.hxx"
#include "timeanimvaluecontext.hxx"
#include "animationtypes.hxx"

using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::xml::sax;
using ::com::sun::star::beans::NamedValue;

namespace oox { namespace ppt {

    struct AnimColor
    {
        AnimColor(sal_Int16 cs, sal_Int32 o, sal_Int32 t, sal_Int32 th )
            : colorSpace( cs ), one( o ), two( t ), three( th )
            {
            }

        Any get()
            {
                sal_Int32 nColor;
                Sequence< double > aHSL( 3 );
                Any aColor;

                switch( colorSpace )
                {
                case AnimationColorSpace::HSL:
                    aHSL[ 0 ] = double(one) / 100000;
                    aHSL[ 1 ] = double(two) / 100000;
                    aHSL[ 2 ] = double(three) / 100000;
                    aColor = Any(aHSL);
                    break;
                case AnimationColorSpace::RGB:
                    nColor = ( ( ( one * 128 ) / 1000 ) & 0xff ) << 16
                        | ( ( ( two * 128 ) / 1000 ) & 0xff ) << 8
                        | ( ( ( three * 128 ) / 1000 )  & 0xff );
                    aColor = Any(nColor);
                    break;
                default:
                    nColor = 0;
                    aColor = Any( nColor );
                    break;
                }
                return  aColor;
            }

        sal_Int16 colorSpace;
        sal_Int32 one;
        sal_Int32 two;
        sal_Int32 three;
    };

    /** CT_TLMediaNodeAudio
            CT_TLMediaNodeVideo */
    class MediaNodeContext
        : public TimeNodeContext
    {
    public:
        MediaNodeContext( FragmentHandler2& rParent, sal_Int32  aElement,
                            const Reference< XFastAttributeList >& xAttribs,
                            const TimeNodePtr & pNode )
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
                , mbIsNarration( false )
                , mbFullScrn( false )
            {
                AttributeList attribs( xAttribs );

                switch( aElement )
                {
                case PPT_TOKEN( audio ):
                    mbIsNarration = attribs.getBool( XML_isNarration, false );
                    break;
                case PPT_TOKEN( video ):
                    mbFullScrn = attribs.getBool( XML_fullScrn, false );
                    break;
                default:
                    break;
                }
            }

        virtual void onEndElement() override
            {
                sal_Int32 aElement = getCurrentElement();
                if( aElement == PPT_TOKEN( audio ) )
                {
                    // TODO deal with mbIsNarration
                }
                else if( aElement == PPT_TOKEN( video ) )
                {
                    // TODO deal with mbFullScrn
                }
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                default:
                    break;
                }

                return this;
            }

    private:
        bool mbIsNarration;
        bool mbFullScrn;
    };

    /** CT_TLSetBehavior
     */
    class SetTimeNodeContext
        : public TimeNodeContext
    {
    public:
        SetTimeNodeContext( FragmentHandler2& rParent, sal_Int32  aElement,
                            const Reference< XFastAttributeList >& xAttribs,
                            const TimeNodePtr & pNode )
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            {

            }

        virtual ~SetTimeNodeContext() throw ()
            {
                if( maTo.hasValue() )
                {
                    // TODO
                    // HACK !!! discard and refactor
                    OUString aString;
                    if( maTo >>= aString )
                    {
                        OSL_TRACE( "Magic conversion %s", OUSTRING_TO_CSTR( aString ) );
                        maTo = makeAny( aString == "visible" );
                        if( !maTo.has<sal_Bool>() )
                            OSL_TRACE( "conversion failed" );
                    }
                    mpNode->setTo( maTo );
                }

            }

            virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( to ):
                    // CT_TLAnimVariant
                    return new AnimVariantContext( *this, aElementToken, maTo );
                default:
                    break;
                }

                return this;
            }
    private:
        Any  maTo;
    };

    /** CT_TLCommandBehavior
     */
    class CmdTimeNodeContext
        : public TimeNodeContext
    {
    public:
        CmdTimeNodeContext( FragmentHandler2& rParent, sal_Int32  aElement,
                            const Reference< XFastAttributeList >& xAttribs,
                            const TimeNodePtr & pNode )
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
                , maType(0)
            {
                switch ( aElement )
                {
                case PPT_TOKEN( cmd ):
                    msCommand = xAttribs->getOptionalValue( XML_cmd );
                    maType = xAttribs->getOptionalValueToken( XML_type, 0 );
                    break;
                default:
                    break;
                }
            }

        virtual ~CmdTimeNodeContext() throw ()
            {
            }

        virtual void onEndElement() override
            {
                if( isCurrentElement( PPT_TOKEN( cmd ) ) )
                {
                    try {
                        // see sd/source/filter/ppt/pptinanimations.cxx
                        // in AnimationImporter::importCommandContainer()
                        // REFACTOR?
                        // a good chunk of this code has been copied verbatim *sigh*
                        sal_Int16 nCommand = EffectCommands::CUSTOM;
                        NamedValue aParamValue;

                        switch( maType )
                        {
                        case XML_verb:
                            aParamValue.Name = "Verb";
                            // TODO make sure msCommand has what we want
                            aParamValue.Value <<= msCommand.toInt32();
                            nCommand = EffectCommands::VERB;
                            break;
                        case XML_evt:
                        case XML_call:
                            if ( msCommand == "onstopaudio" )
                            {
                                nCommand = EffectCommands::STOPAUDIO;
                            }
                            else if ( msCommand == "play" )
                            {
                                nCommand = EffectCommands::PLAY;
                            }
                            else if( msCommand == "playFrom" )
                            {
                                const OUString aMediaTime( msCommand.copy( 9, msCommand.getLength() - 10 ) );
                                rtl_math_ConversionStatus eStatus;
                                double fMediaTime = ::rtl::math::stringToDouble( aMediaTime, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus );
                                if( eStatus == rtl_math_ConversionStatus_Ok )
                                {
                                    aParamValue.Name = "MediaTime";
                                    aParamValue.Value <<= fMediaTime;
                                }
                                nCommand = EffectCommands::PLAY;
                            }
                            else if ( msCommand == "togglePause" )
                            {
                                nCommand = EffectCommands::TOGGLEPAUSE;
                            }
                            else if ( msCommand == "stop" )
                            {
                                nCommand = EffectCommands::STOP;
                            }
                            break;
                        }
                        mpNode->getNodeProperties()[ NP_COMMAND ] = makeAny((sal_Int16)nCommand);
                        if( nCommand == EffectCommands::CUSTOM )
                        {
                            OSL_TRACE("OOX: CmdTimeNodeContext::endFastElement(), unknown command!");
                            aParamValue.Name = "UserDefined";
                            aParamValue.Value <<= msCommand;
                        }
                        if( aParamValue.Value.hasValue() )
                        {
                            Sequence< NamedValue > aParamSeq( &aParamValue, 1 );
                            mpNode->getNodeProperties()[ NP_PARAMETER ] = makeAny( aParamSeq );
                        }
                    }
                    catch( RuntimeException& )
                    {
                        OSL_TRACE( "OOX: Exception in CmdTimeNodeContext::endFastElement()" );
                    }
                }
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                default:
                    break;
                }

                return this;
            }

    private:
        OUString msCommand;
        sal_Int32 maType;
    };

    /** CT_TLTimeNodeSequence
     */
    class SequenceTimeNodeContext
        : public TimeNodeContext
    {
    public:
        SequenceTimeNodeContext( FragmentHandler2& rParent, sal_Int32  aElement,
                                 const Reference< XFastAttributeList >& xAttribs,
                                 const TimeNodePtr & pNode )
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
                , mnNextAc(0)
                , mnPrevAc(0)
            {
                AttributeList attribs(xAttribs);
                mbConcurrent = attribs.getBool( XML_concurrent, false );
                mnNextAc = xAttribs->getOptionalValueToken( XML_nextAc, 0 );
                mnPrevAc = xAttribs->getOptionalValueToken( XML_prevAc, 0 );
            }

        virtual ~SequenceTimeNodeContext() throw()
            {
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cTn ):
                    return new CommonTimeNodeContext( *this, aElementToken, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( nextCondLst ):
                    return new CondListContext( *this, aElementToken, rAttribs.getFastAttributeList(), mpNode,
                                                   mpNode->getNextCondition() );
                case PPT_TOKEN( prevCondLst ):
                    return new CondListContext( *this, aElementToken, rAttribs.getFastAttributeList(), mpNode,
                                                   mpNode->getPrevCondition() );
                default:
                    break;
                }

                return this;
            }
    private:
        bool mbConcurrent;
        sal_Int32 mnNextAc, mnPrevAc;
    };

    /** CT_TLTimeNodeParallel
     *  CT_TLTimeNodeExclusive
     */
    class ParallelExclTimeNodeContext
        : public TimeNodeContext
    {
    public:
        ParallelExclTimeNodeContext( FragmentHandler2& rParent, sal_Int32  aElement,
                                     const Reference< XFastAttributeList >& xAttribs,
                                     const TimeNodePtr & pNode )
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            {
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cTn ):
                    return new CommonTimeNodeContext( *this, aElementToken, rAttribs.getFastAttributeList(), mpNode );
                default:
                    break;
                }

                return this;
            }

    protected:

    };

    /** CT_TLAnimateColorBehavior */
    class AnimColorContext
        : public TimeNodeContext
    {
    public:
        AnimColorContext( FragmentHandler2& rParent, sal_Int32  aElement,
                            const Reference< XFastAttributeList >& xAttribs,
                            const TimeNodePtr & pNode ) throw()
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            , mnColorSpace( xAttribs->getOptionalValueToken( XML_clrSpc, 0 ) )
            , mnDir( xAttribs->getOptionalValueToken( XML_dir, 0 ) )
            , mbHasByColor( false )
            , m_byColor( AnimationColorSpace::RGB, 0, 0, 0)
            {
            }
        virtual ~AnimColorContext() throw()
            {
            }

        virtual void onEndElement() override
            {
                //xParentNode
                if( isCurrentElement( mnElement ) )
                {
                    NodePropertyMap & pProps(mpNode->getNodeProperties());
                    pProps[ NP_DIRECTION ] = makeAny( mnDir == XML_cw );
                    pProps[ NP_COLORINTERPOLATION ] = makeAny( mnColorSpace == XML_hsl ? AnimationColorSpace::HSL : AnimationColorSpace::RGB );
                    const GraphicHelper& rGraphicHelper = getFilter().getGraphicHelper();
                    if( maToClr.isUsed() )
                        mpNode->setTo( Any( maToClr.getColor( rGraphicHelper ) ) );
                    if( maFromClr.isUsed() )
                        mpNode->setFrom( Any( maFromClr.getColor( rGraphicHelper ) ) );
                    if( mbHasByColor )
                        mpNode->setBy( m_byColor.get() );
                }
            }

            virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( hsl ):
                    // CT_TLByHslColorTransform
                {
                    if( mbHasByColor )
                    {
                        m_byColor.colorSpace = AnimationColorSpace::HSL;
                        m_byColor.one = rAttribs.getInteger( XML_h, 0 );
                        m_byColor.two = rAttribs.getInteger( XML_s, 0 );
                        m_byColor.three = rAttribs.getInteger( XML_l, 0 );
                    }
                    return this;
                }
                case PPT_TOKEN( rgb ):
                {
                    if( mbHasByColor )
                    {
                        // CT_TLByRgbColorTransform
                        m_byColor.colorSpace = AnimationColorSpace::RGB;
                        m_byColor.one = rAttribs.getInteger( XML_r, 0 );
                        m_byColor.two = rAttribs.getInteger( XML_g, 0 );
                        m_byColor.three = rAttribs.getInteger( XML_b, 0 );
                    }
                    return this;
                }
                case PPT_TOKEN( by ):
                    // CT_TLByAnimateColorTransform
                    mbHasByColor = true;
                    return this;
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( to ):
                    // CT_Color
                    return new ColorContext( *this, maToClr );
                case PPT_TOKEN( from ):
                    // CT_Color
                    return new ColorContext( *this, maFromClr );

                default:
                    break;
                }

                return this;
            }

    private:
        sal_Int32 mnColorSpace;
        sal_Int32 mnDir;
        bool mbHasByColor;
        AnimColor m_byColor;
        oox::drawingml::Color maToClr;
        oox::drawingml::Color maFromClr;
    };

    /** CT_TLAnimateBehavior */
    class AnimContext
        : public TimeNodeContext
    {
    public:
        AnimContext( FragmentHandler2& rParent, sal_Int32  aElement,
                     const Reference< XFastAttributeList >& xAttribs,
                      const TimeNodePtr & pNode ) throw()
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            {
                NodePropertyMap & aProps( pNode->getNodeProperties() );
                sal_Int32 nCalcMode = xAttribs->getOptionalValueToken( XML_calcmode, 0 );
                if(nCalcMode)
                {
                    sal_Int16 nEnum = 0;
                    switch(nCalcMode)
                    {
                    case XML_discrete:
                        nEnum = AnimationCalcMode::DISCRETE;
                        break;
                    case XML_lin:
                        nEnum = AnimationCalcMode::LINEAR;
                        break;
                    case XML_fmla:
                    default:
                        // TODO what value is good ?
                        nEnum = AnimationCalcMode::DISCRETE;
                        break;
                    }
                    aProps[ NP_CALCMODE ] = makeAny(nEnum);
                }
                OUString aStr;
                aStr = xAttribs->getOptionalValue( XML_from );
                if( !aStr.isEmpty() )
                {
                    pNode->setFrom( makeAny( aStr ) );
                }
                aStr = xAttribs->getOptionalValue( XML_by );
                if( !aStr.isEmpty() )
                {
                    pNode->setBy( makeAny( aStr ) );
                }
                aStr = xAttribs->getOptionalValue( XML_to );
                if( !aStr.isEmpty() )
                {
                    pNode->setTo( makeAny( aStr ) );
                }
                mnValueType = xAttribs->getOptionalValueToken( XML_valueType, 0 );
            }

        virtual ~AnimContext() throw ()
            {
                ::std::list< TimeAnimationValue >::iterator iter, end;
                int nKeyTimes = maTavList.size();
                if( nKeyTimes > 0)
                {
                    int i;
                    Sequence< double > aKeyTimes( nKeyTimes );
                    Sequence< Any > aValues( nKeyTimes );

                    NodePropertyMap & aProps( mpNode->getNodeProperties() );
                    end = maTavList.end();
                    for(iter = maTavList.begin(), i=0; iter != end; ++iter,++i)
                    {
                        // TODO what to do if it is Timing_INFINITE ?
                        Any aTime = GetTimeAnimateValueTime( iter->msTime );
                        aTime >>= aKeyTimes[i];
                        aValues[i] = iter->maValue;

                        OUString aTest;
                        iter->maValue >>= aTest;
                        if( !aTest.isEmpty() )
                        {
                            aValues[i] = iter->maValue;
                        }
                        else
                        {
                            aProps[ NP_FORMULA ] <<= iter->msFormula;
                        }
                    }
                    aProps[ NP_VALUES ] <<= aValues;
                    aProps[ NP_KEYTIMES ] <<= aKeyTimes;
                }
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( tavLst ):
                    return new TimeAnimValueListContext ( *this, rAttribs.getFastAttributeList(), maTavList );
                default:
                    break;
                }

                return this;
            }
    private:
        sal_Int32              mnValueType;
        TimeAnimationValueList maTavList;
    };

    /** CT_TLAnimateScaleBehavior */
    class AnimScaleContext
        : public TimeNodeContext
    {
    public:
        AnimScaleContext( FragmentHandler2& rParent, sal_Int32  aElement,
                            const Reference< XFastAttributeList >& xAttribs,
                            const TimeNodePtr & pNode ) throw()
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
                , mbZoomContents( false )
            {
                AttributeList attribs( xAttribs );
                // TODO what to do with mbZoomContents
                mbZoomContents = attribs.getBool( XML_zoomContents, false );
                pNode->getNodeProperties()[ NP_TRANSFORMTYPE ]
                    = makeAny((sal_Int16)AnimationTransformType::SCALE);
            }

        virtual ~AnimScaleContext( ) throw( )
            {
            }

        virtual void onEndElement() override
            {
                if( isCurrentElement( mnElement ) )
                {
                    if( maTo.hasValue() )
                    {
                        mpNode->setTo( maTo );
                    }
                    if( maBy.hasValue() )
                    {
                        mpNode->setBy( maBy );
                    }
                    if( maFrom.hasValue() )
                    {
                        mpNode->setFrom( maFrom );
                    }
                }
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( to ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    maTo <<= p.X;
                    maTo <<= p.Y;
                    return this;
                }
                case PPT_TOKEN( from ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    maFrom <<= p.X;
                    maFrom <<= p.Y;
                    return this;
                }
                case PPT_TOKEN( by ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    maBy <<= p.X;
                    maBy <<= p.Y;
                    return this;
                }
                default:
                    break;
                }

                return this;
            }
    private:
        Any maBy;
        Any maFrom;
        Any maTo;
        bool mbZoomContents;
    };

    /** CT_TLAnimateRotationBehavior */
    class AnimRotContext
        : public TimeNodeContext
    {
    public:
        AnimRotContext( FragmentHandler2& rParent, sal_Int32  aElement,
                        const Reference< XFastAttributeList >& xAttribs,
                         const TimeNodePtr & pNode ) throw()
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            {
                AttributeList attribs( xAttribs );

                pNode->getNodeProperties()[ NP_TRANSFORMTYPE ]
                    = makeAny((sal_Int16)AnimationTransformType::ROTATE);
                // see also DFF_msofbtAnimateRotationData in
                // sd/source/filter/ppt/pptinanimations.cxx
                if(attribs.hasAttribute( XML_by ) )
                {
                    sal_Int32 nBy = attribs.getInteger( XML_by, 0 );
                    pNode->setBy( makeAny( (double) nBy ) );
                }
                if(attribs.hasAttribute( XML_from ) )
                {
                    sal_Int32 nFrom = attribs.getInteger( XML_from, 0 );
                    pNode->setFrom( makeAny( (double) nFrom ) );
                }
                if(attribs.hasAttribute( XML_to ) )
                {
                    sal_Int32 nTo = attribs.getInteger( XML_to, 0 );
                    pNode->setTo( makeAny( (double) nTo ) );
                }
            }

        virtual ~AnimRotContext( ) throw( )
            {
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                default:
                    break;
                }

                return this;
            }
    };

    /** CT_TLAnimateMotionBehavior */
    class AnimMotionContext
        : public TimeNodeContext
    {
    public:
        AnimMotionContext( FragmentHandler2& rParent, sal_Int32  aElement,
                         const Reference< XFastAttributeList >& xAttribs,
                          const TimeNodePtr & pNode ) throw()
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            {
                pNode->getNodeProperties()[ NP_TRANSFORMTYPE ]
                    = makeAny((sal_Int16)AnimationTransformType::TRANSLATE);

                AttributeList attribs( xAttribs );
                sal_Int32 nOrigin = xAttribs->getOptionalValueToken( XML_origin, 0 );
                if( nOrigin != 0 )
                {
                    switch(nOrigin)
                    {
                    case XML_layout:
                    case XML_parent:
                        break;
                    }
                    // TODO
                }

                OUString aStr = xAttribs->getOptionalValue( XML_path );
                // E can appear inside a number, so we only check for its presence at the end
                aStr = aStr.trim();
                if (aStr.endsWith("E"))
                    aStr = aStr.copy(0, aStr.getLength() - 1);
                aStr = aStr.trim();
                pNode->getNodeProperties()[ NP_PATH ] = makeAny(aStr);
                mnPathEditMode = xAttribs->getOptionalValueToken( XML_pathEditMode, 0 );
                msPtsTypes = xAttribs->getOptionalValue( XML_ptsTypes );
                mnAngle = attribs.getInteger( XML_rAng, 0 );
                // TODO make sure the units are right. Likely not.
            }

        virtual ~AnimMotionContext( ) throw()
            {
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( to ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    Any rAny;
                    rAny <<= p.X;
                    rAny <<= p.Y;
                    mpNode->setTo( rAny );
                    return this;
                }
                case PPT_TOKEN( from ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    Any rAny;
                    rAny <<= p.X;
                    rAny <<= p.Y;
                    mpNode->setFrom( rAny );
                    return this;
                }
                case PPT_TOKEN( by ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    Any rAny;
                    rAny <<= p.X;
                    rAny <<= p.Y;
                    mpNode->setBy( rAny );
                    return this;
                }
                case PPT_TOKEN( rCtr ):
                {
                    // CT_TLPoint
                    awt::Point p = GetPointPercent( rAttribs.getFastAttributeList() );
                    // TODO push
                    (void)p;
                    return this;
                }
                default:
                    break;
                }

                return this;
            }
    private:
        OUString msPtsTypes;
        sal_Int32 mnPathEditMode;
        sal_Int32 mnAngle;
    };

    /** CT_TLAnimateEffectBehavior */
    class AnimEffectContext
        : public TimeNodeContext
    {
    public:
        AnimEffectContext( FragmentHandler2& rParent, sal_Int32  aElement,
                             const Reference< XFastAttributeList >& xAttribs,
                             const TimeNodePtr & pNode ) throw()
            : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            {
                sal_Int32 nDir = xAttribs->getOptionalValueToken( XML_transition, 0 );
                OUString sFilter = xAttribs->getOptionalValue( XML_filter );
                // TODO
//              OUString sPrList = xAttribs->getOptionalValue( XML_prLst );

                if( !sFilter.isEmpty() )
                {
                    SlideTransition aFilter( sFilter );
                    aFilter.setMode( nDir != XML_out );
                    pNode->setTransitionFilter( aFilter );
                }
            }

        virtual ~AnimEffectContext( ) throw()
            {
            }

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
            {
                switch ( aElementToken )
                {
                case PPT_TOKEN( cBhvr ):
                    return new CommonBehaviorContext ( *this, rAttribs.getFastAttributeList(), mpNode );
                case PPT_TOKEN( progress ):
                    return new AnimVariantContext( *this, aElementToken, maProgress );
                    // TODO handle it.
                default:
                    break;
                }

                return this;
            }
    private:
        Any maProgress;
    };

    TimeNodeContext * TimeNodeContext::makeContext(
            FragmentHandler2& rParent, sal_Int32  aElement,
            const Reference< XFastAttributeList >& xAttribs,
            const TimeNodePtr & pNode )
    {
        TimeNodeContext *pCtx = nullptr;
        switch( aElement )
        {
        case PPT_TOKEN( animClr ):
            pCtx = new AnimColorContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( par ):
            pCtx = new ParallelExclTimeNodeContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( seq ):
            pCtx = new SequenceTimeNodeContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( excl ):
            pCtx = new ParallelExclTimeNodeContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( anim ):
            pCtx = new AnimContext ( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( animEffect ):
            pCtx = new AnimEffectContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( animMotion ):
            pCtx = new AnimMotionContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( animRot ):
            pCtx = new AnimRotContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( animScale ):
            pCtx = new AnimScaleContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( cmd ):
            pCtx = new CmdTimeNodeContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( set ):
            pCtx = new SetTimeNodeContext( rParent, aElement, xAttribs, pNode );
            break;
        case PPT_TOKEN( audio ):
        case PPT_TOKEN( video ):
            pCtx = new MediaNodeContext( rParent, aElement, xAttribs, pNode );
            break;
        default:
            break;
        }
        return pCtx;
    }

    TimeNodeContext::TimeNodeContext( FragmentHandler2& rParent, sal_Int32 aElement,
            const Reference< XFastAttributeList >& /*xAttribs*/,
            const TimeNodePtr & pNode ) throw()
        : FragmentHandler2( rParent )
        , mnElement( aElement )
        , mpNode( pNode )
    {
    }

    TimeNodeContext::~TimeNodeContext( ) throw()
    {

    }

    TimeNodeListContext::TimeNodeListContext( FragmentHandler2& rParent, TimeNodePtrList & aList )
        throw()
        : FragmentHandler2( rParent )
            , maList( aList )
    {
    }

    TimeNodeListContext::~TimeNodeListContext( ) throw()
    {
    }

    ::oox::core::ContextHandlerRef TimeNodeListContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        sal_Int16 nNodeType;

        switch( aElementToken )
        {
        case PPT_TOKEN( par ):
            nNodeType = AnimationNodeType::PAR;
            break;
        case PPT_TOKEN( seq ):
            nNodeType = AnimationNodeType::SEQ;
            break;
        case PPT_TOKEN( excl ):
            // TODO pick the right type. We choose parallel for now as
            // there does not seem to be an "Exclusive"
            nNodeType = AnimationNodeType::PAR;
            break;
        case PPT_TOKEN( anim ):
            nNodeType = AnimationNodeType::ANIMATE;
            break;
        case PPT_TOKEN( animClr ):
            nNodeType = AnimationNodeType::ANIMATECOLOR;
            break;
        case PPT_TOKEN( animEffect ):
            nNodeType = AnimationNodeType::TRANSITIONFILTER;
            break;
        case PPT_TOKEN( animMotion ):
            nNodeType = AnimationNodeType::ANIMATEMOTION;
            break;
        case PPT_TOKEN( animRot ):
        case PPT_TOKEN( animScale ):
            nNodeType = AnimationNodeType::ANIMATETRANSFORM;
            break;
        case PPT_TOKEN( cmd ):
            nNodeType = AnimationNodeType::COMMAND;
            break;
        case PPT_TOKEN( set ):
            nNodeType = AnimationNodeType::SET;
            break;
        case PPT_TOKEN( audio ):
            nNodeType = AnimationNodeType::AUDIO;
            break;
        case PPT_TOKEN( video ):
            nNodeType = AnimationNodeType::AUDIO;
            OSL_TRACE( "OOX: video requested, gave Audio instead" );
            break;

        default:
            nNodeType = AnimationNodeType::CUSTOM;
            SAL_INFO("oox.ppt", "uhandled token " << aElementToken);
            break;
        }

        TimeNodePtr pNode(new TimeNode(nNodeType));
        maList.push_back( pNode );
        FragmentHandler2 * pContext = TimeNodeContext::makeContext( *this, aElementToken, rAttribs.getFastAttributeList(), pNode );

        return pContext ? pContext : this;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
