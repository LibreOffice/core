/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
