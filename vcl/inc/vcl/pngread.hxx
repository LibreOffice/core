/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pngread.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_PNGREAD_HXX
#define _SV_PNGREAD_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>
#include <vector>

// -------------
// - PNGReader -
// -------------

namespace vcl
{
    class PNGReaderImpl;

    class VCL_DLLPUBLIC PNGReader
    {
        PNGReaderImpl*          mpImpl;

    public:

        /* the PNG chunks are read within the c'tor, so the stream will
        be positioned at the end of the PNG */
        PNGReader( SvStream& rStm );
        ~PNGReader();

        BitmapEx                        Read();

        // retrieve every chunk that resides inside the PNG
        struct ChunkData
        {
            sal_uInt32                  nType;
            std::vector< sal_uInt8 >    aData;
        };
        const std::vector< ChunkData >& GetChunks() const;

        // TODO: when incompatible changes are possible again
        // the preview size hint should be redone
        static void SetPreviewSizeHint( const Size& r ) { aPreviewSizeHint = r; }
        static void DisablePreviewMode() { aPreviewSizeHint = Size(0,0); }
    private:
        static Size aPreviewSizeHint;
    };
}

#endif // _SV_PNGREAD_HXX
