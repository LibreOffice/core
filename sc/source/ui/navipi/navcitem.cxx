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



// INCLUDE ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

#include "navcitem.hxx"
#include "global.hxx"
#include "navipi.hxx"
#include "sc.hrc"       // -> Item-IDs

// STATIC DATA -----------------------------------------------------------


//------------------------------------------------------------------------

ScNavigatorControllerItem::ScNavigatorControllerItem( sal_uInt16          nIdP,
                                                      ScNavigatorDlg& rDlg,
                                                      SfxBindings&    rBindings )
    :   SfxControllerItem   ( nIdP, rBindings ),
        rNavigatorDlg       ( rDlg )
{
}

//------------------------------------------------------------------------

void __EXPORT ScNavigatorControllerItem::StateChanged( sal_uInt16 /* nSID */, SfxItemState /* eState */,
                                                          const SfxPoolItem* pItem )
{
    switch( GetId() )
    {
        case SID_CURRENTCELL:
            if ( pItem )
            {
//              const SfxPointItem* pCellPosItem = dynamic_cast< const SfxPointItem* >( pItem);
                const SfxStringItem* pCellPosItem = dynamic_cast< const SfxStringItem* >( pItem);

                DBG_ASSERT( pCellPosItem, "SfxStringItem expected!" );

                if ( pCellPosItem )
                {
                    String  aAddress( pCellPosItem->GetValue() );
                    ScAddress aScAddress;
                    aScAddress.Parse( aAddress );

                    SCCOL nCol = aScAddress.Col()+1;
                    SCROW nRow = aScAddress.Row()+1;

//                  SCCOL nCol = (sal_uInt16)pCellPosItem->GetValue().X()+1;
//                  SCROW nRow = (sal_uInt16)pCellPosItem->GetValue().Y()+1;

                    rNavigatorDlg.UpdateColumn( &nCol );
                    rNavigatorDlg.UpdateRow   ( &nRow );
                    rNavigatorDlg.CursorPosChanged();
                }
            }
            break;

        case SID_CURRENTTAB:
            if ( pItem )
            {
                const SfxUInt16Item* pTabItem = dynamic_cast< const SfxUInt16Item* >( pItem);

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



