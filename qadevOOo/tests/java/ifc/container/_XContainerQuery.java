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

package ifc.container;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.NamedValue;
import com.sun.star.container.XContainerQuery;
import com.sun.star.container.XEnumeration;


/**
* Testing <code>com.sun.star.container.XContainerQuery</code>
* interface methods :
* <ul>
*  <li><code> createSubSetEnumerationByProperties()</code></li>
*  <li><code> createSubSetEnumerationByQuery()</code></li>
* </ul>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XContainerQuery.createSubSetEnumerationByProperties'</code> :
*    <code>NameValue[]</code> which is a valid argument for
*     <code>createSubSetEnumerationByProperties()</code>.</li>
*  <li> <code>'XContainerQuery.createSubSetEnumerationByQuery'</code> : <b>(optional)</b>
*     Normaly <code>createSubSetEnumerationByProperties()</code> covers all
*    possible queries. But for special cases, i.e. sorted output, the function
*    <code>createSubSetEnumerationByQuery()</code> was made. The special cases was not
*    implemented by default in the implementation object. So this function could be
*    marked as <code>optional</code></li>
* <ul> <p>
* <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.container.XContainerQuery
*/
public class _XContainerQuery extends MultiMethodTest {

    public XContainerQuery oObj = null;
    private NamedValue[] m_querySequenze = null;
    private String[] m_queryStrings = null;


    /**
    * Retrieves object relations
    * @throws StatusException If one of relations not found.
    */
    public void before() throws StatusException {

        m_querySequenze = (NamedValue[]) tEnv.getObjRelation(
                        "XContainerQuery.createSubSetEnumerationByProperties");
        if (m_querySequenze == null) {
            throw new StatusException(
                Status.failed("Could not get object relation " +
                "'XContainerQuery.createSubSetEnumerationByProperties'")) ;
        }

        m_queryStrings = (String[]) tEnv.getObjRelation(
                        "XContainerQuery.createSubSetEnumerationByQuery");
        if (m_queryStrings == null) {
            log.println("Could not get object relation " +
                        "'XContainerQuery.createSubSetEnumerationByQuery'");
        }
    }


    /**
     * If object relation is available, the function was called with relation
     * as parameter. The returned <code>XEnumeration</code> must not be null and
     * elements of it must be valid.
     * If object relation is not available, the result is always <code>true</coed>
    */
    public void _createSubSetEnumerationByQuery() {

        boolean bResult = true;
        if ( m_queryStrings == null ) {
            log.println("This object does not have an implementation for this function");
            // This is not a bug, because it's a feature for future purposes
        } else {
            for (int i = 0; i < m_queryStrings.length; i++){
                String queryString = m_queryStrings[i];
                XEnumeration subSet = oObj.createSubSetEnumerationByQuery( queryString );

                bResult &= subSet.hasMoreElements();

                while (subSet.hasMoreElements()) {
                    try{
                    subSet.nextElement();

                    } catch (com.sun.star.container.NoSuchElementException e){
                        log.println("Exception occurred ");
                        e.printStackTrace(log);
                        bResult = false;
                    } catch (com.sun.star.lang.WrappedTargetException e){
                        log.println("Exception occurred ");
                        e.printStackTrace(log);
                        bResult = false;
                    }
                }
            }
        }

        tRes.tested("createSubSetEnumerationByQuery()", bResult);
    }

    /**
     * The function was called with object relation
     * as parameter. The returned <code>XEnumeration</code> must not be null and
     * elements of it must be valid.
     *
     */
    public void _createSubSetEnumerationByProperties() {

        boolean bResult = true;

        XEnumeration subSet = oObj.createSubSetEnumerationByProperties( m_querySequenze );

        bResult = subSet.hasMoreElements();

        while (subSet.hasMoreElements()) {
            try{
            subSet.nextElement();

            } catch (com.sun.star.container.NoSuchElementException e){
                log.println("Exception occurred ");
                e.printStackTrace(log);
                bResult = false;
            } catch (com.sun.star.lang.WrappedTargetException e){
                log.println("Exception occurred ");
                e.printStackTrace(log);
                bResult = false;
            }
        }

        tRes.tested("createSubSetEnumerationByProperties()", bResult);
    }
}
