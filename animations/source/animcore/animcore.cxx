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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
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
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>

#include <cppuhelper/implbase.hxx>

#include <osl/mutex.hxx>
#include <list>
#include <algorithm>
#include <string.h>

using ::osl::Mutex;
using ::osl::Guard;
using ::cppu::OInterfaceContainerHelper;
using ::cppu::OInterfaceIteratorHelper;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XWeak;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::lang::NoSupportException;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::lang::XTypeProvider;
using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::ElementExistException;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::lang::XUnoTunnel;
using ::com::sun::star::util::XChangesNotifier;
using ::com::sun::star::util::XChangesListener;
using ::com::sun::star::util::ElementChange;
using ::com::sun::star::util::ChangesEvent;

using ::cppu::OWeakObject;

using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::animations::AnimationNodeType;

namespace animcore
{



typedef ::std::list< Reference< XAnimationNode > > ChildList_t;



class AnimationNodeBase :   public XAnimateMotion,
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
                            public XUnoTunnel,
                            public OWeakObject
{
public:
    // our first, last and only protection from mutli-threads!
    Mutex maMutex;
};

class AnimationNode : public AnimationNodeBase
{
public:
    explicit AnimationNode(sal_Int16 nNodeType);
    explicit AnimationNode(const AnimationNode& rNode);
    virtual ~AnimationNode();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw(std::exception) override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(std::exception) override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception) override;

    // XChild
    virtual Reference< XInterface > SAL_CALL getParent() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const Reference< XInterface >& Parent ) throw (NoSupportException, RuntimeException, std::exception) override;

    // XCloneable
    virtual Reference< XCloneable > SAL_CALL createClone() throw (RuntimeException, std::exception) override;

    // XAnimationNode
    virtual sal_Int16 SAL_CALL getType() throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getBegin() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setBegin( const Any& _begin ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getDuration() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setDuration( const Any& _duration ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getEnd() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setEnd( const Any& _end ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getEndSync() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setEndSync( const Any& _endsync ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getRepeatCount() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setRepeatCount( const Any& _repeatcount ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getRepeatDuration() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setRepeatDuration( const Any& _repeatduration ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getFill() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setFill( sal_Int16 _fill ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getFillDefault() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setFillDefault( sal_Int16 _filldefault ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getRestart() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setRestart( sal_Int16 _restart ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getRestartDefault() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setRestartDefault( sal_Int16 _restartdefault ) throw (RuntimeException, std::exception) override;
    virtual double SAL_CALL getAcceleration() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setAcceleration( double _acceleration ) throw (RuntimeException, std::exception) override;
    virtual double SAL_CALL getDecelerate() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setDecelerate( double _decelerate ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoReverse() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoReverse( sal_Bool _autoreverse ) throw (RuntimeException, std::exception) override;
    virtual Sequence< NamedValue > SAL_CALL getUserData() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setUserData( const Sequence< NamedValue >& _userdata ) throw (RuntimeException, std::exception) override;

    // XAnimate
    virtual Any SAL_CALL getTarget() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setTarget( const Any& _target ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getSubItem() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSubItem( sal_Int16 _subitem ) throw (RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAttributeName() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setAttributeName( const OUString& _attribute ) throw (RuntimeException, std::exception) override;
    virtual Sequence< Any > SAL_CALL getValues() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setValues( const Sequence< Any >& _values ) throw (RuntimeException, std::exception) override;
    virtual Sequence< double > SAL_CALL getKeyTimes() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setKeyTimes( const Sequence< double >& _keytimes ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getValueType() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setValueType( sal_Int16 _valuetype ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getCalcMode() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setCalcMode( sal_Int16 _calcmode ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAccumulate() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setAccumulate( sal_Bool _accumulate ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAdditive() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setAdditive( sal_Int16 _additive ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getFrom() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setFrom( const Any& _from ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getTo() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setTo( const Any& _to ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getBy() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setBy( const Any& _by ) throw (RuntimeException, std::exception) override;
    virtual Sequence< TimeFilterPair > SAL_CALL getTimeFilter() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setTimeFilter( const Sequence< TimeFilterPair >& _timefilter ) throw (RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFormula() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setFormula( const OUString& _formula ) throw (RuntimeException, std::exception) override;

    // XAnimateColor
    virtual sal_Int16 SAL_CALL getColorInterpolation() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setColorInterpolation( sal_Int16 _colorspace ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getDirection() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setDirection( sal_Bool _direction ) throw (RuntimeException, std::exception) override;

    // XAnimateMotion
    virtual Any SAL_CALL getPath() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setPath( const Any& _path ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getOrigin() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setOrigin( const Any& _origin ) throw (RuntimeException, std::exception) override;

    // XAnimateTransform
    virtual sal_Int16 SAL_CALL getTransformType() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setTransformType( sal_Int16 _transformtype ) throw (RuntimeException, std::exception) override;

    // XTransitionFilter
    virtual sal_Int16 SAL_CALL getTransition() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setTransition( sal_Int16 _transition ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getSubtype() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSubtype( sal_Int16 _subtype ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMode() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setMode( sal_Bool _mode ) throw (RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFadeColor() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setFadeColor( sal_Int32 _fadecolor ) throw (RuntimeException, std::exception) override;

    // XAudio
    virtual Any SAL_CALL getSource() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSource( const Any& _source ) throw (RuntimeException, std::exception) override;
    virtual double SAL_CALL getVolume() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setVolume( double _volume ) throw (RuntimeException, std::exception) override;


    // XCommand - the following two shadowed by animate, unfortunately
//    virtual Any SAL_CALL getTarget() throw (RuntimeException);
//    virtual void SAL_CALL setTarget( const Any& _target ) throw (RuntimeException);
    virtual sal_Int16 SAL_CALL getCommand() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setCommand( sal_Int16 _command ) throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL getParameter() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setParameter( const Any& _parameter ) throw (RuntimeException, std::exception) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual Reference< XEnumeration > SAL_CALL createEnumeration() throw (RuntimeException, std::exception) override;

    // XTimeContainer
    virtual Reference< XAnimationNode > SAL_CALL insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;
    virtual Reference< XAnimationNode > SAL_CALL insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;
    virtual Reference< XAnimationNode > SAL_CALL replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild ) throw( IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;
    virtual Reference< XAnimationNode > SAL_CALL removeChild( const Reference< XAnimationNode >& oldChild ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception) override;
    virtual Reference< XAnimationNode > SAL_CALL appendChild( const Reference< XAnimationNode >& newChild ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;

    // XIterateContainer
    virtual sal_Int16 SAL_CALL getIterateType() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setIterateType( sal_Int16 _iteratetype ) throw (RuntimeException, std::exception) override;
    virtual double SAL_CALL getIterateInterval() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setIterateInterval( double _iterateinterval ) throw (RuntimeException, std::exception) override;

    // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const Reference< XChangesListener >& aListener ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangesListener( const Reference< XChangesListener >& aListener ) throw (RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething( const Sequence< ::sal_Int8 >& aIdentifier ) throw (RuntimeException, std::exception) override;

    static const Sequence< sal_Int8 > & getUnoTunnelId();
    void fireChangeListener();

private:
    OInterfaceContainerHelper   maChangeListener;

    static void initTypeProvider( sal_Int16 nNodeType ) throw();

    const sal_Int16 mnNodeType;

    // for XTypeProvider
    static Sequence< Type >* mpTypes[12];

    // attributes for the XAnimationNode interface implementation
    Any maBegin, maDuration, maEnd, maEndSync, maRepeatCount, maRepeatDuration;
    sal_Int16 mnFill, mnFillDefault, mnRestart, mnRestartDefault;
    double mfAcceleration, mfDecelerate;
    bool mbAutoReverse;
    Sequence< NamedValue > maUserData;

    // parent interface for XChild interface implementation
    WeakReference<XInterface>   mxParent;
    AnimationNode*          mpParent;

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

    // attributes for XAnimateTransform
    sal_Int16 mnTransformType;

    // attributes for XTransitionFilter
    sal_Int16 mnTransition;
    sal_Int16 mnSubtype;
    bool mbMode;
    sal_Int32 mnFadeColor;

    // XAudio
    double mfVolume;

    // XCommand
    sal_Int16 mnCommand;
    Any maParameter;

    // XIterateContainer
    sal_Int16 mnIterateType;
    double  mfIterateInterval;

    /** sorted list of child nodes for XTimeContainer*/
    ChildList_t             maChildren;
};



class TimeContainerEnumeration : public ::cppu::WeakImplHelper< XEnumeration >
{
public:
    explicit TimeContainerEnumeration( const ChildList_t &rChildren );
    virtual ~TimeContainerEnumeration();

    // Methods
    virtual sal_Bool SAL_CALL hasMoreElements() throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL nextElement(  ) throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception) override;

private:
    /** sorted list of child nodes */
    ChildList_t             maChildren;

    /** current iteration position */
    ChildList_t::iterator   maIter;

    /** our first, last and only protection from mutli-threads! */
    Mutex                   maMutex;
};

TimeContainerEnumeration::TimeContainerEnumeration( const ChildList_t &rChildren )
: maChildren( rChildren )
{
    maIter = maChildren.begin();
}

TimeContainerEnumeration::~TimeContainerEnumeration()
{
}

// Methods
sal_Bool SAL_CALL TimeContainerEnumeration::hasMoreElements() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    return maIter != maChildren.end();
}

Any SAL_CALL TimeContainerEnumeration::nextElement()
    throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( maIter == maChildren.end() )
        throw NoSuchElementException();

    return makeAny( (*maIter++) );
}



Sequence< Type >* AnimationNode::mpTypes[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

AnimationNode::AnimationNode( sal_Int16 nNodeType )
:   maChangeListener(maMutex),
    mnNodeType( nNodeType ),
    mnFill( AnimationFill::DEFAULT ),
    mnFillDefault( AnimationFill::INHERIT ),
    mnRestart( AnimationRestart:: DEFAULT ),
    mnRestartDefault( AnimationRestart:: INHERIT ),
    mfAcceleration( 0.0 ),
    mfDecelerate( 0.0 ),
    mbAutoReverse( false ),
    mpParent(nullptr),
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
    mnCommand(0),
    mnIterateType( css::presentation::ShapeAnimationSubType::AS_WHOLE ),
    mfIterateInterval(0.0)
{
    OSL_ENSURE((sal_uInt32)nNodeType < sizeof(mpTypes)/sizeof(Sequence<Type>*), "NodeType out of range");
}

AnimationNode::AnimationNode( const AnimationNode& rNode )
:   AnimationNodeBase(),
    maChangeListener(maMutex),
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
    mpParent(nullptr),

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

    // attributes for XAnimateTransform
    mnTransformType( rNode.mnTransformType ),

    // attributes for XTransitionFilter
    mnTransition( rNode.mnTransition ),
    mnSubtype( rNode.mnSubtype ),
    mbMode( rNode.mbMode ),
    mnFadeColor( rNode.mnFadeColor ),

    // XAudio
    mfVolume( rNode.mfVolume ),

    // XCommand
    mnCommand( rNode.mnCommand ),
    maParameter( rNode.maParameter ),

    // XIterateContainer
    mnIterateType( rNode.mnIterateType ),
    mfIterateInterval( rNode.mfIterateInterval )
{
}

AnimationNode::~AnimationNode()
{
}

Sequence<OUString> getSupportedServiceNames_PAR()
{
    Sequence<OUString> aRet { "com.sun.star.animations.ParallelTimeContainer" };
    return aRet;
}

OUString getImplementationName_PAR()
{
    return OUString( "animcore::ParallelTimeContainer" );
}

Sequence<OUString> getSupportedServiceNames_SEQ()
{
    Sequence<OUString> aRet { "com.sun.star.animations.SequenceTimeContainer" };
    return aRet;
}

OUString getImplementationName_SEQ()
{
    return OUString( "animcore::SequenceTimeContainer" );
}

Sequence<OUString> getSupportedServiceNames_ITERATE()
{
    Sequence<OUString> aRet { "com.sun.star.animations.IterateContainer" };
    return aRet;
}

OUString getImplementationName_ITERATE()
{
    return OUString( "animcore::IterateContainer" );
}

Sequence<OUString> getSupportedServiceNames_ANIMATE()
{
    Sequence<OUString> aRet { "com.sun.star.animations.Animate" };
    return aRet;
}

OUString getImplementationName_ANIMATE()
{
        return OUString( "animcore::Animate" );
}

Sequence<OUString> getSupportedServiceNames_SET()
{
    Sequence<OUString> aRet { "com.sun.star.animations.AnimateSet" };
    return aRet;
}

OUString getImplementationName_SET()
{
    return OUString( "animcore::AnimateSet" );
}

Sequence<OUString> getSupportedServiceNames_ANIMATECOLOR()
{
    Sequence<OUString> aRet { "com.sun.star.animations.AnimateColor" };
    return aRet;
}

OUString getImplementationName_ANIMATECOLOR()
{
    return OUString( "animcore::AnimateColor" );
}

Sequence<OUString> getSupportedServiceNames_ANIMATEMOTION()
{
    Sequence<OUString> aRet { "com.sun.star.animations.AnimateMotion" };
    return aRet;
}

OUString getImplementationName_ANIMATEMOTION()
{
    return OUString( "animcore::AnimateMotion" );
}

Sequence<OUString> getSupportedServiceNames_ANIMATETRANSFORM()
{
    Sequence<OUString> aRet { "com.sun.star.animations.AnimateTransform" };
    return aRet;
}

OUString getImplementationName_ANIMATETRANSFORM()
{
    return OUString( "animcore::AnimateTransform" );
}

Sequence<OUString> getSupportedServiceNames_TRANSITIONFILTER()
{
    Sequence<OUString> aRet { "com.sun.star.animations.TransitionFilter" };
    return aRet;
}

OUString getImplementationName_TRANSITIONFILTER()
{
        return OUString( "animcore::TransitionFilter" );
}

Sequence<OUString> getSupportedServiceNames_AUDIO()
{
    Sequence<OUString> aRet { "com.sun.star.animations.Audio" };
    return aRet;
}

OUString getImplementationName_AUDIO()
{
        return OUString( "animcore::Audio" );
}

Sequence<OUString> getSupportedServiceNames_COMMAND()
{
    Sequence<OUString> aRet { "com.sun.star.animations.Command" };
    return aRet;
}

OUString getImplementationName_COMMAND()
{
    return OUString( "animcore::Command" );
}

// XInterface
Any SAL_CALL AnimationNode::queryInterface( const Type& aType ) throw (RuntimeException, std::exception)
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
        static_cast< XChangesNotifier* >( this ),
        static_cast< XUnoTunnel* >( this ) ) );

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
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XAudio * >( static_cast< XAudio * >(this) ) );
            break;
        case AnimationNodeType::COMMAND:
            aRet = ::cppu::queryInterface(
                aType,
                static_cast< XCommand * >( static_cast< XCommand * >(this) ) );
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



void AnimationNode::initTypeProvider( sal_Int16 nNodeType ) throw()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

    if(! mpTypes[nNodeType] )
    {
        static const sal_Int32 type_numbers[] =
        {
            7, // CUSTOM
            9, // PAR
            9, // SEQ
            9, // ITERATE
            8, // ANIMATE
            8, // SET
            8, // ANIMATEMOTION
            8, // ANIMATECOLOR
            8, // ANIMATETRANSFORM
            8, // TRANSITIONFILTER
            8, // AUDIO
            8, // COMMAND
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
        pTypeAr[nPos++] = cppu::UnoType<XUnoTunnel>::get();
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
}



Sequence< Type > AnimationNode::getTypes() throw (RuntimeException, std::exception)
{
    if (! mpTypes[mnNodeType])
        initTypeProvider(mnNodeType);
    return *mpTypes[mnNodeType];
}


Sequence< sal_Int8 > AnimationNode::getImplementationId() throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}



// XInterface
void SAL_CALL AnimationNode::acquire(  ) throw ()
{
    OWeakObject::acquire();
}



// XInterface
void SAL_CALL AnimationNode::release(  ) throw ()
{
    OWeakObject::release();
}



// XServiceInfo
OUString AnimationNode::getImplementationName() throw(std::exception)
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
sal_Bool AnimationNode::supportsService(const OUString& ServiceName) throw(std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > AnimationNode::getSupportedServiceNames() throw(std::exception)
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
sal_Int16 SAL_CALL AnimationNode::getType() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnNodeType;
}



// XAnimationNode
Any SAL_CALL AnimationNode::getBegin() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maBegin;
}



// XAnimationNode
void SAL_CALL AnimationNode::setBegin( const Any& _begin ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _begin != maBegin )
    {
        maBegin = _begin;
        fireChangeListener();
    }
}



// XAnimationNode
Any SAL_CALL AnimationNode::getDuration() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maDuration;
}



// XAnimationNode
void SAL_CALL AnimationNode::setDuration( const Any& _duration ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _duration != maDuration )
    {
        maDuration = _duration;
        fireChangeListener();
    }
}



// XAnimationNode
Any SAL_CALL AnimationNode::getEnd() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maEnd;
}



// XAnimationNode
void SAL_CALL AnimationNode::setEnd( const Any& _end ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _end != maEnd )
    {
        maEnd = _end;
        fireChangeListener();
    }
}



// XAnimationNode
Any SAL_CALL AnimationNode::getEndSync() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maEndSync;
}



// XAnimationNode
void SAL_CALL AnimationNode::setEndSync( const Any& _endsync ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _endsync != maEndSync )
    {
        maEndSync = _endsync;
        fireChangeListener();
    }
}



// XAnimationNode
Any SAL_CALL AnimationNode::getRepeatCount() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maRepeatCount;
}



// XAnimationNode
void SAL_CALL AnimationNode::setRepeatCount( const Any& _repeatcount ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _repeatcount != maRepeatCount )
    {
        maRepeatCount = _repeatcount;
        fireChangeListener();
    }
}



// XAnimationNode
Any SAL_CALL AnimationNode::getRepeatDuration() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maRepeatDuration;
}



// XAnimationNode
void SAL_CALL AnimationNode::setRepeatDuration( const Any& _repeatduration ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _repeatduration != maRepeatDuration )
    {
        maRepeatDuration = _repeatduration;
        fireChangeListener();
    }
}



// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getFill() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnFill;
}



// XAnimationNode
void SAL_CALL AnimationNode::setFill( sal_Int16 _fill ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _fill != mnFill )
    {
        mnFill = _fill;
        fireChangeListener();
    }
}



// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getFillDefault() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnFillDefault;
}



// XAnimationNode
void SAL_CALL AnimationNode::setFillDefault( sal_Int16 _filldefault ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _filldefault != mnFillDefault )
    {
        mnFillDefault = _filldefault;
        fireChangeListener();
    }
}



// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getRestart() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnRestart;
}



// XAnimationNode
void SAL_CALL AnimationNode::setRestart( sal_Int16 _restart ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _restart != mnRestart )
    {
        mnRestart = _restart;
        fireChangeListener();
    }
}



// XAnimationNode
sal_Int16 SAL_CALL AnimationNode::getRestartDefault() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnRestartDefault;
}



// XAnimationNode
void SAL_CALL AnimationNode::setRestartDefault( sal_Int16 _restartdefault ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _restartdefault != mnRestartDefault )
    {
        mnRestartDefault = _restartdefault;
        fireChangeListener();
    }
}



// XAnimationNode
double SAL_CALL AnimationNode::getAcceleration() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mfAcceleration;
}



// XAnimationNode
void SAL_CALL AnimationNode::setAcceleration( double _acceleration ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _acceleration != mfAcceleration )
    {
        mfAcceleration = _acceleration;
        fireChangeListener();
    }
}



// XAnimationNode
double SAL_CALL AnimationNode::getDecelerate() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mfDecelerate;
}



// XAnimationNode
void SAL_CALL AnimationNode::setDecelerate( double _decelerate ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _decelerate != mfDecelerate )
    {
        mfDecelerate = _decelerate;
        fireChangeListener();
    }
}



// XAnimationNode
sal_Bool SAL_CALL AnimationNode::getAutoReverse() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mbAutoReverse;
}



// XAnimationNode
void SAL_CALL AnimationNode::setAutoReverse( sal_Bool _autoreverse ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( bool(_autoreverse) != mbAutoReverse )
    {
        mbAutoReverse = _autoreverse;
        fireChangeListener();
    }
}



Sequence< NamedValue > SAL_CALL AnimationNode::getUserData() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maUserData;
}



void SAL_CALL AnimationNode::setUserData( const Sequence< NamedValue >& _userdata ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maUserData = _userdata;
    fireChangeListener();
}



// XChild
Reference< XInterface > SAL_CALL AnimationNode::getParent() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mxParent.get();
}



// XChild
void SAL_CALL AnimationNode::setParent( const Reference< XInterface >& Parent ) throw (NoSupportException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( Parent != mxParent.get() )
    {
        mxParent = Parent;

        mpParent = nullptr;
        Reference< XUnoTunnel > xTunnel( mxParent.get(), UNO_QUERY );
        if( xTunnel.is() )
            mpParent = reinterpret_cast< AnimationNode* >( sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething( getUnoTunnelId() )));

        fireChangeListener();
    }
}



// XCloneable
Reference< XCloneable > SAL_CALL AnimationNode::createClone() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    Reference< XCloneable > xNewNode;
    try
    {
        xNewNode = new AnimationNode( *this );

        if( !maChildren.empty() )
        {
            Reference< XTimeContainer > xContainer( xNewNode, UNO_QUERY );
            if( xContainer.is() )
            {
                ChildList_t::iterator aIter( maChildren.begin() );
                ChildList_t::iterator aEnd( maChildren.end() );
                while( aIter != aEnd )
                {
                    Reference< XCloneable > xCloneable((*aIter++), UNO_QUERY );
                    if( xCloneable.is() ) try
                    {
                        Reference< XAnimationNode > xNewChildNode( xCloneable->createClone(), UNO_QUERY );
                        if( xNewChildNode.is() )
                            xContainer->appendChild( xNewChildNode );
                    }
                    catch(const Exception&)
                    {
                        OSL_TRACE( "animations::AnimationNode::createClone(), exception caught!" );
                    }
                }
            }
        }
    }
    catch(const Exception&)
    {
        OSL_TRACE( "animations::AnimationNode::createClone(), exception caught!" );
    }

    return xNewNode;
}



// XAnimate
Any SAL_CALL AnimationNode::getTarget()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maTarget;
}



// XAnimate
void SAL_CALL AnimationNode::setTarget( const Any& _target )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _target != maTarget )
    {
        maTarget= _target;
        fireChangeListener();
    }
}



// XAnimate
OUString SAL_CALL AnimationNode::getAttributeName() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maAttributeName;
}



// XAnimate
void SAL_CALL AnimationNode::setAttributeName( const OUString& _attribute )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _attribute != maAttributeName )
    {
        maAttributeName = _attribute;
        fireChangeListener();
    }
}



// XAnimate
Sequence< Any > SAL_CALL AnimationNode::getValues()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maValues;
}



// XAnimate
void SAL_CALL AnimationNode::setValues( const Sequence< Any >& _values )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maValues = _values;
    fireChangeListener();
}



// XAnimate
sal_Int16 SAL_CALL AnimationNode::getSubItem() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnSubItem;
}



// XAnimate
void SAL_CALL AnimationNode::setSubItem( sal_Int16 _subitem ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _subitem != mnSubItem )
    {
        mnSubItem = _subitem;
        fireChangeListener();
    }
}



// XAnimate
Sequence< double > SAL_CALL AnimationNode::getKeyTimes() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maKeyTimes;
}



// XAnimate
void SAL_CALL AnimationNode::setKeyTimes( const Sequence< double >& _keytimes ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maKeyTimes = _keytimes;
    fireChangeListener();
}



// XAnimate
sal_Int16 SAL_CALL AnimationNode::getValueType() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnValueType;
}



void SAL_CALL AnimationNode::setValueType( sal_Int16 _valuetype ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _valuetype != mnValueType )
    {
        mnValueType = _valuetype;
        fireChangeListener();
    }
}



// XAnimate
sal_Int16 SAL_CALL AnimationNode::getCalcMode()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnCalcMode;
}



// XAnimate
void SAL_CALL AnimationNode::setCalcMode( sal_Int16 _calcmode )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _calcmode != mnCalcMode )
    {
        mnCalcMode = _calcmode;
        fireChangeListener();
    }
}



// XAnimate
sal_Bool SAL_CALL AnimationNode::getAccumulate()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mbAccumulate;
}



// XAnimate
void SAL_CALL AnimationNode::setAccumulate( sal_Bool _accumulate )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( bool(_accumulate) != mbAccumulate )
    {
        mbAccumulate = _accumulate;
        fireChangeListener();
    }
}



// XAnimate
sal_Int16 SAL_CALL AnimationNode::getAdditive()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnAdditive;
}



// XAnimate
void SAL_CALL AnimationNode::setAdditive( sal_Int16 _additive )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _additive != mnAdditive )
    {
        mnAdditive = _additive;
        fireChangeListener();
    }
}



// XAnimate
Any SAL_CALL AnimationNode::getFrom()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maFrom;
}



// XAnimate
void SAL_CALL AnimationNode::setFrom( const Any& _from )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _from != maFrom )
    {
        maFrom = _from;
        fireChangeListener();
    }
}



// XAnimate
Any SAL_CALL AnimationNode::getTo()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maTo;
}



// XAnimate
void SAL_CALL AnimationNode::setTo( const Any& _to )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _to != maTo )
    {
        maTo = _to;
        fireChangeListener();
    }
}



// XAnimate
Any SAL_CALL AnimationNode::getBy()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maBy;
}



// XAnimate
void SAL_CALL AnimationNode::setBy( const Any& _by )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _by != maBy )
    {
        maBy = _by;
        fireChangeListener();
    }
}



// XAnimate
Sequence< TimeFilterPair > SAL_CALL AnimationNode::getTimeFilter()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maTimeFilter;
}



// XAnimate
void SAL_CALL AnimationNode::setTimeFilter( const Sequence< TimeFilterPair >& _timefilter )
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maTimeFilter = _timefilter;
    fireChangeListener();
}



OUString SAL_CALL AnimationNode::getFormula() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maFormula;
}



void SAL_CALL AnimationNode::setFormula( const OUString& _formula ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _formula != maFormula )
    {
        maFormula = _formula;
        fireChangeListener();
    }
}



// XAnimateColor
sal_Int16 SAL_CALL AnimationNode::getColorInterpolation() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnColorSpace;
}



// XAnimateColor
void SAL_CALL AnimationNode::setColorInterpolation( sal_Int16 _colorspace ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _colorspace != mnColorSpace )
    {
        mnColorSpace = _colorspace;
        fireChangeListener();
    }
}



// XAnimateColor
sal_Bool SAL_CALL AnimationNode::getDirection() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mbDirection;
}



// XAnimateColor
void SAL_CALL AnimationNode::setDirection( sal_Bool _direction ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( bool(_direction) != mbDirection )
    {
        mbDirection = _direction;
        fireChangeListener();
    }
}



// XAnimateMotion
Any SAL_CALL AnimationNode::getPath() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maPath;
}



// XAnimateMotion
void SAL_CALL AnimationNode::setPath( const Any& _path ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maPath = _path;
    fireChangeListener();
}



// XAnimateMotion
Any SAL_CALL AnimationNode::getOrigin() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maOrigin;
}



// XAnimateMotion
void SAL_CALL AnimationNode::setOrigin( const Any& _origin ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maOrigin = _origin;
    fireChangeListener();
}



// XAnimateTransform
sal_Int16 SAL_CALL AnimationNode::getTransformType() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnTransformType;
}



// XAnimateTransform
void SAL_CALL AnimationNode::setTransformType( sal_Int16 _transformtype ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _transformtype != mnTransformType )
    {
        mnTransformType = _transformtype;
        fireChangeListener();
    }
}



// XTransitionFilter
sal_Int16 SAL_CALL AnimationNode::getTransition() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnTransition;
}



// XTransitionFilter
void SAL_CALL AnimationNode::setTransition( sal_Int16 _transition ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _transition != mnTransition )
    {
        mnTransition = _transition;
        fireChangeListener();
    }
}



// XTransitionFilter
sal_Int16 SAL_CALL AnimationNode::getSubtype() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnSubtype;
}



// XTransitionFilter
void SAL_CALL AnimationNode::setSubtype( sal_Int16 _subtype ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _subtype != mnSubtype )
    {
        mnSubtype = _subtype;
        fireChangeListener();
    }
}



// XTransitionFilter
sal_Bool SAL_CALL AnimationNode::getMode() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mbMode;
}



// XTransitionFilter
void SAL_CALL AnimationNode::setMode( sal_Bool _mode ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( bool(_mode) != mbMode )
    {
        mbMode = _mode;
        fireChangeListener();
    }
}



// XTransitionFilter
sal_Int32 SAL_CALL AnimationNode::getFadeColor() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnFadeColor;
}



// XTransitionFilter
void SAL_CALL AnimationNode::setFadeColor( sal_Int32 _fadecolor ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _fadecolor != mnFadeColor )
    {
        mnFadeColor = _fadecolor;
        fireChangeListener();
    }
}



// XAudio
Any SAL_CALL AnimationNode::getSource() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maTarget;
}



// XAudio
void SAL_CALL AnimationNode::setSource( const Any& _source ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maTarget = _source;
    fireChangeListener();
}



// XAudio
double SAL_CALL AnimationNode::getVolume() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mfVolume;
}



// XAudio
void SAL_CALL AnimationNode::setVolume( double _volume ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _volume != mfVolume )
    {
        mfVolume = _volume;
        fireChangeListener();
    }
}



// XCommand
sal_Int16 SAL_CALL AnimationNode::getCommand() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnCommand;
}



// XCommand
void SAL_CALL AnimationNode::setCommand( sal_Int16 _command ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _command != mnCommand )
    {
        mnCommand = _command;
        fireChangeListener();
    }
}



// XCommand
Any SAL_CALL AnimationNode::getParameter() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maParameter;
}



// XCommand
void SAL_CALL AnimationNode::setParameter( const Any& _parameter ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maParameter = _parameter;
    fireChangeListener();
}



// XElementAccess
Type SAL_CALL AnimationNode::getElementType() throw (RuntimeException, std::exception)
{
    return cppu::UnoType<XAnimationNode>::get();
}



// XElementAccess
sal_Bool SAL_CALL AnimationNode::hasElements() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return !maChildren.empty();
}



// XEnumerationAccess
Reference< XEnumeration > SAL_CALL AnimationNode::createEnumeration()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    return new TimeContainerEnumeration( maChildren);
}




// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild )
    throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( !newChild.is() || !refChild.is() )
        throw IllegalArgumentException();

    ChildList_t::iterator before = ::std::find(maChildren.begin(), maChildren.end(), refChild);
    if( before == maChildren.end() )
        throw NoSuchElementException();

    if( ::std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    maChildren.insert( before, newChild );

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    newChild->setParent( xThis );

    return newChild;
}



// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild )
    throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( !newChild.is() || !refChild.is() )
        throw IllegalArgumentException();

    ChildList_t::iterator before = ::std::find(maChildren.begin(), maChildren.end(), refChild);
    if( before == maChildren.end() )
        throw NoSuchElementException();

    if( ::std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    ++before;
    if( before != maChildren.end() )
        maChildren.insert( before, newChild );
    else
        maChildren.push_back( newChild );

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    newChild->setParent( xThis );

    return newChild;
}



// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild )
    throw( IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( !newChild.is() || !oldChild.is() )
        throw IllegalArgumentException();

    ChildList_t::iterator replace = ::std::find(maChildren.begin(), maChildren.end(), oldChild);
    if( replace == maChildren.end() )
        throw NoSuchElementException();

    if( ::std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    Reference< XInterface > xNull( nullptr );
    oldChild->setParent( xNull );

    (*replace) = newChild;

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    newChild->setParent( xThis );

    return newChild;
}



// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::removeChild( const Reference< XAnimationNode >& oldChild )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( !oldChild.is() )
        throw IllegalArgumentException();

    ChildList_t::iterator old = ::std::find(maChildren.begin(), maChildren.end(), oldChild);
    if( old == maChildren.end() )
        throw NoSuchElementException();

    Reference< XInterface > xNull( nullptr );
    oldChild->setParent( xNull );

    maChildren.erase( old );

    return oldChild;
}



// XTimeContainer
Reference< XAnimationNode > SAL_CALL AnimationNode::appendChild( const Reference< XAnimationNode >& newChild )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( !newChild.is() )
        throw IllegalArgumentException();

    if( ::std::find(maChildren.begin(), maChildren.end(), newChild) != maChildren.end() )
        throw ElementExistException();

    Reference< XInterface > xThis( static_cast< OWeakObject * >(this) );
    Reference< XInterface > xChild( newChild );

    if( xThis == xChild )
        throw IllegalArgumentException();

    maChildren.push_back( newChild );

    newChild->setParent( xThis );

    return newChild;
}



// XIterateContainer
sal_Int16 SAL_CALL AnimationNode::getIterateType() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnIterateType;
}



// XIterateContainer
void SAL_CALL AnimationNode::setIterateType( sal_Int16 _iteratetype ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _iteratetype != mnIterateType )
    {
        mnIterateType = _iteratetype;
        fireChangeListener();
    }
}



// XIterateContainer
double SAL_CALL AnimationNode::getIterateInterval() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mfIterateInterval;
}



// XIterateContainer
void SAL_CALL AnimationNode::setIterateInterval( double _iterateinterval ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    if( _iterateinterval != mfIterateInterval )
    {
        mfIterateInterval = _iterateinterval;
        fireChangeListener();
    }
}



// XChangesNotifier
void SAL_CALL AnimationNode::addChangesListener( const Reference< XChangesListener >& aListener ) throw (RuntimeException, std::exception)
{
    maChangeListener.addInterface( aListener );
}



// XChangesNotifier
void SAL_CALL AnimationNode::removeChangesListener( const Reference< XChangesListener >& aListener ) throw (RuntimeException, std::exception)
{
    maChangeListener.removeInterface(aListener);
}



// XUnoTunnel
::sal_Int64 SAL_CALL AnimationNode::getSomething( const Sequence< ::sal_Int8 >& rId ) throw (RuntimeException, std::exception)
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(this));

    }
    else
    {
        return 0;
    }
}

namespace
{
    class theAnimationNodeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theAnimationNodeUnoTunnelId > {};
}

const css::uno::Sequence< sal_Int8 > & AnimationNode::getUnoTunnelId()
{
    return theAnimationNodeUnoTunnelId::get().getSeq();
}



void AnimationNode::fireChangeListener()
{
    Guard< Mutex > aGuard( maMutex );

    OInterfaceIteratorHelper aIterator( maChangeListener );
    if( aIterator.hasMoreElements() )
    {
        Reference< XInterface > xSource( static_cast<OWeakObject*>(this), UNO_QUERY );
        Sequence< ElementChange > aChanges;
        const ChangesEvent aEvent( xSource, makeAny( mxParent.get() ), aChanges );
        while( aIterator.hasMoreElements() )
        {
            Reference< XChangesListener > xListener( aIterator.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->changesOccurred( aEvent );
        }
    }

    //fdo#69645 use WeakReference of mxParent to test if mpParent is still valid
    if (mpParent)
    {
        Reference<XInterface> xGuard(mxParent);
        if (xGuard.is())
            mpParent->fireChangeListener();
    }
}



} // namespace animcore


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_ParallelTimeContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(PAR));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_SequenceTimeContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(SEQ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_IterateContainer_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ITERATE));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_Animate_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATE));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_AnimateSet_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(SET));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_AnimateColor_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATECOLOR));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_AnimateMotion_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATEMOTION));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_AnimateTransform_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(ANIMATETRANSFORM));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_TransitionFilter_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(TRANSITIONFILTER));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_Audio_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(AUDIO));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_animations_Command_get_implementation(css::uno::XComponentContext*,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new animcore::AnimationNode(COMMAND));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
