/*************************************************************************
 *
 *  $RCSfile: _XDocumentHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:33:57 $
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

package ifc.xml.sax;

import java.io.PrintWriter;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.XMLTools;

import com.sun.star.xml.sax.SAXException;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XLocator;

/**
* Testing <code>com.sun.star.xml.sax.XDocumentHandler</code>
* interface methods :
* <ul>
*  <li><code> startDocument()</code></li>
*  <li><code> endDocument()</code></li>
*  <li><code> startElement()</code></li>
*  <li><code> endElement()</code></li>
*  <li><code> characters()</code></li>
*  <li><code> ignorableWhitespace()</code></li>
*  <li><code> processingInstruction()</code></li>
*  <li><code> setDocumentLocator()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XDocumentHandler.XMLData'</code> (of type <code>String[][]
*   </code>):the XML data which will be passed to the handler. Each
*   array of strings corresponds to some handler event. The fisrt
*   string of event array is the type of the event they can have
*   the following values :
*   <ul>
*    <li>'start' : startElement() event. The string with index 1
*     is the name of element, the next array elements are attributes
*     of XML element in order Name, Type, Value, Name, Type, Value, etc.
*    </li>
*    <li>'end' : endElement() event. The string with index 1
*     is the name of element. </li>
*    <li>'chars' : characters() event. The string with index 1
*     is characters. </li>
*    <li>'spaces' : ignorableWhitespace() event. The string with index 1
*     is spaces. </li>
*    <li>'instruct' : processingInstruction() event. The string with
*     index 1 is the target of instruction. The string with index
*     2 is the data of instruction. </li>
*   </ul> </li>
*   <li> <code>'XDocumentHandler.ImportChecker'</code>
*    (of type <code>ifc.xml.sax._XDocumentHandler.ImportChecker</code>) :
*    this relation must be implementation of the interface above
*    ant it must check if the XML data was successfully imported to
*    the document. </li>
*  </li>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.xml.sax.XDocumentHandler
*/
public class _XDocumentHandler extends MultiMethodTest {

    private static class DocumentLocator implements XLocator {
        public boolean aMethodCalled = false ;

        private PrintWriter log = null ;
        public DocumentLocator(PrintWriter log) {
            this.log = log ;
        }
        public int getColumnNumber() {
            log.println("getColumnNumber() method called.") ;
            aMethodCalled = true ;
            return 10 ;
        }
        public int getLineNumber() {
            log.println("getLineNumber() method called.") ;
            aMethodCalled = true ;
            return 9 ;
        }
        public String getPublicId() {
            log.println("getPublicId() method called.") ;
            aMethodCalled = true ;
            return "file://d:/file.txt";
        }
        public String getSystemId() {
            log.println("getSystemId() method called.") ;
            aMethodCalled = true ;
            return "system";
        }
    }

    /**
    * This interface implementation must be passed by component test
    * for checking the whole import process.
    */
    public static interface ImportChecker {
        /**
        * Returns <code>true</code> if the XML data was successfully
        * imported, <code>false</code> in other case.
        */
        boolean checkImport() ;
    }

    public XDocumentHandler oObj = null;
    private String[][] xmlData = null ;
    private DocumentLocator locator = null ;
    private ImportChecker checker = null ;
    private boolean locatorResult = true ;
    private SAXException locatorException = null ;
    private boolean ToBeSkipped = false;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        locator = new DocumentLocator(log) ;
        if (tEnv.getTestCase().getObjectName().equals("XMLSettingsImporter")) {
            log.println("Settings can't be imported in the current Implementation");
            ToBeSkipped = true;
        }
        xmlData = (String[][])tEnv.getObjRelation("XDocumentHandler.XMLData") ;
        checker = (ImportChecker)
            tEnv.getObjRelation("XDocumentHandler.ImportChecker") ;

        if (xmlData == null || checker == null) throw new StatusException
            (Status.failed("Relation wasn't found")) ;
    }

    /**
      * Sets document locator to dummy locator implementation and
         * calls the <code>startDocument</code> method. <p>
         *
     * Has <b> OK </b> status if no runtime exceptions occured.
     */
    public void _startDocument() {
        if (ToBeSkipped) {
            tRes.tested("startDocument()", Status.skipped(true));
            return;
        }

        try {
            oObj.setDocumentLocator(locator) ;
        } catch (SAXException e) {
            locatorException = e ;
            locatorResult = false ;
        }

        boolean result = true ;
        try {
            oObj.startDocument() ;
        } catch (SAXException e) {
            e.printStackTrace(log) ;
            log.println("Wrapped exception :" + e.WrappedException) ;
            result = false ;
        }

        tRes.tested("startDocument()", result) ;
    }

        /**
         * This test is finally executed. It finishes XML data
         * transfering with <code>endDocument</code> method call. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during
         * the whole transfering and if the appropriate changes
         * occured in the document where XML data was trnsfered to.
         * This check is performed by checker relation.
         */
    public void _endDocument() {
        if (ToBeSkipped) {
            tRes.tested("endDocument()", Status.skipped(true));
            return;
        }
        requiredMethod("startElement()") ;
        executeMethod("endElement()") ;
        executeMethod("characters()") ;
        executeMethod("ignorableWhitespace()") ;
        executeMethod("processingInstruction()") ;

        boolean result = true ;
        try {
            oObj.endDocument() ;
        } catch (SAXException e) {
            e.printStackTrace(log) ;
            log.println("Wrapped exception :" + e.WrappedException) ;
            result = false ;
        }

        log.println("Check if import was successful ...") ;
        result &= checker.checkImport() ;

        tRes.tested("endDocument()", result) ;
    }

        /**
         * Transfers XML data obtained from relation
         * <code>'XDocumentHandler.XMLData'</code>. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during XML data
         * transfering in <code>startDocument</code> and
         * <code>startElement</code> method tests. <p>
         *
         * Exact checking of XML transfer is made in <code>endDocument</code>
         */
    public void _startElement() {
        if (ToBeSkipped) {
            tRes.tested("startElement()", Status.skipped(true));
            return;
        }
        boolean result = true ;

        try {
            log.println("StartElement Processing XML data ...") ;
            for(int i = 0; i < xmlData.length; i++) {
                String[] elem = xmlData[i] ;
                String xmlTag = "" ;
                if ("start".equals(elem[0])) {
                    xmlTag += "<" ;
                    String tagName = elem[1] ;
                    xmlTag += tagName ;
                    XMLTools.AttributeList attr = new XMLTools.AttributeList() ;
                    for (int j = 2; j < elem.length; j+=3) {
                        attr.add(elem[j], elem[j+1], elem[j+2]);
                        xmlTag += " " + elem[j] + "(" + elem[j+1] +
                            ")=\"" + elem[j+2] + "\"" ;
                    }
                    xmlTag += ">" ;

                    log.println(xmlTag) ;
                    oObj.startElement(tagName, attr) ;
                } else
                if ("end".equals(elem[0])) {
                    log.println("</" + elem[1] + ">") ;
                    oObj.endElement(elem[1]) ;
                } else
                if ("chars".equals(elem[0])) {
                    log.println("'" + elem[1] + "'") ;
                    oObj.characters(elem[1]) ;
                } else
                if ("spaces".equals(elem[0])) {
                    log.println("(spaces)'" + elem[1] + "'") ;
                    oObj.ignorableWhitespace(elem[1]) ;
                } else
                if ("instruct".equals(elem[0])) {
                    log.println("<?" + elem[1] + " " + elem[2] + "?>") ;
                    oObj.processingInstruction(elem[1], elem[2]) ;
                } else {
                    log.println("!!! Bad object relation !!!") ;
                    throw new StatusException(Status.failed("Bad relation")) ;
                }
            }
        } catch (SAXException e) {
            e.printStackTrace(log) ;
            log.println("Wrapped exception :" + e.WrappedException) ;
            result = false ;
        }

        tRes.tested("startElement()", result) ;
    }

        /**
         * Does nothing. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during XML data
         * transfering in <code>startDocument</code> and
         * <code>startElement</code> method tests.
         */
    public void _endElement() {
        if (ToBeSkipped) {
            tRes.tested("endElement()", Status.skipped(true));
            return;
        }
        requiredMethod("startElement()") ;

        boolean result = true ;

        tRes.tested("endElement()", result) ;
    }

        /**
         * Does nothing. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during XML data
         * transfering in <code>startDocument</code> and
         * <code>startElement</code> method tests.
         */
    public void _characters() {
        if (ToBeSkipped) {
            tRes.tested("characters()", Status.skipped(true));
            return;
        }
        requiredMethod("startElement()") ;

        boolean result = true ;

        tRes.tested("characters()", result) ;
    }

        /**
         * Does nothing. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during XML data
         * transfering in <code>startDocument</code> and
         * <code>startElement</code> method tests.
         */
    public void _ignorableWhitespace() {
        if (ToBeSkipped) {
            tRes.tested("ignorableWhitespace()", Status.skipped(true));
            return;
        }
        requiredMethod("startElement()") ;

        boolean result = true ;

        tRes.tested("ignorableWhitespace()", result) ;
    }

        /**
         * Does nothing. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during XML data
         * transfering in <code>startDocument</code> and
         * <code>startElement</code> method tests.
         */
    public void _processingInstruction() {
        if (ToBeSkipped) {
            tRes.tested("processingInstruction()", Status.skipped(true));
            return;
        }
        requiredMethod("startElement()") ;

        boolean result = true ;

        tRes.tested("processingInstruction()", result) ;
    }

        /**
         * Does nothing. <p>
         *
         * Has <b>OK</b> status if no exceptions occured during XML data
         * transfering in <code>startDocument</code> and
         * <code>startElement</code> method tests.
         */
    public void _setDocumentLocator() {
        if (ToBeSkipped) {
            tRes.tested("setDocumentLocator()", Status.skipped(true));
            return;
        }
        executeMethod("endDocument()") ;

        boolean result = locatorResult ;
        if (locatorException != null) {
            log.println("Exception occured during setDocumentLocator() call:") ;
            locatorException.printStackTrace(log) ;
            log.println("Wrapped exception :"
                + locatorException.WrappedException) ;
            result = false ;
        }

        tRes.tested("setDocumentLocator()", result) ;
    }

}


