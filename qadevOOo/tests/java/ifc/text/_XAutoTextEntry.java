/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAutoTextEntry.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:18:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


