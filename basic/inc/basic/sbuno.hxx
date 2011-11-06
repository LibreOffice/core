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



#ifndef _SB_SBUNO_HXX
#define _SB_SBUNO_HXX

#include <basic/sbxobj.hxx>

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}

// Returns a SbxObject that wrapps an Uno Interface
// Implementation in basic/source/classes/sbunoobj.cxx
SbxObjectRef GetSbUnoObject( const String& aName, const com::sun::star::uno::Any& aUnoObj_ );

// Force creation of all properties for debugging
void createAllObjectProperties( SbxObject* pObj );

::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );

void unoToSbxValue( SbxVariable* pVar, const ::com::sun::star::uno::Any& aValue );

#endif

