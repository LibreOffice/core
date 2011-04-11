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
#ifndef _INIMGR_HXX
#define _INIMGR_HXX

#include <tools/fsys.hxx>
#include <tools/string.hxx>

/*****************************************************************************
Purpose: Allows to work on a local set of initialisation files
If Update is used, the user must ensure that only one set of
Source and Destination Dir is used. Otherwise ForceUpdate has to be used
*****************************************************************************/

class IniManager
{
private:
    sal_Bool bUpdate;

    ByteString sGlobalDir;      /// holds the org. ini dir
    ByteString sLocalPath;      /// holds path of local ini dir

public:
    IniManager( ByteString &rDir, ByteString &rLocalDir );
    IniManager( ByteString &rDir );
    IniManager();

    ByteString ToLocal( ByteString &rPath );
    void Update();          /// Call ForceUpdate the First Time called
    void ForceUpdate();

    static ByteString GetLocalIni();
    static ByteString GetGlobalIni();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
