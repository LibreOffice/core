/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
#define __FRAMEWORK_THREADHELP_IRWLOCK_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

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

};      //  class IRWLock

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
