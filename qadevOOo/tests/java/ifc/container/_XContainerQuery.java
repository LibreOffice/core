/*************************************************************************
 *
 *  $RCSfile: _XContainerQuery.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-01-28 19:27:42 $
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

package ifc.container;

import java.io.PrintWriter;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.beans.NamedValue;
import com.sun.star.container.XContainerQuery;
import com.sun.star.container.XEnumeration;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import java.util.Enumeration;


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
                        log.println("Exception occured ");
                        e.printStackTrace(log);
                        bResult = false;
                    } catch (com.sun.star.lang.WrappedTargetException e){
                        log.println("Exception occured ");
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
                log.println("Exception occured ");
                e.printStackTrace(log);
                bResult = false;
            } catch (com.sun.star.lang.WrappedTargetException e){
                log.println("Exception occured ");
                e.printStackTrace(log);
                bResult = false;
            }
        }

        tRes.tested("createSubSetEnumerationByProperties()", bResult);
    }
}
