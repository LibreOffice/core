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



#ifndef _BMPCORE_HXX
#define _BMPCORE_HXX

#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <vcl/bitmap.hxx>
#include <vector>

// --------------
// - Exit codes -
// --------------

#define EXIT_NOERROR                0
#define EXIT_MISSING_BITMAP         1
#define EXIT_NOSRSFILE              2
#define EXIT_NOIMGLIST              3
#define EXIT_DIMENSIONERROR         4
#define EXIT_IOERROR                5
#define EXIT_COMMONERROR            6
#define EXIT_MISSING_RESOURCE       7
#define EXIT_COLORDEPTHERROR        8
#define EXIT_MISSING_SOLARSRC_ENV   9

// ------------
// - LangInfo -
// ------------

struct LangInfo
{
    char    maLangDir[ 257 ];
    sal_uInt16  mnLangNum;
};

// --------------
// - BmpCreator -
// --------------

class BmpCreator
{
private:

    Bitmap          aOutBmp;
    Size            aOneSize;
    Size            aTotSize;
    Point           aPos;
    SvFileStream*   pSRS;
    sal_uLong           nTotCount;
    sal_uInt16          nPos;

    void            ImplCreate( const ::std::vector< DirEntry >& rInDirs,
                                const DirEntry& rOut,
                                const String& rName,
                                const LangInfo& rLang );

protected:

    virtual void    Message( const String& rText, sal_uInt8 cExitCode = EXIT_NOERROR );

public:

                    BmpCreator();
    virtual         ~BmpCreator();

    void            Create( const String& rSRSName,
                            const ::std::vector< String >& rInDirs,
                            const String& rOutName,
                            const LangInfo& rLang );
};

#endif // _BMPCORE_HXX
