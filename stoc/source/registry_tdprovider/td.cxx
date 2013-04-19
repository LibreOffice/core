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


#include "osl/doublecheckedlocking.h"
#include "base.hxx"

namespace stoc_rdbtdp
{

//------------------------------------------------------------------------------
::osl::Mutex & getMutex()
{
    static ::osl::Mutex * s_pmutex = 0;
    if (s_pmutex == 0)
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if (s_pmutex == 0)
        {
            static ::osl::Mutex s_mutex;
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            s_pmutex = &s_mutex;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return *s_pmutex;
}

TypeDescriptionImpl::~TypeDescriptionImpl() {}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass TypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _eTypeClass;
}
//__________________________________________________________________________________________________
OUString TypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
