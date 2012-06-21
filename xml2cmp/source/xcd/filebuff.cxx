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

#ifdef AIX
#    undef _THREAD_SAFE
#endif

#include "filebuff.hxx"

#include <string.h>
#include <fstream>
#include <ctype.h>


bool
LoadXmlFile( Buffer &           o_rBuffer,
             const char *       i_sXmlFilePath )
{
    std::ifstream aXmlFile;

    aXmlFile.open(i_sXmlFilePath, std::ios::in
#if defined(WNT)
                                          | std::ios::binary
#endif // WNT
    );

    if (! aXmlFile)
        return false;

    // Prepare buffer:
    aXmlFile.seekg(0, std::ios::end);
    unsigned long nBufferSize = (unsigned long) aXmlFile.tellg();
    o_rBuffer.SetSize(nBufferSize + 1);
    o_rBuffer.Data()[nBufferSize] = '\0';
    aXmlFile.seekg(0);

    // Read file:
    aXmlFile.read(o_rBuffer.Data(), (int) nBufferSize);
    bool ret = aXmlFile.good() != 0;
    aXmlFile.close();
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
