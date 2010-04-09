/*************************************************************************
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

#ifndef SD_UI_TASKPANE_PANELID_HXX
#define SD_UI_TASKPANE_PANELID_HXX

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= PanelId
    //==================================================================================================================
    /** List of top level panels that can be shown in the task pane.
    */
    enum PanelId
    {
        PID_UNKNOWN             = 0,
        PID_MASTER_PAGES        = 1,
        PID_LAYOUT              = 2,
        PID_TABLE_DESIGN        = 3,
        PID_CUSTOM_ANIMATION    = 4,
        PID_SLIDE_TRANSITION    = 5,

        PID_FIRST_CUSTOM_PANEL  = 6
    };

    PanelId GetStandardPanelId( const ::rtl::OUString& i_rTaskPanelResourceURL );

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_UI_TASKPANE_PANELID_HXX
