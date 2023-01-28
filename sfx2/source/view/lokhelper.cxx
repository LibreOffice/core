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

#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/lokhelper.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/lok.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/window.hxx>
#include <sal/log.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/msgpool.hxx>

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
LanguageTag g_loadLanguageTag("en-US", true); //< The language used to load.
LOKDeviceFormFactor g_deviceFormFactor = LOKDeviceFormFactor::UNKNOWN;
bool g_isDefaultTimezoneSet = false;
OUString g_DefaultTimezone;
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

void SfxLokHelper::setEditMode(int nMode, vcl::ITiledRenderable* pDoc)
{
    DisableCallbacks dc;
    pDoc->setEditMode(nMode);
}

void SfxLokHelper::destroyView(int nId)
{
    const SfxApplication* pApp = SfxApplication::Get();
    if (pApp == nullptr)
        return;

    const ViewShellId nViewShellId(nId);
    std::vector<SfxViewShell*>& rViewArr = pApp->GetViewShells_Impl();

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
    std::vector<SfxViewShell*>& rViewArr = pApp->GetViewShells_Impl();

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

SfxViewShell* SfxLokHelper::getViewOfId(int nId)
{
    SfxApplication* pApp = SfxApplication::Get();
    if (pApp == nullptr)
        return nullptr;

    const ViewShellId nViewShellId(nId);
    std::vector<SfxViewShell*>& rViewArr = pApp->GetViewShells_Impl();
    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == nViewShellId)
            return pViewShell;
    }

    return nullptr;
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

const LanguageTag& SfxLokHelper::getLoadLanguage() { return g_loadLanguageTag; }

void SfxLokHelper::setLoadLanguage(const OUString& rBcp47LanguageTag)
{
    g_loadLanguageTag = LanguageTag(rBcp47LanguageTag, true);
}

void SfxLokHelper::setViewLanguage(int nId, const OUString& rBcp47LanguageTag)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

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
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

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

void SfxLokHelper::setDefaultTimezone(bool isSet, const OUString& rTimezone)
{
    g_isDefaultTimezoneSet = isSet;
    g_DefaultTimezone = rTimezone;
}

std::pair<bool, OUString> SfxLokHelper::getDefaultTimezone()
{
    return { g_isDefaultTimezoneSet, g_DefaultTimezone };
}

void SfxLokHelper::setViewTimezone(int nId, bool isSet, const OUString& rTimezone)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            pViewShell->SetLOKTimezone(isSet, rTimezone);
            return;
        }
    }
}

std::pair<bool, OUString> SfxLokHelper::getViewTimezone(int nId)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            return pViewShell->GetLOKTimezone();
        }
    }

    return {};
}

/*
* Used for putting a whole JSON string into a string value
* e.g { key: "{JSON}" }
*/
static OString lcl_sanitizeJSONAsValue(const OString &rStr)
{
    if (rStr.getLength() < 1)
        return rStr;
    // FIXME: need an optimized 'escape' method for O[U]String.
    OStringBuffer aBuf(rStr.getLength() + 8);
    for (sal_Int32 i = 0; i < rStr.getLength(); ++i)
    {
        if (rStr[i] == '"' || rStr[i] == '\\')
            aBuf.append('\\');

        if (rStr[i] != '\n')
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
    aMessageProps.put("mode", pView->getEditMode());
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aMessageProps, false /* pretty */);
    const std::string aString = aStream.str();
    return OString(aString.c_str(), aString.size()).trim();
}

static inline OString lcl_generateJSON(const SfxViewShell* pView, int nViewId, std::string_view rKey,
                                       const OString& rPayload)
{
    assert(pView != nullptr && "pView must be valid");
    return OString::Concat("{ \"viewId\": \"") + OString::number(nViewId)
           + "\", \"part\": \"" + OString::number(pView->getPart()) + "\", \"mode\": \""
           + OString::number(pView->getEditMode()) + "\", \"" + rKey + "\": \""
           + lcl_sanitizeJSONAsValue(rPayload) + "\" }";
}

static inline OString lcl_generateJSON(const SfxViewShell* pView, std::string_view rKey,
                                       const OString& rPayload)
{
    return lcl_generateJSON(pView, SfxLokHelper::getView(pView), rKey, rPayload);
}

void SfxLokHelper::notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                   int nType, std::string_view rKey, const OString& rPayload)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    const OString aPayload = lcl_generateJSON(pThisView, rKey, rPayload);
    const int viewId = SfxLokHelper::getView(pThisView);
    pOtherView->libreOfficeKitViewCallbackWithViewId(nType, aPayload.getStr(), viewId);
}

void SfxLokHelper::notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                   int nType, const boost::property_tree::ptree& rTree)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    const int viewId = SfxLokHelper::getView(pThisView);
    pOtherView->libreOfficeKitViewCallbackWithViewId(nType, lcl_generateJSON(pThisView, rTree).getStr(), viewId);
}

void SfxLokHelper::notifyOtherViews(const SfxViewShell* pThisView, int nType, std::string_view rKey,
                                    const OString& rPayload)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    // Cache the payload so we only have to generate it once, at most.
    OString aPayload;
    int viewId = -1;

    const ViewShellDocId nCurrentDocId = pThisView->GetDocId();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView && nCurrentDocId == pViewShell->GetDocId())
        {
            // Payload is only dependent on pThisView.
            if (aPayload.isEmpty())
            {
                aPayload = lcl_generateJSON(pThisView, rKey, rPayload);
                viewId = SfxLokHelper::getView(pThisView);
            }

            pViewShell->libreOfficeKitViewCallbackWithViewId(nType, aPayload.getStr(), viewId);
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
    int viewId = -1;

    const ViewShellDocId nCurrentDocId = pThisView->GetDocId();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView && nCurrentDocId == pViewShell->GetDocId())
        {
            // Payload is only dependent on pThisView.
            if (aPayload.isEmpty())
            {
                aPayload = lcl_generateJSON(pThisView, rTree);
                viewId = SfxLokHelper::getView(pThisView);
            }

            pViewShell->libreOfficeKitViewCallbackWithViewId(nType, aPayload.getStr(), viewId);
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

OString SfxLokHelper::makePayloadJSON(const SfxViewShell* pThisView, int nViewId, std::string_view rKey, const OString& rPayload)
{
    return lcl_generateJSON(pThisView, nViewId, rKey, rPayload);
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

    OStringBuffer aPayload =
        "{ \"id\": \"" + OString::number(nLOKWindowId) + "\""
        ", \"action\": \"" + OUStringToOString(rAction, RTL_TEXTENCODING_UTF8) + "\"";

    for (const auto& rItem: rPayload)
    {
        if (!rItem.first.isEmpty() && !rItem.second.isEmpty())
        {
            aPayload.append(", \"" + rItem.first + "\": \"" +
                    rItem.second).append('"');
        }
    }
    aPayload.append('}');

    const OString s = aPayload.makeStringAndClear();
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_WINDOW, s.getStr());
}

void SfxLokHelper::notifyInvalidation(SfxViewShell const* pThisView, tools::Rectangle const* pRect)
{
    if (DisableCallbacks::disabled())
        return;

    // -1 means all parts
    const int nPart = comphelper::LibreOfficeKit::isPartInInvalidation() ? pThisView->getPart() : INT_MIN;
    const int nMode = pThisView->getEditMode();
    pThisView->libreOfficeKitViewInvalidateTilesCallback(pRect, nPart, nMode);
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
            const int nMode = pThisView->getEditMode();
            pThisView->libreOfficeKitViewInvalidateTilesCallback(&aRectangle, i, nMode);
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
        {
            SfxLokHelper::notifyDocumentSizeChanged(pViewShell, "", pDoc, bInvalidateAll);
            bInvalidateAll = false; // we direct invalidations to all views anyway.
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyPartSizeChangedAllViews(vcl::ITiledRenderable* pDoc, int nPart)
{
    if (DisableCallbacks::disabled())
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->getPart() == nPart)
            SfxLokHelper::notifyDocumentSizeChanged(pViewShell, "", pDoc, false);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

OString SfxLokHelper::makeVisCursorInvalidation(int nViewId, const OString& rRectangle,
    bool bMispelledWord, const OString& rHyperlink)
{
    if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
    {
        OString sHyperlink = rHyperlink.isEmpty() ? "{}" : rHyperlink;
        return OString::Concat("{ \"viewId\": \"") + OString::number(nViewId) +
            "\", \"rectangle\": \"" + rRectangle +
            "\", \"mispelledWord\": \"" +  OString::number(bMispelledWord ? 1 : 0) +
            "\", \"hyperlink\": " + sHyperlink + " }";
    }
    else
    {
        return rRectangle;
    }
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

void SfxLokHelper::notifyContextChange(const css::ui::ContextChangeEventObject& rEvent)
{
    if (DisableCallbacks::disabled())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Get({ rEvent.Source, css::uno::UNO_QUERY });
    if (!pViewShell)
        return;

    OString aBuffer =
        OUStringToOString(rEvent.ApplicationName.replace(' ', '_'), RTL_TEXTENCODING_UTF8) +
        " " +
        OUStringToOString(rEvent.ContextName.replace(' ', '_'), RTL_TEXTENCODING_UTF8);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CONTEXT_CHANGED, aBuffer.getStr());
}

void SfxLokHelper::notifyUpdate(SfxViewShell const* pThisView, int nType)
{
    if (DisableCallbacks::disabled())
        return;

    pThisView->libreOfficeKitViewUpdatedCallback(nType);
}

void SfxLokHelper::notifyUpdatePerViewId(SfxViewShell const* pThisView, int nType)
{
    notifyUpdatePerViewId(pThisView, pThisView, pThisView, nType);
}

void SfxLokHelper::notifyUpdatePerViewId(SfxViewShell const* pTargetShell, SfxViewShell const* pViewShell,
    SfxViewShell const* pSourceShell, int nType)
{
    if (DisableCallbacks::disabled())
        return;

    int viewId = SfxLokHelper::getView(pViewShell);
    int sourceViewId = SfxLokHelper::getView(pSourceShell);
    pTargetShell->libreOfficeKitViewUpdatedCallbackPerViewId(nType, viewId, sourceViewId);
}

void SfxLokHelper::notifyOtherViewsUpdatePerViewId(SfxViewShell const* pThisView, int nType)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled())
        return;

    int viewId = SfxLokHelper::getView(pThisView);
    const ViewShellDocId nCurrentDocId = pThisView->GetDocId();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView && nCurrentDocId == pViewShell->GetDocId())
            pViewShell->libreOfficeKitViewUpdatedCallbackPerViewId(nType, viewId, viewId);

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
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
        std::unique_ptr<LOKAsyncEventData> pLOKEv(static_cast<LOKAsyncEventData*>(pEv));
        if (pLOKEv->mpWindow->isDisposed())
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

        if (pLOKEv->mpWindow->isDisposed())
            return;

        switch (pLOKEv->mnEvent)
        {
        case VclEventId::WindowKeyInput:
        {
            sal_uInt16 nRepeat = pLOKEv->maKeyEvent.GetRepeat();
            KeyEvent singlePress(pLOKEv->maKeyEvent.GetCharCode(),
                                 pLOKEv->maKeyEvent.GetKeyCode());
            for (sal_uInt16 i = 0; i <= nRepeat; ++i)
                if (!pFocusWindow->isDisposed())
                    pFocusWindow->KeyInput(singlePress);
            break;
        }
        case VclEventId::WindowKeyUp:
            if (!pFocusWindow->isDisposed())
                pFocusWindow->KeyUp(pLOKEv->maKeyEvent);
            break;
        case VclEventId::WindowMouseButtonDown:
            pLOKEv->mpWindow->SetLastMousePos(pLOKEv->maMouseEvent.GetPosPixel());
            pLOKEv->mpWindow->MouseButtonDown(pLOKEv->maMouseEvent);
            // Invoke the context menu
            if (pLOKEv->maMouseEvent.GetButtons() & MOUSE_RIGHT)
            {
                const CommandEvent aCEvt(pLOKEv->maMouseEvent.GetPosPixel(), CommandEventId::ContextMenu, true, nullptr);
                pLOKEv->mpWindow->Command(aCEvt);
            }
            break;
        case VclEventId::WindowMouseButtonUp:
            pLOKEv->mpWindow->SetLastMousePos(pLOKEv->maMouseEvent.GetPosPixel());
            pLOKEv->mpWindow->MouseButtonUp(pLOKEv->maMouseEvent);

            // sometimes MouseButtonDown captures mouse and starts tracking, and VCL
            // will not take care of releasing that with tiled rendering
            if (pLOKEv->mpWindow->IsTracking())
                pLOKEv->mpWindow->EndTracking();

            break;
        case VclEventId::WindowMouseMove:
            pLOKEv->mpWindow->SetLastMousePos(pLOKEv->maMouseEvent.GetPosPixel());
            pLOKEv->mpWindow->MouseMove(pLOKEv->maMouseEvent);
            break;
        case VclEventId::ExtTextInput:
        case VclEventId::EndExtTextInput:
            pLOKEv->mpWindow->PostExtTextInputEvent(pLOKEv->mnEvent, pLOKEv->maText);
            break;
        default:
            assert(false);
            break;
        }
    }

    void postEventAsync(LOKAsyncEventData *pEvent)
    {
        if (!pEvent->mpWindow || pEvent->mpWindow->isDisposed())
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

void SfxLokHelper::setBlockedCommandList(int nViewId, const char* blockedCommandList)
{
    SfxViewShell* pViewShell = SfxLokHelper::getViewOfId(nViewId);

    if(pViewShell)
    {
        pViewShell->setBlockedCommandList(blockedCommandList);
    }
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

void SfxLokHelper::dumpState(rtl::OStringBuffer &rState)
{
    SfxViewShell* pShell = SfxViewShell::Current();
    sal_Int32 nDocId = pShell ? static_cast<sal_Int32>(pShell->GetDocId().get()) : -1;

    rState.append("\n\tDocId:\t");
    rState.append(nDocId);

    if (nDocId < 0)
        return;

    rState.append("\n\tViewCount:\t");
    rState.append(static_cast<sal_Int32>(getViewsCount(nDocId)));

    const SfxViewShell* const pCurrentViewShell = SfxViewShell::Current();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pCurrentViewShell == nullptr || pViewShell->GetDocId() == pCurrentViewShell-> GetDocId())
            pViewShell->dumpLibreOfficeKitViewState(rState);

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyMediaUpdate(boost::property_tree::ptree& json)
{
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, json, /*pretty=*/ false);
    const std::string str = aStream.str();

    SfxLokHelper::notifyAllViews(LOK_CALLBACK_MEDIA_SHAPE, str.c_str());
}

bool SfxLokHelper::testInPlaceComponentMouseEventHit(SfxViewShell* pViewShell, int nType, int nX,
                                                     int nY, int nCount, int nButtons,
                                                     int nModifier, double fScaleX, double fScaleY,
                                                     bool bNegativeX)
{
    // In LOK RTL mode draw/svx operates in negative X coordinates
    // But the coordinates from client is always positive, so negate nX.
    if (bNegativeX)
        nX = -nX;

    // check if the user hit a chart/math object which is being edited by this view
    if (LokChartHelper aChartHelper(pViewShell, bNegativeX);
        aChartHelper.postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier, fScaleX, fScaleY))
        return true;

    if (LokStarMathHelper aMathHelper(pViewShell);
        aMathHelper.postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier, fScaleX, fScaleY))
        return true;

    // check if the user hit a chart which is being edited by someone else
    // and, if so, skip current mouse event
    if (nType != LOK_MOUSEEVENT_MOUSEMOVE)
    {
        if (LokChartHelper::HitAny({nX, nY}, bNegativeX))
            return true;
    }

    return false;
}

VclPtr<vcl::Window> SfxLokHelper::getInPlaceDocWindow(SfxViewShell* pViewShell)
{
    if (VclPtr<vcl::Window> pWindow = LokChartHelper(pViewShell).GetWindow())
        return pWindow;
    if (VclPtr<vcl::Window> pWindow = LokStarMathHelper(pViewShell).GetWidgetWindow())
        return pWindow;
    return {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
