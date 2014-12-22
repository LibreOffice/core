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
 * Test is <b> NOT </b> multithread compliant. <p>
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

            XMultiServiceFactory xDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

            Object idxMark = xDocMSF.createInstance
                ("com.sun.star.text.DocumentIndexMark");
            XTextContent xTC = UnoRuntime.queryInterface
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

        util.utils.pause(1000);


        String contentAfter = oObj.getAnchor().getString();
        log.println("Content after: '" + contentAfter + "'");

        bOK &= !contentAfter.equals(contentBefore);
        bOK &= contentAfter.indexOf("IndexMark") > -1;

        tRes.tested("update()",bOK);
    }



}  // finish class _XDocumentIndex


