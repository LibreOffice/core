/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlfd_smpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:10:32 $
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
#ifndef XLFD_SIMPLE_HXX
#define XLFD_SIMPLE_HXX

#ifndef _SALUNX_H
#include <salunx.h>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vclenum.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

class AttributeProvider;

// --------------------------------------------------------------------------
//
//
// broken down structure equivalent to a Xlfd string
//
//
// --------------------------------------------------------------------------

enum XlfdFonttype {
    eTypeUnknown        = TYPE_DONTKNOW,
    eTypeBitmap         = TYPE_RASTER,
    eTypeScalableBitmap = TYPE_VECTOR,
    eTypeScalable       = TYPE_SCALABLE
};

class Xlfd {

    public:

        unsigned short  mnFoundry;
        unsigned short  mnFamily;
        unsigned short  mnWeight;
        unsigned short  mnSlant;
        unsigned short  mnSetwidth;
        unsigned short  mnAddstyle;
        unsigned short  mnPixelSize;
        unsigned short  mnPointSize;
        unsigned short  mnResolutionX;
        unsigned short  mnResolutionY;
        unsigned char   mcSpacing;
        unsigned short  mnAverageWidth;
        unsigned short  mnCharset;

                        // all foundry, family, weight ... information referres
                        // to this factory
        AttributeProvider *mpFactory;

        Bool            IsConformant( const char* pXlfdstring ) const;

    public:
                        Xlfd();
        Bool            FromString( const char* pXlfdstring,
                                    AttributeProvider *pFactory );
        Bool            SameFontoutline( const Xlfd *pComparedTo ) const ;
        XlfdFonttype    Fonttype() const ;
        unsigned short  GetEncoding() const ;
        #if OSL_DEBUG_LEVEL > 1
        const char*     ToString( ByteString &rString ) const ;
        void            Dump() const;
        #endif
};

extern "C" int
XlfdCompare( const void *vFrom, const void *vTo );

#endif /* XLFD_SIMPLE_HXX */

