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

#include <comphelper/lok.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <svtools/javacontext.hxx>
#include <svtools/javainteractionhandler.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;
namespace svt
{

JavaContext::JavaContext( const Reference< XCurrentContext > & ctx)
    : m_aRefCount(0),
      m_xNextContext( ctx )
{
}

JavaContext::~JavaContext()
{
}

Any SAL_CALL JavaContext::queryInterface(const Type& aType )
{
    if (aType == cppu::UnoType<XInterface>::get())
        return Any(Reference<XInterface>(static_cast<XInterface*>(this)));
    else if (aType == cppu::UnoType<XCurrentContext>::get())
        return Any(Reference<XCurrentContext>( static_cast<XCurrentContext*>(this)));
    return Any();
}

void SAL_CALL JavaContext::acquire(  ) noexcept
{
    osl_atomic_increment( &m_aRefCount );
}

void SAL_CALL JavaContext::release(  ) noexcept
{
    if (! osl_atomic_decrement( &m_aRefCount ))
        delete this;
}

Any SAL_CALL JavaContext::getValueByName( const OUString& Name)
{
    Any retVal;

    if ( Name == JAVA_INTERACTION_HANDLER_NAME )
    {
        if ( !comphelper::LibreOfficeKit::isActive() )
        {
            osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
            if (!m_xHandler.is())
                m_xHandler.set( new JavaInteractionHandler );
        }
        retVal <<= m_xHandler;

    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
