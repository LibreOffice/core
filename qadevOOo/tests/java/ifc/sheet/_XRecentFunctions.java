/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.sheet;

import java.util.Random;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.sheet.XRecentFunctions;

/**
* Testing <code>com.sun.star.sheet.XRecentFunctions</code>
* interface methods :
* <ul>
*  <li><code> getRecentFunctionIds()</code></li>
*  <li><code> setRecentFunctionIds()</code></li>
*  <li><code> getMaxRecentFunctions()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'FUNCTIONLIST'</code> (of type <code>XNameAccess</code>):
*   to have the set of available functions </li>
* <ul> <p>
* @see com.sun.star.sheet.XRecentFunctions
*/
public class _XRecentFunctions extends MultiMethodTest {

    public XRecentFunctions oObj = null;
    int iMaxNumber = 0;

    /**
    * Test calls the method, checks returned value and stores it. <p>
    * Has <b> OK </b> status if returned value isn't equal to zero. <p>
    */
    public void _getMaxRecentFunctions() {

        iMaxNumber = oObj.getMaxRecentFunctions();
        log.println("Maximum recent functions : " + iMaxNumber);

        tRes.tested("getMaxRecentFunctions()", iMaxNumber != 0);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null, if length of returned
    * array is equal or less to the maximum number of functions and obtained
    * array doesn't contain equal functions. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMaxRecentFunctions() </code> : to have the maximum number
    *  of recent functions </li>
    * </ul>
    */
    public void _getRecentFunctionIds() {
        requiredMethod("getMaxRecentFunctions()");

        boolean bResult = true;
        int[] IDs = null;
        int iNumber = 0;

        IDs = oObj.getRecentFunctionIds();
        iNumber = IDs.length;
        bResult &= (iNumber <= iMaxNumber);
        log.println("Now there are " + iNumber + " recent functions");
        bResult &= (IDs != null);
        if (bResult) {
            for (int i = 0; i < iNumber - 1; i++)
                for (int j = i + 1; j < iNumber; j++) {
                    bResult &= (IDs[i] != IDs[j]);
                }
        }

        tRes.tested("getRecentFunctionIds()", bResult);
    }

    /**
    * Test gets the set of available functions, sets empty list of recent
    * functions, sets list of maximum size. <p>
    * Has <b> OK </b> status if length of recent function list is equal to zero
    * after list was set to empty, if length of list is equal to maximum size
    * after list was set to it's maximum size and no exception were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMaxRecentFunctions() </code> : to have the maximum number
    *  of recent functions </li>
    * </ul>
    */
    public void _setRecentFunctionIds() {
        requiredMethod("getMaxRecentFunctions()");

        boolean bResult = true;
        int[] IDs = new int[0];
        XNameAccess functionList = null;

        log.println("First, get the set of available functions.");
        functionList = (XNameAccess)tEnv.getObjRelation("FUNCTIONLIST");
        if (functionList == null) throw new StatusException(Status.failed
            ("Relation 'FUNCTIONLIST' not found"));

        log.println("Now trying to set empty list.");
        oObj.setRecentFunctionIds(IDs);
        bResult &= (oObj.getRecentFunctionIds().length == 0);

        log.println("Now trying to set list of maximum size.");
        String[] names = functionList.getElementNames();
        Random rnd = new Random();

        IDs = new int[iMaxNumber];
        int startIdx = rnd.nextInt(names.length - iMaxNumber - 1) + 1;

        try {
            for (int i = startIdx; i < startIdx + iMaxNumber; i++) {
                PropertyValue[] propVals = (PropertyValue[])
                    functionList.getByName(names[i]);
                for (int j = 0; j < propVals.length; j++) {
                    String propName = (String)propVals[j].Name;
                    if (propName.equals("Id")) {
                        IDs[i - startIdx] =
                            ((Integer)propVals[j].Value).intValue();
                        break;
                    }
                }
            }
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            bResult = false;
        } catch(com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            bResult = false;
        }

        oObj.setRecentFunctionIds(IDs);
        bResult &= (oObj.getRecentFunctionIds().length == iMaxNumber);

        tRes.tested("setRecentFunctionIds()", bResult);
    }

}

