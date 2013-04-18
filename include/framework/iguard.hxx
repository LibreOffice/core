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

#ifndef __FRAMEWORK_THREADHELP_IGUARD_H_
#define __FRAMEWORK_THREADHELP_IGUARD_H_

#include <sal/types.h>

namespace framework{

/*-************************************************************************************************************//**
    @descr          interface for guarding a lock
*//*-*************************************************************************************************************/
class SAL_NO_VTABLE IGuard
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /** clears the lock. If the guard does not currently hold the lock, nothing happens.
        */
        virtual void clear() = 0;

        /** attempts to re-establishes the lock, blocking until the attempt is successful.
        */
        virtual void reset() = 0;

    protected:
        ~IGuard() {}

};      //  class IGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IGUARD_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
