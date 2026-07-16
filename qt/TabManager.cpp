/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "TabManager.hpp"
#include "TabShellBridge.hpp"
#include "TabbedWindow.hpp"
#include "WebView.hpp"

#include <QPoint>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.hxx>
#include <Poco/File.h>
#include <Poco/Path.h>

#include <qt/qt.hpp>
#include <common/Log.hpp>
#include <common/SettingsStorage.hpp>

#include <Poco/URI.h>

#include <QApplication>
#include <QCursor>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStackedWidget>
#include <QString>
#include <QUrl>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QWidget>

#include <algorithm>
#include <utility>

namespace
{
constexpr int kShellHeight = 36;

// Which theme fresh tab strips start in: the saved dark mode choice wins,
// otherwise the system theme, like the document views. Updated on SETDARKMODE.
QString& currentTheme()
{
    static QString theme =
        Desktop::getDarkMode().value_or(portalPrefersDark().value_or(false))
            ? QStringLiteral("dark")
            : QStringLiteral("light");
    return theme;
}

// A release can outrun the strip's acceptance and end as leave + cancel
// instead of a drop; such a leave precedes the dragend by mere message
// latency, while a real drag-away is human-scale older.
constexpr qint64 kReleaseUnwindGraceMs = 250;

// The strip page only makes tabs draggable, so the source view identifies a
// tab drag. The text/x-coda-tab type is invisible to QMimeData: Chromium
// pickles custom types into chromium/x-web-custom-data.
bool isTabStripDrag(const QDropEvent* ev)
{
    QObject* src = ev->source();
    if (!src)
        return false;
    for (TabbedWindow* w : TabbedWindow::allWindows())
    {
        QWidget* strip = w->manager()->shellWidget();
        for (QObject* o = src; o; o = o->parent())
            if (o == strip)
                return true;
    }
    return false;
}
} // namespace

TabManager::TabManager(TabbedWindow* window, QWebEngineProfile* profile)
    : QObject(window)
    , _window(window)
    , _profile(profile)
    , _stack(new QStackedWidget(window))
    , _shellView(new QWebEngineView(window))
    , _shellBridge(new TabShellBridge(this))
{
    _shellView->setFixedHeight(kShellHeight);
    // setPage() destroys the default page, so install ours before wiring the channel.
    QWebEnginePage* page = new QWebEnginePage(profile, _shellView);
    _shellView->setPage(page);
    _shellView->setContextMenuPolicy(Qt::NoContextMenu);

    QWebChannel* channel = new QWebChannel(page);
    channel->registerObject(QStringLiteral("tabBridge"), _shellBridge);
    page->setWebChannel(channel);

    // Same dist folder as cool.html, so the strip shares its color palette.
    Poco::Path stripHtmlPath(getDataDir());
    stripHtmlPath.append("/browser/dist/qtapp-tabstrip.html");
    Poco::URI stripUrl(stripHtmlPath);
    stripUrl.setScheme("file");
    stripUrl.addQueryParameter("darkTheme",
                               currentTheme() == QStringLiteral("dark") ? "true" : "false");
    _shellView->load(QUrl(QString::fromStdString(stripUrl.toString())));

    _shellView->setFocusPolicy(Qt::NoFocus);
    // The strip's render widget (its focus proxy) is created lazily by Chromium
    // and can be recreated, so watch the view's child events and (re)attach.
    _shellView->installEventFilter(this);
    attachStripFocusFilter();

    // Hidden until there are 2+ tabs; see emitTabsChangedNow().
    _shellView->hide();
}

TabManager::~TabManager()
{
    // Unparent the views before the unique_ptrs delete them, so Qt's child
    // destruction does not race the deletes. End presentations first so their
    // console/full-screen windows are not left dangling around a deleted view.
    for (auto& entry : _tabs)
    {
        entry.webView->endPresentation();
        _stack->removeWidget(entry.webView->webEngineView());
    }
}

QWidget* TabManager::shellWidget() const { return _shellView; }

void TabManager::attachStripFocusFilter()
{
    QWidget* proxy = _shellView->focusProxy();
    if (!proxy || proxy == _stripProxy)
        return;
    if (_stripProxy)
        _stripProxy->removeEventFilter(this);
    _stripProxy = proxy;
    proxy->installEventFilter(this);
}

void TabManager::focusActiveDocument()
{
    if (QWidget* w = _stack->currentWidget())
        w->setFocus(Qt::OtherFocusReason);
}

bool TabManager::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == _shellView &&
        (ev->type() == QEvent::ChildAdded || ev->type() == QEvent::ChildPolished))
    {
        attachStripFocusFilter();
    }
    else if (obj == _stripProxy && ev->type() == QEvent::FocusIn)
    {
        // Let the strip's focus change settle, then hand focus back to the
        // document - doing it synchronously here gets undone.
        QMetaObject::invokeMethod(this, &TabManager::focusActiveDocument,
                                  Qt::QueuedConnection);
    }
    else if ((obj == _shellView || obj == _stripProxy) &&
             (ev->type() == QEvent::DragEnter || ev->type() == QEvent::DragMove))
    {
        // The != this guard keeps re-registration on moves from clobbering
        // the slot the page reported (see onTargetDragOver).
        if (s_dragHoverTarget != this && isTabStripDrag(static_cast<const QDropEvent*>(ev)))
        {
            s_dragHoverTarget = this;
            s_dragHoverInsertAt = tabCount(); // append until the page reports a slot
            s_lastHoverTarget = nullptr;
        }
    }
    else if ((obj == _shellView || obj == _stripProxy) && ev->type() == QEvent::DragLeave)
    {
        // Only the native leave can end the hover: QDragLeaveEvent has no
        // position, so the page cannot tell this exit from moving between
        // tabs. Demote rather than forget - a dragend right behind this
        // leave is an unwound release (see kReleaseUnwindGraceMs).
        if (s_dragHoverTarget == this)
        {
            s_dragHoverTarget = nullptr;
            s_lastHoverTarget = this;
            s_lastHoverInsertAt = s_dragHoverInsertAt;
            s_lastHoverLeftAt.start();
            emit _shellBridge->dragExited();
        }
    }
    return QObject::eventFilter(obj, ev);
}

WebView* TabManager::webViewForTab(int tabId) const
{
    auto it = findTab(tabId);
    return it == _tabs.end() ? nullptr : it->webView.get();
}

int TabManager::tabIdForWebView(WebView* wv) const
{
    auto it = std::find_if(_tabs.begin(), _tabs.end(),
                           [wv](const Entry& e) { return e.webView.get() == wv; });
    return it == _tabs.end() ? -1 : it->id;
}

std::vector<TabManager::Entry>::iterator TabManager::findTab(int tabId)
{
    return std::find_if(_tabs.begin(), _tabs.end(),
                        [tabId](const Entry& e) { return e.id == tabId; });
}

std::vector<TabManager::Entry>::const_iterator TabManager::findTab(int tabId) const
{
    return std::find_if(_tabs.begin(), _tabs.end(),
                        [tabId](const Entry& e) { return e.id == tabId; });
}

int TabManager::s_nextTabId = 1;
QPointer<TabManager> TabManager::s_dragHoverTarget;
int TabManager::s_dragHoverInsertAt = -1;
QPointer<TabManager> TabManager::s_lastHoverTarget;
int TabManager::s_lastHoverInsertAt = -1;
QElapsedTimer TabManager::s_lastHoverLeftAt;

int TabManager::registerTab(std::unique_ptr<WebView> wv, int insertAt)
{
    const int size = static_cast<int>(_tabs.size());
    if (insertAt < 0 || insertAt > size)
        insertAt = size;

    // A new or adopted tab means this window is not going away; abandon any
    // close-everything chain before it sweeps the new tab up.
    _closingAll = false;

    const int id = s_nextTabId++;
    WebView* raw = wv.get();
    _tabs.insert(_tabs.begin() + insertAt, Entry{ id, std::move(wv) });
    _stack->insertWidget(insertAt, raw->webEngineView());

    raw->setMainWindow(_window);
    raw->setOnCloseRequest([this, raw]() {
        int tid = tabIdForWebView(raw);
        if (tid >= 0)
            closeTab(tid);
    });
    raw->setOnTitleChange([this, raw](const QString& title) { onWebViewTitleChanged(raw, title); });

    activateTab(id);
    return id;
}

std::unique_ptr<WebView> TabManager::detachAt(std::vector<Entry>::iterator it)
{
    if (it == _tabs.end())
        return nullptr;
    // A close parked on this tab's save (see closeTab) holds an override
    // cursor and a callback that would close the tab in whichever window
    // ends up hosting it; release both, the tab is leaving another way.
    if (it->closeWaitsForSave)
    {
        it->closeWaitsForSave = false;
        it->webView->onSaveComplete(nullptr);
        QApplication::restoreOverrideCursor();
    }
    it->webView->endPresentation();
    const int detachedIndex = static_cast<int>(std::distance(_tabs.begin(), it));
    const int detachedId = it->id;
    std::unique_ptr<WebView> wv = std::move(it->webView);
    _tabs.erase(it);

    _stack->removeWidget(wv->webEngineView());
    wv->setOnCloseRequest({});
    wv->setOnTitleChange({});

    // activateTab() below already emits; only emit here if it did not run.
    bool reactivated = false;
    if (detachedId == _activeTabId)
    {
        _activeTabId = -1;
        if (!_tabs.empty())
        {
            int newIdx = std::min(detachedIndex, static_cast<int>(_tabs.size()) - 1);
            activateTab(_tabs[newIdx].id);
            reactivated = true;
        }
    }

    if (!reactivated)
        emitTabsChangedNow();
    return wv;
}

int TabManager::addStarterTab()
{
    auto wv = std::make_unique<WebView>(_profile);
    WebView* raw = wv.get();
    int id = registerTab(std::move(wv), -1);
    raw->load(Poco::URI(), /*newFile*/ false, /*isStarterMode*/ true);
    return id;
}

int TabManager::addDocumentTab(const Poco::URI& fileURL, bool newFile, bool requiresSaveAs)
{
    auto wv = std::make_unique<WebView>(_profile);
    WebView* raw = wv.get();
    int id = registerTab(std::move(wv), -1);
    raw->load(fileURL, newFile, /*isStarterMode*/ false, requiresSaveAs);
    return id;
}

int TabManager::adoptTab(WebView* webView, int insertAt)
{
    return registerTab(std::unique_ptr<WebView>(webView), insertAt);
}

int TabManager::adoptFromOtherWindow(int srcTabId, int insertAt)
{
    for (TabbedWindow* w : TabbedWindow::allWindows())
    {
        TabManager* src = w->manager();
        if (src == this || !src->webViewForTab(srcTabId))
            continue;

        if (auto wv = src->releaseTab(srcTabId))
        {
            int id = adoptTab(wv.release(), insertAt);
            _window->raise();
            _window->activateWindow();
            return id;
        }
        return -1;
    }
    return -1;
}

void TabManager::onTargetDragOver(int insertAt)
{
    // This report can trail the native DragLeave that already ended the
    // hover; never let it re-register a strip the drag has left.
    if (s_dragHoverTarget == this)
        s_dragHoverInsertAt = insertAt;
}

std::unique_ptr<WebView> TabManager::releaseTab(int tabId)
{
    auto wv = detachAt(findTab(tabId));
    if (_tabs.empty())
        emit requestWindowClose();
    return wv;
}

void TabManager::closeTab(int tabId)
{
    auto it = findTab(tabId);
    if (it == _tabs.end())
        return;

    // The close-everything chain only ever closes the current front tab, so a
    // close aimed anywhere else is direct user interaction; abandon the chain
    // rather than resume it when this close completes.
    if (_closingAll && it != _tabs.begin())
        _closingAll = false;

    WebView* wv = it->webView.get();

    // A ready-to-close tab skips both round-trips below: its page-JS may be
    // terminating already (BYE/EXIT_TEST), and a stale save-in-flight flag
    // would park this close on a SAVECOMPLETED that never arrives.
    if (!wv->isReadyToClose())
    {
        // The tab was closed while a save is still round-tripping: wait for
        // the save and then close, rather than asking about "unsaved changes"
        // that are already being saved.
        if (wv->isSaveInFlight())
        {
            // A close is already parked on this save; re-parking would push
            // a second override cursor that nothing ever pops.
            if (it->closeWaitsForSave)
                return;
            it->closeWaitsForSave = true;
            LOG_TRC("TabManager::closeTab: save in flight, deferring close");
            // Busy cursor for immediate feedback: the in-page busy modal
            // paints with a delay and may never appear on a fast finish.
            QApplication::setOverrideCursor(Qt::WaitCursor);
            wv->evalJS(
                "if (window.app && window.app.map)"
                "  window.app.map.fire('showbusy',"
                "    {label: window._('Saving...')});");
            QPointer<TabManager> self = this;
            wv->onSaveComplete([self, tabId]() {
                QApplication::restoreOverrideCursor();
                if (!self)
                    return;
                auto entry = self->findTab(tabId);
                if (entry == self->_tabs.end())
                    return;
                entry->closeWaitsForSave = false;
                WebView* w = entry->webView.get();
                // A re-entry that re-prompts (save failed, doc still
                // modified) must not leave the busy modal hanging behind the
                // prompt.
                w->evalJS(
                    "if (window.app && window.app.map)"
                    "  window.app.map.fire('hidebusy');");
                // The save we waited for is the one for this close; skip the
                // redundant save-if-dirty round-trip on the re-entry.
                w->markReadyToClose();
                self->closeTab(tabId);
            });
            return;
        }

        // Hand off to JS to save if dirty and post CLOSE_WINDOW on
        // completion; its handler trips _readyToClose and re-enters here.
        // No-op for a clean doc or one with no bridge.
        LOG_TRC("TabManager::closeTab: save-if-dirty round-trip");
        wv->saveAndClose();
        return;
    }

    // Announce the orderly close to the per-document collab broker (no-op for
    // local-only docs) before the WebView is destroyed, so it can reach the
    // bridge's _document while it is still alive.
    wv->sendCollabBye();
    detachAt(it);
    if (_tabs.empty())
        emit requestWindowClose();
    else if (_closingAll)
        closeTab(_tabs.front().id);
}

void TabManager::requestCloseAll()
{
    if (_tabs.empty())
    {
        emit requestWindowClose();
        return;
    }
    _closingAll = true;
    closeTab(_tabs.front().id);
}

void TabManager::activateTab(int tabId)
{
    auto it = findTab(tabId);
    if (it == _tabs.end())
        return;
    _activeTabId = tabId;
    WebView* wv = it->webView.get();
    _stack->setCurrentWidget(wv->webEngineView());
    _window->setWindowTitle(wv->composedWindowTitle());
    emitTabsChangedNow();
    focusActiveDocument();
}

void TabManager::reorderTab(int fromIndex, int toIndex)
{
    const int size = static_cast<int>(_tabs.size());
    if (fromIndex < 0 || fromIndex >= size || toIndex < 0 || toIndex >= size ||
        fromIndex == toIndex)
        return;

    // A presenting tab's view lives in the full-screen window, not the stack;
    // end the presentation so we reparent the real view.
    _tabs[fromIndex].webView->endPresentation();

    Entry e = std::move(_tabs[fromIndex]);
    QWebEngineView* view = e.webView->webEngineView();
    const int activeId = _activeTabId;
    const int movedId = e.id;
    _tabs.erase(_tabs.begin() + fromIndex);
    _tabs.insert(_tabs.begin() + toIndex, std::move(e));

    _stack->removeWidget(view);
    _stack->insertWidget(toIndex, view);
    if (activeId == movedId)
        _stack->setCurrentWidget(view);

    emitTabsChangedNow();
    focusActiveDocument();
}

void TabManager::onWebViewTitleChanged(WebView* wv, const QString& title)
{
    int id = tabIdForWebView(wv);
    if (id < 0)
        return;
    if (id == _activeTabId)
        _window->setWindowTitle(title);
    emitTabsChangedNow();
}

void TabManager::onWindowActiveChanged(bool active)
{
    for (const Entry& e : _tabs)
        e.webView->onWindowActiveChanged(active);
}

QString TabManager::currentTabsJson() const
{
    QJsonArray arr;
    for (const auto& e : _tabs)
    {
        QJsonObject obj;
        obj["id"] = e.id;
        obj["title"] = e.webView->documentTitle();
        obj["docType"] = e.webView->documentType();
        obj["modified"] = e.webView->isDocumentModified();
        obj["active"] = (e.id == _activeTabId);
        arr.append(obj);
    }
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void TabManager::emitTabsChangedNow()
{
    _shellView->setVisible(_tabs.size() > 1);
    emit _shellBridge->tabsChanged(currentTabsJson());
}

void TabManager::emitInitialState()
{
    emitTabsChangedNow();
    emit _shellBridge->themeChanged(currentTheme());
}

void TabManager::applyTheme(const QString& theme)
{
    currentTheme() =
        (theme == QStringLiteral("dark")) ? QStringLiteral("dark") : QStringLiteral("light");
    emit _shellBridge->themeChanged(currentTheme());
}

void TabManager::onSourceDragStarted(int tabId)
{
    // A tabsChanged rebuild can destroy the drag source mid-drag and Chromium
    // then never fires its dragend, so the previous drag's hover state can
    // still be set here.
    LOG_TRC("TabManager::onSourceDragStarted: tab " << tabId << ", clearing hover state");
    s_dragHoverTarget = nullptr;
    s_dragHoverInsertAt = -1;
    s_lastHoverTarget = nullptr;
    s_lastHoverInsertAt = -1;
}

void TabManager::onSourceDragEnded(int tabId, bool inStripDropHandled)
{
    // Snapshot and clear the hover state so the next drag starts clean.
    TabManager* target = s_dragHoverTarget.data();
    int insertAt = s_dragHoverInsertAt;
    if (!target && s_lastHoverTarget && !s_lastHoverLeftAt.hasExpired(kReleaseUnwindGraceMs))
    {
        target = s_lastHoverTarget.data();
        insertAt = s_lastHoverInsertAt;
    }
    s_dragHoverTarget = nullptr;
    s_lastHoverTarget = nullptr;

    // A target strip's drop, or our own, already handled the move.
    if (findTab(tabId) == _tabs.end() || inStripDropHandled)
        return;

    // The strip's drop never reached its page; complete the move here.
    if (target && target != this)
    {
        target->adoptFromOtherWindow(tabId, insertAt);
        return;
    }
    // A dropless release on our own strip: keep the current order.
    if (target == this)
        return;

    // Released away from every strip: detach into a new window at the cursor.
    // Wayland compositors may ignore the requested position.
    QPoint cursorPos = QCursor::pos();
    TabbedWindow* fresh = new TabbedWindow(_profile);
    fresh->move(cursorPos.x() - fresh->size().width() / 2, cursorPos.y() - 18);
    fresh->show();
    if (auto wv = releaseTab(tabId))
        fresh->manager()->adoptTab(wv.release(), /*insertAt*/ -1);
    else
        fresh->close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
