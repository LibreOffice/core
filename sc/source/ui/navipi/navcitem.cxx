/*************************************************************************
 *
 *  $RCSfile: navcitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>

#include "navcitem.hxx"
#include "global.hxx"
#include "navipi.hxx"
#include "sc.hrc"       // -> Item-IDs

// STATIC DATA -----------------------------------------------------------


//------------------------------------------------------------------------

ScNavigatorControllerItem::ScNavigatorControllerItem( USHORT          nId,
                                                      ScNavigatorDlg& rDlg,
                                                      SfxBindings&    rBindings )
    :   SfxControllerItem   ( nId, rBindings ),
        rNavigatorDlg       ( rDlg )
{
}

//------------------------------------------------------------------------

void __EXPORT ScNavigatorControllerItem::StateChanged( USHORT nSID, SfxItemState eState,
                                                          const SfxPoolItem* pItem )
{
    switch( GetId() )
    {
        case SID_CURRENTCELL:
            if ( pItem )
            {
//              const SfxPointItem* pCellPosItem = PTR_CAST(SfxPointItem, pItem);
                const SfxStringItem* pCellPosItem = PTR_CAST(SfxStringItem, pItem);

                DBG_ASSERT( pCellPosItem, "SfxStringItem expected!" );

                if ( pCellPosItem )
                {
                    String  aAddress( pCellPosItem->GetValue() );
                    ScAddress aScAddress;
                    aScAddress.Parse( aAddress );

                    USHORT nCol = aScAddress.Col()+1;
                    USHORT nRow = aScAddress.Row()+1;

//                  USHORT nCol = (USHORT)pCellPosItem->GetValue().X()+1;
//                  USHORT nRow = (USHORT)pCellPosItem->GetValue().Y()+1;

                    rNavigatorDlg.UpdateColumn( &nCol );
                    rNavigatorDlg.UpdateRow   ( &nRow );
                    rNavigatorDlg.CursorPosChanged();
                }
            }
            break;

        case SID_CURRENTTAB:
            if ( pItem )
            {
                const SfxUInt16Item* pTabItem = PTR_CAST(SfxUInt16Item, pItem);

                DBG_ASSERT( pTabItem, "SfxUInt16Item expected!" );

                //  Tabelle fuer Basic ist 1-basiert
                if ( pTabItem && pTabItem->GetValue() )
                {
                    USHORT nTab = pTabItem->GetValue() - 1;

                    rNavigatorDlg.UpdateTable( &nTab );
                    rNavigatorDlg.UpdateColumn();
                    rNavigatorDlg.UpdateRow();
                    rNavigatorDlg.CursorPosChanged();
                }
            }
            break;

        case SID_CURRENTDOC:
            //
            //  gar nix mehr, wird ueber SFX_HINT_DOCCHANGED erledigt
            //
            break;


        case SID_SELECT_SCENARIO:
            rNavigatorDlg.aWndScenarios.NotifyState( pItem );
            break;

        default:
            break;
    }
}



