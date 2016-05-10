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


#include <tools/diagnose_ex.h>

#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <comphelper/sequence.hxx>

#include "activitiesfactory.hxx"
#include "smilfunctionparser.hxx"
#include "accumulation.hxx"
#include "activityparameters.hxx"
#include "interpolation.hxx"
#include "tools.hxx"
#include "simplecontinuousactivitybase.hxx"
#include "discreteactivitybase.hxx"
#include "continuousactivitybase.hxx"
#include "continuouskeytimeactivitybase.hxx"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <cmath>
#include <vector>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace {

/** Traits template, to take formula application only for ValueType = double
 */
template<typename ValueType> struct FormulaTraits
{
    static ValueType getPresentationValue(
        const ValueType& rVal, const ExpressionNodeSharedPtr& )
    {
        return rVal;
    }
};

/// Specialization for ValueType = double
template<> struct FormulaTraits<double>
{
    static double getPresentationValue(
        double const& rVal, ExpressionNodeSharedPtr const& rFormula )
    {
        return rFormula ? (*rFormula)(rVal) : rVal;
    }
};

// Various ActivityBase specializations for different animator types
// =================================================================

/** FromToBy handler

    Provides the Activity specializations for FromToBy
    animations (e.g. those without a values list).

    This template makes heavy use of SFINAE, only one of
    the perform*() methods will compile for each of the
    base classes.

    Note that we omit the virtual keyword on the perform()
    overrides on purpose; those that actually do override
    baseclass virtual methods inherit the property, and
    the others won't increase our vtable. What's more,
    having all perform() method in the vtable actually
    creates POIs for them, which breaks the whole SFINAE
    concept (IOW, this template won't compile any longer).

    @tpl BaseType
    Base class to use for this activity. Only
    ContinuousActivityBase and DiscreteActivityBase are
    supported here.

    @tpl AnimationType
    Type of the Animation to call.
*/
template<class BaseType, typename AnimationType>
class FromToByActivity : public BaseType
{
public:
    typedef typename AnimationType::ValueType           ValueType;
    typedef boost::optional<ValueType>                  OptionalValueType;

private:
    // some compilers don't inline whose definition they haven't
    // seen before the call site...
    ValueType getPresentationValue( const ValueType& rVal ) const
    {
        return FormulaTraits<ValueType>::getPresentationValue( rVal, mpFormula);
    }

public:
    /** Create FromToByActivity.

        @param rFrom
        From this value, the animation starts

        @param rTo
        With this value, the animation ends

        @param rBy
        With this value, the animation increments the start value

        @param rParms
        Standard Activity parameter struct

        @param rAnim
        Shared ptr to AnimationType

        @param rInterpolator
        Interpolator object to be used for lerping between
        start and end value (need to be passed, since it
        might contain state, e.g. interpolation direction
        for HSL color space).

        @param bCumulative
        Whether repeated animations should cumulate the
        value, or start fresh each time.
    */
    FromToByActivity(
        const OptionalValueType&                      rTo,
        const OptionalValueType&                      rBy,
        const ActivityParameters&                     rParms,
        const ::boost::shared_ptr< AnimationType >&   rAnim,
        const Interpolator< ValueType >&              rInterpolator,
        bool                                          bCumulative )
        : BaseType( rParms ),
          mpFormula( rParms.mpFormula ),
          maStartValue(),
          maEndValue(),
          maPreviousValue(),
          maStartInterpolationValue(),
          mnIteration( 0 ),
          mpAnim( rAnim ),
          maInterpolator( rInterpolator ),
          mbDynamicStartValue( false ),
          mbCumulative( bCumulative )
    {
        ENSURE_OR_THROW( mpAnim, "Invalid animation object" );

        ENSURE_OR_THROW(
            rTo || rBy,
            "From and one of To or By, or To or By alone must be valid" );
    }

    virtual void endAnimation()
    {
        // end animation
        if (mpAnim)
            mpAnim->end();
    }

    /// perform override for ContinuousActivityBase
    void perform( double nModifiedTime, sal_uInt32 nRepeatCount ) const
    {
        if (this->isDisposed() || !mpAnim)
            return;

        // According to SMIL 3.0 spec 'to' animation if no other (lower priority)
        // animations are active or frozen then a simple interpolation is performed.
        // That is, the start interpolation value is constant while the animation
        // is running, and is equal to the underlying value retrieved when
        // the animation start.
        // However if another animation is manipulating the underlying value,
        // the 'to' animation will initially add to the effect of the lower priority
        // animation, and increasingly dominate it as it nears the end of the
        // simple duration, eventually overriding it completely.
        // That is, each time the underlying value is changed between two
        // computations of the animation function the new underlying value is used
        // as start value for the interpolation.
        // See:
        // http://www.w3.org/TR/SMIL3/smil-animation.html#animationNS-ToAnimation
        // (Figure 6 - Effect of Additive to animation example)
        // Moreover when a 'to' animation is repeated, at each new iteration
        // the start interpolation value is reset to the underlying value
        // of the animated property when the animation started,
        // as it is shown in the example provided by the SMIL 3.0 spec.
        // This is exactly as Firefox performs SVG 'to' animations.
        if( mbDynamicStartValue )
        {
            if( mnIteration != nRepeatCount )
            {
                mnIteration = nRepeatCount;
                maStartInterpolationValue = maStartValue;
            }
            else
            {
                ValueType aActualValue = mpAnim->getUnderlyingValue();
                if( aActualValue != maPreviousValue )
                    maStartInterpolationValue = aActualValue;
            }
        }

        ValueType aValue = maInterpolator( maStartInterpolationValue,
                                           maEndValue, nModifiedTime );

        // According to the SMIL spec:
        // Because 'to' animation is defined in terms of absolute values of
        // the target attribute, cumulative animation is not defined.
        if( mbCumulative && !mbDynamicStartValue )
        {
            // aValue = this.aEndValue * nRepeatCount + aValue;
            aValue = accumulate( maEndValue, nRepeatCount, aValue );
        }

        (*mpAnim)( getPresentationValue( aValue ) );

        if( mbDynamicStartValue )
        {
            maPreviousValue = mpAnim->getUnderlyingValue();
        }

    }

    using BaseType::perform;

    /// perform override for DiscreteActivityBase base
    void perform( sal_uInt32 nFrame, sal_uInt32 nRepeatCount ) const
    {
        if (this->isDisposed() || !mpAnim)
            return;
        (*mpAnim)(
            getPresentationValue(
                accumulate( maEndValue, mbCumulative ? nRepeatCount : 0,
                            lerp( maInterpolator,
                                  (mbDynamicStartValue
                                   ? mpAnim->getUnderlyingValue()
                                   : maStartValue),
                                  maEndValue,
                                  nFrame,
                                  BaseType::getNumberOfKeyTimes() ) ) ) );
    }

    using BaseType::isAutoReverse;

    virtual void performEnd()
    {
        // xxx todo: good guess
        if (mpAnim)
        {
            if (isAutoReverse())
                (*mpAnim)( getPresentationValue( maStartValue ) );
            else
                (*mpAnim)( getPresentationValue( maEndValue ) );
        }
    }

private:
    ExpressionNodeSharedPtr                 mpFormula;

    ValueType                               maStartValue;
    ValueType                               maEndValue;

    mutable ValueType                               maPreviousValue;
    mutable ValueType                               maStartInterpolationValue;
    mutable sal_uInt32                              mnIteration;

    ::boost::shared_ptr< AnimationType >    mpAnim;
    Interpolator< ValueType >               maInterpolator;
    bool                                    mbDynamicStartValue;
    bool                                    mbCumulative;
};


/** Generate Activity corresponding to given FromToBy values

    @tpl BaseType
    BaseType to use for deriving the Activity from

    @tpl AnimationType
    Subtype of the Animation object (e.g. NumberAnimation)
*/
template<class BaseType, typename AnimationType>
AnimationActivitySharedPtr createFromToByActivity(
    const uno::Any&                                          rFromAny,
    const uno::Any&                                          rToAny,
    const uno::Any&                                          rByAny,
    const ActivityParameters&                                rParms,
    const ::boost::shared_ptr< AnimationType >&              rAnim,
    const Interpolator< typename AnimationType::ValueType >& rInterpolator,
    bool                                                     bCumulative,
    const ShapeSharedPtr&                                    rShape,
    const ::basegfx::B2DVector&                              rSlideBounds )
{
    typedef typename AnimationType::ValueType           ValueType;
    typedef boost::optional<ValueType>                  OptionalValueType;

    OptionalValueType aFrom;
    OptionalValueType aTo;
    OptionalValueType aBy;

    ValueType aTmpValue;

    if( rFromAny.hasValue() )
    {
        ENSURE_OR_THROW(
            extractValue( aTmpValue, rFromAny, rShape, rSlideBounds ),
            "createFromToByActivity(): Could not extract from value" );
        aFrom.reset(aTmpValue);
    }
    if( rToAny.hasValue() )
    {
        ENSURE_OR_THROW(
            extractValue( aTmpValue, rToAny, rShape, rSlideBounds ),
            "createFromToByActivity(): Could not extract to value" );
        aTo.reset(aTmpValue);
    }
    if( rByAny.hasValue() )
    {
        ENSURE_OR_THROW(
            extractValue( aTmpValue, rByAny, rShape, rSlideBounds ),
            "createFromToByActivity(): Could not extract by value" );
        aBy.reset(aTmpValue);
    }

    return AnimationActivitySharedPtr(
        new FromToByActivity<BaseType, AnimationType>(
            aTo,
            aBy,
            rParms,
            rAnim,
            rInterpolator,
            bCumulative ) );
}

/* The following table shows which animator combines with
   which Activity type:

   NumberAnimator:  all
   PairAnimation:   all
   ColorAnimation:  all
   StringAnimation: DiscreteActivityBase
   BoolAnimation:   DiscreteActivityBase
*/

/** Values handler

    Provides the Activity specializations for value lists
    animations.

    This template makes heavy use of SFINAE, only one of
    the perform*() methods will compile for each of the
    base classes.

    Note that we omit the virtual keyword on the perform()
    overrides on purpose; those that actually do override
    baseclass virtual methods inherit the property, and
    the others won't increase our vtable. What's more,
    having all perform() method in the vtable actually
    creates POIs for them, which breaks the whole SFINAE
    concept (IOW, this template won't compile any longer).

    @tpl BaseType
    Base class to use for this activity. Only
    ContinuousKeyTimeActivityBase and DiscreteActivityBase
    are supported here. For values animation without key
    times, the client must emulate key times by providing
    a vector of equally spaced values between 0 and 1,
    with the same number of entries as the values vector.

    @tpl AnimationType
    Type of the Animation to call.
*/
template<class BaseType, typename AnimationType>
class ValuesActivity : public BaseType
{
public:
    typedef typename AnimationType::ValueType   ValueType;
    typedef std::vector<ValueType>              ValueVectorType;

private:
    // some compilers don't inline methods whose definition they haven't
    // seen before the call site...
    ValueType getPresentationValue( const ValueType& rVal ) const
    {
        return FormulaTraits<ValueType>::getPresentationValue(
            rVal, mpFormula );
    }

public:
    /** Create ValuesActivity.

        @param rValues
        Value vector to cycle animation through

        @param rParms
        Standard Activity parameter struct

        @param rAnim
        Shared ptr to AnimationType

        @param rInterpolator
        Interpolator object to be used for lerping between
        start and end value (need to be passed, since it
        might contain state, e.g. interpolation direction
        for HSL color space).

        @param bCumulative
        Whether repeated animations should cumulate the
        value, or start afresh each time.
    */
    ValuesActivity(
        const ValueVectorType&                      rValues,
        const ActivityParameters&                   rParms,
        const boost::shared_ptr<AnimationType>&     rAnim,
        const Interpolator< ValueType >&            rInterpolator,
        bool                                        bCumulative )
        : BaseType( rParms ),
          maValues( rValues ),
          mpFormula( rParms.mpFormula ),
          mpAnim( rAnim ),
          maInterpolator( rInterpolator ),
          mbCumulative( bCumulative )
    {
        ENSURE_OR_THROW( mpAnim, "Invalid animation object" );
        ENSURE_OR_THROW( !rValues.empty(), "Empty value vector" );
    }

    virtual void endAnimation()
    {
        // end animation
        if (mpAnim)
            mpAnim->end();
    }

    /// perform override for ContinuousKeyTimeActivityBase base
    void perform( sal_uInt32    nIndex,
                  double        nFractionalIndex,
                  sal_uInt32    nRepeatCount ) const
    {
        if (this->isDisposed() || !mpAnim)
            return;
        ENSURE_OR_THROW( nIndex+1 < maValues.size(),
                          "ValuesActivity::perform(): index out of range" );

        // interpolate between nIndex and nIndex+1 values
        (*mpAnim)(
            getPresentationValue(
                accumulate<ValueType>( maValues.back(),
                            mbCumulative ? nRepeatCount : 0,
                            maInterpolator( maValues[ nIndex ],
                                            maValues[ nIndex+1 ],
                                            nFractionalIndex ) ) ) );
    }

    using BaseType::perform;

    /// perform override for DiscreteActivityBase base
    void perform( sal_uInt32 nFrame, sal_uInt32 nRepeatCount ) const
    {
        if (this->isDisposed() || !mpAnim)
            return;
        ENSURE_OR_THROW( nFrame < maValues.size(),
                          "ValuesActivity::perform(): index out of range" );

        // this is discrete, thus no lerp here.
        (*mpAnim)(
            getPresentationValue(
                accumulate<ValueType>( maValues.back(),
                            mbCumulative ? nRepeatCount : 0,
                            maValues[ nFrame ] ) ) );
    }

    virtual void performEnd()
    {
        // xxx todo: good guess
        if (mpAnim)
            (*mpAnim)( getPresentationValue( maValues.back() ) );
    }

private:
    ValueVectorType                         maValues;

    ExpressionNodeSharedPtr                 mpFormula;

    boost::shared_ptr<AnimationType>        mpAnim;
    Interpolator< ValueType >               maInterpolator;
    bool                                    mbCumulative;
};

/** Generate Activity corresponding to given Value vector

    @tpl BaseType
    BaseType to use for deriving the Activity from

    @tpl AnimationType
    Subtype of the Animation object (e.g. NumberAnimation)
*/
template<class BaseType, typename AnimationType>
AnimationActivitySharedPtr createValueListActivity(
    const uno::Sequence<uno::Any>&                            rValues,
    const ActivityParameters&                                 rParms,
    const boost::shared_ptr<AnimationType>&                   rAnim,
    const Interpolator<typename AnimationType::ValueType>&    rInterpolator,
    bool                                                      bCumulative,
    const ShapeSharedPtr&                                     rShape,
    const ::basegfx::B2DVector&                               rSlideBounds )
{
    typedef typename AnimationType::ValueType   ValueType;
    typedef std::vector<ValueType>              ValueVectorType;

    ValueVectorType aValueVector;
    aValueVector.reserve( rValues.getLength() );

    for( ::std::size_t i=0, nLen=rValues.getLength(); i<nLen; ++i )
    {
        ValueType aValue;
        ENSURE_OR_THROW(
            extractValue( aValue, rValues[i], rShape, rSlideBounds ),
            "createValueListActivity(): Could not extract values" );
        aValueVector.push_back( aValue );
    }

    return AnimationActivitySharedPtr(
        new ValuesActivity<BaseType, AnimationType>(
            aValueVector,
            rParms,
            rAnim,
            rInterpolator,
            bCumulative ) );
}

/** Generate Activity for given XAnimate, corresponding to given Value vector

    @tpl AnimationType
    Subtype of the Animation object (e.g. NumberAnimation)

    @param rParms
    Common activity parameters

    @param xNode
    XAnimate node, to retrieve animation values from

    @param rAnim
    Actual animation to operate with (gets called with the
    time-dependent values)

    @param rInterpolator
    Interpolator object to be used for lerping between
    start and end values (need to be passed, since it
    might contain state, e.g. interpolation direction
    for HSL color space).
*/
template<typename AnimationType>
AnimationActivitySharedPtr createActivity(
    const ActivitiesFactory::CommonParameters&               rParms,
    const uno::Reference< animations::XAnimate >&            xNode,
    const ::boost::shared_ptr< AnimationType >&              rAnim,
    const Interpolator< typename AnimationType::ValueType >& rInterpolator
    = Interpolator< typename AnimationType::ValueType >() )
{
    // setup common parameters
    // =======================

    ActivityParameters aActivityParms( rParms.mpEndEvent,
                                       rParms.mrEventQueue,
                                       rParms.mrActivitiesQueue,
                                       rParms.mnMinDuration,
                                       rParms.maRepeats,
                                       rParms.mnAcceleration,
                                       rParms.mnDeceleration,
                                       rParms.mnMinNumberOfFrames,
                                       rParms.mbAutoReverse );

    // is a formula given?
    const OUString& rFormulaString( xNode->getFormula() );
    if( !rFormulaString.isEmpty() )
    {
        // yep, parse and pass to ActivityParameters
        try
        {
            aActivityParms.mpFormula =
                SmilFunctionParser::parseSmilFunction(
                    rFormulaString,
                    calcRelativeShapeBounds(
                        rParms.maSlideBounds,
                        rParms.mpShape->getBounds() ) );
        }
        catch( ParseError& )
        {
            // parse error, thus no formula
            OSL_FAIL( "createActivity(): Error parsing formula string" );
        }
    }

    // are key times given?
    const uno::Sequence< double >& aKeyTimes( xNode->getKeyTimes() );
    if( aKeyTimes.hasElements() )
    {
        // yes, convert them from Sequence< double >
        aActivityParms.maDiscreteTimes.resize( aKeyTimes.getLength() );
        comphelper::sequenceToArray(
            &aActivityParms.maDiscreteTimes[0],
            aKeyTimes ); // saves us some temporary vectors
    }

    // values sequence given?
    const sal_Int32 nValueLen( xNode->getValues().getLength() );
    if( nValueLen )
    {
        // Value list activity
        // ===================

        // fake keytimes, if necessary
        if( !aKeyTimes.hasElements() )
        {
            // create a dummy vector of key times,
            // with aValues.getLength equally spaced entries.
            for( sal_Int32 i=0; i<nValueLen; ++i )
                aActivityParms.maDiscreteTimes.push_back( double(i)/nValueLen );
        }

        // determine type of animation needed here:
        // Value list activities are possible with
        // ContinuousKeyTimeActivityBase and DiscreteActivityBase
        // specializations
        const sal_Int16 nCalcMode( xNode->getCalcMode() );

        switch( nCalcMode )
        {
            case animations::AnimationCalcMode::DISCRETE:
            {
                // since DiscreteActivityBase suspends itself
                // between the frames, create a WakeupEvent for it.
                aActivityParms.mpWakeupEvent.reset(
                    new WakeupEvent(
                        rParms.mrEventQueue.getTimer(),
                        rParms.mrActivitiesQueue ) );

                AnimationActivitySharedPtr pActivity(
                    createValueListActivity< DiscreteActivityBase >(
                        xNode->getValues(),
                        aActivityParms,
                        rAnim,
                        rInterpolator,
                        xNode->getAccumulate(),
                        rParms.mpShape,
                        rParms.maSlideBounds ) );

                // WakeupEvent and DiscreteActivityBase need circular
                // references to the corresponding other object.
                aActivityParms.mpWakeupEvent->setActivity( pActivity );

                return pActivity;
            }

            default:
                OSL_FAIL( "createActivity(): unexpected case" );
                SAL_FALLTHROUGH;
            case animations::AnimationCalcMode::PACED:
            case animations::AnimationCalcMode::SPLINE:
            case animations::AnimationCalcMode::LINEAR:
                return createValueListActivity< ContinuousKeyTimeActivityBase >(
                    xNode->getValues(),
                    aActivityParms,
                    rAnim,
                    rInterpolator,
                    xNode->getAccumulate(),
                    rParms.mpShape,
                    rParms.maSlideBounds );
        }
    }
    else
    {
        // FromToBy activity
        // =================

        // determine type of animation needed here:
        // FromToBy activities are possible with
        // ContinuousActivityBase and DiscreteActivityBase
        // specializations
        const sal_Int16 nCalcMode( xNode->getCalcMode() );

        switch( nCalcMode )
        {
            case animations::AnimationCalcMode::DISCRETE:
            {
                // fake keytimes, if necessary
                if( !aKeyTimes.hasElements() )
                {
                    // create a dummy vector of 2 key times
                    const ::std::size_t nLen( 2 );
                    for( ::std::size_t i=0; i<nLen; ++i )
                        aActivityParms.maDiscreteTimes.push_back( double(i)/nLen );
                }

                // since DiscreteActivityBase suspends itself
                // between the frames, create a WakeupEvent for it.
                aActivityParms.mpWakeupEvent.reset(
                    new WakeupEvent(
                        rParms.mrEventQueue.getTimer(),
                        rParms.mrActivitiesQueue ) );

                AnimationActivitySharedPtr pActivity(
                    createFromToByActivity< DiscreteActivityBase >(
                        xNode->getFrom(),
                        xNode->getTo(),
                        xNode->getBy(),
                        aActivityParms,
                        rAnim,
                        rInterpolator,
                        xNode->getAccumulate(),
                        rParms.mpShape,
                        rParms.maSlideBounds ) );

                // WakeupEvent and DiscreteActivityBase need circular
                // references to the corresponding other object.
                aActivityParms.mpWakeupEvent->setActivity( pActivity );

                return pActivity;
            }

            default:
                OSL_FAIL( "createActivity(): unexpected case" );
                SAL_FALLTHROUGH;
            case animations::AnimationCalcMode::PACED:
            case animations::AnimationCalcMode::SPLINE:
            case animations::AnimationCalcMode::LINEAR:
                return createFromToByActivity< ContinuousActivityBase >(
                    xNode->getFrom(),
                    xNode->getTo(),
                    xNode->getBy(),
                    aActivityParms,
                    rAnim,
                    rInterpolator,
                    xNode->getAccumulate(),
                    rParms.mpShape,
                    rParms.maSlideBounds );
        }
    }
}

/** Simple activity for ActivitiesFactory::createSimpleActivity

    @tpl Direction
    Determines direction of value generator. A 1 yields a
    forward direction, starting with 0.0 and ending with
    1.0. A 0 yields a backward direction, starting with
    1.0 and ending with 0.0
*/
template<int Direction>
class SimpleActivity : public ContinuousActivityBase
{
public:
    /** Create SimpleActivity.

        @param rParms
        Standard Activity parameter struct
    */
    SimpleActivity( const ActivityParameters&       rParms,
                    const NumberAnimationSharedPtr& rAnim ) :
        ContinuousActivityBase( rParms ),
        mpAnim( rAnim )
    {
        ENSURE_OR_THROW( mpAnim, "Invalid animation object" );
    }

    virtual void startAnimation() override
    {
        if (this->isDisposed() || !mpAnim)
            return;
        ContinuousActivityBase::startAnimation();

        // start animation
        mpAnim->start( getShape(),
                       getShapeAttributeLayer() );
    }

    virtual void endAnimation() override
    {
        // end animation
        if (mpAnim)
            mpAnim->end();
    }

    using SimpleContinuousActivityBase::perform;

    /// perform override for ContinuousActivityBase
    virtual void perform( double nModifiedTime, sal_uInt32 ) const override
    {
        if (this->isDisposed() || !mpAnim)
            return;
        // no cumulation, simple [0,1] range
        (*mpAnim)( 1.0 - Direction + nModifiedTime*(2.0*Direction - 1.0) );
    }

    virtual void performEnd() override
    {
        // xxx todo: review
        if (mpAnim)
            (*mpAnim)( 1.0*Direction );
    }

    /// Disposable:
    virtual void dispose() override
    {
        mpAnim.reset();
        ContinuousActivityBase::dispose();
    }

private:
    NumberAnimationSharedPtr    mpAnim;
};

} // anon namespace


AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                        rParms,
    const NumberAnimationSharedPtr&                rAnim,
    const uno::Reference< animations::XAnimate >&  xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms, xNode, rAnim );
}

AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                        rParms,
    const EnumAnimationSharedPtr&                  rAnim,
    const uno::Reference< animations::XAnimate >&  xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms, xNode, rAnim );
}

AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                        rParms,
    const ColorAnimationSharedPtr&                 rAnim,
    const uno::Reference< animations::XAnimate >&  xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms, xNode, rAnim );
}

AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                            rParms,
    const HSLColorAnimationSharedPtr&                  rAnim,
    const uno::Reference< animations::XAnimateColor >& xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms,
                           uno::Reference< animations::XAnimate >(
                               xNode, uno::UNO_QUERY_THROW ),
                           rAnim,
                           // Direction==true means clockwise in SMIL API
                           Interpolator< HSLColor >( !xNode->getDirection() ) );
}

AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                        rParms,
    const PairAnimationSharedPtr&                  rAnim,
    const uno::Reference< animations::XAnimate >&  xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms, xNode, rAnim );
}

AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                        rParms,
    const StringAnimationSharedPtr&                rAnim,
    const uno::Reference< animations::XAnimate >&  xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms, xNode, rAnim );
}

AnimationActivitySharedPtr ActivitiesFactory::createAnimateActivity(
    const CommonParameters&                        rParms,
    const BoolAnimationSharedPtr&                  rAnim,
    const uno::Reference< animations::XAnimate >&  xNode )
{
    // forward to appropriate template instantiation
    return createActivity( rParms, xNode, rAnim );
}

AnimationActivitySharedPtr ActivitiesFactory::createSimpleActivity(
    const CommonParameters&         rParms,
    const NumberAnimationSharedPtr& rAnim,
    bool                            bDirectionForward )
{
    ActivityParameters aActivityParms( rParms.mpEndEvent,
                                       rParms.mrEventQueue,
                                       rParms.mrActivitiesQueue,
                                       rParms.mnMinDuration,
                                       rParms.maRepeats,
                                       rParms.mnAcceleration,
                                       rParms.mnDeceleration,
                                       rParms.mnMinNumberOfFrames,
                                       rParms.mbAutoReverse );

    if( bDirectionForward )
        return AnimationActivitySharedPtr(
            new SimpleActivity<1>( aActivityParms, rAnim ) );
    else
        return AnimationActivitySharedPtr(
            new SimpleActivity<0>( aActivityParms, rAnim ) );
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
