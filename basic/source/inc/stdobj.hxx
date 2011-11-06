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



#ifndef _SBSTDOBJ_HXX
#define _SBSTDOBJ_HXX

#include <basic/sbxobj.hxx>

class StarBASIC;
class SbStdFactory;

class SbiStdObject : public SbxObject
{
    SbStdFactory* pStdFactory;

   ~SbiStdObject();
    using SbxVariable::GetInfo;
    SbxInfo* GetInfo( short );
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SbiStdObject( const String&, StarBASIC* );
    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual void SetModified( sal_Bool );
};

#endif
