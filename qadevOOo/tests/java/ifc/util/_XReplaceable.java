/*************************************************************************
 *
 *  $RCSfile: _XReplaceable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:31:33 $
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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XReplaceDescriptor;
import com.sun.star.util.XReplaceable;
import com.sun.star.util.XSearchDescriptor;

/**
 * Testing <code>com.sun.star.util.XReplaceable</code>
 * interface methods :
 * <ul>
 *  <li><code> createReplaceDescriptor()</code></li>
 *  <li><code> replaceAll()</code></li>
 * </ul> <p>
 *
 * The requipment for the tested object is that it
 * <b>must containt</b> string 'xTextDoc'. Only
 * in that case this interface is tested correctly. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XReplaceable
 */
public class _XReplaceable extends MultiMethodTest {

    public XReplaceable oObj = null;
    public XReplaceDescriptor Rdesc = null;

    /**
     * Creates the descriptor for replacing string 'xTextDoc'
     * with string '** xTextDoc'. <p>
     * Has <b> OK </b> status if the returned descriptor is not
     * <code>null</code>. <p>
     */
    public void _createReplaceDescriptor() {

        log.println("testing createReplaceDescriptor() ... ");

        Rdesc = oObj.createReplaceDescriptor();
        Rdesc.setSearchString("xTextDoc");
        Rdesc.setReplaceString("** xTextDoc");
        tRes.tested("createReplaceDescriptor()", Rdesc != null);

    }

    /**
     * Replaces the text using descriptor created before. Then
     * search is performed in the target text. <p>
     *
     * Has <b> OK </b> status if the string '**' is found in
     * the text. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> createReplaceDescriptor() </code> : replace
     *    descriptor is created. </li>
     * </ul>
     */
    public void _replaceAll() {
        requiredMethod("createReplaceDescriptor()");
        oObj.replaceAll(Rdesc);
        XSearchDescriptor SDesc = oObj.createSearchDescriptor();
        SDesc.setSearchString("**");
        boolean res = (oObj.findFirst(SDesc) != null);
        tRes.tested("replaceAll()",res);
    }

}

