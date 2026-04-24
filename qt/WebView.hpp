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

#pragma once

#include "Document.hpp"

#include <Poco/URI.h>

#include <QMainWindow>
#include <QObject>
#include <QWebEngineView>

#include <memory>

class Bridge;
class WebView;

class CODAWebEngineView : public QWebEngineView
{
public:
    CODAWebEngineView(QMainWindow* parent)
        : QWebEngineView(parent)
        , _mainWindow(parent)
        , _presenterConsole(nullptr)
    {
    }

    ~CODAWebEngineView();

    void arrangePresentationWindows();
    void exchangeMonitors();

    void createPresentationFS();
    void destroyPresentationFS();

private:
    QMainWindow* _mainWindow;
    // Given the general inability of wayland based environments
    // to restore a window's position, especially after moving
    // it to another monitor full-screen, use a throwaway window
    // for full-screen presentations, and leave the pre-full-screen
    // on the original screen for reuse post-presentation
    std::unique_ptr<QMainWindow> _presenterFSWindow;
    WebView* _presenterConsole;
    QMetaObject::Connection _screenAdded;
    QMetaObject::Connection _screenRemoved;

    QWebEngineView* createWindow(QWebEnginePage::WebWindowType type) override;
};

class WebView
{
public:
    explicit WebView(QWebEngineProfile* profile, bool isWelcome = false, QMainWindow* parentWindow = nullptr);
    ~WebView();
    CODAWebEngineView* webEngineView() { return _webView.get(); }
    QMainWindow* mainWindow() { return _mainWindow; }

    void load(const Poco::URI& fileURL = Poco::URI(), bool newFile = false, bool isStarterMode = false);

    // templatePath and basename can be empty strings and are optional.
    static WebView* createNewDocument(QWebEngineProfile* profile, const std::string& templateType,
                                      const std::string& templatePath, const std::string& baseName);

    static WebView* findOpenDocument(const Poco::URI& documentURI);
    static WebView* findStarterScreen();
    static const std::vector<WebView*>& getAllInstances() { return s_instances; }
    void activateWindow();
    bool isStarterScreen() const { return _document._fakeClientFd == -1 && _document._appDocId == 0; }
    QMainWindow* getMainWindow() const { return _mainWindow; }

private:
    // query gnome font scaling factor and apply it to the web view
    void queryGnomeFontScalingUpdateZoom();
    QMainWindow* _mainWindow;
    std::unique_ptr<CODAWebEngineView> _webView;
    coda::DocumentData _document;
    bool _isWelcome;
    Bridge* _bridge;

    static std::vector<WebView*> s_instances;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
