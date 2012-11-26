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



#ifndef _SB_SBJSMOD_HXX
#define _SB_SBJSMOD_HXX

#include <basic/sbmod.hxx>

// Basic-Modul fuer JavaScript-Sourcen.
// Alle Basic-spezifischen Methoden muessen virtuell ueberladen und deaktiviert
// werden. Die Unterscheidung von normalen Modulen erfolgt uebr RTTI.

class SbJScriptModule : public SbModule
{
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_JSCRIPTMOD,1);
    SbJScriptModule( const String& );       // DURCHREICHEN
};

#endif



