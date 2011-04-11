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
* Test is <b> NOT </b> multithread compilant. <p>
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
                   fr = null;
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
//            if (cnt == 0) result = false ;
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
        boolean result = true ;
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
        } else {
            sFrame = null;
        }

        oObj.setActiveFrame(sFrame) ;
        XFrame gFrame = oObj.getActiveFrame() ;
        if (!(gFrame == null && sFrame == null
              || sFrame.equals(gFrame))) {

            log.println("Active frame set is not equal frame get: FAILED");
            result = false ;
        }

        tRes.tested("setActiveFrame()", result) ;
    }

} // finished class _XFramesSupplier

