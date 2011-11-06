/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


