/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimatePhysics.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XParallelTimeContainer.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/compbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <cppuhelper/implbase.hxx>

#include <sal/log.hxx>
#include <unotools/weakref.hxx>
#include <array>
#include <mutex>
#include <vector>
#include <algorithm>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::beans { struct NamedValue; }

using ::comphelper::OInterfaceContainerHelper4;
using ::comphelper::OInterfaceIteratorHelper4;
using ::cpo::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XWeak;
using ::com::sun::star::uno::Type;
using ::cpo::uno::Any;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::lang::XTypeProvider;
using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::ElementExistException;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::util::XChangesNotifier;
using ::com::sun::star::util::XChangesListener;
using ::com::sun::star::util::ElementChange;
using ::com::sun::star::util::ChangesEvent;

using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::animations::AnimationNodeType;

namespace animcore
{

namespace {
using AnimationNodeBase = comphelper::WeakImplHelper<
                             XAnimateMotion,
                             XAnimatePhysics,
                             XAnimateColor,
                             XTransitionFilter,
                             XAnimateSet,
                             XAnimateTransform,
                             XParallelTimeContainer,
                             XIterateContainer,
                             XServiceInfo,
                             XAudio,
                             XCommand,
                             XCloneable,
                             XChangesNotifier>;

class AnimationNode final:  public AnimationNodeBase
{
public:
    explicit AnimationNode(sal_Int16 nNodeType);
    explicit AnimationNode(const AnimationNode& rNode);

    // XInterface
    virtual Any queryInterface( const Type& aType ) override;

    // XTypeProvider
    virtual Sequence< Type > getTypes() override;

    // XServiceInfo
    OUString getImplementationName() override;
    Sequence< OUString > getSupportedServiceNames() override;
    bool supportsService(const OUString& ServiceName) override;

    // XChild
    virtual Reference< XInterface > getParent() override;
    virtual void setParent( const Reference< XInterface >& Parent ) override;

    // XCloneable
    virtual Reference< XCloneable > createClone() override;

    // XAnimationNode
    virtual sal_Int16 getType() override;
    virtual Any getBegin() override;
    virtual void setBegin( const Any& _begin ) override;
    virtual Any getDuration() override;
    virtual void setDuration( const Any& _duration ) override;
    virtual Any getEnd() override;
    virtual void setEnd( const Any& _end ) override;
    virtual Any getEndSync() override;
    virtual void setEndSync( const Any& _endsync ) override;
    virtual Any getRepeatCount() override;
    virtual void setRepeatCount( const Any& _repeatcount ) override;
    virtual Any getRepeatDuration() override;
    virtual void setRepeatDuration( const Any& _repeatduration ) override;
    virtual sal_Int16 getFill() override;
    virtual void setFill( sal_Int16 _fill ) override;
    virtual sal_Int16 getFillDefault() override;
    virtual void setFillDefault( sal_Int16 _filldefault ) override;
    virtual sal_Int16 getRestart() override;
    virtual void setRestart( sal_Int16 _restart ) override;
    virtual sal_Int16 getRestartDefault() override;
    virtual void setRestartDefault( sal_Int16 _restartdefault ) override;
    virtual double getAcceleration() override;
    virtual void setAcceleration( double _acceleration ) override;
    virtual double getDecelerate() override;
    virtual void setDecelerate( double _decelerate ) override;
    virtual bool getAutoReverse() override;
    virtual void setAutoReverse( bool _autoreverse ) override;
    virtual Sequence< NamedValue > getUserData() override;
    virtual void setUserData( const Sequence< NamedValue >& _userdata ) override;

    // XAnimate
    virtual Any getTarget() override;
    virtual void setTarget( const Any& _target ) override;
    virtual sal_Int16 getSubItem() override;
    virtual void setSubItem( sal_Int16 _subitem ) override;
    virtual OUString getAttributeName() override;
    virtual void setAttributeName( const OUString& _attribute ) override;
    virtual Sequence< Any > getValues() override;
    virtual void setValues( const Sequence< Any >& _values ) override;
    virtual Sequence< double > getKeyTimes() override;
    virtual void setKeyTimes( const Sequence< double >& _keytimes ) override;
    virtual sal_Int16 getValueType() override;
    virtual void setValueType( sal_Int16 _valuetype ) override;
    virtual sal_Int16 getCalcMode() override;
    virtual void setCalcMode( sal_Int16 _calcmode ) override;
    virtual bool getAccumulate() override;
    virtual void setAccumulate( bool _accumulate ) override;
    virtual sal_Int16 getAdditive() override;
    virtual void setAdditive( sal_Int16 _additive ) override;
    virtual Any getFrom() override;
    virtual void setFrom( const Any& _from ) override;
    virtual Any getTo() override;
    virtual void setTo( const Any& _to ) override;
    virtual Any getBy() override;
    virtual void setBy( const Any& _by ) override;
    virtual Sequence< TimeFilterPair > getTimeFilter() override;
    virtual void setTimeFilter( const Sequence< TimeFilterPair >& _timefilter ) override;
    virtual OUString getFormula() override;
    virtual void setFormula( const OUString& _formula ) override;

    // XAnimateColor
    virtual sal_Int16 getColorInterpolation() override;
    virtual void setColorInterpolation( sal_Int16 _colorspace ) override;
    virtual bool getDirection() override;
    virtual void setDirection( bool _direction ) override;

    // XAnimateMotion
    virtual Any getPath() override;
    virtual void setPath( const Any& _path ) override;
    virtual Any getOrigin() override;
    virtual void setOrigin( const Any& _origin ) override;

    // XAnimatePhysics
    virtual Any getStartVelocityX() override;
    virtual void setStartVelocityX( const Any& _startvelocityx ) override;
    virtual Any getStartVelocityY() override;
    virtual void setStartVelocityY( const Any& _startvelocityy ) override;
    virtual Any getDensity() override;
    virtual void setDensity( const Any& _density ) override;
    virtual Any getBounciness() override;
    virtual void setBounciness( const Any& _bounciness ) override;

    // XAnimateTransform
    virtual sal_Int16 getTransformType() override;
    virtual void setTransformType( sal_Int16 _transformtype ) override;

    // XTransitionFilter
    virtual sal_Int16 getTransition() override;
    virtual void setTransition( sal_Int16 _transition ) override;
    virtual sal_Int16 getSubtype() override;
    virtual void setSubtype( sal_Int16 _subtype ) override;
    virtual bool getMode() override;
    virtual void setMode( bool _mode ) override;
    virtual sal_Int32 getFadeColor() override;
    virtual void setFadeColor( sal_Int32 _fadecolor ) override;

    // XAudio
    virtual Any getSource() override;
    virtual void setSource( const Any& _source ) override;
    virtual double getVolume() override;
    virtual void setVolume( double _volume ) override;
    bool getHideDuringShow() override;
    void setHideDuringShow(bool bHideDuringShow) override;
    bool getNarration() override;
    void setNarration(bool bNarration) override;


    // XCommand - the following two shadowed by animate, unfortunately
//    virtual Any getTarget() throw (RuntimeException);
//    virtual void setTarget( const Any& _target ) throw (RuntimeException);
    virtual sal_Int16 getCommand() override;
    virtual void setCommand( sal_Int16 _command ) override;
    virtual Any getParameter() override;
    virtual void setParameter( const Any& _parameter ) override;

    // XElementAccess
    virtual Type getElementType() override;
    virtual bool hasElements() override;

    // XEnumerationAccess
    virtual Reference< XEnumeration > createEnumeration() override;

    // XTimeContainer
    virtual Reference< XAnimationNode > insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) override;
    virtual Reference< XAnimationNode > insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) override;
    virtual Reference< XAnimationNode > replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild ) override;
    virtual Reference< XAnimationNode > removeChild( const Reference< XAnimationNode >& oldChild ) override;
    virtual Reference< XAnimationNode > appendChild( const Reference< XAnimationNode >& newChild ) override;

    // XIterateContainer
    virtual sal_Int16 getIterateType() override;
    virtual void setIterateType( sal_Int16 _iteratetype ) override;
    virtual double getIterateInterval() override;
    virtual void setIterateInterval( double _iterateinterval ) override;

    // XChangesNotifier
    virtual void addChangesListener( const Reference< XChangesListener >& aListener ) override;
    virtual void removeChangesListener( const Reference< XChangesListener >& aListener ) override;

    void fireChangeListener(std::unique_lock<std::mutex>&);

private:
    OInterfaceContainerHelper4<XChangesListener>   maChangeListener;

    static void initTypeProvider( sal_Int16 nNodeType ) noexcept;

    const sal_Int16 mnNodeType;

    // for XTypeProvider
    static std::array<Sequence< Type >*, 13> mpTypes;

    // attributes for the XAnimationNode interface implementation
    Any maBegin, maDuration, maEnd, maEndSync, maRepeatCount, maRepeatDuration;
    sal_Int16 mnFill, mnFillDefault, mnRestart, mnRestartDefault;
    double mfAcceleration, mfDecelerate;
    bool mbAutoReverse;
    Sequence< NamedValue > maUserData;

    // parent interface for XChild interface implementation
    unotools::WeakReference<AnimationNode> mxParent;

    // attributes for XAnimate
    Any maTarget;
    OUString maAttributeName, maFormula;
    Sequence< Any > maValues;
    Sequence< double > maKeyTimes;
    sal_Int16 mnValueType, mnSubItem;
    sal_Int16 mnCalcMode, mnAdditive;
    bool mbAccumulate;
    Any maFrom, maTo, maBy;
    Sequence< TimeFilterPair > maTimeFilter;

    // attributes for XAnimateColor
    sal_Int16 mnColorSpace;
    bool mbDirection;

    // attributes for XAnimateMotion
    Any maPath, maOrigin;

    // attributes for XAnimatePhysics
    Any maStartVelocityX, maStartVelocityY, maDensity, maBounciness;

    // attributes for XAnimateTransform
    sal_Int16 mnTransformType;

    // attributes for XTransitionFilter
    sal_Int16 mnTransition;
    sal_Int16 mnSubtype;
    bool mbMode;
    sal_Int32 mnFadeColor;

    // XAudio
    double mfVolume;
    bool mbHideDuringShow;
    bool mbNarration;

    // XCommand
    sal_Int16 mnCommand;
    Any maParameter;

    // XIterateContainer
    sal_Int16 mnIterateType;
    double  mfIterateInterval;

    /** sorted list of child nodes for XTimeContainer*/
    std::vector< Reference< XAnimationNode > > maChildren;
};


class TimeContainerEnumeration : public ::cppu::WeakImplHelper< XEnumeration >
{
public:
    explicit TimeContainerEnumeration( std::vector< Reference< XAnimationNode > >&& rChildren );

    // Methods
    virtual bool hasMoreElements() override;
    virtual Any nextElement(  ) override;

private:
    std::mutex m_aMutex;

    /** sorted list of child nodes */
    std::vector< Reference< XAnimationNode > > maChildren;

    /** current iteration position */
    std::vector< Reference< XAnimationNode > >::iterator   maIter;
};

}

TimeContainerEnumeration::TimeContainerEnumeration( std::vector< Reference< XAnimationNode > >&& rChildren )
: maChildren( std::move(rChildren) )
{
    maIter = maChildren.begin();
}

// Methods
bool TimeContainerEnumeration::hasMoreElements()
{
    std::unique_lock aGuard( m_aMutex );

    return maIter != maChildren.end();
}

Any TimeContainerEnumeration::nextElement()
{
    std::unique_lock aGuard( m_aMutex );

    if( maIter == maChildren.end() )
        throw NoSuchElementException();

    return Any( *maIter++ );
}


std::array<Sequence< Type >*, 13> AnimationNode::mpTypes = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

AnimationNode::AnimationNode( sal_Int16 nNodeType )
:   AnimationNodeBase(),
    mnNodeType( nNodeType ),
    mnFill( AnimationFill::DEFAULT ),
    mnFillDefault( AnimationFill::INHERIT ),
    mnRestart( AnimationRestart:: DEFAULT ),
    mnRestartDefault( AnimationRestart:: INHERIT ),
    mfAcceleration( 0.0 ),
    mfDecelerate( 0.0 ),
    mbAutoReverse( false ),
    mnValueType( 0 ),
    mnSubItem( 0 ),
    mnCalcMode( (nNodeType == AnimationNodeType::ANIMATEMOTION) ? AnimationCalcMode::PACED : AnimationCalcMode::LINEAR),
    mnAdditive(AnimationAdditiveMode::REPLACE),
    mbAccumulate(false),
    mnColorSpace( AnimationColorSpace::RGB ),
    mbDirection( true ),
    mnTransformType( AnimationTransformType::TRANSLATE ),
    mnTransition(TransitionType::BARWIPE),
    mnSubtype(TransitionSubType::DEFAULT),
    mbMode(true),
    mnFadeColor(0),
    mfVolume(1.0),
    mbHideDuringShow(false),
    mbNarration(false),
    mnCommand(0),
    mnIterateType( css::presentation::ShapeAnimationSubType::AS_WHOLE ),
    mfIterateInterval(0.0)
{
    assert(nNodeType >= 0 && o3tl::make_unsigned(nNodeType) < mpTypes.size());
}

AnimationNode::AnimationNode( const AnimationNode& rNode )
:   AnimationNodeBase(),
    mnNodeType( rNode.mnNodeType ),

    // attributes for the XAnimationNode interface implementation
    maBegin( rNode.maBegin ),
    maDuration( rNode.maDuration ),
    maEnd( rNode.maEnd ),
    maEndSync( rNode.maEndSync ),
    maRepeatCount( rNode.maRepeatCount ),
    maRepeatDuration( rNode.maRepeatDuration ),
    mnFill( rNode.mnFill ),
    mnFillDefault( rNode.mnFillDefault ),
    mnRestart( rNode.mnRestart ),
    mnRestartDefault( rNode.mnRestartDefault ),
    mfAcceleration( rNode.mfAcceleration ),
    mfDecelerate( rNode.mfDecelerate ),
    mbAutoReverse( rNode.mbAutoReverse ),
    maUserData( rNode.maUserData ),

    // attributes for XAnimate
    maTarget( rNode.maTarget ),
    maAttributeName( rNode.maAttributeName ),
    maFormula( rNode.maFormula ),
    maValues( rNode.maValues ),
    maKeyTimes( rNode.maKeyTimes ),
    mnValueType( rNode.mnValueType ),
    mnSubItem( rNode.mnSubItem ),
    mnCalcMode( rNode.mnCalcMode ),
    mnAdditive( rNode.mnAdditive ),
    mbAccumulate( rNode.mbAccumulate ),
    maFrom( rNode.maFrom ),
    maTo( rNode.maTo ),
    maBy( rNode.maBy ),
    maTimeFilter( rNode.maTimeFilter ),

    // attributes for XAnimateColor
    mnColorSpace( rNode.mnColorSpace ),
    mbDirection( rNode.mbDirection ),

    // attributes for XAnimateMotion
    maPath( rNode.maPath ),
    maOrigin( rNode.maOrigin ),

    // attributes for XAnimatePhysics
    maStartVelocityX( rNode.maStartVelocityX ),
    maStartVelocityY( rNode.maStartVelocityY ),
    maDensity( rNode.maDensity ),
    maBounciness( rNode.maBounciness ),

    // attributes for XAnimateTransform
    mnTransformType( rNode.mnTransformType ),

    // attributes for XTransitionFilter
    mnTransition( rNode.mnTransition ),
    mnSubtype( rNode.mnSubtype ),
    mbMode( rNode.mbMode ),
    mnFadeColor( rNode.mnFadeColor ),

    // XAudio
    mfVolume( rNode.mfVolume ),
    mbHideDuringShow(rNode.mbHideDuringShow),
    mbNarration(rNode.mbNarration),

    // XCommand
    mnCommand( rNode.mnCommand ),
    maParameter( rNode.maParameter ),

    // XIterateContainer
    mnIterateType( rNode.mnIterateType ),
    mfIterateInterval( rNode.mfIterateInterval )
{
}

static Sequence<OUString> getSupportedServiceNames_PAR()
{
    return { u"com.sun.star.animations.ParallelTimeContainer"_ustr };
}

static OUString getImplementationName_PAR()
{
    return u"animcore::ParallelTimeContainer"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_SEQ()
{
    return { u"com.sun.star.animations.SequenceTimeContainer"_ustr };
}

static OUString getImplementationName_SEQ()
{
    return u"animcore::SequenceTimeContainer"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_ITERATE()
{
    return { u"com.sun.star.animations.IterateContainer"_ustr };
}

static OUString getImplementationName_ITERATE()
{
    return u"animcore::IterateContainer"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_ANIMATE()
{
    return { u"com.sun.star.animations.Animate"_ustr };
}

static OUString getImplementationName_ANIMATE()
{
        return u"animcore::Animate"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_SET()
{
    return { u"com.sun.star.animations.AnimateSet"_ustr };
}

static OUString getImplementationName_SET()
{
    return u"animcore::AnimateSet"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_ANIMATECOLOR()
{
    return { u"com.sun.star.animations.AnimateColor"_ustr };
}

static OUString getImplementationName_ANIMATECOLOR()
{
    return u"animcore::AnimateColor"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_ANIMATEMOTION()
{
    return { u"com.sun.star.animations.AnimateMotion"_ustr };
}

static OUString getImplementationName_ANIMATEMOTION()
{
    return u"animcore::AnimateMotion"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_ANIMATEPHYSICS()
{
    return { u"com.sun.star.animations.AnimatePhysics"_ustr };
}

static OUString getImplementationName_ANIMATEPHYSICS()
{
    return u"animcore::AnimatePhysics"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_ANIMATETRANSFORM()
{
    return { u"com.sun.star.animations.AnimateTransform"_ustr };
}

static OUString getImplementationName_ANIMATETRANSFORM()
{
    return u"animcore::AnimateTransform"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_TRANSITIONFILTER()
{
    return { u"com.sun.star.animations.TransitionFilter"_ustr };
}

static OUString getImplementationName_TRANSITIONFILTER()
{
        return u"animcore::TransitionFilter"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_AUDIO()
{
    return { u"com.sun.star.animations.Audio"_ustr };
}

static OUString getImplementationName_AUDIO()
{
        return u"animcore::Audio"_ustr;
}

static Sequence<OUString> getSupportedServiceNames_COMMAND()
{
    return { u"com.sun.star.animations.Command"_ustr };
}

static OUString getImplementationName_COMMAND()
{
    return u"animcore::Command"_ustr;
}

// XInterface
Any AnimationNode::queryInterface( const Type& aType )
{
    Any aRet( ::cppu::queryInterface(
        aType,
        static_cast< XServiceInfo * >( this ),
        static_cast< XTypeProvider * >( this ),
        static_cast< XChild * >( static_cast< XIterateContainer * >(this) ),
        static_cast< XCloneable* >( this ),
        static_cast< XAnimationNode* >( static_cast< XIterateContainer * >(this) ),
        static_cast< XInterface* >( getXWeak() ),
        static_cast< XWeak* >( this ),
        static_cast< XChangesNotifier* >( this ) ) );

    if(!aRet.hasValue())
    {
        switch( mnNodeType )
        {
        case AnimationNodeType::PAR:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XParallelTimeContainer * >( this ),
                static_cast< XTimeContainer * >( static_cast< XIterateContainer * >(this) ),
                static_cast< XEnumerationAccess * >( this ),
                static_cast< XElementAccess * >( this ) );
            break;
        case AnimationNodeType::SEQ:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XTimeContainer * >( static_cast< XIterateContainer * >(this) ),
                static_cast< XEnumerationAccess * >( this ),
                static_cast< XElementAccess * >( this ) );
            break;
        case AnimationNodeType::ITERATE:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XTimeContainer * >( static_cast< XIterateContainer * >(this) ),
                static_cast< XIterateContainer * >( this ),
                static_cast< XEnumerationAccess * >( this ),
                static_cast< XElementAccess * >( this ) );
            break;
        case AnimationNodeType::ANIMATE:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XAnimateMotion * >(this) ) );
            break;
        case AnimationNodeType::ANIMATEMOTION:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XAnimateMotion * >(this) ),
                static_cast< XAnimateMotion * >( this ) );
            break;
        case AnimationNodeType::ANIMATEPHYSICS:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XAnimatePhysics * >(this) ),
                static_cast< XAnimatePhysics * >( this ) );
            break;
        case AnimationNodeType::ANIMATECOLOR:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XAnimateColor * >(this) ),
                static_cast< XAnimateColor * >( this ) );
            break;
        case AnimationNodeType::SET:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XAnimateSet * >(this) ),
                static_cast< XAnimateSet * >( this ) );
            break;
        case AnimationNodeType::ANIMATETRANSFORM:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XAnimateTransform * >(this) ),
                static_cast< XAnimateTransform * >( this ) );
            break;
        case AnimationNodeType::AUDIO:
            aRet = ::cppu::queryInterface(aType, static_cast< XAudio * >(this) );
            break;
        case AnimationNodeType::COMMAND:
            aRet = ::cppu::queryInterface(
                aType, static_cast< XCommand * >(this) );
            break;
        case AnimationNodeType::TRANSITIONFILTER:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAnimate * >( static_cast< XTransitionFilter * >(this) ),
                static_cast< XTransitionFilter * >( this ) );
            break;
        }
    }

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( aType );
}


void AnimationNode::initTypeProvider( sal_Int16 nNodeType ) noexcept
{
    static std::mutex aMutex;

    std::scoped_lock aGuard(aMutex);

    if( mpTypes[nNodeType] )
        return;

    static constexpr std::array<sal_Int32, mpTypes.size()> type_numbers =
    {
        6, // CUSTOM
        8, // PAR
        8, // SEQ
        8, // ITERATE
        7, // ANIMATE
        7, // SET
        7, // ANIMATEMOTION
        7, // ANIMATECOLOR
        7, // ANIMATETRANSFORM
        7, // TRANSITIONFILTER
        7, // AUDIO
        7, // COMMAND
        7, // ANIMATEPHYSICS
    };

    // collect types
    Sequence< Type > * types = new Sequence< Type >( type_numbers[nNodeType] );
    Type * pTypeAr = types->getArray();
    sal_Int32 nPos = 0;

    pTypeAr[nPos++] = cppu::UnoType<XWeak>::get();
    pTypeAr[nPos++] = cppu::UnoType<XChild>::get();
    pTypeAr[nPos++] = cppu::UnoType<XCloneable>::get();
    pTypeAr[nPos++] = cppu::UnoType<XTypeProvider>::get();
    pTypeAr[nPos++] = cppu::UnoType<XServiceInfo>::get();
    pTypeAr[nPos++] = cppu::UnoType<XChangesNotifier>::get();

    switch( nNodeType )
    {
    case AnimationNodeType::PAR:
    case AnimationNodeType::SEQ:
        pTypeAr[nPos++] = cppu::UnoType<XTimeContainer>::get();
        pTypeAr[nPos++] = cppu::UnoType<XEnumerationAccess>::get();
        break;
    case AnimationNodeType::ITERATE:
        pTypeAr[nPos++] = cppu::UnoType<XIterateContainer>::get();
        pTypeAr[nPos++] = cppu::UnoType<XEnumerationAccess>::get();
        break;
    case AnimationNodeType::ANIMATE:
        pTypeAr[nPos++] = cppu::UnoType<XAnimate>::get();
        break;
    case AnimationNodeType::ANIMATEMOTION:
        pTypeAr[nPos++] = cppu::UnoType<XAnimateMotion>::get();
        break;
    case AnimationNodeType::ANIMATEPHYSICS:
        pTypeAr[nPos++] = cppu::UnoType<XAnimatePhysics>::get();
        break;
    case AnimationNodeType::ANIMATECOLOR:
        pTypeAr[nPos++] = cppu::UnoType<XAnimateColor>::get();
        break;
    case AnimationNodeType::ANIMATETRANSFORM:
        pTypeAr[nPos++] = cppu::UnoType<XAnimateTransform>::get();
        break;
    case AnimationNodeType::SET:
        pTypeAr[nPos++] = cppu::UnoType<XAnimateSet>::get();
        break;
    case AnimationNodeType::TRANSITIONFILTER:
        pTypeAr[nPos++] = cppu::UnoType<XTransitionFilter>::get();
        break;
    case AnimationNodeType::AUDIO:
        pTypeAr[nPos++] = cppu::UnoType<XAudio>::get();
        break;
    case AnimationNodeType::COMMAND:
        pTypeAr[nPos++] = cppu::UnoType<XCommand>::get();
        break;
    }
    mpTypes[nNodeType] = types;
}


Sequence< Type > AnimationNode::getTypes()
{
    if (! mpTypes[mnNodeType])
        initTypeProvider(mnNodeType);
    return *mpTypes[mnNodeType];
}


// XServiceInfo
OUString AnimationNode::getImplementationName()
{
    switch( mnNodeType )
    {
    case AnimationNodeType::PAR:
        return getImplementationName_PAR();
    case AnimationNodeType::SEQ:
        return getImplementationName_SEQ();
    case AnimationNodeType::ITERATE:
        return getImplementationName_ITERATE();
    case AnimationNodeType::SET:
        return getImplementationName_SET();
    case AnimationNodeType::ANIMATECOLOR:
        return getImplementationName_ANIMATECOLOR();
    case AnimationNodeType::ANIMATEMOTION:
        return getImplementationName_ANIMATEMOTION();
    case AnimationNodeType::ANIMATEPHYSICS:
        return getImplementationName_ANIMATEPHYSICS();
    case AnimationNodeType::TRANSITIONFILTER:
        return getImplementationName_TRANSITIONFILTER();
    case AnimationNodeType::ANIMATETRANSFORM:
        return getImplementationName_ANIMATETRANSFORM();
    case AnimationNodeType::AUDIO:
        return getImplementationName_AUDIO();
    case AnimationNodeType::COMMAND:
        return getImplementationName_COMMAND();
    case AnimationNodeType::ANIMATE:
    default:
        return getImplementationName_ANIMATE();
    }
}

// XServiceInfo
bool AnimationNode::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > AnimationNode::getSupportedServiceNames()
{
    switch( mnNodeType )
    {
    case AnimationNodeType::PAR:
        return getSupportedServiceNames_PAR();
    case AnimationNodeType::SEQ:
        return getSupportedServiceNames_SEQ();
    case AnimationNodeType::ITERATE:
        return getSupportedServiceNames_ITERATE();
    case AnimationNodeType::SET:
        return getSupportedServiceNames_SET();
    case AnimationNodeType::ANIMATECOLOR:
        return getSupportedServiceNames_ANIMATECOLOR();
    case AnimationNodeType::ANIMATEMOTION:
        return getSupportedServiceNames_ANIMATEMOTION();
    case AnimationNodeType::ANIMATEPHYSICS:
        return getSupportedServiceNames_ANIMATEPHYSICS();
    case AnimationNodeType::TRANSITIONFILTER:
        return getSupportedServiceNames_TRANSITIONFILTER();
    case AnimationNodeType::ANIMATETRANSFORM:
        return getSupportedServiceNames_ANIMATETRANSFORM();
    case AnimationNodeType::AUDIO:
        return getSupportedServiceNames_AUDIO();
    case AnimationNodeType::COMMAND:
        return getSupportedServiceNames_COMMAND();
    case AnimationNodeType::ANIMATE:
    default:
        return getSupportedServiceNames_ANIMATE();
    }
}


// XAnimationNode
sal_Int16 AnimationNode::getType()
{
    return mnNodeType;
}


// XAnimationNode
Any AnimationNode::getBegin()
{
    std::unique_lock aGuard( m_aMutex );
    return maBegin;
}


// XAnimationNode
void AnimationNode::setBegin( const Any& _begin )
{
    std::unique_lock aGuard( m_aMutex );
    if( _begin != maBegin )
    {
        maBegin = _begin;
        fireChangeListener(aGuard);
    }
}


// XAnimationNode
Any AnimationNode::getDuration()
{
    std::unique_lock aGuard( m_aMutex );
    return maDuration;
}


// XAnimationNode
void AnimationNode::setDuration( const Any& _duration )
{
    std::unique_lock aGuard( m_aMutex );
    if( _duration != maDuration )
    {
        maDuration = _duration;
        fireChangeListener(aGuard);
    }
}


// XAnimationNode
Any AnimationNode::getEnd()
{
    std::unique_lock aGuard( m_aMutex );
    return maEnd;
}


// XAnimationNode
void AnimationNode::setEnd( const Any& _end )
{
    std::unique_lock aGuard( m_aMutex );
    if( _end != maEnd )
    {
        maEnd = _end;
        fireChangeListener(aGuard);
    }
}


// XAnimationNode
Any AnimationNode::getEndSync()
{
    std::unique_lock aGuard( m_aMutex );
    return maEndSync;
}


// XAnimationNode
void AnimationNode::setEndSync( const Any& _endsync )
{
    std::unique_lock l( m_aMutex );
    if( _endsync != maEndSync )
    {
        maEndSync = _endsync;
        fireChangeListener(l);
    }
}


// XAnimationNode
Any AnimationNode::getRepeatCount()
{
    std::unique_lock aGuard( m_aMutex );
    return maRepeatCount;
}


// XAnimationNode
void AnimationNode::setRepeatCount( const Any& _repeatcount )
{
    std::unique_lock l( m_aMutex );
    if( _repeatcount != maRepeatCount )
    {
        maRepeatCount = _repeatcount;
        fireChangeListener(l);
    }
}


// XAnimationNode
Any AnimationNode::getRepeatDuration()
{
    std::unique_lock aGuard( m_aMutex );
    return maRepeatDuration;
}


// XAnimationNode
void AnimationNode::setRepeatDuration( const Any& _repeatduration )
{
    std::unique_lock l( m_aMutex );
    if( _repeatduration != maRepeatDuration )
    {
        maRepeatDuration = _repeatduration;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 AnimationNode::getFill()
{
    std::unique_lock aGuard( m_aMutex );
    return mnFill;
}


// XAnimationNode
void AnimationNode::setFill( sal_Int16 _fill )
{
    std::unique_lock l( m_aMutex );
    if( _fill != mnFill )
    {
        mnFill = _fill;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 AnimationNode::getFillDefault()
{
    std::unique_lock aGuard( m_aMutex );
    return mnFillDefault;
}


// XAnimationNode
void AnimationNode::setFillDefault( sal_Int16 _filldefault )
{
    std::unique_lock l( m_aMutex );
    if( _filldefault != mnFillDefault )
    {
        mnFillDefault = _filldefault;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 AnimationNode::getRestart()
{
    std::unique_lock aGuard( m_aMutex );
    return mnRestart;
}


// XAnimationNode
void AnimationNode::setRestart( sal_Int16 _restart )
{
    std::unique_lock l( m_aMutex );
    if( _restart != mnRestart )
    {
        mnRestart = _restart;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 AnimationNode::getRestartDefault()
{
    std::unique_lock aGuard( m_aMutex );
    return mnRestartDefault;
}


// XAnimationNode
void AnimationNode::setRestartDefault( sal_Int16 _restartdefault )
{
    std::unique_lock l( m_aMutex );
    if( _restartdefault != mnRestartDefault )
    {
        mnRestartDefault = _restartdefault;
        fireChangeListener(l);
    }
}


// XAnimationNode
double AnimationNode::getAcceleration()
{
    std::unique_lock aGuard( m_aMutex );
    return mfAcceleration;
}


// XAnimationNode
void AnimationNode::setAcceleration( double _acceleration )
{
    std::unique_lock l( m_aMutex );
    if( _acceleration != mfAcceleration )
    {
        mfAcceleration = _acceleration;
        fireChangeListener(l);
    }
}


// XAnimationNode
double AnimationNode::getDecelerate()
{
    std::unique_lock aGuard( m_aMutex );
    return mfDecelerate;
}


// XAnimationNode
void AnimationNode::setDecelerate( double _decelerate )
{
    std::unique_lock l( m_aMutex );
    if( _decelerate != mfDecelerate )
    {
        mfDecelerate = _decelerate;
        fireChangeListener(l);
    }
}


// XAnimationNode
bool AnimationNode::getAutoReverse()
{
    std::unique_lock aGuard( m_aMutex );
    return mbAutoReverse;
}


// XAnimationNode
void AnimationNode::setAutoReverse( bool _autoreverse )
{
    std::unique_lock l( m_aMutex );
    if( bool(_autoreverse) != mbAutoReverse )
    {
        mbAutoReverse = _autoreverse;
        fireChangeListener(l);
    }
}


Sequence< NamedValue > AnimationNode::getUserData()
{
    std::unique_lock aGuard( m_aMutex );
    return maUserData;
}


void AnimationNode::setUserData( const Sequence< NamedValue >& _userdata )
{
    std::unique_lock l( m_aMutex );
    maUserData = _userdata;
    fireChangeListener(l);
}


// XChild
Reference< XInterface > AnimationNode::getParent()
{
    std::unique_lock aGuard( m_aMutex );
    return cppu::getXWeak(mxParent.get().get());
}


// XChild
void AnimationNode::setParent( const Reference< XInterface >& Parent )
{
    std::unique_lock l( m_aMutex );
    if (Parent.get() != cppu::getXWeak(mxParent.get().get()))
    {
        rtl::Reference<AnimationNode> xParent = dynamic_cast<AnimationNode*>(Parent.get());
        mxParent = xParent.get();
        assert(bool(xParent) == bool(Parent) && "only support AnimationNode subtypes");

        fireChangeListener(l);
    }
}


// XCloneable
Reference< XCloneable > AnimationNode::createClone()
{
    std::unique_lock aGuard( m_aMutex );

    rtl::Reference< AnimationNode > xNewNode;
    try
    {
        xNewNode = new AnimationNode( *this );

        if( !maChildren.empty() )
        {
            for (auto const& child : maChildren)
            {
                Reference< XCloneable > xCloneable(child, UNO_QUERY );
                if( xCloneable.is() ) try
                {
                    Reference< XAnimationNode > xNewChildNode( xCloneable->createClone(), UNO_QUERY );
                    if( xNewChildNode.is() )
                        xNewNode->appendChild( xNewChildNode );
                }
                catch(const Exception&)
                {
                    SAL_INFO("animations", "animations::AnimationNode::createClone(), exception caught!");
                }
            }
        }
    }
    catch(const Exception&)
    {
        SAL_INFO("animations", "animations::AnimationNode::createClone(), exception caught!");
    }

    return xNewNode;
}


// XAnimate
Any AnimationNode::getTarget()
{
    std::unique_lock aGuard( m_aMutex );
    return maTarget;
}


// XAnimate
void AnimationNode::setTarget( const Any& _target )
{
    std::unique_lock l( m_aMutex );
    if( _target != maTarget )
    {
        maTarget= _target;
        fireChangeListener(l);
    }
}


// XAnimate
OUString AnimationNode::getAttributeName()
{
    std::unique_lock aGuard( m_aMutex );
    return maAttributeName;
}


// XAnimate
void AnimationNode::setAttributeName( const OUString& _attribute )
{
    std::unique_lock l( m_aMutex );
    if( _attribute != maAttributeName )
    {
        maAttributeName = _attribute;
        fireChangeListener(l);
    }
}


// XAnimate
Sequence< Any > AnimationNode::getValues()
{
    std::unique_lock aGuard( m_aMutex );
    return maValues;
}


// XAnimate
void AnimationNode::setValues( const Sequence< Any >& _values )
{
    std::unique_lock l( m_aMutex );
    maValues = _values;
    fireChangeListener(l);
}


// XAnimate
sal_Int16 AnimationNode::getSubItem()
{
    std::unique_lock aGuard( m_aMutex );
    return mnSubItem;
}


// XAnimate
void AnimationNode::setSubItem( sal_Int16 _subitem )
{
    std::unique_lock l( m_aMutex );
    if( _subitem != mnSubItem )
    {
        mnSubItem = _subitem;
        fireChangeListener(l);
    }
}


// XAnimate
Sequence< double > AnimationNode::getKeyTimes()
{
    std::unique_lock aGuard( m_aMutex );
    return maKeyTimes;
}


// XAnimate
void AnimationNode::setKeyTimes( const Sequence< double >& _keytimes )
{
    std::unique_lock l( m_aMutex );
    maKeyTimes = _keytimes;
    fireChangeListener(l);
}


// XAnimate
sal_Int16 AnimationNode::getValueType()
{
    std::unique_lock aGuard( m_aMutex );
    return mnValueType;
}


void AnimationNode::setValueType( sal_Int16 _valuetype )
{
    std::unique_lock l( m_aMutex );
    if( _valuetype != mnValueType )
    {
        mnValueType = _valuetype;
        fireChangeListener(l);
    }
}


// XAnimate
sal_Int16 AnimationNode::getCalcMode()
{
    std::unique_lock aGuard( m_aMutex );
    return mnCalcMode;
}


// XAnimate
void AnimationNode::setCalcMode( sal_Int16 _calcmode )
{
    std::unique_lock l( m_aMutex );
    if( _calcmode != mnCalcMode )
    {
        mnCalcMode = _calcmode;
        fireChangeListener(l);
    }
}


// XAnimate
bool AnimationNode::getAccumulate()
{
    std::unique_lock aGuard( m_aMutex );
    return mbAccumulate;
}


// XAnimate
void AnimationNode::setAccumulate( bool _accumulate )
{
    std::unique_lock l( m_aMutex );
    if( bool(_accumulate) != mbAccumulate )
    {
        mbAccumulate = _accumulate;
        fireChangeListener(l);
    }
}


// XAnimate
sal_Int16 AnimationNode::getAdditive()
{
    std::unique_lock aGuard( m_aMutex );
    return mnAdditive;
}


// XAnimate
void AnimationNode::setAdditive( sal_Int16 _additive )
{
    std::unique_lock l( m_aMutex );
    if( _additive != mnAdditive )
    {
        mnAdditive = _additive;
        fireChangeListener(l);
    }
}


// XAnimate
Any AnimationNode::getFrom()
{
    std::unique_lock aGuard( m_aMutex );
    return maFrom;
}


// XAnimate
void AnimationNode::setFrom( const Any& _from )
{
    std::unique_lock l( m_aMutex );
    if( _from != maFrom )
    {
        maFrom = _from;
        fireChangeListener(l);
    }
}


// XAnimate
Any AnimationNode::getTo()
{
    std::unique_lock aGuard( m_aMutex );
    return maTo;
}


// XAnimate
void AnimationNode::setTo( const Any& _to )
{
    std::unique_lock l( m_aMutex );
    if( _to != maTo )
    {
        maTo = _to;
        fireChangeListener(l);
    }
}


// XAnimate
Any AnimationNode::getBy()
{
    std::unique_lock aGuard( m_aMutex );
    return maBy;
}


// XAnimate
void AnimationNode::setBy( const Any& _by )
{
    std::unique_lock l( m_aMutex );
    if( _by != maBy )
    {
        maBy = _by;
        fireChangeListener(l);
    }
}


// XAnimate
Sequence< TimeFilterPair > AnimationNode::getTimeFilter()
{
    std::unique_lock aGuard( m_aMutex );
    return maTimeFilter;
}


// XAnimate
void AnimationNode::setTimeFilter( const Sequence< TimeFilterPair >& _timefilter )
{
    std::unique_lock l( m_aMutex );
    maTimeFilter = _timefilter;
    fireChangeListener(l);
}


OUString AnimationNode::getFormula()
{
    std::unique_lock aGuard( m_aMutex );
    return maFormula;
}


void AnimationNode::setFormula( const OUString& _formula )
{
    std::unique_lock l( m_aMutex );
    if( _formula != maFormula )
    {
        maFormula = _formula;
        fireChangeListener(l);
    }
}


// XAnimateColor
sal_Int16 AnimationNode::getColorInterpolation()
{
    std::unique_lock aGuard( m_aMutex );
    return mnColorSpace;
}


// XAnimateColor
void AnimationNode::setColorInterpolation( sal_Int16 _colorspace )
{
    std::unique_lock l( m_aMutex );
    if( _colorspace != mnColorSpace )
    {
        mnColorSpace = _colorspace;
        fireChangeListener(l);
    }
}


// XAnimateColor
bool AnimationNode::getDirection()
{
    std::unique_lock aGuard( m_aMutex );
    return mbDirection;
}


// XAnimateColor
void AnimationNode::setDirection( bool _direction )
{
    std::unique_lock l( m_aMutex );
    if( bool(_direction) != mbDirection )
    {
        mbDirection = _direction;
        fireChangeListener(l);
    }
}


// XAnimateMotion
Any AnimationNode::getPath()
{
    std::unique_lock aGuard( m_aMutex );
    return maPath;
}


// XAnimateMotion
void AnimationNode::setPath( const Any& _path )
{
    std::unique_lock l( m_aMutex );
    maPath = _path;
    fireChangeListener(l);
}


// XAnimateMotion
Any AnimationNode::getOrigin()
{
    std::unique_lock aGuard( m_aMutex );
    return maOrigin;
}


// XAnimateMotion
void AnimationNode::setOrigin( const Any& _origin )
{
    std::unique_lock l( m_aMutex );
    maOrigin = _origin;
    fireChangeListener(l);
}

// XAnimatePhysics
Any AnimationNode::getStartVelocityX()
{
    std::unique_lock aGuard( m_aMutex );
    return maStartVelocityX;
}


// XAnimatePhysics
void AnimationNode::setStartVelocityX( const Any& _startvelocityx )
{
    std::unique_lock l( m_aMutex );
    maStartVelocityX = _startvelocityx;
    fireChangeListener(l);
}

// XAnimatePhysics
Any AnimationNode::getStartVelocityY()
{
    std::unique_lock aGuard( m_aMutex );
    return maStartVelocityY;
}


// XAnimatePhysics
void AnimationNode::setStartVelocityY( const Any& _startvelocityy )
{
    std::unique_lock l( m_aMutex );
    maStartVelocityY = _startvelocityy;
    fireChangeListener(l);
}


// XAnimatePhysics
Any AnimationNode::getDensity()
{
    std::unique_lock aGuard( m_aMutex );
    return maDensity;
}


// XAnimatePhysics
void AnimationNode::setDensity( const Any& _density )
{
    std::unique_lock l( m_aMutex );
    maDensity = _density;
    fireChangeListener(l);
}


// XAnimatePhysics
Any AnimationNode::getBounciness()
{
    std::unique_lock aGuard( m_aMutex );
    return maBounciness;
}


// XAnimatePhysics
void AnimationNode::setBounciness( const Any& _bounciness )
{
    std::unique_lock l( m_aMutex );
    maBounciness = _bounciness;
    fireChangeListener(l);
}


// XAnimateTransform
sal_Int16 AnimationNode::getTransformType()
{
    std::unique_lock aGuard( m_aMutex );
    return mnTransformType;
}


// XAnimateTransform
void AnimationNode::setTransformType( sal_Int16 _transformtype )
{
    std::unique_lock l( m_aMutex );
    if( _transformtype != mnTransformType )
    {
        mnTransformType = _transformtype;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Int16 AnimationNode::getTransition()
{
    std::unique_lock aGuard( m_aMutex );
    return mnTransition;
}


// XTransitionFilter
void AnimationNode::setTransition( sal_Int16 _transition )
{
    std::unique_lock l( m_aMutex );
    if( _transition != mnTransition )
    {
        mnTransition = _transition;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Int16 AnimationNode::getSubtype()
{
    std::unique_lock aGuard( m_aMutex );
    return mnSubtype;
}


// XTransitionFilter
void AnimationNode::setSubtype( sal_Int16 _subtype )
{
    std::unique_lock l( m_aMutex );
    if( _subtype != mnSubtype )
    {
        mnSubtype = _subtype;
        fireChangeListener(l);
    }
}


// XTransitionFilter
bool AnimationNode::getMode()
{
    std::unique_lock aGuard( m_aMutex );
    return mbMode;
}


// XTransitionFilter
void AnimationNode::setMode( bool _mode )
{
    std::unique_lock l( m_aMutex );
    if( bool(_mode) != mbMode )
    {
        mbMode = _mode;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Int32 AnimationNode::getFadeColor()
{
    std::unique_lock aGuard( m_aMutex );
    return mnFadeColor;
}


// XTransitionFilter
void AnimationNode::setFadeColor( sal_Int32 _fadecolor )
{
    std::unique_lock l( m_aMutex );
    if( _fadecolor != mnFadeColor )
    {
        mnFadeColor = _fadecolor;
        fireChangeListener(l);
    }
}


// XAudio
Any AnimationNode::getSource()
{
    std::unique_lock aGuard( m_aMutex );
    return maTarget;
}


// XAudio
void AnimationNode::setSource( const Any& _source )
{
    std::unique_lock l( m_aMutex );
    maTarget = _source;
    fireChangeListener(l);
}


// XAudio
double AnimationNode::getVolume()
{
    std::unique_lock aGuard( m_aMutex );
    return mfVolume;
}


// XAudio
void AnimationNode::setVolume( double _volume )
{
    std::unique_lock l( m_aMutex );
    if( _volume != mfVolume )
    {
        mfVolume = _volume;
        fireChangeListener(l);
    }
}

bool AnimationNode::getHideDuringShow()
{
    std::unique_lock aGuard(m_aMutex);
    return mbHideDuringShow;
}

void AnimationNode::setHideDuringShow(bool bHideDuringShow)
{
    std::unique_lock l(m_aMutex);
    if (bHideDuringShow != mbHideDuringShow)
    {
        mbHideDuringShow = bHideDuringShow;
        fireChangeListener(l);
    }
}

bool AnimationNode::getNarration()
{
    std::unique_lock aGuard(m_aMutex);
    return mbNarration;
}

void AnimationNode::setNarration(bool bNarration)
{
    std::unique_lock l(m_aMutex);
    if (bNarration != mbNarration)
    {
        mbNarration = bNarration;
        fireChangeListener(l);
    }
}

// XCommand
sal_Int16 AnimationNode::getCommand()
{
    std::unique_lock aGuard( m_aMutex );
    return mnCommand;
}


// XCommand
void AnimationNode::setCommand( sal_Int16 _command )
{
    std::unique_lock l( m_aMutex );
    if( _command != mnCommand )
    {
        mnCommand = _command;
        fireChangeListener(l);
    }
}


// XCommand
Any AnimationNode::getParameter()
{
    std::unique_lock aGuard( m_aMutex );
    return maParameter;
}


// XCommand
void AnimationNode::setParameter( const Any& _parameter )
{
    std::unique_lock l( m_aMutex );
    maParameter = _parameter;
    fireChangeListener(l);
}


// XElementAccess
Type AnimationNode::getElementType()
{
    return cppu::UnoType<XAnimationNode>::get();
}


// XElementAccess
bool AnimationNode::hasElements()
{
    std::unique_lock aGuard( m_aMutex );
    return !maChildren.empty();
}


// XEnumerationAccess
Reference< XEnumeration > AnimationNode::createEnumeration()
{
    std::unique_lock aGuard( m_aMutex );

    return new TimeContainerEnumeration(std::vector(maChildren));
}


// XTimeContainer
Reference< XAnimationNode > AnimationNode::insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild )
{
    std::unique_lock l( m_aMutex );

    if( !newChild.is() || !refChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, getXWeak(), -1);

    if( std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    auto before = std::find(maChildren.begin(), maChildren.end(), refChild);
    if( before == maChildren.end() )
        throw NoSuchElementException();

    maChildren.insert( before, newChild );

    Reference<XInterface> xThis(getXWeak());
    l.unlock();
    newChild->setParent( xThis );

    return newChild;
}


// XTimeContainer
Reference< XAnimationNode > AnimationNode::insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild )
{
    std::unique_lock l( m_aMutex );

    if( !newChild.is() || !refChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, getXWeak(), -1);

    if( std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    auto before = std::find(maChildren.begin(), maChildren.end(), refChild);
    if( before == maChildren.end() )
        throw NoSuchElementException();

    ++before;
    if( before != maChildren.end() )
        maChildren.insert( before, newChild );
    else
        maChildren.push_back( newChild );

    Reference<XInterface> xThis(getXWeak());
    l.unlock();
    newChild->setParent( xThis );

    return newChild;
}


// XTimeContainer
Reference< XAnimationNode > AnimationNode::replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild )
{
    std::unique_lock l( m_aMutex );

    if( !newChild.is() || !oldChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, getXWeak(), -1);

    if( std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    auto replace = std::find(maChildren.begin(), maChildren.end(), oldChild);
    if( replace == maChildren.end() )
        throw NoSuchElementException();

    (*replace) = newChild;

    Reference<XInterface> xThis(getXWeak());
    l.unlock();
    oldChild->setParent( Reference< XInterface >() );
    newChild->setParent( xThis );

    return newChild;
}


// XTimeContainer
Reference< XAnimationNode > AnimationNode::removeChild( const Reference< XAnimationNode >& oldChild )
{
    std::unique_lock l( m_aMutex );

    if( !oldChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, getXWeak(), 1);

    auto old = std::find(maChildren.begin(), maChildren.end(), oldChild);
    if( old == maChildren.end() )
        throw NoSuchElementException();

    maChildren.erase( old );

    l.unlock();
    oldChild->setParent( Reference< XInterface >() );

    return oldChild;
}


// XTimeContainer
Reference< XAnimationNode > AnimationNode::appendChild( const Reference< XAnimationNode >& newChild )
{
    Reference<XInterface> xThis(getXWeak());
    {
        std::unique_lock aGuard( m_aMutex );

        if( !newChild.is() )
            throw IllegalArgumentException(u"no child"_ustr, xThis, 1);

        if( std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
            throw ElementExistException({}, xThis);

        if( xThis == newChild )
            throw IllegalArgumentException(u"cannot append self"_ustr, xThis, -1);

        maChildren.push_back( newChild );
    }
    newChild->setParent( xThis );

    return newChild;
}


// XIterateContainer
sal_Int16 AnimationNode::getIterateType()
{
    std::unique_lock aGuard( m_aMutex );
    return mnIterateType;
}


// XIterateContainer
void AnimationNode::setIterateType( sal_Int16 _iteratetype )
{
    std::unique_lock l( m_aMutex );
    if( _iteratetype != mnIterateType )
    {
        mnIterateType = _iteratetype;
        fireChangeListener(l);
    }
}


// XIterateContainer
double AnimationNode::getIterateInterval()
{
    std::unique_lock aGuard( m_aMutex );
    return mfIterateInterval;
}


// XIterateContainer
void AnimationNode::setIterateInterval( double _iterateinterval )
{
    std::unique_lock l( m_aMutex );
    if( _iterateinterval != mfIterateInterval )
    {
        mfIterateInterval = _iterateinterval;
        fireChangeListener(l);
    }
}


// XChangesNotifier
void AnimationNode::addChangesListener( const Reference< XChangesListener >& aListener )
{
    std::unique_lock l( m_aMutex );
    maChangeListener.addInterface( l, aListener );
}


// XChangesNotifier
void AnimationNode::removeChangesListener( const Reference< XChangesListener >& aListener )
{
    std::unique_lock l( m_aMutex );
    maChangeListener.removeInterface(l, aListener);
}


void AnimationNode::fireChangeListener(std::unique_lock<std::mutex>& l)
{
    if( maChangeListener.getLength(l) != 0 )
    {
        Reference<XInterface> xSource(getXWeak(), UNO_QUERY);
        Sequence< ElementChange > aChanges;
        const ChangesEvent aEvent( xSource, Any( css::uno::Reference<XInterface>(cppu::getXWeak(mxParent.get().get())) ), aChanges );
        OInterfaceIteratorHelper4 aIterator( l, maChangeListener );
        l.unlock();
        while( aIterator.hasMoreElements() )
            aIterator.next()->changesOccurred( aEvent );
        l.lock();
    }

    //fdo#69645 use WeakReference of mxParent to test if mpParent is still valid
    rtl::Reference<AnimationNode> xGuard(mxParent);
    if (xGuard.is())
    {
        l.unlock();
        std::unique_lock l2(xGuard->m_aMutex);
        xGuard->fireChangeListener(l2);
    }
}


} // namespace animcore


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_ParallelTimeContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(PAR));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_SequenceTimeContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(SEQ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_IterateContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ITERATE));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_Animate_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATE));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateSet_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(SET));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateColor_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATECOLOR));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateMotion_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATEMOTION));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimatePhysics_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATEPHYSICS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateTransform_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATETRANSFORM));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_TransitionFilter_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(TRANSITIONFILTER));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_Audio_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(AUDIO));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_Command_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(COMMAND));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
