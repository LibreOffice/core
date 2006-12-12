/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unprlout.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:30:08 $
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


#include "unprlout.hxx"

#include "strings.hrc"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"


TYPEINIT1(SdPresentationLayoutUndoAction, SdUndoAction);



/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdPresentationLayoutUndoAction::SdPresentationLayoutUndoAction(
                            SdDrawDocument* pTheDoc,
                            String          aTheOldLayoutName,
                            String          aTheNewLayoutName,
                            AutoLayout      eTheOldAutoLayout,
                            AutoLayout      eTheNewAutoLayout,
                            BOOL            bSet,
                            SdPage*         pThePage):
                      SdUndoAction(pTheDoc)
{
    aOldLayoutName = aTheOldLayoutName;
    aNewLayoutName = aTheNewLayoutName;
    eOldAutoLayout = eTheOldAutoLayout;
    eNewAutoLayout = eTheNewAutoLayout;
    bSetAutoLayout = bSet;

    DBG_ASSERT(pThePage, "keine Page gesetzt!");
    pPage = pThePage;
    aComment = String(SdResId(STR_UNDO_SET_PRESLAYOUT));
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdPresentationLayoutUndoAction::Undo()
{
    pPage->SetPresentationLayout(aOldLayoutName, TRUE, TRUE, TRUE);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eOldAutoLayout, TRUE);
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void SdPresentationLayoutUndoAction::Redo()
{
    pPage->SetPresentationLayout(aNewLayoutName);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eNewAutoLayout, TRUE);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdPresentationLayoutUndoAction::~SdPresentationLayoutUndoAction()
{
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String SdPresentationLayoutUndoAction::GetComment() const
{
    return aComment;
}



