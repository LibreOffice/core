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

#ifndef INCLUDED_FRAMEWORK_IMUTEX_HXX
#define INCLUDED_FRAMEWORK_IMUTEX_HXX

#include <sal/types.h>

namespace framework{

/*-************************************************************************************************************
    @descr          We need this interface to support using of different mutex implementations in a generic way.
*//*-*************************************************************************************************************/
class SAL_NO_VTABLE IMutex
{

    //  public methods

    public:

        /*-****************************************************************************************************
            @descr      These functions must be supported by a derived class!
                            acquire()     -try to register thread
                            release()     -unregister thread
        *//*-*****************************************************************************************************/
        virtual void acquire() = 0;
        virtual void release() = 0;

    protected:
        ~IMutex() {}
};      //  class IMutex

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_IMUTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
