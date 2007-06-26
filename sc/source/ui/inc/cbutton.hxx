/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cbutton.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 11:50:17 $
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

//------------------------------------------------------------------

#ifndef SC_CBUTTON_HXX
#define SC_CBUTTON_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

class OutputDevice;


//==================================================================

class ScDDComboBoxButton
{
public:
            ScDDComboBoxButton( OutputDevice* pOutputDevice );
            ~ScDDComboBoxButton();

    void    SetOutputDevice( OutputDevice* pOutputDevice );

    void    Draw( const Point&  rAt,
                  const Size&   rSize,
                  BOOL          bState,
                  BOOL          bBtnIn = FALSE );

    void    Draw( const Point&  rAt,
                  BOOL          bState,
                  BOOL          bBtnIn = FALSE )
                { Draw( rAt, aBtnSize, bState, bBtnIn ); }

    void    Draw( BOOL          bState,
                  BOOL          bBtnIn = FALSE )
                { Draw( aBtnPos, aBtnSize, bState, bBtnIn ); }

    void    SetOptSizePixel();

    void    SetPosPixel( const Point& rNewPos )  { aBtnPos = rNewPos; }
    Point   GetPosPixel() const                  { return aBtnPos; }

    void    SetSizePixel( const Size& rNewSize ) { aBtnSize = rNewSize; }
    Size    GetSizePixel() const                 { return aBtnSize; }

private:
    void    ImpDrawArrow( const Rectangle&  rRect,
                          BOOL              bState );

protected:
    OutputDevice* pOut;
    Point   aBtnPos;
    Size    aBtnSize;
};


#endif // SC_CBUTTON_HXX


