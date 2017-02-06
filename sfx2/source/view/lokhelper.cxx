/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/lokhelper.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <comphelper/processfactory.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#include <shellimpl.hxx>

using namespace com::sun::star;

int SfxLokHelper::createView()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    if (!pViewFrame)
        return -1;
    SfxRequest aRequest(pViewFrame, SID_NEWWINDOW);
    pViewFrame->ExecView_Impl(aRequest);
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
        return -1;
    return pViewShell->GetViewShellId();
}

void SfxLokHelper::destroyView(int nId)
{
    unsigned nViewShellId = nId;
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == nViewShellId)
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
    unsigned nViewShellId = nId;
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == nViewShellId)
        {
            if (pViewShell == SfxViewShell::Current())
                return;

            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
            pViewFrame->MakeActive_Impl(false);

            // Make comphelper::dispatchCommand() find the correct frame.
            uno::Reference<frame::XFrame> xFrame = pViewFrame->GetFrame().GetFrameInterface();
            uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
            xDesktop->setActiveFrame(xFrame);
            return;
        }
    }

}

int SfxLokHelper::getView(SfxViewShell* pViewShell)
{
    if (!pViewShell)
        pViewShell = SfxViewShell::Current();
    // Still no valid view shell? Then no idea.
    if (!pViewShell)
        return -1;

    return pViewShell->GetViewShellId();
}

std::size_t SfxLokHelper::getViewsCount()
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    return rViewArr.size();
}

bool SfxLokHelper::getViewIds(int* pArray, size_t nSize)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    if (rViewArr.size() > nSize)
        return false;

    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        SfxViewShell* pViewShell = rViewArr[i];
        pArray[i] = pViewShell->GetViewShellId();
    }
    return true;
}

void SfxLokHelper::notifyOtherView(SfxViewShell* pThisView, SfxViewShell* pOtherView, int nType, const OString& rKey, const OString& rPayload)
{
    OString aPayload = OString("{ \"viewId\": \"") + OString::number(SfxLokHelper::getView(pThisView)) +
                       "\", \"part\": \"" + OString::number(pThisView->getPart()) +
                       "\", \"" + rKey + "\": \"" + rPayload + "\" }";

    pOtherView->libreOfficeKitViewCallback(nType, aPayload.getStr());
}

void SfxLokHelper::notifyOtherViews(SfxViewShell* pThisView, int nType, const OString& rKey, const OString& rPayload)
{
    if (SfxLokHelper::getViewsCount() <= 1)
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView)
            notifyOtherView(pThisView, pViewShell, nType, rKey, rPayload);

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyInvalidation(SfxViewShell* pThisView, const OString& rPayload)
{
    std::stringstream ss;
    ss << rPayload.getStr();
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
        ss << ", " << pThisView->getPart();
    OString aPayload = ss.str().c_str();
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, aPayload.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
