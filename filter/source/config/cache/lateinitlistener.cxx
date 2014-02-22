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

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "lateinitlistener.hxx"
#include "lateinitthread.hxx"

#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>


namespace filter{
    namespace config{

LateInitListener::LateInitListener(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : BaseLock(     )
{
    
    
    
    osl_atomic_increment( &m_refCount );

    m_xBroadcaster = css::uno::Reference< css::document::XEventBroadcaster >(
        css::frame::theGlobalEventBroadcaster::get(rxContext),
        css::uno::UNO_QUERY_THROW);

    m_xBroadcaster->addEventListener(static_cast< css::document::XEventListener* >(this));

    osl_atomic_decrement( &m_refCount );
}



LateInitListener::~LateInitListener()
{
}



void SAL_CALL LateInitListener::notifyEvent(const css::document::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    
    
    
    
    if ( aEvent.EventName == "OnNew" || aEvent.EventName == "OnLoad" || aEvent.EventName == "OnCloseApp" )
    {
        
        

        
        ::osl::ResettableMutexGuard aLock(m_aLock);

        if ( !m_xBroadcaster.is() )
            
            
            
            
            
            
            
            return;

        m_xBroadcaster->removeEventListener(static_cast< css::document::XEventListener* >(this));
        m_xBroadcaster.clear();

        aLock.clear();
        

        if ( aEvent.EventName != "OnCloseApp" )
        {
            rtl::Reference< LateInitThread >(new LateInitThread())->launch();
                
                
                
                
        }
    }
}



void SAL_CALL LateInitListener::disposing(const css::lang::EventObject& /* aEvent */ )
    throw(css::uno::RuntimeException)
{
    
    
    
    
    

    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if ( !m_xBroadcaster.is() )
        return;

    m_xBroadcaster->removeEventListener(static_cast< css::document::XEventListener* >(this));
    m_xBroadcaster.clear();
    aLock.clear();
    
}

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
