/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._lng;

import java.io.PrintWriter;

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
 * threads concurrently.
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
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.lingu2.DicList</code>. Then two dictionaries
     * are created (positive and negative) and added to the list, one
     * entry is added to each of dictionaries and they both are activated.
     * The dictionary list is returned as a component for testing.
     */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param, PrintWriter log )
           throws Exception {

        XMultiServiceFactory xMSF = Param.getMSF();
        XInterface oObj = (XInterface)xMSF.createInstance("com.sun.star.lingu2.DicList");

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        //creating a user defined dictionary for XSearchableDictionaryList
        XDictionaryList xDicList = UnoRuntime.queryInterface(
                                                    XDictionaryList.class, oObj);
        xDicList.removeDictionary(xDicList.getDictionaryByName("MyDictionary"));
        XDictionary xDic = xDicList.createDictionary("NegativDic",new Locale(
            "en","US","WIN"),com.sun.star.linguistic2.DictionaryType.NEGATIVE,"");
        XDictionary xDic2 = xDicList.createDictionary("PositivDic",new Locale(
            "en","US","WIN"),com.sun.star.linguistic2.DictionaryType.POSITIVE,"");
        xDic2.add("Positive",false,"");
        xDic.add("Negative",true,"");
        xDicList.addDictionary(xDic);
        xDicList.addDictionary(xDic2);
        xDic.setActive(true);
        xDic2.setActive(true);

        return tEnv;
    }

}    // finish class DicList

