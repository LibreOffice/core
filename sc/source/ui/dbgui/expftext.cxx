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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "expftext.hxx"
#include <vcl/help.hxx>

/*************************************************************************
#*  Member:     ScExpandedFixedText
#*------------------------------------------------------------------------
#*
#*  Klasse:     MD_Test
#*
#*  Funktion:   Konstruktor der Klasse SvxCtrDial
#*
#*  Input:      Parent- Window, Resource ID
#*
#*  Output:     ---
#*
#************************************************************************/

ScExpandedFixedText::ScExpandedFixedText( Window* pParent,
                                         const ResId& rResId) :
    FixedText( pParent, rResId )
{

}

/*************************************************************************
#*  Member:     RequestHelp
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScExpandedFixedText
#*
#*  Funktion:   Die Methode wird gerufen, um Hilfe fuer das Fenster
#*              anzuzeigen. Zeigt das Fenster einen gekuerzten Text
#*              an, so wird selbiger in voller Laenge angezeigt.
#*
#*  Input:      HelpEvent
#*
#*  Output:     ---
#*
#************************************************************************/

void ScExpandedFixedText::RequestHelp(const HelpEvent& rEvt)
{
    String  aTxtStr = GetText();
    long    nTxtWidth = GetTextWidth(aTxtStr);
    if ( ( rEvt.GetMode() & HELPMODE_QUICK ) == HELPMODE_QUICK &&
         nTxtWidth > GetSizePixel().Width())
    {
        Point aShowPoint = OutputToScreenPixel(Point(0,0));
        long nTxtHeight = GetTextHeight();

        Help::ShowQuickHelp( this,
            Rectangle( aShowPoint, Size(nTxtWidth,nTxtHeight) ), aTxtStr,
            QUICKHELP_TOP|QUICKHELP_LEFT );
    }
    else
    {
        FixedText::RequestHelp( rEvt );
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
