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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_BUNDLES_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_BUNDLES_HXX

#include <sal/types.h>
#include "cgmtypes.hxx"
#include <vcl/salbtype.hxx>
#include <vector>
#include <memory>


class Bundle
{

    long                mnBundleIndex;
    sal_uInt32          mnColor;

public:
    void                SetColor( sal_uInt32 nColor ) ;
    sal_uInt32          GetColor() const { return mnColor;}
    long                GetIndex() const { return mnBundleIndex; } ;
    void                SetIndex( long nBundleIndex ) { mnBundleIndex = nBundleIndex; } ;

    Bundle()
        : mnBundleIndex( 0 )
        , mnColor( 0 )
        {};

    virtual std::unique_ptr<Bundle> Clone() { return std::make_unique<Bundle>( *this ); };

    virtual            ~Bundle() {} ;

    Bundle(Bundle const &) = default;
    Bundle(Bundle &&) = default;
    Bundle & operator =(Bundle const &) = default;
    Bundle & operator =(Bundle &&) = default;
};


class LineBundle : public Bundle
{
public:

    LineType            eLineType;
    double              nLineWidth;

    LineBundle()
        : eLineType(LT_SOLID)
        , nLineWidth(0)
    {}

    virtual std::unique_ptr<Bundle> Clone() override { return std::make_unique<LineBundle>( *this ); }
};


class MarkerBundle : public Bundle
{
public:

    MarkerType          eMarkerType;
    double              nMarkerSize;

    MarkerBundle()
        : eMarkerType( MT_POINT )
        , nMarkerSize( 0.0 )
        {};

    virtual std::unique_ptr<Bundle> Clone() override { return std::make_unique<MarkerBundle>( *this ); } ;
};


class EdgeBundle : public Bundle
{
public:

    EdgeType            eEdgeType;
    double              nEdgeWidth;

    EdgeBundle()
        : eEdgeType(ET_NONE)
        , nEdgeWidth(0)
    {}
    virtual std::unique_ptr<Bundle> Clone() override { return std::make_unique<EdgeBundle>( *this ); }
};


class TextBundle : public Bundle
{
public:

    sal_uInt32          nTextFontIndex;
    TextPrecision       eTextPrecision;
    double              nCharacterExpansion;
    double              nCharacterSpacing;

    TextBundle()
        : nTextFontIndex( 0 )
        , eTextPrecision( TPR_UNDEFINED )
        , nCharacterExpansion( 0.0 )
        , nCharacterSpacing( 0.0 )
        {};

    virtual std::unique_ptr<Bundle> Clone() override { return std::make_unique<TextBundle>( *this ); } ;
};


class FillBundle : public Bundle
{
public:

    FillInteriorStyle   eFillInteriorStyle;
    long                nFillPatternIndex;
    long                nFillHatchIndex;

    FillBundle()
        : eFillInteriorStyle(FIS_HOLLOW)
        , nFillPatternIndex(0)
        , nFillHatchIndex(0)
    {}
    virtual std::unique_ptr<Bundle> Clone() override { return std::make_unique<FillBundle>( *this ); }
};


class FontEntry
{
public:
    std::unique_ptr<sal_Int8[]>
                        pFontName;
    CharSetType         eCharSetType;
    std::unique_ptr<sal_Int8[]>
                        pCharSetValue;
    sal_uInt32          nFontType;          // bit 0 = 1 -> Italic,
                                            // bit 1 = 1 -> Bold

                        FontEntry();
                        ~FontEntry();
};

class CGMFList
{
    sal_uInt32      nFontNameCount;
    sal_uInt32      nCharSetCount;
    ::std::vector< std::unique_ptr<FontEntry> >
                    aFontEntryList;
    sal_uInt32      nFontsAvailable;

    void            ImplDeleteList();

public:
                    CGMFList();
                    ~CGMFList();

    FontEntry*      GetFontEntry( sal_uInt32 );
    void            InsertName( sal_uInt8 const * pSource, sal_uInt32 nSize );
    void            InsertCharSet( CharSetType, sal_uInt8 const * pSource, sal_uInt32 nSize );
    CGMFList&       operator=( const CGMFList& rFontList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
