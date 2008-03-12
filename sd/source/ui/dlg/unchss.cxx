/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unchss.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:37:35 $
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


#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#include "unchss.hxx"

#include "strings.hrc"
#include "glob.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"


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
    pNewSet = new SfxItemSet((SfxItemPool&)SdrObject::GetGlobalDrawObjectItemPool(), pTheNewItemSet->GetRanges());
    pTheDoc->MigrateItemSet( pTheNewItemSet, pNewSet, pTheDoc );

    pOldSet = new SfxItemSet((SfxItemPool&)SdrObject::GetGlobalDrawObjectItemPool(),pStyleSheet->GetItemSet().GetRanges());
    pTheDoc->MigrateItemSet( &pStyleSheet->GetItemSet(), pOldSet, pTheDoc );

    aComment = String(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
    String aName(pStyleSheet->GetName());

    // Layoutnamen und Separator loeschen
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ) );
    USHORT nPos = aName.Search(aSep);
    if( nPos != STRING_NOTFOUND )
        aName.Erase(0, nPos + aSep.Len());

    if (aName == String(SdResId(STR_LAYOUT_TITLE)))
    {
        aName = String(SdResId(STR_PSEUDOSHEET_TITLE));
    }
    else if (aName == String(SdResId(STR_LAYOUT_SUBTITLE)))
    {
        aName = String(SdResId(STR_PSEUDOSHEET_SUBTITLE));
    }
    else if (aName == String(SdResId(STR_LAYOUT_BACKGROUND)))
    {
        aName = String(SdResId(STR_PSEUDOSHEET_BACKGROUND));
    }
    else if (aName == String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS)))
    {
        aName = String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS));
    }
    else if (aName == String(SdResId(STR_LAYOUT_NOTES)))
    {
        aName = String(SdResId(STR_PSEUDOSHEET_NOTES));
    }
    else
    {
        String aOutlineStr(SdResId(STR_PSEUDOSHEET_OUTLINE));
        nPos = aName.Search(aOutlineStr);
        if (nPos != STRING_NOTFOUND)
        {
            String aNumStr(aName.Copy(aOutlineStr.Len()));
            aName = String(SdResId(STR_LAYOUT_OUTLINE));
            aName += aNumStr;
        }
    }

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

void StyleSheetUndoAction::Undo()
{
    SfxItemSet aNewSet( mpDoc->GetItemPool(), pOldSet->GetRanges() );
    mpDoc->MigrateItemSet( pOldSet, &aNewSet, mpDoc );

    pStyleSheet->GetItemSet().Set(aNewSet);
    if( pStyleSheet->GetFamily() == SD_STYLE_FAMILY_PSEUDO )
        ( (SdStyleSheet*)pStyleSheet )->GetRealStyleSheet()->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    else
        pStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void StyleSheetUndoAction::Redo()
{
    SfxItemSet aNewSet( mpDoc->GetItemPool(), pOldSet->GetRanges() );
    mpDoc->MigrateItemSet( pNewSet, &aNewSet, mpDoc );

    pStyleSheet->GetItemSet().Set(aNewSet);
    if( pStyleSheet->GetFamily() == SD_STYLE_FAMILY_PSEUDO )
        ( (SdStyleSheet*)pStyleSheet )->GetRealStyleSheet()->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    else
        pStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

StyleSheetUndoAction::~StyleSheetUndoAction()
{
    delete pNewSet;
    delete pOldSet;
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String StyleSheetUndoAction::GetComment() const
{
    return aComment;
}
