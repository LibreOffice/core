/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: context.h,v $
 * $Revision: 1.8 $
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

#ifndef _BRIDGES_REMOTE_CONTEXT_H_
#define _BRIDGES_REMOTE_CONTEXT_H_
#include <osl/interlck.h>
#include <osl/conditn.h>

#include <typelib/typedescription.h>

#include <uno/environment.h>
#include <uno/any2.h>


/** Interface for refcounted contexts of uno-environments.

    Not all uno_Contexts need to be refcounted, in case they are,
    they should be 'derived' from this struct.
    This is used as a base class for the remote_Context.
    @see uno_Environment
    @see uno_getEnvironment()
    @see remote_Context
 */
struct uno_Context
{
    /** increase the refcount of the context
     */
    void (SAL_CALL * acquire)( uno_Context *pContext );

    /** decrements the recount of the context. In case the recount drops to zero,
        the context gets destroye.
    */
    void (SAL_CALL * release)( uno_Context *pContext);

    /** Allows to query for a different interface of the uno_Context.
        The result of the void pointer is unspecified and depends on the concrete context.
     */
    void * (SAL_CALL *query ) ( uno_Context *pContext , rtl_uString *pId);
};

struct remote_Connection;
struct remote_Context;
struct remote_Interface;



/** performs a query-interface for a certain interface via the remote connection !

  @param pEnvRemote The environment, that shall perform the call.
  @param ppRemoteI  in/out parameter contains the interface returned by queryInterface
  @param pOid       the oid of the 'ghost' object on which the call must be done.
 ***/
typedef void ( SAL_CALL * remote_getInstanceFunc ) (
    uno_Environment *pEnvRemote,
    remote_Interface **ppRemoteI,
    rtl_uString *pOid,
    typelib_TypeDescriptionReference *pInterfaceTypeRef,
    uno_Any **ppException );


/** refcounted C-interface, which provides object by name.
 */
struct remote_InstanceProvider
{
    void (SAL_CALL * acquire ) ( remote_InstanceProvider * pProvider );
    void (SAL_CALL * release ) ( remote_InstanceProvider * pProvider );
    void (SAL_CALL * getInstance ) ( remote_InstanceProvider * pProvider ,
                                     uno_Environment *pEnvRemote,
                                     remote_Interface **ppRemoteI,
                                     rtl_uString *pInstanceName,
                                     typelib_InterfaceTypeDescription *pType,
                                     uno_Any **ppException );
};

/** refcounted C-interface, which allows to register a listener to an
    remote bridge to be informed when the bridge gets disposed.

   @see remote_Context
 */
struct remote_DisposingListener
{
    void (SAL_CALL * acquire     ) ( remote_DisposingListener * pProvider );
    void (SAL_CALL * release     ) ( remote_DisposingListener * pProvider );
    void (SAL_CALL * disposing   ) ( remote_DisposingListener * pProvider,
                                     rtl_uString *pBridgeName );
};


/**
  Try to get an existing context characterized by the pIdString. Each ID-String must
  uniquely charcterize a certain connection. The context can't be retrieved via this
  function anymore, after it got disposed.

  @return 0 when such a context does not exist, otherwise
          a pointer to an acquired remote_Context.
 **/
extern "C" remote_Context * SAL_CALL
remote_getContext( rtl_uString *pIdString );

/**
  Create an acquired remote context. The Context is weakly held by the context administration
  and can be accessed later through remote_getContext() (using the same id-string).

  @param pIdString A string, that uniquely describes the connection. For e.g. a socket connection,
                   host and port of the local and remote host should be in the string.

  @param pDescription
                   Description of the connection, that may brought up to the user.

  @param pProtocol
                   The protocol, that the environment uses for
                   communicating with the remote process.
                   The format of the protocol string is : "protocolname,para1=para1value,..."
  @return 0, when a context with this name already exists.

  @see remote_getContext()
  @see remote_Context
 */
extern "C" remote_Context * SAL_CALL
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

/** Registers a listener at the context administration, which allows to keep
    track of existing remote connections.
    @param pObject object which is handed to the listener function, when called.
 */
extern "C" void SAL_CALL
remote_addContextListener( remote_contextListenerFunc listener, void *pObject );


/** Removes a listener from the context administration.
 */
extern "C" void SAL_CALL
remote_removeContextListener( remote_contextListenerFunc listener , void *pObject );

/** Allows to retrieve all existing context strings.

   @param pnStringCount out parameter. Contains the number of rtl_uStrings in the array
   @param memAlloc      a memory allocation function for the array of  pointers to rtl_uStrings

   @return array of rtl strings. The caller must call release on all rtl_uString s and must free
           the pointer array.
 */
extern "C" rtl_uString ** SAL_CALL
remote_getContextList(
    sal_Int32 *pnStringCount,
    void * ( SAL_CALL * memAlloc ) ( sal_Size nBytesToAlloc ) );


struct remote_BridgeImpl;

/** The context structure for a remote bridge.

    @see uno_getEnvironment()
 */
struct remote_Context
{
    struct uno_Context aBase;

    /**
       These methods are implemented by context administration
     */
    void ( SAL_CALL * addDisposingListener ) ( remote_Context *,
                                               remote_DisposingListener * );
    void ( SAL_CALL * removeDisposingListener ) ( remote_Context *,
                                                  remote_DisposingListener * );
    /**
       will be called by the environment when it gets disposed
     */
    void ( SAL_CALL * dispose ) ( remote_Context * );

    /** The method is set by the remote-environment during environment initialization.
      @see remote_getInstanceFunc
     */
    remote_getInstanceFunc getRemoteInstance;

    /**
      The protocol, that the environment uses for communicating with the remote process.
      The format of the protocol string is : "protocolname,para1=para1value,..."
      The parameters are protocol dependend
     */
    rtl_uString *m_pProtocol;

    /**
       It may be the same as m_pName.
       Livetime is handled by the context administration.
     */
    rtl_uString *m_pDescription;

    /**
      The name of this context at context administration.
      A string, that uniquely describes this environment.
      Livetime is handled by the context administration.
     */
    rtl_uString *m_pName;

    /** The instance-provider, which is used to look up unknown object identifiers.
      Is usually called on server side,  when the first client request comes in.
      Maybe 0. Livetime is handled by the context administration.
     */
    remote_InstanceProvider *m_pInstanceProvider;

    /**
       The connection of this context.
       Livetime is handled by the context administration.
     */
    remote_Connection *m_pConnection;

    /**
      Here arbitrary data may be stored. It may be used by a connection
      service to store environment specific data. The bridge does not
      use it.
     */
    void *m_pAdditionalInformation;

    /**
      here the bridge stores its private per environment data.
     */
    struct remote_BridgeImpl *m_pBridgeImpl;
};

#endif

