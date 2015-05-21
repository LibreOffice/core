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
#include <vcl/settings.hxx>
#include <vector>

//these tab pages both have the same basic layout with a preview on the
//right, get both of their non-preview areas to request the same size
//so that the preview appears in the same place in each one so
//flipping between tabs isn't distracting as it jumps around
void setPreviewsToSamePlace(vcl::Window *pParent, VclBuilderContainer *pPage)
{
    vcl::Window *pOurGrid = pPage->get<vcl::Window>("maingrid");
    if (!pOurGrid)
        return;

    std::vector<vcl::Window*> aGrids;
    aGrids.push_back(pOurGrid);

    for (vcl::Window* pChild = pParent->GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        VclBuilderContainer *pPeer = dynamic_cast<VclBuilderContainer*>(pChild);
        if (!pPeer || pPeer == pPage || !pPeer->hasBuilder())
            continue;

        vcl::Window *pOtherGrid = pPeer->get<vcl::Window>("maingrid");
        if (!pOtherGrid)
            continue;

       aGrids.push_back(pOtherGrid);
    }

    if (aGrids.size() > 1)
    {
        std::shared_ptr<VclSizeGroup> xGroup(std::make_shared<VclSizeGroup>());
        for (std::vector<vcl::Window*>::iterator aI = aGrids.begin(); aI != aGrids.end(); ++aI)
        {
            vcl::Window *pWindow = *aI;
            pWindow->remove_from_all_size_groups();
            pWindow->add_to_size_group(xGroup);
        }
    }
}

Size getParagraphPreviewOptimalSize(const vcl::Window *pReference)
{
    return pReference->LogicToPixel(Size(68 , 112), MAP_APPFONT);
}

Size getDrawPreviewOptimalSize(const vcl::Window *pReference)
{
    return pReference->LogicToPixel(Size(88, 42), MAP_APPFONT);
}

Size getDrawListBoxOptimalSize(const vcl::Window *pReference)
{
    return pReference->LogicToPixel(Size(88, 110), MAP_APPFONT);
}

Size getPreviewStripSize(const vcl::Window *pReference)
{
    return pReference->LogicToPixel(Size(70 , 40), MapMode(MAP_APPFONT));
}

Size getPreviewOptionsSize(const vcl::Window *pReference)
{
    return pReference->LogicToPixel(Size(70 , 27), MapMode(MAP_APPFONT));
}

OUString getWidestTime(const LocaleDataWrapper& rWrapper)
{
    Date aDate(22, 12, 2000);
    tools::Time aTime(22, 59, 59);
    DateTime aDateTime(aDate, aTime);
    return formatTime(aDateTime, rWrapper);
}

OUString formatTime(const DateTime& rDateTime, const LocaleDataWrapper& rWrapper)
{
    OUString sString = rWrapper.getDate(rDateTime);
    sString += OUString(' ');
    sString += rWrapper.getTime(rDateTime, false);
    return sString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
