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

#ifndef _VCL_SOLARMUTEX_HXX_
#define _VCL_SOLARMUTEX_HXX_

#include <osl/mutex.hxx>
#include <vcl/dllapi.h>

namespace vcl
{

/** Implementation of the SolarMutex interface.
 */
class VCL_DLLPUBLIC SolarMutexObject : public osl::SolarMutex
{
public:
    //static SolarMutex& SAL_CALL getGlobalMutex();

    /** Creates mutex
     */
    SolarMutexObject();

    /** Implicitly destroys mutex
     */
    virtual ~SolarMutexObject();

    /** Blocks if Mutex is already in use
     */
    virtual void SAL_CALL acquire();

    /** Tries to get the mutex without blocking.
        @return True if mutex could be obtained, otherwise False
     */
    virtual sal_Bool SAL_CALL tryToAcquire();

    /** Releases the mutex.
     */
    virtual void SAL_CALL release();

protected:
    oslMutex    m_solarMutex;

private:
    /* Disable copy/assignment
     */
    SolarMutexObject( const SolarMutexObject& );
    SolarMutexObject& SAL_CALL operator=( const SolarMutexObject& );
};

}

#endif // _VCL_SOLARMUTEX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
