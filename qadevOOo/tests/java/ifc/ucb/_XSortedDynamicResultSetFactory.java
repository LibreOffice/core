/*************************************************************************
 *
 *  $RCSfile: _XSortedDynamicResultSetFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 11:26:24 $
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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.beans.Property;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.NumberedSortingInfo;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.ucb.XSortedDynamicResultSetFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.ucb.XSortedDynamicResultSetFactory</code>
* interface methods :
* <ul>
*  <li><code> createSortedDynamicResultSet()</code></li>
* </ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>solibrary.jar</code> : is used to retrieve
*   content of its root directory.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XSortedDynamicResultSetFactory
*/
public class _XSortedDynamicResultSetFactory extends MultiMethodTest {

    /**
     * Conatins the tested object.
     */
    public XSortedDynamicResultSetFactory oObj;

    /**
     * Creates sorted dynamic result set from result set. For this
     * a dynamic result set is to be created. It is created by
     * retrieving content list from JAR archive.
     * Has <b>OK</b> status if numbers of rows are equal and they are
     * greater then 0 (because JAR file contains at least one entry).
     */
    public void _createSortedDynamicResultSet() {
        boolean result = true ;

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XDynamicResultSet dynResSet = null ;
        try {
            Object oUCB = xMSF.createInstanceWithArguments
                ("com.sun.star.ucb.UniversalContentBroker",
                new Object[] {"Local", "Office"}) ;

            XContentIdentifierFactory ciFac = (XContentIdentifierFactory)
                UnoRuntime.queryInterface
                    (XContentIdentifierFactory.class,oUCB) ;

            String url = util.utils.getFullTestURL("solibrary.jar") ;
            String escUrl = "" ;

            // In base URL of a JAR file in content URL all directory
            // separators ('/') must be replaced with escape symbol '%2F'.
            int idx = url.indexOf("/") ;
            int lastIdx = -1 ;
            while (idx >= 0) {
                escUrl += url.substring(lastIdx + 1, idx) + "%2F" ;
                lastIdx = idx ;
                idx = url.indexOf("/", idx + 1) ;
            }
            escUrl += url.substring(lastIdx + 1) ;
            String cntUrl = "vnd.sun.star.pkg://" + escUrl + "/" ;

            XContentIdentifier CI = ciFac.createContentIdentifier(cntUrl) ;

            XContentProvider cntProv = (XContentProvider)
                UnoRuntime.queryInterface(XContentProvider.class, oUCB) ;

            XContent cnt = cntProv.queryContent(CI) ;

            XCommandProcessor cmdProc = (XCommandProcessor)
                UnoRuntime.queryInterface(XCommandProcessor.class, cnt) ;

            Property prop = new Property() ;
            prop.Name = "Title" ;

            Command cmd = new Command("open", -1, new OpenCommandArgument2
                (OpenMode.ALL, 10000, null, new Property[] {prop},
                 new NumberedSortingInfo[0])) ;

            dynResSet = (XDynamicResultSet) AnyConverter.toObject(
                new Type(XDynamicResultSet.class),cmdProc.execute(cmd, 0, null));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }

        XDynamicResultSet sortedSet = oObj.createSortedDynamicResultSet
            (dynResSet, new NumberedSortingInfo[0], null) ;

        int rowCount = -1 ;
        if (sortedSet != null) {
            XResultSet set = null ;
            try {
                set = sortedSet.getStaticResultSet() ;
            } catch (com.sun.star.ucb.ListenerAlreadySetException e) {
                e.printStackTrace(log);
            }

            try {
                set.last() ;
                rowCount = set.getRow();
                log.println("Number of rows in result set: " + rowCount);
            } catch (com.sun.star.sdbc.SQLException e) {
                log.println("Exception occured while accessing "+
                    "sorted result set :");
                e.printStackTrace(log);
            }
        } else {
            log.println("Null returned !!!");
            result &= false ;
        }

        result &= rowCount > 0 ;

        tRes.tested("createSortedDynamicResultSet()", result) ;
    }


}
