/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XStatusIndicatorFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:10:07 $
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

package ifc.task;

import lib.MultiMethodTest;

import com.sun.star.task.XStatusIndicator;
import com.sun.star.task.XStatusIndicatorFactory;

/**
* Testing <code>com.sun.star.task.XStatusIndicatorFactory</code>
* interface methods :
* <ul>
*  <li><code> createStatusIndicator()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.task.XStatusIndicatorFactory
*/
public class _XStatusIndicatorFactory extends MultiMethodTest {

    public XStatusIndicatorFactory    oObj = null;

    /**
     * A status indicator created. Also some actions performed
     * with it, which are not related to test flow. <p>
     * Has <b> OK </b> status if the method returns not null
     * value. <p>
     */
    public void _createStatusIndicator() {
        boolean bResult = true;

        XStatusIndicator SI1 = oObj.createStatusIndicator();
        XStatusIndicator SI2 = oObj.createStatusIndicator();
        //Start all Indicators
        SI1.start("Status1",100);
        SI2.start("Status2",100);

        //change SI2
        SI2.setText("A new name for the status");
        SI2.setValue(45);
        SI2.reset();
        SI2.end();

        //Now SI1 is active
        SI1.end();

        bResult &= SI1 != null;

        tRes.tested("createStatusIndicator()", bResult);
    }
}


