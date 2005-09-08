/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFunctionDescriptions.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:48:32 $
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

package ifc.sheet;

import java.util.Random;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.XFunctionDescriptions;

/**
* Testing <code>com.sun.star.sheet.XFunctionDescriptions</code>
* interface methods :
* <ul>
*  <li><code> getById()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XFunctionDescriptions
*/
public class _XFunctionDescriptions extends MultiMethodTest {

    public XFunctionDescriptions oObj = null;

    /**
    * Test finds available id, calls method using this id, checks returned
    * value and then tries to get description with wrong id. <p>
    * Has <b>OK</b> status if returned value is equal to value obtained by the
    * method <code>getByIndex()</code> in first call and exception
    * <code>IllegalArgumentException</code> was thrown in second call.<p>
    * @see com.sun.star.lang.IllegalArgumentException
    */
    public void _getById() {
        boolean bResult = true;
        // Finding available id...

        int count = oObj.getCount();
        if (count > 0) {
            Random rnd = new Random();
            int nr = rnd.nextInt(count);

            PropertyValue[] PVals = null;
            try {
                PVals = (PropertyValue[])oObj.getByIndex(nr);
            } catch(com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
                tRes.tested("getById()", false);
                return;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                e.printStackTrace(log);
                tRes.tested("getById()", false);
                return;
            }

            String FName = null;
            Integer FId = null;

            for (int i = 0; i < PVals.length; i++) {
                if (PVals[i].Name.equals("Name"))
                    FName = (String)PVals[i].Value;
                if (PVals[i].Name.equals("Id"))
                    FId = (Integer)PVals[i].Value;
            }

            log.println("The id of function '" + FName + "' is " + FId);

            PropertyValue[] PVals2 = null;
            try {
                PVals2 = oObj.getById(FId.intValue());
            } catch(com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace(log);
                tRes.tested("getById()", false);
                return;
            }

            String objFName = null;
            Integer objFId = null;
            for (int i = 0; i < PVals2.length; i++) {
                if (PVals2[i].Name.equals("Name"))
                    objFName = (String)PVals[i].Value;
                if (PVals2[i].Name.equals("Id"))
                    objFId = (Integer)PVals[i].Value;
            }

            log.println("The id of returned function '" +
                objFName + "' is " + objFId);

            bResult &= FName.equals(objFName);
            bResult &= FId.equals(objFId);
        }

        log.println("OK.");

        try {
            log.println("Now trying to get description with wrong id ... ");
            oObj.getById(-1);
            bResult = false;
            log.println("Exception expected! - FAILED");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception " + e + " - OK!");
        }

        tRes.tested("getById()", bResult);
    }
}  // finish class _XFunctionDescriptions


