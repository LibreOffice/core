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

#include <string.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtksys.hxx>

GtkSalSystem *GtkSalSystem::GetSingleton()
{
    static GtkSalSystem *pSingleton = nullptr;
    if (!pSingleton)
        pSingleton = new GtkSalSystem();
    return pSingleton;
}

SalSystem *GtkInstance::CreateSalSystem()
{
    return GtkSalSystem::GetSingleton();
}

GtkSalSystem::GtkSalSystem() : SalGenericSystem()
{
    mpDisplay = gdk_display_get_default();
    countScreenMonitors();
#if GTK_CHECK_VERSION(3,0,0)
    // rhbz#1285356, native look will be gtk2, which crashes
    // when gtk3 is already loaded. Until there is a solution
    // java-side force look and feel to something that doesn't
    // crash when we are using gtk3
    setenv("STOC_FORCE_SYSTEM_LAF", "true", 1);
#endif
}

GtkSalSystem::~GtkSalSystem()
{
}

int
GtkSalSystem::GetDisplayXScreenCount()
{
    return gdk_display_get_n_screens (mpDisplay);
}

namespace
{

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
            GdkRectangleCoincident aCmp;
            std::sort(aGeometries.begin(), aGeometries.end(), aCmp);
            const std::vector<GdkRectangle>::iterator aUniqueEnd(
                    std::unique(aGeometries.begin(), aGeometries.end(), aCmp));
            nMonitors = std::distance(aGeometries.begin(), aUniqueEnd);
        }
        maScreenMonitors.push_back(std::make_pair(pScreen, nMonitors));
    }
}

// Including gdkx.h kills us with the Window / XWindow conflict
extern "C" {
#if GTK_CHECK_VERSION(3,0,0)
    GType gdk_x11_display_get_type();
#endif
    int   gdk_x11_screen_get_screen_number (GdkScreen *screen);
}

SalX11Screen
GtkSalSystem::getXScreenFromDisplayScreen(unsigned int nScreen)
{
    gint nMonitor;

    GdkScreen *pScreen = getScreenMonitorFromIdx (nScreen, nMonitor);
    if (!pScreen)
        return SalX11Screen (0);
#if GTK_CHECK_VERSION(3,0,0)
    if (!G_TYPE_CHECK_INSTANCE_TYPE (mpDisplay, gdk_x11_display_get_type ()))
        return SalX11Screen (0);
#endif
    return SalX11Screen (gdk_x11_screen_get_screen_number (pScreen));
}

GdkScreen *
GtkSalSystem::getScreenMonitorFromIdx (int nIdx, gint &nMonitor)
{
    GdkScreen *pScreen = nullptr;
    for (ScreenMonitors_t::const_iterator aIt(maScreenMonitors.begin()), aEnd(maScreenMonitors.end()); aIt != aEnd; ++aIt)
    {
        pScreen = aIt->first;
        if (!pScreen)
            break;
        if (nIdx >= aIt->second)
            nIdx -= aIt->second;
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
    for (ScreenMonitors_t::const_iterator aIt(maScreenMonitors.begin()), aEnd(maScreenMonitors.end()); aIt != aEnd; ++aIt)
    {
        if (aIt->first == pScreen)
            return nIdx;
        nIdx += aIt->second;
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

unsigned int GtkSalSystem::GetDisplayScreenCount()
{
    gint nMonitor;
    (void)getScreenMonitorFromIdx (G_MAXINT, nMonitor);
    return G_MAXINT - nMonitor;
}

bool GtkSalSystem::IsUnifiedDisplay()
{
    return gdk_display_get_n_screens (mpDisplay) == 1;
}

namespace {
int _fallback_get_primary_monitor (GdkScreen *pScreen)
{
    // Use monitor name as primacy heuristic
    int max = gdk_screen_get_n_monitors (pScreen);
    for (int i = 0; i < max; ++i)
    {
        char *name = gdk_screen_get_monitor_plug_name (pScreen, i);
        bool bLaptop = (name && !g_ascii_strncasecmp (name, "LVDS", 4));
        g_free (name);
        if (bLaptop)
            return i;
    }
    return 0;
}

int _get_primary_monitor (GdkScreen *pScreen)
{
    static int (*get_fn) (GdkScreen *) = nullptr;
#if GTK_CHECK_VERSION(3,0,0)
    get_fn = gdk_screen_get_primary_monitor;
#endif
    // Perhaps we have a newer gtk+ with this symbol:
    if (!get_fn)
    {
        get_fn = reinterpret_cast<int(*)(GdkScreen*)>(osl_getAsciiFunctionSymbol(nullptr,
            "gdk_screen_get_primary_monitor"));
    }
    if (!get_fn)
        get_fn = _fallback_get_primary_monitor;
    if (get_fn)
        return get_fn (pScreen);
    else
        return 0;
}
} // end anonymous namespace

unsigned int GtkSalSystem::GetDisplayBuiltInScreen()
{
    GdkScreen *pDefault = gdk_display_get_default_screen (mpDisplay);
    int idx = getScreenIdxFromPtr (pDefault);
    return idx + _get_primary_monitor (pDefault);
}

Rectangle GtkSalSystem::GetDisplayScreenPosSizePixel (unsigned int nScreen)
{
    gint nMonitor;
    GdkScreen *pScreen;
    GdkRectangle aRect;
    pScreen = getScreenMonitorFromIdx (nScreen, nMonitor);
    if (!pScreen)
        return Rectangle();
    gdk_screen_get_monitor_geometry (pScreen, nMonitor, &aRect);
    return Rectangle (Point(aRect.x, aRect.y), Size(aRect.width, aRect.height));
}

// convert ~ to indicate mnemonic to '_'
static OString MapToGtkAccelerator(const OUString &rStr)
{
    return OUStringToOString(rStr.replaceFirst("~", "_"), RTL_TEXTENCODING_UTF8);
}

int GtkSalSystem::ShowNativeDialog (const OUString& rTitle, const OUString& rMessage,
                                    const std::list< OUString >& rButtonNames,
                                    int nDefaultButton)
{
    OString aTitle (OUStringToOString (rTitle, RTL_TEXTENCODING_UTF8));
    OString aMessage (OUStringToOString (rMessage, RTL_TEXTENCODING_UTF8));

    GtkDialog *pDialog = GTK_DIALOG (
        g_object_new (GTK_TYPE_MESSAGE_DIALOG,
                      "title", aTitle.getStr(),
                      "message-type", (int)GTK_MESSAGE_WARNING,
                      "text", aMessage.getStr(),
                      nullptr));
    int nButton = 0;
    std::list< OUString >::const_iterator it;
    for (it = rButtonNames.begin(); it != rButtonNames.end(); ++it)
        gtk_dialog_add_button (pDialog, MapToGtkAccelerator(*it).getStr(), nButton++);
    gtk_dialog_set_default_response (pDialog, nDefaultButton);

    nButton = gtk_dialog_run (pDialog);
    if (nButton < 0)
        nButton = -1;

    gtk_widget_destroy (GTK_WIDGET (pDialog));

    return nButton;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
