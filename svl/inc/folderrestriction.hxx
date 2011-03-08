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

#ifndef SVTOOLS_FOLDER_RESTRICTION_HXX
#define SVTOOLS_FOLDER_RESTRICTION_HXX

#include "svl/svldllapi.h"
#include <tools/string.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

//........................................................................
namespace svt
{
//........................................................................

    /** retrieves a list of folders which's access is not restricted.

        <p>Note that this is not meant as security feature, but only as
        method to restrict some UI presentation, such as browsing
        in the file open dialog.</p>
    */
    SVL_DLLPUBLIC void getUnrestrictedFolders( ::std::vector< String >& _rFolders );

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_FOLDER_RESTRICTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
