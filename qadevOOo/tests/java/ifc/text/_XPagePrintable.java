/*************************************************************************
 *
 *  $RCSfile: _XPagePrintable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:17:28 $
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

package ifc.text;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XPagePrintable;

/**
 * Testing <code>com.sun.star.text.XPagePrintable</code>
 * interface methods :
 * <ul>
 *  <li><code> getPagePrintSettings()</code></li>
 *  <li><code> setPagePrintSettings()</code></li>
 *  <li><code> printPages()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XPagePrintable
 */
public class _XPagePrintable extends MultiMethodTest {

    public static XPagePrintable oObj = null;
    public PropertyValue[] PrintSettings = new PropertyValue[0];

    /**
     * Types of print settings properties by order they returned by
     * <code>getPagePrintSettings()</code>.
     */
    public String[] types = new String[]{"Short","Short","Integer","Integer",
        "Integer","Integer","Integer","Integer","Boolean"};

    /**
     * Calls the method and examines the returned array of properties. <p>
     *
     * Has <b>OK</b> status if all properties' types are correspond
     * to their expected values of the <code>types</code> array.
     *
     * @see #types
     */
    public void _getPagePrintSettings() {
        boolean res = true;
        PrintSettings = oObj.getPagePrintSettings();

        for (int i=0;i<PrintSettings.length;i++) {
            String the_type = PrintSettings[i].Value.getClass().toString();
            if (!the_type.endsWith(types[i])) {
                log.println("Name: "+PrintSettings[i].Name);
                log.println("Value: "+PrintSettings[i].Value);
                log.println("Type"+the_type);
                log.println("Expected: java.lang."+types[i]);
                res = false;
            }
        }

        tRes.tested("getPagePrintSettings()",res);
    }

    /**
     * Changes a property 'IsLandscape' in existsing print settings,
     * and sets these settings back. <p>
     *
     * Has <b>OK</b> status if settings gotten again has the changed
     * 'IsLandscape' property value. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getPagePrintSettings() </code> : to have existing
     *   print settings. </li>
     * </ul>
     */
    public void _setPagePrintSettings() {
        requiredMethod("getPagePrintSettings()");
        boolean res = true;

        Boolean landscape = (Boolean) PrintSettings[8].Value;
        Boolean newlandscape = new Boolean(!landscape.booleanValue());
        PrintSettings[8].Value = newlandscape;
        oObj.setPagePrintSettings(PrintSettings);
        res = (oObj.getPagePrintSettings()[8].Value.equals(newlandscape));

        tRes.tested("setPagePrintSettings()",res);
    }

    /**
     * Creates print options for printing into file situated in the SOffice
     * temporary directory. If the file already exists it is deleted.
     * Then calls the method. <p>
     *
     * Has <b>OK</b> status if the file to which printing must be performed
     * is exists.
     */
    public void _printPages() {
        boolean res = true;

        try {
            PropertyValue[] PrintOptions = new PropertyValue[1];
            PropertyValue firstProp = new PropertyValue();
            firstProp.Name = "FileName";
            log.println("Printing to :"+utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF())+
                "XPagePrintable.prt");
            firstProp.Value = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF())+
                "XPagePrintable.prt";
            firstProp.State = com.sun.star.beans.PropertyState.DEFAULT_VALUE;
            PrintOptions[0] = firstProp;
            java.io.File aFile = new java.io.File(utils.getOfficeTempDir(
                (XMultiServiceFactory)tParam.getMSF())+"XPagePrintable.prt");
            if (aFile.exists()) aFile.delete() ;
            oObj.printPages(PrintOptions);
            res = aFile.exists();
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'printPages'");
            res = false;
            ex.printStackTrace(log);
        }

        tRes.tested("printPages()",res);
    }

}  // finish class _XPagePrintable

