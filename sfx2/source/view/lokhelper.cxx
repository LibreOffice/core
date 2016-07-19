/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/lokhelper.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>

#include <shellimpl.hxx>

namespace
{

/// Assigns a view ID to a view shell.
int shellToView(SfxViewShell* pViewShell)
{
    // Deleted view shells are not removed from this map, so view IDs are not
    // reused when deleting, then creating a view.
    static std::map<SfxViewShell*, int> aViewMap;
    auto it = aViewMap.find(pViewShell);
    if (it != aViewMap.end())
        return it->second;

    int nViewId = aViewMap.size();
    aViewMap[pViewShell] = nViewId;
    return nViewId;
}
}

int SfxLokHelper::createView()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    SfxRequest aRequest(pViewFrame, SID_NEWWINDOW);
    pViewFrame->ExecView_Impl(aRequest);

    return shellToView(SfxViewShell::Current());
}

void SfxLokHelper::destroyView(int nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        SfxViewShell* pViewShell = rViewArr[i];
        if (shellToView(pViewShell) == nId)
        {
            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
            SfxRequest aRequest(pViewFrame, SID_CLOSEWIN);
            pViewFrame->Exec_Impl(aRequest);
            break;
        }
    }
}

void SfxLokHelper::setView(int nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        SfxViewShell* pViewShell = rViewArr[i];
        if (shellToView(pViewShell) == nId)
        {
            if (pViewShell == SfxViewShell::Current())
                return;

            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
            pViewFrame->MakeActive_Impl(false);
            return;
        }
    }

}

int SfxLokHelper::getView(SfxViewShell* pViewShell)
{
    if (!pViewShell)
        pViewShell = SfxViewShell::Current();
    return shellToView(pViewShell);
}

std::size_t SfxLokHelper::getViews()
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    return rViewArr.size();
}

void SfxLokHelper::notifyOtherViews(SfxViewShell* pThisView, int nType, const OString& rKey, const OString& rPayload)
{
    if (SfxLokHelper::getViews() <= 1)
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView)
        {
            boost::property_tree::ptree aTree;
            aTree.put("viewId", SfxLokHelper::getView(pThisView));
            aTree.put(rKey.getStr(), rPayload.getStr());
            aTree.put("part", pThisView->getPart());
            aTree.put(rKey.getStr(), rPayload.getStr());
            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            OString aPayload = aStream.str().c_str();
            pViewShell->libreOfficeKitViewCallback(nType, aPayload.getStr());
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
