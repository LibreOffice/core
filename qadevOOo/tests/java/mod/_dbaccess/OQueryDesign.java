/*************************************************************************
 *
 *  $RCSfile: OQueryDesign.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2004-08-02 17:56:59 $
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
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;

public class OQueryDesign extends TestCase {

    XDesktop Desk;
    XFrame Frame;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing Query");

        if (Frame != null) {
            Frame.dispose();
        }
    }

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

        Frame = Desk.getCurrentFrame();

        Object[] params = new Object[3];
        param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        params[0] = param1;
        param2 = new PropertyValue();
        param2.Name = "Frame";
        param2.Value = Frame;
        params[1] = param2;
        PropertyValue param3 = new PropertyValue();
        param3.Name = "QueryDesignView";
        param3.Value = new Boolean(true);
        params[2] = param3;

        oObj = Frame.getController();


        TestEnvironment tEnv = new TestEnvironment(oObj);

        //adding objRelation for XController
        tEnv.addObjRelation ("FirstModel", Frame.getController ().getModel());

        //Adding ObjRelations for XInitialization
        tEnv.addObjRelation("XInitialization.args", params);

        tEnv.addObjRelation("Frame", Frame);

        log.println("ImplementationName"+util.utils.getImplName(oObj));

        return tEnv;

    } // finish method getTestEnvironment

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

