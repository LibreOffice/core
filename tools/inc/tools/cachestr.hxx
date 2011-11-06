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



#ifndef _CACHESTR_HXX
#define _CACHESTR_HXX

#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include "tools/toolsdllapi.h"

// -----------------
// - SvCacheStream -
// -----------------

class TempFile;
class TOOLS_DLLPUBLIC SvCacheStream : public SvStream
{
private:
    String          aFileName;
    sal_uIntPtr           nMaxSize;
    int             bPersistent;

    SvStream*       pSwapStream;
    SvStream*       pCurrentStream;
    TempFile*       pTempFile;

    Link            aFilenameLinkHdl;

    TOOLS_DLLPRIVATE virtual sal_uIntPtr   GetData( void* pData, sal_uIntPtr nSize );
    TOOLS_DLLPRIVATE virtual sal_uIntPtr   PutData( const void* pData, sal_uIntPtr nSize );
    TOOLS_DLLPRIVATE virtual sal_uIntPtr   SeekPos( sal_uIntPtr nPos );
    TOOLS_DLLPRIVATE virtual void    FlushData();
    TOOLS_DLLPRIVATE virtual void    SetSize( sal_uIntPtr nSize );

public:
                    SvCacheStream( sal_uIntPtr nMaxMemSize = 0 );
                    SvCacheStream( const String &rFileName,
                                   sal_uIntPtr nExpectedSize = 0,
                                   sal_uIntPtr nMaxMemSize = 0 );
                    ~SvCacheStream();

    void            SetFilenameHdl( const Link& rLink);
    const Link&     GetFilenameHdl() const;
    void            SetFilename( const String& rFN )
                 { aFileName = rFN; } // darf nur vom FilenameHdl gerufen werden!
    const String&   GetFilename() const { return aFileName; }

    void            SwapOut();
    const void*     GetBuffer();
    sal_uIntPtr           GetSize();

    sal_Bool            IsPersistent() { return bPersistent != 0; }
    void            SetPersistence( sal_Bool b = sal_True ) { bPersistent = b; }
    void            SetSwapStream( SvStream *p )
                 { pSwapStream = p; } // darf nur vom FilenameHdl gerufen werden!
};

#endif
