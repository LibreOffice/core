/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unmovss.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:29:34 $
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


#include "unmovss.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "stlsheet.hxx"


TYPEINIT1(SdMoveStyleSheetsUndoAction, SdUndoAction);



/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdMoveStyleSheetsUndoAction::SdMoveStyleSheetsUndoAction(
                                SdDrawDocument* pTheDoc,
                                List*           pTheStyles,
                                BOOL            bInserted):
                      SdUndoAction(pTheDoc)
{
    DBG_ASSERT(pTheStyles, "keine Liste gesetzt!");
    pStyles   = pTheStyles;
    bMySheets = !bInserted;

    pListOfChildLists = new List;

    // Liste mit den Listen der StyleSheet-Kinder erstellen
    for (SdStyleSheet* pSheet = (SdStyleSheet*)pStyles->First();
         pSheet;
         pSheet = (SdStyleSheet*)pStyles->Next())
    {
        List* pChildList = pSheet->CreateChildList();
        pListOfChildLists->Insert(pChildList, LIST_APPEND);
    }
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdMoveStyleSheetsUndoAction::Undo()
{
    SfxStyleSheetBasePool* pPool  = mpDoc->GetStyleSheetPool();
    SdStyleSheet*          pSheet = NULL;

    /********************************************************************
    |* die StyleSheets sollen wieder in den Pool eingefuegt werden
    \*******************************************************************/
    if (bMySheets)
    {
        /****************************************************************
        |* erst alle StyleSheets wieder in den Pool einfuegen
        \***************************************************************/
        for (pSheet = (SdStyleSheet*)pStyles->First();
             pSheet;
             pSheet = (SdStyleSheet*)pStyles->Next())
        {
            pPool->Insert(pSheet);
        }

        /****************************************************************
        |* jetzt die ehemaligen Kinder wieder zu Kindern machen
        \***************************************************************/
        List* pChildList = (List*)pListOfChildLists->First();
        for (pSheet = (SdStyleSheet*)pStyles->First();
             pSheet;
             pSheet = (SdStyleSheet*)pStyles->Next())
        {
            String aParent(pSheet->GetName());
            for (SfxStyleSheet* pChild = (SfxStyleSheet*)pChildList->First();
                 pChild;
                 pChild = (SfxStyleSheet*)pChildList->Next())
            {
                pChild->SetParent(aParent);
            }
            pChildList = (List*)pListOfChildLists->Next();
        }
    }
    /********************************************************************
    |* die StyleSheets sollen wieder aus dem, Pool entfernt werden
    \*******************************************************************/
    else
    {
        for (pSheet = (SdStyleSheet*)pStyles->First();
             pSheet;
             pSheet = (SdStyleSheet*)pStyles->Next())
        {
            pPool->Remove(pSheet);
        }
    }
    bMySheets = !bMySheets;
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void SdMoveStyleSheetsUndoAction::Redo()
{
    Undo();
}

/*************************************************************************
|*
|* Destruktor, Liste loeschen; ggfs. die enthaltenen StyleSheets loeschen
|*
\************************************************************************/

SdMoveStyleSheetsUndoAction::~SdMoveStyleSheetsUndoAction()
{
    if (bMySheets)
    {
        // die Liste rueckwaerts aufdroeseln; wenn Gliederungsvorlagen ent-
        // halten sind gewaehrleistet dies den geringsten Broadcasting-Aufwand
        SfxStyleSheet* pSheet = (SfxStyleSheet*)pStyles->Last();
        while (pSheet)
        {
            delete pSheet;
            pSheet = (SfxStyleSheet*)pStyles->Prev();
        }
    }
    delete pStyles;

    for (List* pChildList = (List*)pListOfChildLists->First();
         pChildList;
         pChildList = (List*)pListOfChildLists->Next())
    {
        delete pChildList;
    }
    delete pListOfChildLists;
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String SdMoveStyleSheetsUndoAction::GetComment() const
{
    return String();
}


