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



#ifndef _OBJFAC3D_HXX
#define _OBJFAC3D_HXX

#include "svx/svxdllapi.h"


/*************************************************************************
|*
|* Factory fuer Chart-spezifische Objekte
|*
\************************************************************************/

class SdrObjFactory;


#include <tools/link.hxx>

class SVX_DLLPUBLIC E3dObjFactory
{
public:
    E3dObjFactory();
    ~E3dObjFactory();

    DECL_LINK(MakeObject, SdrObjFactory*);
};



#endif // _OBJFAC3D_HXX
