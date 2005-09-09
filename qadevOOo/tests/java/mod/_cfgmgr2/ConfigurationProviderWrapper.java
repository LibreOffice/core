/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationProviderWrapper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:50:51 $
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

public class ConfigurationProviderWrapper extends TestCase {

    /** Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     *
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;

        try {
            PropertyValue[] cArgs = new PropertyValue[2];
            cArgs[0] = new PropertyValue();
            cArgs[0].Name = "Locale";
            cArgs[0].Value = "DE";
            cArgs[1] = new PropertyValue();
            cArgs[1].Name = "EnableAsync";
            cArgs[1].Value = new Boolean(true);

            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstanceWithArguments("com.sun.star.comp.configuration.ConfigurationProvider",cArgs);
        } catch (com.sun.star.uno.Exception e) {
        }

        log.println("Implementation name: "+ util.utils.getImplName(oObj));

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
        tEnv.addObjRelation("needArgs", "ConfigurationProvider");

        return tEnv;

    }

}
