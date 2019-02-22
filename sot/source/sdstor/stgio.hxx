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

#ifndef INCLUDED_SOT_SOURCE_SDSTOR_STGIO_HXX
#define INCLUDED_SOT_SOURCE_SDSTOR_STGIO_HXX

#include "stgcache.hxx"
#include "stgelem.hxx"
#include <tools/link.hxx>
#include <tools/solar.h>

class StgFATStrm;
class StgDataStrm;
class StgDirStrm;

enum class FatError
{
    Ok,
    WrongLength,
    UnrefChain,
    OutOfBounds,

    InMemoryError,
    OnFileError,
    BothError
};

class StgIo : public StgCache {
    void SetupStreams();            // load all internal streams
    bool         m_bCopied;
public:
    StgIo();
   ~StgIo();
    StgHeader    m_aHdr;              // storage file header
    StgFATStrm*  m_pFAT;              // FAT stream
    StgDirStrm*  m_pTOC;              // TOC stream
    StgDataStrm* m_pDataFAT;          // small data FAT stream
    StgDataStrm* m_pDataStrm;         // small data stream
    short        GetDataPageSize() const; // get the logical data page size
    bool Load();                    // load a storage file
    bool Init();                    // set up an empty file
    bool CommitAll();               // commit everything (root commit)

    FatError ValidateFATs( );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
