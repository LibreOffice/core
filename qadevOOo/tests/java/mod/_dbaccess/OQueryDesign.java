/*************************************************************************
 *
 *  $RCSfile: OQueryDesign.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change:$Date: 2005-03-29 11:58:25 $
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

//import com.sun.star.awt.XControl;
//import com.sun.star.awt.XControlModel;
import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import lib.StatusException;
import util.SOfficeFactory;
import com.sun.star.sdb.XDocumentDataSource;

public class OQueryDesign extends TestCase {

    private XDesktop Desk;
    private XFrame Frame;
    private final String sDataSourceName = "Bibliography";

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        Desk = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop((XMultiServiceFactory)Param.getMSF()) );
    }

    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        XInterface oObj = null;

        XDispatchProvider aProv = (XDispatchProvider)
                UnoRuntime.queryInterface(XDispatchProvider.class,Desk);

        XDispatch getting = null;

        log.println( "opening QueryDesign" );
        URL the_url = new URL();
        the_url.Complete = ".component:DB/QueryDesign";
        getting = aProv.queryDispatch(the_url,"Query",12);
        PropertyValue[] Args = new PropertyValue[2];
        PropertyValue param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        Args[0] = param1;
        PropertyValue param2 = new PropertyValue();
        param2.Name = "QueryDesignView";
        param2.Value = new Boolean(false);
        Args[1] = param2;
        getting.dispatch(the_url,Args);

        shortWait();

        Object oDBC = null;
        XMultiServiceFactory xMSF;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oDBC = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );
        }
        catch( com.sun.star.uno.Exception e ) {
            throw new StatusException("Could not instantiate DatabaseContext", e) ;
        }

        Object oDataSource = null;
        try{
            XNameAccess xNA = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBC);
            oDataSource = xNA.getByName(sDataSourceName);
        } catch ( com.sun.star.container.NoSuchElementException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        } catch ( com.sun.star.lang.WrappedTargetException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        }
        XDocumentDataSource xDDS = (XDocumentDataSource) UnoRuntime.queryInterface(XDocumentDataSource.class, oDataSource);
        XModel xMod = (XModel) UnoRuntime.queryInterface(XModel.class, xDDS.getDatabaseDocument ());

        Frame = xMod.getCurrentController().getFrame();

         // get an instance of Frame
        Object oFrame = null;
        XTextDocument xTextDoc = null;;
        SOfficeFactory SOF = null;

        SOF = SOfficeFactory.getFactory( xMSF );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Could not create document", e );
        }

        XModel xDocMod = (XModel) UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XFrame xTextFrame  = xDocMod.getCurrentController().getFrame();

        Object[] params = new Object[3];
        param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        params[0] = param1;
        param2 = new PropertyValue();
        param2.Name = "Frame";
        param2.Value = xTextFrame;
        params[1] = param2;
        PropertyValue param3 = new PropertyValue();
        param3.Name = "QueryDesignView";
        param3.Value = new Boolean(true);
        params[2] = param3;

        Object[] ExceptionParams = new Object[3];
        ExceptionParams = params;
        ((PropertyValue) ExceptionParams[1]).Value = Frame;

        oObj = Frame.getController();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //Adding ObjRelations for XInitialization
        tEnv.addObjRelation("XInitialization.args", params);

        tEnv.addObjRelation("XInitialization.ExceptionArgs", params);

        tEnv.addObjRelation("Frame", Frame);

        tEnv.addObjRelation("XInitialization.xIni", getUnititializedObj(Param));

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        return tEnv;

    } // finish method getTestEnvironment

    private XInitialization getUnititializedObj(TestParameters Param){
        // creating an object wihch ist not initialized

        // get a model of a DataSource
        Object oDBC = null;
        XMultiServiceFactory xMSF;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oDBC = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );
        }
        catch( com.sun.star.uno.Exception e ) {
            throw new StatusException("Could not instantiate DatabaseContext", e) ;
        }

        Object oDataSource = null;
        try{
            XNameAccess xNA = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oDBC);
            oDataSource = xNA.getByName(sDataSourceName);
        } catch ( com.sun.star.container.NoSuchElementException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        } catch ( com.sun.star.lang.WrappedTargetException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        }

        XDocumentDataSource xDDS = (XDocumentDataSource) UnoRuntime.queryInterface(XDocumentDataSource.class, oDataSource);
        XModel xMod = (XModel) UnoRuntime.queryInterface(XModel.class, xDDS.getDatabaseDocument ());

        // get an intaces of QueryDesign
        Object oQueryDesign = null;
        try{
            oQueryDesign = xMSF.createInstance("com.sun.star.sdb.QueryDesign");
        }catch( com.sun.star.uno.Exception e ) {
            throw new StatusException("Could not instantiate QueryDesign", e) ;
        }

        XController xCont = (XController) UnoRuntime.queryInterface(XController.class, oQueryDesign);

        // marry them all
        xCont.attachModel(xMod);
        xMod.connectController(xCont);
        try{
            xMod.setCurrentController(xCont);
        } catch (com.sun.star.container.NoSuchElementException e){
            throw new StatusException("Could not set controller", e) ;
        }

        //xCont.attachFrame(xFrame);

        return (XInitialization) UnoRuntime.queryInterface(XInitialization.class, oQueryDesign);

    }

    /**
    * Sleeps for 0.2 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(5000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }

}    // finish class oDatasourceBrowser

