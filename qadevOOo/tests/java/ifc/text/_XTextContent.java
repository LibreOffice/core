/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextContent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:24:46 $
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

import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextRange;

/**
 * Testing <code>com.sun.star.text.XTextContent</code>
 * interface methods :
 * <ul>
 *  <li><code> attach()</code></li>
 *  <li><code> getAnchor()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'CONTENT'</code> <b>optional</b>
 *  (of type <code>XTextContent</code>):
 *   if this relation exists than it is used as the
 *   tested object. </li>
 *  <li> <code>'TEXT'</code> <b>optional</b>
 *  (of type <code>XText</code>):
 *   the relation must be specified if the 'CONTENT'
 *   relation exists. From this relation an anchor
 *   for <code>attach()</code> method is obtained.</li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextContent
 */
public class _XTextContent extends MultiMethodTest {
    public XTextContent oObj = null;
    public XTextRange oAnchor = null;

    /**
     * Tries to get the anchor of the text content
     * an XTextRange is returned. <p>
     * The test is OK if an not null text range is returned
     */
    public void _getAnchor() {
        log.println("getAnchor()");
        oAnchor = oObj.getAnchor();
        tRes.tested("getAnchor()", oAnchor != null ) ;

    } // end getAnchor()

    /**
     * Tries to attach the text content to the test range
     * gotten with getAnchor(). If relations are found
     * then they are are used for testing. <p>
     *
     * The test is OK if the method works without error.
     * @see #_getAnchor()
     */
    public void _attach() {
        requiredMethod("getAnchor()");
        try {
            XTextContent aContent = (XTextContent) tEnv.getObjRelation("CONTENT");
            XTextRange aRange = (XTextRange) tEnv.getObjRelation("RANGE");

            if ( aContent !=null) {
                aContent.attach(aRange);
            } else {
                oObj.attach(aRange);
            }
            tRes.tested("attach()", true ) ;
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            String noAttach = (String) tEnv.getObjRelation("NoAttach");
            if (noAttach != null) {
                log.println("Exception expected for "+noAttach);
                log.println("This Component doesn't support attach");
                tRes.tested("attach()",true);
            } else {
                ex.printStackTrace(log);
                tRes.tested("attach()",false);
            }
        } catch (com.sun.star.uno.RuntimeException re) {
            String noAttach = (String) tEnv.getObjRelation("NoAttach");
            if (noAttach != null) {
                log.println("Exception expected for "+noAttach);
                log.println("This Component doesn't support attach");
                tRes.tested("attach()",true);
            } else {
                re.printStackTrace(log);
                tRes.tested("attach()",false);
            }
        }
    }
}

