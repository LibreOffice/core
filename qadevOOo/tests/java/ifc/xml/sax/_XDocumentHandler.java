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
*   array of strings corresponds to some handler event. The first
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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.xml.sax.XDocumentHandler
*/
public class _XDocumentHandler extends MultiMethodTest {

    private static class DocumentLocator implements XLocator {
        private final PrintWriter log;
        public DocumentLocator(PrintWriter log) {
            this.log = log ;
        }
        public int getColumnNumber() {
            log.println("getColumnNumber() method called.") ;
            return 10 ;
        }
        public int getLineNumber() {
            log.println("getLineNumber() method called.") ;
            return 9 ;
        }
        public String getPublicId() {
            log.println("getPublicId() method called.") ;
            return "file://d:/file.txt";
        }
        public String getSystemId() {
            log.println("getSystemId() method called.") ;
            return "system";
        }
    }

    /**
    * This interface implementation must be passed by component test
    * for checking the whole import process.
    */
    public interface ImportChecker {
        /**
        * Returns <code>true</code> if the XML data was successfully
        * imported, <code>false</code> in other case.
        */
        boolean checkImport() ;
    }

    /**
    * This interface implementation must be passed by component test
    * for setting a target document to the import process
    */
    public interface TargetDocumentSetter {

        void setTargetDocument();
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
    @Override
    public void before() {
        locator = new DocumentLocator(log) ;
        if (tEnv.getTestCase().getObjectName().equals("XMLSettingsImporter")) {
            log.println("Settings can't be imported in the current Implementation");
            ToBeSkipped = true;
        }
        xmlData = (String[][])tEnv.getObjRelation("XDocumentHandler.XMLData") ;
        checker = (ImportChecker)
            tEnv.getObjRelation("XDocumentHandler.ImportChecker") ;

        TargetDocumentSetter targetDocSet = (TargetDocumentSetter)
            tEnv.getObjRelation("XDocumentHandler.TargetDocumentSetter");

        if (xmlData == null || checker == null) throw new StatusException
            (Status.failed("Relation wasn't found")) ;

        if (targetDocSet == null){
            log.println("object relation 'XDocumentHandler.TargetDocumentSetter' not used.");
            log.println("be sure that the test have a target to write throu");
        }
    }

    /**
      * Sets document locator to dummy locator implementation and
         * calls the <code>startDocument</code> method. <p>
         *
     * Has <b> OK </b> status if no runtime exceptions occurred.
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
         * transferring with <code>endDocument</code> method call. <p>
         *
         * Has <b>OK</b> status if no exceptions occurred during
         * the whole transferring and if the appropriate changes
         * occurred in the document where XML data was trnsfered to.
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
         * Has <b>OK</b> status if no exceptions occurred during XML data
         * transferring in <code>startDocument</code> and
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
         * Has <b>OK</b> status if no exceptions occurred during XML data
         * transferring in <code>startDocument</code> and
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
         * Has <b>OK</b> status if no exceptions occurred during XML data
         * transferring in <code>startDocument</code> and
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
         * Has <b>OK</b> status if no exceptions occurred during XML data
         * transferring in <code>startDocument</code> and
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
         * Has <b>OK</b> status if no exceptions occurred during XML data
         * transferring in <code>startDocument</code> and
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
         * Has <b>OK</b> status if no exceptions occurred during XML data
         * transferring in <code>startDocument</code> and
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
            log.println("Exception occurred during setDocumentLocator() call:") ;
            locatorException.printStackTrace(log) ;
            log.println("Wrapped exception :"
                + locatorException.WrappedException) ;
            result = false ;
        }

        tRes.tested("setDocumentLocator()", result) ;
    }

}

