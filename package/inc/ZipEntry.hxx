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
#ifndef _ZIP_ENTRY_HXX_
#define _ZIP_ENTRY_HXX_

#include <rtl/ustring.hxx>

struct ZipEntry
{
    sal_Int16 nVersion;
    sal_Int16 nFlag;
    sal_Int16 nMethod;
    sal_Int32 nTime;
    sal_Int32 nCrc;
    sal_Int32 nCompressedSize;
    sal_Int32 nSize;
    sal_Int32 nOffset;
    sal_Int16 nPathLen;
    sal_Int16 nExtraLen;
    ::rtl::OUString sPath;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
