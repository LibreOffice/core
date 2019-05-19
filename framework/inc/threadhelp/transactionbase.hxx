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



#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_
#define __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/transactionmanager.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          make it possible to instantiate a transaction manager as first member!
    @descr          If you use a transaction manager as a member of your class and wish to use it earlier than other ones
                    you should have a look at this implementation. You must use it as the first base class
                    of your implementation - because base classes are initialized by his order and before your
                    member! That's why it is a good place to declare this member.

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct TransactionBase
{
    //-------------------------------------------------------------------------------------------------------------
    //  public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        mutable TransactionManager    m_aTransactionManager    ;   /// "your" public manager-member!
                                                                   /// Make it mutable for using in const functions!

};      //  struct TransactionBase

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_H_
