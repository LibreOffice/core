/*************************************************************************
 *
 *  $RCSfile: _Shape.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:29:28 $
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

import com.sun.star.style.XStyle;

/**
* Testing <code>com.sun.star.drawing.Shape</code>
* service properties :
* <ul>
*  <li><code> ZOrder</code></li>
*  <li><code> LayerID</code></li>
*  <li><code> LayerName</code></li>
*  <li><code> Printable</code></li>
*  <li><code> MoveProtect</code></li>
*  <li><code> Name</code></li>
*  <li><code> SizeProtect</code></li>
*  <li><code> Style</code></li>
*  <li><code> Transformation</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Style1', 'Style2'</code>
*   (of type <code>com.sun.star.style.XStyle</code>):
*    relations used to change property 'Style'</li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.Shape
*/
public class _Shape extends MultiPropertyTest {

    XStyle style1 = null;
    XStyle style2 = null;

    /**
     * Custom tester which switches between two styles.
     */
    protected PropertyTester StyleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (util.ValueComparer.equalValue(oldValue,style1))
                return style2; else
                return style1;
        }
    } ;

    /**
     * Custom tester for 'LayerName' property which switches two
     * Strings ('layout' and 'controls').
     */
    protected PropertyTester StringTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (util.ValueComparer.equalValue(oldValue,"layout"))
                return "controls"; else
                return "layout";
        }
    } ;

    /**
     * Tested with custom StyleTester which switches between two
     * styles.
     */
    public void _Style() {
        log.println("Testing with custom Property tester") ;
        style1 = (XStyle) tEnv.getObjRelation("Style1");
        style2 = (XStyle) tEnv.getObjRelation("Style2");
        testProperty("Style", StyleTester) ;
    }

    /**
     * Tested with custom StringTester which switches two
     * Strings ('layout' and 'controls')
     */
    public void _LayerName() {
        if (tEnv.getTestCase().getObjectName().equals("SwXShape")) {
            log.println("There is only one Layer for SwXShape");
            log.println("Therefore this property can't be changed");
            tRes.tested("LayerName",true);
        } else {
            log.println("Testing with custom Property tester") ;
            testProperty("LayerName", StringTester) ;
        }
    }

    public void _ZOrder() {
        testProperty("ZOrder", new Integer(0), new Integer(1));
    }


}

