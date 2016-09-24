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
import lib.Status;
import lib.StatusException;

import com.sun.star.text.XAutoTextEntry;
import com.sun.star.text.XTextDocument;

/**
 * Testing <code>com.sun.star.text.XAutoTextEntry</code>
 * interface methods :
 * <ul>
 *  <li><code> applyTo()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'TEXTDOC'</code> (of type <code>XTextDocument</code>):
 *   the text document for creating a text range.</li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XAutoTextEntry
 */
public class _XAutoTextEntry extends MultiMethodTest {

    public XAutoTextEntry oObj = null;

    /**
     * Applies the entry to a range created for the text document obtained from
     * relation 'TEXTDOC'. <p>
     * Has <b> OK </b> status if text of the document was changed. <p>
     */
    public void _applyTo() {
        XTextDocument textDoc = (XTextDocument)tEnv.getObjRelation("TEXTDOC");
        if (textDoc == null) {
            throw new StatusException
                (Status.failed("Couldn't get relation 'TEXTDOC'"));
        }

        String oldText = textDoc.getText().getString();
        oObj.applyTo(textDoc.getText().createTextCursor());
        String newText = textDoc.getText().getString();

        log.println("Old text:\n" + oldText + "\nNew text:\n" + newText);

        tRes.tested("applyTo()", ! newText.equals(oldText));
    }

}  // finish class _XAutoTextEntry


