/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PanelController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:49:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
