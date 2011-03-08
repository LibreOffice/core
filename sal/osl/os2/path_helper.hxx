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

// YD #pragma warning (disable : 4800)

#ifndef _PATH_HELPER_HXX_
#define _PATH_HELPER_HXX_

#include "path_helper.h"
#include <rtl/ustring.hxx>

namespace osl
{

/*******************************************************************
 osl_systemPathEnsureSeparator
 Adds a trailing path separator to the given system path if not
 already there and if the path is not the root path or a logical
 drive alone
 ******************************************************************/

inline void systemPathEnsureSeparator(/*inout*/ rtl::OUString& Path)
{
    osl_systemPathEnsureSeparator(&Path.pData);
}

/*******************************************************************
 osl_systemPathRemoveSeparator
 Removes the last separator from the given system path if any and
 if the path is not the root path '\'
 ******************************************************************/

inline void systemPathRemoveSeparator(/*inout*/ rtl::OUString& Path)
{
    osl_systemPathRemoveSeparator(&Path.pData);
}

/*******************************************************************
 osl_systemPathIsLogicalDrivePattern
 ******************************************************************/

inline bool systemPathIsLogicalDrivePattern(/*in*/ const rtl::OUString& path)
{
    return osl_systemPathIsLogicalDrivePattern(path.pData);
}

} // end namespace osl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
