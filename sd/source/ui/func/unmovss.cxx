/*************************************************************************
 *
 *  $RCSfile: unmovss.cxx,v $
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

#include "unmovss.hxx"
#include "docshell.hxx"
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

void __EXPORT SdMoveStyleSheetsUndoAction::Undo()
{
    SfxStyleSheetBasePool* pPool  = pDoc->GetStyleSheetPool();
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

void __EXPORT SdMoveStyleSheetsUndoAction::Redo()
{
    Undo();
}

/*************************************************************************
|*
|* Repeat()
|*
\************************************************************************/

void __EXPORT SdMoveStyleSheetsUndoAction::Repeat()
{
    DBG_ASSERT(FALSE, "SdMoveStyleSheetsUndoAction::Repeat: nicht implementiert");
}

/*************************************************************************
|*
|* Destruktor, Liste loeschen; ggfs. die enthaltenen StyleSheets loeschen
|*
\************************************************************************/

__EXPORT SdMoveStyleSheetsUndoAction::~SdMoveStyleSheetsUndoAction()
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

String __EXPORT SdMoveStyleSheetsUndoAction::GetComment() const
{
    return String();
}


