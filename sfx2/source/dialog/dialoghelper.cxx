/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/dialoghelper.hxx>
#include <tools/datetime.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <vector>

//these tab pages both have the same basic layout with a preview on the
//right, get both of their non-preview areas to request the same size
//so that the preview appears in the same place in each one so
//flipping between tabs isn't distracting as it jumps around
void setPreviewsToSamePlace(Window *pParent, VclBuilderContainer *pPage)
{
    Window *pOurGrid = pPage->get<Window>("maingrid");
    if (!pOurGrid)
        return;

    std::vector<Window*> aGrids;
    aGrids.push_back(pOurGrid);

    for (Window* pChild = pParent->GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        VclBuilderContainer *pPeer = dynamic_cast<VclBuilderContainer*>(pChild);
        if (!pPeer || pPeer == pPage || !pPeer->hasBuilder())
            continue;

        Window *pOtherGrid = pPeer->get<Window>("maingrid");
        if (!pOtherGrid)
            continue;

       aGrids.push_back(pOtherGrid);
    }

    if (aGrids.size() > 1)
    {
        boost::shared_ptr< VclSizeGroup > xGroup(new VclSizeGroup);
        for (std::vector<Window*>::iterator aI = aGrids.begin(); aI != aGrids.end(); ++aI)
        {
            Window *pWindow = *aI;
            pWindow->remove_from_all_size_groups();
            pWindow->add_to_size_group(xGroup);
        }
    }
}

Size getParagraphPreviewOptimalSize(const Window *pReference)
{
    return pReference->LogicToPixel(Size(68 , 112), MAP_APPFONT);
}

OUString SFX2_DLLPUBLIC getWidestTime(const LocaleDataWrapper& rWrapper)
{
    Date aDate(22, 12, 2000);
    Time aTime(22, 59, 59);
    DateTime aDateTime(aDate, aTime);
    return formatTime(aDateTime, rWrapper);
}

OUString SFX2_DLLPUBLIC formatTime(const DateTime& rDateTime, const LocaleDataWrapper& rWrapper)
{
    OUString sString = rWrapper.getDate(rDateTime);
    sString += OUString(' ');
    sString += rWrapper.getTime(rDateTime, false);
    return sString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
