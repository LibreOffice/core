/*************************************************************************
 *
 *  $RCSfile: _TextGraphicObject.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:14:00 $
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

package ifc.text;

import java.util.Random;

import lib.MultiPropertyTest;
import util.utils;

import com.sun.star.awt.Point;
import com.sun.star.container.XIndexContainer;

/**
 * Testing <code>com.sun.star.text.TextGraphicObject</code>
 * service properties :
 * <ul>
 *  <li><code> ImageMap</code></li>
 *  <li><code> ContentProtected</code></li>
 *  <li><code> SurroundContour</code></li>
 *  <li><code> ContourOutside</code></li>
 *  <li><code> ContourPolyPolygon</code></li>
 *  <li><code> GraphicCrop</code></li>
 *  <li><code> HoriMirroredOnEvenPages</code></li>
 *  <li><code> HoriMirroredOnOddPages</code></li>
 *  <li><code> VertMirrored</code></li>
 *  <li><code> GraphicURL</code></li>
 *  <li><code> GraphicFilter</code></li>
 *  <li><code> ActualSize</code></li>
 *  <li><code> AdjustLuminance</code></li>
 *  <li><code> AdjustContrast</code></li>
 *  <li><code> AdjustRed</code></li>
 *  <li><code> AdjustGreen</code></li>
 *  <li><code> AdjustBlue</code></li>
 *  <li><code> Gamma</code></li>
 *  <li><code> GraphicIsInverted</code></li>
 *  <li><code> Transparency</code></li>
 *  <li><code> GraphicColorMode</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ImageMap'</code> (an inmplementation of
 *  <code>com.sun.star.image.ImageMapObject</code>):
 *   is used to insert a new Map into collection
 *   from 'ImageMap' property. </li>
 * <ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextGraphicObject
 */
public class _TextGraphicObject extends MultiPropertyTest {

    public Random rdm = new Random();

    /**
     * The tester which can change a sequence of <code>Point</code>'s
     * or create a new one if necessary.
     */
    protected PropertyTester PointTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (utils.isVoid(oldValue))
                return newPoint(); else
                return changePoint((Point[][]) oldValue);
        }
    } ;

    /**
     * Tested with custom <code>PointTester</code>.
     */
    public void _ContourPolyPolygon() {
        log.println("Testing with custom Property tester") ;
        testProperty("ContourPolyPolygon", PointTester) ;
    }

    /**
     * Retrieves an ImageMap from relation and inserts it to the collection
     * obtained as property value. Then this collection is set back.
     * After that property value is get again. The number of elements
     * in the old collection and in just gotten collection is checked.
     *
     * Has <b>OK</b> status if the number of elements in the new obtained
     * collection is greater than in old one.
     */
    public void _ImageMap() {
        try {
            XIndexContainer imgMap = (XIndexContainer)
                oObj.getPropertyValue("ImageMap");
            int previous = imgMap.getCount();
            log.println("Count (previous) "+previous);
            Object im = tEnv.getObjRelation("IMGMAP");
            imgMap.insertByIndex(0,im);
            oObj.setPropertyValue("ImageMap",imgMap);
            imgMap = (XIndexContainer) oObj.getPropertyValue("ImageMap");
            int after = imgMap.getCount();
            log.println("Count (after) "+after);
            tRes.tested("ImageMap",previous < after);
        }
        catch (Exception ex) {}

    }

    /**
     * Creates a new random points sequence.
     */
    public Point[][] newPoint() {
        Point[][] res = new Point[1][185];
        for (int i=0; i<res[0].length; i++) {
            res[0][i] = new Point();
            res[0][i].X = rd()*rd()*rd();
            res[0][i].Y = rd()*rd()*rd();;
        }
        return res;
    }

    public int rd() {
        return rdm.nextInt(6);
    }

    /**
     * Changes the existing point sequence.
     */
    public Point[][] changePoint(Point[][] oldPoint) {
        Point[][] res = oldPoint;
        for (int i=0; i<res[0].length; i++) {
            res[0][i].X += 1;
            res[0][i].Y += 1;
        }
        return res;
    }

}  // finish class _TextGraphicObject


