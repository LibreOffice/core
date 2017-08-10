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
    return (sal_Int32)pViewShell->GetViewShellId();
}

void SfxLokHelper::destroyView(int nId)
{
    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return;

    int nViewShellId = nId;
    SfxViewShellArr_Impl& rViewArr = pApp->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if ((sal_Int32)pViewShell->GetViewShellId() == nViewShellId)
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
    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return;

    int nViewShellId = nId;
    SfxViewShellArr_Impl& rViewArr = pApp->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if ((sal_Int32)pViewShell->GetViewShellId() == nViewShellId)
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

    return (sal_Int32)pViewShell->GetViewShellId();
}

std::size_t SfxLokHelper::getViewsCount()
{
    SfxApplication* pApp = SfxApplication::Get();
    return !pApp ? 0 : pApp->GetViewShells_Impl().size();
}

bool SfxLokHelper::getViewIds(int* pArray, size_t nSize)
{
    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return false;

    SfxViewShellArr_Impl& rViewArr = pApp->GetViewShells_Impl();
    if (rViewArr.size() > nSize)
        return false;

    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        SfxViewShell* pViewShell = rViewArr[i];
        pArray[i] = (sal_Int32)pViewShell->GetViewShellId();
    }
    return true;
}

void SfxLokHelper::notifyOtherView(SfxViewShell* pThisView, SfxViewShell const* pOtherView, int nType, const OString& rKey, const OString& rPayload)
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

void SfxLokHelper::notifyDialog(const OUString& rDialogID, const OUString& rAction)
{
    if (SfxLokHelper::getViewsCount() <= 0)
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    const OString aPayload = OString("{ \"dialogId\": \"") + OUStringToOString(rDialogID, RTL_TEXTENCODING_UTF8).getStr() +
        OString("\", \"action\": \"") + OUStringToOString(rAction, RTL_TEXTENCODING_UTF8).getStr() +
        + "\" }";

    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DIALOG, aPayload.getStr());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyDialogChild(const OUString& rDialogID, const OUString& rAction, const Point& rPos)
{
    if (SfxLokHelper::getViewsCount() <= 0)
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    const OString aPayload = OString("{ \"dialogId\": \"") + OUStringToOString(rDialogID, RTL_TEXTENCODING_UTF8).getStr() +
        OString("\", \"action\": \"") + OUStringToOString(rAction, RTL_TEXTENCODING_UTF8).getStr() +
        OString("\", \"position\": \"") + OString::number(rPos.getX()) + OString(", ") + OString::number(rPos.getY()) +
        + "\" }";

    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DIALOG_CHILD, aPayload.getStr());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyInvalidation(SfxViewShell* pThisView, const OString& rPayload)
{
    OStringBuffer aBuf;
    aBuf.append(rPayload);
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
    {
        aBuf.append(", ");
        aBuf.append((sal_Int32) pThisView->getPart());
    }
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, aBuf.makeStringAndClear().getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
