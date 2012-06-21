/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
