/*************************************************************************
 *
 *  $RCSfile: process.c,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:46:04 $
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
 *osl_getExecut
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


/*
 *   ToDo:
 *      - cleanup of process status things
 *      - cleanup/reimplementation of argument stuff
 *      - cleanup of process spawning
 *      - cleanup of resource transfer
 */


#ifdef LINUX
#include <asm/param.h>
#endif

#ifdef FREEBSD
#include <machine/param.h>
#endif

#include "system.h"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_H_
#include <osl/mutex.h>
#endif

#ifndef _OSL_CONDITN_H_
#include <osl/conditn.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

#ifndef _OSL_SIGNAL_H_
#include <osl/signal.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#include <grp.h>

#include "procimpl.h"
#include "sockimpl.h"
#include "secimpl.h"


#define MAX_ARGS        255
#define MAX_ENVS        255

#if defined(MACOSX) || defined(IORESOURCE_TRANSFER_BSD)
#define CONTROLLEN (sizeof(struct cmsghdr) + sizeof(int))
#endif

/* implemented in file.c */
extern oslFileError FileURLToPath( char *, size_t, rtl_uString* );
extern oslFileHandle osl_createFileHandleFromFD( int fd );


extern void _imp_getProcessLocale( rtl_Locale ** );
extern int  _imp_setProcessLocale( rtl_Locale * );

/******************************************************************************
 *
 *                  Data Type Definition
 *
 ******************************************************************************/

typedef struct {
    int  m_hPipe;
    int  m_hConn;
    sal_Char m_Name[PATH_MAX + 1];
} Pipe;

typedef struct {
    const sal_Char*  m_pszArgs[MAX_ARGS + 1];
    oslProcessOption m_options;
    const sal_Char*  m_pszDir;
    const sal_Char*  m_pszEnv[MAX_ENVS + 1];
    uid_t            m_uid;
    gid_t            m_gid;
    sal_Char*        m_name;
    oslCondition     m_started;
    oslProcessImpl*  m_pProcImpl;
    oslFileHandle    *m_pInputWrite;
    oslFileHandle    *m_pOutputRead;
    oslFileHandle    *m_pErrorRead;
} ProcessData;

typedef struct _oslPipeImpl {
    int      m_Socket;
    sal_Char m_Name[PATH_MAX + 1];
} oslPipeImpl;


/******************************************************************************
 *
 *                  Function Declarations
 *
 *****************************************************************************/

extern sal_Char *getCmdLine();

oslProcessError SAL_CALL osl_psz_executeProcess(sal_Char *pszImageName,
                                                sal_Char *pszArguments[],
                                                oslProcessOption Options,
                                                oslSecurity Security,
                                                sal_Char *pszDirectory,
                                                sal_Char *pszEnvironments[],
                                                oslProcess *pProcess,
                                                oslFileHandle *pInputWrite,
                                                oslFileHandle *pOutputRead,
                                                oslFileHandle *pErrorRead );


oslProcessError SAL_CALL osl_searchPath_impl(
    const sal_Char* pszName,
    const sal_Char* pszPath,
    sal_Char Separator,
    sal_Char *pszBuffer,
    sal_uInt32 Max);


oslProcessError SAL_CALL osl_psz_getExecutableFile(sal_Char* pszBuffer, sal_uInt32 Max);
sal_Bool osl_getFullPath(const sal_Char* pszFilename, sal_Char* pszPath, sal_uInt32 MaxLen);
oslProcessError SAL_CALL osl_psz_getEnvironment(const sal_Char* pszName, sal_Char *pszBuffer, sal_uInt32 Max);
oslProcessError SAL_CALL osl_getCommandArgs(sal_Char* pszBuffer, sal_uInt32 Max);


static oslProcessImpl* ChildList;
static oslMutex        ChildListMutex;

static sal_Char CmdLine[CMD_ARG_MAX + 1] = "";
static int nArgCount = -1;


/******************************************************************************
 *
 *                  Functions for command args
 *
 *****************************************************************************/

#if defined(MACOSX)
/* Can't access environ and __progname directly when linking two-level. */
sal_Char *getCmdLine()
{
    int i;
    int len = 0;
    sal_Char *result;
    int the_argc = *_NSGetArgc();
    char **the_argv = *_NSGetArgv();

    for (i = 0; i < the_argc; i++) {
        len += 1 + strlen(the_argv[i]);
    }

    result = calloc(len + 1, sizeof(sal_Char));
    if (result) {
        sal_Char *dst = result;
        for (i = 0; i < the_argc; i++) {
            strcpy(dst, the_argv[i]);
            dst += 1 + strlen(the_argv[i]);
        }
        *dst = '\0';
        return result;
    }

    return NULL;
}

#endif

#if defined(CMD_ARG_PRG) && defined(CMD_ARG_ENV)
/*
 * mfe: used by FreeBSD, NetBSD, HP-UX, IRIX
 *      (and which other Unix flavours?)
 */
sal_Char *getCmdLine()
{
    /* Memory layout of CMD_ARG_PRG:
       progname\0arg1\0...argn[\0]\0environ\0env2\0...envn\0[\0] */

#if !defined(NETBSD)
    extern sal_Char*  CMD_ARG_PRG;
#endif
    extern sal_Char** CMD_ARG_ENV;

    int   n = *(CMD_ARG_ENV) - (CMD_ARG_PRG);
    sal_Char *pchr;

    if (pchr = calloc(n + 1, sizeof(sal_Char)))
    {
        memcpy(pchr,(CMD_ARG_PRG), n);
        pchr[n] = '\0';
        return pchr;
    }

    return NULL;
}
#endif

#if !defined (CMD_ARG_PRG) && defined (CMD_ARG_ENV)
/*
 * mfe: not used ???
 */
extern sal_Char** CMD_ARG_ENV;
static sal_Char* pEnviron;

/* The environment pointer has to be saved away early because
 * any putenv() may change it. The next line works for GCC on any
 * ELF platform.
 */

static void saveEnviron(void) __attribute__((constructor));

static void saveEnviron(void)
{
    pEnviron = CMD_ARG_ENV;
}

sal_Char *getCmdLine()
{
    sal_Char **pArgs = pEnviron;
    sal_Char *pChr;
    size_t n;

    pArgs--;
    while(*(--pArgs));
    pArgs += 2; /* skip NULL and pointer to number of args */

    n = *(CMD_ARG_ENV) - *(pArgs);

    if (pChr = calloc(n + 1, sizeof(sal_Char))) {
        memcpy(pChr,*pArgs, n);
        pChr[n] = '\0';
        return pChr;
    }
    return (NULL);
}
#endif

#ifdef CMD_ARG_PROC_STREAM
/*
 *  mfe: this is for Linux
 *       (and which other Unix flavours?)
 */
sal_Char *getCmdLine()
{
    FILE *fp;
    sal_Char  name[PATH_MAX + 1];

    name[0] = '\0';

    snprintf(name, sizeof(name), CMD_ARG_PROC_NAME, getpid());

    if ( (fp = fopen(name, "r")) != 0 )
    {
        int   c;
        int   i = 0, n = 32;
        sal_Char *pchr = (sal_Char*) calloc(n, sizeof(sal_Char));

        while ((c = getc(fp)) != EOF)
        {
            if (i >= (n - 2))
                pchr = (sal_Char*) realloc(pchr, (n += 32) * sizeof(sal_Char));

            pchr[i++] = c;
        }

        pchr[i] = pchr[i + 1] = '\0';

        fclose(fp);

        return (pchr);
    }

    return (NULL);
}
#endif

#ifdef CMD_ARG_PROC_IOCTL
/*
 * mfe: this is for Solaris
 *       (and which other Unix flavours?)
 */

sal_Char *getCmdLine()
{
    int   fd;
    sal_Char  name[PATH_MAX + 1];
    sal_Char *pchr=0;

    name[0] = '\0';

    snprintf(name, sizeof(name), CMD_ARG_PROC_NAME, getpid());

    if ((fd = open(name, O_RDONLY)) >= 0)
    {
        CMD_ARG_PROC_TYPE psinfo;

        if (ioctl(fd, CMD_ARG_PROC_IOCTL, &psinfo) >= 0)
        {
            int   j, i = 0, n = 32;
            pchr = (sal_Char*) calloc(n, sizeof(sal_Char));

              if (CMD_ARG_PROC_ARGC(psinfo) > 0)
                  for (j = 0; j < CMD_ARG_PROC_ARGC(psinfo) , CMD_ARG_PROC_ARGV(psinfo)[j] != 0 ; j++)
                  {
                      int l = strlen(CMD_ARG_PROC_ARGV(psinfo)[j]);

                      if (i + l >= (n - 2))
                         pchr = (sal_Char*) realloc(pchr, (n += l + 32) * sizeof(sal_Char));

                      strcpy(&pchr[i], CMD_ARG_PROC_ARGV(psinfo)[j]);

                    i += l;
                      pchr[i++] = '\0';
                  }
            else
                pchr[i++] = '\0';

            pchr[i] = '\0';
        }

        close(fd);
    }

    return (pchr);
}
#endif

#ifdef CMD_ARG_PS
/*
 *  mfe : this is for AIX
 *       (and which other Unix flavours?)
 */

sal_Char *getCmdLine()
{
    FILE    *fp;
    sal_Char    cmd[CMD_ARG_MAX + 1];
    int             i;

    cmd[0] = '\0';

    snprintf(cmd, sizeof(cmd), CMD_ARG_PS, getpid());

    if (fp = popen(cmd, "r"))
    {
        int     c;
        int     i = 0, n = 32;
        sal_Bool sep = sal_True;
        sal_Char *pchr = calloc(n, sizeof(sal_Char));

        while ((c = getc(fp)) != EOF)
        {
            if (i >= (n - 2))
                pchr = realloc(pchr, (n += 32) * sizeof(sal_Char));

            if (! isspace(c))
            {
                pchr[i++] = c;
                sep = sal_False;
            }
            else
                if (! sep)
                {
                    pchr[i++] = '\0';
                    sep = sal_True;
                }
        }

        pchr[i] = pchr[i + 1] = '\0';

        fclose(fp);

        return (pchr);
    }

    return (NULL);
}
#endif

/*******************************************************************
   !!! Working on byte strings is dangerous because of MBCS see
   #104563.
   Don't fix this function because it is only used in profile.c and
   profiles are deprecated
 ******************************************************************/

oslProcessError SAL_CALL osl_psz_getExecutableFile(sal_Char* pszBuffer, sal_uInt32 Max)
{
    const char * pszCmdLine = getCmdLine();
    const char * pszRealPathSrc = pszCmdLine;
    char szAbsolutePath[PATH_MAX] = "";
    char szRealPathBuf[PATH_MAX] = "";

    /* if the command line argument #0 starts with a '/', this program has been */
    /* invoked using a full qualified path */
    if( '/' != pszCmdLine[0] )
    {
        oslProcessError ret = osl_Process_E_None;

        /* if the command line argument #0 contains a '/' somewhere else, it has */
        /* been probably invoked relatively to the current working directory */
        if( strchr(pszCmdLine, '/') )
        {
            if( NULL != getcwd(szAbsolutePath, sizeof(szAbsolutePath)) )
            {
                size_t n = PATH_MAX - strlen(szAbsolutePath);
                size_t n2 = strlen(pszCmdLine);

                /* check remaining size and append '/' and argument #0 is possible */
                if( n + n2 + 1 < PATH_MAX )
                {
                    szAbsolutePath[n] = '/';
                    strncpy(szAbsolutePath+n+1, pszCmdLine, n2+1);

                    /* replace the original pszRealPathSrc pointer */
                    pszRealPathSrc = szAbsolutePath;
                }
            }
        }
        /* this program must be in the PATH variable */
        else
        {
            ret = osl_searchPath_impl(pszCmdLine, NULL, '\0', szAbsolutePath, sizeof(szAbsolutePath));
            if( osl_Process_E_None == ret )
            {
                /* replace the original pszRealPathSrc pointer */
                pszRealPathSrc = szAbsolutePath;
            }
        }

        /* if szAbsolutePath has not been filled, return with an error */
        if( '\0' == szAbsolutePath[0] )
        {
            ret = osl_Process_E_Unknown;
        }

        if( osl_Process_E_None != ret )
        {
            free((void *) pszCmdLine);
            return ret;
        }
    }

    /* get the realpath of the resulting file and convert it to a file URL */
    if( NULL != realpath(pszRealPathSrc, szRealPathBuf) && strlen(szRealPathBuf) < Max )
    {
        strcpy(pszBuffer, szRealPathBuf);
    }
    else
    {
        free((void *)pszCmdLine);
        return osl_Process_E_Unknown;
    }

    free((void *) pszCmdLine);
    return osl_Process_E_None;
}

/******************************************************************************
 Deprecated
 Old and buggy implementation of osl_searchPath used only by the deprected
 osl_psz_getExecutableName.
 A new implemenation is in process_impl.cxx
 *****************************************************************************/

oslProcessError SAL_CALL osl_searchPath_impl(const sal_Char* pszName, const sal_Char* pszPath,
                   sal_Char Separator, sal_Char *pszBuffer, sal_uInt32 Max)
{
    sal_Char path[PATH_MAX + 1];
    sal_Char *pchr;

    path[0] = '\0';

    OSL_ASSERT(pszName != NULL);

    if ( pszName == 0 )
    {
        return osl_Process_E_NotFound;
    }

    if (pszPath == NULL)
        pszPath = "PATH";

    if (Separator == '\0')
        Separator = ':';


    if ( (pchr = getenv(pszPath)) != 0 )
    {
        sal_Char *pstr;

        while (*pchr != '\0')
        {
            pstr = path;

            while ((*pchr != '\0') && (*pchr != Separator))
                *pstr++ = *pchr++;

            if ((pstr > path) && ((*(pstr - 1) != '/')))
                *pstr++ = '/';

            *pstr = '\0';

            strcat(path, pszName);

            if (access(path, 0) == 0)
            {
                char szRealPathBuf[PATH_MAX] = "";

                if( NULL == realpath(path, szRealPathBuf) || (strlen(szRealPathBuf) >= (sal_uInt32)Max))
                    return osl_Process_E_Unknown;

                strcpy(pszBuffer, path);

                return osl_Process_E_None;
            }

            if (*pchr == Separator)
                pchr++;
        }
    }

    return osl_Process_E_NotFound;
}

/**********************************************
 osl_getCommandArgCount
 *********************************************/

sal_uInt32  SAL_CALL osl_getCommandArgCount()
{
    sal_Char pszBuffer[CMD_ARG_MAX+1];

    oslProcessError tErr = osl_Process_E_Unknown;

    pszBuffer[0] = '\0';

    if ( nArgCount == -1 )
    {
        tErr = osl_getCommandArgs(pszBuffer, sizeof(pszBuffer));

        if ( tErr != osl_Process_E_None)
        {
            return 0;
        }
    }

    /*    fprintf(stderr,"osl_getCommandArgCount : ArgCount = '%i'\n",nArgCount);*/

    return nArgCount;
}

/**********************************************
 osl_getCommandArg
 *********************************************/

oslProcessError SAL_CALL osl_getCommandArg( sal_uInt32 nArg, rtl_uString **strCommandArg)
{
    oslProcessError tErr = osl_Process_E_Unknown;
    sal_Char* pChr=0;

    if ( nArgCount == -1 )
    {
        sal_Char pBuffer[CMD_ARG_MAX+1];
        pBuffer[0] = '\0';

        tErr = osl_getCommandArgs(pBuffer, CMD_ARG_MAX+1);
        if ( tErr == osl_Process_E_None )
        {
            return tErr;
        }
    }

/*      fprintf(stderr,"osl_getCommandArg : getting Arg No. '%i'\n",nArg); */

/*  nArg++;*/

    if ( nArg >= 0 && nArg < nArgCount )
    {
        int nIndex=0;
        int nLen;

        pChr=CmdLine;

        while ( nArg != nIndex && nIndex < nArgCount )
        {
/*                        fprintf(stderr,"Having arg '%i' '%s'\n",nIndex,pChr); */
            nLen = strlen(pChr);
            pChr+=nLen+1;
            ++nIndex;
        }
/*          fprintf(stderr,"osl_getCommandArg : Arg '%i' = '%s'\n",nIndex,pChr); */

        rtl_string2UString(
            strCommandArg,
            pChr,
            rtl_str_getLength( pChr ),
            osl_getThreadTextEncoding(),
            OUSTRING_TO_OSTRING_CVTFLAGS );

        tErr=osl_Process_E_None;
    }

    return tErr;
}

/**********************************************
 osl_getCommandArgs
 *********************************************/

oslProcessError SAL_CALL osl_getCommandArgs(sal_Char* pszBuffer, sal_uInt32 Max)
{
    static int  CmdLen = -1;

    if (CmdLen < 0)
    {
        sal_Char *pszCmdLine = getCmdLine();
        sal_Char *pStr       = pszCmdLine;
        sal_Char *pBuffer    = CmdLine;

        OSL_ASSERT(pszCmdLine != NULL);

        if ( pszCmdLine == 0 )
        {
            return osl_Process_E_Unknown;
        }

        nArgCount=0;

        /* skip program name */
        pStr += strlen(pStr) + 1;

        while ((*pStr != '\0') &&
               ((pBuffer + strlen(pszCmdLine)) < (CmdLine + sizeof(CmdLine) - 2)))
        {
            strcpy(pBuffer, pStr);
            pBuffer += strlen(pStr) + 1;
            pStr    += strlen(pStr) + 1;
            ++nArgCount;
        }

        *pBuffer++ = '\0';

        CmdLen = pBuffer - CmdLine;
        free(pszCmdLine);
    }

    OSL_ASSERT(pszBuffer);

    if ( pszBuffer == 0 )
    {
        return osl_Process_E_Unknown;
    }

    if (Max < (sal_uInt32)CmdLen)
        return osl_Process_E_Unknown;

/*      fprintf(stderr,"osl_getCommandArgs : ArgCount is '%i'\n",nArgCount); */

    memcpy(pszBuffer, CmdLine, CmdLen);

    return osl_Process_E_None;
}

/******************************************************************************
 *
 *                  New io resource transfer functions
 *
 *****************************************************************************/


/**********************************************
 sendFdPipe
 *********************************************/

static sal_Bool sendFdPipe(int PipeFD, int SocketFD)
{
    sal_Bool bRet = sal_False;

    struct iovec    iov[1];
    struct msghdr   msg;
    char            buf[2]; /* send_fd()/recv_fd() 2-byte protocol */
    int nSend;
    int RetCode=0;

#if defined(IOCHANNEL_TRANSFER_BSD)

    OSL_TRACE("IOCHANNEL_TRANSFER_BSD send");
/*      OSL_TRACE("sending fd %i\n",SocketFD); */

    iov[0].iov_base = buf;
    iov[0].iov_len  = sizeof(buf);
    msg.msg_iov     = iov;
    msg.msg_iovlen  = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;

    msg.msg_accrights    = (caddr_t) &SocketFD; /* addr of descriptor */
    msg.msg_accrightslen = sizeof(int);     /* pass 1 descriptor */
    buf[1] = 0;                             /* zero status means OK */
    buf[0] = 0;                             /* null byte flag to recv_fd() */

#else

    struct cmsghdr* cmptr = (struct cmsghdr*)malloc(CONTROLLEN);

    OSL_TRACE("!!!!!! IOCHANNEL_TRANSFER_BSD_RENO send");
/*      OSL_TRACE("sending fd %i\n",SocketFD); */

    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_control = (caddr_t) cmptr;
    msg.msg_controllen = CONTROLLEN;

    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    cmptr->cmsg_len = CONTROLLEN;
    *(int*)CMSG_DATA(cmptr) = SocketFD;

#endif

    if ( ( nSend = sendmsg(PipeFD, &msg, 0) ) > 0 )
    {
        bRet = sal_True;
        OSL_TRACE("sendFdPipe : send '%i' bytes\n",nSend);

    }
    else
    {
        OSL_TRACE("sendFdPipe : sending failed (%s)",strerror(errno));
    }

    nSend=read(PipeFD,&RetCode,sizeof(RetCode));

    if ( nSend > 0 && RetCode == 1 )
    {
        OSL_TRACE("sendFdPipe : resource was received\n");
    }
    else
    {
        OSL_TRACE("sendFdPipe : resource wasn't received\n");
    }

#if defined(IOCHANNEL_TRANSFER_BSD_RENO)
    free(cmptr);
#endif

    return bRet;
}

/**********************************************
 receiveFdPipe
 *********************************************/

static oslSocket receiveFdPipe(int PipeFD)
{
    oslSocket pSocket = 0;
    struct msghdr msghdr;
    struct iovec iov[1];
    char buffer[2];
    sal_Int32 nRead;
    int newfd=-1;
    int nRetCode=0;
/*      char *ptr; */

#if defined(IOCHANNEL_TRANSFER_BSD)

    OSL_TRACE("IOCHANNEL_TRANSFER_BSD receive\n");

    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);
    msghdr.msg_name = NULL;
    msghdr.msg_namelen = 0;
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 1;
    msghdr.msg_accrights = (caddr_t) &newfd; /* addr of descriptor   */
    msghdr.msg_accrightslen = sizeof(int);   /* receive 1 descriptor */

#else
    struct cmsghdr* cmptr = (struct cmsghdr*)malloc(CONTROLLEN);

    OSL_TRACE(" !!!! IOCHANNEL_TRANSFER_BSD_RENO receive");

    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);
    msghdr.msg_name = NULL;
    msghdr.msg_namelen = 0;
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 1;

    msghdr.msg_control = (caddr_t) cmptr;
    msghdr.msg_controllen = CONTROLLEN;

#endif


#if defined(IOCHANNEL_TRANSFER_BSD)

    if ( ( nRead = recvmsg(PipeFD, &msghdr, 0) ) > 0 )
    {
        OSL_TRACE("receiveFdPipe : received '%i' bytes\n",nRead);
    }
#else

    if ( ( ( nRead = recvmsg(PipeFD, &msghdr, 0) ) > 0 ) &&
         ( msghdr.msg_controllen == CONTROLLEN ) )
    {
        OSL_TRACE("receiveFdPipe : received '%i' bytes\n",nRead);
        newfd = *(int*)CMSG_DATA(cmptr);
    }
#endif
    else
    {
        OSL_TRACE("receiveFdPipe : receiving failed (%s)",strerror(errno));
    }

    if ( newfd >= 0 )
    {
        pSocket = __osl_createSocketImpl(newfd);
        nRetCode=1;
        OSL_TRACE("received fd %i\n",newfd);
    }

    OSL_TRACE("receiveFdPipe : writing back %i",nRetCode);
    nRead=write(PipeFD,&nRetCode,sizeof(nRetCode));

#if defined(IOCHANNEL_TRANSFER_BSD_RENO)
    free(cmptr);
#endif

    return pSocket;
}

/**********************************************
 osl_sendResourcePipe
 *********************************************/

sal_Bool osl_sendResourcePipe(oslPipe pPipe, oslSocket pSocket)
{
    sal_Bool bRet = sal_False;

    if ( pSocket == 0 || pPipe == 0 )
    {
        return sal_False;
    }

    bRet = sendFdPipe(pPipe->m_Socket,pSocket->m_Socket);

    return bRet;
}

/**********************************************
 osl_receiveResourcePipe
 *********************************************/

oslSocket osl_receiveResourcePipe(oslPipe pPipe)
{
    oslSocket pSocket=0;

    if ( pPipe ==  0 )
    {
        return 0;
    }

    pSocket = receiveFdPipe(pPipe->m_Socket);

    return (oslSocket) pSocket;
}



/******************************************************************************
 *
 *                  Functions for starting a process
 *
 *****************************************************************************/

static void ChildStatusProc(void *pData)
{
    int   i;
/*  int   first = 0;*/
    pid_t pid;
/*  int   status;*/
    int   channel[2];
    ProcessData  data;
    ProcessData *pdata;
    int     stdOutput[2] = { -1, -1 }, stdInput[2] = { -1, -1 }, stdError[2] = { -1, -1 };

    pdata = (ProcessData *)pData;

    /* make a copy of our data, because forking will only copy
       our local stack of the thread, so the process data will not be accessible
       in our child process */
    memcpy(&data, pData, sizeof(data));

    socketpair(AF_UNIX, SOCK_STREAM, 0, channel);

    fcntl(channel[0], F_SETFD, FD_CLOEXEC);
    fcntl(channel[1], F_SETFD, FD_CLOEXEC);

    /* Create redirected IO pipes */

    if ( data.m_pInputWrite )
        pipe( stdInput );

    if ( data.m_pOutputRead )
        pipe( stdOutput );

    if ( data.m_pErrorRead )
        pipe( stdError );

    if ((pid = fork()) == 0)
    {
        /* Child */
        close(channel[0]);

        if ((data.m_uid != (uid_t)-1) && ((data.m_uid != getuid()) || (data.m_gid != getgid())))
        {
            OSL_ASSERT(geteuid() == 0);     /* must be root */

            if (! INIT_GROUPS(data.m_name, data.m_gid) || (setuid(data.m_uid) != 0))
                OSL_TRACE("Failed to change uid and guid, errno=%d (%s)\n", errno, strerror(errno));
#if defined(LINUX) || defined (FREEBSD)
            unsetenv("HOME");
#else
            putenv("HOME=");
#endif
        }

           if ((data.m_uid == (uid_t)-1) || ((data.m_uid == getuid()) && (data.m_gid == getgid())))

        {
              if (data.m_pszDir)
                  chdir(data.m_pszDir);

            for (i = 0; data.m_pszEnv[i] != NULL; i++)
                 putenv(data.m_pszEnv[i]);

#if defined(LINUX)
            /* mfe: linux likes to have just one thread when the exec family is called */
            /*      this np function has this purpose ...                              */
            pthread_kill_other_threads_np();
#endif
            OSL_TRACE("ChildStatusProc : starting '%s'",data.m_pszArgs[0]);

            /* Connect std IO to pipe ends */

            /* Write end of stdInput not used in child process */
            close( stdInput[1] );

            /* Read end of stdOutput not used in child process */
            close( stdOutput[0] );

            /* Read end of stdError not used in child process */
            close( stdError[0] );

            /* Redirect pipe ends to std IO */

            if ( stdInput[0] != STDIN_FILENO )
            {
                dup2( stdInput[0], STDIN_FILENO );
                close( stdInput[0] );
            }

            if ( stdOutput[1] != STDOUT_FILENO )
            {
                dup2( stdOutput[1], STDOUT_FILENO );
                close( stdOutput[1] );
            }

            if ( stdError[1] != STDERR_FILENO )
            {
                dup2( stdError[1], STDERR_FILENO );
                close( stdError[1] );
            }

            pid=execv(data.m_pszArgs[0], (sal_Char **)data.m_pszArgs);

        }

        OSL_TRACE("Failed to exec, errno=%d (%s)\n", errno, strerror(errno));

        OSL_TRACE("ChildStatusProc : starting '%s' failed",data.m_pszArgs[0]);

        /* if we reach here, something went wrong */
        write(channel[1], &errno, sizeof(errno));

        close(channel[1]);

        _exit(255);
    }
    else
    {   /* Parent  */
        int   status;
        Pipe* rpipe;

        close(channel[1]);

        /* Close unused pipe ends */
        close( stdInput[0] );
        close( stdOutput[1] );
        close( stdError[1] );

        while (((i = read(channel[0], &status, sizeof(status))) < 0))
        {
            if (errno != EINTR)
                break;
        }

        close(channel[0]);


        if (i == 0)
        {
            osl_acquireMutex(ChildListMutex);

            pdata->m_pProcImpl->m_pid = pid;
            pdata->m_pProcImpl->m_pnext = ChildList;
            ChildList = pdata->m_pProcImpl;

            /* Store used pipe ends in data structure */

            if ( pdata->m_pInputWrite )
                *(pdata->m_pInputWrite) = osl_createFileHandleFromFD( stdInput[1] );

            if ( pdata->m_pOutputRead )
                *(pdata->m_pOutputRead) = osl_createFileHandleFromFD( stdOutput[0] );

            if ( pdata->m_pErrorRead )
                *(pdata->m_pErrorRead) = osl_createFileHandleFromFD( stdError[0] );

            osl_releaseMutex(ChildListMutex);

            osl_setCondition(pdata->m_started);

            if ((pid = waitpid(pid, &status, 0)) < 0)
                OSL_TRACE("Failed to wait for child process, errno=%d (%s)\n", errno, strerror(errno));
            else
            {
                oslProcessImpl* pChild;

                osl_acquireMutex(ChildListMutex);

                pChild = ChildList;

                /* check if it is one of our child processes */
                while (pChild != NULL)
                {
                    if (pChild->m_pid == pid)
                    {
                        if (WIFEXITED(status))
                            pChild->m_status = WEXITSTATUS(status);
                        else
                            pChild->m_status = -1;

                        osl_setCondition(pChild->m_terminated);
                    }

                    pChild = pChild->m_pnext;
                }

                osl_releaseMutex(ChildListMutex);
            }
        }
        else
        {
            OSL_TRACE("ChildStatusProc : starting '%s' failed",data.m_pszArgs[0]);
            OSL_TRACE("Failed to launch child process, child reports errno=%d (%s)\n", status, strerror(status));

            /* Close pipe ends */
            if ( pdata->m_pInputWrite )
                *pdata->m_pInputWrite = NULL;

            if ( pdata->m_pOutputRead )
                *pdata->m_pOutputRead = NULL;

            if ( pdata->m_pErrorRead )
                *pdata->m_pErrorRead = NULL;

            close( stdInput[1] );
            close( stdOutput[0] );
            close( stdError[0] );

            /* notify (and unblock) parent thread */
            osl_setCondition(pdata->m_started);
        }
    }
}

/**********************************************
 osl_executeProcess_WithRedirectedIO
 *********************************************/

oslProcessError SAL_CALL osl_executeProcess_WithRedirectedIO(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess,
                                            oslFileHandle   *pInputWrite,
                                            oslFileHandle   *pOutputRead,
                                            oslFileHandle   *pErrorRead
                                            )
{

    oslProcessError Error;
    sal_Char* pszWorkDir=0;
    sal_Char** pArguments=0;
    sal_Char** pEnvironment=0;
    unsigned int index;

    char szImagePath[PATH_MAX] = "";
    char szWorkDir[PATH_MAX] = "";

    if ( ustrImageName && ustrImageName->length )
    {
        FileURLToPath( szImagePath, PATH_MAX, ustrImageName );
    }

    if ( ustrWorkDir != 0 && ustrWorkDir->length )
    {
        FileURLToPath( szWorkDir, PATH_MAX, ustrWorkDir );
        pszWorkDir = szWorkDir;
    }

    if ( pArguments == 0 && nArguments > 0 )
    {
        pArguments = (sal_Char**) malloc( ( nArguments + 2 ) * sizeof(sal_Char*) );
    }


    for ( index = 0 ; index < nArguments ; ++index )
    {
        rtl_String* strArg =0;


        rtl_uString2String( &strArg,
                            rtl_uString_getStr(ustrArguments[index]),
                            rtl_uString_getLength(ustrArguments[index]),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pArguments[index]=strdup(rtl_string_getStr(strArg));
        rtl_string_release(strArg);
        pArguments[index+1]=0;
    }

    for ( index = 0 ; index < nEnvironmentVars ; ++index )
    {
        rtl_String* strEnv=0;

        if ( pEnvironment == 0 )
        {
            pEnvironment = (sal_Char**) malloc( ( nEnvironmentVars + 2 ) * sizeof(sal_Char*) );
        }

        rtl_uString2String( &strEnv,
                            rtl_uString_getStr(ustrEnvironment[index]),
                            rtl_uString_getLength(ustrEnvironment[index]),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pEnvironment[index]=strdup(rtl_string_getStr(strEnv));
        rtl_string_release(strEnv);
        pEnvironment[index+1]=0;
    }


    Error = osl_psz_executeProcess(szImagePath,
                                   pArguments,
                                   Options,
                                   Security,
                                   pszWorkDir,
                                   pEnvironment,
                                   pProcess,
                                   pInputWrite,
                                   pOutputRead,
                                   pErrorRead
                                   );

    if ( pArguments != 0 )
    {
        for ( index = 0 ; index < nArguments ; ++index )
        {
            if ( pArguments[index] != 0 )
            {
                free(pArguments[index]);
            }
        }
        free(pArguments);
    }

    if ( pEnvironment != 0 )
    {
        for ( index = 0 ; index < nEnvironmentVars ; ++index )
        {
            if ( pEnvironment[index] != 0 )
            {
                free(pEnvironment[index]);
            }
        }
        free(pEnvironment);
    }

    return Error;
}

/**********************************************
 osl_executeProcess
 *********************************************/

oslProcessError SAL_CALL osl_executeProcess(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess
                                            )
{
    return osl_executeProcess_WithRedirectedIO(
        ustrImageName,
        ustrArguments,
        nArguments,
        Options,
        Security,
        ustrWorkDir,
        ustrEnvironment,
        nEnvironmentVars,
        pProcess,
        NULL,
        NULL,
        NULL
        );
}

/**********************************************
 osl_psz_executeProcess
 *********************************************/

oslProcessError SAL_CALL osl_psz_executeProcess(sal_Char *pszImageName,
                                                sal_Char *pszArguments[],
                                                oslProcessOption Options,
                                                oslSecurity Security,
                                                sal_Char *pszDirectory,
                                                sal_Char *pszEnvironments[],
                                                oslProcess *pProcess,
                                                oslFileHandle   *pInputWrite,
                                                oslFileHandle   *pOutputRead,
                                                oslFileHandle   *pErrorRead
                                                )
{
    int     i;
    sal_Char    path[PATH_MAX + 1];
    ProcessData Data;
    oslThread hThread;

    path[0] = '\0';

    memset(&Data,0,sizeof(ProcessData));
    Data.m_pInputWrite = pInputWrite;
    Data.m_pOutputRead = pOutputRead;
    Data.m_pErrorRead = pErrorRead;

    if (pszImageName == NULL)
        pszImageName = pszArguments[0];

    OSL_ASSERT(pszImageName != NULL);

    if ( pszImageName == 0 )
    {
        return osl_Process_E_NotFound;
    }

    if ((Options & osl_Process_SEARCHPATH) &&
        (osl_searchPath_impl(pszImageName, NULL, '\0', path, sizeof(path)) == osl_Process_E_None))
        pszImageName = path;

    Data.m_pszArgs[0] = strdup(pszImageName);
    Data.m_pszArgs[1] = 0;

    if ( pszArguments != 0 )
    {
        for (i = 0; ((i + 2) < MAX_ARGS) && (pszArguments[i] != NULL); i++)
            Data.m_pszArgs[i+1] = strdup(pszArguments[i]);
        Data.m_pszArgs[i+2] = NULL;
    }

    Data.m_options = Options;
    Data.m_pszDir  = (pszDirectory != NULL) ? strdup(pszDirectory) : NULL;

    if (pszEnvironments != NULL)
    {
        for (i = 0; ((i + 1) < MAX_ENVS) &&  (pszEnvironments[i] != NULL); i++)
            Data.m_pszEnv[i] = strdup(pszEnvironments[i]);
         Data.m_pszEnv[i+1] = NULL;
    }
    else
         Data.m_pszEnv[0] = NULL;

    if (Security != NULL)
    {
        Data.m_uid  = ((oslSecurityImpl*)Security)->m_pPasswd.pw_uid;
        Data.m_gid  = ((oslSecurityImpl*)Security)->m_pPasswd.pw_gid;
        Data.m_name = ((oslSecurityImpl*)Security)->m_pPasswd.pw_name;
    }
    else
        Data.m_uid = (uid_t)-1;

    Data.m_pProcImpl = (oslProcessImpl*) malloc(sizeof(oslProcessImpl));
    Data.m_pProcImpl->m_pid = 0;
    Data.m_pProcImpl->m_terminated = osl_createCondition();
    Data.m_pProcImpl->m_pnext = NULL;

    if (ChildListMutex == NULL)
        ChildListMutex = osl_createMutex();

    Data.m_started = osl_createCondition();

    hThread = osl_createThread(ChildStatusProc, &Data);

    osl_waitCondition(Data.m_started, NULL);
    osl_destroyCondition(Data.m_started);

    for (i = 0; Data.m_pszArgs[i] != NULL; i++)
          free((void *)Data.m_pszArgs[i]);

    for (i = 0; Data.m_pszEnv[i] != NULL; i++)
          free((void *)Data.m_pszEnv[i]);

    if ( Data.m_pszDir != 0 )
    {
        free((void *)Data.m_pszDir);
    }

    osl_destroyThread(hThread);

    if (Data.m_pProcImpl->m_pid != 0)
    {
        *pProcess = Data.m_pProcImpl;

         if (Options & osl_Process_WAIT)
            osl_joinProcess(*pProcess);

        return osl_Process_E_None;
    }

    osl_destroyCondition(Data.m_pProcImpl->m_terminated);
    free(Data.m_pProcImpl);

    return osl_Process_E_Unknown;
}


/******************************************************************************
 *
 *                  Functions for processes
 *
 *****************************************************************************/


/**********************************************
 osl_terminateProcess
 *********************************************/

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == NULL)
        return osl_Process_E_Unknown;

    if (kill(((oslProcessImpl*)Process)->m_pid, SIGKILL) != 0)
    {
        switch (errno)
        {
            case EPERM:
                return osl_Process_E_NoPermission;

            case ESRCH:
                return osl_Process_E_NotFound;

            default:
                return osl_Process_E_Unknown;
        }
    }

    return osl_Process_E_None;
}

/**********************************************
 osl_getProcess
 *********************************************/

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    oslProcessImpl *pProcImpl;

    if (kill(Ident, 0) != -1)
    {
        oslProcessImpl* pChild;

        if (ChildListMutex == NULL)
            ChildListMutex = osl_createMutex();

        osl_acquireMutex(ChildListMutex);

        pChild = ChildList;

        /* check if it is one of our child processes */
        while (pChild != NULL)
        {
            if (Ident == (sal_uInt32) pChild->m_pid)
                break;

            pChild = pChild->m_pnext;
        }

        pProcImpl = (oslProcessImpl*) malloc(sizeof(oslProcessImpl));
        pProcImpl->m_pid        = Ident;
        pProcImpl->m_terminated = osl_createCondition();

        if (pChild != NULL)
        {
            /* process is a child so insert into list */
            pProcImpl->m_pnext = pChild->m_pnext;
            pChild->m_pnext = pProcImpl;

            pProcImpl->m_status = pChild->m_status;

            if (osl_checkCondition(pChild->m_terminated))
                osl_setCondition(pProcImpl->m_terminated);
        }
        else
            pProcImpl->m_pnext = NULL;

        osl_releaseMutex(ChildListMutex);
    }
    else
        pProcImpl = NULL;

    return (pProcImpl);
}

/**********************************************
 osl_freeProcessHandle
 *********************************************/

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    if (Process != NULL)
    {
        oslProcessImpl *pChild, *pPrev = NULL;

        OSL_ASSERT(ChildListMutex != NULL);

        if ( ChildListMutex == 0 )
        {
            return;
        }

        osl_acquireMutex(ChildListMutex);

        pChild = ChildList;

        /* remove process from child list */
        while (pChild != NULL)
        {
            if (pChild == (oslProcessImpl*)Process)
            {
                if (pPrev != NULL)
                    pPrev->m_pnext = pChild->m_pnext;
                else
                    ChildList = pChild->m_pnext;

                break;
            }

            pPrev  = pChild;
            pChild = pChild->m_pnext;
        }

        osl_releaseMutex(ChildListMutex);

        osl_destroyCondition(((oslProcessImpl*)Process)->m_terminated);

        free(Process);
    }
}

#if defined(LINUX)
struct osl_procStat
{
   /* from 'stat' */
    pid_t pid;                /* pid */
    char command[16];         /* 'argv[0]' */ /* mfe: it all right char comm[16] in kernel! */
    char state;               /* state (running, stopped, ...) */
    pid_t ppid;               /* parent pid */
    pid_t pgrp;               /* parent group */
    int session;              /* session ID */
    int tty;                  /* no of tty */
    pid_t tpgid;              /* group of process owning the tty */
    unsigned long flags;      /* flags dunno */
    unsigned long minflt;     /* minor page faults */
    unsigned long cminflt;    /* minor page faults with children */
    unsigned long majflt;     /* major page faults */
    unsigned long cmajflt;    /* major page faults with children */
    unsigned long utime;      /* no of jiffies in user mode */
    unsigned long stime;      /* no of jiffies in kernel mode */
    unsigned long cutime;     /* no of jiffies in user mode with children */
    unsigned long cstime;     /* no of jiffies in kernel mode with children */
    unsigned long priority;   /* nice value + 15 (kernel scheduling prio)*/
    long nice;                /* nice value */
    long timeout;             /* no of jiffies of next process timeout */
    long itrealvalue;         /* no jiffies before next SIGALRM */
    unsigned long starttime;  /* process started this no of jiffies after boot */
    unsigned long vsize;      /* virtual memory size (in bytes) */
    long rss;                 /* resident set size (in pages) */
    unsigned long rss_rlim;   /* rss limit (in bytes) */
    unsigned long startcode;   /* address above program text can run */
    unsigned long endcode;    /* address below program text can run */
    unsigned long startstack; /* address of start of stack */
    unsigned long kstkesp;    /* current value of 'esp' (stack pointer) */
    unsigned long kstkeip;    /* current value of 'eip' (instruction pointer) */
    /* mfe: Linux > 2.1.7x have more signals (88) */
/*#ifdef LINUX */
    char signal[24];          /* pending signals */
    char blocked[24];         /* blocked signals */
    char sigignore[24];       /* ignored signals */
    char sigcatch[24];        /* catched signals */
/*#else*/
/*  long long signal;*/
/*  long long blocked;*/
/*  long long sigignore;*/
/*  long long sigcatch;*/
/*#endif */
    unsigned long wchan;      /* 'channel' the process is waiting in */
    unsigned long nswap;      /* ? */
    unsigned long cnswap;     /* ? */

    /* from 'statm' */
    long size;                /* numbers of pages in memory */
    long resident;            /* number of resident pages */
    long share;               /* number of shared pages */
    long trs;                 /* text resident size */
    long lrs;                 /* library resident size */
    long drs;                 /* data resident size */
    long dt;                  /* ditry pages */

    /* from 'status' */
    int ruid;                 /* real uid */
    int euid;                 /* effective uid */
    int suid;                 /* saved uid */
    int fuid;                 /* file access uid */
    int rgid;                 /* real gid */
    int egid;                 /* effective gid */
    int sgid;                 /* saved gid */
    int fgid;                 /* file access gid */
    unsigned long vm_size;    /* like vsize but on kb */
    unsigned long vm_lock;    /* locked pages in kb */
    unsigned long vm_rss;     /* like rss but in kb */
    unsigned long vm_data;    /* data size */
    unsigned long vm_stack;   /* stack size */
    unsigned long vm_exe;     /* executable size */
    unsigned long vm_lib;     /* library size */
};

/**********************************************
 osl_getProcStat
 *********************************************/

void osl_getProcStat(pid_t pid, struct osl_procStat* procstat)
{
    int fd = 0;
    char name[PATH_MAX + 1];
    snprintf(name, sizeof(name), "/proc/%u/stat", pid);

    if ((fd = open(name,O_RDONLY)) >=0 )
    {
        char* tmp=0;
        char prstatbuf[512];
        memset(prstatbuf,0,512);
        read(fd,prstatbuf,512);

        close(fd);
        /*printf("%s\n\n",prstatbuf);*/


        tmp = strrchr(prstatbuf, ')');
        *tmp = '\0';
        memset(procstat->command, 0, sizeof(procstat->command));

        sscanf(prstatbuf, "%d (%15c", &procstat->pid, procstat->command);
        sscanf(tmp + 2,
               "%c"
               "%i %i %i %i %i"
               "%lu %lu %lu %lu %lu"
               "%lu %lu %lu %lu"
               "%lu %li %li %li"
               "%lu %lu %li %lu"
               "%lu %lu %lu %lu %lu"
               "%s %s %s %s"
               "%lu %lu %lu",
               &procstat->state,
               &procstat->ppid,      &procstat->pgrp,    &procstat->session,    &procstat->tty,         &procstat->tpgid,
               &procstat->flags,     &procstat->minflt,  &procstat->cminflt,    &procstat->majflt,      &procstat->cmajflt,
               &procstat->utime,     &procstat->stime,   &procstat->cutime,     &procstat->cstime,
               &procstat->priority,  &procstat->nice,    &procstat->timeout,    &procstat->itrealvalue,
               &procstat->starttime, &procstat->vsize,   &procstat->rss,        &procstat->rss_rlim,
               &procstat->startcode, &procstat->endcode, &procstat->startstack, &procstat->kstkesp,     &procstat->kstkeip,
               procstat->signal,     procstat->blocked,  procstat->sigignore,   procstat->sigcatch,
               &procstat->wchan,     &procstat->nswap,   &procstat->cnswap
            );
    }
}

/**********************************************
 osl_getProcStatm
 *********************************************/

void osl_getProcStatm(pid_t pid, struct osl_procStat* procstat)
{
    int fd = 0;
    char name[PATH_MAX + 1];
    snprintf(name, sizeof(name), "/proc/%u/statm", pid);

    if ((fd = open(name,O_RDONLY)) >=0 )
    {
        char prstatmbuf[512];
        memset(prstatmbuf,0,512);
        read(fd,prstatmbuf,512);

        close(fd);

        /*      printf("\n\n%s\n\n",prstatmbuf);*/

        sscanf(prstatmbuf,"%li %li %li %li %li %li %li",
               &procstat->size, &procstat->resident, &procstat->share,
               &procstat->trs,  &procstat->lrs,      &procstat->drs,
               &procstat->dt
            );
    }
}

/**********************************************
 osl_getProcStatus
 *********************************************/

void osl_getProcStatus(pid_t pid, struct osl_procStat* procstat)
{
    int fd = 0;
    char name[PATH_MAX + 1];
    snprintf(name, sizeof(name), "/proc/%u/status", pid);

    if ((fd = open(name,O_RDONLY)) >=0 )
    {
        char* tmp=0;
        char prstatusbuf[512];
        memset(prstatusbuf,0,512);
        read(fd,prstatusbuf,512);

        close(fd);

        /*      printf("\n\n%s\n\n",prstatusbuf);*/

        tmp = strstr(prstatusbuf,"Uid:");
        if(tmp)
        {
            sscanf(tmp,"Uid:\t%d\t%d\t%d\t%d",
                   &procstat->ruid, &procstat->euid, &procstat->suid, &procstat->fuid
                );
        }


        tmp = strstr(prstatusbuf,"Gid:");
        if(tmp)
        {
            sscanf(tmp,"Gid:\t%d\t%d\t%d\t%d",
                   &procstat->rgid, &procstat->egid, &procstat->sgid, &procstat->fgid
                );
        }

        tmp = strstr(prstatusbuf,"VmSize:");
        if(tmp)
        {
            sscanf(tmp,
                   "VmSize: %lu kB\n"
                   "VmLck: %lu kB\n"
                   "VmRSS: %lu kB\n"
                   "VmData: %lu kB\n"
                   "VmStk: %lu kB\n"
                   "VmExe: %lu kB\n"
                   "VmLib: %lu kB\n",
                   &procstat->vm_size, &procstat->vm_lock, &procstat->vm_rss, &procstat->vm_data,
                   &procstat->vm_stack, &procstat->vm_exe, &procstat->vm_lib
                );
        }

        tmp = strstr(prstatusbuf,"SigPnd:");
        if(tmp)
        {
            sscanf(tmp, "SigPnd: %s SigBlk: %s SigIgn: %s %*s %s",
                   procstat->signal, procstat->blocked, procstat->sigignore, procstat->sigcatch
                );
        }
    }
}

#endif

/**********************************************
 osl_getProcessInfo
 *********************************************/

oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields, oslProcessInfo* pInfo)
{
    pid_t   pid;

    if (Process == NULL)
        pid = getpid();
    else
        pid = ((oslProcessImpl*)Process)->m_pid;

    if (! pInfo || (pInfo->Size != sizeof(oslProcessInfo)))
        return osl_Process_E_Unknown;

    pInfo->Fields = 0;

    if (Fields & osl_Process_IDENTIFIER)
    {
        pInfo->Ident  = pid;
        pInfo->Fields |= osl_Process_IDENTIFIER;
    }

    if (Fields & osl_Process_EXITCODE)
    {
        if ((Process != NULL) &&
            osl_checkCondition(((oslProcessImpl*)Process)->m_terminated))
        {
            pInfo->Code = ((oslProcessImpl*)Process)->m_status;
            pInfo->Fields |= osl_Process_EXITCODE;
        }
    }

    if (Fields & (osl_Process_HEAPUSAGE | osl_Process_CPUTIMES))
    {

#if defined(SOLARIS)

        int  fd;
        sal_Char name[PATH_MAX + 1];

        snprintf(name, sizeof(name), "/proc/%u", pid);

        if ((fd = open(name, O_RDONLY)) >= 0)
        {
            prstatus_t prstatus;

            if (ioctl(fd, PIOCSTATUS, &prstatus) >= 0)
            {
                if (Fields & osl_Process_CPUTIMES)
                {
                    pInfo->UserTime.Seconds   = prstatus.pr_utime.tv_sec;
                    pInfo->UserTime.Nanosec   = prstatus.pr_utime.tv_nsec;
                    pInfo->SystemTime.Seconds = prstatus.pr_stime.tv_sec;
                    pInfo->SystemTime.Nanosec = prstatus.pr_stime.tv_nsec;

                    pInfo->Fields |= osl_Process_CPUTIMES;
                }

                if (Fields & osl_Process_HEAPUSAGE)
                {
                    pInfo->HeapUsage = prstatus.pr_brksize;

                    pInfo->Fields |= osl_Process_HEAPUSAGE;
                }

                close(fd);

                return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
            }
            else
                close(fd);
        }

#elif defined(HPUX)

        struct pst_status prstatus;

        if (pstat_getproc(&prstatus, sizeof(prstatus), (size_t)0, pid) == 1)
        {
            if (Fields & osl_Process_CPUTIMES)
            {
                pInfo->UserTime.Seconds   = prstatus.pst_utime;
                pInfo->UserTime.Nanosec   = 500000L;
                pInfo->SystemTime.Seconds = prstatus.pst_stime;
                pInfo->SystemTime.Nanosec = 500000L;

                pInfo->Fields |= osl_Process_CPUTIMES;
            }

            if (Fields & osl_Process_HEAPUSAGE)
            {
                pInfo->HeapUsage = prstatus.pst_vdsize*PAGESIZE;

                pInfo->Fields |= osl_Process_HEAPUSAGE;
            }

            return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
        }

#elif defined(IRIX)

        int  fd;
        sal_Char name[PATH_MAX + 1];

        snprintf(name, sizeof(name), "/proc/%u", pid);

        if ((fd = open(name, O_RDONLY)) >= 0)
        {
            prstatus_t prstatus;
            prpsinfo_t prpsinfo;

            if (ioctl(fd, PIOCSTATUS, &prstatus) >= 0 &&
                ioctl(fd, PIOCPSINFO, &prpsinfo) >= 0)
            {
                if (Fields & osl_Process_CPUTIMES)
                {
                    pInfo->UserTime.Seconds   = prstatus.pr_utime.tv_sec;
                    pInfo->UserTime.Nanosec   = prstatus.pr_utime.tv_nsec;
                    pInfo->SystemTime.Seconds = prstatus.pr_stime.tv_sec;
                    pInfo->SystemTime.Nanosec = prstatus.pr_stime.tv_nsec;

                    pInfo->Fields |= osl_Process_CPUTIMES;
                }

                if (Fields & osl_Process_HEAPUSAGE)
                {
                    int pagesize = getpagesize();

                    pInfo->HeapUsage = prpsinfo.pr_size*pagesize;

                    pInfo->Fields |= osl_Process_HEAPUSAGE;
                }

                close(fd);

                return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
            }
            else
                close(fd);
        }

#elif defined(LINUX)

/*      int fd = 0;*/
        struct osl_procStat procstat;
        memset(&procstat,0,sizeof(procstat));

        osl_getProcStat(pid, &procstat);
        osl_getProcStatm(pid, &procstat);
        osl_getProcStatus(pid, &procstat);

        if ( Fields & osl_Process_CPUTIMES)
        {
            /*
             *  mfe:
             *  We calculate only time of the process proper.
             *  Threads are processes, we do not consider their time here!
             *  (For this, cutime and cstime should be used, it seems not
             *   to work in 2.0.36)
             */

            unsigned long userseconds=(procstat.utime/HZ);
            unsigned long systemseconds=(procstat.stime/HZ);

            pInfo->UserTime.Seconds   = userseconds;
            pInfo->UserTime.Nanosec   = procstat.utime - (userseconds * HZ);
            pInfo->SystemTime.Seconds = systemseconds;
            pInfo->SystemTime.Nanosec = procstat.stime - (systemseconds * HZ);

            pInfo->Fields |= osl_Process_CPUTIMES;
        }

        if (Fields & osl_Process_HEAPUSAGE)
        {
            /*
             *  mfe:
             *  vm_data (found in status) shows the size of the data segment
             *  it a rough approximation of the core heap size
             */
            pInfo->HeapUsage = procstat.vm_data*1024;

            pInfo->Fields |= osl_Process_HEAPUSAGE;
        }

        return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
#endif

    }

    return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
}


/***********************************************
 helper function for osl_joinProcessWithTimeout
 **********************************************/

static int is_timeout(const struct timeval* tend)
{
    struct timeval tcurrent;
    gettimeofday(&tcurrent, NULL);
    return (tcurrent.tv_sec >= tend->tv_sec);
}

/**********************************************
 kill(pid, 0) is usefull for checking if a
 process is still alive, but remember that
 kill even returns 0 if the process is already
 a zombie.
 *********************************************/

static int is_process_dead(pid_t pid)
{
    return ((-1 == kill(pid, 0)) && (ESRCH == errno));
}

/**********************************************
 osl_joinProcessWithTimeout
 *********************************************/

oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout)
{
    oslProcessImpl* pChild    = ChildList;
    oslProcessError osl_error = osl_Process_E_None;

    OSL_PRECOND(Process, "osl_joinProcess: Invalid parameter");
    OSL_ASSERT(ChildListMutex);

    if (NULL == Process || 0 == ChildListMutex)
        return osl_Process_E_Unknown;

    osl_acquireMutex(ChildListMutex);

    /* check if process is a child of ours */
    while (pChild != NULL)
    {
        if (pChild == (oslProcessImpl*)Process)
            break;

        pChild = pChild->m_pnext;
    }

    osl_releaseMutex(ChildListMutex);

    if (pChild != NULL)
    {
        oslConditionResult cond_res = osl_waitCondition(pChild->m_terminated, pTimeout);

        if (osl_cond_result_timeout == cond_res)
            osl_error = osl_Process_E_TimedOut;
        else if (osl_cond_result_ok != cond_res)
            osl_error = osl_Process_E_Unknown;
    }
    else /* alien process; StatusThread will not be able
               to set the condition terminated */
    {
        pid_t pid = ((oslProcessImpl*)Process)->m_pid;

        if (pTimeout)
        {
            int timeout = 0;
            struct timeval tend;

            gettimeofday(&tend, NULL);

            tend.tv_sec += pTimeout->Seconds;

            while (!is_process_dead(pid) && ((timeout = is_timeout(&tend)) == 0))
                sleep(1);

            if (timeout)
                osl_error = osl_Process_E_TimedOut;
        }
        else /* infinite */
        {
            while (!is_process_dead(pid))
                sleep(1);
        }
    }
    return osl_error;
}

/**********************************************
 osl_joinProcess
 *********************************************/

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    return osl_joinProcessWithTimeout(Process, NULL);
}


/******************************************************************************
 *
 *                  Functions for the environment
 *
 *****************************************************************************/

oslProcessError SAL_CALL osl_getEnvironment(rtl_uString* pustrEnvVar, rtl_uString** ppustrValue)
{
    rtl_String*     pstr_env_var = 0;
    char*           p_env_var;
    oslProcessError osl_error = osl_Process_E_NotFound;

    OSL_PRECOND(pustrEnvVar, "osl_getEnvironment: Invalid parameter");
    OSL_PRECOND(ppustrValue, "osl_getEnvironment: Invalid parameter");

    rtl_uString2String(
        &pstr_env_var,
        rtl_uString_getStr(pustrEnvVar),
        rtl_uString_getLength(pustrEnvVar),
        osl_getThreadTextEncoding(),
        OUSTRING_TO_OSTRING_CVTFLAGS);

    if ((p_env_var = getenv(rtl_string_getStr(pstr_env_var))))
    {
        rtl_string2UString(
            ppustrValue,
            p_env_var,
            strlen(p_env_var),
            osl_getThreadTextEncoding(),
            OSTRING_TO_OUSTRING_CVTFLAGS);

        osl_error = osl_Process_E_None;
    }

    rtl_string_release(pstr_env_var);

    return osl_error;
}


/******************************************************************************
 *
 *                  new function to return the process working dir
 *
 *****************************************************************************/

oslProcessError SAL_CALL osl_getProcessWorkingDir(rtl_uString **ppustrWorkingDir)
{
    oslProcessError osl_error = osl_Process_E_None;
    char            buffer[PATH_MAX];

    OSL_PRECOND(ppustrWorkingDir, "osl_getProcessWorkingDir: invalid parameter");

    if (getcwd(buffer, PATH_MAX))
    {
        rtl_uString* ustrTmp = NULL;

        rtl_string2UString(
            &ustrTmp,
            buffer,
            strlen(buffer),
            osl_getThreadTextEncoding(),
            OSTRING_TO_OUSTRING_CVTFLAGS);

        osl_error = osl_getFileURLFromSystemPath(ustrTmp, ppustrWorkingDir);

        rtl_uString_release(ustrTmp);
    }

    return osl_error;
}

/******************************************************************************
 *
 *              new functions to set/return the current process locale
 *
 *****************************************************************************/

static rtl_Locale * theProcessLocale = NULL;
static pthread_mutex_t aLocalMutex = PTHREAD_MUTEX_INITIALIZER;

/**********************************************
 osl_getProcessLocale
 *********************************************/

oslProcessError SAL_CALL osl_getProcessLocale( rtl_Locale ** ppLocale )
{
    OSL_ASSERT( ppLocale );

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    if( NULL == theProcessLocale )
        _imp_getProcessLocale( &theProcessLocale );

    *ppLocale = theProcessLocale;

    pthread_mutex_unlock( &aLocalMutex );

    return osl_Process_E_None;
}

/**********************************************
 osl_setProcessLocale
 *********************************************/

oslProcessError SAL_CALL osl_setProcessLocale( rtl_Locale * pLocale )
{
    oslProcessError ret = osl_Process_E_Unknown;

    OSL_ASSERT( pLocale );

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* try to set the new locale */
    if( 0 == _imp_setProcessLocale( pLocale ) )
    {
        theProcessLocale = pLocale;
        ret = osl_Process_E_None;
    }

    pthread_mutex_unlock( &aLocalMutex );

    return ret;
}


