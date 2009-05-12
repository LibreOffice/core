/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xlfd_smpl.hxx,v $
 * $Revision: 1.7 $
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
#ifndef XLFD_SIMPLE_HXX
#define XLFD_SIMPLE_HXX

#include <salunx.h>
#include <vcl/vclenum.hxx>
#include <tools/string.hxx>

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

