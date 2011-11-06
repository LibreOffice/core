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



 #ifndef _OSL_UUNXAPI_HXX_
 #define _OSL_UUNXAPI_HXX_

 #ifndef _OSL_UUNXAPI_H_
 #include "uunxapi.h"
 #endif

 #ifndef _RTL_USTRING_HXX_
 #include <rtl/ustring.hxx>
 #endif

 namespace osl
 {

     /***********************************
      osl::access

      @see access
      **********************************/

     inline int access(const rtl::OUString& ustrPath, int mode)
      {
         return access_u(ustrPath.pData, mode);
       }

     /***********************************
      osl::realpath

      @descr
      The return value differs from the
      realpath function

      @returns sal_True on success else
      sal_False

      @see realpath
      **********************************/

     inline sal_Bool realpath(
         const rtl::OUString& ustrFileName,
        rtl::OUString& ustrResolvedName)
     {
         return realpath_u(ustrFileName.pData, &ustrResolvedName.pData);
      }


     /***********************************
      osl::lstat

      @see lstat
      **********************************/

      inline int lstat(const rtl::OUString& ustrPath, struct stat& buf)
      {
           return lstat_u(ustrPath.pData, &buf);
      }

    /***********************************
          osl::mkdir
          @see mkdir
          **********************************/
    inline int mkdir(const rtl::OUString& aPath, mode_t aMode)
    {
        return mkdir_u(aPath.pData, aMode);
    }

 } // end namespace osl


 #endif /* _OSL_UUNXAPI_HXX_ */

