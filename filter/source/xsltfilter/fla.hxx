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



/*
Function Language for Attributes. See .cxx-File for Syntax. Used in WordML-Filter.
*/
#ifndef __FLA_HXX__
#define __FLA_HXX__

#include <sal/types.h>
#include <rtl/ustring.hxx>
namespace FLA {


class Evaluator
{
private:
    sal_Unicode environment[100][10];
    sal_Unicode _buf[1024];
    sal_Int32 evalFunc(const sal_Unicode*op, sal_Int32 opLen, sal_Int32 args, sal_Unicode* argv[10], sal_Unicode *result);
    sal_Int32 evalExp(const sal_Unicode *expr, sal_Int32 exprLen, sal_Unicode *buf, sal_Int32 *bufLen);
public:
    const sal_Unicode *eval(const sal_Unicode *expr, sal_Int32 exprLen);
};
}



#endif /* __FLA_HXX__ */
