/*************************************************************************
 *
 *  $RCSfile: _XSubmit.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:35:47 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.XControl;
import com.sun.star.beans.XPropertySet;
import com.sun.star.form.XSubmit;
import com.sun.star.form.XSubmitListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.form.XSubmit</code>
* interface methods :
* <ul>
*  <li><code> submit()</code></li>
*  <li><code> addSubmitListener()</code></li>
*  <li><code> removeSubmitListener()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'XSubmit.Control'</code> <b>optional</b>
*  (of type <code>com.sun.star.awt.XControl</code>):
*   is used to pass as parameters to <code>submit</code>
*   method. <code>null</code> is passed if the relation
*   is not found.</li>
* <ul> <p>
*
* Other <b> prerequicity </b> is the object must have
* <code>TargetURL</code> property. <p>
*
* Short description : test adds two listeners, call
* <code> submit </code> method and cecks if both listeners
* were called. Then one listener is removed and after
* <code> submit </code> method call it must not be called. <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XSubmit
*/
public class _XSubmit extends MultiMethodTest {

    public static XSubmit oObj = null;

    public class MySubmitListener implements XSubmitListener {
        public int called = 0 ;
        public void disposing ( EventObject oEvent ) {
        }
        public boolean approveSubmit( EventObject oEvent ) {
            called += 1;
            System.out.println("Listener called");
            return true;
        }
    }


    MySubmitListener listener1 = new MySubmitListener();
    MySubmitListener listener2 = new MySubmitListener();

    /**
    * Just adds two submit listeners. <p>
    * Status of this method test is defined in <code>
    * submit </code> method test.
    */
    public void _addSubmitListener() {
        log.println("Testing addSubmitListener ...");
        oObj.addSubmitListener( listener1 );
        oObj.addSubmitListener( listener2 );
    }

    /**
    * Before submision tries to set 'TargetURL' property
    * of component to some value assuming that component
    * supports <code>com.sun.star.form.HTMLForm</code>
    * service.
    * Then calls the <code> submit </code> method and checks
    * if listener removed were not called, and other was
    * called only once.<p>
    *
    * Has <b> OK </b> status for <code>submit</code> if
    * listener was called at least ones, for
    * <code>addSubmitListener</code> method if the remaining
    * listener was called only once, for
    * <code>removeSubmitListener</code> method if the removed
    * listener was not called. <p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> removeSubmitListener </code> : to have one listener
    *  added and other removed.</li>
    * </ul>
    */
    public void _submit() {
        executeMethod("removeSubmitListener()");
        log.println("Testing submit() ...");
        XControl cntrl = (XControl) tEnv.getObjRelation("XSubmit.Control") ;

        XPropertySet xPS = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, oObj) ;

        if (xPS != null) {
            try {
                xPS.setPropertyValue("TargetURL", "someserver");
            } catch (com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                e.printStackTrace(log);
            }
        } else {
            log.println("!!! The tested compoennt doesn't support XPropertySet ");
        }

        oObj.submit(cntrl, new MouseEvent());
        shortWait();

        log.println("Listener1 called " + listener1.called + " times");
        log.println("Listener2 called " + listener2.called + " times");

        tRes.tested("addSubmitListener()", listener2.called == 1);
        tRes.tested("removeSubmitListener()", listener1.called == 0);
        tRes.tested("submit()", listener2.called > 0);
        oObj.removeSubmitListener(listener2);
    }

    /**
    * Just removes one of submit listeners. <p>
    * Status of this method test is defined in <code>
    * submit </code> method test.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> removeSubmitListener </code> : to have listeners added</li>
    * </ul>
    */
    public void _removeSubmitListener() {
        requiredMethod("addSubmitListener()");
        oObj.removeSubmitListener(listener1);
    }

    /**
    * Sleeps for 0.2 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

}

