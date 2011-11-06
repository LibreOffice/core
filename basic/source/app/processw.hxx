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



#ifndef _PROCESSW_HXX
#define _PROCESSW_HXX

#include <basic/sbxfac.hxx>
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif
#include <basic/sbxobj.hxx>
#include <basic/process.hxx>

class ProcessWrapper : public SbxObject
{
using SbxVariable::GetInfo;
// Definition of a table entry. This is done here because
// through this methods and property can declared as private.
#if defined ( ICC ) || defined ( HPUX ) || defined ( C50 ) || defined ( C52 )
public:
#endif
    typedef void( ProcessWrapper::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, sal_Bool bWrite );
#if defined ( ICC ) || defined ( HPUX )
private:
#endif

    struct Methods {
        const char* pName;      // Name of the entry
        SbxDataType eType;      // Data type
        pMeth pFunc;            // Function Pointer
        short nArgs;            // Arguments and flags
    };
    static Methods aProcessMethods[];   // Method table
    Methods *pMethods;  // Current method table

    void PSetImage( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PStart( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PGetExitCode( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PIsRunning( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void PWasGPF( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

    // Internal members and methods
    Process *pProcess;

    // Fill info block
    SbxInfo* GetInfo( short nIdx );

    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    ProcessWrapper();
    ~ProcessWrapper();
    // Search for an element
    virtual SbxVariable* Find( const String&, SbxClassType );
};

// Factory
class ProcessFactory : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif
