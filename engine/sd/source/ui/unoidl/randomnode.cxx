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

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/ParallelTimeContainer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/processfactory.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>
#include <CustomAnimationPreset.hxx>
#include <randomnode.hxx>
#include <mutex>

using ::cpo::uno::Reference;
using ::cpo::uno::Sequence;
using ::cpo::uno::Any;
using ::cpo::uno::UNO_QUERY;
using ::cpo::uno::XInterface;
using ::cpo::uno::WeakReference;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::lang::XInitialization;
using ::cpo::uno::Type;
using ::cpo::uno::XWeak;
using ::com::sun::star::presentation::ParagraphTarget;
using ::com::sun::star::drawing::XShape;

using namespace ::com::sun::star::animations;

namespace sd
{

typedef ::cppu::WeakImplHelper< XTimeContainer, XEnumerationAccess, XCloneable, XServiceInfo, XInitialization > RandomAnimationNodeBase;

namespace {

class RandomAnimationNode : public RandomAnimationNodeBase
{
public:
    RandomAnimationNode( const RandomAnimationNode& rNode );
    explicit RandomAnimationNode( sal_Int16 nPresetClass );
    RandomAnimationNode();

    // XInitialization
    void initialize( const Sequence< Any >& aArguments ) override;

    // XChild
    Reference< XInterface > getParent(  ) override;
    void setParent( const Reference< XInterface >& Parent ) override;

    // XCloneable
    virtual Reference< XCloneable > createClone() override;

    // XServiceInfo
    OUString getImplementationName() override;
    Sequence< OUString > getSupportedServiceNames() override;
    bool supportsService(const OUString& ServiceName) override;

    // XAnimationNode
    ::sal_Int16 getType() override;
    Any getBegin() override;
    void setBegin( const Any& _begin ) override;
    Any getDuration() override;
    void setDuration( const Any& _duration ) override;
    Any getEnd() override;
    void setEnd( const Any& _end ) override;
    Any getEndSync() override;
    void setEndSync( const Any& _endsync ) override;
    Any getRepeatCount() override;
    void setRepeatCount( const Any& _repeatcount ) override;
    Any getRepeatDuration() override;
    void setRepeatDuration( const Any& _repeatduration ) override;
    ::sal_Int16 getFill() override;
    void setFill( ::sal_Int16 _fill ) override;
    ::sal_Int16 getFillDefault() override;
    void setFillDefault( ::sal_Int16 _filldefault ) override;
    ::sal_Int16 getRestart() override;
    void setRestart( ::sal_Int16 _restart ) override;
    ::sal_Int16 getRestartDefault() override;
    void setRestartDefault( ::sal_Int16 _restartdefault ) override;
    double getAcceleration() override;
    void setAcceleration( double _acceleration ) override;
    double getDecelerate() override;
    void setDecelerate( double _decelerate ) override;
    bool getAutoReverse() override;
    void setAutoReverse( bool _autoreverse ) override;
    Sequence< NamedValue > getUserData() override;
    void setUserData( const Sequence< NamedValue >& _userdata ) override;

    // XElementAccess
    virtual Type getElementType() override;
    virtual bool hasElements() override;

    // XEnumerationAccess
    virtual Reference< XEnumeration > createEnumeration() override;

    // XTimeContainer
    Reference< XAnimationNode > insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) override;
    Reference< XAnimationNode > insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) override;
    Reference< XAnimationNode > replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild ) override;
    Reference< XAnimationNode > removeChild( const Reference< XAnimationNode >& oldChild ) override;
    Reference< XAnimationNode > appendChild( const Reference< XAnimationNode >& newChild ) override;

private:
    // our first, last and only protection from multi-threads!
    std::mutex maMutex;

    sal_Int16 mnPresetClass;
    WeakReference<XInterface> mxParent;

    Any maBegin, maDuration, maEnd, maEndSync, maRepeatCount, maRepeatDuration, maTarget;
    sal_Int16 mnFill, mnFillDefault, mnRestart, mnRestartDefault;
    double mfAcceleration, mfDecelerate;
    bool mbAutoReverse;
    Sequence< NamedValue > maUserData;

    Reference< XAnimate > mxFirstNode;
};

}

Reference< XInterface > RandomAnimationNode_createInstance( sal_Int16 nPresetClass )
{
    Reference< XInterface > xInt( static_cast<XWeak*>( new RandomAnimationNode( nPresetClass ) ) );
    return xInt;
}

RandomAnimationNode::RandomAnimationNode( const RandomAnimationNode& rNode )
:   RandomAnimationNodeBase(rNode),
    mnPresetClass( rNode.mnPresetClass ),
    maBegin( rNode.maBegin ),
    maDuration( rNode.maDuration ),
    maEnd( rNode.maEnd ),
    maEndSync( rNode.maEndSync ),
    maRepeatCount( rNode.maRepeatCount ),
    maRepeatDuration( rNode.maRepeatDuration ),
    maTarget( rNode.maTarget ),
    mnFill( rNode.mnFill ),
    mnFillDefault( rNode.mnFillDefault ),
    mnRestart( rNode.mnRestart ),
    mnRestartDefault( rNode.mnRestartDefault ),
    mfAcceleration( rNode.mfAcceleration ),
    mfDecelerate( rNode.mfDecelerate ),
    mbAutoReverse( rNode.mbAutoReverse ),
    maUserData( rNode.maUserData )
{
}

RandomAnimationNode::RandomAnimationNode( sal_Int16 nPresetClass )
    : mnPresetClass(nPresetClass)
    , mnFill(AnimationFill::DEFAULT)
    , mnFillDefault(AnimationFill::INHERIT)
    , mnRestart(AnimationRestart::DEFAULT)
    , mnRestartDefault(AnimationRestart::INHERIT)
    , mfAcceleration(0.0)
    , mfDecelerate(0.0)
    , mbAutoReverse(false)
{
}

RandomAnimationNode::RandomAnimationNode()
    : RandomAnimationNode(1)
{
}

// XInitialization
void RandomAnimationNode::initialize( const Sequence< Any >& aArguments )
{
    if( aArguments.getLength() != 1 )
        throw IllegalArgumentException();

    if( aArguments[0].getValueType() == ::cppu::UnoType<sal_Int16>::get() )
    {
        aArguments[0] >>= mnPresetClass;
    }
    else if( aArguments[0].getValueType() != ::cppu::UnoType<ParagraphTarget>::get() )
    {
        Reference< XShape > xShape;
        aArguments[0] >>= xShape;
        if( !xShape.is() )
            throw IllegalArgumentException();
    }
    maTarget = aArguments[0];
}

// XAnimationNode
sal_Int16 RandomAnimationNode::getType()
{
    return css::animations::AnimationNodeType::PAR;
}

// XAnimationNode
Any RandomAnimationNode::getBegin()
{
    std::unique_lock aGuard( maMutex );
    return maBegin;
}

// XAnimationNode
void RandomAnimationNode::setBegin( const Any& _begin )
{
    std::unique_lock aGuard( maMutex );
    maBegin = _begin;
}

// XAnimationNode
Any RandomAnimationNode::getDuration()
{
    std::unique_lock aGuard( maMutex );
    return maDuration;
}

// XAnimationNode
void RandomAnimationNode::setDuration( const Any& _duration )
{
    std::unique_lock aGuard( maMutex );
    maDuration = _duration;
}

// XAnimationNode
Any RandomAnimationNode::getEnd()
{
    std::unique_lock aGuard( maMutex );
    return maEnd;
}

// XAnimationNode
void RandomAnimationNode::setEnd( const Any& _end )
{
    std::unique_lock aGuard( maMutex );
    maEnd = _end;
}

// XAnimationNode
Any RandomAnimationNode::getEndSync()
{
    std::unique_lock aGuard( maMutex );
    return maEndSync;
}

// XAnimationNode
void RandomAnimationNode::setEndSync( const Any& _endsync )
{
    std::unique_lock aGuard( maMutex );
    maEndSync = _endsync;
}

// XAnimationNode
Any RandomAnimationNode::getRepeatCount()
{
    std::unique_lock aGuard( maMutex );
    return maRepeatCount;
}

// XAnimationNode
void RandomAnimationNode::setRepeatCount( const Any& _repeatcount )
{
    std::unique_lock aGuard( maMutex );
    maRepeatCount = _repeatcount;
}

// XAnimationNode
Any RandomAnimationNode::getRepeatDuration()
{
    std::unique_lock aGuard( maMutex );
    return maRepeatDuration;
}

// XAnimationNode
void RandomAnimationNode::setRepeatDuration( const Any& _repeatduration )
{
    std::unique_lock aGuard( maMutex );
    maRepeatDuration = _repeatduration;
}

// XAnimationNode
sal_Int16 RandomAnimationNode::getFill()
{
    std::unique_lock aGuard( maMutex );
    return mnFill;
}

// XAnimationNode
void RandomAnimationNode::setFill( sal_Int16 _fill )
{
    std::unique_lock aGuard( maMutex );
    mnFill = _fill;
}

// XAnimationNode
sal_Int16 RandomAnimationNode::getFillDefault()
{
    std::unique_lock aGuard( maMutex );
    return mnFillDefault;
}

// XAnimationNode
void RandomAnimationNode::setFillDefault( sal_Int16 _filldefault )
{
    std::unique_lock aGuard( maMutex );
    mnFillDefault = _filldefault;
}

// XAnimationNode
sal_Int16 RandomAnimationNode::getRestart()
{
    std::unique_lock aGuard( maMutex );
    return mnRestart;
}

// XAnimationNode
void RandomAnimationNode::setRestart( sal_Int16 _restart )
{
    std::unique_lock aGuard( maMutex );
    mnRestart = _restart;
}

// XAnimationNode
sal_Int16 RandomAnimationNode::getRestartDefault()
{
    std::unique_lock aGuard( maMutex );
    return mnRestartDefault;
}

// XAnimationNode
void RandomAnimationNode::setRestartDefault( sal_Int16 _restartdefault )
{
    std::unique_lock aGuard( maMutex );
    mnRestartDefault = _restartdefault;
}

// XAnimationNode
double RandomAnimationNode::getAcceleration()
{
    std::unique_lock aGuard( maMutex );
    return mfAcceleration;
}

// XAnimationNode
void RandomAnimationNode::setAcceleration( double _acceleration )
{
    std::unique_lock aGuard( maMutex );
    mfAcceleration = _acceleration;
}

// XAnimationNode
double RandomAnimationNode::getDecelerate()
{
    std::unique_lock aGuard( maMutex );
    return mfDecelerate;
}

// XAnimationNode
void RandomAnimationNode::setDecelerate( double _decelerate )
{
    std::unique_lock aGuard( maMutex );
    mfDecelerate = _decelerate;
}

// XAnimationNode
bool RandomAnimationNode::getAutoReverse()
{
    std::unique_lock aGuard( maMutex );
    return mbAutoReverse;
}

// XAnimationNode
void RandomAnimationNode::setAutoReverse( bool _autoreverse )
{
    std::unique_lock aGuard( maMutex );
    mbAutoReverse = _autoreverse;
}

Sequence< NamedValue > RandomAnimationNode::getUserData()
{
    std::unique_lock aGuard( maMutex );
    return maUserData;
}

void RandomAnimationNode::setUserData( const Sequence< NamedValue >& _userdata )
{
    std::unique_lock aGuard( maMutex );
    maUserData = _userdata;
}

// XChild
Reference< XInterface > RandomAnimationNode::getParent()
{
    std::unique_lock aGuard( maMutex );
    return mxParent.get();
}

// XChild
void RandomAnimationNode::setParent( const Reference< XInterface >& Parent )
{
    std::unique_lock aGuard( maMutex );
    mxParent = Parent;
}

// XCloneable
Reference< XCloneable > RandomAnimationNode::createClone()
{
    std::unique_lock aGuard( maMutex );
    Reference< XCloneable > xNewNode( new RandomAnimationNode( *this ) );
    return xNewNode;
}

// XElementAccess
Type RandomAnimationNode::getElementType()
{
    return cppu::UnoType<XAnimationNode>::get();
}

// XElementAccess
bool RandomAnimationNode::hasElements()
{
    return true;
}

// XEnumerationAccess
Reference< XEnumeration > RandomAnimationNode::createEnumeration()
{
    std::unique_lock aGuard( maMutex );

    if( !maTarget.hasValue() && mxFirstNode.is() )
    {
        Any aTarget( mxFirstNode->getTarget() );
        if( aTarget.hasValue() )
        {
            maTarget = std::move(aTarget);
            mxFirstNode.clear();
        }
    }

    Reference< XEnumeration > xEnum;

    Reference< XEnumerationAccess > aEnumAccess( CustomAnimationPresets::getCustomAnimationPresets().getRandomPreset( mnPresetClass ), UNO_QUERY );

    if( aEnumAccess.is() )
    {
        Reference< XEnumeration > xEnumeration = aEnumAccess->createEnumeration();
        if( xEnumeration.is() )
        {
            while( xEnumeration->hasMoreElements() )
            {
                Reference< XAnimate > xAnimate( xEnumeration->nextElement(), UNO_QUERY );
                if( xAnimate.is() )
                    xAnimate->setTarget( maTarget );
            }
        }
        xEnum = aEnumAccess->createEnumeration();
    }
    else
    {
        // no presets? give empty node!
        Reference< XParallelTimeContainer > xTimeContainer  = ParallelTimeContainer::create( comphelper::getProcessComponentContext() );
        xEnum = xTimeContainer->createEnumeration();
    }

    return xEnum;
}

// XTimeContainer
Reference< XAnimationNode > RandomAnimationNode::insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >&  )
{
    return appendChild( newChild );
}

// XTimeContainer
Reference< XAnimationNode > RandomAnimationNode::insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >&  )
{
    return appendChild( newChild );
}

// XTimeContainer
Reference< XAnimationNode > RandomAnimationNode::replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >&  )
{
    return appendChild( newChild );
}

// XTimeContainer
Reference< XAnimationNode > RandomAnimationNode::removeChild( const Reference< XAnimationNode >& oldChild )
{
    return oldChild;
}

// XTimeContainer
Reference< XAnimationNode > RandomAnimationNode::appendChild( const Reference< XAnimationNode >& newChild )
{
    Reference< XAnimate > xAnimate( newChild, UNO_QUERY );
    if( xAnimate.is() )
    {
        Any aTarget( xAnimate->getTarget() );
        if( aTarget.hasValue() )
            maTarget = std::move(aTarget);
    }

    if( !maTarget.hasValue() && !mxFirstNode.is() )
        mxFirstNode = std::move(xAnimate);

    return newChild;
}

// XServiceInfo
OUString RandomAnimationNode::getImplementationName()
{
    return u"sd::RandomAnimationNode"_ustr ;
}

// XServiceInfo
bool RandomAnimationNode::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > RandomAnimationNode::getSupportedServiceNames()
{
    return { u"com.sun.star.animations.ParallelTimeContainer"_ustr, u"com.sun.star.comp.sd.RandomAnimationNode"_ustr };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
RandomAnimationNode_get_implementation(cpo::uno::XComponentContext*,
                                                                    cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new sd::RandomAnimationNode());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
