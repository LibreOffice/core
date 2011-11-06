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




