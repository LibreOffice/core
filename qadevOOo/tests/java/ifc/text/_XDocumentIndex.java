/*************************************************************************
 *
 *  $RCSfile: _XDocumentIndex.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:15:49 $
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


