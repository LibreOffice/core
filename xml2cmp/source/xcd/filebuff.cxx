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
#if defined(WNT) || defined(OS2)
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
