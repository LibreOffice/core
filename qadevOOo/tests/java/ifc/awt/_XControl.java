/*************************************************************************
 *
 *  $RCSfile: _XControl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:10:19 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XView;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.awt.XControl</code>
* interface methods:
* <ul>
*  <li><code> setContext() </code></li>
*  <li><code> getContext() </code></li>
*  <li><code> createPeer() </code></li>
*  <li><code> getPeer() </code></li>
*  <li><code> setModel() </code></li>
*  <li><code> getModel() </code></li>
*  <li><code> setDesignMode() </code></li>
*  <li><code> isDesignMode() </code></li>
*  <li><code> isTransparent() </code></li>
*  <li><code> getView() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CONTEXT'</code> (of type <code>XInterface</code>):
*  used as a parameter to setContext() and for testing getContext().</li>
*  <li> <code>'WINPEER'</code> (of type <code>XWindowPeer</code>):
*  used as a parameter to createPeer() and for testing getPeer()</li>
*  <li> <code>'TOOLKIT'</code> (of type <code>XToolkit</code>):
*  used as a parameter to createPeer()</li>
*  <li> <code>'MODEL'</code> (of type <code>XControlModel</code>):
*  used as a parameter to setModel() and for testing getModel()</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XControl
*/
public class _XControl extends MultiMethodTest {
    public XControl oObj = null;
    public XControlModel aModel = null;
    public boolean desMode;

    /**
    * After test calls the method, the Context is set to a corresponding
    * object relation.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setContext() {
        XInterface cont = (XInterface) tEnv.getObjRelation("CONTEXT");
        oObj.setContext(cont);
        tRes.tested("setContext()",true);
    }

    /**
    * After test calls the method, the Context is gotten and compared
    * with object relation 'CONTEXT'.<p>
    * Has <b> OK </b> status if get value is equals to value set before.<p>
    * The following method tests are to be completed successfully before:
    * <ul>
    *  <li> <code> setContext() </code> : set Context to a corresponding
    * object relation</li>
    * </ul>
    */
    public void _getContext() {
        requiredMethod("setContext()");
        XInterface cont = (XInterface) tEnv.getObjRelation("CONTEXT");
        Object get = oObj.getContext();
        boolean res = get.equals(cont);
        if (!res) {
            log.println("!!! Error: getting: "+get.toString());
            log.println("!!! expected: "+cont.toString());
        }
        tRes.tested("getContext()",res);
    }


    /**
    * The objects needed to create peer are obtained
    * from corresponding object relations, then the peer is created.
    * <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _createPeer() {
        XWindowPeer the_win = (XWindowPeer) tEnv.getObjRelation("WINPEER");
        XToolkit the_kit = (XToolkit) tEnv.getObjRelation("TOOLKIT");
        oObj.createPeer(the_kit,the_win);
        tRes.tested("createPeer()",true);
    }

    /**
    * Test calls the method. Then the the object ralation 'WINPEER' is
    * obtained, and compared with the peer, gotten from (XControl) oObj
    * variable.<p>
    * Has <b> OK </b> status if peer gotten isn't null
    * The following method tests are to be completed successfully before :
    */
    public void _getPeer() {
        requiredMethod("createPeer()");
        boolean res = false;
        XWindowPeer get = oObj.getPeer();
        if (get == null) {
            log.println("The method 'getPeer' returns NULL");
        } else {
           res = true;
        }
        tRes.tested("getPeer()",res);
    }


    /**
    * At first current model is obtained and saved to variable aModel.
    * Then object relation 'MODEL' is gotten and test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setModel() {
        aModel = oObj.getModel();
        XControlModel the_model = (XControlModel) tEnv.getObjRelation("MODEL");
        oObj.setModel(the_model);
        tRes.tested("setModel()",true);
    }

    /**
    * Test calls the method, then object relation 'MODEL' is gotten and
    * compared with object returned by the method. Then previously saved
    * value of model (aModel) restored to (XControl) oObj<p>
    * Has <b> OK </b> status if models set and get are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setModel() </code> : setting model from corresponding
    *  object relation </li>
    * </ul>
    */
    public void _getModel() {
        requiredMethod("setModel()");
        XControlModel the_model = (XControlModel) tEnv.getObjRelation("MODEL");
        XControlModel get = oObj.getModel();
        boolean res = (get.equals(the_model));
        if (!res) {
            log.println("getting: "+get.toString());
            log.println("expected: "+the_model.toString());
        }
        if (aModel != null) {
        oObj.setModel(aModel);
    }
        tRes.tested("getModel()",res);
    }

    /**
    * Test calls the method. Then mode is checked using isDesignMode().<p>
    * Has <b> OK </b> status if mode is swithed.
    */
    public void _setDesignMode() {
    desMode = oObj.isDesignMode();
        oObj.setDesignMode(!desMode);
        tRes.tested("setDesignMode()",oObj.isDesignMode() == !desMode);
    }

    /**
    * The mode is changed and result is checked.<p>
    * Has <b> OK </b> status if the mode changed successfully.
    */
    public void _isDesignMode() {
    requiredMethod("setDesignMode()");
        oObj.setDesignMode(desMode);
        tRes.tested("isDesignMode()", oObj.isDesignMode() == desMode);
    }

    /**
    * Test calls the method.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _isTransparent() {
        boolean isT = oObj.isTransparent();
        tRes.tested("isTransparent()",true );
    }

    /**
    * Test calls the method.<p>
    * Has <b> OK </b> status if the method returns not null. <p>
    */
    public void _getView() {
        XView the_view = oObj.getView();
        tRes.tested("getView()", the_view != null);
    }

}


