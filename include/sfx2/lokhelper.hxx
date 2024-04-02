/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_LOKHELPER_HXX
#define INCLUDED_SFX2_LOKHELPER_HXX

#include <vcl/IDialogRenderable.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/event.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/viewsh.hxx>
#include <tools/gen.hxx>
#include <cstddef>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <list>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <sfx2/app.hxx>

#define LOK_NOTIFY_LOG_TO_CLIENT 1

#define LOK_LOG_STREAM(level, area, stream) \
    do { \
            ::std::ostringstream lok_detail_stream; \
            lok_detail_stream << level << ":"; \
            if (std::strcmp(level, "debug") != 0) \
                lok_detail_stream << area << ":"; \
            lok_detail_stream << SAL_WHERE << stream; \
            SfxLokHelper::notifyLog(lok_detail_stream); \
        } while (false)

#if LOK_NOTIFY_LOG_TO_CLIENT > 0
#define LOK_INFO(area, stream) \
    LOK_LOG_STREAM("info", area, stream) \

#define LOK_WARN(area, stream) \
    LOK_LOG_STREAM("warn", area, stream)

#define LOK_DBG(stream) \
    LOK_LOG_STREAM("debug", "", stream)
#else
#define LOK_INFO(area, stream) \
    SAL_INFO(area, stream) \

#define LOK_WARN(area, stream) \
    SAL_WARN(area, stream)

#define LOK_DBG(stream) \
    SAL_DEBUG(stream)
#endif

struct SFX2_DLLPUBLIC LokMouseEventData
{
    int mnType;
    Point maPosition;
    int mnCount;
    MouseEventModifiers meModifiers;
    int mnButtons;
    int mnModifier;
    std::optional<Point> maLogicPosition;

    LokMouseEventData(int nType, Point aPosition, int nCount, MouseEventModifiers eModifiers, int nButtons, int nModifier)
        : mnType(nType)
        , maPosition(aPosition)
        , mnCount(nCount)
        , meModifiers(eModifiers)
        , mnButtons(nButtons)
        , mnModifier(nModifier)
    {}
};

#include <boost/property_tree/ptree_fwd.hpp>

namespace com::sun::star::ui { struct ContextChangeEventObject; };

class SFX2_DLLPUBLIC SfxLokHelper
{
public:
    /// Gets the short cut accelerators.
    static std::unordered_map<OUString, css::uno::Reference<com::sun::star::ui::XAcceleratorConfiguration>>& getAcceleratorConfs();
    /// Create a new view shell from the current view frame.
    /// This assumes a single document is ever loaded.
    static int createView();
    /// Create a new view shell for the given DocId, for multi-document support.
    static int createView(int nDocId);
    /// Destroy a view shell from the global shell list.
    static void destroyView(int nId);
    /// Set a view shell as current one.
    static void setView(int nId);
    /// Determines if a call to setView() is in progress or not.
    static bool isSettingView();
    /// Set the edit mode for a document with callbacks disabled.
    static void setEditMode(int nMode, vcl::ITiledRenderable* pDoc);
    /// Get view shell with id
    static SfxViewShell* getViewOfId(int nId);
    /// Get the currently active view.
    static int getView(const SfxViewShell* pViewShell = nullptr);
    /// Get the number of views of the current DocId.
    static std::size_t getViewsCount(int nDocId);
    /// Get viewIds of views of the current DocId.
    static bool getViewIds(int nDocId, int* pArray, size_t nSize);
    /// Set View Blocked for some uno commands
    static void setBlockedCommandList(int nViewId, const char* blockedCommandList);
    /// Get the document id for a view
    static int getDocumentIdOfView(int nViewId);
    /// Get the default language that should be used for views
    static const LanguageTag & getDefaultLanguage();
    /// Set language of the given view.
    static void setViewLanguage(int nId, const OUString& rBcp47LanguageTag);
    /// Set the default language for views.
    static void setDefaultLanguage(const OUString& rBcp47LanguageTag);
    /// Enable/Disable AT support for the given view.
    static void setAccessibilityState(int nId, bool nEnabled);
    /// Get the language used by the loading view (used for all save operations).
    static const LanguageTag & getLoadLanguage();
    /// Set the language used by the loading view (used for all save operations).
    static void setLoadLanguage(const OUString& rBcp47LanguageTag);
    /// Set the locale for the given view.
    static void setViewLocale(int nId, const OUString& rBcp47LanguageTag);
    /// Get the device form factor that should be used for a new view.
    static LOKDeviceFormFactor getDeviceFormFactor();
    /// Set the device form factor that should be used for a new view.
    static void setDeviceFormFactor(std::u16string_view rDeviceFormFactor);

    /// Set timezone of the given view.
    /// @isSet true to use @rTimezone, even if it's empty. Otherwise, no timezone.
    /// @rTimezone the value to set (which could be empty).
    static void setDefaultTimezone(bool isSet, const OUString& rTimezone);
    /// Get timezone of the given view. See @setDefaultTimezone.
    static std::pair<bool, OUString> getDefaultTimezone();
    /// Set the timezone of the given view.
    static void setViewTimezone(int nId, bool isSet, const OUString& rTimezone);
    /// Get the timezone of the given view.
    static std::pair<bool, OUString> getViewTimezone(int nId);

    /// Iterate over any view shell, except pThisViewShell, passing it to the f function.
    template<typename ViewShellType, typename FunctionType>
    static void forEachOtherView(ViewShellType* pThisViewShell, FunctionType f);

    /// Invoke the LOK callback of all other views showing the same document as pThisView, with a payload of rKey-rPayload.
    static void notifyOtherViews(const SfxViewShell* pThisView, int nType, std::string_view rKey,
                                 const OString& rPayload);
    /// Invoke the LOK callback of all views except pThisView, with a JSON payload created from the given property tree.
    static void notifyOtherViews(const SfxViewShell* pThisView, int nType,
                                 const boost::property_tree::ptree& rTree);
    /// Same as notifyOtherViews(), but works on a selected "other" view, not on all of them.
    static void notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                int nType, std::string_view rKey, const OString& rPayload);
    /// Same as notifyOtherViews(), the property-tree version, but works on a selected "other" view, not on all of them.
    static void notifyOtherView(const SfxViewShell* pThisView, SfxViewShell const* pOtherView,
                                int nType, const boost::property_tree::ptree& rTree);

    /// Emits a LOK_CALLBACK_STATE_CHANGED
    static void sendUnoStatus(const SfxViewShell* pShell, const SfxPoolItem* pItem);
    /// Emits a LOK_CALLBACK_WINDOW
    static void notifyWindow(const SfxViewShell* pThisView,
                             vcl::LOKWindowId nWindowId,
                             std::u16string_view rAction,
                             const std::vector<vcl::LOKPayloadItem>& rPayload = std::vector<vcl::LOKPayloadItem>());
    /// Emits a LOK_CALLBACK_DOCUMENT_SIZE_CHANGED - if @bInvalidateAll - first invalidates all parts
    static void notifyDocumentSizeChanged(SfxViewShell const* pThisView, const OString& rPayload, vcl::ITiledRenderable* pDoc, bool bInvalidateAll = true);
    /// Emits a LOK_CALLBACK_DOCUMENT_SIZE_CHANGED for all views of the same document - if @bInvalidateAll - first invalidates all parts
    static void notifyDocumentSizeChangedAllViews(vcl::ITiledRenderable* pDoc, bool bInvalidateAll = true);
    /// Emits a LOK_CALLBACK_DOCUMENT_SIZE_CHANGED for all views of the same document with the same part
    static void notifyPartSizeChangedAllViews(vcl::ITiledRenderable* pDoc, int nPart);
    /// Emits a LOK_CALLBACK_INVALIDATE_TILES, but tweaks it according to setOptionalFeatures() if needed.
    static void notifyInvalidation(SfxViewShell const* pThisView, int nPart, tools::Rectangle const *);
    /// Emits a LOK_CALLBACK_INVALIDATE_TILES, but tweaks it according to setOptionalFeatures() if needed
    /// uses the Part reported by pThisView
    static void notifyInvalidation(SfxViewShell const* pThisView, tools::Rectangle const *);
    /// Notifies all views with the given type and payload.
    static void notifyAllViews(int nType, const OString& rPayload);

    /// Notify about the editing context change.
    static void notifyContextChange(const css::ui::ContextChangeEventObject& rEvent);

    /// Emits an LOK_CALLBACK_VIEW_RENDER_STATE
    static void notifyViewRenderState(SfxViewShell const* pViewShell, vcl::ITiledRenderable* pDoc);

    // Notify about the given type needing an update.
    static void notifyUpdate(SfxViewShell const* pViewShell, int nType);
    // Notify about the given type needing a per-viewid update.
    static void notifyUpdatePerViewId(SfxViewShell const* pViewShell, int nType);
    /// Same as notifyUpdatePerViewId(), pTargetShell will be notified, relevant viewId in pViewShell,
    /// pSourceView->getLOKPayload() will be called to get the data.
    static void notifyUpdatePerViewId(SfxViewShell const* pTargetShell, SfxViewShell const* pViewShell,
        SfxViewShell const* pSourceShell, int nType);
    // Notify other views about the given type needing a per-viewid update.
    static void notifyOtherViewsUpdatePerViewId(SfxViewShell const* pViewShell, int nType);

    static OString makePayloadJSON(const SfxViewShell* pThisView, int nViewId, std::string_view rKey, const OString& rPayload);
    /// Makes a LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR payload, but tweaks it according to setOptionalFeatures() if needed.
    static OString makeVisCursorInvalidation(int nViewId, const OString& rRectangle,
                                             bool bMispelledWord = false, const OString& rHyperlink = "");

    /// Helper for posting async key event
    static void postKeyEventAsync(const VclPtr<vcl::Window> &xWindow,
                                  int nType, int nCharCode, int nKeyCode, int nRepeat = 0);

    /// Helper for posting input event
    static void postExtTextEventAsync(const VclPtr<vcl::Window> &xWindow,
                                      int nType, const OUString &rText);

    /// Helper for posting async mouse event
    static void postMouseEventAsync(const VclPtr<vcl::Window> &xWindow, LokMouseEventData const & rLokMouseEventData);

    /// A special value to signify 'infinity'.
    /// This value is chosen such that sal_Int32 will not overflow when manipulated.
    static const tools::Long MaxTwips = 1e9;

    /// Helper for diagnosing run-time problems
    static void dumpState(rtl::OStringBuffer &rState);

    /// Process the mouse event in the currently active in-place component (if any).
    /// Returns true if the event has been processed, and no further processing is necessary.
    static bool testInPlaceComponentMouseEventHit(SfxViewShell* pViewShell, int nType, int nX,
                                                  int nY, int nCount, int nButtons, int nModifier,
                                                  double fScaleX, double fScaleY,
                                                  bool bNegativeX = false);

    static VclPtr<vcl::Window> getInPlaceDocWindow(SfxViewShell* pViewShell);

    /// Sends Network Access error to LOK
    static void sendNetworkAccessError(std::string_view rAction);

    static void notifyLog(const std::ostringstream& stream);

private:
    static int createView(SfxViewFrame* pViewFrame, ViewShellDocId docId);
};

template<typename ViewShellType, typename FunctionType>
void SfxLokHelper::forEachOtherView(ViewShellType* pThisViewShell, FunctionType f)
{
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        auto pOtherViewShell = dynamic_cast<ViewShellType*>(pViewShell);
        if (pOtherViewShell != nullptr && pOtherViewShell != pThisViewShell && pOtherViewShell->GetDocId() == pThisViewShell->GetDocId())
        {
            f(pOtherViewShell);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

/// If LOK is active, switch to the language/locale of the provided shell and back on delete.
class SfxLokLanguageGuard
{
    bool m_bSetLanguage;
    const SfxViewShell* m_pOldShell;
    const SfxViewShell* m_pNewShell;

public:
    SfxLokLanguageGuard(SfxViewShell* pNewShell);
    ~SfxLokLanguageGuard();
};

typedef std::list<SfxViewShell*> ViewShellList;

/// Used to keep track of the last N views that text edited a document through an EditView
class SFX2_DLLPUBLIC LOKEditViewHistory
{
public:
    typedef std::list<SfxViewShell*> ViewShellList;
    typedef std::unordered_map<int, ViewShellList> EditViewHistoryMap;

    static void Update(bool bRemove = false);
    static ViewShellList GetHistoryForDoc(ViewShellDocId aDocId);
    static ViewShellList GetSortedViewsForDoc(ViewShellDocId aDocId);
private:
    static EditViewHistoryMap maEditViewHistory;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
