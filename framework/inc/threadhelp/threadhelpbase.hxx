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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_THREADHELPBASE_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_THREADHELPBASE_HXX

#include <threadhelp/lockhelper.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          "baseclass" to make own classes threadsafe
    @descr          Sometimes you must share your lock- or mutex member with any other baseclasses.
                    And baseclasses are initialized erlier then members! That's why you should use
                    this struct as first of your baseclasses!!!
                    Then you will get a public member "m_aLock" which can be used by special guard implementations
                    to make your code threadsafe.

    @seealso        class LockHelper

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct ThreadHelpBase
{

    //  public methods

    public:
        ThreadHelpBase( comphelper::SolarMutex* pSolarMutex = NULL )
                :   m_aLock( pSolarMutex )
        {
        }


    //  public member
    //  Make it mutable for using in const functions!

    public:

        mutable LockHelper m_aLock;
};

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_THREADHELPBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
