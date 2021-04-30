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

#include "UpdateRequest.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework
{
UpdateRequest::UpdateRequest() noexcept
    : UpdateRequestInterfaceBase(MutexOwner::maMutex)
{
}

UpdateRequest::~UpdateRequest() noexcept {}

void SAL_CALL UpdateRequest::execute(const Reference<XConfiguration>&)
{
    // Do nothing here.  The configuration is updated when the request queue
    // becomes empty.
}

OUString SAL_CALL UpdateRequest::getName() { return "UpdateRequest"; }

void SAL_CALL UpdateRequest::setName(const OUString&)
{
    // Ignored.
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
