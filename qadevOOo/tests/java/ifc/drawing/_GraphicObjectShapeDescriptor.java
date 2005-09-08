/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _GraphicObjectShapeDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:34:00 $
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

package ifc.drawing;

import lib.MultiPropertyTest;

import com.sun.star.awt.XBitmap;

public class _GraphicObjectShapeDescriptor extends MultiPropertyTest {

    protected boolean compare(Object ob1, Object ob2) {

        return super.compare(ob1, ob2);

    }

    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    protected PropertyTester BitmapTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            XBitmap aBitmap1=(XBitmap) tEnv.getObjRelation("Bitmap1");
            XBitmap aBitmap2=(XBitmap) tEnv.getObjRelation("Bitmap2");
            if (oldValue.equals(aBitmap1)) return aBitmap2; else return aBitmap1;
        }
    } ;

    public void _GraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicURL", URLTester) ;
    }

    public void _GraphicObjectFillBitmap() {
        log.println("Testing with custom Property tester") ;
        testProperty("GraphicObjectFillBitmap", BitmapTester) ;
    }

}

