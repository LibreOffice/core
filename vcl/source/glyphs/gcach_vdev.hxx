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

#include <vcl/glyphcache.hxx>

// -----------------------------------------------------------------------

class VirtDevServerFont : public ServerFont
{
public:
    virtual bool                GetAntialiasAdvice( void ) const;
    virtual bool                GetGlyphBitmap1( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphBitmap8( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphOutline( int nGlyphIndex, PolyPolygon& ) const;

protected:
    friend class GlyphCache;
    static void                 AnnounceFonts( ImplDevFontList* );
    static void                 ClearFontList();

    static VirtDevServerFont*   CreateFont( const ImplFontSelectData& );
    virtual void                FetchFontMetric( ImplFontMetricData&, long& rFactor ) const;
    virtual sal_uLong               GetKernPairs( ImplKernPairData** ) const;
    virtual int                 GetGlyphKernValue( int, int ) const;

    virtual int                 GetGlyphIndex( sal_Unicode ) const;
    virtual void                InitGlyphData( int nGlyphIndex, GlyphData& ) const;

private:
                                VirtDevServerFont( const ImplFontSelectData& );
};

// -----------------------------------------------------------------------
