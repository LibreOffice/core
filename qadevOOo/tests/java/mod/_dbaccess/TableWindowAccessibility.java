/*************************************************************************
 *
 *  $RCSfile: TableWindowAccessibility.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-02-27 13:09:16 $
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

package mod._dbaccess;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import com.sun.star.beans.PropertyValue;
import util.DBTools;
import com.sun.star.uno.XNamingService;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbc.XIsolatedConnection;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdb.XQueryDefinitionsSupplier;
import com.sun.star.container.XNameContainer;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.lang.XComponent;
import drafts.com.sun.star.awt.XExtendedToolkit;
import util.AccessibilityTools;
import com.sun.star.awt.XWindow;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.AccessibleRole;
import drafts.com.sun.star.accessibility.XAccessibleSelection;
import drafts.com.sun.star.accessibility.XAccessibleComponent;

/**
* Object implements the following interfaces :
* <ul>
*   <li><code>drafts::com::sun::star::accessibility::XAccessible</code></li>
*   <li><code>drafts::com::sun::star::accessibility::XAccessibleContext
*   </code></li>
*   <li><code>drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster
*   </code></li>
* </ul><p>
* @see drafts.com.sun.star.accessibility.XAccessible
* @see drafts.com.sun.star.accessibility.XAccessibleContext
* @see drafts.com.sun.star.accessibility.XAccessibleEventBroadcaster
* @see ifc.accessibility._XAccessible
* @see ifc.accessibility._XAccessibleContext
* @see ifc.accessibility._XAccessibleEventBroadcaster
*/
public class TableWindowAccessibility extends TestCase {

    XAccessibleAction actionCancel = null;
    XWindow queryWindow = null;
    Object[] savedObj = null;
    String[] elNames = null;
    XNamingService namingService = null;

    /**
     * Creates a new DataSource and registers it.
     * Creates a connection and using it
     * creates two tables in database.
     * Creates a new query and adds it to DefinitionContainer.
     * Opens a DatasourceAdministration dialog,
     * selects the tab page 'Query', pushs the button 'Edit query' using
     * accessibility components. Pushing of the button causes opening of
     * Query dialog. Then obtains an accessible object with
     * the role <code>AccessibleRole.PANEL</code>.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     * @return
     * @throws StatusException
     * @see TestEnvironment
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        Object oDBContext = null, oDBSource = null, newQuery = null;
        Object toolkit = null;
        try {
            oDBContext = Param.getMSF().createInstance(
                "com.sun.star.sdb.DatabaseContext");
            oDBSource = Param.getMSF().createInstance(
                "com.sun.star.sdb.DataSource");
            newQuery = Param.getMSF().createInstance(
                "com.sun.star.sdb.QueryDefinition");
            toolkit = Param.getMSF().createInstance(
                "com.sun.star.awt.Toolkit");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                Status.failed("Couldn't create instance"));
        }

        String jdbcURL = (String) Param.get("jdbc.url");
        if (jdbcURL == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'jdbc.url' from ini-file"));
        }
        String user = (String) Param.get("jdbc.user");
        String password = (String) Param.get("jdbc.password");
        if (user == null || password == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'jdbc.user' or 'jdbc.password' from ini-file"));
        }

        PropertyValue[] info = new PropertyValue[3];
        info[0] = new PropertyValue();
        info[0].Name = "JavaDriverClass";
        info[0].Value = DBTools.TST_JDBC_DRIVER;
        info[1] = new PropertyValue();
        info[1].Name = "user";
        info[1].Value = user;
        info[2] = new PropertyValue();
        info[2].Name = "password";
        info[2].Value = password;

        XPropertySet propSetDBSource = (XPropertySet)UnoRuntime.queryInterface
            (XPropertySet.class, oDBSource);

        try {
            propSetDBSource.setPropertyValue("URL", "jdbc:" + jdbcURL);
            propSetDBSource.setPropertyValue("Info", info);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        }

        namingService = (XNamingService)
            UnoRuntime.queryInterface(XNamingService.class, oDBContext);
        XNameAccess nameAccess = (XNameAccess)
            UnoRuntime.queryInterface(XNameAccess.class, oDBContext);

        final String sourceName = "AAADBSource for dbu-accessibility";

        if (nameAccess.hasByName(sourceName)) {
            try {
                namingService.revokeObject(sourceName);
            } catch(com.sun.star.uno.Exception e) {
                e.printStackTrace(log);
                throw new StatusException(
                    Status.failed("Couldn't revoke object"));
            }
        }

        //because a bug with selection in ListBox
        //content of TabControl isn't changed after selecting new list item
        elNames = nameAccess.getElementNames();
/*        savedObj = new Object[elNames.length];
        for(int i = 0; i < elNames.length; i++) {
            try {
                savedObj[i] = nameAccess.getByName(elNames[i]);
            } catch(com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
            } catch(com.sun.star.container.NoSuchElementException e) {
                e.printStackTrace(log);
            }

            try {
                namingService.revokeObject(elNames[i]);
            } catch(com.sun.star.uno.Exception e) {
                e.printStackTrace(log);
                throw new StatusException(
                    Status.failed("Couldn't revoke object"));
            }
        }
*/

        try {
            namingService.registerObject(sourceName, oDBSource);
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                Status.failed("Couldn't register object"));
        }

        XIsolatedConnection isolConnection = (XIsolatedConnection)
            UnoRuntime.queryInterface(XIsolatedConnection.class, oDBSource);

        XConnection connection = null;
        XStatement statement = null;

        final String tbl_name1 = "tst_table1";
        final String tbl_name2 = "tst_table2";
        final String col_name1 = "id1";
        final String col_name2 = "id2";

        try {
            connection = isolConnection.getIsolatedConnection(user, password);
            statement =  connection.createStatement();
            statement.executeUpdate("drop table if exists " + tbl_name1);
            statement.executeUpdate("drop table if exists " + tbl_name2);
            statement.executeUpdate("create table " + tbl_name1 + " (" +
                col_name1 + " int)");
            statement.executeUpdate("create table " + tbl_name2 + " (" +
                col_name2 + " int)");
        } catch(com.sun.star.sdbc.SQLException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed("SQLException"));
        }

        XQueryDefinitionsSupplier querySuppl = (XQueryDefinitionsSupplier)
            UnoRuntime.queryInterface(
            XQueryDefinitionsSupplier.class, oDBSource);

        XNameAccess defContainer = querySuppl.getQueryDefinitions();

        XPropertySet queryProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, newQuery);

        try {
            final String query = "select * from " + tbl_name1 + ", " +
                tbl_name2 + " where " + tbl_name1 + "." + col_name1 + "=" +
                tbl_name2 + "." + col_name2;
            queryProp.setPropertyValue("Command", query);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't set property value"));
        }

        XNameContainer queryContainer = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, defContainer);

        try {
            queryContainer.insertByName("Query1", newQuery);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed("Couldn't insert query"));
        } catch(com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed("Couldn't insert query"));
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed("Couldn't insert query"));
        }

        DiagThread Diag = new DiagThread(Param.getMSF());

        Diag.start();

        shortWait();

        XExtendedToolkit tk = (XExtendedToolkit)
            UnoRuntime.queryInterface(XExtendedToolkit.class, toolkit);

        AccessibilityTools at = new AccessibilityTools();

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = (XWindow)
            UnoRuntime.queryInterface(XWindow.class, atw);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XAccessibleContext accContextCancel = at.getAccessibleObjectForRole
            (xRoot, AccessibleRole.PUSHBUTTON, "Cancel");
        actionCancel = (XAccessibleAction)UnoRuntime.queryInterface
            (XAccessibleAction.class, accContextCancel);

        XAccessibleContext accContextTabList = at.getAccessibleObjectForRole(
            xRoot, AccessibleRole.PAGETABLIST);
        XAccessibleSelection tabListSelection = (XAccessibleSelection)
            UnoRuntime.queryInterface(XAccessibleSelection.class,
            accContextTabList);

        final int queriesChildNum = 3;
        try {
            tabListSelection.selectAccessibleChild(queriesChildNum);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't select child " + queriesChildNum));
        }

        shortWait();

        XAccessibleContext acEditQuery = at.getAccessibleObjectForRole(
            xRoot, AccessibleRole.PUSHBUTTON, "Edit Query");
        XAccessibleAction actionEditQuery = (XAccessibleAction)
            UnoRuntime.queryInterface(XAccessibleAction.class, acEditQuery);

        try {
            actionEditQuery.doAccessibleAction(0);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't do accessible action"));
        }

        shortWait();

        atw = tk.getActiveTopWindow();

        xWindow = (XWindow)
            UnoRuntime.queryInterface(XWindow.class, atw);

        queryWindow = xWindow;

        xRoot = at.getAccessibleObject(xWindow);

//        at.printAccessibleTree(log, xRoot);

        oObj = at.getAccessibleObjectForRole(
            xRoot, AccessibleRole.PANEL);

/*        for(int i = 0; i < savedObj.length; i++) {
            try {
                namingService.registerObject(elNames[i], savedObj[i]);
            } catch(com.sun.star.uno.Exception e) {
                e.printStackTrace(log);
            }
        }*/

        log.println("ImplementationName " + util.utils.getImplName(oObj));

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        final XAccessibleComponent acc = (XAccessibleComponent)
                UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    acc.grabFocus();
                }
            });

        return tEnv;
    } // finish method getTestEnvironment

    /**
     * Closes the DatasourceAdministration dialog and Query Dialog.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        queryWindow.dispose();

        shortWait();

        try {
            actionCancel.doAccessibleAction(0);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed(
                "Couldn't do accessible action"));
        }
    }

    /**
     * Thread for opening modal dialog 'Datasource Administration Dialog'.
     */
    public class DiagThread extends Thread {
        XMultiServiceFactory msf = null;

        public DiagThread(XMultiServiceFactory msf) {
            this.msf = msf;
        }

        public void run() {
            Object dbAdminDlg = null;

            try {
                dbAdminDlg = msf.createInstance(
                    "com.sun.star.sdb.DatasourceAdministrationDialog");
            } catch(com.sun.star.uno.Exception e) {}

            XExecutableDialog adminDlg = (XExecutableDialog)
                UnoRuntime.queryInterface(XExecutableDialog.class, dbAdminDlg);

            adminDlg.execute();
        }
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}

