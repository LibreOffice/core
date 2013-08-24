/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _OSL_UUNXAPI_HXX_
#define _OSL_UUNXAPI_HXX_

#include "uunxapi.h"

#include <rtl/ustring.hxx>

namespace osl
{
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

    inline int lstat(const rtl::OUString& ustrPath, struct stat& buf)
    {
        return lstat_u(ustrPath.pData, &buf);
    }

    inline int mkdir(const rtl::OUString& aPath, mode_t aMode)
    {
        return mkdir_u(aPath.pData, aMode);
    }
} // end namespace osl

#endif /* _OSL_UUNXAPI_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
