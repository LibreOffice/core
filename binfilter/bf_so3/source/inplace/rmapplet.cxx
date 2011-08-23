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

#ifdef REMOTE_VERSION

#include <svgen.hxx>
#include <thread.hxx>

#include <dcca/object.hxx>
#include <dcca/message.hxx>
#include <dcca/broker.hxx>

#include <string.h>

#include <owncont.hxx>
#include <tlgen.hxx>

#include <rsvstrm.hxx>

#include <RmApplet.hxx>

namespace binfilter {

// Attention :  MT_LINK should be here temporarily only for testing
// and is supposed to be moved into SV !

#if !defined WNT
// caution: faster but be aware of deadlocks
// todo:    someone in the main thread must call Lock()/Unlock()
#define MT_LINK
#endif

//	Example:
// 		MT_Link aLink = LINK( Instance, Class, Method );
// 		long n = aLink.Call( (void*)pData );
//	Caution:
//		Never call MT_Link::Call() in the main thread

#if !defined MT_LINK
class MT_Link
{
    Link   	       	rLink_;
    long	   		nReturn_;
    ULONG			nEvent_;
    SignaledEvent	*aOut_;
    static int		count;
    DECL_LINK( Call_Impl, void* );
public:
    long Call( void *p );
    MT_Link( Link r );
    ~MT_Link();
};

IMPL_LINK( MT_Link, Call_Impl, void*, p )
{
    nEvent_  = 0;
    nReturn_ = rLink_.Call( p );
    aOut_->Set();
    return nReturn_;
}
int MT_Link::count = 0;

long MT_Link::Call( void *p )
{
    Link aLink = LINK( this, MT_Link, Call_Impl );

    nEvent_ = GetpApp()->PostUserEvent( aLink, p );
    aOut_->Wait();
    return nReturn_;
}

MT_Link::MT_Link( Link r ) : rLink_( r )
{
    nEvent_ = 0;
    String name1 = "Event";

    name1 += count++;
    aOut_ = new SignaledEvent(name1, FALSE);
}

MT_Link::~MT_Link()
{
    if( nEvent_ ) GetpApp()->RemoveUserEvent( nEvent_ );
    if (aOut_)
        delete aOut_;
}

#else
class MT_Link
{
    Link   	       	rLink_;

    static Mutex	aMtx__;
public:
    static void Lock()   { aMtx__.Lock(); }
    static void Unlock() { aMtx__.Unlock(); }

    long Call( void *p );

    MT_Link( Link r ) : rLink_( r ) {}
};

Mutex MT_Link::aMtx__;

long MT_Link::Call( void *p )
{
    Lock();
    long nReturn = rLink_.Call( p );
    Unlock();
    return nReturn;
}
#endif


// Link Deklarationen


INetURLObject *getURL(String& rStr)
{
    if( rStr.Len() >= 8 && rStr.Compare( "file:/", 6 ) == COMPARE_EQUAL
        && rStr[(USHORT)7] == ':' )
    {
        // Verkehrte JavaURL von file:/?: auf file:///?| bringen;
        String aNew = "file:///";
        aNew += rStr[(USHORT)6];
        aNew += '|';
        aNew += rStr.Erase( 0, 8 );
        rStr = aNew;
        //aStr = "file:///" + aStr[(USHORT)6] + '|' + aStr.Erase( 0, 7 );
    }

    INetURLObject *aDocURL = new INetURLObject();
    aDocURL->SetURL( rStr );
    return aDocURL;
}
/**
    macht aus C++ URL eine JAVA kompatible URL
*/
String getJavaURL(const INetURLObject & rDocBase)
{
    String aURL = rDocBase.GetMainURL(NetURLObject::DECODE_TO_IURI);
    if( aURL.Len() )
    {
        //WorkAround, weil Java mit dem | nicht zurecht kommt
        if( rDocBase.GetProtocol() == INET_PROT_FILE
          && aURL[ (USHORT)9 ] == INET_ENC_DELIM_TOKEN )
            aURL[ (USHORT)9 ] = INET_DELIM_TOKEN;
    }
    return aURL;
}

RmAppletStub::RmAppletStub( ) 	 :
    RmBasisObject()
{
    pClassName = "ApOAppletStub";
}

void RmAppletStub::Create(RmAppletContext* pContext, const INetURLObject& rURL1, const SvCommandList & rList,const INetURLObject& rURL2 , BOOL bMayScript  )
{
    DBG_TRACE1( "Rm: AppletStub::Create() - Befor createObject() - %s", pClassName );

    char * aBuf
        = new char[ strlen( RMAPPLET_PACKAGE ) + strlen( pClassName ) + 1 ];
    strcpy( aBuf, RMAPPLET_PACKAGE ); // #100211# - checked
    strcat( aBuf, pClassName );       // #100211# - checked
    createObject( DCA_NAMESPACE(OObjectBroker, dca)::getBroker("dcabroker"), aBuf );
    delete[] aBuf;

    DBG_TRACE( "Rm: Window::Create() - After createObject()" );

    DCA_NAMESPACE(OCallMethodMsg, dca) aMsg(this, MSG_APPLET_CREATE);
    DCA_NAMESPACE(OObjStream , dca)& s = aMsg.getData();

    s << getJavaURL(rURL1);
    s << getJavaURL(rURL2);

    s << pContext;

    long size = rList.Count();
    s << size;
    for( long i = 0; i < size; i++ )
    {
        const SvCommand & rCmd = rList[i];
        s <<  rCmd.GetCommand().Lower() ;
        s <<  rCmd.GetArgument();

    }

    s << bMayScript;
    aMsg.post();
}


// -----------------------------------------------------------------------

RmAppletContext::RmAppletContext( ) :
    RmBasisObject()
{
    pClassName = "ApOAppletContainer";
}

// -----------------------------------------------------------------------

void RmAppletContext::Create( )
{
    DBG_TRACE1( "Rm: AppletContext::Create() - Befor createObject() - %s", pClassName );

    char * aBuf
        = new char[ strlen( RMAPPLET_PACKAGE ) + strlen( pClassName ) + 1 ];
    strcpy( aBuf, RMAPPLET_PACKAGE ); // #100211# - checked
    strcat( aBuf, pClassName );       // #100211# - checked
    createObject( DCA_NAMESPACE(OObjectBroker, dca)::getBroker("dcabroker"), aBuf );
    delete[] aBuf;

    DBG_TRACE( "Rm: Window::Create() - After createObject()" );

}

// -----------------------------------------------------------------------

RmAppletObject::RmAppletObject( )  	 :
    RmBasisObject()
{
    pClassName = "ApOAppletPanel";
}

void RmAppletObject::Create(const String& sClassName, const String& sName, const RmAppletStub& stub, const Window& parent )
{
    DBG_TRACE1( "Rm: AppletObject::Create() - Befor createObject() - %s", pClassName );

    char * aBuf
        = new char[ strlen( RMAPPLET_PACKAGE ) + strlen( pClassName ) + 1 ];
    strcpy( aBuf, RMAPPLET_PACKAGE ); // #100211# - checked
    strcat( aBuf, pClassName );       // #100211# - checked
    createObject( DCA_NAMESPACE(OObjectBroker, dca)::getBroker("dcabroker"), aBuf );
    delete[] aBuf;

    DBG_TRACE( "Rm: Window::Create() - After createObject()" );
    DCA_NAMESPACE(OCallMethodMsg, dca) aMsg( this, MSG_APPLET_CREATE );
    DCA_NAMESPACE(OObjStream , dca)& s = aMsg.getData();

    s << sClassName;
    s << sName;
    s << (RmAppletStub*)&stub;
    void *dummy = (void *)parent.ImpGetRemoteObject();
    s << (DCA_NAMESPACE(OObj , dca)*)dummy;

    aMsg.post();
}

void  RmAppletObject::Start( const Size & rSize)
{
    DCA_NAMESPACE(OCallMethodMsg, dca) aMsg( this, MSG_APPLET_START );
    DCA_NAMESPACE(OObjStream , dca)& s = aMsg.getData();

    s << rSize.Width();
    s << rSize.Height();

    aMsg.post();

}

void RmAppletObject::Stop()
{
    DCA_NAMESPACE(OCallMethodMsg, dca) aMsg( this, MSG_APPLET_STOP );
    aMsg.post();

}

void RmAppletObject::SetSizePixel( const Size & rSize)
{
    DCA_NAMESPACE(OCallMethodMsg, dca) aMsg( this, MSG_APPLET_SETSIZE );
    DCA_NAMESPACE(OObjStream , dca)& s = aMsg.getData();
    s << rSize.Width();
    s << rSize.Height();
    aMsg.post();

}


/* -------- Basis Object ---------------- */

RmBasisObject::RmBasisObject( )
{
}


Boolean RmBasisObject::dispatchMessage(DCA_NAMESPACE(OCallMethodMsg , dca)& rMsg)
{
    // we have to switch to the main Thread
    MT_Link aLink (LINK((RmBasisObject*) this, RmBasisObject,executeMessage ));
    aLink.Call(&rMsg);
    return 1;
}

IMPL_LINK( RmBasisObject, executeMessage, DCA_NAMESPACE(OCallMethodMsg , dca) *, pMsg )
{
    int                 nType = pMsg->getType();

    switch ( nType )
    {
        case MSG_SHOWSTATUS:
            {
                DBG_TRACE( "Rm: ShowStatus" );
                DCA_NAMESPACE(OObjStream , dca)& s = pMsg->getData();
                String statusText;
                s >> statusText;
                ((RmAppletContext*)this) -> showStatus(statusText);
            }
            break;
        case MSG_GETAPPLET	:
            {
                DBG_TRACE( "Rm: GetApplet" );
                DCA_NAMESPACE(OObjStream , dca)& s = pMsg->getData();
                String sName;
                s >> sName;
                RmAppletObject * pObj = ((RmAppletContext *)this) -> getApplet(sName );
                DCA_NAMESPACE(OObjStream , dca)& rStream = pMsg->getData();
                rStream << pObj;
                pMsg->post();
            }
            break;
        case MSG_GETAPPLETS :
            {
                DBG_TRACE( "Rm: GetApplets" );
                DCA_NAMESPACE(OObjStream , dca)& s = pMsg->getData();
                List rList;
                ((RmAppletContext *) this) -> fillApplets(rList );
                DCA_NAMESPACE(OObjStream , dca)& rStream = pMsg->getData();
                rStream << (long)rList.Count();
                for (int i = 0; i < (int)rList.Count(); i++)
                    rStream << (RmAppletObject*)rList.GetObject(i);
                pMsg->post();
            }
            break;
        case MSG_SHOWDOCUMENT :
            {
                DBG_TRACE( "Rm: ShowDocument" );
                DCA_NAMESPACE(OObjStream , dca)& s = pMsg->getData();
                String sURL;
                String sDest;
                INetURLObject *pURL;
                s >> sURL;
                s >> sDest;
                pURL = getURL(sURL);
                ((RmAppletContext*) this) -> showDocument((*pURL), sDest );
            }
            break;
        case MSG_APPLETRESIZE :
            {
                DBG_TRACE( "Rm: AppletResize" );
                long width;
                long height;
                DCA_NAMESPACE(OObjStream , dca)& s = pMsg->getData();
                s >> width;
                s >> height;
                ((RmAppletStub *) this) -> appletResize(width, height);
            }
            break;
        case MSG_APPLETSTARTED:
            {
                DBG_TRACE( "Rm: AppletStarted" );
                ((RmAppletStub *) this) -> appletStarted();
            }
            break;
        case MSG_APPLETABORT:
            {
                DBG_TRACE( "Rm: AppletAbort" );
                ((RmAppletStub *) this) -> onAppletAbort();
            }
            break;


        default:
            DBG_TRACE1( "Rm: Unknown dispatchMessage (%d)", nType );
            //OObj::dispatchMessage( rMsg );
            break;
    }

    return 1;
}


}

#endif // REMOTE_VERSION

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
