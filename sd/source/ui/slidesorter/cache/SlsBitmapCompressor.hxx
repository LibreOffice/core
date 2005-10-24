/*************************************************************************
 *
 *  $RCSfile: SlsBitmapCompressor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:39:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_BITMAP_COMPRESSOR_HXX
#define SD_SLIDESORTER_BITMAP_COMPRESSOR_HXX

#include <sal/types.h>
#include <tools/gen.hxx>
#include <boost/shared_ptr.hpp>

class BitmapEx;

namespace sd { namespace slidesorter { namespace cache {

class BitmapReplacement;


/** This interface class provides the minimal method set for classes that
    implement the compression and decompression of preview bitmaps.
*/
class BitmapCompressor
{
public:
    /** Compress the given bitmap into a replacement format that is specific
        to the compressor class.
    */
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (
        const ::boost::shared_ptr<BitmapEx>& rpBitmap) const = 0;

    /** Decompress the given replacement data into a preview bitmap.
        Depending on the compression technique the returned bitmap may
        differ from the original bitmap given to the Compress() method.  It
        may even of the wrong size or empty or the NULL pointer.  It is the
        task of the caller to create a new preview bitmap if the returned
        one is not as desired.
    */
    virtual ::boost::shared_ptr<BitmapEx> Decompress (const BitmapReplacement& rBitmapData)const=0;

    /** Return whether the compression and decompression is lossless.  This
        value is used by the caller of Decompress() to decide whether to use
        the returned bitmap as is or if a new preview has to be created.
    */
    virtual bool IsLossless (void) const = 0;
};



/** Interface for preview bitmap replacements.  Each bitmap
    compressor/decompressor has to provide an implementation that is
    suitable to store the compressed bitmaps.
*/
class BitmapReplacement
{
public:
    virtual sal_Int32 GetMemorySize (void) const { return 0; }
};




/** This is one trivial bitmap compressor.  It stores bitmaps unmodified
    instead of compressing them.
    This compressor is lossless.
*/
class NoBitmapCompression
    : public BitmapCompressor
{
    class DummyReplacement;
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (
        const ::boost::shared_ptr<BitmapEx>& rpBitmap) const;
    virtual ::boost::shared_ptr<BitmapEx> Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};




/** This is another trivial bitmap compressor.  Instead of compressing a
    bitmap, it throws the bitmap away.  Its Decompress() method returns a
    NULL pointer.  The caller has to create a new preview bitmap instead.
    This compressor clearly is not lossless.
*/
class CompressionByDeletion
    : public BitmapCompressor
{
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (
        const ::boost::shared_ptr<BitmapEx>& rpBitmap) const;
    virtual ::boost::shared_ptr<BitmapEx> Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};




/** Compress a preview bitmap by reducing its resolution.  While the aspect
    ratio is maintained the horizontal resolution is scaled down to 100
    pixels.
    This compressor is not lossless.
*/
class ResolutionReduction
    : public BitmapCompressor
{
    class ResolutionReducedReplacement;
    static const sal_Int32 mnWidth = 100;
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (
        const ::boost::shared_ptr<BitmapEx>& rpBitmap) const;
    /** Scale the replacement bitmap up to the original size.
    */
    virtual ::boost::shared_ptr<BitmapEx> Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};




/** Compress preview bitmaps using the PNG format.
    This compressor is lossless.
*/
class PngCompression
    : public BitmapCompressor
{
    class PngReplacement;
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (
        const ::boost::shared_ptr<BitmapEx>& rpBitmap) const;
    virtual ::boost::shared_ptr<BitmapEx> Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};


} } } // end of namespace ::sd::slidesorter::cache

#endif
