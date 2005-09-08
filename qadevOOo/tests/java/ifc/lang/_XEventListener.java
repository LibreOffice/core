/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XEventListener.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:14:52 $
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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XEventListener;

/**
* Testing <code>com.sun.star.lang.XEventListener</code>
* interface methods :
* <ul>
*  <li><code> disposing()</code></li>
* </ul> <p>
* Tests nothing, all methods has <b>OK</b> status.
* @see com.sun.star.lang.XEventListener
*/
public class _XEventListener extends MultiMethodTest {

    public XEventListener oObj = null;

    /**
    * Nothing to test. Always has <b>OK</b> status.
    */
    public void _disposing() {
        log.println("The method 'disposing'");
        log.println("gets called when the broadcaster is about to be"+
            " disposed.") ;
        log.println("All listeners and all other objects which reference "+
            "the broadcaster should release the references.");
        log.println("So there is nothing to test here");
        tRes.tested("disposing()",true);

    }

} //EOF of XEventListener

