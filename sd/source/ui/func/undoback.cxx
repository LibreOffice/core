/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoback.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:59:37 $
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
    SdUndoAction( pDoc ),
    mrPage( rPage ),
    mpBackgroundObj( pBackgroundObj ? pBackgroundObj->Clone() : NULL )
{
    String aString( SdResId( STR_UNDO_CHANGE_PAGEFORMAT ) );
    SetComment( aString );
}

// -----------------------------------------------------------------------------

SdBackgroundObjUndoAction::~SdBackgroundObjUndoAction()
{
    delete mpBackgroundObj;
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

void SdBackgroundObjUndoAction::Repeat()
{
}

// -----------------------------------------------------------------------------

BOOL SdBackgroundObjUndoAction::CanRepeat( SfxRepeatTarget& ) const
{
    return FALSE;
}

// -----------------------------------------------------------------------------

SdUndoAction* SdBackgroundObjUndoAction::Clone() const
{
    return new SdBackgroundObjUndoAction( *pDoc, mrPage, mpBackgroundObj );
}
