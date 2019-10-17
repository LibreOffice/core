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
#include <vcl/lok.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <sal/log.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <editeng/outliner.hxx>

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
    return static_cast<sal_Int32>(pViewShell->GetViewShellId());
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
        if (static_cast<sal_Int32>(pViewShell->GetViewShellId()) == nViewShellId)
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
        if (static_cast<sal_Int32>(pViewShell->GetViewShellId()) == nViewShellId)
        {
            // update the current LOK language for the dialog tunneling
            comphelper::LibreOfficeKit::setLanguageTag(pViewShell->GetLOKLanguageTag());

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

    return static_cast<sal_Int32>(pViewShell->GetViewShellId());
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
        pArray[i] = static_cast<sal_Int32>(pViewShell->GetViewShellId());
    }
    return true;
}

void SfxLokHelper::setViewLanguage(int nId, const OUString& rBcp47LanguageTag)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            pViewShell->SetLOKLanguageTag(rBcp47LanguageTag);
            return;
        }
    }
}

static OString lcl_escapeQuotes(const OString &rStr)
{
    if (rStr.getLength() < 1)
        return rStr;
    // FIXME: need an optimized 'escape' method for O[U]String.
    OStringBuffer aBuf(rStr.getLength() + 8);
    for (sal_Int32 i = 0; i < rStr.getLength(); ++i)
    {
        if (rStr[i] == '"' || rStr[i] == '\\')
            aBuf.append('\\');
        aBuf.append(rStr[i]);
    }
    return aBuf.makeStringAndClear();
}

void SfxLokHelper::notifyOtherView(SfxViewShell* pThisView, SfxViewShell const* pOtherView, int nType, const OString& rKey, const OString& rPayload)
{
    OString aPayload = OStringLiteral("{ \"viewId\": \"") + OString::number(SfxLokHelper::getView(pThisView)) +
                       "\", \"part\": \"" + OString::number(pThisView->getPart()) +
                       "\", \"" + rKey + "\": \"" + lcl_escapeQuotes(rPayload) + "\" }";

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

void SfxLokHelper::notifyWindow(const SfxViewShell* pThisView,
                                vcl::LOKWindowId nLOKWindowId,
                                const OUString& rAction,
                                const std::vector<vcl::LOKPayloadItem>& rPayload)
{
    assert(pThisView);

    if (SfxLokHelper::getViewsCount() <= 0 || nLOKWindowId == 0)
        return;

    OStringBuffer aPayload;
    aPayload.append("{ \"id\": \"").append(OString::number(nLOKWindowId)).append("\"");
    aPayload.append(", \"action\": \"").append(OUStringToOString(rAction, RTL_TEXTENCODING_UTF8)).append("\"");

    for (const auto& rItem: rPayload)
    {
        if (!rItem.first.isEmpty() && !rItem.second.isEmpty())
        {
            aPayload.append(", \"").append(rItem.first).append("\": \"")
                .append(rItem.second).append("\"");
        }
    }
    aPayload.append("}");

    auto s = aPayload.makeStringAndClear();
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_WINDOW, s.getStr());
}

void SfxLokHelper::notifyInvalidation(SfxViewShell const* pThisView, const OString& rPayload)
{
    OStringBuffer aBuf;
    aBuf.append(rPayload);
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
    {
        aBuf.append(", ");
        aBuf.append(static_cast<sal_Int32>(pThisView->getPart()));
    }
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, aBuf.makeStringAndClear().getStr());
}

void SfxLokHelper::notifyDocumentSizeChanged(SfxViewShell const* pThisView, const OString& rPayload, vcl::ITiledRenderable* pDoc, bool bInvalidateAll)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    if (!pDoc)
        return;

    if (bInvalidateAll)
    {
        for (int i = 0; i < pDoc->getParts(); ++i)
        {
            tools::Rectangle aRectangle(0, 0, 1000000000, 1000000000);
            OString sPayload = aRectangle.toString() + ", " + OString::number(i);
            pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, sPayload.getStr());
        }
    }
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, rPayload.getStr());
}

void SfxLokHelper::notifyDocumentSizeChangedAllViews(vcl::ITiledRenderable* pDoc, bool bInvalidateAll)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        SfxLokHelper::notifyDocumentSizeChanged(pViewShell, "", pDoc, bInvalidateAll);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyVisCursorInvalidation(OutlinerViewShell const* pThisView, const OString& rRectangle)
{
    OString sPayload;
    if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
    {
        sPayload = OStringLiteral("{ \"viewId\": \"") + OString::number(SfxLokHelper::getView()) +
            "\", \"rectangle\": \"" + rRectangle + "\" }";
    }
    else
    {
        sPayload = rRectangle;
    }
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, sPayload.getStr());
}

void SfxLokHelper::notifyAllViews(int nType, const OString& rPayload)
{
    const auto payload = rPayload.getStr();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(nType, payload);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyContextChange(SfxViewShell const* pViewShell, const OUString& aApplication, const OUString& aContext)
{
    OString aBuffer =
        OUStringToOString(aApplication.replace(' ', '_'), RTL_TEXTENCODING_UTF8) +
        " " +
        OUStringToOString(aContext.replace(' ', '_'), RTL_TEXTENCODING_UTF8);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CONTEXT_CHANGED, aBuffer.getStr());
}


namespace
{
    struct LOKAsyncEventData
    {
        int mnView; // Window is not enough.
        VclPtr<vcl::Window> mpWindow;
        VclEventId mnEvent;
        MouseEvent maMouseEvent;
        KeyEvent maKeyEvent;
        OUString maText;
    };

    void LOKPostAsyncEvent(void* pEv, void*)
    {
        LOKAsyncEventData* pLOKEv = static_cast<LOKAsyncEventData*>(pEv);
        if (pLOKEv->mpWindow->IsDisposed())
            return;

        int nView = SfxLokHelper::getView(nullptr);
        if (nView != pLOKEv->mnView)
        {
            SAL_INFO("sfx.view", "LOK - view mismatch " << nView << " vs. " << pLOKEv->mnView);
            SfxLokHelper::setView(pLOKEv->mnView);
        }

        if (!pLOKEv->mpWindow->HasChildPathFocus(true))
        {
            SAL_INFO("sfx.view", "LOK - focus mismatch, switching focus");
            pLOKEv->mpWindow->GrabFocus();
        }

        VclPtr<vcl::Window> pFocusWindow = pLOKEv->mpWindow->GetFocusedWindow();
        if (!pFocusWindow)
            pFocusWindow = pLOKEv->mpWindow;

        switch (pLOKEv->mnEvent)
        {
        case VclEventId::WindowKeyInput:
        {
            sal_uInt16 nRepeat = pLOKEv->maKeyEvent.GetRepeat();
            KeyEvent singlePress(pLOKEv->maKeyEvent.GetCharCode(),
                                 pLOKEv->maKeyEvent.GetKeyCode());
            for (sal_uInt16 i = 0; i <= nRepeat; ++i)
                pFocusWindow->KeyInput(singlePress);
            break;
        }
        case VclEventId::WindowKeyUp:
            pFocusWindow->KeyUp(pLOKEv->maKeyEvent);
            break;
        case VclEventId::WindowMouseButtonDown:
            pLOKEv->mpWindow->LogicMouseButtonDown(pLOKEv->maMouseEvent);
            // Invoke the context menu
            if (pLOKEv->maMouseEvent.GetButtons() & MOUSE_RIGHT)
            {
                const CommandEvent aCEvt(pLOKEv->maMouseEvent.GetPosPixel(), CommandEventId::ContextMenu, true, nullptr);
                pLOKEv->mpWindow->Command(aCEvt);
            }
            break;
        case VclEventId::WindowMouseButtonUp:
            pLOKEv->mpWindow->LogicMouseButtonUp(pLOKEv->maMouseEvent);

            // sometimes MouseButtonDown captures mouse and starts tracking, and VCL
            // will not take care of releasing that with tiled rendering
            if (pLOKEv->mpWindow->IsTracking())
                pLOKEv->mpWindow->EndTracking();

            break;
        case VclEventId::WindowMouseMove:
            pLOKEv->mpWindow->LogicMouseMove(pLOKEv->maMouseEvent);
            break;
        case VclEventId::ExtTextInput:
        case VclEventId::EndExtTextInput:
            pLOKEv->mpWindow->PostExtTextInputEvent(pLOKEv->mnEvent, pLOKEv->maText);
            break;
        default:
            assert(false);
            break;
        }

        delete pLOKEv;
    }

    void postEventAsync(LOKAsyncEventData *pEvent)
    {
        if (!pEvent->mpWindow || pEvent->mpWindow->IsDisposed())
        {
            SAL_WARN("vcl", "Async event post - but no valid window as destination " << pEvent->mpWindow.get());
            delete pEvent;
            return;
        }

        pEvent->mnView = SfxLokHelper::getView(nullptr);
        if (vcl::lok::isUnipoll())
        {
            if (!Application::IsMainThread())
                SAL_WARN("lok", "Posting event directly but not called from main thread!");
            LOKPostAsyncEvent(pEvent, nullptr);
        }
        else
            Application::PostUserEvent(Link<void*, void>(pEvent, LOKPostAsyncEvent));
    }
}

void SfxLokHelper::postKeyEventAsync(const VclPtr<vcl::Window> &xWindow,
                                     int nType, int nCharCode, int nKeyCode, int nRepeat)
{
    LOKAsyncEventData* pLOKEv = new LOKAsyncEventData;
    switch (nType)
    {
    case LOK_KEYEVENT_KEYINPUT:
        pLOKEv->mnEvent = VclEventId::WindowKeyInput;
        break;
    case LOK_KEYEVENT_KEYUP:
        pLOKEv->mnEvent = VclEventId::WindowKeyUp;
        break;
    default:
        assert(false);
    }
    pLOKEv->maKeyEvent = KeyEvent(nCharCode, nKeyCode, nRepeat);
    pLOKEv->mpWindow = xWindow;
    postEventAsync(pLOKEv);
}

void SfxLokHelper::postExtTextEventAsync(const VclPtr<vcl::Window> &xWindow,
                                         int nType, const OUString &rText)
{
    LOKAsyncEventData* pLOKEv = new LOKAsyncEventData;
    switch (nType)
    {
    case LOK_EXT_TEXTINPUT:
        pLOKEv->mnEvent = VclEventId::ExtTextInput;
        pLOKEv->maText = rText;
        break;
    case LOK_EXT_TEXTINPUT_END:
        pLOKEv->mnEvent = VclEventId::EndExtTextInput;
        pLOKEv->maText = "";
        break;
    default:
        assert(false);
    }
    pLOKEv->mpWindow = xWindow;
    postEventAsync(pLOKEv);
}

void SfxLokHelper::postMouseEventAsync(const VclPtr<vcl::Window> &xWindow,
                                       int nType, const Point &rPos,
                                       int nCount, MouseEventModifiers aModifiers,
                                       int nButtons, int nModifier)
{
    LOKAsyncEventData* pLOKEv = new LOKAsyncEventData;
    switch (nType)
    {
    case LOK_MOUSEEVENT_MOUSEBUTTONDOWN:
        pLOKEv->mnEvent = VclEventId::WindowMouseButtonDown;
        break;
    case LOK_MOUSEEVENT_MOUSEBUTTONUP:
        pLOKEv->mnEvent = VclEventId::WindowMouseButtonUp;
        break;
    case LOK_MOUSEEVENT_MOUSEMOVE:
        pLOKEv->mnEvent = VclEventId::WindowMouseMove;
        break;
    default:
        assert(false);
    }

    // no reason - just always true so far.
    assert (aModifiers == MouseEventModifiers::SIMPLECLICK);

    pLOKEv->maMouseEvent = MouseEvent(rPos, nCount,
                                      aModifiers, nButtons, nModifier);
    pLOKEv->mpWindow = xWindow;
    postEventAsync(pLOKEv);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
