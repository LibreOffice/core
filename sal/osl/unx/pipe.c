/*************************************************************************
 *
 *  $RCSfile: pipe.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-19 13:29:30 $
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


#include "system.h"

#include <osl/pipe.h>
#include <osl/diagnose.h>
#include <osl/signal.h>

#define PIPEDEFAULTPATH     "/tmp"
#define PIPEALTERNATEPATH   "/var/tmp"

#define PIPENAMEMASK    "OSL_PIPE_%s"
#define SECPIPENAMEMASK "OSL_PIPE_%s_%s"

sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax);
oslPipe SAL_CALL osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options, oslSecurity Security);

/*#define DEBUG_OSL_PIPE*/
/*#define TRACE_OSL_PIPE*/


/*****************************************************************************/
/* enum oslPipeImpl */
/*****************************************************************************/

typedef struct _oslPipeImpl {
    int  m_Socket;
    sal_Char m_Name[PATH_MAX + 1];
#if defined(LINUX)
    sal_Bool bInShutDown;
#endif
} oslPipeImpl;


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

oslPipeImpl* __osl_createPipeImpl()
{
    oslPipeImpl* pPipeImpl;

    pPipeImpl = (oslPipeImpl*)calloc(1, sizeof(oslPipeImpl));

    return (pPipeImpl);
}

void __osl_destroyPipeImpl(oslPipeImpl *pImpl)
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
    rtl_String* strPipeName=0;
    sal_Char* pszPipeName=0;

    if ( ustrPipeName != 0 )
    {
        rtl_uString2String( &strPipeName,
                            rtl_uString_getStr(ustrPipeName),
                            rtl_uString_getLength(ustrPipeName),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszPipeName = rtl_string_getStr(strPipeName);
        pPipe = osl_psz_createPipe(pszPipeName, Options, Security);

        if ( strPipeName != 0 )
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
    struct sockaddr_un addr;

    sal_Char     name[PATH_MAX + 1] = "";
    oslPipeImpl* pPipeImpl;

    if (access(PIPEDEFAULTPATH, O_RDWR) == 0)
    {
        strcpy(name, PIPEDEFAULTPATH);
    }
    else
    {
        strcpy(name, PIPEALTERNATEPATH);
    }


    strcat(name, "/");

    if (Security)
    {
        sal_Char Ident[256] = "";

        OSL_VERIFY(osl_psz_getUserIdent(Security, Ident, sizeof(Ident)));

        sprintf(&name[strlen(name)], SECPIPENAMEMASK, Ident, pszPipeName);
    }
    else
    {
        sprintf(&name[strlen(name)], PIPENAMEMASK, pszPipeName);
    }


    /* alloc memory */
    pPipeImpl= __osl_createPipeImpl();

    /* create socket */
    pPipeImpl->m_Socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( pPipeImpl->m_Socket < 0 )
    {
        OSL_TRACE("osl_createPipe socket failed. Errno: %d; %s\n",errno, strerror(errno));
        __osl_destroyPipeImpl(pPipeImpl);
        return NULL;
    }

/*    OSL_TRACE("osl_createPipe : new Pipe on fd %i\n",pPipeImpl->m_Socket);*/

    /* set close-on-exec flag */
    if ((Flags = fcntl(pPipeImpl->m_Socket, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(pPipeImpl->m_Socket, F_SETFD, Flags) == -1)
        {
            OSL_TRACE("osl_createPipe failed changing socket flags. Errno: %d; %s\n",errno,strerror(errno));
        }
    }

    memset(&addr, 0, sizeof(addr));

    OSL_TRACE("osl_createPipe : Pipe Name '%s'",name);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, name);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);

    if ( Options & osl_Pipe_CREATE )
    {
        struct stat status;

        /* check if there exists an orphan filesystem entry */
        if ( ( stat(name, &status) == 0) &&
             ( S_ISSOCK(status.st_mode) || S_ISFIFO(status.st_mode) ) )
        {
            if ( connect(pPipeImpl->m_Socket,(struct sockaddr *)&addr,len) >= 0 )
            {
                OSL_TRACE("osl_createPipe : Pipe already in use. Errno: %d; %s\n",errno,strerror(errno));
                close (pPipeImpl->m_Socket);
                __osl_destroyPipeImpl(pPipeImpl);
                return NULL;
            }

            unlink(name);
        }

        /* ok, fs clean */
        if ( bind(pPipeImpl->m_Socket, (struct sockaddr *)&addr, len) < 0 )
        {
            OSL_TRACE("osl_createPipe : failed to bind socket. Errno: %d; %s\n",errno,strerror(errno));
            close (pPipeImpl->m_Socket);
            __osl_destroyPipeImpl(pPipeImpl);
            return NULL;
        }
        chmod(name,S_IRWXU | S_IRWXG |S_IRWXO);


        strcpy(pPipeImpl->m_Name, name);

        if ( listen(pPipeImpl->m_Socket, 5) < 0 )
        {
            OSL_TRACE("osl_createPipe failed to listen. Errno: %d; %s\n",errno,strerror(errno));
            unlink(name);   /* remove filesystem entry */
            close (pPipeImpl->m_Socket);
            __osl_destroyPipeImpl(pPipeImpl);
            return NULL;
        }

        return (pPipeImpl);
    }
    else
    {   /* osl_pipe_OPEN */
        if ( access(name, F_OK) != -1 )
        {
            if ( connect( pPipeImpl->m_Socket, (struct sockaddr *)&addr, len) >= 0 )
            {
                return (pPipeImpl);
            }

            OSL_TRACE("osl_createPipe failed to connect. Errno: %d; %s\n",errno,strerror(errno));
        }

        close (pPipeImpl->m_Socket);
        __osl_destroyPipeImpl(pPipeImpl);
        return NULL;
    }

    /* if we reach here something went wrong */
    /* should never come here */
    close (pPipeImpl->m_Socket);
    __osl_destroyPipeImpl(pPipeImpl);
    return NULL;
}

/*****************************************************************************/
/* osl_copyPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_copyPipe(oslPipe Pipe)
{
    oslPipeImpl* pPipeImpl;
    oslPipeImpl* pParamPipeImpl;

    OSL_ASSERT(Pipe);

    if ( Pipe == 0 )
    {
        return 0;
    }

    /* alloc memory */
    pPipeImpl= __osl_createPipeImpl();

    OSL_ASSERT(pPipeImpl);

    if ( pPipeImpl == 0 )
    {
        return 0;
    }

    pParamPipeImpl= (oslPipeImpl*)Pipe;

    /* copy socket */
    memcpy(pPipeImpl, pParamPipeImpl, sizeof(oslPipeImpl));

    return (oslPipe)pPipeImpl;
}


/*****************************************************************************/
/* osl_destroyPipe  */
/*****************************************************************************/
void SAL_CALL osl_destroyPipe(oslPipe Pipe)
{
    oslPipeImpl* pPipeImpl;
    int nRet;
#if defined(LINUX)
    size_t     len;
    struct sockaddr_un addr;
    int fd;
#endif
    int ConnFD;

    /* socket already invalid */
    if( Pipe == NULL )
        return;

    OSL_TRACE("In  osl_destroyPipe");

    pPipeImpl = (oslPipeImpl*) Pipe;

    ConnFD = pPipeImpl->m_Socket;

    nRet = shutdown(ConnFD, 2);
    if ( nRet < 0 )
    {
        OSL_TRACE("shutdown in destroyPipe failed : '%s'\n",strerror(errno));
    }

#if defined(LINUX)
    pPipeImpl->m_Socket = -1;
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));

    OSL_TRACE("osl_destroyPipe : Pipe Name '%s'",pPipeImpl->m_Name);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, pPipeImpl->m_Name);
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);

    nRet = connect( fd, (struct sockaddr *)&addr, len);
#if defined(DEBUG)
    if ( nRet < 0 )
    {
        perror("connect in osl_destroyPipe");
    }
#endif
    close(fd);
#endif

    nRet = close(ConnFD);
    if ( nRet < 0 )
    {
        OSL_TRACE("close in destroyPipe failed : '%s'\n",strerror(errno));
    }
    /* remove filesystem entry */
    if ( strlen(pPipeImpl->m_Name) > 0 )
    {
        unlink(pPipeImpl->m_Name);
    }

    /* free memory */
    __osl_destroyPipeImpl(pPipeImpl);

    OSL_TRACE("Out osl_destroyPipe");
}

/*****************************************************************************/
/* osl_acceptPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_acceptPipe(oslPipe Pipe)
{
    int          s;
    oslPipeImpl* pPipeImpl;
    oslPipeImpl* pParamPipeImpl;
    OSL_ASSERT(Pipe);

    if ( Pipe == 0 )
    {
        return 0;
    }

    pParamPipeImpl= (oslPipeImpl*)Pipe;

    OSL_ASSERT(strlen(pParamPipeImpl->m_Name) > 0);

    if ((s = accept(pParamPipeImpl->m_Socket, NULL, NULL)) < 0)
    {
        return NULL;
    }
    else
    {
        /* alloc memory */
        pPipeImpl= __osl_createPipeImpl();

        OSL_ASSERT(pPipeImpl);

        if(pPipeImpl==NULL)
            return NULL;

        pPipeImpl->m_Socket = s;
    }


    return (oslPipe)pPipeImpl;
}

/*****************************************************************************/
/* osl_receivePipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receivePipe(oslPipe Pipe,
                        void* pBuffer,
                        sal_uInt32 BytesToRead)
{
    oslPipeImpl* pPipeImpl;
    int nRet = 0;

    OSL_ASSERT(Pipe);

    if ( Pipe == 0 )
    {
        OSL_TRACE("osl_receivePipe : Invalid socket");
        errno=EINVAL;
        return -1;
    }

    pPipeImpl= (oslPipeImpl*)Pipe;

/*    OSL_TRACE("osl_receivePipe : receiving %i bytes on fd %i\n",BytesToRead,pPipeImpl->m_Socket);*/

    nRet = recv(pPipeImpl->m_Socket,
                  (sal_Char*)pBuffer,
                  BytesToRead, 0);


    if ( nRet <= 0 )
    {
        OSL_TRACE("osl_receivePipe failed : %i '%s'",nRet,strerror(errno));
    }

/*      fprintf(stderr,"osl_receivePipe : received "); */
/*      for ( i = 0 ; i < nRet  ; i++ ) */
/*      { */
/*          fprintf(stderr," '%c'",((sal_Char*)pBuffer)[i]); */
/*      } */
/*      fprintf(stderr,"\n"); */

      return nRet;
}


/*****************************************************************************/
/* osl_sendPipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendPipe(oslPipe Pipe,
                       const void* pBuffer,
                       sal_uInt32 BytesToSend)
{
    oslPipeImpl* pPipeImpl;
    int nRet=0;

    OSL_ASSERT(Pipe);

    if ( Pipe == 0 )
    {
        OSL_TRACE("osl_sendPipe : Invalid socket");
        errno=EINVAL;
        return -1;
    }

    pPipeImpl= (oslPipeImpl*)Pipe;

/*    OSL_TRACE("osl_sendPipe : sending %i bytes on fd %i\n",BytesToSend,pPipeImpl->m_Socket);*/

    nRet = send(pPipeImpl->m_Socket,
                  (sal_Char*)pBuffer,
                  BytesToSend, 0);


    if ( nRet <= 0 )
    {
        OSL_TRACE("osl_sendPipe failed : %i '%s'",nRet,strerror(errno));
    }

/*      fprintf(stderr,"osl_sendPipe : sended "); */
/*      for ( i = 0 ; i < nRet  ; i++ ) */
/*      { */
/*          fprintf(stderr," '%c'",((sal_Char*)pBuffer)[i]); */
/*      } */
/*      fprintf(stderr,"\n"); */

     return nRet;
}


/*****************************************************************************/
/* osl_getLastPipeError  */
/*****************************************************************************/
oslPipeError SAL_CALL osl_getLastPipeError(oslPipe Pipe)
{
    return ERROR_FROM_NATIVE(errno);
}


