/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <sfx2/lokhelper.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/lok.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <sal/log.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/msgpool.hxx>

#include <shellimpl.hxx>

#include <boost/property_tree/json_parser.hpp>

using namespace com::sun::star;

namespace {
/// Used to disable callbacks.
/// Needed to avoid recursion when switching views,
/// which can cause clients to invoke LOKit API and
/// implicitly set the view, which might cause an
/// infinite recursion if not detected and prevented.
class DisableCallbacks
{
public:
    DisableCallbacks()
    {
        assert(m_nDisabled >= 0 && "Expected non-negative DisabledCallbacks state when disabling.");
        ++m_nDisabled;
    }

    ~DisableCallbacks()
    {
        assert(m_nDisabled > 0 && "Expected positive DisabledCallbacks state when re-enabling.");
        --m_nDisabled;
    }

    static inline bool disabled()
    {
        return !comphelper::LibreOfficeKit::isActive() || m_nDisabled != 0;
    }

private:
    static int m_nDisabled;
};

int DisableCallbacks::m_nDisabled = 0;
}

namespace
{
LanguageTag g_defaultLanguageTag("en-US", true);
LOKDeviceFormFactor g_deviceFormFactor = LOKDeviceFormFactor::UNKNOWN;
}

int SfxLokHelper::createView(SfxViewFrame* pViewFrame, ViewShellDocId docId)
{
    assert(docId >= ViewShellDocId(0) && "Cannot createView for invalid (negative) DocId.");

    if (pViewFrame == nullptr)
        return -1;

    SfxViewShell::SetCurrentDocId(docId);
    SfxRequest aRequest(pViewFrame, SID_NEWWINDOW);
    pViewFrame->ExecView_Impl(aRequest);
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell == nullptr)
        return -1;

    assert(pViewShell->GetDocId() == docId && "DocId must be already set!");
    return static_cast<sal_Int32>(pViewShell->GetViewShellId());
}

int SfxLokHelper::createView()
{
    // Assumes a single document, or at least that the
    // current view belongs to the document on which the
    // view will be created.
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell == nullptr)
        return -1;

    return createView(pViewShell->GetViewFrame(), pViewShell->GetDocId());
}

int SfxLokHelper::createView(int nDocId)
{
    const SfxApplication* pApp = SfxApplication::Get();
    if (pApp == nullptr)
        return -1;

    // Find a shell with the given DocId.
    const ViewShellDocId docId(nDocId);
    for (const SfxViewShell* pViewShell : pApp->GetViewShells_Impl())
    {
        if (pViewShell->GetDocId() == docId)
            return createView(pViewShell->GetViewFrame(), docId);
    }

    // No frame with nDocId found.
    return -1;
}

void SfxLokHelper::destroyView(int nId)
{
    const SfxApplication* pApp = SfxApplication::Get();
    if (pApp == nullptr)
        return;

    const ViewShellId nViewShellId(nId);
    SfxViewShellArr_Impl& rViewArr = pApp->GetViewShells_Impl();

    for (const SfxViewShell* pViewShell : rViewArr)
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
    SfxApplication* pApp = SfxApplication::Get();
    if (pApp == nullptr)
        return;

    const ViewShellId nViewShellId(nId);
    SfxViewShellArr_Impl& rViewArr = pApp->GetViewShells_Impl();

    for (const SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == nViewShellId)
        {
            DisableCallbacks dc;

            // update the current LOK language and locale for the dialog tunneling
            comphelper::LibreOfficeKit::setLanguageTag(pViewShell->GetLOKLanguageTag());
            comphelper::LibreOfficeKit::setLocale(pViewShell->GetLOKLocale());

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

int SfxLokHelper::getView(const SfxViewShell* pViewShell)
{
    if (!pViewShell)
        pViewShell = SfxViewShell::Current();
    // Still no valid view shell? Then no idea.
    if (!pViewShell)
        return -1;

    return static_cast<sal_Int32>(pViewShell->GetViewShellId());
}

std::size_t SfxLokHelper::getViewsCount(int nDocId)
{
    assert(nDocId != -1 && "Cannot getViewsCount for invalid DocId -1");

    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return 0;

    const ViewShellDocId nCurrentDocId(nDocId);
    std::size_t n = 0;
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId() == nCurrentDocId)
            n++;
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }

    return n;
}

bool SfxLokHelper::getViewIds(int nDocId, int* pArray, size_t nSize)
{
    assert(nDocId != -1 && "Cannot getViewsIds for invalid DocId -1");

    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return false;

    const ViewShellDocId nCurrentDocId(nDocId);
    std::size_t n = 0;
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId() == nCurrentDocId)
        {
            if (n == nSize)
                return false;

            pArray[n] = static_cast<sal_Int32>(pViewShell->GetViewShellId());
            n++;
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }

    return true;
}

int SfxLokHelper::getDocumentIdOfView(int nViewId)
{
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetViewShellId() == ViewShellId(nViewId))
            return static_cast<int>(pViewShell->GetDocId());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
    return -1;
}

const LanguageTag & SfxLokHelper::getDefaultLanguage()
{
    return g_defaultLanguageTag;
}

void SfxLokHelper::setDefaultLanguage(const OUString& rBcp47LanguageTag)
{
    g_defaultLanguageTag = LanguageTag(rBcp47LanguageTag, true);
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

void SfxLokHelper::setViewLocale(int nId, const OUString& rBcp47LanguageTag)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            pViewShell->SetLOKLocale(rBcp47LanguageTag);
            return;
        }
    }
}

LOKDeviceFormFactor SfxLokHelper::getDeviceFormFactor()
{
    return g_deviceFormFactor;
}

void SfxLokHelper::setDeviceFormFactor(std::u16string_view rDeviceFormFactor)
{
    if (rDeviceFormFactor == u"desktop")
        g_deviceFormFactor = LOKDeviceFormFactor::DESKTOP;
    else if (rDeviceFormFactor == u"tablet")
        g_deviceFormFactor = LOKDeviceFormFactor::TABLET;
    else if (rDeviceFormFactor == u"mobile")
        g_deviceFormFactor = LOKDeviceFormFactor::MOBILE;
    else
        g_deviceFormFactor = LOKDeviceFormFactor::UNKNOWN;
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

static OString lcl_generateJSON(const SfxViewShell* pView, const boost::property_tree::ptree& rTree)
{
    assert(pView != nullptr && "pView must be valid");
    boost::property_tree::ptree aMessageProps = rTree;
    aMessageProps.put("viewId", SfxLokHelper::getView(pView));
    aMessageProps.put("part", pView->getPart());
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aMessageProps, false /* pretty */);
    const std::string aString = aStream.str();
    return OString(aString.c_str(), aString.size()).trim();
}

static inline OString lcl_generateJSON(const SfxViewShell* pView, std::string_view rKey,
                                       const OString& rPayload)
{
    assert(pView != nullptr && "pView must be valid");
    return OStringLiteral("{ \"viewId\": \"") + OString::number(SfxLokHelper::getView(pView))
           + "\", \"part\": \"" + OString::number(pView->getPart()) + "\", \"" + rKey + "\": \""
           + lcl_escapeQuotes(rPayload) + "\" }";
}

void SfxLokHelper::notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                   int nType, std::string_view rKey, const OString& rPayload)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    const OString aPayload = lcl_generateJSON(pThisView, rKey, rPayload);
    pOtherView->libreOfficeKitViewCallback(nType, aPayload.getStr());
}

void SfxLokHelper::notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                   int nType, const boost::property_tree::ptree& rTree)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    pOtherView->libreOfficeKitViewCallback(nType, lcl_generateJSON(pThisView, rTree).getStr());
}

void SfxLokHelper::notifyOtherViews(const SfxViewShell* pThisView, int nType, std::string_view rKey,
                                    const OString& rPayload)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    // Cache the payload so we only have to generate it once, at most.
    OString aPayload;

    const ViewShellDocId nCurrentDocId = pThisView->GetDocId();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView && nCurrentDocId == pViewShell->GetDocId())
        {
            // Payload is only dependent on pThisView.
            if (aPayload.isEmpty())
                aPayload = lcl_generateJSON(pThisView, rKey, rPayload);

            pViewShell->libreOfficeKitViewCallback(nType, aPayload.getStr());
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyOtherViews(const SfxViewShell* pThisView, int nType,
                                    const boost::property_tree::ptree& rTree)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    // Cache the payload so we only have to generate it once, at most.
    OString aPayload;

    const ViewShellDocId nCurrentDocId = pThisView->GetDocId();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView && nCurrentDocId == pViewShell->GetDocId())
        {
            // Payload is only dependent on pThisView.
            if (aPayload.isEmpty())
                aPayload = lcl_generateJSON(pThisView, rTree);

            pViewShell->libreOfficeKitViewCallback(nType, aPayload.getStr());
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

namespace {
    OUString lcl_getNameForSlot(const SfxViewShell* pShell, sal_uInt16 nWhich)
    {
        if (pShell && pShell->GetFrame())
        {
            const SfxSlot* pSlot = SfxSlotPool::GetSlotPool(pShell->GetFrame()).GetSlot(nWhich);
            if (pSlot)
            {
                const char* pName = pSlot->GetUnoName();
                if (pName)
                {
                    return ".uno:" + OStringToOUString(pName, RTL_TEXTENCODING_ASCII_US);
                }
            }
        }

        return "";
    }
}

void SfxLokHelper::sendUnoStatus(const SfxViewShell* pShell, const SfxPoolItem* pItem)
{
    if (!pShell || !pItem || pItem == INVALID_POOL_ITEM || DisableCallbacks::disabled())
        return;

    boost::property_tree::ptree aItem = pItem->dumpAsJSON();

    if (aItem.count("state"))
    {
        OUString sCommand = lcl_getNameForSlot(pShell, pItem->Which());
        if (!sCommand.isEmpty())
            aItem.put("commandName", sCommand);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aItem);
        pShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, aStream.str().c_str());
    }
}

void SfxLokHelper::notifyWindow(const SfxViewShell* pThisView,
                                vcl::LOKWindowId nLOKWindowId,
                                std::u16string_view rAction,
                                const std::vector<vcl::LOKPayloadItem>& rPayload)
{
    assert(pThisView != nullptr && "pThisView must be valid");

    if (nLOKWindowId == 0 || DisableCallbacks::disabled())
        return;

    OStringBuffer aPayload;
    aPayload.append("{ \"id\": \"").append(OString::number(nLOKWindowId)).append('"');
    aPayload.append(", \"action\": \"").append(OUStringToOString(rAction, RTL_TEXTENCODING_UTF8)).append('"');

    for (const auto& rItem: rPayload)
    {
        if (!rItem.first.isEmpty() && !rItem.second.isEmpty())
        {
            aPayload.append(", \"").append(rItem.first).append("\": \"")
                .append(rItem.second).append('"');
        }
    }
    aPayload.append('}');

    const OString s = aPayload.makeStringAndClear();
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_WINDOW, s.getStr());
}

void SfxLokHelper::notifyInvalidation(SfxViewShell const* pThisView, std::string_view rPayload)
{
    OStringBuffer aBuf(32);

    if (DisableCallbacks::disabled())
        return;

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
    if (!pDoc || pDoc->isDisposed() || DisableCallbacks::disabled())
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
    if (DisableCallbacks::disabled())
        return;

    // FIXME: Do we know whether it is the views for the document that is in the "current" view that has changed?
    const SfxViewShell* const pCurrentViewShell = SfxViewShell::Current();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        // FIXME: What if SfxViewShell::Current() returned null?
        // Should we then do this for all views of all open documents
        // or not?
        if (pCurrentViewShell == nullptr || pViewShell->GetDocId() == pCurrentViewShell-> GetDocId())
            SfxLokHelper::notifyDocumentSizeChanged(pViewShell, "", pDoc, bInvalidateAll);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyVisCursorInvalidation(OutlinerViewShell const* pThisView, const OString& rRectangle, bool bMispelledWord, const OString& rHyperlink)
{
    if (DisableCallbacks::disabled())
        return;

    OString sPayload;
    if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
    {
        OString sHyperlink = rHyperlink.isEmpty() ? "{}" : rHyperlink;
        sPayload = OStringLiteral("{ \"viewId\": \"") + OString::number(SfxLokHelper::getView()) +
            "\", \"rectangle\": \"" + rRectangle +
            "\", \"mispelledWord\": \"" +  OString::number(bMispelledWord ? 1 : 0) +
            "\", \"hyperlink\": " + sHyperlink + " }";
    }
    else
    {
        sPayload = rRectangle;
    }
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, sPayload.getStr());
}

void SfxLokHelper::notifyAllViews(int nType, const OString& rPayload)
{
    if (DisableCallbacks::disabled())
        return;

    const auto payload = rPayload.getStr();
    const SfxViewShell* const pCurrentViewShell = SfxViewShell::Current();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId() == pCurrentViewShell->GetDocId())
            pViewShell->libreOfficeKitViewCallback(nType, payload);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyContextChange(SfxViewShell const* pViewShell, const OUString& aApplication, const OUString& aContext)
{
    if (DisableCallbacks::disabled())
        return;

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

void SfxLokHelper::postMouseEventAsync(const VclPtr<vcl::Window> &xWindow, LokMouseEventData const & rLokMouseEventData)
{
    LOKAsyncEventData* pLOKEv = new LOKAsyncEventData;
    switch (rLokMouseEventData.mnType)
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
    assert (rLokMouseEventData.meModifiers == MouseEventModifiers::SIMPLECLICK);

    pLOKEv->maMouseEvent = MouseEvent(rLokMouseEventData.maPosition, rLokMouseEventData.mnCount,
                                      rLokMouseEventData.meModifiers, rLokMouseEventData.mnButtons,
                                      rLokMouseEventData.mnModifier);
    if (rLokMouseEventData.maLogicPosition)
    {
        pLOKEv->maMouseEvent.setLogicPosition(*rLokMouseEventData.maLogicPosition);
    }
    pLOKEv->mpWindow = xWindow;
    postEventAsync(pLOKEv);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
