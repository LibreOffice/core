/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PanelController.java,v $
 * $Revision: 1.3 $
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

package org.openoffice.setup;

import javax.swing.JPanel;

public abstract class PanelController {

    private SetupFrame frame;
    private JPanel panel;
    private String name;
    private String next;
    private String prev;

    private PanelController () {
    }

    public PanelController (String name, JPanel panel) {
        this.name  = name;
        this.panel = panel;
    }

    public final JPanel getPanel () {
        return this.panel;
    }

    public final void setPanel (JPanel panel) {
        this.panel = panel;
    }

    public final String getName () {
        return this.name;
    }

    public final void setName (String name) {
        this.name = name;
    }

    final void setSetupFrame (SetupFrame frame) {
        this.frame = frame;
    }

    public final SetupFrame getSetupFrame () {
        return this.frame;
    }

    public String getNext () {
        return null;
    }

    public String getDialogText () {
        return null;
    }

    public String getPrevious () {
        return null;
    }

    public void beforeShow () {
    }

    public void duringShow () {
    }

    public boolean afterShow (boolean nextButtonPressed) {
        boolean repeatDialog = false;
        return repeatDialog;
    }

    public abstract String getHelpFileName();

}
