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

#include "oox/ppt/timenode.hxx"

#include <boost/bind.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>

#include "oox/helper/helper.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "sal/log.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::presentation;

namespace oox { namespace ppt {

        OUString TimeNode::getServiceName( sal_Int16 nNodeType )
        {
            OUString sServiceName;
            switch( nNodeType )
            {
            case AnimationNodeType::PAR:
                sServiceName = "com.sun.star.animations.ParallelTimeContainer";
                break;
            case AnimationNodeType::SEQ:
                sServiceName = "com.sun.star.animations.SequenceTimeContainer";
                break;
            case AnimationNodeType::ANIMATE:
                sServiceName = "com.sun.star.animations.Animate";
                break;
            case AnimationNodeType::ANIMATECOLOR:
                sServiceName = "com.sun.star.animations.AnimateColor";
                break;
            case AnimationNodeType::TRANSITIONFILTER:
                sServiceName = "com.sun.star.animations.TransitionFilter";
                break;
            case AnimationNodeType::ANIMATEMOTION:
                sServiceName = "com.sun.star.animations.AnimateMotion";
                break;
            case AnimationNodeType::ANIMATETRANSFORM:
                sServiceName = "com.sun.star.animations.AnimateTransform";
                break;
            case AnimationNodeType::COMMAND:
                sServiceName = "com.sun.star.animations.Command";
                break;
            case AnimationNodeType::SET:
                sServiceName = "com.sun.star.animations.AnimateSet";
                break;
            case AnimationNodeType::AUDIO:
                sServiceName = "com.sun.star.animations.Audio";
                break;
            default:
                SAL_INFO("oox.ppt","OOX: uhandled type " << nNodeType );
                break;
            }
            return sServiceName;
        }



    TimeNode::TimeNode( sal_Int16 nNodeType )
        : mnNodeType( nNodeType )
        , mbHasEndSyncValue( false )
    {
    }


    TimeNode::~TimeNode()
    {
    }

// BEGIN CUT&PASTE from sd/source/filter/ppt/pptinanimations.hxx
// --------------------------------------------------------------------
    static void fixMainSequenceTiming( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
    {
        try
        {
            bool bFirst = true;
            Reference< XEnumerationAccess > xEA( xNode, UNO_QUERY_THROW );
            Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
            while( xE->hasMoreElements() )
            {
                // click node
                Reference< XAnimationNode > xClickNode( xE->nextElement(), UNO_QUERY );

                Event aEvent;
                aEvent.Trigger = EventTrigger::ON_NEXT;
                aEvent.Repeat = 0;
                xClickNode->setBegin( makeAny( aEvent ) );

                if( bFirst )
                {
                    bFirst = false;
                    Reference< XEnumerationAccess > xEA2( xClickNode, UNO_QUERY_THROW );
                    Reference< XEnumeration > xE2( xEA2->createEnumeration(), UNO_QUERY_THROW );
                    if( xE2->hasMoreElements() )
                    {
                        // with node
                        xE2->nextElement() >>= xEA2;
                        if( xEA2.is() )
                            xE2.query( xEA2->createEnumeration() );
                        else
                            xE2.clear();

                        if( xE2.is() && xE2->hasMoreElements() )
                        {
                            Reference< XAnimationNode > xEffectNode( xE2->nextElement(), UNO_QUERY_THROW );
                            const Sequence< NamedValue > aUserData( xEffectNode->getUserData() );
                            const NamedValue* p = aUserData.getConstArray();
                            sal_Int32 nLength = aUserData.getLength();
                            while( nLength-- )
                            {
                                if ( p->Name == "node-type" )
                                {
                                    sal_Int16 nNodeType = 0;
                                    p->Value >>= nNodeType;
                                    if( nNodeType != ::com::sun::star::presentation::EffectNodeType::ON_CLICK )
                                    {
                                        // first effect does not start on click, so correct
                                        // first click nodes begin to 0s
                                        xClickNode->setBegin( makeAny( (double)0.0 ) );
                                        break;
                                    }
                                }
                                p++;
                            }
                        }
                    }
                }
            }
        }
        catch( Exception& e )
        {
            (void)e;
            SAL_INFO("oox.ppt","fixMainSequenceTiming(), exception caught!" );
        }
    }

// --------------------------------------------------------------------

    static void fixInteractiveSequenceTiming( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
    {
        try
        {
            Any aBegin( xNode->getBegin() );
            Any aEmpty;
            xNode->setBegin( aEmpty );

            Reference< XEnumerationAccess > xEA( xNode, UNO_QUERY_THROW );
            Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
            while( xE->hasMoreElements() )
            {
                // click node
                Reference< XAnimationNode > xClickNode( xE->nextElement(), UNO_QUERY );
                xClickNode->setBegin( aBegin );
            }
        }
        catch( Exception& e )
        {
            (void)e;
            SAL_INFO("oox.ppt","fixInteractiveSequenceTiming(), exception caught!" );
        }
    }

// END CUT&PASTE

    void TimeNode::addNode( const XmlFilterBase& rFilter, const Reference< XAnimationNode >& rxNode, const SlidePersistPtr & pSlide )
    {
        try {
            OUString sServiceName = getServiceName( mnNodeType );
            Reference< XAnimationNode > xNode = createAndInsert( rFilter, sServiceName, rxNode );
            setNode( rFilter, xNode, pSlide );
        }
        catch( const Exception& e )
        {
            SAL_INFO("oox.ppt","OOX: exception raised in TimeNode::addNode() - " <<
                                 OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }

    void TimeNode::setNode( const XmlFilterBase& rFilter, const Reference< XAnimationNode >& xNode, const SlidePersistPtr & pSlide )
    {
        SAL_WARN_IF( !xNode.is(), "oox.ppt", "null node passed" );

        try {
            if( !msId.isEmpty() )
            {
                pSlide->getAnimNodesMap()[ msId ] = xNode;
            }

            if( mpTarget )
            {
                sal_Int16 nSubType(0);
                maNodeProperties[ NP_TARGET ] = mpTarget->convert( pSlide, nSubType );
                if( mpTarget->mnType == XML_spTgt )
                {
                    maNodeProperties[ NP_SUBITEM ] <<= nSubType;
                }
            }

            if( !maStCondList.empty() )
            {
                Any aAny = AnimationCondition::convertList( pSlide, maStCondList );
                 if( aAny.hasValue() )
                {
                    xNode->setBegin( aAny );
                }

            }
            if( !maEndCondList.empty() )
            {
                Any aAny = AnimationCondition::convertList( pSlide, maEndCondList );
                if( aAny.hasValue() )
                {
                    xNode->setEnd( aAny );
                }
            }
#if 0  // FIXME even the binary filter has this disabled.
            if( !maNextCondList.empty() )
            {
                Any aAny = AnimationCondition::convertList( pSlide, maNextCondList );
                if( aAny.hasValue() )
                {
                    xNode->setNext( aAny );
                }
            }
            if( !maPrevCondList.empty() )
            {
                Any aAny = AnimationCondition::convertList( pSlide, maPrevCondList );
                if( aAny.hasValue() )
                {
                    xNode->setPrev( aAny );
                }
            }
#endif
            if( mbHasEndSyncValue )
            {
                Any aValue = maEndSyncValue.convert( pSlide );
                xNode->setEndSync(aValue);
            }

            if( !maUserData.empty() )
            {
                Sequence< NamedValue > aUserDataSeq( static_cast< sal_Int32 >( maUserData.size() ) );
                NamedValue* pValues = aUserDataSeq.getArray();
                for( UserDataMap::const_iterator aIt = maUserData.begin(), aEnd = maUserData.end(); aIt != aEnd; ++aIt, ++pValues )
                {
                    pValues->Name = aIt->first;
                    pValues->Value = aIt->second;
                }
                maNodeProperties[ NP_USERDATA ] <<= aUserDataSeq;
            }

            Reference< XAnimate > xAnimate( xNode, UNO_QUERY );
            Reference< XAnimateColor > xAnimateColor( xNode, UNO_QUERY );
            Reference< XAnimateMotion > xAnimateMotion( xNode, UNO_QUERY );
            Reference< XAnimateTransform > xAnimateTransform( xNode, UNO_QUERY );
            Reference< XCommand > xCommand( xNode, UNO_QUERY );
            Reference< XIterateContainer > xIterateContainer( xNode, UNO_QUERY );
            sal_Int16 nInt16 = 0;
            sal_Bool bBool = sal_False;
            double fDouble = 0;
            OUString sString;
            Sequence< NamedValue > aSeq;

            for( int i = 0; i < _NP_SIZE; i++)
            {
                Any & aValue( maNodeProperties[ i ] );
                if( aValue.hasValue() )
                {
                    switch( i )
                    {
                    case NP_TO:
                        if( xAnimate.is() )
                            xAnimate->setTo( aValue );
                        break;
                    case NP_FROM:
                        if( xAnimate.is() )
                            xAnimate->setFrom( aValue );
                        break;
                    case NP_BY:
                        if( xAnimate.is() )
                            xAnimate->setBy( aValue );
                        break;
                    case NP_TARGET:
                        if( xAnimate.is() )
                            xAnimate->setTarget( aValue );
                        break;
                    case NP_SUBITEM:
                        if( xAnimate.is() )
                        {
                            if( aValue >>= nInt16 )
                                xAnimate->setSubItem( nInt16 );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_ATTRIBUTENAME:
                        if( xAnimate.is() )
                        {
                            if( aValue >>= sString )
                                xAnimate->setAttributeName( sString );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_CALCMODE:
                        if( xAnimate.is() )
                        {
                            if( aValue >>= nInt16 )
                                xAnimate->setCalcMode( nInt16 );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_KEYTIMES:
                        if( xAnimate.is() )
                        {
                            Sequence<double> aKeyTimes;
                            if( aValue >>= aKeyTimes )
                                xAnimate->setKeyTimes(aKeyTimes);
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_VALUES:
                        if( xAnimate.is() )
                        {
                            Sequence<Any> aValues;
                            if( aValue >>= aValues )
                                xAnimate->setValues(aValues);
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_FORMULA:
                        if( xAnimate.is() )
                        {
                            if( aValue >>= sString )
                                xAnimate->setFormula(sString);
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_COLORINTERPOLATION:
                        if( xAnimateColor.is() )
                        {
                            if( aValue >>= nInt16 )
                                xAnimateColor->setColorInterpolation( nInt16 );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_DIRECTION:
                        if( xAnimateColor.is() )
                        {
                            if( aValue >>= bBool )
                                xAnimateColor->setDirection( bBool );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_PATH:
                        if( xAnimateMotion.is() )
                            xAnimateMotion->setPath( aValue );
                        break;
                    case NP_TRANSFORMTYPE:
                        if( xAnimateTransform.is() )
                        {
                            if( aValue >>= nInt16 )
                                xAnimateTransform->setTransformType( nInt16 );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_USERDATA:
                        if( aValue >>= aSeq )
                            xNode->setUserData( aSeq );
                        else
                        {
                            SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                        }
                        break;
                    case NP_ACCELERATION:
                        if( aValue >>= fDouble )
                            xNode->setAcceleration( fDouble );
                        else
                        {
                            SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                        }
                        break;
                    case NP_DECELERATE:
                        if( aValue >>= fDouble )
                            xNode->setDecelerate( fDouble );
                        else
                        {
                            SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                        }
                        break;
                    case NP_AUTOREVERSE:
                        if( aValue >>= bBool )
                            xNode->setAutoReverse( bBool );
                        else
                        {
                            SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                        }
                        break;
                    case NP_DURATION:
                        xNode->setDuration( aValue );
                        break;
                    case NP_FILL:
                        if( aValue >>= nInt16 )
                            xNode->setFill( nInt16 );
                        else
                        {
                            SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                        }
                        break;
                    case NP_REPEATCOUNT:
                        xNode->setRepeatCount( aValue );
                        break;
                    case NP_REPEATDURATION:
                        xNode->setRepeatDuration( aValue );
                        break;
                    case NP_RESTART:
                        if( aValue >>= nInt16 )
                            xNode->setRestart( nInt16 );
                        else
                        {
                            SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                        }
                        break;
                    case NP_COMMAND:
                        if( xCommand.is() )
                        {
                            if( aValue >>= nInt16 )
                                xCommand->setCommand( nInt16 );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_PARAMETER:
                        if( xCommand.is() )
                            xCommand->setParameter( aValue );
                        break;
                    case NP_ITERATETYPE:
                        if( xIterateContainer.is() )
                        {
                            if( aValue >>= nInt16 )
                                xIterateContainer->setIterateType( nInt16 );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    case NP_ITERATEINTERVAL:
                        if( xIterateContainer.is() )
                        {
                            if( aValue >>= fDouble )
                                xIterateContainer->setIterateInterval( fDouble );
                            else
                            {
                                SAL_INFO("oox.ppt","any >>= failed " << __LINE__ );
                            }
                        }
                        break;
                    default:
                        SAL_INFO("oox.ppt","ERR-OOX: unknown prop index " << i );
                        break;
                    }
                }
            }

            if( mnNodeType == AnimationNodeType::TRANSITIONFILTER )
            {

                Reference< XTransitionFilter > xFilter( xNode, UNO_QUERY );
                maTransitionFilter.setTransitionFilterProperties( xFilter );
            }

            std::for_each( maChildren.begin(), maChildren.end(),
                           boost::bind(&TimeNode::addNode, _1, boost::cref(rFilter), boost::ref(xNode),
                                       boost::ref(pSlide) ) );

            switch( mnNodeType )
            {
            case AnimationNodeType::SEQ:
            {
                sal_Int16 nEnum = 0;
                if( maUserData[ "node-type" ] >>= nEnum )
                {
                    if( nEnum == EffectNodeType::MAIN_SEQUENCE )
                    {
                        fixMainSequenceTiming( xNode );
                    }
                    else if( nEnum ==  EffectNodeType::INTERACTIVE_SEQUENCE )
                    {
                        fixInteractiveSequenceTiming( xNode );
                    }
                }
                break;
            }
            case AnimationNodeType::PAR:
                // some other cut&paste... from AnimationImporter::importAnimationContainer()
                break;
            }
        }
        catch( const Exception& e )
        {
            SAL_INFO("oox.ppt","OOX: exception raised in TimeNode::setNode() - " << e.Message );
        }
    }


    Reference< XAnimationNode > TimeNode::createAndInsert(
            const XmlFilterBase& rFilter,
            const OUString& rServiceName,
            const Reference< XAnimationNode >& rxNode )
    {
        try {
            Reference< XAnimationNode > xNode( Reference<css::lang::XMultiServiceFactory>(rFilter.getComponentContext()->getServiceManager(), UNO_QUERY_THROW)->createInstance( rServiceName ), UNO_QUERY_THROW );;
            Reference< XTimeContainer > xParentContainer( rxNode, UNO_QUERY_THROW );

            xParentContainer->appendChild( xNode );
            return xNode;
        }
        catch( const Exception& e )
        {
            SAL_INFO("oox.ppt","OOX: exception raised in TimeNode::createAndInsert() trying to create a service " <<  OUStringToOString( rServiceName, RTL_TEXTENCODING_ASCII_US).getStr() << " = " << OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }

        return Reference< XAnimationNode >();
    }


    void    TimeNode::setId( sal_Int32 nId )
    {
        msId = OUString::number(nId);
    }

    void TimeNode::setTo( const Any & aTo )
    {
        maNodeProperties[ NP_TO ] = aTo;
    }


    void TimeNode::setFrom( const Any & aFrom )
    {
        maNodeProperties[ NP_FROM ] = aFrom;
    }

    void TimeNode::setBy( const Any & aBy )
    {
        maNodeProperties[ NP_BY ] = aBy;
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
