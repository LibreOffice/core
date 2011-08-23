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
#ifndef _SCROLL_HXX
#define _SCROLL_HXX

#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
namespace binfilter {

class SwRect;

class SwScrollbar: public ScrollBar
{
    Size    aDocSz;
    BOOL 	bHori		:1;		// Horizontal = TRUE, sonst Vertikal
    BOOL	bAuto		:1;		// fuer Scrollingmode
    BOOL	bThumbEnabled:1;
    BOOL	bVisible	:1;		// Show/Hide sollen nur noch dieses Flag setzen
    BOOL	bSizeSet	:1;		// wurde die Groesse bereits gesetzt?

    void	AutoShow();
public:
    void	Show( BOOL bVisible = TRUE );
    void	Hide() { Show( FALSE ); }
    BOOL	IsVisible(BOOL bReal = FALSE) const { return bReal ? ScrollBar::IsVisible() : bVisible; }
        // Aenderung der Dokumentgroesse
        // Aenderung des sichtbaren Bereiches
        // was fuer einer ist es denn ??
    int 	IsHoriScroll() const { return bHori; }

    void 	SetAuto(BOOL bSet);
    BOOL 	IsAuto() { return bAuto;}

    //Bewegungen des Thumbs abklemmen

    SwScrollbar(Window *pParent, int bHori = TRUE );
    ~SwScrollbar();
};



} //namespace binfilter
#endif
