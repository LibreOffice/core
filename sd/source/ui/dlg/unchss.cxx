/*************************************************************************
 *
 *  $RCSfile: unchss.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:34 $
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

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#include "unchss.hxx"

#include "strings.hrc"
#include "glob.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"



TYPEINIT1(StyleSheetUndoAction, SdUndoAction);



/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

StyleSheetUndoAction::StyleSheetUndoAction(SdDrawDocument* pTheDoc,
                                           SfxStyleSheet* pTheStyleSheet,
                                           const SfxItemSet* pTheNewItemSet) :
                      SdUndoAction(pTheDoc)
{
    DBG_ASSERT(pTheStyleSheet, "Undo ohne StyleSheet ???");
    pStyleSheet = pTheStyleSheet;

    // ItemSets anlegen; Vorsicht, das neue koennte aus einem anderen Pool
    // stammen, also mitsamt seinen Items clonen
    pNewSet = pTheNewItemSet->Clone(TRUE, &(pTheDoc->GetPool()));
    pOldSet = new SfxItemSet(pStyleSheet->GetItemSet());

    aComment = String(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
    String aName(pStyleSheet->GetName());

    // Layoutnamen und Separator loeschen
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ) );
    USHORT nPos = aName.Search(aSep);
    aName.Erase(0, nPos + aSep.Len());

    // Platzhalter durch Vorlagennamen ersetzen
    nPos = aComment.Search(sal_Unicode('$'));
    aComment.Erase(nPos, 1);
    aComment.Insert(aName, nPos);
}


/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void __EXPORT StyleSheetUndoAction::Undo()
{
    pStyleSheet->GetItemSet().Set(*pOldSet);
    pStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void __EXPORT StyleSheetUndoAction::Redo()
{
    pStyleSheet->GetItemSet().Set(*pNewSet);
    pStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
}

/*************************************************************************
|*
|* Repeat()
|*
\************************************************************************/

void __EXPORT StyleSheetUndoAction::Repeat()
{
    DBG_ASSERT(FALSE, "StyleSheetUndoAction::Repeat: nicht implementiert");
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT StyleSheetUndoAction::~StyleSheetUndoAction()
{
    delete pNewSet;
    delete pOldSet;
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String __EXPORT StyleSheetUndoAction::GetComment() const
{
    return aComment;
}






