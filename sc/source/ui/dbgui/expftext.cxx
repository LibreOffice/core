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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "expftext.hxx"
#include <vcl/help.hxx>

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




