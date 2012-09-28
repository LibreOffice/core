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

package ifc.ucb;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;

import java.util.Calendar;
import java.util.GregorianCalendar;

/**
* Testing <code>com.sun.star.ucb.XSimpleFileAccess</code>
* interface methods. <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>XSimpleFileAccess/XSimpleFileAccess.txt</code> :
*   text file of length 17 and 2000 year created .</li>
*  <li> <code>XSimpleFileAccess/XSimpleFileAccess2.txt</code> :
*   text file for <code>openFileReadWrite</code> method test.</li>
* <ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'InteractionHandler'</code>
*  (of type <code>XInteractionHandler</code>)
*  instance of <code>com.sun.star.sdb.InteractionHandler</code>
*  </li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XSimpleFileAccess
*/
public class _XSimpleFileAccess extends MultiMethodTest {

    public static XSimpleFileAccess oObj = null;

    /**
    * Copies <b>XSimpleFileAccess.txt</b> to a new file, checks
    * if it was successfully copied and then deletes it. <p>
    * Has <b> OK </b> status if after method call new copy of file
    * exists and no exceptions were thrown. <p>
    */
    public void _copy() {
        try {
            String copiedFile = "";
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String filename = dirname+"XSimpleFileAccess.txt";
            copiedFile = dirnameTo + "XSimpleFileAccess_copy.txt";

            if (oObj.exists(copiedFile))
                oObj.kill(copiedFile);

            oObj.copy(filename,copiedFile);
            tRes.tested("copy()",oObj.exists(copiedFile));
            oObj.kill(copiedFile);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'copy()'");
            ex.printStackTrace(log);
            tRes.tested("copy()",false);
        }

    } //EOF copy()

    /**
    * Copies <b>XSimpleFileAccess.txt</b> to a new file, tries to
    * rename it, then checks
    * if it was successfully renamed and then deletes it. <p>
    * Has <b> OK </b> status if after method call new file
    * exists and no exceptions were thrown. <p>
    */
    public void _move() {
        try {
            String copiedFile = "";
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String filename = dirname+"XSimpleFileAccess.txt";
            copiedFile = dirnameTo + "XSimpleFileAccess_copy.txt";

            if (oObj.exists(copiedFile))
                oObj.kill(copiedFile);

            oObj.copy(filename,copiedFile);
            filename = copiedFile;
            copiedFile = dirnameTo + "XSimpleFileAccess_move.txt";
            oObj.move(filename,copiedFile);
            tRes.tested("move()",oObj.exists(copiedFile));
            oObj.kill(copiedFile);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'move()'");
            ex.printStackTrace(log);
            tRes.tested("move()",false);
        }

    } //EOF move()

    /**
    * Copies <b>XSimpleFileAccess.txt</b> to a new file, deletes it
    * and checks if it isn't exist. <p>
    * Has <b> OK </b> status if after method call new copy of file
    * doesn't exist and no exceptions were thrown. <p>
    */
    public void _kill() {
        try {
            String copiedFile = "";
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String filename = dirname+"XSimpleFileAccess.txt";
            copiedFile = dirnameTo + "XSimpleFileAccess_copy.txt";

            if (oObj.exists(copiedFile))
                oObj.kill(copiedFile);

            oObj.copy(filename,copiedFile);
            oObj.kill(copiedFile);
            tRes.tested("kill()",!oObj.exists(copiedFile));
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'kill()'");
            ex.printStackTrace(log);
            tRes.tested("kill()",false);
        }

    } //EOF kill()

    /**
    * Tries to check if <b>XSimpleFileAccess</b> is folder. <p>
    * Has <b>OK</b> status if the method returns <code>true</code>
    */
    public void _isFolder() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            tRes.tested("isFolder()",oObj.isFolder(dirname));
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'isFolder()'");
            ex.printStackTrace(log);
            tRes.tested("isFolder()",false);
        }

    } //EOF isFolder()

    /**
     * Copies <b>XSimpleFileAccess.txt</b> to a new file, sets
     * 'READONLY' attribute and checks it. Second clears 'READONLY'
     * attribute and checks it again. The copy of file is deleted
     * finally.<p>
     *
     * Has <b> OK </b> status if in the first case method returns
     * <code></code>, and in the second case - <code>false</code>
     * and no exceptions were thrown. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> setReadOnly </code> </li>
     * </ul>
     */
    public void _isReadOnly() {
        requiredMethod("setReadOnly()");
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            boolean result = true;

            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String readonlyCopy = dirnameTo + "XSimpleFileAccess_copy.txt" ;

            if (oObj.exists(readonlyCopy))
                oObj.kill(readonlyCopy);

            oObj.copy(filename, readonlyCopy);

            oObj.setReadOnly(readonlyCopy, true);
            result &= oObj.isReadOnly(readonlyCopy);
            oObj.setReadOnly(readonlyCopy, false);
            result &= !oObj.isReadOnly(readonlyCopy);

            oObj.kill(readonlyCopy);
            tRes.tested("isReadOnly()",result);
        } catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'isReadOnly()'");
            ex.printStackTrace(log);
            tRes.tested("isReadOnly()",false);
        }

    } //EOF isReadOnly()


    /**
     * Copies <b>XSimpleFileAccess.txt</b> to a new file, sets
     * 'READONLY' attribute and checks it. Second clears 'READONLY'
     * attribute and checks it again. The copy of file is deleted
     * finally.<p>
     *
     * Has <b> OK </b> status if in the first case method returns
     * <code></code>, and in the second case - <code>false</code>
     * and no exceptions were thrown. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> setReadOnly </code> </li>
     * </ul>
     */
    public void _setReadOnly() {
        boolean result = true ;

        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";

            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String readonlyCopy = dirnameTo + "XSimpleFileAccess_copy.txt" ;

            if (oObj.exists(readonlyCopy))
                oObj.kill(readonlyCopy);

            oObj.copy(filename, readonlyCopy);

            oObj.setReadOnly(readonlyCopy, true);
            result &= oObj.isReadOnly(readonlyCopy) ;
            oObj.setReadOnly(readonlyCopy, false);
            result &= !oObj.isReadOnly(readonlyCopy) ;
            tRes.tested("setReadOnly()", result);

            oObj.kill(readonlyCopy);
        }
        catch (Exception ex) {
            log.println("Exception occurred while testing 'setReadOnly()'");
            ex.printStackTrace(log);
            tRes.tested("setReadOnly()",false);
        }
    } //EOF setReadOnly()

    /**
     * Creates folder and then checks if it was successfully created. <p>
     * Has <b>OK</b> status if folder was created and no exceptions
     * were thrown.
     */
    public void _createFolder() {
        try {
            String tmpdirname = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String newFolder = tmpdirname+"SimpleSubFolder";

            if (oObj.exists(newFolder))
                oObj.kill(newFolder);

            oObj.createFolder(newFolder);
            tRes.tested("createFolder()",oObj.isFolder(newFolder));
            oObj.kill(newFolder);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'createFolder()'");
            ex.printStackTrace(log);
            tRes.tested("createFolder()",false);
        }

    } //EOF createFolder()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess.txt</b>
    * file tested.<p>
    * Has <b> OK </b> status if the method returns <code>17</code>
    * and no exceptions were thrown. <p>
    */
    public void _getSize() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            int fSize = oObj.getSize(filename);
            tRes.tested("getSize()", fSize == 17 );
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'getSize()'");
            ex.printStackTrace(log);
            tRes.tested("getSize()",false);
        }

    } //EOF getSize()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess.txt</b>
    * file tested.<p>
    * Has <b> OK </b> status if the method returns String
    * <code>'application/vnd.sun.staroffice.fsys-file'</code>
    * and no exceptions were thrown. <p>
    */
    public void _getContentType() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            String fType = oObj.getContentType(filename);
            tRes.tested("getContentType()",
                "application/vnd.sun.staroffice.fsys-file".equals(fType) );
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'getContentType()'");
            ex.printStackTrace(log);
            tRes.tested("getContentType()",false);
        }

    } //EOF getContentType()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess.txt</b>
    * file tested.<p>
    * Has <b> OK </b> status if the method returns date with
    * 2001 year and no exceptions were thrown. <p>
    */
    public void _getDateTimeModified() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            DateTime fTime = oObj.getDateTimeModified(filename);

            java.io.File the_file = new java.io.File(filename);
            long lastModified = the_file.lastModified();
            java.util.Date lastMod = new java.util.Date(lastModified);
            GregorianCalendar lastModCal = new GregorianCalendar();
            lastModCal.setTime(lastMod);

            //compare the dates gained by java with those gained by this method
            boolean res = true;
            boolean partResult = (fTime.Day == lastModCal.get(Calendar.DAY_OF_WEEK));
            if (!partResult) {
                log.println("Wrong Day");
                log.println("Expected: "+lastModCal.get(Calendar.DAY_OF_WEEK));
                log.println("Gained: "+fTime.Day);
                log.println("------------------------------");
            }
            partResult = (fTime.Month == lastModCal.get(Calendar.MONTH));
            if (!partResult) {
                log.println("Wrong Month");
                log.println("Expected: "+lastModCal.get(Calendar.MONTH));
                log.println("Gained: "+fTime.Month);
                log.println("------------------------------");
            }

            partResult = (fTime.Year == (lastModCal.get(Calendar.MONTH) - 1900));
            if (!partResult) {
                log.println("Wrong Year");
                log.println("Expected: "+(lastModCal.get(Calendar.MONTH) - 1900));
                log.println("Gained: "+fTime.Year);
                log.println("------------------------------");
            }

            tRes.tested("getDateTimeModified()", res);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'getDateTimeModified()'");
            ex.printStackTrace(log);
            tRes.tested("getDateTimeModified()",false);
        }

    } //EOF getDateTimeModified()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess</b>
    * directory used.<p>
    * Has <b> OK </b> status if the method returns non zero length
    * array and no exceptions were thrown. <p>
    */
    public void _getFolderContents() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String[] cont = oObj.getFolderContents(dirname,false);
            tRes.tested("getFolderContents()", cont.length>0);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'getFolderContents()'");
            ex.printStackTrace(log);
            tRes.tested("getFolderContents()",false);
        }

    } //EOF getFolderContents()

    /**
    * First it check file <b>XSimpleFileAccess.txt</b> for
    * existence, second file <b>I_do_not_exists.txt</b> is checked
    * for existence. <p>
    * Has <b> OK </b> status if in the first case method returns
    * <code>true</code> and in the second - <code>flase</code>
    * and no exceptions were thrown. <p>
    */
    public void _exists() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            String wrongname = dirname+"I_do_not_exists.txt";
            tRes.tested("exists()",
                oObj.exists(filename) && !oObj.exists(wrongname));
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'exists()'");
            ex.printStackTrace(log);
            tRes.tested("exists()",false);
        }

    } //EOF exists()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess.txt</b>
    * file used.<p>
    * Has <b> OK </b> status if the method returns not
    * <code>null</code> value and no exceptions were thrown. <p>
    */
    public void _openFileRead() {
        try {
            String dirname = util.utils.getFullTestURL("XSimpleFileAccess");
            String filename = dirname+"XSimpleFileAccess.txt";
            com.sun.star.io.XInputStream iStream = oObj.openFileRead(filename);
            tRes.tested("openFileRead()", iStream != null);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'openFileRead()'");
            ex.printStackTrace(log);
            tRes.tested("openFileRead()",false);
        }

    } //EOF openFileRead()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess.txt</b>
    * file used.<p>
    * Has <b> OK </b> status if the method returns not
    * <code>null</code> value and no exceptions were thrown. <p>
    */
    public void _openFileWrite() {
        try {
            String tmpdirname = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;

            String copiedFile = tmpdirname+"XSimpleFileAccess_openWrite.txt";

            if (oObj.exists(copiedFile))
                oObj.kill(copiedFile);

            com.sun.star.io.XOutputStream oStream =
                oObj.openFileWrite(copiedFile);
            tRes.tested("openFileWrite()", oStream != null);

            oStream.closeOutput();
            oObj.kill(copiedFile);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'openFileWrite()'");
            ex.printStackTrace(log);
            tRes.tested("openFileWrite()",false);
        }

    } //EOF openFileWrite()

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <b>XSimpleFileAccess2.txt</b>
    * file used.<p>
    * Has <b> OK </b> status if the method returns not
    * <code>null</code> value and no exceptions were thrown. <p>
    */
    public void _openFileReadWrite() {
        try {
            String dirnameTo = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
            String copiedFile = dirnameTo + "XSimpleFileAccess2.txt" ;

            if (oObj.exists(copiedFile))
                oObj.kill(copiedFile);

            com.sun.star.io.XStream aStream =
                oObj.openFileReadWrite(copiedFile);
            tRes.tested("openFileReadWrite()", aStream != null);

            aStream.getInputStream().closeInput();
            aStream.getOutputStream().closeOutput();

            oObj.kill(copiedFile);
        }
        catch (com.sun.star.uno.Exception ex) {
            log.println("Exception occurred while testing 'openFileReadWrite()'");
            ex.printStackTrace(log);
            tRes.tested("openFileReadWrite()",false);
        }

    } //EOF openFileReadWrite()

    /**
    * Test calls the method and checks that no exceptions were thrown.
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _setInteractionHandler() {
        XInteractionHandler handler = null;
        Object oHandler = tEnv.getObjRelation("InteractionHandler");

        if (oHandler == null)
            throw new StatusException
                (Status.failed("Reelation InteractionHandler not found"));

        try {
            handler = UnoRuntime.queryInterface
                (XInteractionHandler.class, oHandler);
            oObj.setInteractionHandler(handler);
            tRes.tested("setInteractionHandler()", true);
        } catch (Exception ex) {
            log.println("Exception occurred while testing 'setInteractionHandler()'");
            ex.printStackTrace(log);
            tRes.tested("setInteractionHandler()", false);
        }

    } //EOF setInteractionHandler()

}  // finish class _XSimpleFileAccess

