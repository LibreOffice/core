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

#ifndef INCLUDED_COMPHELPER_GUARDING_HXX
#define INCLUDED_COMPHELPER_GUARDING_HXX

#include <osl/mutex.hxx>


namespace comphelper
{



// = class MutexRelease -


/** opposite of OGuard :)
    (a mutex is released within the constructor and acquired within the desctructor)
    use only when you're sure the mutex is acquired !
*/
template <class MUTEX>
class ORelease
{
    MUTEX&  m_rMutex;

public:
    ORelease(MUTEX& _rMutex) : m_rMutex(_rMutex) { _rMutex.release(); }
    ~ORelease() { m_rMutex.acquire(); }
};

typedef ORelease< ::osl::Mutex >    MutexRelease;


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_GUARDING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
