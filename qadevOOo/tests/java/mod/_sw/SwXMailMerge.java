/*************************************************************************
 *
 *  $RCSfile: SwXMailMerge.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2003-09-08 12:46:48 $
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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbcx.XRowLocate;
import com.sun.star.task.XJob;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Here <code>com.sun.star.text.MailMerge</code> service is tested.<p>
* @see com.sun.star.text.MailMerge
* @see com.sun.star.task.XJob
* @see com.sun.star.text.XMailMergeBroadcaster
*/
public class SwXMailMerge extends TestCase {


    public void initialize( TestParameters tParam, PrintWriter log ) {
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested. <p>
     * Creates <code>MailMerge</code> service     * Object relations created :
    * <ul>
    *  <li> <code>'executeArgs'</code> for
    *    {@link ifc.text._XMailMergeBroadcaster} : NamedValue[]</li>
    *  <li> <code>'Job'</code> for
    *    {@link ifc.text._XMailMergeBroadcaster} : XJob</li>
    *  <li> <code>'XJobArgs'</code> for
    *    {@link ifc.task._XJob} : Object[]</li>
    * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XInterface oRowSet = null;
        Object oConnection = null;
        XConnection oXConnection = null;
        XJob Job = null;

        try {
            oObj = (XInterface) ( (XMultiServiceFactory) Param.getMSF()).createInstance
                ("com.sun.star.text.MailMerge");
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object environment", e) ;
        }
        if (oObj == null) System.out.println("mist");
        // <set some variables>
        String cTestDoc = utils.getFullTestURL("MailMerge.sxw");
        //cMailMerge_DocumentURL = cTestDoc
        String cOutputURL = utils.getOfficeTemp( (XMultiServiceFactory) Param.getMSF());
        String cDataSourceName  = "Bibliography";
        String cDataCommand = "biblio";
        Object[] sel = new Object[2];
        sel[0] = new int[2];
        sel[1] = new int[5];
        Object[] myBookMarks = new Object[2];
        // </set some variables>

        // <create XResultSet>
        try {
            oRowSet = (XInterface) ( (XMultiServiceFactory) Param.getMSF()).createInstance
                ("com.sun.star.sdb.RowSet");
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create com.sun.star.sdb.RowSet", e);
        }
        XPropertySet oRowSetProps = (XPropertySet)
                        UnoRuntime.queryInterface(XPropertySet.class, oRowSet);
        XRowSet xRowSet = (XRowSet)
                        UnoRuntime.queryInterface(XRowSet.class, oRowSet);
        try {
            oRowSetProps.setPropertyValue("DataSourceName",cDataSourceName);
            oRowSetProps.setPropertyValue("Command",cDataCommand);
            oRowSetProps.setPropertyValue("CommandType", new Integer(com.sun.star.sdb.CommandType.TABLE));
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            throw new StatusException("Can't set properties on oRowSet", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            throw new StatusException("Can't set properties on oRowSet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            throw new StatusException("Can't set properties on oRowSet", e);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            throw new StatusException("Can't set properties on oRowSet", e);
        }
        try {
            xRowSet.execute();
        } catch (com.sun.star.sdbc.SQLException e) {
            throw new StatusException("Can't execute oRowSet", e);
        }

        XResultSet oResultSet = (XResultSet)
                           UnoRuntime.queryInterface(XResultSet.class, oRowSet);

        //wait(200)


        try {
            oConnection = oRowSetProps.getPropertyValue("ActiveConnection");
        } catch (com.sun.star.beans.UnknownPropertyException e){
            throw new StatusException("Can't get active connection", e);
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            throw new StatusException("Can't get active connection", e);
        }
        oXConnection = (XConnection)
                    UnoRuntime.queryInterface(XConnection.class, oConnection);
        // </create XResultSet>


        // <create Bookmarks>
        try {
            XRowLocate oRowLocate = (XRowLocate) UnoRuntime.queryInterface(
                                                  XRowLocate.class, oResultSet);
            oResultSet.first();
            myBookMarks[0] = oRowLocate.getBookmark();
            oResultSet.next();
            myBookMarks[1] = oRowLocate.getBookmark();
        } catch (com.sun.star.sdbc.SQLException e) {
            throw new StatusException("Cant get Bookmarks", e);
        }
        // </create Bookmarks>
        // <fill object with values>
        XPropertySet oObjProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class, oObj);
        try {
            oObjProps.setPropertyValue("DataSourceName", cDataSourceName);
            oObjProps.setPropertyValue("Command", cDataCommand);
            oObjProps.setPropertyValue("CommandType", new Integer(com.sun.star.sdb.CommandType.TABLE));
            oObjProps.setPropertyValue("OutputType", new Short(com.sun.star.text.MailMergeType.FILE));
            oObjProps.setPropertyValue("DocumentURL", cTestDoc);
            oObjProps.setPropertyValue("OutputURL", cOutputURL);
            System.out.println("OUTPUT: "+cOutputURL);
            oObjProps.setPropertyValue("FileNamePrefix", "Author");
            oObjProps.setPropertyValue("FileNameFromColumn", new Boolean(false));
            oObjProps.setPropertyValue("Selection", new Object[0]);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            throw new StatusException("Can't set properties on oObj", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            throw new StatusException("Can't set properties on oObj", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            throw new StatusException("Can't set properties on oObj", e);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            throw new StatusException("Can't set properties on oObj", e);
        }
        // </fill object with values>


        // <create object relations>
        Object[] vXJobArgs = new Object[4];
    NamedValue[] vXJobArg0 = new NamedValue[8];
    NamedValue[] vXJobArg1 = new NamedValue[7];
        NamedValue[] vXJobArg2 = new NamedValue[10];
    NamedValue[] vXJobArg3 = new NamedValue[0];

        // first Arguments
           vXJobArg0[0] = new NamedValue("DataSourceName", cDataSourceName);
    vXJobArg0[1] = new NamedValue("Command", cDataCommand);
        vXJobArg0[2] = new NamedValue("CommandType",
                               new Integer(com.sun.star.sdb.CommandType.TABLE));
    vXJobArg0[3] = new NamedValue("OutputType",
                             new Short(com.sun.star.text.MailMergeType.FILE));
    vXJobArg0[4] = new NamedValue("DocumentURL", cTestDoc);
    vXJobArg0[5] = new NamedValue("OutputURL", cOutputURL);
    vXJobArg0[6] = new NamedValue("FileNamePrefix", "Identifier");
    vXJobArg0[7] = new NamedValue("FileNameFromColumn", new Boolean(true));
//  vXJobArg0[8] = new NamedValue("Selection", sel);

        //second Arguments
           vXJobArg1[0] = new NamedValue("DataSourceName", cDataSourceName);
    vXJobArg1[1] = new NamedValue("Command", cDataCommand);
        vXJobArg1[2] = new NamedValue("CommandType",
                               new Integer(com.sun.star.sdb.CommandType.TABLE));
    vXJobArg1[3] = new NamedValue("OutputType",
                             new Short(com.sun.star.text.MailMergeType.PRINTER));
    vXJobArg1[4] = new NamedValue("DocumentURL", cTestDoc);
    vXJobArg1[5] = new NamedValue("FileNamePrefix", "Author");
    vXJobArg1[6] = new NamedValue("FileNameFromColumn", new Boolean(true));
//  vXJobArg1[7] = new NamedValue("Selection", sel);

        // third Arguments
           vXJobArg2[0] = new NamedValue("ActiveConnection", oXConnection);
    vXJobArg2[1] = new NamedValue("DataSourceName", cDataSourceName);
        vXJobArg2[2] = new NamedValue("Command", cDataCommand);
        vXJobArg2[3] = new NamedValue("CommandType",
                               new Integer(com.sun.star.sdb.CommandType.TABLE));
    vXJobArg2[4] = new NamedValue("OutputType",
                             new Short(com.sun.star.text.MailMergeType.FILE));
    vXJobArg2[5] = new NamedValue("ResultSet", oResultSet);
    vXJobArg2[6] = new NamedValue("OutputURL", cOutputURL);
    vXJobArg2[7] = new NamedValue("FileNamePrefix", "Identifier");
    vXJobArg2[8] = new NamedValue("FileNameFromColumn", new Boolean(true));
    vXJobArg2[9] = new NamedValue("Selection", myBookMarks);
//      vXJobArg2[9] = new NamedValue("Selection", sel);

        vXJobArgs[0] = vXJobArg0;
        vXJobArgs[1] = vXJobArg1;
        vXJobArgs[2] = vXJobArg2;
        vXJobArgs[3] = vXJobArg3;


       Job = (XJob) UnoRuntime.queryInterface(XJob.class, oObj);

        // </create object relations>

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        // <adding object relations>

        // com.sun.star.text.XMailMergeBroadcaster
        tEnv.addObjRelation( "executeArgs", vXJobArg0);
        tEnv.addObjRelation( "Job", Job);

        // com.sun.star.task.XJob
        tEnv.addObjRelation("XJobArgs", vXJobArgs);

        // </adding object relations>


        // adding connection string as relation
//        tEnv.addObjRelation("XAcceptor.connectStr",
//            "socket,host=" + sOfficeHost + ",port=" + curPort) ;

        // adding port number for freeing it.
        //tEnv.addObjRelation("Acceptor.Port", new Integer(curPort)) ;

        return tEnv ;
    }

}


