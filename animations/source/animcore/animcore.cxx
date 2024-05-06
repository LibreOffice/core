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
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XWeak;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Any;
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

using ::cppu::OWeakObject;

using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::animations::AnimationNodeType;

namespace animcore
{

namespace {
class AnimationNodeBase :   public XAnimateMotion,
                            public XAnimatePhysics,
                            public XAnimateColor,
                            public XTransitionFilter,
                            public XAnimateSet,
                            public XAnimateTransform,
                            public XParallelTimeContainer,
                            public XIterateContainer,
                            public XServiceInfo,
                            public XTypeProvider,
                            public XAudio,
                            public XCommand,
                            public XCloneable,
                            public XChangesNotifier,
                            public OWeakObject
{

};

class AnimationNode final:  public AnimationNodeBase
{
public:
    explicit AnimationNode(sal_Int16 nNodeType);
    explicit AnimationNode(const AnimationNode& rNode);

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() override;
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    // XChild
    virtual Reference< XInterface > SAL_CALL getParent() override;
    virtual void SAL_CALL setParent( const Reference< XInterface >& Parent ) override;

    // XCloneable
    virtual Reference< XCloneable > SAL_CALL createClone() override;

    // XAnimationNode
    virtual sal_Int16 SAL_CALL getType() override;
    virtual Any SAL_CALL getBegin() override;
    virtual void SAL_CALL setBegin( const Any& _begin ) override;
    virtual Any SAL_CALL getDuration() override;
    virtual void SAL_CALL setDuration( const Any& _duration ) override;
    virtual Any SAL_CALL getEnd() override;
    virtual void SAL_CALL setEnd( const Any& _end ) override;
    virtual Any SAL_CALL getEndSync() override;
    virtual void SAL_CALL setEndSync( const Any& _endsync ) override;
    virtual Any SAL_CALL getRepeatCount() override;
    virtual void SAL_CALL setRepeatCount( const Any& _repeatcount ) override;
    virtual Any SAL_CALL getRepeatDuration() override;
    virtual void SAL_CALL setRepeatDuration( const Any& _repeatduration ) override;
    virtual sal_Int16 SAL_CALL getFill() override;
    virtual void SAL_CALL setFill( sal_Int16 _fill ) override;
    virtual sal_Int16 SAL_CALL getFillDefault() override;
    virtual void SAL_CALL setFillDefault( sal_Int16 _filldefault ) override;
    virtual sal_Int16 SAL_CALL getRestart() override;
    virtual void SAL_CALL setRestart( sal_Int16 _restart ) override;
    virtual sal_Int16 SAL_CALL getRestartDefault() override;
    virtual void SAL_CALL setRestartDefault( sal_Int16 _restartdefault ) override;
    virtual double SAL_CALL getAcceleration() override;
    virtual void SAL_CALL setAcceleration( double _acceleration ) override;
    virtual double SAL_CALL getDecelerate() override;
    virtual void SAL_CALL setDecelerate( double _decelerate ) override;
    virtual sal_Bool SAL_CALL getAutoReverse() override;
    virtual void SAL_CALL setAutoReverse( sal_Bool _autoreverse ) override;
    virtual Sequence< NamedValue > SAL_CALL getUserData() override;
    virtual void SAL_CALL setUserData( const Sequence< NamedValue >& _userdata ) override;

    // XAnimate
    virtual Any SAL_CALL getTarget() override;
    virtual void SAL_CALL setTarget( const Any& _target ) override;
    virtual sal_Int16 SAL_CALL getSubItem() override;
    virtual void SAL_CALL setSubItem( sal_Int16 _subitem ) override;
    virtual OUString SAL_CALL getAttributeName() override;
    virtual void SAL_CALL setAttributeName( const OUString& _attribute ) override;
    virtual Sequence< Any > SAL_CALL getValues() override;
    virtual void SAL_CALL setValues( const Sequence< Any >& _values ) override;
    virtual Sequence< double > SAL_CALL getKeyTimes() override;
    virtual void SAL_CALL setKeyTimes( const Sequence< double >& _keytimes ) override;
    virtual sal_Int16 SAL_CALL getValueType() override;
    virtual void SAL_CALL setValueType( sal_Int16 _valuetype ) override;
    virtual sal_Int16 SAL_CALL getCalcMode() override;
    virtual void SAL_CALL setCalcMode( sal_Int16 _calcmode ) override;
    virtual sal_Bool SAL_CALL getAccumulate() override;
    virtual void SAL_CALL setAccumulate( sal_Bool _accumulate ) override;
    virtual sal_Int16 SAL_CALL getAdditive() override;
    virtual void SAL_CALL setAdditive( sal_Int16 _additive ) override;
    virtual Any SAL_CALL getFrom() override;
    virtual void SAL_CALL setFrom( const Any& _from ) override;
    virtual Any SAL_CALL getTo() override;
    virtual void SAL_CALL setTo( const Any& _to ) override;
    virtual Any SAL_CALL getBy() override;
    virtual void SAL_CALL setBy( const Any& _by ) override;
    virtual Sequence< TimeFilterPair > SAL_CALL getTimeFilter() override;
    virtual void SAL_CALL setTimeFilter( const Sequence< TimeFilterPair >& _timefilter ) override;
    virtual OUString SAL_CALL getFormula() override;
    virtual void SAL_CALL setFormula( const OUString& _formula ) override;

    // XAnimateColor
    virtual sal_Int16 SAL_CALL getColorInterpolation() override;
    virtual void SAL_CALL setColorInterpolation( sal_Int16 _colorspace ) override;
    virtual sal_Bool SAL_CALL getDirection() override;
    virtual void SAL_CALL setDirection( sal_Bool _direction ) override;

    // XAnimateMotion
    virtual Any SAL_CALL getPath() override;
    virtual void SAL_CALL setPath( const Any& _path ) override;
    virtual Any SAL_CALL getOrigin() override;
    virtual void SAL_CALL setOrigin( const Any& _origin ) override;

    // XAnimatePhysics
    virtual Any SAL_CALL getStartVelocityX() override;
    virtual void SAL_CALL setStartVelocityX( const Any& _startvelocityx ) override;
    virtual Any SAL_CALL getStartVelocityY() override;
    virtual void SAL_CALL setStartVelocityY( const Any& _startvelocityy ) override;
    virtual Any SAL_CALL getDensity() override;
    virtual void SAL_CALL setDensity( const Any& _density ) override;
    virtual Any SAL_CALL getBounciness() override;
    virtual void SAL_CALL setBounciness( const Any& _bounciness ) override;

    // XAnimateTransform
    virtual sal_Int16 SAL_CALL getTransformType() override;
    virtual void SAL_CALL setTransformType( sal_Int16 _transformtype ) override;

    // XTransitionFilter
    virtual sal_Int16 SAL_CALL getTransition() override;
    virtual void SAL_CALL setTransition( sal_Int16 _transition ) override;
    virtual sal_Int16 SAL_CALL getSubtype() override;
    virtual void SAL_CALL setSubtype( sal_Int16 _subtype ) override;
    virtual sal_Bool SAL_CALL getMode() override;
    virtual void SAL_CALL setMode( sal_Bool _mode ) override;
    virtual sal_Int32 SAL_CALL getFadeColor() override;
    virtual void SAL_CALL setFadeColor( sal_Int32 _fadecolor ) override;

    // XAudio
    virtual Any SAL_CALL getSource() override;
    virtual void SAL_CALL setSource( const Any& _source ) override;
    virtual double SAL_CALL getVolume() override;
    virtual void SAL_CALL setVolume( double _volume ) override;
    sal_Bool SAL_CALL getHideDuringShow() override;
    void SAL_CALL setHideDuringShow(sal_Bool bHideDuringShow) override;
    sal_Bool SAL_CALL getNarration() override;
    void SAL_CALL setNarration(sal_Bool bNarration) override;


    // XCommand - the following two shadowed by animate, unfortunately
//    virtual Any SAL_CALL getTarget() throw (RuntimeException);
//    virtual void SAL_CALL setTarget( const Any& _target ) throw (RuntimeException);
    virtual sal_Int16 SAL_CALL getCommand() override;
    virtual void SAL_CALL setCommand( sal_Int16 _command ) override;
    virtual Any SAL_CALL getParameter() override;
    virtual void SAL_CALL setParameter( const Any& _parameter ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual Reference< XEnumeration > SAL_CALL createEnumeration() override;

    // XTimeContainer
    virtual Reference< XAnimationNode > SAL_CALL insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) override;
    virtual Reference< XAnimationNode > SAL_CALL insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) override;
    virtual Reference< XAnimationNode > SAL_CALL replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild ) override;
    virtual Reference< XAnimationNode > SAL_CALL removeChild( const Reference< XAnimationNode >& oldChild ) override;
    virtual Reference< XAnimationNode > SAL_CALL appendChild( const Reference< XAnimationNode >& newChild ) override;

    // XIterateContainer
    virtual sal_Int16 SAL_CALL getIterateType() override;
    virtual void SAL_CALL setIterateType( sal_Int16 _iteratetype ) override;
    virtual double SAL_CALL getIterateInterval() override;
    virtual void SAL_CALL setIterateInterval( double _iterateinterval ) override;

    // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const Reference< XChangesListener >& aListener ) override;
    virtual void SAL_CALL removeChangesListener( const Reference< XChangesListener >& aListener ) override;

    void fireChangeListener(std::unique_lock<std::mutex>&);

private:
    std::mutex m_aMutex;
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
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual Any SAL_CALL nextElement(  ) override;

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
sal_Bool SAL_CALL TimeContainerEnumeration::hasMoreElements()
{
    std::unique_lock aGuard( m_aMutex );

    return maIter != maChildren.end();
}

Any SAL_CALL TimeContainerEnumeration::nextElement()
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
Any SAL_CALL AnimationNode::queryInterface( const Type& aType )
{
    Any aRet( ::cppu::queryInterface(
        aType,
        static_cast< XServiceInfo * >( this ),
        static_cast< XTypeProvider * >( this ),
        static_cast< XChild * >( static_cast< XTimeContainer * >( static_cast< XIterateContainer * >(this) ) ),
        static_cast< XCloneable* >( this ),
        static_cast< XAnimationNode* >( static_cast< XTimeContainer * >( static_cast< XIterateContainer * >(this) ) ),
        static_cast< XInterface* >(static_cast< OWeakObject * >(this)),
        static_cast< XWeak* >(static_cast< OWeakObject * >(this)),
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


Sequence< sal_Int8 > AnimationNode::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// XInterface
void SAL_CALL AnimationNode::acquire(  ) noexcept
{
    OWeakObject::acquire();
}


// XInterface
void SAL_CALL AnimationNode::release(  ) noexcept
{
    OWeakObject::release();
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
sal_Bool AnimationNode::supportsService(const OUString& ServiceName)
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
sal_Int16 SAL_CALL AnimationNode::getType()
{
    return mnNodeType;
}


// XAnimationNode
Any SAL_CALL AnimationNode::getBegin()
{
    std::unique_lock aGuard( m_aMutex );
    return maBegin;
}


// XAnimationNode
void SAL_CALL AnimationNode::setBegin( const Any& _begin )
{
    std::unique_lock aGuard( m_aMutex );
    if( _begin != maBegin )
    {
        maBegin = _begin;
        fireChangeListener(aGuard);
    }
}


// XAnimationNode
Any SAL_CALL AnimationNode::getDuration()
{
    std::unique_lock aGuard( m_aMutex );
    return maDuration;
}


// XAnimationNode
void SAL_CALL AnimationNode::setDuration( const Any& _duration )
{
    std::unique_lock aGuard( m_aMutex );
    if( _duration != maDuration )
    {
        maDuration = _duration;
        fireChangeListener(aGuard);
    }
}


// XAnimationNode
Any SAL_CALL AnimationNode::getEnd()
{
    std::unique_lock aGuard( m_aMutex );
    return maEnd;
}


// XAnimationNode
void SAL_CALL AnimationNode::setEnd( const Any& _end )
{
    std::unique_lock aGuard( m_aMutex );
    if( _end != maEnd )
    {
        maEnd = _end;
        fireChangeListener(aGuard);
    }
}


// XAnimationNode
Any SAL_CALL AnimationNode::getEndSync()
{
    std::unique_lock aGuard( m_aMutex );
    return maEndSync;
}


// XAnimationNode
void SAL_CALL AnimationNode::setEndSync( const Any& _endsync )
{
    std::unique_lock l( m_aMutex );
    if( _endsync != maEndSync )
    {
        maEndSync = _endsync;
        fireChangeListener(l);
    }
}


// XAnimationNode
Any SAL_CALL AnimationNode::getRepeatCount()
{
    std::unique_lock aGuard( m_aMutex );
    return maRepeatCount;
}


// XAnimationNode
void SAL_CALL AnimationNode::setRepeatCount( const Any& _repeatcount )
{
    std::unique_lock l( m_aMutex );
    if( _repeatcount != maRepeatCount )
    {
        maRepeatCount = _repeatcount;
        fireChangeListener(l);
    }
}


// XAnimationNode
Any SAL_CALL AnimationNode::getRepeatDuration()
{
    std::unique_lock aGuard( m_aMutex );
    return maRepeatDuration;
}


// XAnimationNode
void SAL_CALL AnimationNode::setRepeatDuration( const Any& _repeatduration )
{
    std::unique_lock l( m_aMutex );
    if( _repeatduration != maRepeatDuration )
    {
        maRepeatDuration = _repeatduration;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getFill()
{
    std::unique_lock aGuard( m_aMutex );
    return mnFill;
}


// XAnimationNode
void SAL_CALL AnimationNode::setFill( sal_Int16 _fill )
{
    std::unique_lock l( m_aMutex );
    if( _fill != mnFill )
    {
        mnFill = _fill;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getFillDefault()
{
    std::unique_lock aGuard( m_aMutex );
    return mnFillDefault;
}


// XAnimationNode
void SAL_CALL AnimationNode::setFillDefault( sal_Int16 _filldefault )
{
    std::unique_lock l( m_aMutex );
    if( _filldefault != mnFillDefault )
    {
        mnFillDefault = _filldefault;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getRestart()
{
    std::unique_lock aGuard( m_aMutex );
    return mnRestart;
}


// XAnimationNode
void SAL_CALL AnimationNode::setRestart( sal_Int16 _restart )
{
    std::unique_lock l( m_aMutex );
    if( _restart != mnRestart )
    {
        mnRestart = _restart;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getRestartDefault()
{
    std::unique_lock aGuard( m_aMutex );
    return mnRestartDefault;
}


// XAnimationNode
void SAL_CALL AnimationNode::setRestartDefault( sal_Int16 _restartdefault )
{
    std::unique_lock l( m_aMutex );
    if( _restartdefault != mnRestartDefault )
    {
        mnRestartDefault = _restartdefault;
        fireChangeListener(l);
    }
}


// XAnimationNode
double SAL_CALL AnimationNode::getAcceleration()
{
    std::unique_lock aGuard( m_aMutex );
    return mfAcceleration;
}


// XAnimationNode
void SAL_CALL AnimationNode::setAcceleration( double _acceleration )
{
    std::unique_lock l( m_aMutex );
    if( _acceleration != mfAcceleration )
    {
        mfAcceleration = _acceleration;
        fireChangeListener(l);
    }
}


// XAnimationNode
double SAL_CALL AnimationNode::getDecelerate()
{
    std::unique_lock aGuard( m_aMutex );
    return mfDecelerate;
}


// XAnimationNode
void SAL_CALL AnimationNode::setDecelerate( double _decelerate )
{
    std::unique_lock l( m_aMutex );
    if( _decelerate != mfDecelerate )
    {
        mfDecelerate = _decelerate;
        fireChangeListener(l);
    }
}


// XAnimationNode
sal_Bool SAL_CALL AnimationNode::getAutoReverse()
{
    std::unique_lock aGuard( m_aMutex );
    return mbAutoReverse;
}


// XAnimationNode
void SAL_CALL AnimationNode::setAutoReverse( sal_Bool _autoreverse )
{
    std::unique_lock l( m_aMutex );
    if( bool(_autoreverse) != mbAutoReverse )
    {
        mbAutoReverse = _autoreverse;
        fireChangeListener(l);
    }
}


Sequence< NamedValue > SAL_CALL AnimationNode::getUserData()
{
    std::unique_lock aGuard( m_aMutex );
    return maUserData;
}


void SAL_CALL AnimationNode::setUserData( const Sequence< NamedValue >& _userdata )
{
    std::unique_lock l( m_aMutex );
    maUserData = _userdata;
    fireChangeListener(l);
}


// XChild
Reference< XInterface > SAL_CALL AnimationNode::getParent()
{
    std::unique_lock aGuard( m_aMutex );
    return static_cast<cppu::OWeakObject*>(mxParent.get().get());
}


// XChild
void SAL_CALL AnimationNode::setParent( const Reference< XInterface >& Parent )
{
    std::unique_lock l( m_aMutex );
    if( Parent.get() != static_cast<cppu::OWeakObject*>(mxParent.get().get()) )
    {
        rtl::Reference<AnimationNode> xParent = dynamic_cast<AnimationNode*>(Parent.get());
        mxParent = xParent.get();
        assert(bool(xParent) == bool(Parent) && "only support AnimationNode subtypes");

        fireChangeListener(l);
    }
}


// XCloneable
Reference< XCloneable > SAL_CALL AnimationNode::createClone()
{
    std::unique_lock aGuard( m_aMutex );

    Reference< XCloneable > xNewNode;
    try
    {
        xNewNode = new AnimationNode( *this );

        if( !maChildren.empty() )
        {
            Reference< XTimeContainer > xContainer( xNewNode, UNO_QUERY );
            if( xContainer.is() )
            {
                for (auto const& child : maChildren)
                {
                    Reference< XCloneable > xCloneable(child, UNO_QUERY );
                    if( xCloneable.is() ) try
                    {
                        Reference< XAnimationNode > xNewChildNode( xCloneable->createClone(), UNO_QUERY );
                        if( xNewChildNode.is() )
                            xContainer->appendChild( xNewChildNode );
                    }
                    catch(const Exception&)
                    {
                        SAL_INFO("animations", "animations::AnimationNode::createClone(), exception caught!");
                    }
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
Any SAL_CALL AnimationNode::getTarget()
{
    std::unique_lock aGuard( m_aMutex );
    return maTarget;
}


// XAnimate
void SAL_CALL AnimationNode::setTarget( const Any& _target )
{
    std::unique_lock l( m_aMutex );
    if( _target != maTarget )
    {
        maTarget= _target;
        fireChangeListener(l);
    }
}


// XAnimate
OUString SAL_CALL AnimationNode::getAttributeName()
{
    std::unique_lock aGuard( m_aMutex );
    return maAttributeName;
}


// XAnimate
void SAL_CALL AnimationNode::setAttributeName( const OUString& _attribute )
{
    std::unique_lock l( m_aMutex );
    if( _attribute != maAttributeName )
    {
        maAttributeName = _attribute;
        fireChangeListener(l);
    }
}


// XAnimate
Sequence< Any > SAL_CALL AnimationNode::getValues()
{
    std::unique_lock aGuard( m_aMutex );
    return maValues;
}


// XAnimate
void SAL_CALL AnimationNode::setValues( const Sequence< Any >& _values )
{
    std::unique_lock l( m_aMutex );
    maValues = _values;
    fireChangeListener(l);
}


// XAnimate
sal_Int16 SAL_CALL AnimationNode::getSubItem()
{
    std::unique_lock aGuard( m_aMutex );
    return mnSubItem;
}


// XAnimate
void SAL_CALL AnimationNode::setSubItem( sal_Int16 _subitem )
{
    std::unique_lock l( m_aMutex );
    if( _subitem != mnSubItem )
    {
        mnSubItem = _subitem;
        fireChangeListener(l);
    }
}


// XAnimate
Sequence< double > SAL_CALL AnimationNode::getKeyTimes()
{
    std::unique_lock aGuard( m_aMutex );
    return maKeyTimes;
}


// XAnimate
void SAL_CALL AnimationNode::setKeyTimes( const Sequence< double >& _keytimes )
{
    std::unique_lock l( m_aMutex );
    maKeyTimes = _keytimes;
    fireChangeListener(l);
}


// XAnimate
sal_Int16 SAL_CALL AnimationNode::getValueType()
{
    std::unique_lock aGuard( m_aMutex );
    return mnValueType;
}


void SAL_CALL AnimationNode::setValueType( sal_Int16 _valuetype )
{
    std::unique_lock l( m_aMutex );
    if( _valuetype != mnValueType )
    {
        mnValueType = _valuetype;
        fireChangeListener(l);
    }
}


// XAnimate
sal_Int16 SAL_CALL AnimationNode::getCalcMode()
{
    std::unique_lock aGuard( m_aMutex );
    return mnCalcMode;
}


// XAnimate
void SAL_CALL AnimationNode::setCalcMode( sal_Int16 _calcmode )
{
    std::unique_lock l( m_aMutex );
    if( _calcmode != mnCalcMode )
    {
        mnCalcMode = _calcmode;
        fireChangeListener(l);
    }
}


// XAnimate
sal_Bool SAL_CALL AnimationNode::getAccumulate()
{
    std::unique_lock aGuard( m_aMutex );
    return mbAccumulate;
}


// XAnimate
void SAL_CALL AnimationNode::setAccumulate( sal_Bool _accumulate )
{
    std::unique_lock l( m_aMutex );
    if( bool(_accumulate) != mbAccumulate )
    {
        mbAccumulate = _accumulate;
        fireChangeListener(l);
    }
}


// XAnimate
sal_Int16 SAL_CALL AnimationNode::getAdditive()
{
    std::unique_lock aGuard( m_aMutex );
    return mnAdditive;
}


// XAnimate
void SAL_CALL AnimationNode::setAdditive( sal_Int16 _additive )
{
    std::unique_lock l( m_aMutex );
    if( _additive != mnAdditive )
    {
        mnAdditive = _additive;
        fireChangeListener(l);
    }
}


// XAnimate
Any SAL_CALL AnimationNode::getFrom()
{
    std::unique_lock aGuard( m_aMutex );
    return maFrom;
}


// XAnimate
void SAL_CALL AnimationNode::setFrom( const Any& _from )
{
    std::unique_lock l( m_aMutex );
    if( _from != maFrom )
    {
        maFrom = _from;
        fireChangeListener(l);
    }
}


// XAnimate
Any SAL_CALL AnimationNode::getTo()
{
    std::unique_lock aGuard( m_aMutex );
    return maTo;
}


// XAnimate
void SAL_CALL AnimationNode::setTo( const Any& _to )
{
    std::unique_lock l( m_aMutex );
    if( _to != maTo )
    {
        maTo = _to;
        fireChangeListener(l);
    }
}


// XAnimate
Any SAL_CALL AnimationNode::getBy()
{
    std::unique_lock aGuard( m_aMutex );
    return maBy;
}


// XAnimate
void SAL_CALL AnimationNode::setBy( const Any& _by )
{
    std::unique_lock l( m_aMutex );
    if( _by != maBy )
    {
        maBy = _by;
        fireChangeListener(l);
    }
}


// XAnimate
Sequence< TimeFilterPair > SAL_CALL AnimationNode::getTimeFilter()
{
    std::unique_lock aGuard( m_aMutex );
    return maTimeFilter;
}


// XAnimate
void SAL_CALL AnimationNode::setTimeFilter( const Sequence< TimeFilterPair >& _timefilter )
{
    std::unique_lock l( m_aMutex );
    maTimeFilter = _timefilter;
    fireChangeListener(l);
}


OUString SAL_CALL AnimationNode::getFormula()
{
    std::unique_lock aGuard( m_aMutex );
    return maFormula;
}


void SAL_CALL AnimationNode::setFormula( const OUString& _formula )
{
    std::unique_lock l( m_aMutex );
    if( _formula != maFormula )
    {
        maFormula = _formula;
        fireChangeListener(l);
    }
}


// XAnimateColor
sal_Int16 SAL_CALL AnimationNode::getColorInterpolation()
{
    std::unique_lock aGuard( m_aMutex );
    return mnColorSpace;
}


// XAnimateColor
void SAL_CALL AnimationNode::setColorInterpolation( sal_Int16 _colorspace )
{
    std::unique_lock l( m_aMutex );
    if( _colorspace != mnColorSpace )
    {
        mnColorSpace = _colorspace;
        fireChangeListener(l);
    }
}


// XAnimateColor
sal_Bool SAL_CALL AnimationNode::getDirection()
{
    std::unique_lock aGuard( m_aMutex );
    return mbDirection;
}


// XAnimateColor
void SAL_CALL AnimationNode::setDirection( sal_Bool _direction )
{
    std::unique_lock l( m_aMutex );
    if( bool(_direction) != mbDirection )
    {
        mbDirection = _direction;
        fireChangeListener(l);
    }
}


// XAnimateMotion
Any SAL_CALL AnimationNode::getPath()
{
    std::unique_lock aGuard( m_aMutex );
    return maPath;
}


// XAnimateMotion
void SAL_CALL AnimationNode::setPath( const Any& _path )
{
    std::unique_lock l( m_aMutex );
    maPath = _path;
    fireChangeListener(l);
}


// XAnimateMotion
Any SAL_CALL AnimationNode::getOrigin()
{
    std::unique_lock aGuard( m_aMutex );
    return maOrigin;
}


// XAnimateMotion
void SAL_CALL AnimationNode::setOrigin( const Any& _origin )
{
    std::unique_lock l( m_aMutex );
    maOrigin = _origin;
    fireChangeListener(l);
}

// XAnimatePhysics
Any SAL_CALL AnimationNode::getStartVelocityX()
{
    std::unique_lock aGuard( m_aMutex );
    return maStartVelocityX;
}


// XAnimatePhysics
void SAL_CALL AnimationNode::setStartVelocityX( const Any& _startvelocityx )
{
    std::unique_lock l( m_aMutex );
    maStartVelocityX = _startvelocityx;
    fireChangeListener(l);
}

// XAnimatePhysics
Any SAL_CALL AnimationNode::getStartVelocityY()
{
    std::unique_lock aGuard( m_aMutex );
    return maStartVelocityY;
}


// XAnimatePhysics
void SAL_CALL AnimationNode::setStartVelocityY( const Any& _startvelocityy )
{
    std::unique_lock l( m_aMutex );
    maStartVelocityY = _startvelocityy;
    fireChangeListener(l);
}


// XAnimatePhysics
Any SAL_CALL AnimationNode::getDensity()
{
    std::unique_lock aGuard( m_aMutex );
    return maDensity;
}


// XAnimatePhysics
void SAL_CALL AnimationNode::setDensity( const Any& _density )
{
    std::unique_lock l( m_aMutex );
    maDensity = _density;
    fireChangeListener(l);
}


// XAnimatePhysics
Any SAL_CALL AnimationNode::getBounciness()
{
    std::unique_lock aGuard( m_aMutex );
    return maBounciness;
}


// XAnimatePhysics
void SAL_CALL AnimationNode::setBounciness( const Any& _bounciness )
{
    std::unique_lock l( m_aMutex );
    maBounciness = _bounciness;
    fireChangeListener(l);
}


// XAnimateTransform
sal_Int16 SAL_CALL AnimationNode::getTransformType()
{
    std::unique_lock aGuard( m_aMutex );
    return mnTransformType;
}


// XAnimateTransform
void SAL_CALL AnimationNode::setTransformType( sal_Int16 _transformtype )
{
    std::unique_lock l( m_aMutex );
    if( _transformtype != mnTransformType )
    {
        mnTransformType = _transformtype;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Int16 SAL_CALL AnimationNode::getTransition()
{
    std::unique_lock aGuard( m_aMutex );
    return mnTransition;
}


// XTransitionFilter
void SAL_CALL AnimationNode::setTransition( sal_Int16 _transition )
{
    std::unique_lock l( m_aMutex );
    if( _transition != mnTransition )
    {
        mnTransition = _transition;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Int16 SAL_CALL AnimationNode::getSubtype()
{
    std::unique_lock aGuard( m_aMutex );
    return mnSubtype;
}


// XTransitionFilter
void SAL_CALL AnimationNode::setSubtype( sal_Int16 _subtype )
{
    std::unique_lock l( m_aMutex );
    if( _subtype != mnSubtype )
    {
        mnSubtype = _subtype;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Bool SAL_CALL AnimationNode::getMode()
{
    std::unique_lock aGuard( m_aMutex );
    return mbMode;
}


// XTransitionFilter
void SAL_CALL AnimationNode::setMode( sal_Bool _mode )
{
    std::unique_lock l( m_aMutex );
    if( bool(_mode) != mbMode )
    {
        mbMode = _mode;
        fireChangeListener(l);
    }
}


// XTransitionFilter
sal_Int32 SAL_CALL AnimationNode::getFadeColor()
{
    std::unique_lock aGuard( m_aMutex );
    return mnFadeColor;
}


// XTransitionFilter
void SAL_CALL AnimationNode::setFadeColor( sal_Int32 _fadecolor )
{
    std::unique_lock l( m_aMutex );
    if( _fadecolor != mnFadeColor )
    {
        mnFadeColor = _fadecolor;
        fireChangeListener(l);
    }
}


// XAudio
Any SAL_CALL AnimationNode::getSource()
{
    std::unique_lock aGuard( m_aMutex );
    return maTarget;
}


// XAudio
void SAL_CALL AnimationNode::setSource( const Any& _source )
{
    std::unique_lock l( m_aMutex );
    maTarget = _source;
    fireChangeListener(l);
}


// XAudio
double SAL_CALL AnimationNode::getVolume()
{
    std::unique_lock aGuard( m_aMutex );
    return mfVolume;
}


// XAudio
void SAL_CALL AnimationNode::setVolume( double _volume )
{
    std::unique_lock l( m_aMutex );
    if( _volume != mfVolume )
    {
        mfVolume = _volume;
        fireChangeListener(l);
    }
}

sal_Bool SAL_CALL AnimationNode::getHideDuringShow()
{
    std::unique_lock aGuard(m_aMutex);
    return mbHideDuringShow;
}

void SAL_CALL AnimationNode::setHideDuringShow(sal_Bool bHideDuringShow)
{
    std::unique_lock l(m_aMutex);
    if (static_cast<bool>(bHideDuringShow) != mbHideDuringShow)
    {
        mbHideDuringShow = bHideDuringShow;
        fireChangeListener(l);
    }
}

sal_Bool SAL_CALL AnimationNode::getNarration()
{
    std::unique_lock aGuard(m_aMutex);
    return mbNarration;
}

void SAL_CALL AnimationNode::setNarration(sal_Bool bNarration)
{
    std::unique_lock l(m_aMutex);
    if (static_cast<bool>(bNarration) != mbNarration)
    {
        mbNarration = bNarration;
        fireChangeListener(l);
    }
}

// XCommand
sal_Int16 SAL_CALL AnimationNode::getCommand()
{
    std::unique_lock aGuard( m_aMutex );
    return mnCommand;
}


// XCommand
void SAL_CALL AnimationNode::setCommand( sal_Int16 _command )
{
    std::unique_lock l( m_aMutex );
    if( _command != mnCommand )
    {
        mnCommand = _command;
        fireChangeListener(l);
    }
}


// XCommand
Any SAL_CALL AnimationNode::getParameter()
{
    std::unique_lock aGuard( m_aMutex );
    return maParameter;
}


// XCommand
void SAL_CALL AnimationNode::setParameter( const Any& _parameter )
{
    std::unique_lock l( m_aMutex );
    maParameter = _parameter;
    fireChangeListener(l);
}


// XElementAccess
Type SAL_CALL AnimationNode::getElementType()
{
    return cppu::UnoType<XAnimationNode>::get();
}


// XElementAccess
sal_Bool SAL_CALL AnimationNode::hasElements()
{
    std::unique_lock aGuard( m_aMutex );
    return !maChildren.empty();
}


// XEnumerationAccess
Reference< XEnumeration > SAL_CALL AnimationNode::createEnumeration()
{
    std::unique_lock aGuard( m_aMutex );

    return new TimeContainerEnumeration(std::vector(maChildren));
}


// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild )
{
    std::unique_lock l( m_aMutex );

    if( !newChild.is() || !refChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, static_cast<cppu::OWeakObject*>(this), -1);

    if( std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    auto before = std::find(maChildren.begin(), maChildren.end(), refChild);
    if( before == maChildren.end() )
        throw NoSuchElementException();

    maChildren.insert( before, newChild );

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    l.unlock();
    newChild->setParent( xThis );

    return newChild;
}


// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild )
{
    std::unique_lock l( m_aMutex );

    if( !newChild.is() || !refChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, static_cast<cppu::OWeakObject*>(this), -1);

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

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    l.unlock();
    newChild->setParent( xThis );

    return newChild;
}


// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild )
{
    std::unique_lock l( m_aMutex );

    if( !newChild.is() || !oldChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, static_cast<cppu::OWeakObject*>(this), -1);

    if( std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    auto replace = std::find(maChildren.begin(), maChildren.end(), oldChild);
    if( replace == maChildren.end() )
        throw NoSuchElementException();

    (*replace) = newChild;

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    l.unlock();
    oldChild->setParent( Reference< XInterface >() );
    newChild->setParent( xThis );

    return newChild;
}


// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::removeChild( const Reference< XAnimationNode >& oldChild )
{
    std::unique_lock l( m_aMutex );

    if( !oldChild.is() )
        throw IllegalArgumentException(u"no child"_ustr, static_cast<cppu::OWeakObject*>(this), 1);

    auto old = std::find(maChildren.begin(), maChildren.end(), oldChild);
    if( old == maChildren.end() )
        throw NoSuchElementException();

    maChildren.erase( old );

    l.unlock();
    oldChild->setParent( Reference< XInterface >() );

    return oldChild;
}


// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::appendChild( const Reference< XAnimationNode >& newChild )
{
    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
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
sal_Int16 SAL_CALL AnimationNode::getIterateType()
{
    std::unique_lock aGuard( m_aMutex );
    return mnIterateType;
}


// XIterateContainer
void SAL_CALL AnimationNode::setIterateType( sal_Int16 _iteratetype )
{
    std::unique_lock l( m_aMutex );
    if( _iteratetype != mnIterateType )
    {
        mnIterateType = _iteratetype;
        fireChangeListener(l);
    }
}


// XIterateContainer
double SAL_CALL AnimationNode::getIterateInterval()
{
    std::unique_lock aGuard( m_aMutex );
    return mfIterateInterval;
}


// XIterateContainer
void SAL_CALL AnimationNode::setIterateInterval( double _iterateinterval )
{
    std::unique_lock l( m_aMutex );
    if( _iterateinterval != mfIterateInterval )
    {
        mfIterateInterval = _iterateinterval;
        fireChangeListener(l);
    }
}


// XChangesNotifier
void SAL_CALL AnimationNode::addChangesListener( const Reference< XChangesListener >& aListener )
{
    std::unique_lock l( m_aMutex );
    maChangeListener.addInterface( l, aListener );
}


// XChangesNotifier
void SAL_CALL AnimationNode::removeChangesListener( const Reference< XChangesListener >& aListener )
{
    std::unique_lock l( m_aMutex );
    maChangeListener.removeInterface(l, aListener);
}


void AnimationNode::fireChangeListener(std::unique_lock<std::mutex>& l)
{
    if( maChangeListener.getLength(l) != 0 )
    {
        Reference< XInterface > xSource( static_cast<OWeakObject*>(this), UNO_QUERY );
        Sequence< ElementChange > aChanges;
        const ChangesEvent aEvent( xSource, Any( css::uno::Reference<XInterface>(static_cast<cppu::OWeakObject*>(mxParent.get().get())) ), aChanges );
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
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(PAR));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_SequenceTimeContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(SEQ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_IterateContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ITERATE));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_Animate_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATE));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateSet_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(SET));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateColor_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATECOLOR));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateMotion_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATEMOTION));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimatePhysics_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATEPHYSICS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_AnimateTransform_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATETRANSFORM));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_TransitionFilter_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(TRANSITIONFILTER));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_Audio_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(AUDIO));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_animations_Command_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(COMMAND));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
