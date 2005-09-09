/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XRemoteContentProviderAcceptor.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:33:03 $
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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XRemoteContentProviderAcceptor;
import com.sun.star.ucb.XRemoteContentProviderDoneListener;

/**
 * Testing <code>com.sun.star.ucb.XRemoteContentProviderAcceptor</code>
 * interface methods :
 * <ul>
 *  <li><code> addRemoteContentProvider()</code></li>
 *  <li><code> removeRemoteContentProvider()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ucb.XRemoteContentProviderAcceptor
 */
public class _XRemoteContentProviderAcceptor extends MultiMethodTest {

    public XRemoteContentProviderAcceptor oObj = null;

    /**
     * The simple <code>XRemoteContentProviderDoneListener</code>
     * implementation.
     */
    public class DoneListener implements XRemoteContentProviderDoneListener {

        public void doneWithRemoteContentProviders
                (XRemoteContentProviderAcceptor xRCPA) {
        }
        public void disposing (com.sun.star.lang.EventObject obj) {}

    };

    XRemoteContentProviderDoneListener aDoneListener = new DoneListener();

    /**
     * Adds a remote provider. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>.
     */
    public void _addRemoteContentProvider() {
        boolean res = false;

        String[] template = new String[]{"file"};
        res = oObj.addRemoteContentProvider("ContentID",(XMultiServiceFactory)tParam.getMSF(),
            template,aDoneListener);

        tRes.tested("addRemoteContentProvider()",res);
    }

    /**
     * Removes the remote provider added before. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addRemoteContentProvider() </code> : rovider must
     *  be added first </li>
     * </ul>
     */
    public void _removeRemoteContentProvider() {
        requiredMethod("addRemoteContentProvider()") ;

        boolean res = false;

        res = oObj.removeRemoteContentProvider("ContentID");
        tRes.tested("removeRemoteContentProvider()",res);
    }


}

