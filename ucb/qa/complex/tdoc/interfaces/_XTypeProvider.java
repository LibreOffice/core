/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTypeProvider.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:09:31 $
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

package complex.tdoc.interfaces;

import lib.MultiMethodTest;

import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import share.LogWriter;

/**
* Testing <code>com.sun.star.lang.XTypeProvider</code>
* interface methods :
* <ul>
*  <li><code> getTypes()</code></li>
*  <li><code> getImplementationId()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.lang.XTypeProvider
*/
public class _XTypeProvider {

    public static XTypeProvider oObj = null;
    public static Type[] types = null;
    public LogWriter log = null;

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public boolean _getImplementationId() {
        boolean result = true;
        log.println("testing getImplementationId() ... ");

        log.println("The ImplementationId is "+oObj.getImplementationId());
        result = true;

        return result;

    } // end getImplementationId()


    /**
    * ?alls the method and checks the return value.<p>
    * Has <b>OK</b> status if one of the return value equals to the
    * type <code>com.sun.star.lang.XTypeProvider</code>.
    */
    public boolean _getTypes() {
        boolean result = false;
        log.println("getting Types...");
        types = oObj.getTypes();
        for (int i=0;i<types.length;i++) {
            int k = i+1;
            log.println(k+". Type is "+types[i].toString());
            if (types[i].toString().equals
                    ("Type[com.sun.star.lang.XTypeProvider]")) {
                result = true;
            }
        }
        if (!result) {
            log.println("Component must provide Type "
                +"<com.sun.star.lang.XTypeProvider>");
        }

        return result;

    } // end getTypes()

}

