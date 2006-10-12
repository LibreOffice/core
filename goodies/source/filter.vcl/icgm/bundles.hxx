/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bundles.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:35:22 $
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
