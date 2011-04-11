/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.
 *
 *
 *    GNU General Public License, version 2
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 *
 ************************************************************************/
#ifndef _IPDF_PNGHELPER_HXX
#define _IPDF_PNGHELPER_HXX

#include "sal/types.h"
#include "pdfioutdev_gpl.hxx"


namespace pdfi
{
    class PngHelper
    {
        static sal_uInt32 crc_table[ 256 ];
        static bool bCRCTableInit;

        static void initCRCTable();
        static void appendFileHeader( OutputBuffer& o_rOutputBuf );
        static size_t startChunk( const char* pChunkName, OutputBuffer& o_rOut );
        static void endChunk( size_t nStart, OutputBuffer& o_rOut );

        static void set( sal_uInt32 i_nValue, OutputBuffer& o_rOutputBuf, size_t i_nIndex );
        static void append( sal_uInt32 i_nValue, OutputBuffer& o_rOutputBuf )
        {
            size_t nCur = o_rOutputBuf.size();
            o_rOutputBuf.insert( o_rOutputBuf.end(), 4, (Output_t)0 );
            set( i_nValue, o_rOutputBuf, nCur );
        }

        static void appendIHDR( OutputBuffer& o_rOutputBuf, int width, int height, int depth, int colortype );
        static void appendIEND( OutputBuffer& o_rOutputBuf );

    public:
        static void updateCRC( sal_uInt32& io_rCRC, const sal_uInt8* i_pBuf, size_t i_nLen );
        static sal_uInt32 getCRC( const sal_uInt8* i_pBuf, size_t i_nLen );

        // deflates the passed buff i_pBuf and appends it to the output vector
        // returns the number of bytes added to the output
        static sal_uInt32 deflateBuffer( const Output_t* i_pBuf, size_t i_nLen, OutputBuffer& o_rOut );

        static void createPng( OutputBuffer&     o_rOutputBuf,
                               Stream*           str,
                               int               width,
                               int               height,
                               GfxRGB&           zeroColor,
                               GfxRGB&           oneColor,
                               bool              bIsMask
                               );
        static void createPng( OutputBuffer& o_rOutputBuf,
                               Stream* str,
                               int width, int height, GfxImageColorMap* colorMap,
                               Stream* maskStr,
                               int maskWidth, int maskHeight, GfxImageColorMap* maskColorMap );

        // for one bit masks
        static void createPng( OutputBuffer& o_rOutputBuf,
                               Stream* str,
                               int width, int height, GfxImageColorMap* colorMap,
                               Stream* maskStr,
                               int maskWidth, int maskHeight, bool maskInvert );

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
