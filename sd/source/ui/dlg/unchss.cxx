/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/smplhint.hxx>
#include <svx/svdobj.hxx>
#include <svx/globaldrawitempool.hxx>

#include "unchss.hxx"

#include "strings.hrc"
#include "glob.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

StyleSheetUndoAction::StyleSheetUndoAction(
    SdDrawDocument& rTheDoc,
    SfxStyleSheet& rTheStyleSheet,
    const SfxItemSet& rTheNewItemSet)
:   SdUndoAction(&rTheDoc),
    mrStyleSheet(rTheStyleSheet),
    maNewSet(rTheNewItemSet),
    maOldSet(mrStyleSheet.GetItemSet())
{
    aComment = String(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
    String aName(mrStyleSheet.GetName());

    // Layoutnamen und Separator loeschen
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ) );
    sal_uInt16 nPos = aName.Search(aSep);
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
    mrStyleSheet.GetItemSet().Set(maOldSet);

    if(SD_STYLE_FAMILY_PSEUDO == mrStyleSheet.GetFamily())
    {
        static_cast< SdStyleSheet& >(mrStyleSheet).GetRealStyleSheet()->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    }
    else
    {
        mrStyleSheet.Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    }
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void StyleSheetUndoAction::Redo()
{
    mrStyleSheet.GetItemSet().Set(maNewSet);

    if(SD_STYLE_FAMILY_PSEUDO == mrStyleSheet.GetFamily())
    {
        static_cast< SdStyleSheet& >(mrStyleSheet).GetRealStyleSheet()->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    }
    else
    {
        mrStyleSheet.Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

StyleSheetUndoAction::~StyleSheetUndoAction()
{
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

// eof
