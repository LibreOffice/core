/*************************************************************************
 *
 *  $RCSfile: bundles.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    virtual Bundle&     operator=( Bundle& rBundle );

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
    virtual LineBundle& operator=( LineBundle& rLineBundle );
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
    virtual MarkerBundle&   operator=( MarkerBundle& rMarkerBundle );
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
    virtual EdgeBundle& operator=( EdgeBundle& rEdgeBundle );
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
    virtual TextBundle& operator=( TextBundle& rTextBundle );
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
    virtual FillBundle& operator=( FillBundle& rFillBundle );
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

class FontList
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
                        FontList();
    FontList&           operator=( FontList& rFontList );
                        ~FontList();
};


#endif
