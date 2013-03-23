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

#pragma once
#if 1

#include <stgcache.hxx>
#include <stgelem.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>

class StgFATStrm;
class StgDataStrm;
class StgDirStrm;
class String;

enum FAT_ERROR
{
    FAT_OK,
    FAT_WRONGLENGTH,
    FAT_UNREFCHAIN,
    FAT_OVERWRITE,
    FAT_OUTOFBOUNDS,

    FAT_INMEMORYERROR,
    FAT_ONFILEERROR,
    FAT_BOTHERROR
};

struct StgLinkArg
{
    String aFile;
    sal_uLong nErr;
};

class StgIo : public StgCache {
    void SetupStreams();            // load all internal streams
    sal_Bool         bCopied;
public:
    StgIo();
   ~StgIo();
    StgHeader    aHdr;              // storage file header
    StgFATStrm*  pFAT;              // FAT stream
    StgDirStrm*  pTOC;              // TOC stream
    StgDataStrm* pDataFAT;          // small data FAT stream
    StgDataStrm* pDataStrm;         // small data stream
    short        GetDataPageSize(); // get the logical data page size
    sal_Bool Load();                    // load a storage file
    sal_Bool Init();                    // set up an empty file
    sal_Bool CommitAll();               // commit everything (root commit)

    static void SetErrorLink( const Link& );
    static const Link& GetErrorLink();
    sal_uLong ValidateFATs( );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
