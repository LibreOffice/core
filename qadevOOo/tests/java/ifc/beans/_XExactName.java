/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XExactName.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:03:54 $
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

package ifc.beans;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XExactName;


public class _XExactName extends MultiMethodTest {
    protected String expectedName = "";
    public XExactName oObj = null;

    public void _getExactName() {
        String nameFor = expectedName.toLowerCase();

        log.println("Getting exact name for " + nameFor);

        String getting = oObj.getExactName(nameFor);

        boolean res = true;

        if (getting == null) {
            res = false;
        } else {
            res = getting.equals(expectedName);
        }

        if (!res) {
            log.println("didn't get the expected Name:");
            log.println("getting: " + getting);
            log.println("Expected: " + expectedName);
        }

        nameFor = expectedName.toUpperCase();

        log.println("Getting exact name for " + nameFor);
        getting = oObj.getExactName(nameFor);

        if (getting == null) {
            res = false;
        } else {
            res &= getting.equals(expectedName);
        }

        if (!getting.equals(expectedName)) {
            log.println("didn't get the expected Name:");
            log.println("getting: " + getting);
            log.println("Expected: " + expectedName);
        }

        tRes.tested("getExactName()", res);
    }

    /**
     * Checking if the Object relation expectedName is given
     * throw a StatusException if not.
     */
    protected void before() {
        expectedName = (String) tEnv.getObjRelation("expectedName");

        if (expectedName == null) {
            throw new StatusException(Status.failed(
                                              "Object relation expectedName is missing"));
        }
    }
}