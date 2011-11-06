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



#ifndef _BASRDLL_HXX
#define _BASRDLL_HXX

class ResMgr;

#include <vcl/accel.hxx>

class BasicDLL
{
private:
    ResMgr*     pSttResMgr;
    ResMgr*     pBasResMgr;

    sal_Bool        bDebugMode;
    sal_Bool        bBreakEnabled;

public:
                BasicDLL();
                ~BasicDLL();

    ResMgr*     GetSttResMgr() const { return pSttResMgr; }
    ResMgr*     GetBasResMgr() const { return pBasResMgr; }

    static void BasicBreak();

    static void EnableBreak( sal_Bool bEnable );
    static void SetDebugMode( sal_Bool bDebugMode );
};

#define BASIC_DLL() (*(BasicDLL**)GetAppData( SHL_BASIC ) )

#endif //_BASRDLL_HXX
