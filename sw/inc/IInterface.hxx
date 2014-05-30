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

#ifndef INCLUDED_SW_INC_IINTERFACE_HXX
#define INCLUDED_SW_INC_IINTERFACE_HXX

#include <sal/types.h>

/** The base interface
 */
class IInterface
{
public:

    /** Acquire a reference to an instance. A caller shall release
        the instance by calling 'release' when it is no longer needed.
        'acquire' and 'release' calls need to be balanced.

        @returns
        the current reference count of the instance for debugging purposes.
    */
    virtual sal_Int32 acquire() = 0;

    /** Releases a reference to an instance. A caller has to call
        'release' when a before acquired reference to an instance
        is no longer needed. 'acquire' and 'release' calls need to
        be balanced.

    @returns
        the current reference count of the instance for debugging purposes.
    */
    virtual sal_Int32 release() = 0;

    /** Returns the current reference count. This method should be used for
        debugging purposes. Using it otherwise is a signal of a design flaw.
    */
    virtual sal_Int32 getReferenceCount() const = 0;

protected:
    virtual ~IInterface() {};
};

#endif // INCLUDED_SW_INC_IINTERFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
