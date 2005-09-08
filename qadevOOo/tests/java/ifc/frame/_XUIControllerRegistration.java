/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUIControllerRegistration.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:05:50 $
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

package ifc.frame;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XUIControllerRegistration;
import lib.MultiMethodTest;

public class _XUIControllerRegistration extends MultiMethodTest {
    public XUIControllerRegistration oObj = null;
    private String aRegisteredController = null;
    private String aController = ".uno:MySecondController";

    public void before() {
        aRegisteredController = (String)tEnv.getObjRelation("XUIControllerRegistration.RegisteredController");
    }


    public void _registerController() {
        oObj.registerController(aController, "", "com.sun.star.comp.framework.FooterMenuController");
        oObj.registerController(aRegisteredController, "", "com.sun.star.comp.framework.HeaderMenuController");
        tRes.tested("registerController()", true);
    }

    public void _deregisterController() {
        requiredMethod("registerController()");
        oObj.deregisterController(aController, "");
        tRes.tested("deregisterController()", true);
    }

    public void _hasController() {
        requiredMethod("registerController()");
        requiredMethod("deregisterController()");
        boolean res = oObj.hasController(aRegisteredController, "");
        res &= !oObj.hasController(aController, "");
        tRes.tested("hasController()", res);
    }

}