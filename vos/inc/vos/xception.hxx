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



#ifndef _XCEPTION_HXX_
#define _XCEPTION_HXX_

#include <vos/object.hxx>

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

namespace vos
{

/* avoid clashes with <vos/exception.hxx> */
#define OException OBaseException

/*
 * Macros for true try/catch based Exception Handling (public)
 * based on true rtti type checking
 */

#define THROW_AGAIN                     throw;
#define THROW( Constructor )            throw Constructor;

/*
 * declaration of the exceptions that may be thrown by a function
 * (e.g.) void myfunction(sal_Int32 a) throw ( std::bad_alloc );
 * is not fully supported by all compilers
 */

#define THROWS( ARG ) throw ARG

/*
 * just a base class for further exceptions
 */

class OException : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OException, vos));

public:
    virtual ~OException();
    OException() {}
    OException( const OException & ) : OObject() {}

    OException& SAL_CALL operator=(const OException&)
        { return *this; }
};

}

#endif /* _XCEPTION_HXX_ */

