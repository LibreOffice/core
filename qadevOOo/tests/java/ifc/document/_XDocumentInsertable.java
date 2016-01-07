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

package ifc.document;

import com.sun.star.io.IOException;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XDocumentInsertable;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;


/**
* Testing <code>com.sun.star.document.XDocumentInsertable</code>
* interface methods :
* <ul>
*  <li><code> insertDocumentFromURL()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XDocumentInsertable.Checker'</code>
*   (of type <code>_XDocumentInsertable.InsertChecker</code>)
*   <b> optional </b> :
*   relation for checking if document was inserted properly and
*   for obtaining document file name. For details see the class
*   description. If the relation doesn't exist default document
*   name is used, and <code>XTextRange</code> interface of
*   component is used for checking.</li>
* <ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>XDocumentInsertable.sxw</code> : StarWriter document
*    which content started with 'XDocumentInsertable test.' string.
*    The file is needed if no other file name specified by relation.
*    </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.document.XDocumentInsertable
*/
public class _XDocumentInsertable extends MultiMethodTest {

    public XDocumentInsertable oObj = null;
    protected XTextRange range = null ;
    protected static final String defaultFileName = "XDocumentInsertable.sxw" ;
    protected InsertChecker checker = null ;
    protected String fileName = defaultFileName ;

    /**
     * Abstract class for relation passing. It must check if
     * document was inserted successfully and can specify its
     * own document name to be inserted.
     */
    public static abstract class InsertChecker {
        /**
         * Must be overridden to check if document was
         * successfully inserted.
         * @return <code>true</code> if document was inserted.
         */
        public abstract boolean isInserted() ;
        /**
         * Can be overridden to specify different than default
         * document name. This document must be situated in
         * the test document disrectory, and its name must
         * be specified relational to this directory. By
         * default 'XDocumentInsertable.swx' file name returned.
         * @return File name of the document to be inserted.
         */
        public String getFileNameToInsert() {
            return defaultFileName ;
        }
    }

    /**
     * Retrieves object relation. If the relation is not found
     * then the object tested is tried to query <code>XTextRange</code>
     * interface for testing. If the relation is found then document name
     * for testing is retrieved, else the default one is used.
     *
     * @throws StatusException If neither relation found nor
     * <code>XTextRange</code> interface is queried.
     */
    @Override
    public void before() {
        checker = (InsertChecker)
            tEnv.getObjRelation("XDocumentInsertable.Checker") ;

        if (checker == null) {
            log.println("Relation not found, trying to query for "+
                "XTextRange ...") ;
            range = UnoRuntime.queryInterface (XTextRange.class, oObj) ;
            if (range == null) {
                log.println("XTextRange isn't supported by the component.");
                throw new StatusException(Status.failed
                    ("XTextRange isn't supported and relation not found")) ;
            }
        } else {
            fileName = checker.getFileNameToInsert();
        }
    }

    /**
    * Tries to insert document from URL specified by relation or
    * from default URL. If no relation was passed, text range is
    * checked for existence of loaded document content. In case
    * if relation was found, then its <code>isInserted</code>
    * method is used to check insertion.<p>
    * A Second test uses an invalid URL and checks for correct exceptions.
    *
    * Has <b> OK </b> status if at first insertion was completed successfully
    * and no exceptions were thrown and as second a expected exception was thrown. <p>
    */
    public void _insertDocumentFromURL() {
        boolean result = true ;

        try {
            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String docURL = utils.getFullTestURL(fileName) ;
            log.println("Inserting document from URL '" + docURL + "'");
            oObj.insertDocumentFromURL(docURL, szEmptyArgs);

            if (checker == null) {
                log.println("Checker is not specified, testing through "+
                    "XTextRange ...") ;
                String text = range.getString() ;
                log.println("Document text :\n" + text);
                log.println("---");
                result &= ( text.indexOf("XDocumentInsertable test.") >= 0 );
            } else {
                result &= checker.isInserted();
            }

        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while testing "+
                "insertDocumentFromURL()");
            ex.printStackTrace(log);
            result = false ;
        } catch (com.sun.star.io.IOException ex) {
            log.println("Exception occurred while testing "+
                "insertDocumentFromURL()");
            ex.printStackTrace(log);
            result = false ;
        }

        try {
            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String docURL = "file:///c:/ThisIsAnInvaldURL";
            log.println("Inserting document from URL '" + docURL + "'");
            oObj.insertDocumentFromURL(docURL, szEmptyArgs);

            result=false;

        } catch (IOException ex) {
            log.println("expected exception was thrown -> ok");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("expected exception was thrown -> ok");
        }


        tRes.tested("insertDocumentFromURL()", result);
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }
}  // finish class _XDocumentInsertable

