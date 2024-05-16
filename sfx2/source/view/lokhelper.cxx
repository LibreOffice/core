/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string>
#include <string_view>
#include <list>

#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/lokhelper.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>

#include <comphelper/processfactory.hxx>
#include <o3tl/string_view.hxx>
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
#include <comphelper/scopeguard.hxx>
#include <tools/json_writer.hxx>

#include <boost/property_tree/json_parser.hpp>

using namespace com::sun::star;

namespace {
bool g_bSettingView(false);

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
const std::size_t g_logNotifierCacheMaxSize = 50;
::std::list<::std::string> g_logNotifierCache;
}

int SfxLokHelper::createView(SfxViewFrame& rViewFrame, ViewShellDocId docId)
{
    assert(docId >= ViewShellDocId(0) && "Cannot createView for invalid (negative) DocId.");

    SfxViewShell::SetCurrentDocId(docId);
    SfxRequest aRequest(rViewFrame, SID_NEWWINDOW);
    rViewFrame.ExecView_Impl(aRequest);
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

std::unordered_map<OUString, css::uno::Reference<com::sun::star::ui::XAcceleratorConfiguration>>& SfxLokHelper::getAcceleratorConfs()
{
    return SfxApplication::GetOrCreate()->GetAcceleratorConfs_Impl();
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

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == nViewShellId)
        {
            pViewShell->SetLOKAccessibilityState(false);
            SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();
            SfxRequest aRequest(rViewFrame, SID_CLOSEWIN);
            rViewFrame.Exec_Impl(aRequest);
            break;
        }
    }
}

bool SfxLokHelper::isSettingView()
{
    return g_bSettingView;
}

void SfxLokHelper::setView(int nId)
{
    g_bSettingView = true;
    comphelper::ScopeGuard g([] { g_bSettingView = false; });

    SfxApplication* pApp = SfxApplication::Get();
    if (pApp == nullptr)
        return;

    const ViewShellId nViewShellId(nId);
    std::vector<SfxViewShell*>& rViewArr = pApp->GetViewShells_Impl();

    const auto itViewShell = std::find_if(rViewArr.begin(), rViewArr.end(), [nViewShellId](SfxViewShell* pViewShell){ return pViewShell->GetViewShellId() == nViewShellId; });
    if (itViewShell == rViewArr.end())
        return;

    const SfxViewShell* pViewShell = *itViewShell;
    DisableCallbacks dc;

    bool bIsCurrShell = (pViewShell == SfxViewShell::Current());
    if (bIsCurrShell && comphelper::LibreOfficeKit::getLanguageTag().getBcp47() == pViewShell->GetLOKLanguageTag().getBcp47())
        return;

    if (bIsCurrShell)
    {
        // If we wanted to set the SfxViewShell that is actually set, we could skip it.
        // But it looks like that the language can go wrong, so we have to fix that.
        // This can happen, when someone sets the language or SfxViewShell::Current() separately.
        SAL_WARN("lok", "LANGUAGE mismatch at setView! ... old (wrong) lang:"
                        << comphelper::LibreOfficeKit::getLanguageTag().getBcp47()
                        << " new lang:" << pViewShell->GetLOKLanguageTag().getBcp47());
    }

    // update the current LOK language and locale for the dialog tunneling
    comphelper::LibreOfficeKit::setLanguageTag(pViewShell->GetLOKLanguageTag());
    comphelper::LibreOfficeKit::setLocale(pViewShell->GetLOKLocale());

    if (bIsCurrShell)
        return;

    SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();
    rViewFrame.MakeActive_Impl(false);

    // Make comphelper::dispatchCommand() find the correct frame.
    uno::Reference<frame::XFrame> xFrame = rViewFrame.GetFrame().GetFrameInterface();
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
    xDesktop->setActiveFrame(xFrame);
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

void SfxLokHelper::setViewReadOnly(int nId, bool readOnly)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell && pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            LOK_INFO("lok.readonlyview", "SfxLokHelper::setViewReadOnly: view id: " << nId << ", readOnly: " << readOnly);
            pViewShell->SetLokReadOnlyView(readOnly);
            return;
        }
    }
}

void SfxLokHelper::setAllowChangeComments(int nId, bool allow)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell && pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            LOK_INFO("lok.readonlyview", "SfxLokHelper::setAllowChangeComments: view id: " << nId << ", allow: " << allow);
            pViewShell->SetAllowChangeComments(allow);
            return;
        }
    }
}

void SfxLokHelper::setAccessibilityState(int nId, bool nEnabled)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell && pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            LOK_INFO("lok.a11y", "SfxLokHelper::setAccessibilityState: view id: " << nId << ", nEnabled: " << nEnabled);
            pViewShell->SetLOKAccessibilityState(nEnabled);
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
    return OString(o3tl::trim(aStream.str()));
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
    pOtherView->libreOfficeKitViewCallbackWithViewId(nType, aPayload, viewId);
}

void SfxLokHelper::notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                   int nType, const boost::property_tree::ptree& rTree)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (DisableCallbacks::disabled() || !pOtherView)
        return;

    const int viewId = SfxLokHelper::getView(pThisView);
    pOtherView->libreOfficeKitViewCallbackWithViewId(nType, lcl_generateJSON(pThisView, rTree), viewId);
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

            pViewShell->libreOfficeKitViewCallbackWithViewId(nType, aPayload, viewId);
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxLokHelper::notifyOtherViews(const SfxViewShell* pThisView, int nType,
                                    const boost::property_tree::ptree& rTree)
{
    assert(pThisView != nullptr && "pThisView must be valid");
    if (!pThisView || DisableCallbacks::disabled())
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

            pViewShell->libreOfficeKitViewCallbackWithViewId(nType, aPayload, viewId);
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
                if (!pSlot->GetUnoName().isEmpty())
                {
                    return pSlot->GetCommand();
                }
            }
        }

        return "";
    }
}

void SfxLokHelper::sendUnoStatus(const SfxViewShell* pShell, const SfxPoolItem* pItem)
{
    if (!pShell || !pItem || IsInvalidItem(pItem) || DisableCallbacks::disabled())
        return;

    boost::property_tree::ptree aItem = pItem->dumpAsJSON();

    if (aItem.count("state"))
    {
        OUString sCommand = lcl_getNameForSlot(pShell, pItem->Which());
        if (!sCommand.isEmpty())
            aItem.put("commandName", sCommand);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aItem);
        pShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, OString(aStream.str()));
    }
}

void SfxLokHelper::notifyViewRenderState(const SfxViewShell* pShell, vcl::ITiledRenderable* pDoc)
{
    pShell->libreOfficeKitViewCallback(LOK_CALLBACK_VIEW_RENDER_STATE, pDoc->getViewRenderState());
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
            auto aFirst = rItem.first.replaceAll("\""_ostr, "\\\""_ostr);
            auto aSecond = rItem.second.replaceAll("\""_ostr, "\\\""_ostr);
            aPayload.append(", \"" + aFirst + "\": \"" + aSecond + "\"");
        }
    }
    aPayload.append('}');

    const OString s = aPayload.makeStringAndClear();
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_WINDOW, s);
}

void SfxLokHelper::notifyInvalidation(SfxViewShell const* pThisView, tools::Rectangle const* pRect)
{
    // -1 means all parts
    const int nPart = comphelper::LibreOfficeKit::isPartInInvalidation() ? pThisView->getPart() : INT_MIN;
    SfxLokHelper::notifyInvalidation(pThisView, nPart, pRect);
}

void SfxLokHelper::notifyInvalidation(SfxViewShell const* pThisView, const int nInPart, tools::Rectangle const* pRect)
{
    if (DisableCallbacks::disabled())
        return;

    // -1 means all parts
    const int nPart = comphelper::LibreOfficeKit::isPartInInvalidation() ? nInPart : INT_MIN;
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
    pThisView->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, rPayload);
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
            SfxLokHelper::notifyDocumentSizeChanged(pViewShell, ""_ostr, pDoc, bInvalidateAll);
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
        if (// FIXME should really filter on pViewShell->GetDocId() too
            pViewShell->getPart() == nPart)
            SfxLokHelper::notifyDocumentSizeChanged(pViewShell, ""_ostr, pDoc, false);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

OString SfxLokHelper::makeVisCursorInvalidation(int nViewId, const OString& rRectangle,
    bool bMispelledWord, const OString& rHyperlink)
{
    if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
    {
        OString sHyperlink = rHyperlink.isEmpty() ? "{}"_ostr : rHyperlink;
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
    if (!pCurrentViewShell)
        return;
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

    OUString aBuffer =
        rEvent.ApplicationName.replace(' ', '_') +
        " " +
        rEvent.ContextName.replace(' ', '_');
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CONTEXT_CHANGED, aBuffer.toUtf8());
}

void SfxLokHelper::notifyLog(const std::ostringstream& stream)
{
    if (DisableCallbacks::disabled())
       return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
       return;
    if (pViewShell->getLibreOfficeKitViewCallback())
    {
        if (!g_logNotifierCache.empty())
        {
            for (const auto& msg : g_logNotifierCache)
            {
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CORE_LOG, msg.c_str());
            }
            g_logNotifierCache.clear();
        }
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CORE_LOG, stream.str().c_str());
    }
    else
    {
        while (g_logNotifierCache.size() >= g_logNotifierCacheMaxSize)
            g_logNotifierCache.pop_front();
        g_logNotifierCache.push_back(stream.str());
    }
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
            pLOKEv->mpWindow->RequestHelp(HelpEvent{
                pLOKEv->mpWindow->OutputToScreenPixel(pLOKEv->maMouseEvent.GetPosPixel()),
                HelpEventMode::QUICK }); // If needed, HelpEventMode should be taken from a config
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

void SfxLokHelper::sendNetworkAccessError(std::string_view rAction)
{
    tools::JsonWriter aWriter;
    aWriter.put("code", static_cast<sal_uInt32>(
        ErrCode(ErrCodeArea::Inet, sal_uInt16(ErrCodeClass::Access))));
    aWriter.put("kind", "network");
    aWriter.put("cmd", rAction);

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(
            LOK_CALLBACK_ERROR, aWriter.finishAndGetAsOString());
    }
}

SfxLokLanguageGuard::SfxLokLanguageGuard(SfxViewShell* pNewShell)
    : m_bSetLanguage(false)
    , m_pOldShell(nullptr)
    , m_pNewShell(pNewShell)
{
    m_pOldShell = SfxViewShell::Current();
    if (!comphelper::LibreOfficeKit::isActive() || !m_pNewShell || m_pNewShell == m_pOldShell)
    {
        return;
    }

    // The current view ID is not the one that belongs to this frame, update
    // language/locale.
    comphelper::LibreOfficeKit::setLanguageTag(m_pNewShell->GetLOKLanguageTag());
    comphelper::LibreOfficeKit::setLocale(m_pNewShell->GetLOKLocale());
    m_bSetLanguage = true;
}

SfxLokLanguageGuard::~SfxLokLanguageGuard()
{
    if (!m_bSetLanguage || !m_pOldShell)
    {
        return;
    }

    comphelper::LibreOfficeKit::setLanguageTag(m_pOldShell->GetLOKLanguageTag());
    comphelper::LibreOfficeKit::setLocale(m_pOldShell->GetLOKLocale());
}

LOKEditViewHistory::EditViewHistoryMap LOKEditViewHistory::maEditViewHistory;


void LOKEditViewHistory::Update(bool bRemove)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell)
    {
        int nDocId = pViewShell->GetDocId().get();
        if (maEditViewHistory.find(nDocId) != maEditViewHistory.end())
            maEditViewHistory[nDocId].remove(pViewShell);
        if (!bRemove)
        {
            maEditViewHistory[nDocId].push_back(pViewShell);
            if (maEditViewHistory[nDocId].size() > 10)
                maEditViewHistory[nDocId].pop_front();
        }
    }
}

ViewShellList LOKEditViewHistory::GetHistoryForDoc(ViewShellDocId aDocId)
{
    int nDocId = aDocId.get();
    ViewShellList aResult;
    if (maEditViewHistory.find(nDocId) != maEditViewHistory.end())
        aResult = maEditViewHistory.at(nDocId);
    return aResult;
}

 ViewShellList LOKEditViewHistory::GetSortedViewsForDoc(ViewShellDocId aDocId)
 {
     ViewShellList aEditViewHistoryForDoc = LOKEditViewHistory::GetHistoryForDoc(aDocId);
     // all views where document is loaded
     ViewShellList aCurrentDocViewList;
     // active views that are listed in the edit history
     ViewShellList aEditedViewList;

     // Populate aCurrentDocViewList and aEditedViewList
     SfxViewShell* pViewShell = SfxViewShell::GetFirst();
     while (pViewShell)
     {
         if (pViewShell->GetDocId() == aDocId)
         {
             if (aEditViewHistoryForDoc.empty() ||
                 std::find(aEditViewHistoryForDoc.begin(), aEditViewHistoryForDoc.end(),
                           pViewShell) == aEditViewHistoryForDoc.end())
             {
                 // append views not listed in the edit history;
                 // the edit history is limited to 10 views,
                 // so it could miss some view where in place editing is occurring
                 aCurrentDocViewList.push_back(pViewShell);
             }
             else
             {
                 // view is listed in the edit history
                 aEditedViewList.push_back(pViewShell);
             }
         }
         pViewShell = SfxViewShell::GetNext(*pViewShell);
     }

     // in case some no more active view needs to be removed from the history
     aEditViewHistoryForDoc.remove_if(
         [&aEditedViewList](SfxViewShell* pHistoryItem) {
             return std::find(aEditedViewList.begin(), aEditedViewList.end(), pHistoryItem) == aEditedViewList.end();
         });

     // place views belonging to the edit history at the end
     aCurrentDocViewList.splice(aCurrentDocViewList.end(), aEditViewHistoryForDoc);

     return aCurrentDocViewList;
 }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
