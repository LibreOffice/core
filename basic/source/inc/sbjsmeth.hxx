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




#ifndef _SB_SBJSMETH_HXX
#define _SB_SBJSMETH_HXX

#include <basic/sbmeth.hxx>

// Basic-Modul fuer JavaScript-Sourcen.
// Alle Basic-spezifischen Methoden muessen virtuell ueberladen und deaktiviert
// werden. Die Unterscheidung von normalen Modulen erfolgt uebr RTTI.

class SbJScriptMethod : public SbMethod
{
public:
    SbJScriptMethod( const String&, SbxDataType, SbModule* );
    virtual ~SbJScriptMethod();

    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_JSCRIPTMETH,2);
};

#ifndef __SB_SBJSCRIPTMETHODREF_HXX
#define __SB_SBJSCRIPTMETHODREF_HXX
SV_DECL_IMPL_REF(SbJScriptMethod)
#endif

#endif
