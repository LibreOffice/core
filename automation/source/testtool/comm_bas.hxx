/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _COMM_BAS_HXX
#define _COMM_BAS_HXX

#include <basic/sbxfac.hxx>
#include <basic/sbxvar.hxx>
#include <basic/sbxobj.hxx>

class CommunicationManagerClientViaSocket;
class CommunicationLink;

class CommunicationWrapper : public SbxObject   // one for manager(s) and links
{
    // definition of a table entry. That's done here because the
    // methods and properties can be declared private that way.
#if defined ( ICC ) || defined ( C50 )
public:
#endif
    typedef void( CommunicationWrapper::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, sal_Bool bWrite );
#if defined ( ICC )
private:
#endif

    struct Methods {
        const char* pName;
        SbxDataType eType;
        pMeth pFunc;
        short nArgs;
    };
    static Methods aManagerMethods[];   // method table
    static Methods aLinkMethods[];      // method table
    Methods *m_pMethods;    // current method table

    // methods
    //      manager
    void MStartCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MStopAllCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MIsCommunicationRunning( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MGetMyName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MIsLinkValid( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void MSetCommunicationEventHandler( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

    //      link
    void LStopCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LGetMyName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LGetHostName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LSend( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void LGetString( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

    // internal members and methods
    CommunicationManagerClientViaSocket *m_pManager;
    CommunicationLink *m_pLink;
    sal_Bool m_bIsManager;

    DECL_LINK( Open, CommunicationLink* );
    DECL_LINK( Close, CommunicationLink* );
    DECL_LINK( Data, CommunicationLink* );
    void Events( String aType, CommunicationLink* pLink );
    sal_Bool m_bCatchOpen;
    CommunicationLink *m_pNewLink;
    String m_aEventHandlerName;

    using SbxVariable::GetInfo;
    SbxInfo* GetInfo( short nIdx );

    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    CommunicationWrapper( const String& );
    CommunicationWrapper( CommunicationLink *pThisLink );
    ~CommunicationWrapper();

    virtual SbxVariable* Find( const String&, SbxClassType );

    CommunicationLink* GetCommunicationLink() { return m_pLink; }
};



class CommunicationFactory : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
