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
#ifndef SFX_SIDEBAR_ASYNCHRONOUS_CALL_HXX
#define SFX_SIDEBAR_ASYNCHRONOUS_CALL_HXX

#include <boost/function.hpp>
#include <tools/solar.h>
#include <tools/link.hxx>

namespace sfx2 { namespace sidebar {

/** A simple asynchronous call via Application::PostUserCall.
*/
class AsynchronousCall
{
public:
    typedef ::boost::function<void(void)> Action;

    AsynchronousCall (void);
    AsynchronousCall (const Action& rAction);
    ~AsynchronousCall (void);

    void RequestCall (const Action& rAction);
    void RequestCall (void);
    void CancelRequest (void);

private:
    Action maAction;
    sal_uLong mnCallId;

    DECL_LINK(HandleUserCall, void*);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
