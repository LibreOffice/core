/*************************************************************************
 *
 *  $RCSfile: _XTypeDetection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:27:39 $
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

package ifc.document;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XTypeDetection;

/**
 * Testing <code>com.sun.star.document.XTypeDetection</code>
 * interface methods :
 * <ul>
 *  <li><code> queryTypeByURL()</code></li>
 *  <li><code> queryTypeByDescriptor()</code></li>
 * </ul> <p>
 *
 * The following predefined files needed to complete the test:
 * <ul>
 *  <li> <code>XTypeDetection.sxw</code> : <b>Calc</b>
 *      document which extension is <b>sxw</b>. </li>
 * <ul> <p>
 *
 * @see com.sun.star.document.XTypeDetection
 */
public class _XTypeDetection extends MultiMethodTest {
    public XTypeDetection oObj = null;

    private String docURL = null;

    /**
     * Get the document URL.
     */
    public void before() {
        docURL = utils.getFullTestURL("XTypeDetection.sxw");
    }

    /**
      * Tries to detect type by writer document URL. <p>
      *
      * Has <b> OK </b> status if type returned contains
      * 'writer' as substring.
      */
    public void _queryTypeByURL() {

        boolean result = true ;
        String type = oObj.queryTypeByURL(docURL) ;
        result &= type.indexOf("writer") > -1;

        tRes.tested("queryTypeByURL()", result) ;
    }

    /**
     * Tries to detect type of the document using <i>flat</i>
     * and <i>deep</i> detection. For flat detection a writer type
     * must be returned (file has writer extension), but deep
     * detection must return calc type (document has calc contents)<p>
     *
     * Has <b> OK </b> status if for the first case type contains
     * 'writer' string and for the second 'calc' string.
     */
    public void _queryTypeByDescriptor() {
        boolean result = true ;

        PropertyValue[][] mediaDescr = new PropertyValue[1][1];
        mediaDescr[0][0] = new PropertyValue();
        mediaDescr[0][0].Name = "URL";
        mediaDescr[0][0].Value = docURL;

        String type = oObj.queryTypeByDescriptor(mediaDescr, false);
        result &= type.indexOf("writer") > -1;

        type = oObj.queryTypeByDescriptor(mediaDescr, true);
        result &= type.indexOf("calc") > -1;

        tRes.tested("queryTypeByDescriptor()", result) ;
    }
}

