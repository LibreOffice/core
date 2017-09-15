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

#ifndef INCLUDED_COMPHELPER_SOLARMUTEX_HXX
#define INCLUDED_COMPHELPER_SOLARMUTEX_HXX

#include <sal/config.h>

#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper {


/**
 * Abstract SolarMutex interface, needed for VCL's
 * Application::GetSolarMutex().
 *
 * The SolarMutex is the one big recursive code lock used
 * to protect the vast majority of the LibreOffice code-base,
 * in particular anything that is graphical and the cores of
 * the applications.
 */
class COMPHELPER_DLLPUBLIC SolarMutex {
public:
    typedef void (*BeforeReleaseHandler) ();

    void acquire( sal_uInt32 nLockCount = 1 );
    sal_uInt32 release( bool bUnlockAll = false );

    virtual bool tryToAcquire() = 0;

    // returns true, if the mutex is owned by the current thread
    virtual bool IsCurrentThread() const = 0;

    /// Help components to get the SolarMutex easily.
    static SolarMutex *get();

protected:
    SolarMutex();
    virtual ~SolarMutex();

    /// allow VCL to push its one-big-lock down here.
    static void setSolarMutex( SolarMutex *pMutex );

    virtual sal_uInt32 doRelease( bool bUnlockAll ) = 0;
    virtual void doAcquire( sal_uInt32 nLockCount ) = 0;

private:
    SolarMutex(const SolarMutex&) = delete;
    SolarMutex& operator=(const SolarMutex&) = delete;
};

inline void SolarMutex::acquire( sal_uInt32 nLockCount )
{
    assert( nLockCount > 0 );
    doAcquire( nLockCount );
}

inline sal_uInt32 SolarMutex::release( bool bUnlockAll )
{
     return doRelease( bUnlockAll );
}


/**
 * Generic implementation of the abstract SolarMutex interface.
 *
 * Treat this as a singleton, as its constructor calls
 * setSolarMutex( this )!
 *
 * Kept seperately from SolarMutex, so others can implement fascades.
 */
class COMPHELPER_DLLPUBLIC GenericSolarMutex
    : public SolarMutex
{
public:
    void SetBeforeReleaseHandler( const BeforeReleaseHandler& rLink )
         { m_aBeforeReleaseHandler = rLink; }

    virtual bool tryToAcquire() override;
    virtual bool IsCurrentThread() const override;

protected:
    osl::Mutex           m_aMutex;
    sal_uInt32           m_nCount;
    oslThreadIdentifier  m_nThreadId;

    virtual void doAcquire( sal_uInt32 nLockCount ) override;
    virtual sal_uInt32 doRelease( bool bUnlockAll ) override;

protected:
    GenericSolarMutex();
    virtual ~GenericSolarMutex() override;

private:
    BeforeReleaseHandler  m_aBeforeReleaseHandler;
};

}

#endif // INCLUDED_COMPHELPER_SOLARMUTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
