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
#include "AsynchronousCall.hxx"

#include <vcl/svapp.hxx>


namespace sfx2 { namespace sidebar {

AsynchronousCall::AsynchronousCall (void)
    : maAction(),
      mnCallId(0)
{
}




AsynchronousCall::AsynchronousCall (const Action& rAction)
    : maAction(rAction),
      mnCallId(0)
{
}




AsynchronousCall::~AsynchronousCall (void)
{
    CancelRequest();
}




void AsynchronousCall::RequestCall (const Action& rAction)
{
    CancelRequest();
    maAction = rAction;
    RequestCall();
}




void AsynchronousCall::RequestCall (void)
{
    if (mnCallId == 0)
    {
        Link aLink (LINK(this, AsynchronousCall, HandleUserCall));
        mnCallId = Application::PostUserEvent(aLink);
    }
}




void AsynchronousCall::CancelRequest (void)
{
    if (mnCallId != 0)
    {
        Application::RemoveUserEvent(mnCallId);
        mnCallId = 0;
    }
}




IMPL_LINK(AsynchronousCall, HandleUserCall, void*, EMPTYARG )
{
    mnCallId = 0;
    if (maAction)
        maAction();

    return sal_True;
}


} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
