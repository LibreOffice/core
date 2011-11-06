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



#ifndef SC_SUBTOTAL_HXX
#define SC_SUBTOTAL_HXX

#include "global.hxx"

class SubTotal
{
public:

    static  sal_Bool SafePlus( double& fVal1, double fVal2);
    static  sal_Bool SafeMult( double& fVal1, double fVal2);
    static  sal_Bool SafeDiv( double& fVal1, double fVal2);
};


struct ScFunctionData                   // zum Berechnen von einzelnen Funktionen
{
    ScSubTotalFunc  eFunc;
    double          nVal;
    long            nCount;
    sal_Bool            bError;

    ScFunctionData( ScSubTotalFunc eFn ) :
        eFunc(eFn), nVal(0.0), nCount(0), bError(sal_False) {}
};


#endif


