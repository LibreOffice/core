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

#include <buffer.hxx>
#include <parser.hxx>

#include <basic/sberrors.hxx>

// Patch of a Location

void SbiBuffer::Patch( sal_uInt32 off, sal_uInt32 val )
{
    if ((off + sizeof(sal_uInt32)) <= GetSize())
        write(aBuf.begin() + off, val);
}

// Forward References upon label and procedures
// establish a linkage. The beginning of the linkage is at the passed parameter,
// the end of the linkage is 0.

void SbiBuffer::Chain( sal_uInt32 off )
{
    for (sal_uInt32 i = off; i;)
    {
        if ((i + sizeof(sal_uInt32)) > GetSize())
        {
            pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, "BACKCHAIN" );
            break;
        }
        auto ip = aBuf.begin() + i;
        i = ip[0] | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24);
        write(ip, GetSize());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
