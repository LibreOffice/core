/*************************************************************************
 *
 *  $RCSfile: _XFramesSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:39:43 $
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
                   log.println("Exception occured while calling getByIndex() method :") ;
                   e.printStackTrace(log) ;
                   return;
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                   log.println("Exception occured while calling getByIndex() method :") ;
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
                    log.println("Exception occured while calling getByIndex() method :") ;
                    e.printStackTrace(log) ;
                    return;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.println("Exception occured while calling getByIndex() method :") ;
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
                    log.println("Exception occured while calling getByIndex() method :") ;
                    e.printStackTrace(log) ;
                    return;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.println("Exception occured while calling getByIndex() method :") ;
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

