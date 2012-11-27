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


#include "tools/AsynchronousCall.hxx"

#include "DrawViewShell.hxx"

namespace sd { namespace tools {

AsynchronousCall::AsynchronousCall (void)
    : maTimer(),
      mpFunction()
{
    Link aCallback (LINK(this,AsynchronousCall,TimerCallback));
    maTimer.SetTimeoutHdl(aCallback);
}




AsynchronousCall::~AsynchronousCall (void)
{
    mpFunction.reset();
    maTimer.Stop();
}




void AsynchronousCall::Post (
    const AsynchronousFunction& rFunction,
    sal_uInt32 nTimeoutInMilliseconds)
{
    mpFunction.reset(new AsynchronousFunction(rFunction));
    maTimer.SetTimeout(nTimeoutInMilliseconds);
    maTimer.Start();
}




IMPL_LINK(AsynchronousCall,TimerCallback,Timer*,pTimer)
{
    if (pTimer == &maTimer)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<AsynchronousFunction> pFunction (mpFunction);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        mpFunction.reset();
        (*pFunction)();
    }
    return 0;
}


} } // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
