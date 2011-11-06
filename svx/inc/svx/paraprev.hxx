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

    // Einzuege
    long                nLeftMargin;
    long                nRightMargin;
    short               nFirstLineOfst;
    // Abstaende
    sal_uInt16              nUpper;
    sal_uInt16              nLower;
    // Ausrichtung
    SvxAdjust           eAdjust;
    // Letzte Zeile im Blocksatz
    SvxAdjust           eLastLine;
    // Zeilenabstand
    SvxPrevLineSpace    eLine;
    sal_uInt16              nLineVal;

    String              aText;
    Rectangle           Lines[9];

protected:
    virtual void Paint( const Rectangle& rRect );

    void DrawParagraph( sal_Bool bAll );

public:
    SvxParaPrevWindow( Window* pParent, const ResId& rId );

    void        SetFirstLineOfst( short nNew ) { nFirstLineOfst = nNew; }
    void        SetLeftMargin( long nNew )  { nLeftMargin = nNew; }
    void        SetRightMargin( long nNew ) { nRightMargin = nNew; }
    void        SetUpper( sal_uInt16 nNew )         { nUpper = nNew; }
    void        SetLower( sal_uInt16 nNew )         { nLower = nNew; }
    void        SetAdjust( SvxAdjust eNew )     { eAdjust = eNew; }
    void        SetLastLine( SvxAdjust eNew )   { eLastLine = eNew; }
    void        SetLineSpace( SvxPrevLineSpace eNew, sal_uInt16 nNew = 0 )
                    {   eLine = eNew; nLineVal = nNew; }
    void        SetText( const String& rStr )   { aText = rStr; }
    void        SetSize( Size aNew )            { aSize = aNew; }

    short       GetFirstLineOfst() const        { return nFirstLineOfst; }
    long        GetLeftMargin() const           { return nLeftMargin; }
    long        GetRightMargin() const          { return nRightMargin; }
    sal_uInt16      GetUpper() const                { return nUpper; }
    sal_uInt16      GetLower() const                { return nLower; }
    SvxAdjust   GetAdjust() const               { return eAdjust; }

    SvxPrevLineSpace    GetLineEnum() const     { return eLine; }
    sal_uInt16              GetLineValue() const    { return nLineVal; }
    String              GetText() const         { return aText; }
    Size                GetSize() const         { return aSize; }

    void        OutputSizeChanged();

    void        Draw( sal_Bool bAll )               { DrawParagraph( bAll ); }
};

#endif

