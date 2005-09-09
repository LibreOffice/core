/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XStringEscape.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:44:46 $
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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XStringEscape;

public class _XStringEscape extends MultiMethodTest {

    public XStringEscape oObj;

    public void _escapeString() {
        log.println("The Implementation of this Interface doesn't really do anything");
        boolean res = true;
        try {
            String toCheck = ";:<>/*";
            String eString = oObj.escapeString(toCheck);
            res = toCheck.equals(eString);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("IllegalArgument");
        }
        tRes.tested("escapeString()",res);
    }

    public void _unescapeString() {
        log.println("The Implementation of this Interface doesn't really do anything");
        boolean res = true;
        try {
            String toCheck = ";:<>/*";
            String ueString = oObj.unescapeString(toCheck);
            res = toCheck.equals(ueString);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("IllegalArgument");
        }
        tRes.tested("unescapeString()",res);
    }

}
