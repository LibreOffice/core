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

#include "ModifyListenerCallBack.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/compbase1.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart {

typedef ::cppu::WeakComponentImplHelper1<
        ::com::sun::star::util::XModifyListener >
    ModifyListenerCallBack_Base;

class ModifyListenerCallBack_impl
    : public ::chart::MutexContainer
    , public ModifyListenerCallBack_Base
{
public:
    explicit ModifyListenerCallBack_impl( const Link& rCallBack );
    virtual ~ModifyListenerCallBack_impl();

    void startListening( const Reference< util::XModifyBroadcaster >& xBroadcaster );
    void stopListening();

    
    virtual void SAL_CALL modified( const lang::EventObject& aEvent ) throw (uno::RuntimeException);

    
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) throw (uno::RuntimeException);

    using ::cppu::WeakComponentImplHelperBase::disposing;

private:
    Link m_aLink;
    Reference< util::XModifyBroadcaster > m_xBroadcaster;
};

ModifyListenerCallBack_impl::ModifyListenerCallBack_impl( const Link& rCallBack )
                        : ModifyListenerCallBack_Base( m_aMutex )
                        , m_aLink( rCallBack )
                        , m_xBroadcaster(0)
{
}

ModifyListenerCallBack_impl::~ModifyListenerCallBack_impl()
{
}


void SAL_CALL ModifyListenerCallBack_impl::modified( const lang::EventObject& /*aEvent*/ ) throw (uno::RuntimeException)
{
    m_aLink.Call(0);
}


void SAL_CALL ModifyListenerCallBack_impl::disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
{
    m_xBroadcaster.clear();
}

void ModifyListenerCallBack_impl::startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster )
{
    if( m_xBroadcaster == xBroadcaster )
        return;

    stopListening();
    m_xBroadcaster = xBroadcaster;
    if( m_xBroadcaster.is() )
        m_xBroadcaster->addModifyListener( this );
}
void ModifyListenerCallBack_impl::stopListening()
{
    if( m_xBroadcaster.is() )
    {
        m_xBroadcaster->removeModifyListener( this );
        m_xBroadcaster.clear();
    }
}

ModifyListenerCallBack::ModifyListenerCallBack( const Link& rCallBack )
                        : pModifyListener_impl( new ModifyListenerCallBack_impl(rCallBack) )
                        , m_xModifyListener( pModifyListener_impl )
{
}

ModifyListenerCallBack::~ModifyListenerCallBack()
{
    stopListening();
}

void ModifyListenerCallBack::startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster )
{
    pModifyListener_impl->startListening( xBroadcaster );
}
void ModifyListenerCallBack::stopListening()
{
    pModifyListener_impl->stopListening();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
