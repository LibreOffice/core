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

package ifc.frame;

import lib.MultiMethodTest;

import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.frame.XFramesSupplier</code>
* interface methods:
* <ul>
*  <li><code> getActiveFrame() </code></li>
*  <li><code> getFrames() </code></li>
*  <li><code> setActiveFrame() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.frame.XFramesSupplier
*/
public class _XFramesSupplier extends MultiMethodTest {
    public static XFramesSupplier oObj = null;
    protected XIndexAccess frames = null ;
    protected XFrame active = null ;
    protected int activeIdx = -1 ;

    /**
    * Test calls the method, then result is checked. Also active frame index
    * is saved in activeIdx variable.<p>
    *
    * Has <b> OK </b> status if the method does not return null and the object
    * contains returned frame. Or if no frames available and the method
    * returns null.<p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFrames() </code> : obtains frames from the object </li>
    * </ul>
    */
    public void _getActiveFrame() {
        boolean result = true ;

        requiredMethod("getFrames()") ;
        active = oObj.getActiveFrame() ;
        if (active == null) {
            // if no child frames then no active frame could be
            result =  oObj.getFrames().getCount() == 0;
            log.println("getActiveFrame() returned null") ;
        }
        else {
            boolean hasActiveFrame = false ;
            for (int i = 0; i < frames.getCount(); i++) {
                XFrame fr = null ;
                try {
                   try {
                       fr = (XFrame) AnyConverter.toObject(
                                new Type(XFrame.class),frames.getByIndex(i));
                   } catch (com.sun.star.lang.IllegalArgumentException iae) {
                       log.println("Can't convert any");
                   }
                } catch (com.sun.star.lang.WrappedTargetException e) {
                   log.println("Exception occurred while calling getByIndex() method :") ;
                   e.printStackTrace(log) ;
                   return;
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                   log.println("Exception occurred while calling getByIndex() method :") ;
                   e.printStackTrace(log) ;
                   return;
                }
                if (active.equals(fr)) {
                    hasActiveFrame = true ;
                    activeIdx = i ;
                }
            }
            if (!hasActiveFrame) {
                log.println("getActiveFrame() isn't contained " +
                    "in getFrames() collection") ;
                result = false ;
            }
        }

        tRes.tested("getActiveFrame()", result) ;
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method does not return null,
    * number of returned frames is not zero and each of them is not null too.
    */
    public void _getFrames() {
        boolean result = true ;
        int cnt = 0;

        frames = oObj.getFrames() ;
        if (frames != null) {
            cnt = frames.getCount() ;
            log.println("There are " + cnt + " frames.") ;
        } else {
            log.println("getFrames() returned null !!!") ;
            result = false ;
        }
        for (int i = 0; i < cnt; i++) {
            try {
                if (frames.getByIndex(i) == null) {
                    log.println("Frame(" + i + ") == null") ;
                    result = false ;
                }
            } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while calling getByIndex() method :") ;
                    e.printStackTrace(log) ;
                    return;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.println("Exception occurred while calling getByIndex() method :") ;
                    e.printStackTrace(log) ;
                    return;
            }
        }

        tRes.tested("getFrames()", result) ;
    }

    /**
    * After selecting frame to be activated, test calls the method. <p>
    *
    * Has <b> OK </b> status if set and gotten active frames are equal.<p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getActiveFrame() </code> : gets active frame </li>
    * </ul>
    */
    public void _setActiveFrame() {
        XFrame sFrame = null ;

        requiredMethod("getActiveFrame()") ;
        if (frames.getCount() > 1) {
            try {
                if (activeIdx != 0)
                   try {
                       sFrame = (XFrame) AnyConverter.toObject(
                                new Type(XFrame.class),frames.getByIndex(0));
                   } catch (com.sun.star.lang.IllegalArgumentException iae) {
                       log.println("Can't convert any");
                   }
                else
                   try {
                       sFrame = (XFrame) AnyConverter.toObject(
                                new Type(XFrame.class),frames.getByIndex(1));
                   } catch (com.sun.star.lang.IllegalArgumentException iae) {
                       log.println("Can't convert any");
                   }
            } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while calling getByIndex() method :") ;
                    e.printStackTrace(log) ;
                    return;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.println("Exception occurred while calling getByIndex() method :") ;
                    e.printStackTrace(log) ;
                    return;
            }
        } else if (frames.getCount() > 0) {
            sFrame = active ;
        }

        oObj.setActiveFrame(sFrame) ;
        XFrame gFrame = oObj.getActiveFrame() ;

        boolean result;
        if (gFrame == null && sFrame == null)
            result = true;
        else if (gFrame != null && sFrame != null)
            result = sFrame.equals(gFrame);
        else
            result = false;

        if (!result) {
            log.println("Active frame set is not equal frame get: FAILED");
            result = false ;
        }

        tRes.tested("setActiveFrame()", result) ;
    }

} // finished class _XFramesSupplier

