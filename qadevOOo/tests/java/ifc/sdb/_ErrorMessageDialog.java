/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _ErrorMessageDialog.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:24:52 $
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

package ifc.sdb;

import lib.MultiPropertyTest;

/**
 * Testing <code>com.sun.star.sdb.ErrorMessageDialog</code>
 * service properties :
 * <ul>
 *  <li><code> Title</code></li>
 *  <li><code> ParentWindow</code></li>
 *  <li><code> SQLException</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ERR1', 'ERR2'</code>
 *  (of type <code>com.sun.star.sdbc.SQLException</code>):
 *   two objects which are used for changing 'SQLException'
 *   property. </li>
 * <ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.sdb.ErrorMessageDialog
 */
public class _ErrorMessageDialog extends MultiPropertyTest {

    /**
     * <code>SQLException</code> instances must be used as property
     * value.
     */
    public void _SQLException() {
        log.println("Testing with custom Property tester") ;
        testProperty("SQLException", tEnv.getObjRelation("ERR1"),
            tEnv.getObjRelation("ERR2")) ;
    }

    public void _ParentWindow(){
        log.println("Testing with custom Property tester");
        testProperty("ParentWindow", tEnv.getObjRelation("ERR_XWindow"), null);
    }

}  // finish class _ErrorMessageDialog


