/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XItemListener.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:58:40 $
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

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XItemListener;

/**
* Testing <code>com.sun.star.awt.XItemListener</code>
* interface methods :
* <ul>
*  <li><code> itemStateChanged()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'TestItemListener'</code>
*   (of type <code>ifc.awt._XItemListener.TestItemListener</code>):
*   this <code>XItemListener</code> implementation must be
*   added to the object tested for checking
*   <code> itemStateChanged()</code> method call. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XItemListener
*/
public class _XItemListener extends MultiMethodTest {

    public XItemListener oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    * and stores event passed.
    */
    public static class TestItemListener implements com.sun.star.awt.XItemListener {
        public boolean itemStateChangedCalled = false ;
        public ItemEvent event = null ;

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            itemStateChangedCalled = true ;
            event = e ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {}

    }

    TestItemListener itemListener = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
    public void before() {
        itemListener = (TestItemListener) tEnv.getObjRelation("TestItemListener") ;
        if (itemListener == null)
            throw new StatusException(Status.failed("Relation not found")) ;
    }

    /**
    * First a <code>ItemEvent</code> object created and
    * it is passed to <code>itemStateChanged</code> method
    * call. Then a short wait follows for listener already
    * registered at the object to be caled. <p>
    * Has <b> OK </b> status if the listener was called with
    * the same <code>ItemEvent</code> object as was created
    * before.
    */
    public void _itemStateChanged() {

        boolean result = true ;

        ItemEvent event = new ItemEvent() ;
        event.Selected = 1 ;
        event.Highlighted = 2 ;
        oObj.itemStateChanged(event) ;

        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {}

        result = itemListener.itemStateChangedCalled &&
            itemListener.event.Selected == 1 &&
            itemListener.event.Highlighted == 2 ;

        tRes.tested("itemStateChanged()", result) ;
    }

}


