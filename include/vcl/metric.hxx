/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

    FontInfo&           operator=( const FontInfo& );
    sal_Bool                operator==( const FontInfo& ) const;
    sal_Bool                operator!=( const FontInfo& rInfo ) const
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
    sal_Bool                operator==( const FontMetric& rMetric ) const;
    sal_Bool                operator!=( const FontMetric& rMetric ) const
                            { return !operator==( rMetric ); }
};

// ---------------
// - FontCharMap -
// ---------------

class VCL_DLLPUBLIC FontCharMap
{
private:
    const ImplFontCharMap* mpImpl;

public:
                        FontCharMap();
                        ~FontCharMap();

    sal_Bool                IsDefaultMap() const;
    sal_Bool                HasChar( sal_uInt32 ) const;
    int                 CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const;
    int                 GetCharCount() const;

    sal_uInt32          GetFirstChar() const;

    sal_uInt32          GetNextChar( sal_uInt32 ) const;
    sal_uInt32          GetPrevChar( sal_uInt32 ) const;

    int                 GetIndexFromChar( sal_uInt32 ) const;
    sal_uInt32          GetCharFromIndex( int ) const;


private:
    friend class OutputDevice;
    void                Reset( const ImplFontCharMap* pNewMap = NULL );

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
    sal_uInt16          mnLineCount;
    sal_Bool            mbEllipsis;

public:
                    TextRectInfo();

    sal_uInt16          GetLineCount() const { return mnLineCount; }
    long            GetMaxLineWidth() const { return mnMaxWidth; }
    sal_Bool            IsEllipses() const { return mbEllipsis; }

    sal_Bool            operator ==( const TextRectInfo& rInfo ) const
                        { return ((mnMaxWidth   == rInfo.mnMaxWidth)    &&
                                  (mnLineCount  == rInfo.mnLineCount)   &&
                                  (mbEllipsis   == rInfo.mbEllipsis)); }
    sal_Bool            operator !=( const TextRectInfo& rInfo ) const
                        { return !(TextRectInfo::operator==( rInfo )); }
};

inline TextRectInfo::TextRectInfo()
{
    mnMaxWidth      = 0;
    mnLineCount     = 0;
    mbEllipsis      = sal_False;
}

#endif // _SV_METRIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
