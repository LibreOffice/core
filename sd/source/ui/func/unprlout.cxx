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
                            sal_Bool            bSet,
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
    pPage->SetPresentationLayout(aOldLayoutName, sal_True, sal_True, sal_True);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eOldAutoLayout, sal_True);
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
        pPage->SetAutoLayout(eNewAutoLayout, sal_True);
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



