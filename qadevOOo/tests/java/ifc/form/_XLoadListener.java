/*************************************************************************
 *
 *  $RCSfile: _XLoadListener.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:35:17 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XLoadListener;

/**
* Testing <code>com.sun.star.form.XLoadListener</code>
* interface methods :
* <ul>
*  <li><code> loaded()</code></li>
*  <li><code> unloading()</code></li>
*  <li><code> unloaded()</code></li>
*  <li><code> reloading()</code></li>
*  <li><code> reloaded()</code></li>
* </ul> <p>
* This interface methods are not testable, because their
* behaviour is not documented.
* @see com.sun.star.form.XLoadListener
*/
public class _XLoadListener extends MultiMethodTest {

    public XLoadListener oObj = null;

    /**
    * Always has <b>OK</b> status.
    */
    public void _loaded() {
        log.println("nothing to test here");
        tRes.tested("loaded()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _reloaded() {
        log.println("nothing to test here");
        tRes.tested("reloaded()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _reloading() {
        log.println("nothing to test here");
        tRes.tested("reloading()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _unloaded() {
        log.println("nothing to test here");
        tRes.tested("unloaded()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _unloading() {
        log.println("nothing to test here");
        tRes.tested("unloading()",true);
    }

} //EOF of XLoadListener


