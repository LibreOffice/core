/*************************************************************************
 *
 *  $RCSfile: _XDataFormatTranslator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:24:20 $
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

package ifc.datatransfer;

import lib.MultiMethodTest;

import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.XDataFormatTranslator;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.datatransfer.XDataFormatTranslator</code>
* interface methods :
* <ul>
*  <li><code> getSystemDataTypeFromDataFlavor()</code></li>
*  <li><code> getDataFlavorFromSystemDataType()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.datatransfer.XDataFormatTranslator
*/
public class _XDataFormatTranslator extends MultiMethodTest {

    public XDataFormatTranslator oObj = null;

    /**
    * Get a system data type for 'text/html' MIME type. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned.
    */
    public void _getSystemDataTypeFromDataFlavor() {
        DataFlavor df = new DataFlavor
            ("text/html","HTML-Documents", new Type());
        Object res = oObj.getSystemDataTypeFromDataFlavor(df);
        tRes.tested("getSystemDataTypeFromDataFlavor()",res != null);
    }

    /**
     * Gets data flavour from system data type, which was gotten
     * from 'text/html' MIME type. <p>
     * Has <b>OK</b> status if DataFlavour returned has 'text/hrml' MIME
     * type.
     */
    public void _getDataFlavorFromSystemDataType() {
        DataFlavor in = new DataFlavor
            ("text/html","HTML-Documents", new Type());
        Object res = oObj.getSystemDataTypeFromDataFlavor(in);
        DataFlavor out = oObj.getDataFlavorFromSystemDataType(res);
        tRes.tested("getDataFlavorFromSystemDataType()",
            out.MimeType.equals("text/html"));
    }

}

