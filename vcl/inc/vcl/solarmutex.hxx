/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
