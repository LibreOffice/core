/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XRemoteContentProviderActivator.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:33:20 $
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
import com.sun.star.ucb.XContentProviderManager;
import com.sun.star.ucb.XRemoteContentProviderAcceptor;
import com.sun.star.ucb.XRemoteContentProviderActivator;

/**
 * Testing <code>com.sun.star.ucb.XRemoteContentProviderActivator</code>
 * interface methods :
 * <ul>
 *  <li><code> activateRemoteContentProviders()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'RCPA'</code>
 *   (of type <code>XRemoteContentProviderAcceptor</code>):
 *   this acceptor is used to add a provider first before
 *   its activation. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.ucb.XRemoteContentProviderActivator
 */
public class _XRemoteContentProviderActivator extends MultiMethodTest {

    public XRemoteContentProviderActivator oObj = null;

    /**
     * First the relation is retrieved and a remote provider is
     * added to the acceptor. Then it is activated and
     * removed. <p>
     * Has <b> OK </b> status if <code>activateRemoteContentProviders</code>
     * method returns not <code>null</code> value. <p>
     */
    public void _activateRemoteContentProviders() {
        boolean res = false;

        XRemoteContentProviderAcceptor xRCPA = (XRemoteContentProviderAcceptor)
                                                    tEnv.getObjRelation("RCPA");
        String[] template = new String[]{"file"};
        xRCPA.addRemoteContentProvider("ContentID",(XMultiServiceFactory)tParam.getMSF(),template,null);
        XContentProviderManager CPM = oObj.activateRemoteContentProviders();
        res = (CPM != null);
        xRCPA.removeRemoteContentProvider("ContentID");

        tRes.tested("activateRemoteContentProviders()",res);
    }

}

