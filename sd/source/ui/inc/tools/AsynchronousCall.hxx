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

#pragma once

#include <vcl/timer.hxx>

#include <memory>
#include <functional>

namespace sd::tools
{
/** Store a function object and execute it asynchronous.

    The features of this class are:
    a) It provides a wrapper around a VCL Timer so that generic function
    objects can be used.
    b) When more than one function objects are posted to be executed later
    then the pending ones are erased and only the last one will actually be
    executed.

    Use this class like this:
    aInstanceOfAsynchronousCall.Post(
        ::std::bind(
            ::std::mem_fun(&DrawViewShell::SwitchPage),
            pDrawViewShell,
            11));
*/
class AsynchronousCall
{
public:
    /** Create a new asynchronous call.  Each object of this class processes
        one (semantical) type of call.
    */
    AsynchronousCall();

    ~AsynchronousCall();

    /** Post a function object that is to be executed asynchronously.  When
        this method is called while the current function object has not been
        executed then the latter is destroyed and only the given function
        object will be executed.
        @param rFunction
            The function object that may be called asynchronously in the
            near future.
    */
    typedef ::std::function<void()> AsynchronousFunction;
    void Post(const AsynchronousFunction& rFunction);

private:
    Timer maTimer;
    /** The function object that will be executed when the TimerCallback
        function is called the next time.  This pointer may be NULL.
    */
    ::std::unique_ptr<AsynchronousFunction> mpFunction;
    DECL_LINK(TimerCallback, Timer*, void);
};

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
