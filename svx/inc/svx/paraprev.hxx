/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVX_PARAPREV_HXX
#define _SVX_PARAPREV_HXX

// include ---------------------------------------------------------------

#include <vcl/window.hxx>
#include <editeng/svxenum.hxx>
#include "svx/svxdllapi.h"

// enum ------------------------------------------------------------------

enum SvxPrevLineSpace
{
    SVX_PREV_LINESPACE_1 = 0,
    SVX_PREV_LINESPACE_15,
    SVX_PREV_LINESPACE_2,
    SVX_PREV_LINESPACE_PROP,
    SVX_PREV_LINESPACE_MIN,
    SVX_PREV_LINESPACE_DURCH
};

// class SvxParaPrevWindow -----------------------------------------------

class SVX_DLLPUBLIC SvxParaPrevWindow : public Window
{
    using Window::Draw;
private:
    Size                aWinSize;
    Size                aSize;

    // indentation
    long                nLeftMargin;
    long                nRightMargin;
    short               nFirstLineOfst;
    // distances
    USHORT              nUpper;
    USHORT              nLower;
    // adjustment
    SvxAdjust           eAdjust;
    // last line in justification
    SvxAdjust           eLastLine;
    // line distance
    SvxPrevLineSpace    eLine;
    USHORT              nLineVal;

    String              aText;
    Rectangle           Lines[9];

protected:
    virtual void Paint( const Rectangle& rRect );

    void DrawParagraph( BOOL bAll );

public:
    SvxParaPrevWindow( Window* pParent, const ResId& rId );

    void        SetFirstLineOfst( short nNew ) { nFirstLineOfst = nNew; }
    void        SetLeftMargin( long nNew )  { nLeftMargin = nNew; }
    void        SetRightMargin( long nNew ) { nRightMargin = nNew; }
    void        SetUpper( USHORT nNew )         { nUpper = nNew; }
    void        SetLower( USHORT nNew )         { nLower = nNew; }
    void        SetAdjust( SvxAdjust eNew )     { eAdjust = eNew; }
    void        SetLastLine( SvxAdjust eNew )   { eLastLine = eNew; }
    void        SetLineSpace( SvxPrevLineSpace eNew, USHORT nNew = 0 )
                    {   eLine = eNew; nLineVal = nNew; }
    void        SetText( const String& rStr )   { aText = rStr; }
    void        SetSize( Size aNew )            { aSize = aNew; }

    short       GetFirstLineOfst() const        { return nFirstLineOfst; }
    long        GetLeftMargin() const           { return nLeftMargin; }
    long        GetRightMargin() const          { return nRightMargin; }
    USHORT      GetUpper() const                { return nUpper; }
    USHORT      GetLower() const                { return nLower; }
    SvxAdjust   GetAdjust() const               { return eAdjust; }

    SvxPrevLineSpace    GetLineEnum() const     { return eLine; }
    USHORT              GetLineValue() const    { return nLineVal; }
    String              GetText() const         { return aText; }
    Size                GetSize() const         { return aSize; }

    void        OutputSizeChanged();

    void        Draw( BOOL bAll )               { DrawParagraph( bAll ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
