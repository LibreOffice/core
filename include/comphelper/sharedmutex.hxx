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

#ifndef INCLUDED_COMPHELPER_SHAREDMUTEX_HXX
#define INCLUDED_COMPHELPER_SHAREDMUTEX_HXX

#include <comphelper/comphelperdllapi.h>
#include <osl/mutex.hxx>
#include <memory>

namespace comphelper
{


    //= SharedMutex

    class COMPHELPER_DLLPUBLIC SharedMutex
    {
    public:
        SharedMutex();
        SharedMutex( const SharedMutex& );
        SharedMutex& operator=( const SharedMutex& );

        operator ::osl::Mutex& () { return *m_pMutexImpl; }

    private:
        std::shared_ptr< ::osl::Mutex >  m_pMutexImpl;
    };


    //= SharedMutexBase

    /** sometimes, it's necessary to have an initialized ::osl::Mutex to pass
        to some ctor call of your base class. In this case, you can't hold the
        SharedMutex as member, but you need to move it into another base class,
        which is initialized before the mutex-requiring class is.
    */
    class SharedMutexBase
    {
    protected:
        SharedMutexBase()
        {
        }
        ~SharedMutexBase()
        {
        }

    protected:
        ::osl::Mutex&   getMutex() const { return m_aMutex; }
        SharedMutex&    getSharedMutex() const { return m_aMutex; }

    private:
        mutable SharedMutex m_aMutex;
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_SHAREDMUTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
