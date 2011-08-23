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

#ifndef SC_HDRCONT_HXX
#define SC_HDRCONT_HXX

#include <vcl/window.hxx>
#include <vcl/seleng.hxx>
namespace binfilter {

// ---------------------------------------------------------------------------


#define HDR_HORIZONTAL		0
#define HDR_VERTICAL		1

#define HDR_SIZE_OPTIMUM	0xFFFF


                                    // Groesse des Sliders
#define HDR_SLIDERSIZE		2

class ScHeaderControl : public Window
{
private:
    SelectionEngine*	pSelEngine;
    Font				aNormFont;
    Font				aBoldFont;
    BOOL				bBoldSet;

    USHORT			nFlags;
    BOOL			bVertical;				// Vertikal = Zeilenheader

    long			nWidth;
    long			nSmallWidth;
    long			nBigWidth;

    USHORT			nSize;

    USHORT			nMarkStart;
    USHORT			nMarkEnd;
    BOOL			bMarkRange;

    BOOL			bDragging;				// Groessen aendern
    USHORT			nDragNo;
    long			nDragStart;
    long			nDragPos;
    BOOL			bDragMoved;

    BOOL			bIgnoreMove;

    long			GetScrPos( USHORT nEntryNo );


    void			DoPaint( USHORT nStart, USHORT nEnd );

protected:
                    //	von Window ueberladen




                    //	neue Methoden

    virtual USHORT	GetPos() = 0;								// aktuelle Position (Scrolling)
    virtual USHORT	GetEntrySize( USHORT nEntryNo ) = 0;		// Breite / Hoehe (Pixel)
    virtual String	GetEntryText( USHORT nEntryNo ) = 0;


    virtual void	SetEntrySize( USHORT nPos, USHORT nNewWidth ) = 0;
    virtual void	HideEntries( USHORT nStart, USHORT nEnd ) = 0;

    virtual void	SetMarking( BOOL bSet );


public:
            ScHeaderControl( Window* pParent, SelectionEngine* pSelectionEngine,
                                USHORT nNewSize, USHORT nNewFlags );
            ~ScHeaderControl();

    void	SetIgnoreMove(BOOL bSet)			{ bIgnoreMove = bSet; }

     void	StopMarking();

    void	SetMark( BOOL bNewSet, USHORT nNewStart, USHORT nNewEnd );

    long	GetWidth() const					{ return nWidth; }
    long	GetSmallWidth() const				{ return nSmallWidth; }
    long	GetBigWidth() const					{ return nBigWidth; }
    void	SetWidth( long nNew );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
