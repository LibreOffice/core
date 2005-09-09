/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gcach_vdev.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:14:36 $
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

#include <glyphcache.hxx>

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
    virtual ULONG               GetKernPairs( ImplKernPairData** ) const;
    virtual int                 GetGlyphKernValue( int, int ) const;

    virtual int                 GetGlyphIndex( sal_Unicode ) const;
    virtual void                InitGlyphData( int nGlyphIndex, GlyphData& ) const;

private:
                                VirtDevServerFont( const ImplFontSelectData& );
};

// -----------------------------------------------------------------------
