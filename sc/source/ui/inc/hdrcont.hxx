/*************************************************************************
 *
 *  $RCSfile: hdrcont.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:34:09 $
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

#ifndef SC_HDRCONT_HXX
#define SC_HDRCONT_HXX

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif
#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

// ---------------------------------------------------------------------------


#define HDR_HORIZONTAL      0
#define HDR_VERTICAL        1

#define HDR_SIZE_OPTIMUM    0xFFFF


                                    // Groesse des Sliders
#define HDR_SLIDERSIZE      2

class ScHeaderControl : public Window
{
private:
    SelectionEngine*    pSelEngine;
    Font                aNormFont;
    Font                aBoldFont;
    BOOL                bBoldSet;

    USHORT          nFlags;
    BOOL            bVertical;              // Vertikal = Zeilenheader

    long            nWidth;
    long            nSmallWidth;
    long            nBigWidth;

    SCCOLROW        nSize;

    SCCOLROW        nMarkStart;
    SCCOLROW        nMarkEnd;
    BOOL            bMarkRange;

    BOOL            bDragging;              // Groessen aendern
    SCCOLROW        nDragNo;
    long            nDragStart;
    long            nDragPos;
    BOOL            bDragMoved;

    BOOL            bIgnoreMove;

    long            GetScrPos( SCCOLROW nEntryNo );
    SCCOLROW        GetMousePos( const MouseEvent& rMEvt, BOOL& rBorder );

    void            ShowDragHelp();

    void            DoPaint( SCCOLROW nStart, SCCOLROW nEnd );

protected:
                    //  von Window ueberladen

    virtual void    Paint( const Rectangle& rRect );

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );

    virtual void    RequestHelp( const HelpEvent& rHEvt );

                    //  neue Methoden

    virtual SCCOLROW    GetPos() = 0;                               // aktuelle Position (Scrolling)
    virtual USHORT  GetEntrySize( SCCOLROW nEntryNo ) = 0;      // Breite / Hoehe (Pixel)
    virtual String  GetEntryText( SCCOLROW nEntryNo ) = 0;

    virtual SCCOLROW GetHiddenCount( SCCOLROW nEntryNo );
    virtual BOOL    IsLayoutRTL();
    virtual BOOL    IsMirrored();

    virtual void    SetEntrySize( SCCOLROW nPos, USHORT nNewWidth ) = 0;
    virtual void    HideEntries( SCCOLROW nStart, SCCOLROW nEnd ) = 0;

    virtual void    SetMarking( BOOL bSet );
    virtual void    SelectWindow();
    virtual BOOL    IsDisabled();
    virtual BOOL    ResizeAllowed();
    virtual String  GetDragHelp( long nVal );

    virtual void    DrawInvert( long nDragPos );
    virtual void    Command( const CommandEvent& rCEvt );

public:
            ScHeaderControl( Window* pParent, SelectionEngine* pSelectionEngine,
                                SCCOLROW nNewSize, USHORT nNewFlags );
            ~ScHeaderControl();

    void    SetIgnoreMove(BOOL bSet)            { bIgnoreMove = bSet; }

    void    StopMarking();

    void    SetMark( BOOL bNewSet, SCCOLROW nNewStart, SCCOLROW nNewEnd );

    long    GetWidth() const                    { return nWidth; }
    long    GetSmallWidth() const               { return nSmallWidth; }
    long    GetBigWidth() const                 { return nBigWidth; }
    void    SetWidth( long nNew );
};



#endif

