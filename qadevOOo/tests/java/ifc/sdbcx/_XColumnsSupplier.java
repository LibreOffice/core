/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XColumnsSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:33:04 $
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

package ifc.sdbcx;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XColumnsSupplier;

/**
* Testing <code>com.sun.star.sdbcx.XColumnsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getColumns()</code></li>
* </ul> <p>
* @see com.sun.star.sdbcx.XColumnsSupplier
*/
public class _XColumnsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XColumnsSupplier oObj = null ;

    /**
    * Has OK status if method returns not null
    * <code>XNameAccess</code> object, FAILED otherwise.
    */
    public void _getColumns() {

        XNameAccess cols = oObj.getColumns() ;

        /*
        String[] colNames = cols.getElementNames() ;

        log.println("Column names:") ;
        for (int i =0 ; i < colNames.length; i++) {
            log.println("  " + colNames[i]) ;
        }

        XServiceInfo info = null ;
        try {
            info = (XServiceInfo) UnoRuntime.queryInterface
                (XServiceInfo.class, cols.getByName("colNames[0]")) ;
        } catch (com.sun.star.uno.Exception e) {}

        log.println("Support : " +
            info.supportsService("com.sun.star.sdbcx.Column")) ;
        */

        tRes.tested("getColumns()", cols != null) ;
    }

}  // finish class _XColumnsSupplier


