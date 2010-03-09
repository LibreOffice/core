/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XDocumentIndex;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;

/**
 * Testing <code>com.sun.star.text.XDocumentIndex</code>
 * interface methods :
 * <ul>
 *  <li><code> getServiceName()</code></li>
 *  <li><code> update()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'TextDoc'</code> (of type <code>XTextDocument</code>):
 *   the text document for creating and inserting index mark.</li>
 * <ul> <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XDocumentIndex
 */
public class _XDocumentIndex extends MultiMethodTest {

    public XDocumentIndex oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the retruned service name
     * is equal to 'com.sun.star.text.DocumentIndex'.
     */
    public void _getServiceName() {
        String serv = oObj.getServiceName();
        tRes.tested("getServiceName()",
            serv.equals("com.sun.star.text.DocumentIndex"));
    }

    /**
     * Gets the document from relation and insert a new index mark.
     * Then it stores the text content of document index before
     * update and after.<p>
     *
     * Has <b> OK </b> status if index content is changed and
     * new index contains index mark inserted. <p>
     */
     public void _update() {
        boolean bOK = true;

        try {
            XTextDocument xTextDoc = (XTextDocument)
                tEnv.getObjRelation("TextDoc");
            XText xText = xTextDoc.getText();
            XTextRange xTR = xText.getEnd();
            xTR.setString("IndexMark");

            XMultiServiceFactory xDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

            Object idxMark = xDocMSF.createInstance
                ("com.sun.star.text.DocumentIndexMark");
            XTextContent xTC = (XTextContent) UnoRuntime.queryInterface
                (XTextContent.class, idxMark);
            xText.insertTextContent(xTR, xTC, true);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't insert index mark.");
            e.printStackTrace(log);
            bOK = false ;
        }

        String contentBefore = oObj.getAnchor().getString();
        log.println("Content before: '" + contentBefore + "'");

        oObj.update();

        try {
            Thread.sleep(1000);
        }
        catch (InterruptedException ex) {
        }


        String contentAfter = oObj.getAnchor().getString();
        log.println("Content after: '" + contentAfter + "'");

        bOK &= !contentAfter.equals(contentBefore);
        bOK &= contentAfter.indexOf("IndexMark") > -1;

        tRes.tested("update()",bOK);
    }



}  // finish class _XDocumentIndex


