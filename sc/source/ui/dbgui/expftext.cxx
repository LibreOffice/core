/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expftext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:22:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "expftext.hxx"
#include <vcl/help.hxx>

/*************************************************************************
#*  Member:     ScExpandedFixedText                         Datum:18.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScExpandedFixedText
#*
#*  Funktion:   Konstruktor der Klasse ScExpandedFixedText
#*
#*  Input:      Parent- Window, Fenster-Attribute
#*
#*  Output:     ---
#*
#************************************************************************/

ScExpandedFixedText::ScExpandedFixedText( Window* pParent,
                                         WinBits nWinStyle) :

    FixedText( pParent, nWinStyle )
{

}

/*************************************************************************
#*  Member:     ScExpandedFixedText                         Datum:18.09.97
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
#*  Member:     RequestHelp                                 Datum:18.09.97
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




