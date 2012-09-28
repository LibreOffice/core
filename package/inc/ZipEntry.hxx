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
    sal_Int64 nCompressedSize;
    sal_Int64 nSize;
    sal_Int64 nOffset;
    sal_Int16 nPathLen;
    sal_Int16 nExtraLen;
    ::rtl::OUString sPath;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
