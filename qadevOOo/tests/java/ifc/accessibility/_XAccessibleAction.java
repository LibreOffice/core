/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAccessibleAction.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:46:34 $
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

package ifc.accessibility;

import com.sun.star.accessibility.XAccessibleAction;

public class _XAccessibleAction extends lib.MultiMethodTest {

    public XAccessibleAction oObj = null;
    public int count = 0;

    /**
     * calls the method and stores the result in the <br>
     * variable count. Is OK if no excpetion occurs
     */

    public void _getAccessibleActionCount() {
        count = oObj.getAccessibleActionCount();
        tRes.tested("getAccessibleActionCount()",count > 0);
    }

    /**
     * calls the method with invalid argument and check if the <br>
     * expected Exception is thrown.<br>
     * Calls the method afterwards the first valid parameter.<br>
     * This is the last method called and the environment is disposed<br>
     * afterwards.
     */

    public void _doAccessibleAction() {
        requiredMethod("getAccessibleActionKeyBinding()");
        boolean res = true;

        log.println("Calling method with wrong argument");
        try {
            oObj.doAccessibleAction(count);
            log.println("Exception expected -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Expected exception -- OK");
            res &= true;
        }

        try {
            boolean act = false;
            for (int i = 0; i< count; i++) {
                log.println("do Action "+ oObj.getAccessibleActionDescription(i));
                act = oObj.doAccessibleAction(i);
                log.println("Worked: "+act);
            }
            log.println("Did action: "+act);
            res &= act ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Unexepected exception -- FAILED");
            res &= false;
        }

        tRes.tested("doAccessibleAction()",res);
    }

    /**
     * calls the method with invalid argument and check if the <br>
     * expected Exception is thrown.<br>
     * Calls the method afterwards all valid parameters.<br>
     * Is ok if the exception is thrown and the resulting value
     * for the calls with valid parameters aren't null.
     */

    public void _getAccessibleActionDescription() {
        requiredMethod("getAccessibleActionCount()");
        boolean res = true;

        log.println("Calling method with wrong argument");
        try {
            oObj.getAccessibleActionDescription(count);
            log.println("Exception expected -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Expected exception -- OK");
            res &= true;
        }

        for (int i=0;i<count;i++) {
            try {
                String desc = oObj.getAccessibleActionDescription(i);
                log.println("Found action: "+desc);
                res &= desc!=null ;
            } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
                log.println("Unexepected exception -- FAILED");
                res &= false;
            }
        }

        tRes.tested("getAccessibleActionDescription()",res);
    }

    /**
     * calls the method with invalid argument and check if the <br>
     * expected Exception is thrown.<br>
     * Calls the method afterwards all valid parameters.<br>
     * Is ok if the exception is thrown and the resulting value
     * for the calls with valid parameters aren't null.
     */

    public void _getAccessibleActionKeyBinding() {
        requiredMethod("getAccessibleActionDescription()");
        boolean res = true;

        log.println("Calling method with wrong argument");
        try {
            oObj.getAccessibleActionKeyBinding(count);
            log.println("Exception expected -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Expected exception -- OK");
            res &= true;
        }

        for (int i=0;i<count;i++) {
            try {
                Object key = oObj.getAccessibleActionKeyBinding(i);
                if (key != null ) {
                    log.println("Found key: "+key.toString());
                }
                res &= true;
            } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
                log.println("Unexepected exception -- FAILED");
                res &= false;
            }
        }

        tRes.tested("getAccessibleActionKeyBinding()",res);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

}
