/*************************************************************************
 *
 *  $RCSfile: system.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-28 13:08:45 $
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

#ifdef NO_PTHREAD_RTL

static pthread_mutex_t getrtl_mutex = PTHREAD_MUTEX_INITIALIZER;

/* struct passwd differs on some platforms */
#if defined NETBSD || defined MACOSX || defined FREEBSD
#include <pwd.h>
#include <sys/types.h>


struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
{
      struct passwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = getpwnam(name) )
      {
        int nname, npasswd, nclass, ngecos, ndir;

        nname= strlen(res->pw_name)+1;
        npasswd= strlen(res->pw_passwd)+1;
        nclass= strlen(res->pw_class)+1;
        ngecos= strlen(res->pw_gecos)+1;
        ndir= strlen(res->pw_dir)+1;

        if (nname+npasswd+nclass+ngecos
                +ndir+strlen(res->pw_shell) < size)
        {
            memcpy(s, res, sizeof(struct passwd));

            strcpy(buffer, res->pw_name);
              s->pw_name = buffer;
            buffer += nname;

            strcpy(buffer, res->pw_passwd);
              s->pw_passwd = buffer;
            buffer += npasswd;

            strcpy(buffer, res->pw_class);
              s->pw_class = buffer;
            buffer += nclass;

            strcpy(buffer, res->pw_gecos);
              s->pw_gecos = buffer;
            buffer += ngecos;

            strcpy(buffer, res->pw_dir);
              s->pw_dir = buffer;
            buffer += ndir;

            strcpy(buffer, res->pw_shell);
              s->pw_shell = buffer;

               res = s;
        }
        else
            res = 0;
    }

    pthread_mutex_unlock(&getrtl_mutex);

      return res;
}

struct tm *localtime_r(const time_t *timep, struct tm *buffer)
{
    struct tm* res;

      pthread_mutex_lock(&getrtl_mutex);

    if (res = localtime(timep))
    {
        memcpy(buffer, res, sizeof(struct tm));
        res = buffer;
    }

    pthread_mutex_unlock(&getrtl_mutex);

    return res;
}

struct tm *gmtime_r(const time_t *timep, struct tm *buffer)
{
    struct tm* res;

      pthread_mutex_lock(&getrtl_mutex);

    if (res = gmtime(timep))
    {
        memcpy(buffer, res, sizeof(struct tm));
        res = buffer;
    }

    pthread_mutex_unlock(&getrtl_mutex);

    return res;
}

#endif  /* defined NETBSD || defined MACOSX */

#ifdef SCO
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>

struct spwd *getspnam_r(const char *name, struct spwd* s, char* buffer, int size )
{
      struct spwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = getspnam(name) )
      {
        int nnamp;

        nnamp = strlen(res->sp_namp)+1;

        if (nnamp+strlen(res->sp_pwdp) < size) {
            memcpy(s, res, sizeof(struct spwd));

            strcpy(buffer, res->sp_namp);
              s->sp_namp = buffer;
            buffer += nnamp;

            strcpy(buffer, res->sp_pwdp);
              s->sp_pwdp = buffer;

               res = s;
        }
        else
            res = 0;
    }

      pthread_mutex_unlock(&getrtl_mutex);

      return res;
}

struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
{
      struct passwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = getpwnam(name) )
      {
        int nname, npasswd, nage;
        int ncomment, ngecos, ndir;

        nname= strlen(res->pw_name)+1;
        npasswd= strlen(res->pw_passwd)+1;
        nage= strlen(res->pw_age)+1;
        ncomment= strlen(res->pw_comment)+1;
        ngecos= strlen(res->pw_gecos)+1;
        ndir= strlen(res->pw_dir)+1;

        if (nname+npasswd+nage+ncomment+ngecos+ndir
                +strlen(res->pw_shell) < size)
        {
            memcpy(s, res, sizeof(struct passwd));

            strcpy(buffer, res->pw_name);
              s->pw_name = buffer;
            buffer += nname;

            strcpy(buffer, res->pw_passwd);
              s->pw_passwd = buffer;
            buffer += npasswd;

            strcpy(buffer, res->pw_age);
              s->pw_age = buffer;
            buffer += nage;

            strcpy(buffer, res->pw_comment);
              s->pw_comment = buffer;
            buffer += ncomment;

            strcpy(buffer, res->pw_gecos);
              s->pw_gecos = buffer;
            buffer += ngecos;

            strcpy(buffer, res->pw_dir);
              s->pw_dir = buffer;
            buffer += ndir;

            strcpy(buffer, res->pw_shell);
              s->pw_shell = buffer;

               res = s;
        }
        else
            res = 0;
    }

    pthread_mutex_unlock(&getrtl_mutex);

      return res;
}
#endif /* defined SCO */

extern int h_errno;

struct hostent *gethostbyname_r(const char *name, struct hostent *result,
                                char *buffer, int buflen, int *h_errnop)
{
    /* buffer layout:   name\0
     *                  array_of_pointer_to_aliases
     *                  NULL
     *                  alias1\0...aliasn\0
     *                  array_of_pointer_to_addresses
     *                  NULL
     *                  addr1addr2addr3...addrn
     */
      struct hostent* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = gethostbyname(name) )
      {
        int nname, naliases, naddr_list, naliasesdata, ncntaddr_list, n;
        char **p, **parray, *data;

        /* Check buffer size before copying, we want to leave the
         * buffers unmodified in case something goes wrong.
         *
         * Is this required?
         */

        nname= strlen(res->h_name)+1;

        naliases = naddr_list = naliasesdata = 0;

        for ( p = res->h_aliases; *p != NULL; p++) {
            naliases++;
            naliasesdata += strlen(*p)+1;
        }

        for ( p = res->h_addr_list; *p != NULL; p++)
            naddr_list++;

        if ( nname
             + (naliases+1)*sizeof(char*) + naliasesdata
             + (naddr_list+1)*sizeof(char*) + naddr_list*res->h_length
             <= buflen )
        {
            memcpy(result, res, sizeof(struct hostent));

            strcpy(buffer, res->h_name);
              result->h_name = buffer;
            buffer += nname;

            result->h_aliases = buffer;
            parray = (char**)buffer;
            data = buffer + (naliases+1)*sizeof(char*);
            for ( p = res->h_aliases; *p != NULL; p++) {
                n = strlen(*p)+1;
                *parray++ = data;
                memcpy(data, *p, n);
                data += n;
            }
            *parray = NULL;
            buffer = data;

            result->h_addr_list = buffer;
            parray = (char**)buffer;
            data = buffer + (naddr_list+1)*sizeof(char*);
            for ( p = res->h_addr_list; *p != NULL; p++) {
                *parray++ = data;
                memcpy(data, *p, res->h_length);
                data += res->h_length;
            }
            *parray = NULL;

               res = result;
        }
        else
        {
            errno = ERANGE;
            res = NULL;
        }
    }
    else
    {
        *h_errnop = h_errno;
    }

    pthread_mutex_unlock(&getrtl_mutex);

      return res;
}


#endif /* NO_PTHREAD_RTL */

#if (defined (LINUX) && (GLIBC >= 2))
/* The linux kernel thread implemention, always return the pid of the
   thread subprocess and not of the main process. So we save the main
   pid at startup
*/

static pid_t pid = -1;

static void savePid(void) __attribute__((constructor));

static void savePid(void)
{
    if (pid == -1)
        pid = __getpid();
}

pid_t getpid(void)
{
    if (pid == -1)
        savePid();

    return (pid);
}
#endif /*  (defined (LINUX) && (GLIBC >= 2)) */

#ifdef NO_PTHREAD_SEMAPHORES
int sem_init(sem_t* sem, int pshared, unsigned int value)
{
    pthread_mutex_init(&sem->mutex, PTHREAD_MUTEXATTR_DEFAULT);
    pthread_cond_init(&sem->increased, PTHREAD_CONDATTR_DEFAULT);

    sem->value = (int)value;
    return 0;
}

int sem_destroy(sem_t* sem)
{
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->increased);
    sem->value = 0;
    return 0;
}

int sem_wait(sem_t* sem)
{
    pthread_mutex_lock(&sem->mutex);

    while (sem->value <= 0)
    {
        pthread_cond_wait(&sem->increased, &sem->mutex);
    }

    sem->value--;
    pthread_mutex_unlock(&sem->mutex);

    return 0;
}

int sem_trywait(sem_t* sem)
{
    int result = 0;

    pthread_mutex_lock(&sem->mutex);

    if (sem->value > 0)
    {
        sem->value--;
    }
    else
    {
        errno = EAGAIN;
        result = -1;
    }

    pthread_mutex_unlock(&sem->mutex);

    return result;
}

int sem_post(sem_t* sem)
{
    pthread_mutex_lock(&sem->mutex);

    sem->value++;

    pthread_mutex_unlock(&sem->mutex);

    pthread_cond_signal(&sem->increased);

    return 0;
}
#endif

