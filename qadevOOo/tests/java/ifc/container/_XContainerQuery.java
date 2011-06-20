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
*    implemented by default in the implemetation object. So this function could be
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
            log.println("This object does not have an implemetation for this function");
            // This is not a bug, because it's a feature for future purposes
        } else {
            for (int i = 0; i < m_queryStrings.length; i++){
                String queryString = m_queryStrings[i];
                XEnumeration subSet = oObj.createSubSetEnumerationByQuery( queryString );

                bResult &= subSet.hasMoreElements();

                while (subSet.hasMoreElements()) {
                    try{
                    Object element = subSet.nextElement();

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
            Object element = subSet.nextElement();

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
