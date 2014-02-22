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

#include "PresenterCurrentSlideObserver.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {



PresenterCurrentSlideObserver::PresenterCurrentSlideObserver (
    const ::rtl::Reference<PresenterController>& rxPresenterController,
    const Reference<presentation::XSlideShowController>& rxSlideShowController)
    : PresenterCurrentSlideObserverInterfaceBase(m_aMutex),
      mpPresenterController(rxPresenterController),
      mxSlideShowController(rxSlideShowController)
{
    if( mpPresenterController.is() )
    {
        mpPresenterController->addEventListener(this);
    }

    if( mxSlideShowController.is() )
    {
        
        mxSlideShowController->addSlideShowListener(static_cast<XSlideShowListener*>(this));
    }
}

PresenterCurrentSlideObserver::~PresenterCurrentSlideObserver (void)
{
}

void SAL_CALL PresenterCurrentSlideObserver::disposing (void)
{
    
    if(mxSlideShowController.is())
    {
        mxSlideShowController->removeSlideShowListener(static_cast<XSlideShowListener*>(this));
        mxSlideShowController = NULL;
    }
}



void SAL_CALL PresenterCurrentSlideObserver::beginEvent (
    const Reference<animations::XAnimationNode>& rNode)
    throw (css::uno::RuntimeException)
{
    (void)rNode;
}

void SAL_CALL PresenterCurrentSlideObserver::endEvent (
    const Reference<animations::XAnimationNode>& rNode)
    throw(css::uno::RuntimeException)
{
    (void)rNode;
}

void SAL_CALL PresenterCurrentSlideObserver::repeat (
    const css::uno::Reference<css::animations::XAnimationNode>& rNode,
    sal_Int32)
    throw (com::sun::star::uno::RuntimeException)
{
    (void)rNode;
}

void SAL_CALL PresenterCurrentSlideObserver::paused (void)
    throw (com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::resumed (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::slideEnded (sal_Bool bReverse)
    throw (css::uno::RuntimeException)
{
    
    
    
    if (mxSlideShowController.is() && !bReverse)
        if (mxSlideShowController->getNextSlideIndex() < 0)
            if( mpPresenterController.is() )
                mpPresenterController->UpdateCurrentSlide(+1);
}

void SAL_CALL PresenterCurrentSlideObserver::hyperLinkClicked (const OUString &)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::slideTransitionStarted (void)
    throw (css::uno::RuntimeException)
{
    if( mpPresenterController.is() )
        mpPresenterController->UpdateCurrentSlide(0);
}

void SAL_CALL PresenterCurrentSlideObserver::slideTransitionEnded (void)
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL PresenterCurrentSlideObserver::slideAnimationsEnded (void)
    throw (css::uno::RuntimeException)
{
}



void SAL_CALL PresenterCurrentSlideObserver::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == Reference<XInterface>(static_cast<XWeak*>(mpPresenterController.get())))
        dispose();
    else if (rEvent.Source == mxSlideShowController)
        mxSlideShowController = NULL;
}

} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
