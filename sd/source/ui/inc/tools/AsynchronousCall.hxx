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

#ifndef SD_ASYNCHRONOUS_CALL_HXX
#define SD_ASYNCHRONOUS_CALL_HXX

#include <vcl/timer.hxx>
#include <memory>
#include <boost/function.hpp>

namespace sd { namespace tools {


/** Store a function object and execute it asynchronous.

    The features of this class are:
    a) It provides a wrapper around a VCL Timer so that generic function
    objects can be used.
    b) When more than one function objects are posted to be executed later
    then the pending ones are erased and only the last one will actually be
    executed.

    Use this class like this:
    aInstanceOfAsynchronousCall.Post(
        ::boost::bind(
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
    AsynchronousCall (void);

    ~AsynchronousCall (void);

    /** Post a function object that is to be executed asynchronously.  When
        this method is called while the current function object has not bee
        executed then the later is destroyed and only the given function
        object will be executed.
        @param rFunction
            The function object that may be called asynchronously in the
            near future.
        @param nTimeoutInMilliseconds
            The timeout in milliseconds until the function object is
            executed.
    */
    typedef ::boost::function0<void> AsynchronousFunction;
    void Post (
        const AsynchronousFunction& rFunction,
        sal_uInt32 nTimeoutInMilliseconds=10);

private:
    Timer maTimer;
    /** The function object that will be executed when the TimerCallback
        function is called the next time.  This pointer may be NULL.
    */
    ::std::auto_ptr<AsynchronousFunction> mpFunction;
    DECL_LINK(TimerCallback,Timer*);
};


} } // end of namespace ::sd::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
