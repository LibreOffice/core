/*************************************************************************
 *
 *  $RCSfile: unprlout.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:37 $
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

#pragma hdrstop

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

void __EXPORT SdPresentationLayoutUndoAction::Undo()
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

void __EXPORT SdPresentationLayoutUndoAction::Redo()
{
    pPage->SetPresentationLayout(aNewLayoutName);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eNewAutoLayout, TRUE);
}

/*************************************************************************
|*
|* Repeat()
|*
\************************************************************************/

void __EXPORT SdPresentationLayoutUndoAction::Repeat()
{
    DBG_ASSERT(FALSE, "SdPresentationLayoutUndoAction::Repeat: nicht implementiert");
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT SdPresentationLayoutUndoAction::~SdPresentationLayoutUndoAction()
{
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String __EXPORT SdPresentationLayoutUndoAction::GetComment() const
{
    return aComment;
}



