/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package ifc.view;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.utils;

import com.sun.star.beans.PropertyValue;
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
 * Test is <b> NOT </b> multithread compliant. <p>
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
     * returns printer with changed property. <p>
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
     * <code>com.sun.star.ucb.SimpleFileAccess</code> can't be
     * created.
     */
    public void _print() throws Exception {
        boolean result = true ;

        final String file = "XPrintable.prt" ;
        final String fileName = utils.getOfficeTempDirSys(tParam.getMSF())+file ;
        final String fileURL = utils.getOfficeTemp(tParam.getMSF()) + file ;

        Object oFAcc =
            tParam.getMSF().createInstance
            ("com.sun.star.ucb.SimpleFileAccess") ;
        XSimpleFileAccess fAcc = UnoRuntime.queryInterface
            (XSimpleFileAccess.class, oFAcc) ;
        if (fAcc == null) throw new StatusException
            (Status.failed("Can't create SimpleFileAccess service")) ;
        if (fAcc.exists(fileURL)) {
            log.println("Old file exists and will be deleted");
            fAcc.kill(fileURL);
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
            PrintOptions[1].Value = Boolean.TRUE;
            oObj.print(PrintOptions);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("couldn't print");
            ex.printStackTrace(log);
            result = false ;
        }

        try {
            boolean fileExists = fAcc.exists(fileURL);

            log.println("File "+fileName+" exists = "+fileExists);

            if (result) {
                result &= fileExists ;
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error while checking file '" +
                fileURL + "': ");
            e.printStackTrace(log);
            result = false ;
        }

        tRes.tested("print()", result) ;

    } // finish _print

}  // finish class _XPrintable


