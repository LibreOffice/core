/*************************************************************************
 *
 *  $RCSfile: brwhead.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-27 11:48:15 $
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

#include "brwhead.hxx"
#include "brwbox.hxx"

#pragma hdrstop

//===================================================================

BrowserHeader::BrowserHeader( BrowseBox* pParent, WinBits nWinBits )
:   HeaderBar( pParent, nWinBits ),
    _pBrowseBox( pParent )
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


