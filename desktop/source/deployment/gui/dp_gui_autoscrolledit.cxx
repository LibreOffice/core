/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_autoscrolledit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:21:53 $
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
#include "precompiled_desktop.hxx"


#include "svtools/svmedit2.hxx"
#include "svtools/lstner.hxx"
#include "svtools/xtextedt.hxx"
#include "vcl/scrbar.hxx"

#include "dp_gui_autoscrolledit.hxx"


namespace dp_gui {


AutoScrollEdit::AutoScrollEdit( Window* pParent, const ResId& rResId )
    : ExtMultiLineEdit( pParent, rResId )
{
    ScrollBar*  pScroll = GetVScrollBar();
    if (pScroll)
        pScroll->Hide();
//    SetLeftMargin( 0 );
    StartListening( *GetTextEngine() );
}

AutoScrollEdit::~AutoScrollEdit()
{
    EndListeningAll();
}

void AutoScrollEdit::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE(TextHint) ) )
    {
        ULONG   nId = ((const TextHint&)rHint).GetId();
        if ( nId == TEXT_HINT_VIEWSCROLLED )
        {
            ScrollBar*  pScroll = GetVScrollBar();
            if ( pScroll )
                pScroll->Show();
        }
    }
}


} // namespace dp_gui

