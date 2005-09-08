/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _RowSet.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:29:00 $
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

package ifc.sdbc;

import lib.MultiPropertyTest;

import com.sun.star.uno.Any;


/**
* Testing <code>com.sun.star.sdbc.RowSet</code>
* service properties :
* <ul>
*  <li><code> DataSourceName</code></li>
*  <li><code> URL</code></li>
*  <li><code> Command</code></li>
*  <li><code> TransactionIsolation</code></li>
*  <li><code> TypeMap</code></li>
*  <li><code> EscapeProcessing</code></li>
*  <li><code> QueryTimeOut</code></li>
*  <li><code> MaxFieldSize</code></li>
*  <li><code> MaxRows</code></li>
*  <li><code> User</code></li>
*  <li><code> Password</code></li>
*  <li><code> ResultSetType</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sdbc.RowSet
*/
public class _RowSet extends MultiPropertyTest {

    /**
    * Redefined method returns object, that contains changed property value.
    */
    public void _TypeMap() {
        boolean result = false;
        try {
            Any TypeMap = (Any) oObj.getPropertyValue("TypeMap");
            String TypeName = TypeMap.getType().getTypeName();
            String expected = "com.sun.star.container.XNameAccess";
            result = TypeName.equals(expected);
            if (! result ) {
                log.println("Expected Type is "+expected);
                log.println("but the returned is "+TypeName);
            }
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception " + e.getMessage());
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception " + e.getMessage());
        }

        tRes.tested("TypeMap",result);
    }

}  // finish class _RowSet

