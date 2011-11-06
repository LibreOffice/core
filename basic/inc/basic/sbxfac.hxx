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



#ifndef __SBX_SBX_FACTORY_HXX
#define __SBX_SBX_FACTORY_HXX

#include <basic/sbxdef.hxx>

class SbxBase;
class SbxObject;
class String;
class UniString;

class SbxFactory
{
    sal_Bool bHandleLast;   // sal_True: Factory is asked at last because of its expensiveness
public:
    SbxFactory( sal_Bool bLast=sal_False )  { bHandleLast = bLast; }
    sal_Bool IsHandleLast( void )       { return bHandleLast; }
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

#endif
