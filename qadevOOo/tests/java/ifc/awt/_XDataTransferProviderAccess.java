/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XDataTransferProviderAccess
*/
public class _XDataTransferProviderAccess extends MultiMethodTest {

    public XDataTransferProviderAccess oObj = null;
    protected XWindow win = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
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

