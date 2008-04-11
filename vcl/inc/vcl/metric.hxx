/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: metric.hxx,v $
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

#ifndef _SV_METRIC_HXX
#define _SV_METRIC_HXX

#include <vcl/dllapi.h>
#include <vcl/font.hxx>

class ImplFontMetric;
class ImplFontCharMap;

typedef sal_uInt32 sal_UCS4;

// ------------
// - FontInfo -
// ------------

class VCL_DLLPUBLIC FontInfo : public Font
{
    friend class OutputDevice;

protected:
    ImplFontMetric*     mpImplMetric;    // Implementation

public:
                        FontInfo();
                        FontInfo( const FontInfo& );
                        ~FontInfo();

    FontType            GetType() const;
    BOOL                IsDeviceFont() const;
    BOOL                SupportsLatin() const;
    BOOL                SupportsCJK() const;
    BOOL                SupportsCTL() const;

    FontInfo&           operator=( const FontInfo& );
    BOOL                operator==( const FontInfo& ) const;
    BOOL                operator!=( const FontInfo& rInfo ) const
                            { return !operator==( rInfo ); }
};

// --------------
// - FontMetric -
// --------------

class VCL_DLLPUBLIC FontMetric : public FontInfo
{
public:
                        FontMetric() {}
                        FontMetric( const FontMetric& );
                        ~FontMetric() {}

    long                GetAscent() const;
    long                GetDescent() const;
    long                GetIntLeading() const;
    long                GetExtLeading() const;
    long                GetLineHeight() const;
    long                GetSlant() const;

    FontMetric&         operator=( const FontMetric& rMetric );
    BOOL                operator==( const FontMetric& rMetric ) const;
    BOOL                operator!=( const FontMetric& rMetric ) const
                            { return !operator==( rMetric ); }
};

// ---------------
// - FontCharMap -
// ---------------

class VCL_DLLPUBLIC FontCharMap
{
private:
    ImplFontCharMap*    mpImpl;

public:
                        FontCharMap();
                        ~FontCharMap();

    BOOL                IsDefaultMap() const;
    BOOL                HasChar( sal_uInt32 ) const;
    int                 CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const;
    int                 GetCharCount() const;

    sal_uInt32          GetFirstChar() const;
    sal_uInt32          GetLastChar() const;

    sal_uInt32          GetNextChar( sal_uInt32 ) const;
    sal_uInt32          GetPrevChar( sal_uInt32 ) const;

    int                 GetIndexFromChar( sal_uInt32 ) const;
    sal_uInt32          GetCharFromIndex( int ) const;


private:
    friend class OutputDevice;
    void                Reset( ImplFontCharMap* pNewMap = NULL );

    // prevent assignment and copy construction
                        FontCharMap( const FontCharMap& );
    void                operator=( const FontCharMap& );
};

// ----------------
// - TextRectInfo -
// ----------------

class VCL_DLLPUBLIC TextRectInfo
{
    friend class OutputDevice;

private:
    long            mnMaxWidth;
    USHORT          mnLineCount;
    BOOL            mbEllipsis;

public:
                    TextRectInfo();

    USHORT          GetLineCount() const { return mnLineCount; }
    long            GetMaxLineWidth() const { return mnMaxWidth; }
    BOOL            IsEllipses() const { return mbEllipsis; }

    BOOL            operator ==( const TextRectInfo& rInfo ) const
                        { return ((mnMaxWidth   == rInfo.mnMaxWidth)    &&
                                  (mnLineCount  == rInfo.mnLineCount)   &&
                                  (mbEllipsis   == rInfo.mbEllipsis)); }
    BOOL            operator !=( const TextRectInfo& rInfo ) const
                        { return !(TextRectInfo::operator==( rInfo )); }
};

inline TextRectInfo::TextRectInfo()
{
    mnMaxWidth      = 0;
    mnLineCount     = 0;
    mbEllipsis      = FALSE;
}

#endif // _SV_METRIC_HXX
