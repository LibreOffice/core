/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XContentProviderFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:31:19 $
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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XContentProviderFactory;


/**
* Testing <code>com.sun.star.ucb.XContentProviderFactory</code>
* interface methods :
* <ul>
*  <li><code> createContentProvider()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XContentProviderFactory
*/
public class _XContentProviderFactory extends MultiMethodTest {

    public static XContentProviderFactory oObj = null;

    /**
     * Creates a propvider. <p>
     * Has <b> OK </b> status if the returned value is not
     * <code>null</code>. <p>
     */
    public void _createContentProvider() {
        XContentProvider CP = oObj.createContentProvider
            ("com.sun.star.ucb.ContentProviderFactory");
        tRes.tested("createContentProvider()",CP != null);
    }
}

