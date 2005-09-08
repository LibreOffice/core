/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextListener.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:02:10 $
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

package ifc.awt;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XTextListener;

/**
* Testing <code>com.sun.star.awt.XTextListener</code>
* interface methods:
* <ul>
* <li><code> textChanged() </code></li>
* </ul><p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'TestTextListener'</code>
*   (of type <code>ifc.awt._XTextListener.TestTextListener</code>):
*   this listener implementation must be registered for object tested for
*   checking
*   <code>textChanged()</code> method call. The listener must be registered
*   in object environment creation because it's not a fact that tested
*   component supports <code>XTextComponent</code> interface and the listener
*   can be registered in another object.</li>
* <ul> <p>
*
* @see com.sun.star.awt.XTextListener
*/
public class _XTextListener extends MultiMethodTest {
    public XTextListener oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    * and stores event passed.
    */
    public static class TestTextListener implements
            com.sun.star.awt.XTextListener {
        public boolean textChangedCalled = false ;
        public TextEvent event = null ;

        public void textChanged(TextEvent e) {
            textChangedCalled = true ;
            event = e ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {}

    }

    TestTextListener textListener = null;
    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
    public void before() {
        textListener = (TestTextListener)
            tEnv.getObjRelation("TestTextListener");
        if (textListener == null) {
            throw new StatusException(Status.failed("Relation not found"));
        }
    }

    /**
    * First a <code>TextEvent</code> object created and
    * it is passed to <code>textChanged</code> method
    * call. Then a short wait follows for listener already
    * registered at the object to be called. <p>
    * Has <b> OK </b> status if the listener was called with
    * the same <code>TextEvent</code> object as was created
    * before.
    */
    public void _textChanged() {

        boolean result = true ;

        TextEvent event = new TextEvent() ;
        event.dummy1 = 2;
        oObj.textChanged(event);

        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {}

        result = textListener.textChangedCalled &&
            textListener.event.dummy1 == 2;

        tRes.tested("textChanged()", result) ;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }


}

