/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XStringSubstitution.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:45:00 $
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

import com.sun.star.util.XStringSubstitution;
import lib.MultiMethodTest;

public class _XStringSubstitution extends MultiMethodTest {

    public XStringSubstitution oObj;

    public void _getSubstituteVariableValue() {
        boolean res = true;
        try {
            log.println("try to get the valid variable $(user) ...");
            String toCheck = "$(user)";
            String eString = oObj.getSubstituteVariableValue(toCheck);
            res = eString.startsWith("file:///");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("$(user) does not exist");
            tRes.tested("getSubstituteVariableValue()",false);
        }
        try {
            log.println("try to get a invalid variable...");
            String toCheck = "$(ThisVariableShouldNoExist)";
            String eString = oObj.getSubstituteVariableValue(toCheck);
            log.println("$(ThisVariableShouldNoExist) should not exist");
            tRes.tested("getSubstituteVariableValue()",false);

        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected exception was thrown.");
            res &= true;
        }

        tRes.tested("getSubstituteVariableValue()",res);
    }

    public void _substituteVariables() {
        boolean res = true;
        try {
            log.println("try to get a valid variable...");
            String toCheck = "$(user)";
            String eString = oObj.substituteVariables(toCheck, false);
            log.println(eString);
            res = eString.startsWith("file:///");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("$(user) does not exist");
            tRes.tested("substituteVariables()",false);
        }
        try {
            log.println("try to get a invalid variable...");
            String toCheck = "$(ThisVariableShouldNoExist)";
            String eString = oObj.substituteVariables(toCheck,true);
            log.println("$(ThisVariableShouldNoExist) should not exist");
            tRes.tested("substituteVariables()",false);

        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected exception was thrown.");
            res &= true;
        }

        tRes.tested("substituteVariables()",res);
    }

    public void _reSubstituteVariables() {
        boolean res = true;
        log.println("try to get a valid variable...");
        String toCheck = "file:///";
        String eString = oObj.reSubstituteVariables(toCheck);
        log.println(eString);
        res = eString.startsWith("file:///");

        tRes.tested("reSubstituteVariables()",res);
    }

}
