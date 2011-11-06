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



#ifndef _COMM_BAS_HXX
#define _COMM_BAS_HXX

#include <basic/sbxfac.hxx>
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif
#include <basic/sbxobj.hxx>

class CommunicationManagerClientViaSocket;
class CommunicationLink;

class CommunicationWrapper : public SbxObject   // Einer für Manager und Links
{
    // Definition eines Tabelleneintrags. Dies wird hier gemacht,
    // da dadurch die Methoden und Properties als private deklariert
    // werden koennen.
#if defined ( ICC ) || defined ( HPUX ) || defined ( C50 ) || defined ( C52 )
public:
#endif
    typedef void( CommunicationWrapper::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, sal_Bool bWrite );
#if defined ( ICC ) || defined ( HPUX )
private:
#endif

    struct Methods {
        const char* pName;      // Name des Eintrags
        SbxDataType eType;      // Datentyp
        pMeth pFunc;            // Function Pointer
        short nArgs;            // Argumente und Flags
    };
    static Methods aManagerMethods[];   // Methodentabelle
    static Methods aLinkMethods[];      // Methodentabelle
    Methods *m_pMethods;    // Aktuelle Methodentabelle

    // Methoden
    //      Manager
    void MStartCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MStopAllCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MIsCommunicationRunning( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MGetMyName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MIsLinkValid( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MSetCommunicationEventHandler( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

    //      Link
    void LStopCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LGetMyName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LGetHostName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LSend( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LGetString( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

    // Interne Member und Methoden
    CommunicationManagerClientViaSocket *m_pManager;
    CommunicationLink *m_pLink;
    sal_Bool m_bIsManager;  // Ist es kein Manager, so ist es ein Link

    // Kram für Manager
    DECL_LINK( Open, CommunicationLink* );
    DECL_LINK( Close, CommunicationLink* );
    DECL_LINK( Data, CommunicationLink* );
    void Events( String aType, CommunicationLink* pLink );
    sal_Bool m_bCatchOpen;
    CommunicationLink *m_pNewLink;
    String m_aEventHandlerName;

    using SbxVariable::GetInfo;
    // Infoblock auffuellen
    SbxInfo* GetInfo( short nIdx );

    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    CommunicationWrapper( const String& );
    CommunicationWrapper( CommunicationLink *pThisLink );
    ~CommunicationWrapper();
    // Suchen eines Elements
    virtual SbxVariable* Find( const String&, SbxClassType );

    CommunicationLink* GetCommunicationLink() { return m_pLink; }
};


// Die dazugehoerige Factory:

class CommunicationFactory : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif
