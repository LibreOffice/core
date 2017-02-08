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




#include "system.h"

#include <osl/pipe.h>
#include <osl/diagnose.h>
/*#include <osl/signal.h>*/
#include <osl/thread.h>
#include <osl/interlck.h>

#include "sockimpl.h"

#define PIPEDEFAULTPATH     "/tmp"
#define PIPEALTERNATEPATH   "/var/tmp"

#define PIPENAMEMASK    "%s/OSL_PIPE_%s"
#define SECPIPENAMEMASK "%s/OSL_PIPE_%s_%s"

sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax);
oslPipe SAL_CALL osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options, oslSecurity Security);

/*#define DEBUG_OSL_PIPE*/
/*#define TRACE_OSL_PIPE*/


/*****************************************************************************/
/* enum oslPipeError */
/*****************************************************************************/

static struct
{
    int            errcode;
    oslPipeError   error;
} PipeError[]= {
    { 0,               osl_Pipe_E_None              },  /* no error */
    { EPROTOTYPE,      osl_Pipe_E_NoProtocol        },  /* Protocol wrong type for socket */
    { ENOPROTOOPT,     osl_Pipe_E_NoProtocol        },  /* Protocol not available */
    { EPROTONOSUPPORT, osl_Pipe_E_NoProtocol        },  /* Protocol not supported */
    { ESOCKTNOSUPPORT, osl_Pipe_E_NoProtocol        },  /* Socket type not supported */
    { EPFNOSUPPORT,    osl_Pipe_E_NoProtocol        },  /* Protocol family not supported */
    { EAFNOSUPPORT,    osl_Pipe_E_NoProtocol        },  /* Address family not supported by */
                                                        /* protocol family */
    { ENETRESET,       osl_Pipe_E_NetworkReset      },  /* Network dropped connection because */
                                                         /* of reset */
    { ECONNABORTED,    osl_Pipe_E_ConnectionAbort   },  /* Software caused connection abort */
    { ECONNRESET,      osl_Pipe_E_ConnectionReset   },  /* Connection reset by peer */
    { ENOBUFS,         osl_Pipe_E_NoBufferSpace     },  /* No buffer space available */
    { ETIMEDOUT,       osl_Pipe_E_TimedOut          },  /* Connection timed out */
    { ECONNREFUSED,    osl_Pipe_E_ConnectionRefused },  /* Connection refused */
    { -1,              osl_Pipe_E_invalidError      }
};


/* map */
/* mfe: NOT USED
   static int osl_NativeFromPipeError(oslPipeError errorCode)
   {
   int i = 0;

   while ((PipeError[i].error != osl_Pipe_E_invalidError) &&
   (PipeError[i].error != errorCode)) i++;

   return PipeError[i].errcode;

   }
*/

/* reverse map */
static oslPipeError osl_PipeErrorFromNative(int nativeType)
{
    int i = 0;

    while ((PipeError[i].error != osl_Pipe_E_invalidError) &&
           (PipeError[i].errcode != nativeType)) i++;

    return PipeError[i].error;
}


/* macros */
#define ERROR_TO_NATIVE(x)      osl_NativeFromPipeError(x)
#define ERROR_FROM_NATIVE(y)    osl_PipeErrorFromNative(y)


/*****************************************************************************/
/* osl_create/destroy-PipeImpl */
/*****************************************************************************/

oslPipe __osl_createPipeImpl()
{
    oslPipe pPipeImpl;

    pPipeImpl = (oslPipe)calloc(1, sizeof(struct oslPipeImpl));
    pPipeImpl->m_nRefCount =1;
    pPipeImpl->m_bClosed = sal_False;
#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    pPipeImpl->m_bIsInShutdown = sal_False;
    pPipeImpl->m_bIsAccepting = sal_False;
#endif
    return pPipeImpl;
}

void __osl_destroyPipeImpl(oslPipe pImpl)
{
    if (pImpl != NULL)
        free(pImpl);
}


/*****************************************************************************/
/* osl_createPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_createPipe(rtl_uString *ustrPipeName, oslPipeOptions Options, oslSecurity Security)
{
    oslPipe pPipe=0;
    rtl_String* strPipeName=NULL;
    sal_Char* pszPipeName=NULL;

    if ( ustrPipeName != NULL )
    {
        rtl_uString2String( &strPipeName,
                            rtl_uString_getStr(ustrPipeName),
                            rtl_uString_getLength(ustrPipeName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszPipeName = rtl_string_getStr(strPipeName);
        pPipe = osl_psz_createPipe(pszPipeName, Options, Security);

        if ( strPipeName != NULL )
        {
            rtl_string_release(strPipeName);
        }
    }

    return pPipe;

}

oslPipe SAL_CALL osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options,
                       oslSecurity Security)
{
    int    Flags;
    size_t     len;
    union
    {
        struct sockaddr addr;
        struct sockaddr_un addr_un;
    } s;

    sal_Char     name[PATH_MAX + 1];
    const sal_Char   *pPath;
    oslPipe  pPipe;

    if (access(PIPEDEFAULTPATH, R_OK|W_OK) == 0)
    {
        pPath = PIPEDEFAULTPATH;
    }
    else
    {
        pPath = PIPEALTERNATEPATH;
    }

    if (Security)
    {
        sal_Char Ident[256];

        Ident[0] = '\0';

        OSL_VERIFY(osl_psz_getUserIdent(Security, Ident, sizeof(Ident)));

        snprintf(name, sizeof(name), SECPIPENAMEMASK, pPath, Ident, pszPipeName);
    }
    else
    {
        snprintf(name, sizeof(name), PIPENAMEMASK, pPath, pszPipeName);
    }


    /* alloc memory */
    pPipe= __osl_createPipeImpl();

    /* create socket */
    pPipe->m_Socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( pPipe->m_Socket < 0 )
    {
        OSL_TRACE("osl_createPipe socket failed. Errno: %d; %s\n",errno, strerror(errno));
        __osl_destroyPipeImpl(pPipe);
        return NULL;
    }

/*    OSL_TRACE("osl_createPipe : new Pipe on fd %i\n",pPipe->m_Socket);*/

    /* set close-on-exec flag */
    if ((Flags = fcntl(pPipe->m_Socket, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(pPipe->m_Socket, F_SETFD, Flags) == -1)
        {
            OSL_TRACE("osl_createPipe failed changing socket flags. Errno: %d; %s\n",errno,strerror(errno));
        }
    }

    memset(&s.addr_un, 0, sizeof(s.addr_un));

    OSL_TRACE("osl_createPipe : Pipe Name '%s'",name);

    s.addr_un.sun_family = AF_UNIX;
    strncpy(s.addr_un.sun_path, name, sizeof(s.addr_un.sun_path));
#if defined(FREEBSD)
    len = SUN_LEN(&s.addr_un);
#else
    len = sizeof(s.addr_un);
#endif

    if ( Options & osl_Pipe_CREATE )
    {
        struct stat status;

        /* check if there exists an orphan filesystem entry */
        if ( ( stat(name, &status) == 0) &&
             ( S_ISSOCK(status.st_mode) || S_ISFIFO(status.st_mode) ) )
        {
            if ( connect(pPipe->m_Socket,&s.addr,len) >= 0 )
            {
                OSL_TRACE("osl_createPipe : Pipe already in use. Errno: %d; %s\n",errno,strerror(errno));
                close (pPipe->m_Socket);
                __osl_destroyPipeImpl(pPipe);
                return NULL;
            }

            unlink(name);
        }

        /* ok, fs clean */
        if ( bind(pPipe->m_Socket, &s.addr, len) < 0 )
        {
            OSL_TRACE("osl_createPipe : failed to bind socket. Errno: %d; %s\n",errno,strerror(errno));
            close (pPipe->m_Socket);
            __osl_destroyPipeImpl(pPipe);
            return NULL;
        }

        /*  Only give access to all if no security handle was specified, otherwise security
            depends on umask */

        if ( !Security )
            chmod(name,S_IRWXU | S_IRWXG |S_IRWXO);


        strncpy(pPipe->m_Name, name, sizeof(pPipe->m_Name));

        if ( listen(pPipe->m_Socket, 5) < 0 )
        {
            OSL_TRACE("osl_createPipe failed to listen. Errno: %d; %s\n",errno,strerror(errno));
            unlink(name);   /* remove filesystem entry */
            close (pPipe->m_Socket);
            __osl_destroyPipeImpl(pPipe);
            return NULL;
        }

        return (pPipe);
    }
    else
    {   /* osl_pipe_OPEN */
        if ( access(name, F_OK) != -1 )
        {
            if ( connect( pPipe->m_Socket, &s.addr, len) >= 0 )
            {
                return (pPipe);
            }

            OSL_TRACE("osl_createPipe failed to connect. Errno: %d; %s\n",errno,strerror(errno));
        }

        close (pPipe->m_Socket);
        __osl_destroyPipeImpl(pPipe);
        return NULL;
    }
}

void SAL_CALL osl_acquirePipe( oslPipe pPipe )
{
    osl_incrementInterlockedCount( &(pPipe->m_nRefCount) );
}

void SAL_CALL osl_releasePipe( oslPipe pPipe )
{

    if( 0 == pPipe )
        return;

    if( 0 == osl_decrementInterlockedCount( &(pPipe->m_nRefCount) ) )
    {
        if( ! pPipe->m_bClosed )
            osl_closePipe( pPipe );

        __osl_destroyPipeImpl( pPipe );
    }
}

void SAL_CALL osl_closePipe( oslPipe pPipe )
{
    int nRet;
#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    size_t     len;
    union
    {
        struct sockaddr_un addr_un;
        struct sockaddr addr;
    } s;
    int fd;
#endif
    int ConnFD;

    if( ! pPipe )
    {
        return;
    }

    if( pPipe->m_bClosed )
    {
        return;
    }

    ConnFD = pPipe->m_Socket;

    /*
      Thread does not return from accept on some operating systems, so
      connect to the accepting pipe
     */
#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    if ( pPipe->m_bIsAccepting )
    {
        pPipe->m_bIsInShutdown = sal_True;
        pPipe->m_Socket = -1;
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        memset(&s.addr_un, 0, sizeof(s.addr_un));

        OSL_TRACE("osl_destroyPipe : Pipe Name '%s'",pPipe->m_Name);

        s.addr_un.sun_family = AF_UNIX;
        strncpy(s.addr_un.sun_path, pPipe->m_Name, sizeof(s.addr_un.sun_path));
#if defined(FREEBSD)
        len = SUN_LEN(&s.addr_un);
#else
        len = sizeof(s.addr_un);
#endif

        nRet = connect( fd, &s.addr, len);
#if OSL_DEBUG_LEVEL > 1
        if ( nRet < 0 )
        {
            perror("connect in osl_destroyPipe");
        }
#endif /* OSL_DEBUG_LEVEL */
        close(fd);
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */


    nRet = shutdown(ConnFD, 2);
    if ( nRet < 0 )
    {
        OSL_TRACE("shutdown in destroyPipe failed : '%s'\n",strerror(errno));
    }

    nRet = close(ConnFD);
    if ( nRet < 0 )
    {
        OSL_TRACE("close in destroyPipe failed : '%s'\n",strerror(errno));
    }
    /* remove filesystem entry */
    if ( strlen(pPipe->m_Name) > 0 )
    {
        unlink(pPipe->m_Name);
    }
    pPipe->m_bClosed = sal_True;

/*      OSL_TRACE("Out osl_destroyPipe");     */
}


/*****************************************************************************/
/* osl_acceptPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_acceptPipe(oslPipe pPipe)
{
    int     s, flags;
    oslPipe pAcceptedPipe;

    OSL_ASSERT(pPipe);
    if ( pPipe == 0 )
    {
        return NULL;
    }

    OSL_ASSERT(strlen(pPipe->m_Name) > 0);

#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    pPipe->m_bIsAccepting = sal_True;
#endif

    s = accept(pPipe->m_Socket, NULL, NULL);

#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    pPipe->m_bIsAccepting = sal_False;
#endif

    if (s < 0)
    {
        OSL_TRACE("osl_acceptPipe : accept error '%s'", strerror(errno));
        return NULL;
    }

#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    if ( pPipe->m_bIsInShutdown  )
    {
        close(s);
        return NULL;
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */
    else
    {
        /* alloc memory */
        pAcceptedPipe= __osl_createPipeImpl();

        OSL_ASSERT(pAcceptedPipe);
        if(pAcceptedPipe==NULL)
        {
            close(s);
            return NULL;
        }

        /* set close-on-exec flag */
        if (!((flags = fcntl(s, F_GETFD, 0)) < 0))
        {
            flags |= FD_CLOEXEC;
            if (fcntl(s, F_SETFD, flags) < 0)
            {
                OSL_TRACE("osl_acceptPipe: error changing socket flags. "
                          "Errno: %d; %s",errno,strerror(errno));
            }
        }

        pAcceptedPipe->m_Socket = s;
    }

    return pAcceptedPipe;
}

/*****************************************************************************/
/* osl_receivePipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receivePipe(oslPipe pPipe,
                        void* pBuffer,
                        sal_Int32 BytesToRead)
{
    int nRet = 0;

    OSL_ASSERT(pPipe);

    if ( pPipe == 0 )
    {
        OSL_TRACE("osl_receivePipe : Invalid socket");
        errno=EINVAL;
        return -1;
    }

    nRet = recv(pPipe->m_Socket,
                  (sal_Char*)pBuffer,
                  BytesToRead, 0);

    if ( nRet < 0 )
    {
        OSL_TRACE("osl_receivePipe failed : %i '%s'",nRet,strerror(errno));
    }

      return nRet;
}


/*****************************************************************************/
/* osl_sendPipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendPipe(oslPipe pPipe,
                       const void* pBuffer,
                       sal_Int32 BytesToSend)
{
    int nRet=0;

    OSL_ASSERT(pPipe);

    if ( pPipe == 0 )
    {
        OSL_TRACE("osl_sendPipe : Invalid socket");
        errno=EINVAL;
        return -1;
    }

    nRet = send(pPipe->m_Socket,
                  (sal_Char*)pBuffer,
                  BytesToSend, 0);


    if ( nRet <= 0 )
    {
        OSL_TRACE("osl_sendPipe failed : %i '%s'",nRet,strerror(errno));
    }

     return nRet;
}


/*****************************************************************************/
/* osl_getLastPipeError  */
/*****************************************************************************/
oslPipeError SAL_CALL osl_getLastPipeError(oslPipe pPipe)
{
    (void) pPipe; /* unused */
    return ERROR_FROM_NATIVE(errno);
}


sal_Int32 SAL_CALL osl_writePipe( oslPipe pPipe, const void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were send or an error occurred */
    sal_Int32 BytesSend= 0;
    sal_Int32 BytesToSend= n;

    OSL_ASSERT(pPipe);
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendPipe(pPipe, pBuffer, BytesToSend);

        /* error occurred? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }

    return BytesSend;
}

sal_Int32 SAL_CALL osl_readPipe( oslPipe pPipe, void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were read or an error occurred */
    sal_Int32 BytesRead= 0;
    sal_Int32 BytesToRead= n;

    OSL_ASSERT( pPipe );
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe(pPipe, pBuffer, BytesToRead);

        /* error occurred? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }
    return BytesRead;
}


