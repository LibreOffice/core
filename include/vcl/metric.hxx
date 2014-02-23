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

#ifndef INCLUDED_VCL_METRIC_HXX
#define INCLUDED_VCL_METRIC_HXX

#include <vcl/dllapi.h>
#include <vcl/font.hxx>

class ImplFontMetric;
class ImplFontCharMap;

typedef sal_uInt32 sal_UCS4;


// - FontInfo -


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
    bool                operator==( const FontInfo& ) const;
    bool                operator!=( const FontInfo& rInfo ) const
                            { return !operator==( rInfo ); }
};


// - FontMetric -


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
    bool                operator==( const FontMetric& rMetric ) const;
    bool                operator!=( const FontMetric& rMetric ) const
                            { return !operator==( rMetric ); }
};


// - FontCharMap -


class VCL_DLLPUBLIC FontCharMap
{
private:
    const ImplFontCharMap* mpImpl;

public:
                        FontCharMap();
                        ~FontCharMap();

    bool                IsDefaultMap( void ) const;
    bool                HasChar( sal_UCS4 ) const;
    int                 CountCharsInRange( sal_UCS4 cMin, sal_UCS4 cMax ) const;
    int                 GetCharCount( void ) const;

    sal_UCS4            GetFirstChar() const;

    sal_UCS4            GetNextChar( sal_UCS4 ) const;
    sal_UCS4            GetPrevChar( sal_UCS4 ) const;

    int                 GetIndexFromChar( sal_UCS4 ) const;
    sal_UCS4            GetCharFromIndex( int ) const;


private:
    friend class OutputDevice;
    void                Reset( const ImplFontCharMap* pNewMap = NULL );

    // prevent assignment and copy construction
                        FontCharMap( const FontCharMap& );
    void                operator=( const FontCharMap& );
};


// - TextRectInfo -


class VCL_DLLPUBLIC TextRectInfo
{
    friend class OutputDevice;

private:
    long            mnMaxWidth;
    sal_uInt16          mnLineCount;
    bool            mbEllipsis;

public:
                    TextRectInfo();

    sal_uInt16          GetLineCount() const { return mnLineCount; }
    long            GetMaxLineWidth() const { return mnMaxWidth; }
    bool            IsEllipses() const { return mbEllipsis; }

    bool            operator ==( const TextRectInfo& rInfo ) const
                        { return ((mnMaxWidth   == rInfo.mnMaxWidth)    &&
                                  (mnLineCount  == rInfo.mnLineCount)   &&
                                  (mbEllipsis   == rInfo.mbEllipsis)); }
    bool            operator !=( const TextRectInfo& rInfo ) const
                        { return !(TextRectInfo::operator==( rInfo )); }
};

inline TextRectInfo::TextRectInfo()
{
    mnMaxWidth      = 0;
    mnLineCount     = 0;
    mbEllipsis      = false;
}

#endif // INCLUDED_VCL_METRIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
