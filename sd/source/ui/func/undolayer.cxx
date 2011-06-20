/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sd.hxx"

#include "undolayer.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
