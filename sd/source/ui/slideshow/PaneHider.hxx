/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_PANE_HIDE_HXX
#define SD_PANE_HIDE_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>


namespace sd {

class ViewShell;
class SlideshowImpl;

/** Hide the windows of the side panes and restore the original visibility
    later. Used by the in-window slide show in order to use the whole frame
    window for the show.
*/
class PaneHider
{
public:
    /** The constructor hides all side panes that belong to the
        ViewShellBase of the given view shell.
    */
    PaneHider (const ViewShell& rViewShell, SlideshowImpl* pSlideShow);

    /** Restore the original visibility of the side panes.
    */
    ~PaneHider (void);

private:
    const ViewShell& mrViewShell;
    /** Remember whether the visibility states of the windows of the  panes
        has been modified and have to be restored.
    */
    bool mbWindowVisibilitySaved;
    bool mbOriginalLeftPaneWindowVisibility;
    bool mbOriginalRightPaneWindowVisibility;

    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfiguration>
        mxConfiguration;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
