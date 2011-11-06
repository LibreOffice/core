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




#ifndef BASICMOD_HXX
#define BASICMOD_HXX

#include <sfx2/module.hxx>
#include <tools/shl.hxx>

class BasicIDEModule : public SfxModule
{
public:
    BasicIDEModule( ResMgr *pMgr, SfxObjectFactory *pObjFact) :
       SfxModule( pMgr, sal_False, pObjFact, NULL )
       {}
};


#define BASIC_MOD() ( *(BasicIDEModule**) GetAppData(SHL_IDE) )

#endif
