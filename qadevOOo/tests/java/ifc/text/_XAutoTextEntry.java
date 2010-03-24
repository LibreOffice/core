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
 * Test is <b> NOT </b> multithread compilant. <p>
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


