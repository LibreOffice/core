/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XEventBroadcaster.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-05-17 13:33:02 $
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

package ifc.document;

import com.sun.star.beans.XPropertySet;
import lib.MultiMethodTest;

import com.sun.star.document.XEventBroadcaster;
import com.sun.star.document.XEventListener;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.uno.UnoRuntime;
import lib.StatusException;

public class _XEventBroadcaster extends MultiMethodTest {

    public XEventBroadcaster oObj;
    protected static boolean listenerCalled=false;
    private static XEventListener listener=null;

    public class MyEventListener implements XEventListener {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

        public void notifyEvent(com.sun.star.document.EventObject eventObject) {
            System.out.println("EventObject "+eventObject.EventName);
            listenerCalled = true;
        }

    }

    private void switchFocus() {
        XModel docModel = (XModel) UnoRuntime.queryInterface(
                XModel.class,tEnv.getTestObject());
        docModel.getCurrentController().getFrame().getContainerWindow().setFocus();
        util.utils.shortWait(1000);
        XController xc = (XController) UnoRuntime.queryInterface(XController.class,tEnv.getObjRelation("CONT2"));
        xc.getFrame().getContainerWindow().setFocus();
    }

    public void _addEventListener() {
        listener = new MyEventListener();
        listenerCalled = false;
        oObj.addEventListener(listener);
        switchFocus();
        util.utils.shortWait(1000);
        tRes.tested("addEventListener()",listenerCalled);
    }

    public void _removeEventListener() {
        requiredMethod("addEventListener()");
        listenerCalled = false;
        oObj.removeEventListener(listener);
        switchFocus();
        util.utils.shortWait(1000);
        tRes.tested("removeEventListener()",!listenerCalled);
    }

}
