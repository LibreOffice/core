/*************************************************************************
 *
 *  $RCSfile: _XDocumentInfo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:25:57 $
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

import com.sun.star.document.XDocumentInfo;
import com.sun.star.lang.ArrayIndexOutOfBoundsException;

/**
* Testing <code>com.sun.star.document.XDocumentInfo</code>
* interface methods :
* <ul>
*  <li><code> getUserFieldCount()</code></li>
*  <li><code> getUserFieldName()</code></li>
*  <li><code> setUserFieldName()</code></li>
*  <li><code> getUserFieldValue()</code></li>
*  <li><code> setUserFieldValue()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XDocumentInfo
*/
public class _XDocumentInfo extends MultiMethodTest {

    public XDocumentInfo oObj = null;

    short fieldCount;

    /**
    * Gets user field count. <p>
    * Has <b> OK </b> status if count is positive.
    */
    public void _getUserFieldCount() {
        fieldCount = oObj.getUserFieldCount();
        tRes.tested("getUserFieldCount()", fieldCount >= 0);
    }

    String[] oldNames;
    String[] oldValues;

    /**
    * Retrieves all user field names and stores them. <p>
    * Has <b> OK </b> status if no exceptions were thrown and
    * names returned are not <code>null</code> values. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getUserFieldCount() </code> : to obtain number of
    *    fields </li>
    * </ul>
    */
    public void _getUserFieldName() {
        requiredMethod("getUserFieldCount()");

        oldNames = new String[fieldCount];

        for (short i = 0; i < fieldCount; i++) {
            try {
                oldNames[i] = oObj.getUserFieldName(i);
                if (oldNames[i] == null) {
                    tRes.tested("getUserFieldName()", false);
                    return;
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                log.println("Couldn't get an user field name at " + i);
                tRes.tested("getUserFieldName()", false);
                return;
            }
        }

        tRes.tested("getUserFieldName()", true);
    }

    /**
    * For each field its name changed and the checked if it's properly
    * changed. Finally old names are restored.<p>
    * Has <b> OK </b> status if names were properly changed. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getUserFieldName() </code> : to retrieve old names </li>
    * </ul>
    */
    public void _setUserFieldName() {
        requiredMethod("getUserFieldName()");

        for (short i = 0; i < fieldCount; i++) {
            String newName = oldNames[i] + "_new";

            try {
                oObj.setUserFieldName(i, newName);
            } catch (ArrayIndexOutOfBoundsException e) {
                log.println("Couldn't set an user field name at " + i);
                tRes.tested("setUserFieldName()", false);
                return;
            }

            try {
                if (!newName.equals(oObj.getUserFieldName(i))) {
                    tRes.tested("setUserFieldName()", false);
                    return;
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                log.println("Couldn't set an user field name at " + i);
                tRes.tested("setUserFieldName()", false);
                return;
            } finally {
                try {
                    oObj.setUserFieldName(i, oldNames[i]);
                } catch (ArrayIndexOutOfBoundsException e) {
                    log.println("Couldn't restore an user field name at " + i);
                    tRes.tested("setUserFieldName()", false);
                    return;
                }
            }
        }

        tRes.tested("setUserFieldName()", true);
    }


    /**
    * Retrieves all user field values and stores them. <p>
    * Has <b> OK </b> status if no exceptions were thrown and
    * values returned are not <code>null</code> values. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getUserFieldCount() </code> : to obtain number of
    *    fields </li>
    * </ul>
    */
    public void _getUserFieldValue() {
        requiredMethod("getUserFieldCount()");

        oldValues = new String[fieldCount];

        for (short i = 0; i < fieldCount; i++) {
            try {
                oldValues[i] = oObj.getUserFieldValue(i);
                if (oldValues[i] == null) {
                    tRes.tested("getUserFieldValue()", false);
                    return;
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                log.println("Couldn't get an user field value at " + i);
                tRes.tested("getUserFieldValue()", false);
                return;
            }
        }

        tRes.tested("getUserFieldValue()", true);
    }

    /**
    * For each field its value changed and the checked if it's properly
    * changed. Finally old values are restored.<p>
    * Has <b> OK </b> status if values were properly changed. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getUserFieldValue() </code> : to retrieve old values. </li>
    * </ul>
    */
    public void _setUserFieldValue() {
        requiredMethod("getUserFieldValue()");

        for (short i = 0; i < fieldCount; i++) {
            String newValue = oldValues[i] + "_new";

            try {
                oObj.setUserFieldValue(i, newValue);
            } catch (ArrayIndexOutOfBoundsException e) {
                log.println("Couldn't set an user field value at " + i);
                tRes.tested("setUserFieldValue()", false);
                return;
            }

            try {
                if (!newValue.equals(oObj.getUserFieldValue(i))) {
                    tRes.tested("setUserFieldValue()", false);
                    return;
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                log.println("Couldn't set an user field value at " + i);
                tRes.tested("setUserFieldValue()", false);
                return;
            } finally {
                try {
                    oObj.setUserFieldValue(i, oldNames[i]);
                } catch (ArrayIndexOutOfBoundsException e) {
                    log.println("Couldn't restore an user field value at " + i);
                    tRes.tested("setUserFieldValue()", false);
                    return;
                }
            }
        }

        tRes.tested("setUserFieldValue()", true);
    }

}  // finish class _XDocumentInfo

