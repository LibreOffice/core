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
#ifndef _VCL_GTKSYS_HXX_
#define _VCL_GTKSYS_HXX_

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

    virtual bool          IsUnifiedDisplay();
    virtual unsigned int  GetDisplayScreenCount();
    virtual unsigned int  GetDisplayBuiltInScreen();
    virtual OUString GetDisplayScreenName                  (unsigned int nScreen);
    virtual Rectangle     GetDisplayScreenPosSizePixel   (unsigned int nScreen);
    virtual Rectangle     GetDisplayScreenWorkAreaPosSizePixel (unsigned int nScreen);
    virtual int           ShowNativeDialog (const OUString&              rTitle,
                                            const OUString&              rMessage,
                                            const std::list< OUString >& rButtons,
                                            int                        nDefButton);
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

#endif // _VCL_GTKSYS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
