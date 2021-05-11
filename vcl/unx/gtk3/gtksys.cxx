/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <gtk/gtk.h>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtksys.hxx>
#include <unx/gtk/gtkbackend.hxx>
#include <osl/module.h>

GtkSalSystem *GtkSalSystem::GetSingleton()
{
    static GtkSalSystem *pSingleton = new GtkSalSystem();
    return pSingleton;
}

SalSystem *GtkInstance::CreateSalSystem()
{
    return GtkSalSystem::GetSingleton();
}

GtkSalSystem::GtkSalSystem() : SalGenericSystem()
{
    mpDisplay = gdk_display_get_default();
#if !GTK_CHECK_VERSION(4, 0, 0)
    countScreenMonitors();
#endif
    // rhbz#1285356, native look will be gtk2, which crashes
    // when gtk3 is already loaded. Until there is a solution
    // java-side force look and feel to something that doesn't
    // crash when we are using gtk3
    setenv("STOC_FORCE_SYSTEM_LAF", "true", 1);
}

GtkSalSystem::~GtkSalSystem()
{
}

namespace
{

struct GdkRectangleCoincidentLess
{
    // fdo#78799 - detect and elide overlaying monitors of different sizes
    bool operator()(GdkRectangle const& rLeft, GdkRectangle const& rRight)
    {
        return
            rLeft.x < rRight.x
            || rLeft.y < rRight.y
            ;
    }
};
struct GdkRectangleCoincident
{
    // fdo#78799 - detect and elide overlaying monitors of different sizes
    bool operator()(GdkRectangle const& rLeft, GdkRectangle const& rRight)
    {
        return
            rLeft.x == rRight.x
            && rLeft.y == rRight.y
            ;
    }
};

}

#if !GTK_CHECK_VERSION(4, 0, 0)
/**
 * GtkSalSystem::countScreenMonitors()
 *
 * This method builds the vector which allows us to map from VCL's
 * idea of linear integer ScreenNumber to gtk+'s rather more
 * complicated screen + monitor concept.
 */
void
GtkSalSystem::countScreenMonitors()
{
    maScreenMonitors.clear();
    for (gint i = 0; i < gdk_display_get_n_screens(mpDisplay); i++)
    {
        GdkScreen* const pScreen(gdk_display_get_screen(mpDisplay, i));
        gint nMonitors(pScreen ? gdk_screen_get_n_monitors(pScreen) : 0);
        if (nMonitors > 1)
        {
            std::vector<GdkRectangle> aGeometries;
            aGeometries.reserve(nMonitors);
            for (gint j(0); j != nMonitors; ++j)
            {
                GdkRectangle aGeometry;
                gdk_screen_get_monitor_geometry(pScreen, j, &aGeometry);
                aGeometries.push_back(aGeometry);
            }
            std::sort(aGeometries.begin(), aGeometries.end(),
                    GdkRectangleCoincidentLess());
            const std::vector<GdkRectangle>::iterator aUniqueEnd(
                    std::unique(aGeometries.begin(), aGeometries.end(),
                    GdkRectangleCoincident()));
            nMonitors = std::distance(aGeometries.begin(), aUniqueEnd);
        }
        maScreenMonitors.emplace_back(pScreen, nMonitors);
    }
}
#endif

SalX11Screen
GtkSalSystem::getXScreenFromDisplayScreen(unsigned int nScreen)
{
    if (!DLSYM_GDK_IS_X11_DISPLAY(mpDisplay))
        return SalX11Screen (0);

#if GTK_CHECK_VERSION(4, 0, 0)
    GdkX11Screen *pScreen = gdk_x11_display_get_screen(mpDisplay);
    (void)nScreen;
#else
    gint nMonitor;
    GdkScreen *pScreen = getScreenMonitorFromIdx (nScreen, nMonitor);
    if (!pScreen)
        return SalX11Screen (0);
#endif
    return SalX11Screen(gdk_x11_screen_get_screen_number(pScreen));
}

#if !GTK_CHECK_VERSION(4, 0, 0)
GdkScreen *
GtkSalSystem::getScreenMonitorFromIdx (int nIdx, gint &nMonitor)
{
    GdkScreen *pScreen = nullptr;
    for (auto const& screenMonitor : maScreenMonitors)
    {
        pScreen = screenMonitor.first;
        if (!pScreen)
            break;
        if (nIdx >= screenMonitor.second)
            nIdx -= screenMonitor.second;
        else
            break;
    }
    nMonitor = nIdx;

    // handle invalid monitor indexes as non-existent screens
    if (nMonitor < 0 || (pScreen && nMonitor >= gdk_screen_get_n_monitors (pScreen)))
        pScreen = nullptr;

    return pScreen;
}

int
GtkSalSystem::getScreenIdxFromPtr (GdkScreen *pScreen)
{
    int nIdx = 0;
    for (auto const& screenMonitor : maScreenMonitors)
    {
        if (screenMonitor.first == pScreen)
            return nIdx;
        nIdx += screenMonitor.second;
    }
    g_warning ("failed to find screen %p", pScreen);
    return 0;
}

int GtkSalSystem::getScreenMonitorIdx (GdkScreen *pScreen,
                                       int nX, int nY)
{
    // TODO: this will fail horribly for exotic combinations like two
    // monitors in mirror mode and one extra. Hopefully such
    // abominations are not used (or, even better, not possible) in
    // practice .-)
    return getScreenIdxFromPtr (pScreen) +
        gdk_screen_get_monitor_at_point (pScreen, nX, nY);
}
#endif

unsigned int GtkSalSystem::GetDisplayScreenCount()
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return g_list_model_get_n_items(gdk_display_get_monitors(mpDisplay));
#else
    gint nMonitor;
    (void)getScreenMonitorFromIdx (G_MAXINT, nMonitor);
    return G_MAXINT - nMonitor;
#endif
}

bool GtkSalSystem::IsUnifiedDisplay()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    return gdk_display_get_n_screens (mpDisplay) == 1;
#else
    return true;
#endif
}

unsigned int GtkSalSystem::GetDisplayBuiltInScreen()
{
#if GTK_CHECK_VERSION(4, 0, 0)
#if defined(GDK_WINDOWING_X11)
    if (DLSYM_GDK_IS_X11_DISPLAY(mpDisplay))
    {
        GdkMonitor* pPrimary = gdk_x11_display_get_primary_monitor(mpDisplay);
        GListModel* pList = gdk_display_get_monitors(mpDisplay);
        int nIndex = 0;
        while (gpointer pElem = g_list_model_get_item(pList, nIndex))
        {
            if (pElem == pPrimary)
                return nIndex;
            ++nIndex;
        }
    }
#endif
    // nothing for wayland ?, hope for the best that its at index 0
    return 0;
#else // !GTK_CHECK_VERSION(4, 0, 0)
    GdkScreen *pDefault = gdk_display_get_default_screen (mpDisplay);
    int idx = getScreenIdxFromPtr (pDefault);
    return idx + gdk_screen_get_primary_monitor(pDefault);
#endif
}

tools::Rectangle GtkSalSystem::GetDisplayScreenPosSizePixel(unsigned int nScreen)
{
    GdkRectangle aRect;
#if GTK_CHECK_VERSION(4, 0, 0)
    GListModel* pList = gdk_display_get_monitors(mpDisplay);
    GdkMonitor* pMonitor = static_cast<GdkMonitor*>(g_list_model_get_item(pList, nScreen));
    if (!pMonitor)
        return tools::Rectangle();
    gdk_monitor_get_geometry(pMonitor, &aRect);
#else
    gint nMonitor;
    GdkScreen *pScreen;
    pScreen = getScreenMonitorFromIdx (nScreen, nMonitor);
    if (!pScreen)
        return tools::Rectangle();
    gdk_screen_get_monitor_geometry (pScreen, nMonitor, &aRect);
#endif
    return tools::Rectangle (Point(aRect.x, aRect.y), Size(aRect.width, aRect.height));
}

// convert ~ to indicate mnemonic to '_'
static OString MapToGtkAccelerator(const OUString &rStr)
{
    return OUStringToOString(rStr.replaceFirst("~", "_"), RTL_TEXTENCODING_UTF8);
}

int GtkSalSystem::ShowNativeDialog (const OUString& rTitle, const OUString& rMessage,
                                    const std::vector< OUString >& rButtonNames)
{
    OString aTitle (OUStringToOString (rTitle, RTL_TEXTENCODING_UTF8));
    OString aMessage (OUStringToOString (rMessage, RTL_TEXTENCODING_UTF8));

    GtkDialog *pDialog = GTK_DIALOG (
        g_object_new (GTK_TYPE_MESSAGE_DIALOG,
                      "title", aTitle.getStr(),
                      "message-type", int(GTK_MESSAGE_WARNING),
                      "text", aMessage.getStr(),
                      nullptr));
    int nButton = 0;
    for (auto const& buttonName : rButtonNames)
        gtk_dialog_add_button (pDialog, MapToGtkAccelerator(buttonName).getStr(), nButton++);
    gtk_dialog_set_default_response (pDialog, 0/*nDefaultButton*/);

    nButton = gtk_dialog_run (pDialog);
    if (nButton < 0)
        nButton = -1;

#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_destroy(GTK_WIDGET(pDialog));
#else
    gtk_window_destroy(GTK_WINDOW(pDialog));
#endif

    return nButton;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
