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


#include "undolayer.hxx"

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "strings.hrc"
#include "sdresid.hxx"

TYPEINIT1(SdLayerModifyUndoAction, SdUndoAction);

SdLayerModifyUndoAction::SdLayerModifyUndoAction(
    SdDrawDocument* _pDoc, SdrLayer* pLayer,
    const OUString& rOldLayerName, const OUString& rOldLayerTitle, const OUString& rOldLayerDesc, bool bOldIsVisible, bool bOldIsLocked, bool bOldIsPrintable,
    const OUString& rNewLayerName, const OUString& rNewLayerTitle, const OUString& rNewLayerDesc, bool bNewIsVisible, bool bNewIsLocked, bool bNewIsPrintable )
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
    OUString aString(SdResId(STR_MODIFYLAYER));
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
