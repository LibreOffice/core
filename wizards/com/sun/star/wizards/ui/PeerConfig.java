/*************************************************************************
*
*  $RCSfile: PeerConfig.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: vg $ $Date: 2005-02-21 14:05:32 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): Berend Cornelius
*
*/

package com.sun.star.wizards.ui;

import java.util.Vector;

import com.sun.star.awt.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class PeerConfig implements XWindowListener{
    Vector aPeerTasks = new Vector();


    public PeerConfig(XWindow xDialogWindow) {
        xDialogWindow.addWindowListener(this);
    }

    class PeerTask{
        XControl xControl;
        String[] propnames;
        Object[] propvalues;

        public PeerTask(XControl _xControl, String[] propNames_, Object[] propValues_){
            propnames = propNames_;
            propvalues = propValues_;
            xControl = _xControl;
        }
    }


    public void windowResized(WindowEvent arg0) {
    }

    public void windowMoved(WindowEvent arg0) {
    }

    public void windowShown(EventObject arg0) {
        try {
            for (int i = 0; i < this.aPeerTasks.size(); i++){
                PeerTask aPeerTask = (PeerTask) aPeerTasks.elementAt(i);
                XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, aPeerTask.xControl.getPeer());
                for (int n = 0; i < aPeerTask.propnames.length; i++)
                    xVclWindowPeer.setProperty(aPeerTask.propnames[n], aPeerTask.propvalues[n]);
            }
        } catch (RuntimeException re) {
            re.printStackTrace();
            throw re;
        }
    }

    public void windowHidden(EventObject arg0) {
    }


    public void disposing(EventObject arg0) {
    }

    /**
     *
     * @param oAPIControl an API control that the interface XControl can be derived from
     * @param _scontrolname
     */
    public void setAccessiblityName(Object oAPIControl, String _saccessname) {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, oAPIControl);
        setPeerProperties(xControl, new String[] { "AccessibilityName" }, new String[]{_saccessname});
    }


    public void setAccessiblityName(XControl _xControl, String _saccessname) {
        setPeerProperties(_xControl, new String[] { "AccessibilityName" }, new String[]{_saccessname});
    }

    /**
     *
     * @param oAPIControl an API control that the interface XControl can be derived from
     * @param propnames
     * @param propvalues
     */
    public void setPeerProperties(Object oAPIControl, String[] _propnames, Object[] _propvalues) {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, oAPIControl);
        setPeerProperties(xControl, _propnames, _propvalues);
    }


    public void setPeerProperties(XControl _xControl, String[] propnames, Object[] propvalues) {
        PeerTask  oPeerTask = new PeerTask(_xControl, propnames, propvalues);
        this.aPeerTasks.add(oPeerTask);
    }
}
