/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navcitem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:13:59 $
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

                    SCCOL nCol = aScAddress.Col()+1;
                    SCROW nRow = aScAddress.Row()+1;

//                  SCCOL nCol = (USHORT)pCellPosItem->GetValue().X()+1;
//                  SCROW nRow = (USHORT)pCellPosItem->GetValue().Y()+1;

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
                    SCTAB nTab = pTabItem->GetValue() - 1;

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



