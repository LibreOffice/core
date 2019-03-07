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

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <osl/diagnose.h>

#include <navcitem.hxx>
#include <navipi.hxx>
#include <sc.hrc>

ScNavigatorControllerItem::ScNavigatorControllerItem( sal_uInt16          nIdP,
                                                      ScNavigatorDlg& rDlg,
                                                      SfxBindings&    rBindings )
    :   SfxControllerItem   ( nIdP, rBindings ),
        rNavigatorDlg       ( rDlg )
{
}

void ScNavigatorControllerItem::StateChanged( sal_uInt16 /* nSID */, SfxItemState /* eState */,
                                                          const SfxPoolItem* pItem )
{
    switch( GetId() )
    {
        case SID_CURRENTCELL:
            if ( pItem )
            {
                const SfxStringItem* pCellPosItem = dynamic_cast<const SfxStringItem*>( pItem );

                OSL_ENSURE( pCellPosItem, "SfxStringItem expected!" );

                if ( pCellPosItem )
                {
                    const OUString&  aAddress( pCellPosItem->GetValue() );
                    ScAddress aScAddress;
                    aScAddress.Parse( aAddress );

                    SCCOL nCol = aScAddress.Col()+1;
                    SCROW nRow = aScAddress.Row()+1;

                    rNavigatorDlg.UpdateColumn( &nCol );
                    rNavigatorDlg.UpdateRow   ( &nRow );
                }
            }
            break;

        case SID_CURRENTTAB:
            if ( pItem )
            {
                const SfxUInt16Item* pTabItem = dynamic_cast< const SfxUInt16Item *>( pItem );

                OSL_ENSURE( pTabItem, "SfxUInt16Item expected!" );

                //  table for Basic is 1-based
                if ( pTabItem && pTabItem->GetValue() )
                {
                    SCTAB nTab = pTabItem->GetValue() - 1;

                    rNavigatorDlg.UpdateTable( &nTab );
                    rNavigatorDlg.UpdateColumn();
                    rNavigatorDlg.UpdateRow();
                }
            }
            break;

        case SID_CURRENTDOC:

            //  nothing is done via SfxHintId::DocChanged

            break;

        case SID_SELECT_SCENARIO:
            rNavigatorDlg.aWndScenarios->NotifyState( pItem );
            break;

        default:
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
