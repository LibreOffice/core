/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef CGM_BUNDLES_HXX_
#define CGM_BUNDLES_HXX_

#include <sal/types.h>
#include "cgmtypes.hxx"
#include <tools/list.hxx>
#include <vcl/salbtype.hxx>

// ---------------------------------------------------------------

class CGM;

class Bundle
{

    long                mnBundleIndex;
    sal_uInt32          mnColor;

public:
    void                SetColor( sal_uInt32 nColor ) ;
    sal_uInt32              GetColor() ;
    long                GetIndex() const { return mnBundleIndex; } ;
    void                SetIndex( long nBundleIndex ) { mnBundleIndex = nBundleIndex; } ;

                        Bundle() {};
    virtual Bundle*     Clone() { return new Bundle( *this ); };
            Bundle&     operator=( Bundle& rBundle );

    virtual             ~Bundle() {} ;
};

// ---------------------------------------------------------------

class LineBundle : public Bundle
{
public:

    LineType            eLineType;
    double              nLineWidth;

                        LineBundle() {};
    virtual Bundle*     Clone() { return new LineBundle( *this ); };
            LineBundle& operator=( LineBundle& rLineBundle );
    virtual             ~LineBundle() {};
};

// ---------------------------------------------------------------

class MarkerBundle : public Bundle
{
public:

    MarkerType          eMarkerType;
    double              nMarkerSize;

                        MarkerBundle() {};
    virtual Bundle*     Clone() { return new MarkerBundle( *this ); } ;
            MarkerBundle&   operator=( MarkerBundle& rMarkerBundle );
    virtual             ~MarkerBundle() {};
};

// ---------------------------------------------------------------

class EdgeBundle : public Bundle
{
public:

    EdgeType            eEdgeType;
    double              nEdgeWidth;

                        EdgeBundle() {};
    virtual Bundle*     Clone() { return new EdgeBundle( *this ); } ;
            EdgeBundle& operator=( EdgeBundle& rEdgeBundle );
    virtual             ~EdgeBundle() {};
};

// ---------------------------------------------------------------

class TextBundle : public Bundle
{
public:

    sal_uInt32              nTextFontIndex;
    TextPrecision       eTextPrecision;
    double              nCharacterExpansion;
    double              nCharacterSpacing;

                        TextBundle() {};
    virtual Bundle*     Clone() { return new TextBundle( *this ); } ;
            TextBundle& operator=( TextBundle& rTextBundle );
    virtual             ~TextBundle() {};
};

// ---------------------------------------------------------------

class FillBundle : public Bundle
{
public:

    FillInteriorStyle   eFillInteriorStyle;
    long                nFillPatternIndex;
    long                nFillHatchIndex;

                        FillBundle() {};
    virtual Bundle*     Clone() { return new FillBundle( *this ); } ;
            FillBundle& operator=( FillBundle& rFillBundle );
    virtual             ~FillBundle() {};
};


// ---------------------------------------------------------------

class FontEntry
{
public:
    sal_Int8*               pFontName;
    CharSetType         eCharSetType;
    sal_Int8*               pCharSetValue;
    sal_uInt32              nFontType;          // bit 0 = 1 -> Italic,
                                            // bit 1 = 1 -> Bold

                        FontEntry();
    FontEntry*          Clone() { return new FontEntry( *this ); } ;
                        ~FontEntry();
};

// ---------------------------------------------------------------

class CGMFList
{
    sal_uInt32              nFontNameCount;
    sal_uInt32              nCharSetCount;
    List                aFontEntryList;
    void                ImplDeleteList();
public:
    sal_uInt32              nFontsAvailable;
    FontEntry*          GetFontEntry( sal_uInt32 );
    void                InsertName( sal_uInt8* pSource, sal_uInt32 nSize );
    void                InsertCharSet( CharSetType, sal_uInt8* pSource, sal_uInt32 nSize );
                        CGMFList();
    CGMFList&           operator=( CGMFList& rFontList );
                        ~CGMFList();
};


#endif
