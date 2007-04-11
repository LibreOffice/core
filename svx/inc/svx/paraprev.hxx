/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paraprev.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:04:33 $
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
#ifndef _SVX_PARAPREV_HXX
#define _SVX_PARAPREV_HXX

// include ---------------------------------------------------------------

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
    USHORT              nUpper;
    USHORT              nLower;
    // Ausrichtung
    SvxAdjust           eAdjust;
    // Letzte Zeile im Blocksatz
    SvxAdjust           eLastLine;
    // Zeilenabstand
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

