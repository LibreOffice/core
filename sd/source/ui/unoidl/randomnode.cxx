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

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/ParallelTimeContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/processfactory.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include "CustomAnimationPreset.hxx"
#include "facreg.hxx"
#include "randomnode.hxx"

using ::osl::Mutex;
using ::osl::Guard;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::NoSupportException;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::ElementExistException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::lang::XInitialization;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::XWeak;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::presentation::ParagraphTarget;
using ::com::sun::star::drawing::XShape;

using namespace ::com::sun::star::animations;
namespace sd
{

typedef ::cppu::WeakImplHelper< XTimeContainer, XEnumerationAccess, XCloneable, XServiceInfo, XInitialization > RandomAnimationNodeBase;
class RandomAnimationNode : public RandomAnimationNodeBase
{
public:
    RandomAnimationNode( const RandomAnimationNode& rNode );
    explicit RandomAnimationNode( sal_Int16 nPresetClass );
    RandomAnimationNode();

    void init( sal_Int16 nPresetClass );

    // XInitialization
    void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException, std::exception) override;

    // XChild
    Reference< XInterface > SAL_CALL getParent(  ) throw (RuntimeException, std::exception) override;
    void SAL_CALL setParent( const Reference< XInterface >& Parent ) throw (NoSupportException, RuntimeException, std::exception) override;

    // XCloneable
    virtual Reference< XCloneable > SAL_CALL createClone() throw (RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw(std::exception) override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(std::exception) override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception) override;

    // XAnimationNode
    ::sal_Int16 SAL_CALL getType() throw (RuntimeException, std::exception) override;
    Any SAL_CALL getBegin() throw (RuntimeException, std::exception) override;
    void SAL_CALL setBegin( const Any& _begin ) throw (RuntimeException, std::exception) override;
    Any SAL_CALL getDuration() throw (RuntimeException, std::exception) override;
    void SAL_CALL setDuration( const Any& _duration ) throw (RuntimeException, std::exception) override;
    Any SAL_CALL getEnd() throw (RuntimeException, std::exception) override;
    void SAL_CALL setEnd( const Any& _end ) throw (RuntimeException, std::exception) override;
    Any SAL_CALL getEndSync() throw (RuntimeException, std::exception) override;
    void SAL_CALL setEndSync( const Any& _endsync ) throw (RuntimeException, std::exception) override;
    Any SAL_CALL getRepeatCount() throw (RuntimeException, std::exception) override;
    void SAL_CALL setRepeatCount( const Any& _repeatcount ) throw (RuntimeException, std::exception) override;
    Any SAL_CALL getRepeatDuration() throw (RuntimeException, std::exception) override;
    void SAL_CALL setRepeatDuration( const Any& _repeatduration ) throw (RuntimeException, std::exception) override;
    ::sal_Int16 SAL_CALL getFill() throw (RuntimeException, std::exception) override;
    void SAL_CALL setFill( ::sal_Int16 _fill ) throw (RuntimeException, std::exception) override;
    ::sal_Int16 SAL_CALL getFillDefault() throw (RuntimeException, std::exception) override;
    void SAL_CALL setFillDefault( ::sal_Int16 _filldefault ) throw (RuntimeException, std::exception) override;
    ::sal_Int16 SAL_CALL getRestart() throw (RuntimeException, std::exception) override;
    void SAL_CALL setRestart( ::sal_Int16 _restart ) throw (RuntimeException, std::exception) override;
    ::sal_Int16 SAL_CALL getRestartDefault() throw (RuntimeException, std::exception) override;
    void SAL_CALL setRestartDefault( ::sal_Int16 _restartdefault ) throw (RuntimeException, std::exception) override;
    double SAL_CALL getAcceleration() throw (RuntimeException, std::exception) override;
    void SAL_CALL setAcceleration( double _acceleration ) throw (RuntimeException, std::exception) override;
    double SAL_CALL getDecelerate() throw (RuntimeException, std::exception) override;
    void SAL_CALL setDecelerate( double _decelerate ) throw (RuntimeException, std::exception) override;
    sal_Bool SAL_CALL getAutoReverse() throw (RuntimeException, std::exception) override;
    void SAL_CALL setAutoReverse( sal_Bool _autoreverse ) throw (RuntimeException, std::exception) override;
    Sequence< NamedValue > SAL_CALL getUserData() throw (RuntimeException, std::exception) override;
    void SAL_CALL setUserData( const Sequence< NamedValue >& _userdata ) throw (RuntimeException, std::exception) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual Reference< XEnumeration > SAL_CALL createEnumeration() throw (RuntimeException, std::exception) override;

    // XTimeContainer
    Reference< XAnimationNode > SAL_CALL insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;
    Reference< XAnimationNode > SAL_CALL insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& refChild ) throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;
    Reference< XAnimationNode > SAL_CALL replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >& oldChild ) throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;
    Reference< XAnimationNode > SAL_CALL removeChild( const Reference< XAnimationNode >& oldChild ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception) override;
    Reference< XAnimationNode > SAL_CALL appendChild( const Reference< XAnimationNode >& newChild ) throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception) override;

private:
    // our first, last and only protection from mutli-threads!
    Mutex maMutex;

    sal_Int16 mnPresetClass;
    Reference< XInterface > mxParent;

    Any maBegin, maDuration, maEnd, maEndSync, maRepeatCount, maRepeatDuration, maTarget;
    sal_Int16 mnFill, mnFillDefault, mnRestart, mnRestartDefault;
    double mfAcceleration, mfDecelerate;
    bool mbAutoReverse;
    Sequence< NamedValue > maUserData;

    Reference< XAnimate > mxFirstNode;
};

Reference< XInterface > RandomAnimationNode_createInstance( sal_Int16 nPresetClass )
{
    Reference< XInterface > xInt( static_cast<XWeak*>( new RandomAnimationNode( nPresetClass ) ) );
    return xInt;
}

RandomAnimationNode::RandomAnimationNode( const RandomAnimationNode& rNode )
:   RandomAnimationNodeBase(),
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
{
    init( nPresetClass );
}

RandomAnimationNode::RandomAnimationNode()
{
    init( 1 );
}

void RandomAnimationNode::init( sal_Int16 nPresetClass )
{
    mnPresetClass = nPresetClass;
    mnFill = AnimationFill::DEFAULT;
    mnFillDefault = AnimationFill::INHERIT;
    mnRestart = AnimationRestart::DEFAULT;
    mnRestartDefault = AnimationRestart::INHERIT;
    mfAcceleration = 0.0;
    mfDecelerate = 0.0;
    mbAutoReverse = false;
}

// XInitialization
void SAL_CALL RandomAnimationNode::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException, std::exception)
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
sal_Int16 SAL_CALL RandomAnimationNode::getType() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return ::com::sun::star::animations::AnimationNodeType::PAR;
}

// XAnimationNode
Any SAL_CALL RandomAnimationNode::getBegin() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maBegin;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setBegin( const Any& _begin ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maBegin = _begin;
}

// XAnimationNode
Any SAL_CALL RandomAnimationNode::getDuration() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maDuration;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setDuration( const Any& _duration ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maDuration = _duration;
}

// XAnimationNode
Any SAL_CALL RandomAnimationNode::getEnd() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maEnd;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setEnd( const Any& _end ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maEnd = _end;
}

// XAnimationNode
Any SAL_CALL RandomAnimationNode::getEndSync() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maEndSync;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setEndSync( const Any& _endsync ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maEndSync = _endsync;
}

// XAnimationNode
Any SAL_CALL RandomAnimationNode::getRepeatCount() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maRepeatCount;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setRepeatCount( const Any& _repeatcount ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maRepeatCount = _repeatcount;
}

// XAnimationNode
Any SAL_CALL RandomAnimationNode::getRepeatDuration() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maRepeatDuration;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setRepeatDuration( const Any& _repeatduration ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maRepeatDuration = _repeatduration;
}

// XAnimationNode
sal_Int16 SAL_CALL RandomAnimationNode::getFill() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnFill;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setFill( sal_Int16 _fill ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mnFill = _fill;
}

// XAnimationNode
sal_Int16 SAL_CALL RandomAnimationNode::getFillDefault() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnFillDefault;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setFillDefault( sal_Int16 _filldefault ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mnFillDefault = _filldefault;
}

// XAnimationNode
sal_Int16 SAL_CALL RandomAnimationNode::getRestart() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnRestart;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setRestart( sal_Int16 _restart ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mnRestart = _restart;
}

// XAnimationNode
sal_Int16 SAL_CALL RandomAnimationNode::getRestartDefault() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mnRestartDefault;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setRestartDefault( sal_Int16 _restartdefault ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mnRestartDefault = _restartdefault;
}

// XAnimationNode
double SAL_CALL RandomAnimationNode::getAcceleration() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mfAcceleration;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setAcceleration( double _acceleration ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mfAcceleration = _acceleration;
}

// XAnimationNode
double SAL_CALL RandomAnimationNode::getDecelerate() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mfDecelerate;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setDecelerate( double _decelerate ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mfDecelerate = _decelerate;
}

// XAnimationNode
sal_Bool SAL_CALL RandomAnimationNode::getAutoReverse() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mbAutoReverse;
}

// XAnimationNode
void SAL_CALL RandomAnimationNode::setAutoReverse( sal_Bool _autoreverse ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mbAutoReverse = _autoreverse;
}

Sequence< NamedValue > SAL_CALL RandomAnimationNode::getUserData() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return maUserData;
}

void SAL_CALL RandomAnimationNode::setUserData( const Sequence< NamedValue >& _userdata ) throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    maUserData = _userdata;
}

// XChild
Reference< XInterface > SAL_CALL RandomAnimationNode::getParent() throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    return mxParent;
}

// XChild
void SAL_CALL RandomAnimationNode::setParent( const Reference< XInterface >& Parent ) throw (NoSupportException, RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );
    mxParent = Parent;
}

// XCloneable
Reference< XCloneable > SAL_CALL RandomAnimationNode::createClone() throw (RuntimeException, std::exception)
{
    Reference< XCloneable > xNewNode( new RandomAnimationNode( *this ) );
    return xNewNode;
}

// XElementAccess
Type SAL_CALL RandomAnimationNode::getElementType() throw (RuntimeException, std::exception)
{
    return cppu::UnoType<XAnimationNode>::get();
}

// XElementAccess
sal_Bool SAL_CALL RandomAnimationNode::hasElements() throw (RuntimeException, std::exception)
{
    return sal_True;
}

// XEnumerationAccess
Reference< XEnumeration > SAL_CALL RandomAnimationNode::createEnumeration()
    throw (RuntimeException, std::exception)
{
    Guard< Mutex > aGuard( maMutex );

    if( !maTarget.hasValue() && mxFirstNode.is() )
    {
        Any aTarget( mxFirstNode->getTarget() );
        if( aTarget.hasValue() )
        {
            maTarget = aTarget;
            mxFirstNode.clear();
        }
    }

    Reference< XEnumeration > xEnum;

    Reference< XEnumerationAccess > aEnumAccess( CustomAnimationPresets::getCustomAnimationPresets().getRandomPreset( mnPresetClass ), UNO_QUERY );

    if( aEnumAccess.is() )
    {
        Reference< XEnumeration > xEnumeration( aEnumAccess->createEnumeration(), UNO_QUERY );
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
Reference< XAnimationNode > SAL_CALL RandomAnimationNode::insertBefore( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >&  )
    throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    return appendChild( newChild );
}

// XTimeContainer
Reference< XAnimationNode > SAL_CALL RandomAnimationNode::insertAfter( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >&  )
    throw (IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    return appendChild( newChild );
}

// XTimeContainer
Reference< XAnimationNode > SAL_CALL RandomAnimationNode::replaceChild( const Reference< XAnimationNode >& newChild, const Reference< XAnimationNode >&  )
    throw( IllegalArgumentException, NoSuchElementException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    return appendChild( newChild );
}

// XTimeContainer
Reference< XAnimationNode > SAL_CALL RandomAnimationNode::removeChild( const Reference< XAnimationNode >& oldChild )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    return oldChild;
}

// XTimeContainer
Reference< XAnimationNode > SAL_CALL RandomAnimationNode::appendChild( const Reference< XAnimationNode >& newChild )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    Reference< XAnimate > xAnimate( newChild, UNO_QUERY );
    if( xAnimate.is() )
    {
        Any aTarget( xAnimate->getTarget() );
        if( aTarget.hasValue() )
            maTarget = aTarget;
    }

    if( !maTarget.hasValue() && !mxFirstNode.is() )
        mxFirstNode = xAnimate;

    return newChild;
}

// XServiceInfo
OUString RandomAnimationNode::getImplementationName() throw(std::exception)
{
    return OUString( "sd::RandomAnimationNode" ) ;
}

// XServiceInfo
sal_Bool RandomAnimationNode::supportsService(const OUString& ServiceName) throw(std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > RandomAnimationNode::getSupportedServiceNames() throw(std::exception)
{
    Sequence< OUString > aSeq( 2 );
    aSeq[0] = "com.sun.star.animations.ParallelTimeContainer";
    aSeq[1] = "com.sun.star.comp.sd.RandomAnimationNode";
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
RandomAnimationNode_get_implementation(::com::sun::star::uno::XComponentContext*,
                                                                    ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::RandomAnimationNode());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
