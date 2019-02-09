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



#include <rtl/string.h>

#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#  ifndef __USE_GNU
#  define __USE_GNU
#  endif
#endif
#include <dlfcn.h>

SAL_DLLPUBLIC_EXPORT void initpyuno ()
{
    Dl_info dl_info;
    void (*func)(void);

    if (dladdr((void*)&initpyuno, &dl_info) != 0) {
        void* h = 0;
    size_t len = strrchr(dl_info.dli_fname, '/') - dl_info.dli_fname + 1;
    char* libname = malloc(len + RTL_CONSTASCII_LENGTH( SAL_DLLPREFIX "pyuno" SAL_DLLEXTENSION ) + 1);
        strncpy(libname, dl_info.dli_fname, len);
        strcpy(libname + (len), SAL_DLLPREFIX "pyuno" SAL_DLLEXTENSION);

        h = dlopen (libname, RTLD_NOW | RTLD_GLOBAL);
    free(libname);
        if( h )
        {
            func = (void (*)())dlsym (h, "initpyuno");
            (func) ();
        }
    }
}
