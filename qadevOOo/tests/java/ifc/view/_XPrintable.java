/*************************************************************************
 *
 *  $RCSfile: _XPrintable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:33:02 $
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

package ifc.view;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.view.PaperOrientation;
import com.sun.star.view.XPrintable;

/**
 * Testing <code>com.sun.star.view.XPrintable</code>
 * interface methods :
 * <ul>
 *  <li><code> getPrinter()</code></li>
 *  <li><code> setPrinter()</code></li>
 *  <li><code> print()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.view.XPrintable
 */
public class _XPrintable extends MultiMethodTest {

    public XPrintable oObj = null;
    public PropertyValue[] the_printer = null;

    /**
     * Test calls the method and stores returned value. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getPrinter(){

        the_printer = oObj.getPrinter();
        tRes.tested("getPrinter()",the_printer != null);
    } // finish _getPrinter

    /**
     * Changes <code>PaperOrientation</code> property in the old
     * printer configuration and sets changed value as a new printer.<p>
     *
     * Has <b> OK </b> status if the <code>getPrinter</code> method
     * retursn printer with changed property. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getPrinter() </code> : to change one property
     *   in existing printer configuration. </li>
     * </ul>
     */
    public void _setPrinter(){
        requiredMethod("getPrinter()");
        int propIdx = 0 ;
        while (!"PaperOrientation".equals(the_printer[propIdx].Name)) {
            propIdx++ ;
        }
        PaperOrientation newVal = null ;
        if (the_printer[propIdx].Value == PaperOrientation.PORTRAIT)
            newVal = PaperOrientation.LANDSCAPE ;
        else
            newVal = PaperOrientation.PORTRAIT ;

        the_printer[propIdx].Value = newVal ;

        try {
            oObj.setPrinter(the_printer);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("couldn't set printer");
            ex.printStackTrace(log);
            tRes.tested("setPrinter()",false);
        }

        //oObj.setPrinter(the_printer);
        the_printer = oObj.getPrinter() ;

        propIdx = 0 ;
        while (!"PaperOrientation".equals(the_printer[propIdx].Name)) {
            propIdx++ ;
        }

        boolean the_same = the_printer[propIdx].Value == newVal;
        tRes.tested("setPrinter()", the_same);

    } // finish _setPrinter

    /**
     * Printing performed into file in SOffice temp directory.
     * First this file is deleted if it already exist (using
     * <code>com.sun.star.ucb.SimpleFileAccess</code> service.
     * After that the method with appropriate parameter is
     * called.<p>
     *
     * Has <b> OK </b> status if the file to which printing is made
     * exists. <p>
     *
     * @throws StatusException if service
     * <code>com.sun.star.ucb.SimpleFileAccess</code> cann't be
     * created.
     */
    public void _print(){
        boolean result = true ;

        final String file = "XPrintable.prt" ;
        final String fileName = utils.getOfficeTempDirSys((XMultiServiceFactory)tParam.getMSF())+file ;
        final String fileURL = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) + file ;

        XSimpleFileAccess fAcc = null ;
        try {
            Object oFAcc =
                ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.ucb.SimpleFileAccess") ;
            fAcc = (XSimpleFileAccess) UnoRuntime.queryInterface
                (XSimpleFileAccess.class, oFAcc) ;
            if (fAcc == null) throw new StatusException
                (Status.failed("Can't create SimpleFileAccess service")) ;
            if (fAcc.exists(fileURL)) {
                log.println("Old file exists and will be deleted");
                fAcc.kill(fileURL);
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error accessing file '" + fileURL + "'");
            e.printStackTrace(log);
        }

        try {
            PropertyValue[] PrintOptions = new PropertyValue[2];
            PropertyValue firstProp = new PropertyValue();
            firstProp.Name = "FileName";
            log.println("Printing to :"+fileName);
            firstProp.Value = fileName;
            firstProp.State = com.sun.star.beans.PropertyState.DEFAULT_VALUE;
            PrintOptions[0] = firstProp;
            PrintOptions[1] = new PropertyValue();
            PrintOptions[1].Name = "Wait";
            PrintOptions[1].Value = new Boolean(true);
            oObj.print(PrintOptions);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("couldn't print");
            ex.printStackTrace(log);
            result = false ;
        }

        java.io.File the_file = new java.io.File(fileName);
        log.println("File "+fileName+" exists = "+the_file.exists());

        try {
            if (result) {
                result &= fAcc.exists(fileURL) ;
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error while while checking file '" +
                fileURL + "': ");
            e.printStackTrace(log);
            result = false ;
        }

        tRes.tested("print()", result) ;

    } // finish _print

}  // finish class _XPrintable


