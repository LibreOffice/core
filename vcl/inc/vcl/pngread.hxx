/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pngread.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:02:55 $
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

#ifndef _SV_PNGREAD_HXX
#define _SV_PNGREAD_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
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
