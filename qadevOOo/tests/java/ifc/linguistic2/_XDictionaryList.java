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

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.DictionaryListEvent;
import com.sun.star.linguistic2.XDictionary;
import com.sun.star.linguistic2.XDictionaryList;
import com.sun.star.linguistic2.XDictionaryListEventListener;

/**
* Testing <code>com.sun.star.linguistic2.XDictionaryList</code>
* interface methods:
* <ul>
*   <li><code>getCount()</code></li>
*   <li><code>getDictionaries()</code></li>
*   <li><code>getDictionaryByName()</code></li>
*   <li><code>addDictionary()</code></li>
*   <li><code>removeDictionary()</code></li>
*   <li><code>addDictionaryListEventListener()</code></li>
*   <li><code>removeDictionaryListEventListener()</code></li>
*   <li><code>beginCollectEvents()</code></li>
*   <li><code>endCollectEvents()</code></li>
*   <li><code>flushEvents()</code></li>
*   <li><code>createDictionary()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XDictionaryList
*/
public class _XDictionaryList extends MultiMethodTest {

    public XDictionaryList oObj = null;
    public XDictionary addedDic = null;

    /**
    * Flag for testing of listeners.
    */
    public boolean listenerCalled = false;

    /**
    * Class implements interface <code>XDictionaryListEventListener</code>
    * for test method <code>addDictionaryListEventListener</code>.
    * @see com.sun.star.linguistic2.XDictionaryListEventListener
    */
    public class MyDictionaryListEventListener implements
            XDictionaryListEventListener {

        public void disposing ( EventObject oEvent ) {
            log.println("Listener has been disposed");
        }
        public void processDictionaryListEvent( DictionaryListEvent aDicEvent) {
            listenerCalled = true;
        }
    };

    XDictionaryListEventListener listener = new MyDictionaryListEventListener();

    short count = 0;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is greater than zero. <p>
    */
    public void _getCount() {
        count = oObj.getCount();
        tRes.tested("getCount()",(count > 0) );
    }

    /**
    * Test calls the method and checks number of obtained dictionaries
    * with value that was returned by method <code>getCount</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getCount() </code> : to have number of dictionaries </li>
    * </ul>
    */
    public void _getDictionaries() {
        requiredMethod("getCount()");

        XDictionary[] dics = oObj.getDictionaries();
        boolean res = (dics.length == count);
        if (!res) {
            log.println("Expected: " + oObj.getCount());
            log.println("Gained: " + dics.length);
        }
        tRes.tested("getDictionaries()", res);
    }

    /**
    * Test calls the method, makes some actions that leads to event
    * <code>processDictionaryListEvent</code>, removes listener, checks flag
    * <code>listenerCalled</code> and checks returned value. <p>
    * Has <b> OK </b> status if returned value is true and value of flag
    * <code>listenerCallled</code> is true. <p>
    */
    public void _addDictionaryListEventListener() {
        listenerCalled = false;

        XDictionary xDic = oObj.createDictionary("ListenDic",
            new Locale("en","US","WIN"),
            com.sun.star.linguistic2.DictionaryType.POSITIVE,"");

        boolean res = oObj.addDictionaryListEventListener(listener, false);

        oObj.flushEvents();
        oObj.addDictionary(xDic);
        xDic.add("Positiv", false, "");
        xDic.setActive(true);
        oObj.flushEvents();
        oObj.removeDictionary(xDic);

        oObj.removeDictionaryListEventListener(listener);

        tRes.tested("addDictionaryListEventListener()",listenerCalled && res);
    }

    /**
    * Test calls the method, makes some actions that leads to event
    * <code>processDictionaryListEvent</code>, checks flag
    * <code>listenerCalled</code> and checks returned value. <p>
    * Has <b> OK </b> status if returned value is false and value of flag
    * <code>listenerCallled</code> is false. <p>
    */
    public void _removeDictionaryListEventListener() {
        listenerCalled = false;

        XDictionary xDic = oObj.createDictionary("ListenDic",
            new Locale("en","US","WIN"),
            com.sun.star.linguistic2.DictionaryType.POSITIVE,"");

        oObj.addDictionaryListEventListener(listener,false);

        oObj.flushEvents();
        oObj.addDictionary(xDic);
        xDic.add("Positiv", false,"");
        xDic.setActive(true);

        listenerCalled = false;
        boolean res = oObj.removeDictionaryListEventListener(listener);

        oObj.flushEvents();
        oObj.removeDictionary(xDic);

        tRes.tested(
            "removeDictionaryListEventListener()",
            listenerCalled == false && res == true );
    }

    /**
    * Test creates new dictionary, adds the dictionary to list and compares
    * number of dictionaries after adding with number of dictionaries before.<p>
    * Has <b> OK </b> status if number of dictionaries after method call is
    * greater than number of dictionaries before method call. <p>
    */
    public void _addDictionary() {
        short previous = oObj.getCount();
        addedDic = oObj.createDictionary("AddedDic",new Locale("en","US","WIN"),
                        com.sun.star.linguistic2.DictionaryType.POSITIVE,"");
        addedDic.add("Positiv",false,"");

        oObj.addDictionary(addedDic);

        short after = oObj.getCount();

        tRes.tested( "addDictionary()", (after > previous) );
    }

    /**
    * Test calls the method and compares number of dictionaries
    * before method call and after. <p>
    * Has <b> OK </b> status if number of dictionaries before method call is
    * less than number of dictionaries after method call. <p>
    */
    public void _removeDictionary() {
        short previous = oObj.getCount();
        oObj.removeDictionary(addedDic);
        short after = oObj.getCount();
        tRes.tested("removeDictionary()",(after < previous) );
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getDictionaryByName() {
        XDictionary getting = oObj.getDictionaryByName("NegativDic");
        tRes.tested("getDictionaryByName()", getting != null );
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _createDictionary() {
        XDictionary tmpDic = oObj.createDictionary("AddedDic",
            new Locale("en","US","WIN"),
            com.sun.star.linguistic2.DictionaryType.POSITIVE,"");
        tRes.tested("createDictionary()", tmpDic != null );
    }

    /**
    * Test creates dictionary, adds dictionary list event listener,
    * begins collect events, makes some actions that leads to event
    * <code>processDictionaryListEvent</code>, ends collect events,
    * removes the listener and checks the flag <code>listenerCalled</code> . <p>
    * Has <b> OK </b> status if value of the flag is true. <p>
    */
    public void _beginCollectEvents() {
        listenerCalled = false;

        XDictionary xDic = oObj.createDictionary("ListenDic",
            new Locale("en","US","WIN"),
            com.sun.star.linguistic2.DictionaryType.POSITIVE,"");

        oObj.addDictionaryListEventListener(listener,false);
        oObj.beginCollectEvents();

        oObj.addDictionary(xDic);
        xDic.add("Positiv",false,"");
        xDic.setActive(true);

        oObj.removeDictionary(xDic);
        oObj.endCollectEvents();

        oObj.removeDictionaryListEventListener(listener);

        tRes.tested("beginCollectEvents()", listenerCalled );
    }

    /**
    * Test does nothing. <p>
    * Has <b> OK </b> status if method
    * <code>addDictionaryListEventListener()</code> was completed
    * successfully. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addDictionaryListEventListener() </code> :
    *  if listener adding worked, flushEvents was already used and worked </li>
    * </ul>
    */
    public void _flushEvents() {
        requiredMethod("addDictionaryListEventListener()");
        // if listener adding worked, flushEvents was already used and worked
        tRes.tested("flushEvents()",true);
    }

    /**
    * Test does nothing. <p>
    * Has <b> OK </b> status if method
    * <code>beginCollectEvents()</code> was completed successfully. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> beginCollectEvents() </code> :
    *  if beginCollectEvents() worked then endCollectEvents was already
    *  used and worked </li>
    * </ul>
    */
    public void _endCollectEvents() {
        requiredMethod("beginCollectEvents()");
        // if beginCollectEvents() worked, endCollectEvents
        // was already used and worked
        tRes.tested("endCollectEvents()",true);
    }

}  // finish class _XDictionaryList


