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

#ifndef INCLUDED_COMPHELPER_BROADCASTHELPER_HXX
#define INCLUDED_COMPHELPER_BROADCASTHELPER_HXX

#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>

//... namespace comphelper .......................................................
namespace comphelper
{



    //= OMutexAndBroadcastHelper - a class which holds a Mutex and a OBroadcastHelper;
    //=                 needed because when deriving from OPropertySetHelper,
    //=                 the OBroadcastHelper has to be initialized before
    //=                 the OPropertySetHelper

    class OMutexAndBroadcastHelper
    {
    protected:
        ::osl::Mutex                m_aMutex;
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OMutexAndBroadcastHelper() : m_aBHelper( m_aMutex ) { }

        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return m_aBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return m_aBHelper; }

    };

    // base class for all classes who are derived from OPropertySet and from OComponent
    // @deprecated, you should use cppu::BaseMutex instead (cppuhelper/basemutex.hxx)

    class OBaseMutex
    {
    protected:
        mutable ::osl::Mutex m_aMutex;
    };
}
#endif // INCLUDED_COMPHELPER_BROADCASTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
