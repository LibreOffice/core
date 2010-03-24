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

package mod._lng;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.linguistic2.XDictionary;
import com.sun.star.linguistic2.XDictionaryList;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.lingu2.DicList</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::linguistic2::XSearchableDictionaryList</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::linguistic2::XDictionaryList</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.linguistic2.XSearchableDictionaryList
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.linguistic2.XDictionaryList
 * @see ifc.linguistic2._XSearchableDictionaryList
 * @see ifc.lang._XComponent
 * @see ifc.lang._XServiceInfo
 * @see ifc.linguistic2._XDictionaryList
 */
public class DicList extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.lingu2.DicList</code>. Then two dictionaries
     * are created (positive and negative) and added to the list, one
     * entry is added to each of dictionaries and they both are activated.
     * The distionary list is retruned as a component for testing.
     */
    public synchronized TestEnvironment createTestEnvironment( TestParameters Param, PrintWriter log )
           throws StatusException {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)xMSF.createInstance("com.sun.star.lingu2.DicList");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        //creating a user defined dictionary for XSearchableDictionaryList
        XDictionaryList xDicList = (XDictionaryList) UnoRuntime.queryInterface(
                                                    XDictionaryList.class, oObj);
        xDicList.removeDictionary(xDicList.getDictionaryByName("MyDictionary"));
        XDictionary xDic = xDicList.createDictionary("NegativDic",new Locale(
            "en","US","WIN"),com.sun.star.linguistic2.DictionaryType.NEGATIVE,"");
        XDictionary xDic2 = xDicList.createDictionary("PositivDic",new Locale(
            "en","US","WIN"),com.sun.star.linguistic2.DictionaryType.POSITIVE,"");
        xDic2.add("Positiv",false,"");
        xDic.add("Negativ",true,"");
        xDicList.addDictionary(xDic);
        xDicList.addDictionary(xDic2);
        xDic.setActive(true);
        xDic2.setActive(true);

        return tEnv;
    }

}    // finish class DicList

