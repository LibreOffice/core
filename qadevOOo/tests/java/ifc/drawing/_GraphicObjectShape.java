/*************************************************************************
 *
 *  $RCSfile: _GraphicObjectShape.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:28:40 $
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

package ifc.drawing;

import lib.MultiPropertyTest;

import com.sun.star.awt.XBitmap;
import com.sun.star.container.XIndexContainer;

/**
* Testing <code>com.sun.star.drawing.GraphicObjectShape</code>
* service properties :
* <ul>
*  <li><code> GraphicURL</code></li>
*  <li><code> GraphicStreamURL</code></li>
*  <li><code> GraphicObjectFillBitmap</code></li>
*  <li><code> AdjustLuminance</code></li>
*  <li><code> AdjustContrast</code></li>
*  <li><code> AdjustRed</code></li>
*  <li><code> AdjustGreen</code></li>
*  <li><code> AdjustBlue</code></li>
*  <li><code> Gamma</code></li>
*  <li><code> Transparency</code></li>
*  <li><code> GraphicColorMode</code></li>
*  <li><code> ImageMap</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Bitmap1', 'Bitmap2'</code> (of type <code>XBitmap</code>):
*   values to be set for property GraphicObjectFillBitmap </li>
*  <li> <code>'IMAP'</code>
*   (<code>com.sun.star.image.ImageMapRectangleObject</code> service instance):
*   is used to be added to ImageMap container.</li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.GraphicObjectShape
*/
public class _GraphicObjectShape extends MultiPropertyTest {

    /**
     * Property tester which changes URL.
     */
    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    /**
     * Property tester which switches two XBitmap objects.
     * It also uses two object relations with bitmaps.
     */
    protected PropertyTester BitmapTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue){
            XBitmap aBitmap1=(XBitmap) tEnv.getObjRelation("Bitmap1");
            XBitmap aBitmap2=(XBitmap) tEnv.getObjRelation("Bitmap2");
            if (oldValue.equals(aBitmap1)) return aBitmap2;
            else return aBitmap1;
        }
    } ;

    public XIndexContainer set = null;

    /**
     * Property tester which returns new <code>XIndexAccess</code> object.
     */
    protected PropertyTester ImapTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            return set;
        }
    } ;


    /**
     * This property must have URL format
     */
    public void _GraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicURL", URLTester) ;
    }

    public void _GraphicStreamURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicStreamURL", URLTester) ;
    }

    public void _GraphicObjectFillBitmap() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicObjectFillBitmap", BitmapTester) ;
    }

    /**
     * The test first retrieves ImageMap relation, then inserts it
     * to the current container.
     */
    public void _ImageMap() {
        if (! util.utils.hasPropertyByName(oObj,"ImageMap")) {
            log.println("optional property 'ImageMap' isn't available");
            tRes.tested("ImageMap",true);
            return;
        }
        try {
            Object imap = tEnv.getObjRelation("IMAP");
            XIndexContainer get = (XIndexContainer)
                oObj.getPropertyValue("ImageMap");
            set = get;
            set.insertByIndex(0,imap);
            testProperty("ImageMap", ImapTester) ;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking 'ImageMap'");
            e.printStackTrace(log);
            tRes.tested("ImageMap",false);
        }
    }

}

