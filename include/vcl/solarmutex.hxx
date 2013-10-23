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

#ifndef INCLUDED_VCL_SOLARMUTEX_HXX
#define INCLUDED_VCL_SOLARMUTEX_HXX

#include <comphelper/solarmutex.hxx>
#include <osl/mutex.h>
#include <vcl/dllapi.h>

namespace vcl
{

/** Implementation of the SolarMutex interface.
 */
class VCL_DLLPUBLIC SolarMutexObject : public comphelper::SolarMutex
{
public:
    //static SolarMutex& SAL_CALL getGlobalMutex();

    /** Creates mutex
     */
    SolarMutexObject();

    /** Implicitly destroys mutex
     */
    virtual ~SolarMutexObject();

    virtual void acquire();

    virtual void release();

    virtual bool tryToAcquire();

protected:
    oslMutex    m_solarMutex;

private:
    /* Disable copy/assignment
     */
    SolarMutexObject( const SolarMutexObject& );
    SolarMutexObject& SAL_CALL operator=( const SolarMutexObject& );
};

}

#endif // INCLUDED_VCL_SOLARMUTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
