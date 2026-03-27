/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string>
#include <string_view>
#include <list>
#include <mutex>

#include <sfx2/kit/componenthelpers.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/kit/unocommandlist.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/xml/crypto/XCertificateCreator.hpp>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/JsonToPropertyValues_with_boost.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/lok.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/window.hxx>
#include <sal/log.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <COKit/COKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/msgpool.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/base64.hxx>
#include <tools/json_writer.hxx>
#include <svl/cryptosign.hxx>
#include <tools/urlobj.hxx>

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
        return !comphelper::COKit::isActive() || m_nDisabled != 0;
    }

private:
    static int m_nDisabled;
};

int DisableCallbacks::m_nDisabled = 0;
}

namespace
{
LanguageTag g_defaultLanguageTag(u"en-US"_ustr, true);
LanguageTag g_loadLanguageTag(u"en-US"_ustr, true); //< The language used to load.
LOKDeviceFormFactor g_deviceFormFactor = LOKDeviceFormFactor::UNKNOWN;
bool g_isDefaultTimezoneSet = false;
OUString g_DefaultTimezone;
const std::size_t g_logNotifierCacheMaxSize = 50;
::std::list<::std::string> g_logNotifierCache;
}

#if !defined NDEBUG
static bool isSfxMediumMissingInteractionHandled(SfxViewFrame& rViewFrame)
{
    const SfxObjectShell* pObjSh = rViewFrame.GetObjectShell();
    const SfxMedium* pMed = pObjSh ? pObjSh->GetMedium() : nullptr;
    if (!pMed)
    {
        // In this unlikely case it's not going to matter.
        return false;
    }
    const SfxUnoAnyItem *pItem = pMed->GetItemSet().GetItemIfSet(SID_INTERACTIONHANDLER, false);
    return !pItem || !pItem->GetValue().hasValue();
}
#endif

int KitHelper::createView(SfxViewFrame& rViewFrame, ViewShellDocId docId)
{
    assert(docId >= ViewShellDocId(0) && "Cannot createView for invalid (negative) DocId.");

    // The XInteractionHandler installed during the original lo_documentLoad
    // should still be present when new views are created. If it is not
    // something has likely cleared the original SID_INTERACTIONHANDLER
    assert(!isSfxMediumMissingInteractionHandled(rViewFrame) && "original XInteractionHandler missing");

    comphelper::COKit::setDocId(docId);
    SfxRequest aRequest(rViewFrame, SID_NEWWINDOW);
    rViewFrame.ExecView_Impl(aRequest);
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell == nullptr)
        return -1;

    assert(pViewShell->GetDocId() == docId && "DocId must be already set!");
    return static_cast<sal_Int32>(pViewShell->GetViewShellId());
}

int KitHelper::createView()
{
    // Assumes a single document, or at least that the
    // current view belongs to the document on which the
    // view will be created.
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell == nullptr)
        return -1;

    return createView(pViewShell->GetViewFrame(), pViewShell->GetDocId());
}

std::unordered_map<OUString, css::uno::Reference<css::ui::XAcceleratorConfiguration>>& KitHelper::getAcceleratorConfs()
{
    return SfxApplication::GetOrCreate()->GetAcceleratorConfs_Impl();
}

int KitHelper::createView(int nDocId)
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

void KitHelper::setEditMode(int nMode, vcl::ITiledRenderable* pDoc)
{
    DisableCallbacks dc;
    pDoc->setEditMode(nMode);
}

void KitHelper::destroyView(int nId)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        pViewShell->SetLOKAccessibilityState(false);
        SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();
        SfxRequest aRequest(rViewFrame, SID_CLOSEWIN);
        rViewFrame.Exec_Impl(aRequest);
    }
}

bool KitHelper::isSettingView()
{
    return g_bSettingView;
}

void KitHelper::setView(int nId)
{
    g_bSettingView = true;
    comphelper::ScopeGuard g([] { g_bSettingView = false; });

    const SfxViewShell* pViewShell = getViewOfId(nId);
    if (!pViewShell)
        return;

    DisableCallbacks dc;

    bool bIsCurrShell = (pViewShell == SfxViewShell::Current());
    if (bIsCurrShell && comphelper::COKit::getLanguageTag().getBcp47() == pViewShell->GetLOKLanguageTag().getBcp47())
        return;

    if (bIsCurrShell)
    {
        // If we wanted to set the SfxViewShell that is actually set, we could skip it.
        // But it looks like that the language can go wrong, so we have to fix that.
        // This can happen, when someone sets the language or SfxViewShell::Current() separately.
        SAL_WARN("lok", "LANGUAGE mismatch at setView! ... old (wrong) lang:"
                        << comphelper::COKit::getLanguageTag().getBcp47()
                        << " new lang:" << pViewShell->GetLOKLanguageTag().getBcp47());
    }

    // update the current LOK language and locale for the dialog tunneling
    comphelper::COKit::setLanguageTag(pViewShell->GetLOKLanguageTag());
    comphelper::COKit::setLocale(pViewShell->GetLOKLocale());

    if (bIsCurrShell)
        return;

    SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();
    rViewFrame.MakeActive_Impl(false);

    // Make comphelper::dispatchCommand() find the correct frame.
    uno::Reference<frame::XFrame> xFrame = rViewFrame.GetFrame().GetFrameInterface();
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
    xDesktop->setActiveFrame(xFrame);
}

SfxViewShell* KitHelper::getViewOfId(int nId)
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

int KitHelper::getView(const SfxViewShell& rViewShell)
{
    return static_cast<sal_Int32>(rViewShell.GetViewShellId());
}

int KitHelper::getCurrentView()
{
    SfxViewShell* pViewShell = SfxViewShell::Current();
    // No valid view shell? Then no idea.
    if (!pViewShell)
        return -1;
    return KitHelper::getView(*pViewShell);
}

std::size_t KitHelper::getViewsCount(int nDocId)
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

// SfxApplication::SetViewFrame_Impl ensures that ViewShells
// are kept in MRU order
int KitHelper::getViewId(int nDocId)
{
    assert(nDocId != -1 && "Cannot getViewId for invalid DocId -1");

    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return -1;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId().get() == nDocId)
            return pViewShell->GetViewShellId().get();
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }

    return -1;
}

std::size_t KitHelper::getDocsCount()
{
    SfxApplication* pApp = SfxApplication::Get();
    if (!pApp)
        return 0;

    std::set<ViewShellDocId> aDocs;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        aDocs.insert(pViewShell->GetDocId());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }

    return aDocs.size();
}

bool KitHelper::getViewIds(int nDocId, int* pArray, size_t nSize)
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

int KitHelper::getDocumentIdOfView(int nViewId)
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

const LanguageTag & KitHelper::getDefaultLanguage()
{
    return g_defaultLanguageTag;
}

void KitHelper::setDefaultLanguage(const OUString& rBcp47LanguageTag)
{
    g_defaultLanguageTag = LanguageTag(rBcp47LanguageTag, true);
}

const LanguageTag& KitHelper::getLoadLanguage() { return g_loadLanguageTag; }

void KitHelper::setLoadLanguage(const OUString& rBcp47LanguageTag)
{
    g_loadLanguageTag = LanguageTag(rBcp47LanguageTag, true);
}

void KitHelper::setViewLanguage(int nId, const OUString& rBcp47LanguageTag)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        pViewShell->SetLOKLanguageTag(rBcp47LanguageTag);
        // sync also global getter if we are the current view
        bool bIsCurrShell = (pViewShell == SfxViewShell::Current());
        if (bIsCurrShell)
            comphelper::COKit::setLanguageTag(LanguageTag(rBcp47LanguageTag));
    }
}

void KitHelper::setViewLanguageAndLocale(int nId, const OUString& rBcp47LanguageTag)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            pViewShell->SetLOKLanguageAndLocale(rBcp47LanguageTag);
            // sync also global getter if we are the current view
            bool bIsCurrShell = (pViewShell == SfxViewShell::Current());
            if (bIsCurrShell)
            {
                comphelper::COKit::setLanguageTag(LanguageTag(rBcp47LanguageTag));
                comphelper::COKit::setLocale(LanguageTag(rBcp47LanguageTag));
            }
            return;
        }
    }
}

void KitHelper::setViewReadOnly(int nId, bool readOnly)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        LOK_INFO("lok.readonlyview", "KitHelper::setViewReadOnly: view id: " << nId << ", readOnly: " << readOnly);
        pViewShell->SetLokReadOnlyView(readOnly);
    }
}

void KitHelper::setAllowChangeComments(int nId, bool allow)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        LOK_INFO("lok.readonlyview", "KitHelper::setAllowChangeComments: view id: " << nId << ", allow: " << allow);
        pViewShell->SetAllowChangeComments(allow);
    }
}

void KitHelper::setAllowManageRedlines(int nId, bool allow)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        LOK_INFO("lok.readonlyview", "KitHelper::setAllowManageRedlines: view id: " << nId << ", allow: " << allow);
        pViewShell->SetAllowManageRedlines(allow);
    }
}

void KitHelper::setAccessibilityState(int nId, bool nEnabled)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        LOK_INFO("lok.a11y", "KitHelper::setAccessibilityState: view id: " << nId << ", nEnabled: " << nEnabled);
        pViewShell->SetLOKAccessibilityState(nEnabled);
    }
}

void KitHelper::setViewLocale(int nId, const OUString& rBcp47LanguageTag)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        pViewShell->SetLOKLocale(rBcp47LanguageTag);
        if (pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            // sync also global getter if we are the current view
            bool bIsCurrShell = (pViewShell == SfxViewShell::Current());
            if (bIsCurrShell)
            {
                comphelper::COKit::setLocale(LanguageTag(rBcp47LanguageTag));
            }
            return;
        }
    }
}

LOKDeviceFormFactor KitHelper::getDeviceFormFactor()
{
    return g_deviceFormFactor;
}

void KitHelper::setDeviceFormFactor(std::u16string_view rDeviceFormFactor)
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

void KitHelper::setColorPreviewState(int nId, bool nEnabled)
{
    std::vector<SfxViewShell*>& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (SfxViewShell* pViewShell : rViewArr)
    {
        if (pViewShell && pViewShell->GetViewShellId() == ViewShellId(nId))
        {
            pViewShell->SetLOKColorPreviewState(nEnabled);
            return;
        }
    }
}

void KitHelper::setDefaultTimezone(bool isSet, const OUString& rTimezone)
{
    g_isDefaultTimezoneSet = isSet;
    g_DefaultTimezone = rTimezone;
}

std::pair<bool, OUString> KitHelper::getDefaultTimezone()
{
    return { g_isDefaultTimezoneSet, g_DefaultTimezone };
}

void KitHelper::setViewTimezone(int nId, bool isSet, const OUString& rTimezone)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        pViewShell->SetLOKTimezone(isSet, rTimezone);
    }
}

std::pair<bool, OUString> KitHelper::getViewTimezone(int nId)
{
    if (SfxViewShell* pViewShell = getViewOfId(nId))
    {
        return pViewShell->GetLOKTimezone();
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

static OString lcl_generateJSON(const SfxViewShell& rView, const boost::property_tree::ptree& rTree)
{
    boost::property_tree::ptree aMessageProps = rTree;
    aMessageProps.put("viewId", KitHelper::getView(rView));
    aMessageProps.put("part", rView.getPart());
    aMessageProps.put("mode", rView.getEditMode());
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aMessageProps, false /* pretty */);
    return OString(o3tl::trim(aStream.str()));
}

static inline OString lcl_generateJSON(const SfxViewShell& rView, int nViewId, std::string_view rKey,
                                       const OString& rPayload)
{
    return OString::Concat("{ \"viewId\": \"") + OString::number(nViewId)
           + "\", \"part\": \"" + OString::number(rView.getPart()) + "\", \"mode\": \""
           + OString::number(rView.getEditMode()) + "\", \"" + rKey + "\": \""
           + lcl_sanitizeJSONAsValue(rPayload) + "\" }";
}

static inline OString lcl_generateJSON(const SfxViewShell& rView, std::string_view rKey,
                                       const OString& rPayload)
{
    return lcl_generateJSON(rView, KitHelper::getView(rView), rKey, rPayload);
}

void KitHelper::notifyOtherView(const SfxViewShell& rThisView, SfxViewShell const* pOtherView,
                                   int nType, std::string_view rKey, const OString& rPayload)
{
    if (DisableCallbacks::disabled())
        return;

    const OString aPayload = lcl_generateJSON(rThisView, rKey, rPayload);
    const int viewId = KitHelper::getView(rThisView);
    pOtherView->viewCallbackWithViewId(nType, aPayload, viewId);
}

void KitHelper::notifyOtherView(const SfxViewShell& rThisView, SfxViewShell const* pOtherView,
                                   int nType, const boost::property_tree::ptree& rTree)
{
    if (DisableCallbacks::disabled() || !pOtherView)
        return;

    const int viewId = KitHelper::getView(rThisView);
    pOtherView->viewCallbackWithViewId(nType, lcl_generateJSON(rThisView, rTree), viewId);
}

void KitHelper::notifyOtherViews(const SfxViewShell* pThisView, int nType, std::string_view rKey,
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
                aPayload = lcl_generateJSON(*pThisView, rKey, rPayload);
                viewId = KitHelper::getView(*pThisView);
            }

            pViewShell->viewCallbackWithViewId(nType, aPayload, viewId);
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void KitHelper::notifyOtherViews(const SfxViewShell* pThisView, int nType,
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
                aPayload = lcl_generateJSON(*pThisView, rTree);
                viewId = KitHelper::getView(*pThisView);
            }

            pViewShell->viewCallbackWithViewId(nType, aPayload, viewId);
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

OString KitHelper::makePayloadJSON(const SfxViewShell* pThisView, int nViewId, std::string_view rKey, const OString& rPayload)
{
    return lcl_generateJSON(*pThisView, nViewId, rKey, rPayload);
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

        return u""_ustr;
    }
}

void KitHelper::sendUnoStatus(const SfxViewShell* pShell, const SfxPoolItem* pItem)
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
        pShell->viewCallback(LOK_CALLBACK_STATE_CHANGED, OString(aStream.str()));
    }
}

void KitHelper::notifyViewRenderState(const SfxViewShell* pShell, vcl::ITiledRenderable* pDoc)
{
    pShell->viewCallback(LOK_CALLBACK_VIEW_RENDER_STATE, pDoc->getViewRenderState());
}

void KitHelper::notifyWindow(const SfxViewShell* pThisView,
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
    pThisView->viewCallback(LOK_CALLBACK_WINDOW, s);
}

void KitHelper::notifyCursorInvalidation(SfxViewShell const* pThisView, tools::Rectangle const* pRect, bool bControlEvent, int windowID)
{
    int nViewId = KitHelper::getView(*pThisView);
    OString sPayload = OString::Concat("{ \"viewId\": \"") + OString::number(nViewId) + "\", \"rectangle\": \"" + pRect->toString();
    if (bControlEvent)
    {
        sPayload += "\", \"controlEvent\": true, \"windowId\": \"" + OString::number(windowID) + "\"";
    }
    sPayload += " }";
    pThisView->viewCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, sPayload);
}

void KitHelper::notifyInvalidation(SfxViewShell const* pThisView, tools::Rectangle const* pRect)
{
    // -1 means all parts
    const int nPart = comphelper::COKit::isPartInInvalidation() ? pThisView->getPart() : INT_MIN;
    KitHelper::notifyInvalidation(pThisView, nPart, pRect);
}

void KitHelper::notifyInvalidation(SfxViewShell const* pThisView, const int nInPart, tools::Rectangle const* pRect)
{
    if (DisableCallbacks::disabled())
        return;

    // -1 means all parts
    const int nPart = comphelper::COKit::isPartInInvalidation() ? nInPart : INT_MIN;
    const int nMode = pThisView->getEditMode();
    pThisView->viewInvalidateTilesCallback(pRect, nPart, nMode);
}

void KitHelper::notifyDocumentSizeChanged(SfxViewShell const* pThisView, const OString& rPayload, vcl::ITiledRenderable* pDoc, bool bInvalidateAll)
{
    if (!pDoc || pDoc->isDisposed() || DisableCallbacks::disabled())
        return;

    if (bInvalidateAll)
    {
        for (int i = 0; i < pDoc->getParts(); ++i)
        {
            tools::Rectangle aRectangle(0, 0, 1000000000, 1000000000);
            const int nMode = pThisView->getEditMode();
            pThisView->viewInvalidateTilesCallback(&aRectangle, i, nMode);
        }
    }
    pThisView->viewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, rPayload);
}

void KitHelper::notifyDocumentSizeChangedAllViews(vcl::ITiledRenderable* pDoc, bool bInvalidateAll)
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
            KitHelper::notifyDocumentSizeChanged(pViewShell, ""_ostr, pDoc, bInvalidateAll);
            bInvalidateAll = false; // we direct invalidations to all views anyway.
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void KitHelper::notifyCurrentPageSizeChangedAllViews(vcl::ITiledRenderable *pDoc)
{
    if (!pDoc || pDoc->isDisposed() || DisableCallbacks::disabled())
        return;

    const SfxViewShell* const pCurrentViewShell = SfxViewShell::Current();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pCurrentViewShell == nullptr || pViewShell->GetDocId() == pCurrentViewShell->GetDocId())
        {
            OString aPayload = ".uno:CurrentPageResize"_ostr;
            pViewShell->viewCallback(LOK_CALLBACK_STATE_CHANGED, aPayload);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void KitHelper::notifyPartSizeChangedAllViews(vcl::ITiledRenderable* pDoc, int nPart)
{
    if (DisableCallbacks::disabled())
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (// FIXME should really filter on pViewShell->GetDocId() too
            pViewShell->getPart() == nPart)
            KitHelper::notifyDocumentSizeChanged(pViewShell, ""_ostr, pDoc, false);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

OString KitHelper::makeVisCursorInvalidation(int nViewId, const OString& rRectangle,
    bool bMispelledWord, const OString& rHyperlink)
{
    if (comphelper::COKit::isViewIdForVisCursorInvalidation())
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

void KitHelper::notifyAllViews(int nType, const OString& rPayload)
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
            pViewShell->viewCallback(nType, payload);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void KitHelper::notifyContextChange(const css::ui::ContextChangeEventObject& rEvent)
{
    if (DisableCallbacks::disabled())
        return;

    SfxViewShell* pViewShell = SfxViewShell::Get({ rEvent.Source, css::uno::UNO_QUERY });
    if (!pViewShell)
    {
        // Source may be an embedded controller (e.g. ChartController) that
        // is not an SfxViewShell itself. Fall back to the current view shell
        // so that context changes from embedded editors reach LOKit clients.
        pViewShell = SfxViewShell::Current();
    }
    if (!pViewShell)
        return;

    OUString aBuffer =
        rEvent.ApplicationName.replace(' ', '_') +
        " " +
        rEvent.ContextName.replace(' ', '_');
    pViewShell->viewCallback(LOK_CALLBACK_CONTEXT_CHANGED, aBuffer.toUtf8());
}

void KitHelper::notifyLog(const std::ostringstream& stream)
{
    if (DisableCallbacks::disabled())
       return;

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
       return;
    if (pViewShell->getCOKitViewCallback())
    {
        if (!g_logNotifierCache.empty())
        {
            for (const auto& msg : g_logNotifierCache)
            {
                pViewShell->viewCallback(LOK_CALLBACK_CORE_LOG, msg.c_str());
            }
            g_logNotifierCache.clear();
        }
        pViewShell->viewCallback(LOK_CALLBACK_CORE_LOG, stream.str().c_str());
    }
    else
    {
        while (g_logNotifierCache.size() >= g_logNotifierCacheMaxSize)
            g_logNotifierCache.pop_front();
        g_logNotifierCache.push_back(stream.str());
    }
}

namespace
{
std::string extractCertificateWithOffset(const std::string& certificate, size_t& rOffset)
{
    static constexpr std::string_view header("-----BEGIN CERTIFICATE-----");
    static constexpr std::string_view footer("-----END CERTIFICATE-----");

    std::string result;

    size_t pos1 = certificate.find(header, rOffset);
    if (pos1 == std::string::npos)
        return result;

    size_t pos2 = certificate.find(footer, pos1 + 1);
    if (pos2 == std::string::npos)
        return result;

    pos1 = pos1 + header.length();
    size_t len = pos2 - pos1;

    rOffset = pos2;
    return certificate.substr(pos1, len);
}
}

std::string KitHelper::extractCertificate(const std::string & certificate)
{
    size_t nOffset = 0;
    return extractCertificateWithOffset(certificate, nOffset);
}

std::vector<std::string> KitHelper::extractCertificates(const std::string& rCerts)
{
    std::vector<std::string> aRet;
    size_t nOffset = 0;
    while (true)
    {
        std::string aNext = extractCertificateWithOffset(rCerts, nOffset);
        if (aNext.empty())
        {
            break;
        }

        aRet.push_back(std::move(aNext));
    }
    return aRet;
}

namespace
{
std::string extractKey(const std::string & privateKey)
{
    static constexpr std::string_view header("-----BEGIN PRIVATE KEY-----");
    static constexpr std::string_view footer("-----END PRIVATE KEY-----");

    std::string result;

    size_t pos1 = privateKey.find(header);
    if (pos1 == std::string::npos)
        return result;

    size_t pos2 = privateKey.find(footer, pos1 + 1);
    if (pos2 == std::string::npos)
        return result;

    pos1 = pos1 + header.length();
    pos2 = pos2 - pos1;

    return privateKey.substr(pos1, pos2);
}
}

css::uno::Reference<css::security::XCertificate> KitHelper::getSigningCertificate(const std::string& rCert, const std::string& rKey)
{
    const uno::Reference<uno::XComponentContext>& xContext = comphelper::getProcessComponentContext();
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    if (!xSecurityContext.is())
    {
        return {};
    }

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Reference<xml::crypto::XCertificateCreator> xCertificateCreator(xSecurityEnvironment, uno::UNO_QUERY);

    if (!xCertificateCreator.is())
    {
        return {};
    }

    uno::Sequence<sal_Int8> aCertificateSequence;

    std::string aCertificateBase64String = extractCertificate(rCert);
    if (!aCertificateBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String);
        comphelper::Base64::decode(aCertificateSequence, aBase64OUString);
    }
    else
    {
        aCertificateSequence.realloc(rCert.size());
        std::copy(rCert.c_str(), rCert.c_str() + rCert.size(), aCertificateSequence.getArray());
    }

    uno::Sequence<sal_Int8> aPrivateKeySequence;
    std::string aPrivateKeyBase64String = extractKey(rKey);
    if (!aPrivateKeyBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aPrivateKeyBase64String);
        comphelper::Base64::decode(aPrivateKeySequence, aBase64OUString);
    }
    else
    {
        aPrivateKeySequence.realloc(rKey.size());
        std::copy(rKey.c_str(), rKey.c_str() + rKey.size(), aPrivateKeySequence.getArray());
    }

    uno::Reference<security::XCertificate> xCertificate = xCertificateCreator->createDERCertificateWithPrivateKey(aCertificateSequence, aPrivateKeySequence);
    return xCertificate;
}

uno::Reference<security::XCertificate> KitHelper::addCertificate(
    const css::uno::Reference<css::xml::crypto::XCertificateCreator>& xCertificateCreator,
    const css::uno::Sequence<sal_Int8>& rCert)
{
    // Trust arg is handled by CERT_DecodeTrustString(), see 'man certutil'.
    return xCertificateCreator->addDERCertificateToTheDatabase(rCert, u"TCu,Cu,Tu"_ustr);
}

void KitHelper::addCertificates(const std::vector<std::string>& rCerts)
{
    const uno::Reference<uno::XComponentContext>& xContext = comphelper::getProcessComponentContext();
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    if (!xSecurityContext.is())
    {
        return;
    }

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Reference<xml::crypto::XCertificateCreator> xCertificateCreator(xSecurityEnvironment, uno::UNO_QUERY);
    if (!xCertificateCreator.is())
    {
        return;
    }

    for (const auto& rCert : rCerts)
    {
        uno::Sequence<sal_Int8> aCertificateSequence;
        OUString aBase64OUString = OUString::fromUtf8(rCert);
        comphelper::Base64::decode(aCertificateSequence, aBase64OUString);
        addCertificate(xCertificateCreator, aCertificateSequence);
    }

    // Update the signature state, perhaps the signing certificate is now trusted.
    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    if (!pObjectShell)
    {
        return;
    }

    pObjectShell->RecheckSignature(false);
}

bool KitHelper::supportsCommand(std::u16string_view rCommand)
{
    static const std::initializer_list<std::u16string_view> vSupport = { u"Signature" };

    return std::find(vSupport.begin(), vSupport.end(), rCommand) != vSupport.end();
}

std::map<OUString, OUString> KitHelper::parseCommandParameters(std::u16string_view rCommand)
{
    std::map<OUString, OUString> aMap;

    INetURLObject aParser(rCommand);
    OUString aArguments = aParser.GetParam();
    sal_Int32 nParamIndex = 0;
    do
    {
        std::u16string_view aParam = o3tl::getToken(aArguments, 0, '&', nParamIndex);
        sal_Int32 nIndex = 0;
        OUString aKey;
        OUString aValue;
        do
        {
            std::u16string_view aToken = o3tl::getToken(aParam, 0, '=', nIndex);
            if (aKey.isEmpty())
                aKey = aToken;
            else
                aValue = aToken;
        } while (nIndex >= 0);
        aMap[aKey] = INetURLObject::decode(aValue, INetURLObject::DecodeMechanism::WithCharset);
    } while (nParamIndex >= 0);

    return aMap;
}

void KitHelper::getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand)
{
    static constexpr OString aSignature(".uno:Signature"_ostr);
    if (!o3tl::starts_with(rCommand, aSignature))
    {
        return;
    }

    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    if (!pObjectShell)
    {
        return;
    }

    svl::crypto::SigningContext aSigningContext;
    std::map<OUString, OUString> aMap
        = KitHelper::parseCommandParameters(OUString::fromUtf8(rCommand));
    auto it = aMap.find("signatureTime");
    if (it != aMap.end())
    {
        // Signature time is provided: prefer it over the system time.
        aSigningContext.m_nSignatureTime = it->second.toInt64();
    }
    pObjectShell->SignDocumentContentUsingCertificate(aSigningContext);
    // Set commandName, this is a reply to a request.
    rJsonWriter.put("commandName", aSignature);
    auto aCommandValues = rJsonWriter.startNode("commandValues");
    rJsonWriter.put("signatureTime", aSigningContext.m_nSignatureTime);

    uno::Sequence<sal_Int8> aDigest(reinterpret_cast<sal_Int8*>(aSigningContext.m_aDigest.data()),
                                    aSigningContext.m_aDigest.size());
    OUStringBuffer aBuffer;
    comphelper::Base64::encode(aBuffer, aDigest);
    rJsonWriter.put("digest", aBuffer.makeStringAndClear());
}

void KitHelper::notifyUpdate(SfxViewShell const* pThisView, int nType)
{
    if (DisableCallbacks::disabled() || !pThisView)
        return;

    pThisView->viewUpdatedCallback(nType);
}

void KitHelper::notifyUpdatePerViewId(SfxViewShell const& rThisView, int nType)
{
    notifyUpdatePerViewId(rThisView, &rThisView, rThisView, nType);
}

void KitHelper::notifyUpdatePerViewId(SfxViewShell const& rTargetShell, SfxViewShell const* pViewShell,
    SfxViewShell const& rSourceShell, int nType)
{
    if (DisableCallbacks::disabled())
        return;

    // This getCurrentView() is dubious
    SAL_WARN_IF(!pViewShell, "lok", "no explicit viewshell set");
    int viewId = pViewShell ? KitHelper::getView(*pViewShell) : KitHelper::getCurrentView();
    int sourceViewId = KitHelper::getView(rSourceShell);
    rTargetShell.viewUpdatedCallbackPerViewId(nType, viewId, sourceViewId);
}

void KitHelper::notifyOtherViewsUpdatePerViewId(SfxViewShell const* pThisView, int nType)
{
    if (DisableCallbacks::disabled() || !pThisView)
        return;

    int viewId = KitHelper::getView(*pThisView);
    const ViewShellDocId nCurrentDocId = pThisView->GetDocId();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell != pThisView && nCurrentDocId == pViewShell->GetDocId())
            pViewShell->viewUpdatedCallbackPerViewId(nType, viewId, viewId);

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void KitHelper::registerViewCallbacks()
{
    comphelper::COKit::setViewSetter([](int nView) {
        KitHelper::setView(nView);
    });
    comphelper::COKit::setViewGetter([]() -> int {
        return KitHelper::getCurrentView();
    });
}

void KitHelper::dispatchUnoCommand(const boost::property_tree::ptree& tree)
{
    auto command = OStringToOUString(tree.get_child("name").get_value<std::string>(),
                                     RTL_TEXTENCODING_UTF8);
    // Check if the uno command is allowed
    if (std::u16string_view rest;
        !command.startsWith(".uno:", &rest) || !GetKitUnoCommandList().contains(rest))
    {
        LOK_WARN("lok.transform",
                 "UnoCommand command is not recognized: '" << command << "'");
        return;
    }
    std::vector<beans::PropertyValue> arguments;
    if (auto args = tree.get_child_optional("arguments"))
    {
        arguments = comphelper::JsonToPropertyValues(*args);
    }
    // Make the uno command synchron
    arguments.push_back(comphelper::makePropertyValue(u"SynchronMode"_ustr, true));

    comphelper::dispatchCommand(command, comphelper::containerToSequence(arguments));
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

        int nView = KitHelper::getCurrentView();
        if (nView != pLOKEv->mnView)
        {
            SAL_INFO("sfx.view", "LOK - view mismatch " << nView << " vs. " << pLOKEv->mnView);
            KitHelper::setView(pLOKEv->mnView);
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

            if (pLOKEv->maKeyEvent.GetKeyCode().GetCode() == KEY_CONTEXTMENU)
            {
                // later do use getCaretPosition probably, or get focused obj position, smt like that
                Point aPos = pFocusWindow->GetPointerPosPixel();
                CommandEvent aCEvt( aPos, CommandEventId::ContextMenu);
                pFocusWindow->Command(aCEvt);
            }
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

        pEvent->mnView = KitHelper::getCurrentView();
        if (vcl::lok::isUnipoll())
        {
            if (!Application::IsMainThread())
                SAL_WARN("lok", "Posting event directly but not called from main thread!");
            LOKPostAsyncEvent(pEvent, nullptr);
        }
        else
            Application::PostUserEvent(LINK_NONMEMBER(pEvent, LOKPostAsyncEvent));
    }
}

void KitHelper::postKeyEventAsync(const VclPtr<vcl::Window> &xWindow,
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

void KitHelper::setBlockedCommandList(int nViewId, const char* blockedCommandList)
{
    SfxViewShell* pViewShell = KitHelper::getViewOfId(nViewId);

    if(pViewShell)
    {
        pViewShell->setBlockedCommandList(blockedCommandList);
    }
}

void KitHelper::postExtTextEventAsync(const VclPtr<vcl::Window> &xWindow,
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

void KitHelper::postMouseEventAsync(const VclPtr<vcl::Window> &xWindow, LokMouseEventData const & rLokMouseEventData)
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

void KitHelper::dumpState(rtl::OStringBuffer &rState)
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
            pViewShell->dumpCOKitViewState(rState);

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

bool KitHelper::testInPlaceComponentMouseEventHit(SfxViewShell* pViewShell, int nType, int nX,
                                                     int nY, int nCount, int nButtons,
                                                     int nModifier, double fScaleX, double fScaleY,
                                                     bool bNegativeX)
{
    // In LOK RTL mode draw/svx operates in negative X coordinates
    // But the coordinates from client is always positive, so negate nX.
    if (bNegativeX)
        nX = -nX;

    // check if the user hit a chart/math object which is being edited by this view
    if (KitChartHelper aChartHelper(pViewShell, bNegativeX);
        aChartHelper.postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier, fScaleX, fScaleY))
        return true;

    if (KitStarMathHelper aMathHelper(pViewShell);
        aMathHelper.postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier, fScaleX, fScaleY))
        return true;

    // check if the user hit a chart which is being edited by someone else
    // and, if so, skip current mouse event
    if (nType != LOK_MOUSEEVENT_MOUSEMOVE)
    {
        if (KitChartHelper::HitAny({nX, nY}, bNegativeX))
            return true;
    }

    return false;
}

VclPtr<vcl::Window> KitHelper::getInPlaceDocWindow(SfxViewShell* pViewShell)
{
    if (VclPtr<vcl::Window> pWindow = KitChartHelper(pViewShell).GetWindow())
        return pWindow;
    if (VclPtr<vcl::Window> pWindow = KitStarMathHelper(pViewShell).GetWidgetWindow())
        return pWindow;
    return {};
}

void KitHelper::sendNetworkAccessError(std::string_view rAction)
{
    tools::JsonWriter aWriter;
    aWriter.put("code", static_cast<sal_uInt32>(
        ErrCode(ErrCodeArea::Inet, sal_uInt16(ErrCodeClass::Access))));
    aWriter.put("kind", "network");
    aWriter.put("cmd", rAction);

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell)
    {
        pViewShell->viewCallback(
            LOK_CALLBACK_ERROR, aWriter.finishAndGetAsOString());
    }
}

SfxLokLanguageGuard::SfxLokLanguageGuard(const SfxViewShell* pNewShell)
    : m_bSetLanguage(false)
    , m_pOldShell(nullptr)
{
    m_pOldShell = SfxViewShell::Current();
    if (!comphelper::COKit::isActive() || !pNewShell || pNewShell == m_pOldShell)
    {
        return;
    }

    // The current view ID is not the one that belongs to this frame, update
    // language/locale.
    comphelper::COKit::setLanguageTag(pNewShell->GetLOKLanguageTag());
    comphelper::COKit::setLocale(pNewShell->GetLOKLocale());
    m_bSetLanguage = true;
}

SfxLokLanguageGuard::~SfxLokLanguageGuard()
{
    if (!m_bSetLanguage || !m_pOldShell)
    {
        return;
    }

    comphelper::COKit::setLanguageTag(m_pOldShell->GetLOKLanguageTag());
    comphelper::COKit::setLocale(m_pOldShell->GetLOKLocale());
}

LOKEditViewHistory::EditViewHistoryMap LOKEditViewHistory::maEditViewHistory;


void LOKEditViewHistory::Update(bool bRemove)
{
    if (!comphelper::COKit::isActive())
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
