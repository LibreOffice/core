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



#ifndef _FILE_ERROR_TRANSL_H_
#define _FILE_ERROR_TRANSL_H_

#include <osl/file.h>
#include <sal/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************
   oslTranslateFileError
   Translate errno's to osl file errors

   @param bIsError [in] specifies if nErrno
           should be interpreted as error,
          some libc functions signaling an error
           but errno is nevertheless 0 in this
           case the function should at least
           return osl_File_E_Unknown but in no
           case osl_File_E_None!

   @param nErrno [in] the errno if errno is 0
            and bIsError is true the function
           returns osl_File_E_Unknown

   @returns the osl error code appropriate to
              the errno

 *********************************************/

#define OSL_FET_SUCCESS sal_False
#define OSL_FET_ERROR   sal_True

oslFileError oslTranslateFileError(sal_Bool bIsError, int Errno);


#ifdef __cplusplus
}
#endif

#endif
