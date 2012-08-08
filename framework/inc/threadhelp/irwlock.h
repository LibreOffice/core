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

#ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
#define __FRAMEWORK_THREADHELP_IRWLOCK_H_

namespace framework{

/*-************************************************************************************************************//**
    @descr          A guard (specialy a write guard) support different internal working states.
                    His lock can set for reading or writing/reading! Or he was unlocked by user ...
*//*-*************************************************************************************************************/
enum ELockMode
{
    E_NOLOCK    ,
    E_READLOCK  ,
    E_WRITELOCK
};

/*-************************************************************************************************************//**
    @descr          We implement two guards for using an rw-lock. But if you wish to implement
                    different rw-locks to you will have problems by using with same guard implementation!
                    Thats why we define this "pure virtual base class" ...
                    All rw-locks must support this base interface for working and all guard must use this one too!
*//*-*************************************************************************************************************/
class IRWLock
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
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

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
