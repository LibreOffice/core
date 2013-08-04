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


#include <brwhead.hxx>
#include <brwbox.hxx>

//===================================================================

BrowserHeader::BrowserHeader( BrowseBox* pParent, WinBits nWinBits )
 :HeaderBar( pParent, nWinBits )
 ,_pBrowseBox( pParent )
{
    long nHeight = pParent->IsZoom() ? pParent->CalcZoom(pParent->GetTitleHeight()) : pParent->GetTitleHeight();

    SetPosSizePixel( Point( 0, 0),
                     Size( pParent->GetOutputSizePixel().Width(),
                           nHeight ) );
    Show();
}

//-------------------------------------------------------------------

void BrowserHeader::Command( const CommandEvent& rCEvt )
{
    if ( !GetCurItemId() && COMMAND_CONTEXTMENU == rCEvt.GetCommand() )
    {
        Point aPos( rCEvt.GetMousePosPixel() );
        if ( _pBrowseBox->IsFrozen(0) )
            aPos.X() += _pBrowseBox->GetColumnWidth(0);
        _pBrowseBox->GetDataWindow().Command( CommandEvent(
                Point( aPos.X(), aPos.Y() - GetSizePixel().Height() ),
                COMMAND_CONTEXTMENU, rCEvt.IsMouseEvent() ) );
    }
}

//-------------------------------------------------------------------

void BrowserHeader::Select()
{
    HeaderBar::Select();
}

//-------------------------------------------------------------------

void BrowserHeader::EndDrag()
{
    // call before other actions, it looks more nice in most cases
    HeaderBar::EndDrag();
    Update();

    // not aborted?
    sal_uInt16 nId = GetCurItemId();
    if ( nId )
    {
        // handle column?
        if ( nId == USHRT_MAX-1 )
            nId = 0;

        if ( !IsItemMode() )
        {
            // column resize
            _pBrowseBox->SetColumnWidth( nId, GetItemSize( nId ) );
            _pBrowseBox->ColumnResized( nId );
            SetItemSize( nId, _pBrowseBox->GetColumnWidth( nId ) );
        }
        else
        {
            // column drag
            // did the position actually change?
            // take the handle column into account
            sal_uInt16 nOldPos = _pBrowseBox->GetColumnPos(nId),
                nNewPos = GetItemPos( nId );

            if (_pBrowseBox->GetColumnId(0) == BrowseBox::HandleColumnId)
                nNewPos++;

            if (nOldPos != nNewPos)
            {
                _pBrowseBox->SetColumnPos( nId, nNewPos );
                _pBrowseBox->ColumnMoved( nId );
            }
        }
    }
}
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
