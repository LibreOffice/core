/*************************************************************************
 *
 *  $RCSfile: _XStringSubstitution.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:26:00 $
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
