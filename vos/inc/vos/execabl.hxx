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




#ifndef _VOS_EXECABL_HXX_
#define _VOS_EXECABL_HXX_

#   include <vos/types.hxx>
#   include <vos/refernce.hxx>

namespace vos
{


/** IExecutable

    The IExecutable-interface is to be understood as wrapper around
    your application-code, which allows it to be executed asynchronously.

    Wether you want real asynchronous behaviour or just pseudo-multitasking
    depends on which kind of execution-service you use to execute your executable.

    (E.g. Threadpool/Fiberpool)

    @author  Bernd Hofner
    @version 0.1
*/

class IExecutable : public vos::IReference
{
public:

    /** Overload to implement your functionality.
        Return True, if you want "execute()" to be called again.
    */
    virtual sal_Bool SAL_CALL execute()= 0;


    /** If you use blocking calls within your execute-function,
        you should provide here a means to unblock cleanly.
        @Return False if you are not able to unblock the
        thread.

    */
    virtual sal_Bool SAL_CALL unblock()= 0;

    /**
        STL demands this operators if you want to place
        IExecutables per value in collections.
    */
    virtual sal_Bool SAL_CALL operator<(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator>(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator==(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator!=(const IExecutable&) const= 0;
};


/** OExecutable
    added default impl. of IReferenceCounter
*/
class OExecutable : public vos::IExecutable,
                    public vos::OReference

{
public:

    virtual ~OExecutable()
    {
    }

    /*
        Since the dominance of the OReferenceCounter impl.
        of the IReferenceCounter is not granted, delegate
        the methods to this branch of the diamond-shaped
        inheritance tree.
    */

    virtual RefCount SAL_CALL acquire()
    {
        return OReference::acquire();
    }
    virtual RefCount SAL_CALL release()
    {
        return OReference::release();
    }
    virtual RefCount SAL_CALL referenced() const
    {
        return OReference::referenced();
    }


    /** Default implementation of unblock does nothing.
    */
    virtual sal_Bool SAL_CALL unblock() { return sal_True; }


    /**
        STL demands this operators if you want to place
        IExecutables per value in collections.
        Use a default implementation of the comparison-operators
        here without a correct semantic.
    */
    virtual sal_Bool SAL_CALL operator<(const IExecutable&) const
    {
        return sal_False;
    }

    virtual sal_Bool SAL_CALL operator>(const IExecutable&) const
    {
        return sal_False;
    }

    virtual sal_Bool SAL_CALL operator==(const IExecutable&) const
    {
        return sal_True;
    }

    virtual sal_Bool SAL_CALL operator!=(const IExecutable&) const
    {
        return sal_False;
    }
};

}


#endif  // _VOS_EXECABL_HXX_

