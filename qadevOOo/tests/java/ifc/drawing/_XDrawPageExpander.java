/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDrawPageExpander.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:39:11 $
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

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageExpander;
import com.sun.star.drawing.XDrawPages;

/**
* Testing <code>com.sun.star.drawing.XDrawPageExpander</code>
* interface methods :
* <ul>
*  <li><code> expand()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPage'</code> (of type <code>XDrawPage</code>):
*   draw page to be expanded. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XDrawPageExpander
*/
public class _XDrawPageExpander extends MultiMethodTest {
    public XDrawPageExpander oObj = null;

    /**
    * First the relation is retrieved. Then the method is called.<p>
    * Has <b> OK </b> status if the method returns not null value. <p>
    */
    public void _expand() {
        boolean result = true;
        XDrawPage oDrawPage = (XDrawPage) tEnv.getObjRelation("DrawPage");
        if ( oDrawPage == null ) {
            // SKIPPED.FAILED - the ObjectRelation is not available
            Status.failed( "ObjectRelation('DrawPages') XDrawPages n.a.");
            return;
        }
        log.println("testing expand() ... ");

        XDrawPages pages = oObj.expand(oDrawPage);
        result = pages != null;

        tRes.tested("expand()", result);
    } // end expand
} // end DrawPageExpander

