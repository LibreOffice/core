/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bmpcore.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:53:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    USHORT  mnLangNum;
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
    ULONG           nTotCount;
    USHORT          nPos;

    void            ImplCreate( SvStream& rStm,
                                const ::std::vector< DirEntry >& rInDirs,
                                const DirEntry& rOut,
                                const String& rName,
                                const LangInfo& rLang );

protected:

    virtual void    Message( const String& rText, BYTE cExitCode = EXIT_NOERROR );

public:

                    BmpCreator();
    virtual         ~BmpCreator();

    void            Create( const String& rSRSName,
                            const ::std::vector< String >& rInDirs,
                            const String& rOutName,
                            const LangInfo& rLang );
};

#endif // _BMPCORE_HXX
