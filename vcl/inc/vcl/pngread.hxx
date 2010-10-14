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

        /* an empty preview size hint (=default) will read the whole image
        */
        BitmapEx                        Read( const Size& i_rPreviewHint = Size() );

        // retrieve every chunk that resides inside the PNG
        struct ChunkData
        {
            sal_uInt32                  nType;
            std::vector< sal_uInt8 >    aData;
        };
        const std::vector< ChunkData >& GetChunks() const;

        void SetIgnoreGammaChunk( sal_Bool b );
    };
}

#endif // _SV_PNGREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
