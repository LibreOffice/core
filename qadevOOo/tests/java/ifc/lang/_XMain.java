/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMain.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:15:40 $
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

import com.sun.star.lang.XMain;


/**
 * Testing <code>com.sun.star.lang.XMain</code>
 * interface methods :
 * <ul>
 *  <li><code> run()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ARGS'</code> (of type <code>String[]</code>):
 *   used as argument for <code>run</code> method. </li>
 * <ul> <p>
 *
 * @see com.sun.star.lang.XMain
 */
public class _XMain extends MultiMethodTest{

    public static XMain oObj = null;

    /**
     * Just calls the method. <p>
     *
     * Has <b> OK </b> status if no runtime exceptions occured.
     */
    public void _run() {
        String[] args = (String[])tEnv.getObjRelation("ARGS");

        log.println("Running with arguments:");
        for (int i=0; i< args.length; i++)
            log.println("#" + i + ": " + args[i]);

        oObj.run(args);

        tRes.tested("run()", true);
    }

}
