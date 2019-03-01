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

#include <helper/simplereferencecomponent.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

using com::sun::star::uno::RuntimeException;
using sd::SimpleReferenceComponent;

SimpleReferenceComponent::SimpleReferenceComponent()
: m_nCount(0)
, mbDisposed(false)
{
}

SimpleReferenceComponent::~SimpleReferenceComponent()
{
    OSL_ASSERT(m_nCount == 0);
    OSL_ASSERT(mbDisposed);
}

void SimpleReferenceComponent::acquire()
{
    osl_atomic_increment(&m_nCount);
}

void SimpleReferenceComponent::release()
{
    if((1 == m_nCount) && !mbDisposed)
    {
        try
        {
            Dispose();
        }
        catch (RuntimeException const &) // don't break throw ()
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "sd", exceptionToString(ex) );
        }
    }

    if(osl_atomic_decrement(&m_nCount) == 0) delete this;
}

void SimpleReferenceComponent::Dispose()
{
    if( !mbDisposed )
    {
        mbDisposed = true;
        disposing();
    }
}

void SimpleReferenceComponent::disposing()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
