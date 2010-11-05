/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/brwhead.hxx>
#include <svtools/brwbox.hxx>

#ifndef GCC
#endif

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
        // Handle-Column?
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
            // Hat sich die Position eigentlich veraendert
            // Handlecolumn beruecksichtigen
            sal_uInt16 nOldPos = _pBrowseBox->GetColumnPos(nId),
                nNewPos = GetItemPos( nId );

            if (!_pBrowseBox->GetColumnId(0))   // Handle
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




