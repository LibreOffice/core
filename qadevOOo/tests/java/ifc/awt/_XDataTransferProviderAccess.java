/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.awt;


import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.awt.XDataTransferProviderAccess;
import com.sun.star.awt.XWindow;
import com.sun.star.datatransfer.clipboard.XClipboard;
import com.sun.star.datatransfer.dnd.XDragGestureRecognizer;
import com.sun.star.datatransfer.dnd.XDragSource;
import com.sun.star.datatransfer.dnd.XDropTarget;

/**
* Testing <code>com.sun.star.awt.XDataTransferProviderAccess</code>
* interface methods :
* <ul>
*  <li><code> getDragGestureRecognizer()</code></li>
*  <li><code> getDragSource()</code></li>
*  <li><code> getDropTarget()</code></li>
*  <li><code> getClipboard()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XDataTransferProviderAccess.XWindow'</code>
*  (of type <code>com.sun.star.awt.XWindow</code>):
*   this window must created by the Toolkit tested. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XDataTransferProviderAccess
*/
public class _XDataTransferProviderAccess extends MultiMethodTest {

    public XDataTransferProviderAccess oObj = null;
    protected XWindow win = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        win = (XWindow) tEnv.getObjRelation
            ("XDataTransferProviderAccess.XWindow") ;
        if (win == null) throw new StatusException(Status.failed
            ("Relation not found")) ;
    }

    /**
    * Tries to get gesture recognizer for the window passed as
    * relation. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned
    */
    public void _getDragGestureRecognizer() {

        boolean result = true ;
        XDragGestureRecognizer rec = oObj.getDragGestureRecognizer(win) ;

        result = rec != null ;

        tRes.tested("getDragGestureRecognizer()", result) ;
    }

    /**
    * Tries to get drag source for the window passed as
    * relation. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned
    */
    public void _getDragSource() {

        boolean result = true ;
        XDragSource src = oObj.getDragSource(win) ;

        result = src != null ;

        tRes.tested("getDragSource()", result) ;
    }

    /**
    * Tries to get drop target for the window passed as
    * relation. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned
    */
    public void _getDropTarget() {

        boolean result = true ;
        XDropTarget targ = oObj.getDropTarget(win) ;

        result = targ != null ;

        tRes.tested("getDropTarget()", result) ;
    }

    /**
    * Tries to obtain default clipboard.<p>
    * Has <b> OK </b> status if not <code>null</code> value returned.
    */
    public void _getClipboard() {

        boolean result = true ;
        XClipboard cb = oObj.getClipboard("") ;

        result = cb != null ;

        tRes.tested("getClipboard()", result) ;
    }
}

