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




#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

/*
 * NOTE: Since no one is really interested in correct unload behavior I've
 * disabled the shared library unload check. If you want to reenable it comment
 * the following line out
 */
#define NO_UNLOAD_CHECK

static const char *pprog_name   = "checkdll";
static const char *psymbol      = "GetVersionInfo";

void usage()
{
    fprintf(stderr, "usage: %s [-s] <dllname>\n", pprog_name);
    return;
}

int main(int argc, char *argv[])
{
    int     rc;
    int     silent=0;
    void    *phandle;
    char    *(*pfun)(void);

    if ( argc < 2 || argc > 4) {
        usage();
        return 1;
    }

    if ( !strcmp(argv[1],"-s") ) {
        silent = 1;
        ++argv, --argc;
    }

    if ( (rc = access( argv[1], R_OK )) == -1 ) {
        fprintf(stderr, "%s: ERROR: %s: %s\n",
                pprog_name, argv[1], strerror(errno));
        return 2;
    }

    if (!silent) printf("Checking DLL %s ...", argv[1]);
    fflush(stdout);

    if ( (phandle = dlopen(argv[1], RTLD_NOW)) != NULL ) {
        if  ( (pfun = (char *(*)(void))dlsym(phandle, psymbol)) != NULL ) {
            if (!silent) printf(": ok\n");
        }
        else
        {
            printf(": WARNING: %s\n", dlerror());
        }
#ifdef NO_UNLOAD_CHECK
        _exit(0);
#else
        dlclose(phandle);
#endif
        return 0;
    }

    printf(": ERROR: %s\n", dlerror());
    return 3;
}


