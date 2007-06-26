/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undolayer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 13:40:16 $
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
#include "precompiled_sd.hxx"

#include "undolayer.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "strings.hrc"
#include "sdresid.hxx"

TYPEINIT1(SdLayerModifyUndoAction, SdUndoAction);

SdLayerModifyUndoAction::SdLayerModifyUndoAction(
    SdDrawDocument* _pDoc, SdrLayer* pLayer,
    const String& rOldLayerName, const String& rOldLayerTitle, const String& rOldLayerDesc, bool bOldIsVisible, bool bOldIsLocked, bool bOldIsPrintable,
    const String& rNewLayerName, const String& rNewLayerTitle, const String& rNewLayerDesc, bool bNewIsVisible, bool bNewIsLocked, bool bNewIsPrintable )
:   SdUndoAction( _pDoc ),
    mpLayer( pLayer ),
    maOldLayerName( rOldLayerName ),
    maOldLayerTitle( rOldLayerTitle ),
    maOldLayerDesc( rOldLayerDesc ),
    mbOldIsVisible( bOldIsVisible ),
    mbOldIsLocked( bOldIsLocked ),
    mbOldIsPrintable( bOldIsPrintable ),
    maNewLayerName( rNewLayerName ),
    maNewLayerTitle( rNewLayerTitle ),
    maNewLayerDesc( rNewLayerDesc ),
    mbNewIsVisible( bNewIsVisible ),
    mbNewIsLocked( bNewIsLocked ),
    mbNewIsPrintable( bNewIsPrintable )
{
    String aString(SdResId(STR_MODIFYLAYER));
    SetComment(aString);
}

void SdLayerModifyUndoAction::Undo()
{
    ::sd::DrawDocShell* mpDocSh = mpDoc->GetDocSh();
    if( mpDocSh )
    {
        ::sd::DrawViewShell* pDrViewSh =
              PTR_CAST(::sd::DrawViewShell, mpDocSh->GetViewShell() );
        if( pDrViewSh )
        {
            pDrViewSh->ModifyLayer( mpLayer, maOldLayerName, maOldLayerTitle, maOldLayerDesc, mbOldIsVisible, mbOldIsLocked, mbOldIsPrintable );
        }
    }
}

void SdLayerModifyUndoAction::Redo()
{
    ::sd::DrawDocShell* mpDocSh = mpDoc->GetDocSh();
    if( mpDocSh )
    {
        ::sd::DrawViewShell* pDrViewSh =
              PTR_CAST(::sd::DrawViewShell, mpDocSh->GetViewShell() );
        if( pDrViewSh )
        {
            pDrViewSh->ModifyLayer( mpLayer, maNewLayerName, maNewLayerTitle, maNewLayerDesc, mbNewIsVisible, mbNewIsLocked, mbNewIsPrintable );
        }
    }
}
