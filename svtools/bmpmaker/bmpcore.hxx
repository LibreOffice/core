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
