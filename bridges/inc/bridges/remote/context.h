/*************************************************************************
 *
 *  $RCSfile: context.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:48 $
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

#ifndef _BRIDGES_REMOTE_CONTEXT_H_
#define _BRIDGES_REMOTE_CONTEXT_H_
#include <osl/interlck.h>
#include <osl/conditn.h>

#include <typelib/typedescription.h>

#include <uno/environment.h>

struct uno_Context
{
    void (SAL_CALL * acquire)( uno_Context *pContext );

    void (SAL_CALL * release)( uno_Context *pContext);

    void * (SAL_CALL *query ) ( uno_Context *pContext , rtl_uString *pId);
};

struct remote_Connection;
struct remote_Context;
struct remote_Interface;


/*****
 * @deprecated
 ****/
typedef void ( SAL_CALL * remote_createStubFunc ) (
    remote_Interface **ppRemoteI,
    rtl_uString *pOid ,
    typelib_TypeDescriptionReference *pTypeRef,
    uno_Environment *pEnvRemote );

/***
 * performs a query-interface for a certain interface via the remote connection !
 *
 * @param pEnvRemote The environment, that shall perform the call.
 * @param ppRemoteI  in/out parameter contains the interface returned by queryInterface
 * @param pOid       the oid of the 'ghost' object on which the call must be done.
 ***/
typedef void ( SAL_CALL * remote_getInstanceFunc ) (
    uno_Environment *pEnvRemote,
    remote_Interface **ppRemoteI,
    rtl_uString *pOid,
    typelib_TypeDescriptionReference *pInterfaceTypeRef);

struct remote_InstanceProvider
{
    void (SAL_CALL * acquire ) ( remote_InstanceProvider * pProvider );
    void (SAL_CALL * release ) ( remote_InstanceProvider * pProvider );
    void (SAL_CALL * getInstance ) ( remote_InstanceProvider * pProvider ,
                                     uno_Environment *pEnvRemote,
                                     remote_Interface **ppRemoteI,
                                     rtl_uString *pInstanceName,
                                     typelib_InterfaceTypeDescription *pType
                                     );
};

struct remote_DisposingListener
{
    void (SAL_CALL * acquire     ) ( remote_DisposingListener * pProvider );
    void (SAL_CALL * release     ) ( remote_DisposingListener * pProvider );
    void (SAL_CALL * disposing   ) ( remote_DisposingListener * pProvider,
                                     rtl_uString *pBridgeName );
};


/**
 * Try to get an existing context characterized by the IDString. Each ID-String must
 * uniquely charcterize a certain connection.
 *
 * @return 0 when such a context does not exist, otherwise
 *         a pointer to an acquired uno remote context
 **/
extern "C" SAL_DLLEXPORT remote_Context * SAL_CALL
remote_getContext( rtl_uString *pIdString );

/**
 * Create an acquired remote context. The Context is weakly held by the context administration
 * and can be accessed later through getRemoteContext() (using the same id-string).
 *
 * @param pIdString A string, that uniquely describes the connection. For e.g. a socket connection,
 *                  host and port of the local and remote host should be in the string.
 *
 * @param pDescription
 *                  Description of the connection, that may brought up to the user.
 *
 * @param pProtocol
 *                  The protocol, that the environment uses for
 *                  communicating with the remote process.
 *                  The format of the protocol string is : "protocolname,para1=para1value,..."
 * @return 0, when a context with this name already exists.
 **/
extern "C" SAL_DLLEXPORT remote_Context * SAL_CALL
remote_createContext( remote_Connection *pConnection,
                      rtl_uString *pIdStr,
                      rtl_uString *pDescription,
                      rtl_uString *pProtocol,
                      remote_InstanceProvider *);

const sal_Int32 REMOTE_CONTEXT_CREATE = 1;
const sal_Int32 REMOTE_CONTEXT_DESTROY = 2;

typedef void ( SAL_CALL * remote_contextListenerFunc ) (
    void *pThis,
    sal_Int32 nRemoteContextMode,
    rtl_uString *sName,
    rtl_uString *sDescription
    );


extern "C" SAL_DLLEXPORT void SAL_CALL
remote_addContextListener( remote_contextListenerFunc listener, void *pObject );

extern "C" SAL_DLLEXPORT void SAL_CALL
remote_removeContextListener( remote_contextListenerFunc listener , void *pObject );

/****
 * @param pnStringCount out parameter. Contains the number of rtl_uStrings in the array
 * @param memAlloc      a memory allocation function for the array of  pointers to rtl_uStrings
 *
 * @return array of rtl strings. The caller must call release on all rtl_uString s and must free
 *         the pointer array.
 *
 ***/
extern "C" SAL_DLLEXPORT rtl_uString ** SAL_CALL
remote_getContextList(
    sal_Int32 *pnStringCount,
    void * ( SAL_CALL * memAlloc ) ( sal_uInt32 nBytesToAlloc ) );


struct remote_BridgeImpl;

struct remote_Context
{
    struct uno_Context aBase;

    /***
     * These methods are implemented by context administration
     ***/
    void ( SAL_CALL * addDisposingListener ) ( remote_Context *,
                                               remote_DisposingListener * );
    void ( SAL_CALL * removeDisposingListener ) ( remote_Context *,
                                                  remote_DisposingListener * );
    /***
     * will be called by the environment when it gets disposed
     ***/
    void ( SAL_CALL * dispose ) ( remote_Context * );

    /********
     * see above declaration of remote_getInstanceFunc
     * The method is set by the environment during environment initialization.
     *******/
    remote_getInstanceFunc getRemoteInstance;

    /*******
     * The protocol, that the environment uses for communicating with the remote process.
     * The format of the protocol string is : "protocolname,para1=para1value,..."
     * The parameters are protocol dependend
     ******/
    rtl_uString *m_pProtocol;

    /****
     * It may be the same as m_pName.
     * Livetime is handled by the context administration.
     ****/
    rtl_uString *m_pDescription;

    /****
     * The name of this context at context administration.
     * A string, that uniquely describes this environment.
     * Livetime is handled by the context administration.
     ****/
    rtl_uString *m_pName;

    /**
     * GetInstance method is called every time when a remote call with an unknown oid comes in.
     * Is usually called on server side,  when the first client request comes in.
     * Maybe 0. Livetime is handled by the context administration.
     ***/
    remote_InstanceProvider *m_pInstanceProvider;

    /***
     * The connection of this context.
     * Livetime is handled by the context administration.
     ***/
    remote_Connection *m_pConnection;

    /**
     * Here arbitrary data may be stored. It may be used by a connection
     * service to store environment specific data. The bridge does not
     * use it.
     **/
    void *m_pAdditionalInformation;

    /**
     * here the bridge stores its private per environment data.
     **/
    struct remote_BridgeImpl *m_pBridgeImpl;
};

#endif

