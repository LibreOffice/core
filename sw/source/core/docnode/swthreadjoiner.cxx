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

#include <swthreadjoiner.hxx>
#include <finalthreadmanager.hxx>
#include <com/sun/star/util/JobManager.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

/** Testing

    @author OD
*/
using namespace ::com::sun::star;

uno::Reference< util::XJobManager > SwThreadJoiner::mpThreadJoiner;

namespace
{
    class theJoinerMutex : public rtl::Static<osl::Mutex, theJoinerMutex> {};
}

uno::Reference< util::XJobManager >& SwThreadJoiner::GetThreadJoiner()
{
    osl::MutexGuard aGuard(theJoinerMutex::get());

    if ( !mpThreadJoiner.is() )
    {
        mpThreadJoiner = util::JobManager::create( comphelper::getProcessComponentContext() );
    }

    return mpThreadJoiner;
}

void SwThreadJoiner::ReleaseThreadJoiner()
{
    mpThreadJoiner.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
