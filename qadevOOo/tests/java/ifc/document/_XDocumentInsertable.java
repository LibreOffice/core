/*************************************************************************
 *
 *  $RCSfile: _XDocumentInsertable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:26:07 $
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
* Test is <b> NOT </b> multithread compilant. <p>
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
         * Must be overriden to check if document was
         * successfully inserted.
         * @return <code>true</code> if document was inserted.
         */
        public abstract boolean isInserted() ;
        /**
         * Can be overriden to specify different than default
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
    public void before() {
        checker = (InsertChecker)
            tEnv.getObjRelation("XDocumentInsertable.Checker") ;

        if (checker == null) {
            log.println("Relaion not found, trying to query for "+
                "XTextRange ...") ;
            range = (XTextRange)
                UnoRuntime.queryInterface (XTextRange.class, oObj) ;
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
    * checked for existance of loaded document content. In case
    * if relation was found, then its <code>isInserted</code>
    * method is used to check insertion.<p>
    *
    * Has <b> OK </b> status if insertion was completed successfully
    * and no exceptions were thrown. <p>
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
            log.println("Exception occured while testing "+
                "insertDocumentFromURL()");
            ex.printStackTrace(log);
            result = false ;
        } catch (com.sun.star.io.IOException ex) {
            log.println("Exception occured while testing "+
                "insertDocumentFromURL()");
            ex.printStackTrace(log);
            result = false ;
        }

        tRes.tested("insertDocumentFromURL()", result);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }
}  // finish class _XDocumentInsertable

