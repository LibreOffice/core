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

#include <osl/interlck.h>

namespace sd
{
/** A simple base implementation for reference-counted components.
    acts like sal::SimpleReferenceObject but calls the virtual disposing()
    methods before the ref count switches from 1 to zero.
 */
class SimpleReferenceComponent
{
public:
    SimpleReferenceComponent();

    /** @ATTENTION
        The results are undefined if, for any individual instance of
        SimpleReferenceComponent, the total number of calls to acquire() exceeds
        the total number of calls to release() by a platform dependent amount
        (which, hopefully, is quite large).
     */
    void acquire();
    void release();

    void Dispose();

    bool isDisposed() const { return mbDisposed; }

protected:
    virtual void disposing();

    virtual ~SimpleReferenceComponent();

private:
    oslInterlockedCount m_nCount;

    SimpleReferenceComponent(SimpleReferenceComponent const&) = delete;
    void operator=(SimpleReferenceComponent const&) = delete;

    bool mbDisposed;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
