/*************************************************************************
 *
 *  $RCSfile: condedit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

// include ---------------------------------------------------------------

#pragma hdrstop

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SV_EVENT_HXX //autogen
#include <vcl/event.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#ifndef _CONDEDIT_HXX
#include <condedit.hxx>
#endif

#define DB_DD_DELIM 0x0b

// STATIC DATA -----------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ConditionEdit::ConditionEdit( Window* pParent, const ResId& rResId ) :
        Edit( pParent, rResId ),
        bBrackets(TRUE)
{
    EnableDrop();
}

/*--------------------------------------------------------------------
    Beschreibung: Drop moeglich, bzw Format bekannt?
 --------------------------------------------------------------------*/

BOOL ConditionEdit::QueryDrop( DropEvent& rDEvt )
{
    rDEvt.SetAction(DROP_COPY);

    if (DragServer::HasFormat(0, SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE ))
    {
        BOOL bString = FALSE;

        for (USHORT nItem = 0; nItem < DragServer::GetItemCount(); nItem++)
        {
            if (DragServer::HasFormat(nItem, FORMAT_STRING))
            {
                bString = TRUE;
                break;
            }
        }
        if (!bString)   // String reinpacken, um DD-Cursor zu erhalten
        {
            if (DragServer::HasFormat(0, SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE))
            {
                USHORT nLen = (USHORT)DragServer::GetDataLen(0, SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE) - 1;
                DBG_ASSERT(!(nLen%2), "not a UniString")
                String sTxt;
                sal_Unicode *pTxt = sTxt.AllocBuffer(nLen / 2);

                DragServer::PasteData(0, pTxt, nLen, SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE);

                String sDBName;
                if (bBrackets)
                    sDBName += '[';
                sDBName += sTxt.GetToken(0, DB_DD_DELIM);
                sDBName += '.';
                sDBName += sTxt.GetToken(1, DB_DD_DELIM);
                sDBName += '.';
                sDBName += sTxt.GetToken(3, DB_DD_DELIM);   // ColumnName
                if (bBrackets)
                    sDBName += ']';
                DragServer::CopyString(sDBName);
            }
        }
    }

    return Edit::QueryDrop( rDEvt );
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.7  2000/09/18 16:06:17  willem.vandorp
      OpenOffice header added.

      Revision 1.6  2000/04/18 15:14:08  os
      UNICODE

      Revision 1.5  2000/02/11 15:00:29  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.4  1998/11/17 21:18:48  JP
      Task #59398#: ClipboardFormatId Umstellungen


      Rev 1.3   17 Nov 1998 22:18:48   JP
   Task #59398#: ClipboardFormatId Umstellungen

      Rev 1.2   18 May 1998 14:53:24   OM
   #50001 Keine eckigen Klammern in DB-Condition-Fields

      Rev 1.1   12 Mar 1998 16:32:42   OM
   Datenbanknamen in eckigen Klammern bei Drop

      Rev 1.0   19 Nov 1997 16:21:42   OM
   Initial revision.

*************************************************************************/


