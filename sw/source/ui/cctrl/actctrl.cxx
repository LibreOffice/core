/*************************************************************************
 *
 *  $RCSfile: actctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop


#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#include "actctrl.hxx"



void NumEditAction::Action()
{
    aActionLink.Call( this );
}


long NumEditAction::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        const USHORT nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_RETURN &&
                !nModifier)
        {
            Action();
            nHandled = 1;
        }

    }
    if(!nHandled)
        NumericField::Notify( rNEvt );
    return nHandled;
}

/*------------------------------------------------------------------------
 Beschreibung:  KeyInput fuer ShortName - Edits ohne Spaces
------------------------------------------------------------------------*/


void NoSpaceEdit::KeyInput(const KeyEvent& rEvt)
{
    BOOL bCallParent = TRUE;
    if(rEvt.GetCharCode())
    {
        String sKey = rEvt.GetCharCode();
        if( STRING_NOTFOUND != sForbiddenChars.Search(sKey))
            bCallParent = FALSE;
    }
    if(bCallParent)
        Edit::KeyInput(rEvt);
}
/* -----------------------------11.02.00 15:28--------------------------------

 ---------------------------------------------------------------------------*/
void NoSpaceEdit::Modify()
{
    Selection aSel = GetSelection();
    String sTemp = GetText();
    for(USHORT i = 0; i < sForbiddenChars.Len(); i++)
    {
        sTemp.EraseAllChars( sForbiddenChars.GetChar(i) );
    }
    USHORT nDiff = GetText().Len() - sTemp.Len();
    if(nDiff)
    {
        aSel.setMin(aSel.getMin() - nDiff);
        aSel.setMax(aSel.getMin());
        SetText(sTemp);
        SetSelection(aSel);
    }
    if(GetModifyHdl().IsSet())
        GetModifyHdl().Call(this);
}

/*************************************************************************

    $Log: not supported by cvs2svn $
    Revision 1.67  2000/09/18 16:05:12  willem.vandorp
    OpenOffice header added.

    Revision 1.66  2000/07/10 09:25:33  os
    #76692# check CharCode first

    Revision 1.65  2000/02/21 07:16:20  os
    #73223# Call ModifyHdl in ::Modify

    Revision 1.64  2000/02/11 14:46:03  os
    #71914# Check content in ::Modify

    Revision 1.63  2000/02/01 10:08:25  os
    #71914# forbidden chars in table names: <>. and space

    Revision 1.62  1999/06/10 07:45:50  OS
    NoSpaceEdit: illegal characters can be set


      Rev 1.61   10 Jun 1999 09:45:50   OS
   NoSpaceEdit: illegal characters can be set

      Rev 1.60   05 Nov 1998 10:43:42   OS
   #58990# Notify sollte richtig auf KEYINPUT reagieren

      Rev 1.59   21 Apr 1998 08:43:20   OS
   NoSpaceEdit, TableNameEdit hierher verschoben

      Rev 1.58   30 Mar 1998 18:46:04   TJ
   chg: header wg. internal Comperror

      Rev 1.57   12 Nov 1997 17:29:42   MBA
   SP3:Notify,EVENT_KEYUP

      Rev 1.56   03 Nov 1997 13:09:44   MA
   precomp entfernt

      Rev 1.55   17 Oct 1997 08:22:12   OS
   Notify anstelle von KeyInput ueberladen #44732#

      Rev 1.54   13 Feb 1997 16:36:26   MA
   NameEdit entfernt

      Rev 1.53   28 Aug 1996 08:23:40   OS
   includes

      Rev 1.52   24 Nov 1995 16:57:36   OM
   PCH->PRECOMPILED

      Rev 1.51   08 Nov 1995 10:18:32   OS
   switch => if, keine member -> kein DTOR

      Rev 1.50   12 Oct 1995 15:27:26   MA
   Klasse EditAction entfernt

      Rev 1.49   21 Mar 1995 19:00:08   OS
   EditAction wieder drin

      Rev 1.48   20 Mar 1995 19:05:22   OS
   unbenutzte Funktionen entfernt

      Rev 1.47   15 Mar 1995 11:01:46   MS
   n.b. Ctors entfernt

      Rev 1.46   15 Mar 1995 10:36:46   MS
   ActionControlls reduziert

      Rev 1.45   04 Mar 1995 23:13:30   ER
   Help::EnableBalloonMode() hammernichmehr

      Rev 1.44   07 Feb 1995 19:23:44   ER
   #elif UNX  wird vielleicht wahr werden oder auch nicht..

      Rev 1.43   09 Jan 1995 16:17:44   ER
    del: ctype h

      Rev 1.42   25 Oct 1994 14:55:18   ER
   add: PCH

      Rev 1.41   22 Aug 1994 08:17:32   MS
   SPoints entfernt

*************************************************************************/


