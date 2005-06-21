/*************************************************************************
 *
 *  $RCSfile: comm_bas.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-06-21 19:07:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMM_BAS_HXX
#define _COMM_BAS_HXX

#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <basic/sbxfac.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif

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
        ( SbxVariable* pThis, SbxArray* pArgs, BOOL bWrite );
#if defined ( ICC ) || defined ( HPUX )
private:
#endif

    struct Methods {
        const char* pName;      // Name des Eintrags
        SbxDataType eType;      // Datentyp
        short nArgs;            // Argumente und Flags
        pMeth pFunc;            // Function Pointer
    };
    static Methods aManagerMethods[];   // Methodentabelle
    static Methods aLinkMethods[];      // Methodentabelle
    Methods *pMethods;  // Aktuelle Methodentabelle

    // Methoden
    //      Manager
    void MStartCommunication( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void MStopAllCommunication( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void MIsCommunicationRunning( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void MGetMyName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void MIsLinkValid( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void MSetCommunicationEventHandler( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );

    //      Link
    void LStopCommunication( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void LGetMyName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void LGetHostName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void LSend( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void LGetString( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );

    // Interne Member und Methoden
    CommunicationManagerClientViaSocket *pManager;
    CommunicationLink *pLink;
    BOOL bIsManager;    // Ist es kein Manager, so ist es ein Link

    // Kram für Manager
    DECL_LINK( Open, CommunicationLink* );
    DECL_LINK( Close, CommunicationLink* );
    DECL_LINK( Data, CommunicationLink* );
    void Events( String aType, CommunicationLink* pLink );
    BOOL bCatchOpen;
    CommunicationLink *pNewLink;
    String aEventHandlerName;

    // Kram für Link


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

    CommunicationLink* GetCommunicationLink() { return pLink; }
};


// Die dazugehoerige Factory:

class CommunicationFactory : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif
