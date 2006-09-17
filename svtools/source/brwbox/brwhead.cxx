/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brwhead.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:18:39 $
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
#include "precompiled_svtools.hxx"

#include "brwhead.hxx"
#include "brwbox.hxx"

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
    USHORT nId = GetCurItemId();
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
            USHORT nOldPos = _pBrowseBox->GetColumnPos(nId),
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




