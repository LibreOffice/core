/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Michael Meeks <michael.meeks@novell.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <string.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtksys.hxx>

GtkSalSystem *GtkSalSystem::GetSingleton()
{
    static GtkSalSystem *pSingleton = NULL;
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

struct GdkRectangleEqual
{
    bool operator()(GdkRectangle const& rLeft, GdkRectangle const& rRight)
    {
        return
            rLeft.x == rRight.x
            && rLeft.y == rRight.y
            && rLeft.width == rRight.width
            && rLeft.height == rRight.height
            ;
    }
};

}

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
            GdkRectangleEqual aCmp;
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
    GType gdk_x11_display_get_type (void);
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
    GdkScreen *pScreen = NULL;
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
#if GTK_CHECK_VERSION(2,14,0)
static int _fallback_get_primary_monitor (GdkScreen *pScreen)
{
    // Use monitor name as primacy heuristic
    int ret = -1;
    int max = gdk_screen_get_n_monitors (pScreen);
    for (int i = 0; i < max && ret < 0; i++)
    {
        char *name = gdk_screen_get_monitor_plug_name (pScreen, i);
        if (name && !g_ascii_strncasecmp (name, "LVDS", 4))
            ret = i;
        g_free (name);
    }
    return 0;
}
#endif

static int _get_primary_monitor (GdkScreen *pScreen)
{
    static int (*get_fn) (GdkScreen *) = NULL;
#if GTK_CHECK_VERSION(3,0,0)
    get_fn = gdk_screen_get_primary_monitor;
#endif
    // Perhaps we have a newer gtk+ with this symbol:
    if (!get_fn)
    {
        GModule *module = g_module_open (NULL, (GModuleFlags) 0);
        if (!g_module_symbol (module, "gdk_screen_get_primary_monitor",
                              (gpointer *)&get_fn))
            get_fn = NULL;
        g_module_close (module);
    }
#if GTK_CHECK_VERSION(2,14,0)
    if (!get_fn)
        get_fn = _fallback_get_primary_monitor;
#endif
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

Rectangle GtkSalSystem::GetDisplayScreenWorkAreaPosSizePixel (unsigned int nScreen)
{
    // FIXME: in theory we need extra code here to collect
    // the work area, ignoring fixed panels etc. on the screen.
    // surely gtk+ should have API to get this for us (?)
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString GtkSalSystem::GetDisplayScreenName(unsigned int nScreen)
{
    gchar *pStr;
    gint nMonitor;
    GdkScreen *pScreen;
    pScreen = getScreenMonitorFromIdx (nScreen, nMonitor);
    if (!pScreen)
        return rtl::OUString();

#if GTK_CHECK_VERSION(3,0,0) || GTK_CHECK_VERSION(2,14,0)
    pStr = gdk_screen_get_monitor_plug_name (pScreen, nMonitor);
#else
    static gchar * (*get_fn) (GdkScreen *, int) = NULL;

    GModule *module = g_module_open (NULL, (GModuleFlags) 0);
    if (!g_module_symbol (module, "gdk_screen_get_monitor_plug_name",
                          (gpointer *)&get_fn))
        get_fn = NULL;
    g_module_close (module);

    if (get_fn)
        pStr = get_fn (pScreen, nMonitor);
    else
        pStr = g_strdup_printf ("%d", nScreen);
#endif
    rtl::OUString aRet (pStr, strlen (pStr), RTL_TEXTENCODING_UTF8);
    g_free (pStr);
    return aRet;
}

// convert ~ to indicate mnemonic to '_'
static rtl::OString MapToGtkAccelerator(const rtl::OUString &rStr)
{
    return rtl::OUStringToOString(rStr.replaceFirst("~", "_"), RTL_TEXTENCODING_UTF8);
}

int GtkSalSystem::ShowNativeDialog (const rtl::OUString& rTitle, const rtl::OUString& rMessage,
                                    const std::list< rtl::OUString >& rButtonNames,
                                    int nDefaultButton)
{
    rtl::OString aTitle (rtl::OUStringToOString (rTitle, RTL_TEXTENCODING_UTF8));
    rtl::OString aMessage (rtl::OUStringToOString (rMessage, RTL_TEXTENCODING_UTF8));

    GtkDialog *pDialog = GTK_DIALOG (
        g_object_new (GTK_TYPE_MESSAGE_DIALOG,
                      "title", aTitle.getStr(),
                      "message-type", (int)GTK_MESSAGE_WARNING,
                      "text", aMessage.getStr(),
                      NULL));
    int nButton = 0;
    std::list< rtl::OUString >::const_iterator it;
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
