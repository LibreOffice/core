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

#include <framecontrol.hxx>
#include <OConnectionPointContainerHelper.hxx>

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>

//  namespaces

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace unocontrols {

namespace {

enum PropertyHandle  // values represent index in PropertyArray
{                   // for FrameControl
    Componenturl    = 0,
    Frame           = 1,
    Loaderarguments = 2
};

}

//  construct/destruct

FrameControl::FrameControl( const Reference< XComponentContext >& rxContext)
    : BaseControl                   ( rxContext                                     )
    , OBroadcastHelper              ( m_aMutex                                      )
    , OPropertySetHelper            ( *static_cast< OBroadcastHelper * >(this)      )
    , m_aConnectionPointContainer   ( new OConnectionPointContainerHelper(m_aMutex) )
{
}

FrameControl::~FrameControl()
{
}

//  XInterface
Any SAL_CALL FrameControl::queryInterface( const Type& rType )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by WeakComponentImplHelper!
    Any aReturn ( ::cppu::queryInterface(   rType                                               ,
                                               static_cast< XControlModel*              > ( this )  ,
                                               static_cast< XConnectionPointContainer*  > ( this )
                                        )
                );

    if (aReturn.hasValue())
        return aReturn;

    // If searched interface not supported by this class ...
    // ... ask baseclasses.
    aReturn = OPropertySetHelper::queryInterface(rType);
    if (aReturn.hasValue())
        return aReturn;
    return BaseControl::queryInterface(rType);
}

//  XInterface
void SAL_CALL FrameControl::acquire() noexcept
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//  XInterface
void SAL_CALL FrameControl::release() noexcept
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}

//  XTypeProvider

Sequence< Type > SAL_CALL FrameControl::getTypes()
{
    static OTypeCollection ourTypeCollection(
                cppu::UnoType<XControlModel>::get(),
                cppu::UnoType<XControlContainer>::get(),
                cppu::UnoType<XConnectionPointContainer>::get(),
                BaseControl::getTypes() );

    return ourTypeCollection.getTypes();
}

OUString FrameControl::getImplementationName()
{
    return u"stardiv.UnoControls.FrameControl"_ustr;
}

css::uno::Sequence<OUString> FrameControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.FrameControl"_ustr };
}

//  XControl

void SAL_CALL FrameControl::createPeer( const   Reference< XToolkit >&      xToolkit    ,
                                        const   Reference< XWindowPeer >&   xParentPeer )
{
    BaseControl::createPeer( xToolkit, xParentPeer );
    if ( impl_getPeerWindow().is() )
    {
        if( !m_sComponentURL.isEmpty() )
        {
            impl_createFrame( getPeer(), m_sComponentURL, m_seqLoaderArguments );
        }
    }
}

//  XControl

sal_Bool SAL_CALL FrameControl::setModel( const Reference< XControlModel >& /*xModel*/ )
{
    // We have no model.
    return false;
}

//  XControl

Reference< XControlModel > SAL_CALL FrameControl::getModel()
{
    // We have no model.
    return Reference< XControlModel >();
}

//  XControl

void SAL_CALL FrameControl::dispose()
{
    Reference< XFrame2 >  xOldFrame;
    {
        // do not dispose the frame in this guarded section (deadlock?)
        MutexGuard aGuard( m_aMutex );
        xOldFrame = std::move(m_xFrame);
    }

    // notify the listeners
    sal_Int32 nFrameId = PropertyHandle::Frame;
    Reference< XFrame2 >  xNullFrame;
    Any aNewFrame( &xNullFrame, cppu::UnoType<XFrame2>::get());
    Any aOldFrame( &xOldFrame, cppu::UnoType<XFrame2>::get());
    fire( &nFrameId, &aNewFrame, &aOldFrame, 1, false );

    // dispose the frame
    if( xOldFrame.is() )
        xOldFrame->dispose();

    m_aConnectionPointContainer.clear();
    BaseControl::dispose();
}

//  XView

sal_Bool SAL_CALL FrameControl::setGraphics( const Reference< XGraphics >& /*xDevice*/ )
{
    // it is not possible to print this control
    return false;
}

//  XView

Reference< XGraphics > SAL_CALL FrameControl::getGraphics()
{
    // when it's not possible to set graphics ! then it's possible to return null
    return Reference< XGraphics >();
}

//  XConnectionPointContainer

Sequence< Type > SAL_CALL FrameControl::getConnectionPointTypes()
{
    // Forwarded to helper class
    return m_aConnectionPointContainer->getConnectionPointTypes();
}

//  XConnectionPointContainer

Reference< XConnectionPoint > SAL_CALL FrameControl::queryConnectionPoint( const Type& aType )
{
    // Forwarded to helper class
    return m_aConnectionPointContainer->queryConnectionPoint( aType );
}

//  XConnectionPointContainer

void SAL_CALL FrameControl::advise( const   Type&                       aType       ,
                                    const   Reference< XInterface >&    xListener   )
{
    // Forwarded to helper class
    m_aConnectionPointContainer->advise( aType, xListener );
}

//  XConnectionPointContainer

void SAL_CALL FrameControl::unadvise(   const   Type&                       aType       ,
                                        const   Reference< XInterface >&    xListener   )
{
    // Forwarded to helper class
    m_aConnectionPointContainer->unadvise( aType, xListener );
}

//  OPropertySetHelper

sal_Bool FrameControl::convertFastPropertyValue(        Any&        rConvertedValue ,
                                                        Any&        rOldValue       ,
                                                        sal_Int32   nHandle         ,
                                                const   Any&        rValue          )
{
    bool bReturn = false;
    switch (nHandle)
    {
        case PropertyHandle::Componenturl        :      rConvertedValue   = rValue;
                                                        rOldValue       <<= m_sComponentURL;
                                                        bReturn           = true;
                                                        break;

        case PropertyHandle::Loaderarguments     :      rConvertedValue   = rValue;
                                                        rOldValue       <<= m_seqLoaderArguments;
                                                        bReturn           = true;
                                                        break;
    }

    if ( !bReturn )
    {
        throw IllegalArgumentException("unknown handle " + OUString::number(nHandle), getXWeak(), 1);
    }

    return bReturn;
}

//  OPropertySetHelper

void FrameControl::setFastPropertyValue_NoBroadcast(            sal_Int32   nHandle ,
                                                        const   Any&        rValue  )
{
    // this method only set the value
    MutexGuard  aGuard (m_aMutex);
    switch (nHandle)
    {
        case PropertyHandle::Componenturl        :      rValue >>= m_sComponentURL;
                                                        if (getPeer().is())
                                                        {
                                                            impl_createFrame ( getPeer(), m_sComponentURL, m_seqLoaderArguments );
                                                        }
                                                        break;

        case PropertyHandle::Loaderarguments     :      rValue >>= m_seqLoaderArguments;
                                                        break;

        default :                                       OSL_ENSURE ( nHandle == -1, "This is an invalid property handle." );
    }
}

//  OPropertySetHelper

void FrameControl::getFastPropertyValue(    Any&        rRet    ,
                                            sal_Int32   nHandle ) const
{
    MutexGuard  aGuard ( Mutex::getGlobalMutex() );

    switch (nHandle)
    {
        case PropertyHandle::Componenturl    :      rRet <<= m_sComponentURL;
                                                    break;

        case PropertyHandle::Loaderarguments :      rRet <<= m_seqLoaderArguments;
                                                    break;

        case PropertyHandle::Frame           :      rRet <<= m_xFrame;
                                                       break;

        default :                                   OSL_ENSURE ( nHandle == -1, "This is an invalid property handle." );
    }
}

//  OPropertySetHelper

IPropertyArrayHelper& FrameControl::getInfoHelper()
{
    // Create a table that map names to index values.
    // attention: properties need to be sorted by name!
    static OPropertyArrayHelper ourPropertyInfo(
                {
                    Property( u"ComponentUrl"_ustr, PropertyHandle::Componenturl, cppu::UnoType<OUString>::get(),
                            PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED ),
                    Property( u"Frame"_ustr, PropertyHandle::Frame, cppu::UnoType<XFrame>::get(),
                            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT ),
                    Property( u"LoaderArguments"_ustr, PropertyHandle::Loaderarguments, cppu::UnoType<Sequence<PropertyValue>>::get(),
                            PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED )
                },
                true );

    return ourPropertyInfo;
}

//  OPropertySetHelper

Reference< XPropertySetInfo > SAL_CALL FrameControl::getPropertySetInfo()
{
    // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
    // (Use method "getInfoHelper()".)
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );

    return xInfo;
}

//  BaseControl

WindowDescriptor FrameControl::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    WindowDescriptor aDescriptor;

    aDescriptor.Type               = WindowClass_CONTAINER;
    aDescriptor.ParentIndex        = -1;
    aDescriptor.Parent             = xParentPeer;
    aDescriptor.Bounds             = getPosSize ();
    aDescriptor.WindowAttributes   = 0;

    return aDescriptor;
}

//  private method

void FrameControl::impl_createFrame(    const   Reference< XWindowPeer >&   xPeer       ,
                                        const   OUString&                   rURL        ,
                                        const   Sequence< PropertyValue >&  rArguments  )
{
    Reference< XFrame2 >     xOldFrame;
    Reference< XFrame2 >     xNewFrame;

    {
        MutexGuard  aGuard ( m_aMutex );
        xOldFrame = m_xFrame;
    }

    xNewFrame = Frame::create( impl_getComponentContext() );

    Reference< XWindow >  xWP ( xPeer, UNO_QUERY );
    xNewFrame->initialize ( xWP );

    //  option
    //xFrame->setName( "WhatYouWant" );

    Reference< XURLTransformer > xTrans = URLTransformer::create( impl_getComponentContext() );
    // load file
    URL aURL;
    aURL.Complete = rURL;
    xTrans->parseStrict( aURL );

    Reference< XDispatch >  xDisp = xNewFrame->queryDispatch ( aURL, OUString (), FrameSearchFlag::SELF );
    if (xDisp.is())
    {
        xDisp->dispatch ( aURL, rArguments );
    }

    // set the frame
    {
        MutexGuard aGuard ( m_aMutex );
        m_xFrame = xNewFrame;
    }

    // notify the listeners
    sal_Int32   nFrameId = PropertyHandle::Frame;
    Any aNewFrame ( &xNewFrame, cppu::UnoType<XFrame>::get());
    Any aOldFrame ( &xOldFrame, cppu::UnoType<XFrame>::get());

    fire ( &nFrameId, &aNewFrame, &aOldFrame, 1, false );

    if (xOldFrame.is())
    {
        xOldFrame->dispose ();
    }
}

}   // namespace unocontrols

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
stardiv_UnoControls_FrameControl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new unocontrols::FrameControl(context));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
