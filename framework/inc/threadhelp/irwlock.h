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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_IRWLOCK_H
#define INCLUDED_FRAMEWORK_INC_THREADHELP_IRWLOCK_H

namespace framework{

/*-************************************************************************************************************
    @descr          A guard (specialy a write guard) support different internal working states.
                    His lock can set for reading or writing/reading! Or he was unlocked by user ...
*//*-*************************************************************************************************************/
enum ELockMode
{
    E_NOLOCK    ,
    E_READLOCK  ,
    E_WRITELOCK
};

/*-************************************************************************************************************
    @descr          We implement two guards for using an rw-lock. But if you wish to implement
                    different rw-locks to you will have problems by using with same guard implementation!
                    Thats why we define this "pure virtual base class" ...
                    All rw-locks must support this base interface for working and all guard must use this one too!
*//*-*************************************************************************************************************/
class IRWLock
{

    //  public methods

    public:

        /*-****************************************************************************************************
            @descr      These functions must be supported by a derived class!
                            acquireReadAccess()     -try to register thread as reader
                            releaseReadAccess()     -unregister thread as reader
                            acquireWriteAccess()    -try to register thread as writer
                            releaseWriteAccess()    -unregister thread as writer
                            downgradeWriteAccess()  -make writer to reader
        *//*-*****************************************************************************************************/
        virtual void acquireReadAccess    () =0;
        virtual void releaseReadAccess    () =0;
        virtual void acquireWriteAccess   () =0;
        virtual void releaseWriteAccess   () =0;
        virtual void downgradeWriteAccess () =0;

    protected:
        ~IRWLock() {}
};      //  class IRWLock

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_IRWLOCK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
