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



#ifndef __FRAMEWORK_THREADHELP_IGATE_H_
#define __FRAMEWORK_THREADHELP_IGATE_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <osl/time.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          We need this interface to support using of different gate implementations in a generic way.
*//*-*************************************************************************************************************/
class IGate
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @descr      These functions must be supported by a derived class!
                            open()      -open access for all waiting threads
                            close()     -close access for all further coming threads
                            openGap()   -open access for current waiting threads only
                            wait()      -must be called to pass the gate
        *//*-*****************************************************************************************************/
        virtual void     open    (                                   ) = 0;
        virtual void     close   (                                   ) = 0;
        virtual void     openGap (                                   ) = 0;
        virtual sal_Bool wait    ( const TimeValue* pTimeOut = NULL  ) = 0;

};      //  class IGate

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IGATE_H_
