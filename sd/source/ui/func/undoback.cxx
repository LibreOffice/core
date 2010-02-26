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

#include "undoback.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "strings.hrc"

// ---------------------------
// - BackgroundObjUndoAction -
// ---------------------------

TYPEINIT1( SdBackgroundObjUndoAction, SdUndoAction );

// -----------------------------------------------------------------------------

SdBackgroundObjUndoAction::SdBackgroundObjUndoAction( SdDrawDocument& rDoc, SdPage& rPage, const SdrObject* pBackgroundObj ) :
    SdUndoAction( &rDoc ),
    mrPage( rPage ),
    mpBackgroundObj( pBackgroundObj ? pBackgroundObj->Clone() : NULL )
{
    String aString( SdResId( STR_UNDO_CHANGE_PAGEFORMAT ) );
    SetComment( aString );
}

// -----------------------------------------------------------------------------

SdBackgroundObjUndoAction::~SdBackgroundObjUndoAction()
{
    SdrObject::Free( mpBackgroundObj );
}

// -----------------------------------------------------------------------------

void SdBackgroundObjUndoAction::ImplRestoreBackgroundObj()
{
    SdrObject* pOldObj = mrPage.GetBackgroundObj();

    if( pOldObj )
        pOldObj = pOldObj->Clone();

    mrPage.SetBackgroundObj( mpBackgroundObj );
    mpBackgroundObj = pOldObj;

    // #110094#-15
    // tell the page that it's visualization has changed
    mrPage.ActionChanged();
}

// -----------------------------------------------------------------------------

void SdBackgroundObjUndoAction::Undo()
{
    ImplRestoreBackgroundObj();
}

// -----------------------------------------------------------------------------

void SdBackgroundObjUndoAction::Redo()
{
    ImplRestoreBackgroundObj();
}

// -----------------------------------------------------------------------------

SdUndoAction* SdBackgroundObjUndoAction::Clone() const
{
    return new SdBackgroundObjUndoAction( *mpDoc, mrPage, mpBackgroundObj );
}
