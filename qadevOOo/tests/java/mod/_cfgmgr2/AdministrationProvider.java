/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AdministrationProvider.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:50:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._cfgmgr2;

import java.io.PrintWriter;
import java.util.Vector;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;


public class AdministrationProvider extends TestCase {

    /** Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects.
     *
     * For this an instance of com.sun.star.comp.configuration.AdministrationProvider
     * is created at the MultiServiceFactory of the Office
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     *
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstance("com.sun.star.comp.configuration.AdministrationProvider");
        } catch (com.sun.star.uno.Exception e) {
        }

        util.dbg.printInterfaces(oObj);

        PropertyValue[] nodeArgs = new PropertyValue[1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = "org.openoffice.Setup";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        Vector args = new Vector();

        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);
        args.add(0, nodeArgs);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XMSF.Args",
                            args.toArray(new Object[args.size()][]));
        tEnv.addObjRelation("needArgs", "AdministrationProvider");

        return tEnv;
    }
}