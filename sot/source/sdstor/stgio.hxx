/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _STGIO_HXX
#define _STGIO_HXX

#include <stgcache.hxx>
#include <stgelem.hxx>
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
