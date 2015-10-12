/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKSYS_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKSYS_HXX

#include "generic/gensys.h"
#include <gtk/gtk.h>
#include <unx/saltype.h>
#include <deque>

class GtkSalSystem : public SalGenericSystem
{
    typedef std::deque<std::pair<GdkScreen*, int> > ScreenMonitors_t;

    GdkDisplay *mpDisplay;
    // Number of monitors for every active screen.
    ScreenMonitors_t maScreenMonitors;
public:
             GtkSalSystem();
    virtual ~GtkSalSystem();
    static   GtkSalSystem *GetSingleton();

    virtual bool          IsUnifiedDisplay() override;
    virtual unsigned int  GetDisplayScreenCount() override;
    virtual unsigned int  GetDisplayBuiltInScreen() override;
    virtual Rectangle     GetDisplayScreenPosSizePixel   (unsigned int nScreen) override;
    virtual int           ShowNativeDialog (const OUString&              rTitle,
                                            const OUString&              rMessage,
                                            const std::list< OUString >& rButtons,
                                            int                        nDefButton) override;
    SalX11Screen      GetDisplayDefaultXScreen()
            { return getXScreenFromDisplayScreen( GetDisplayBuiltInScreen() ); }
    int               GetDisplayXScreenCount();
    SalX11Screen      getXScreenFromDisplayScreen(unsigned int nDisplayScreen);
    void              countScreenMonitors();
    // We have a 'screen' number that is combined from screen-idx + monitor-idx
    int        getScreenIdxFromPtr     (GdkScreen *pScreen);
    int        getScreenMonitorIdx     (GdkScreen *pScreen, int nX, int nY);
    GdkScreen *getScreenMonitorFromIdx (int nIdx, gint &nMonitor);
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKSYS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
