/*************************************************************************
 *
 *  $RCSfile: _TextEmbeddedObject.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2004-03-19 14:35:30 $
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

import lib.MultiPropertyTest;


/**
 * Testing <code>com.sun.star.text.TextEmbeddedObject</code>
 * service properties :
 * <ul>
 *  <li><code> CLSID</code></li>
 *  <li><code> Model</code></li>
 *  <li><code> Component</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextEmbeddedObject
 */
public class _TextEmbeddedObject extends MultiPropertyTest {

    /**
     * This property can't be set, as soon as the object is inserted
     * so the set method will be skipped.
     */
    public void _CLSID() {
        boolean result = false;

        try {
            String clsid = (String) oObj.getPropertyValue("CLSID");
            log.println("Getting " + clsid);
            log.println(
                    "According to the idl-description this property can't be set as soon as the Object is inserted in the document, so setting will be skipped");
            result=true;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while getting Property 'CLSID' " +
                        e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while getting Property 'CLSID' " +
                        e.getMessage());
        }

        tRes.tested("CLSID", result);
    }
} // finish class _TextEmbeddedObject
