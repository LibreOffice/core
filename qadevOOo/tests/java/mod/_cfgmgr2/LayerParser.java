/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayerParser.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:51:19 $
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

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.File;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
 *
 * @author  sw93809
 */
public class LayerParser extends TestCase {


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
        Object Pipe = null;
        XSimpleFileAccess simpleAccess = null;
        XInputStream xStream = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstance("com.sun.star.comp.configuration.backend.xml.LayerParser");
            Object fileacc = ((XMultiServiceFactory)tParam.getMSF()).createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            simpleAccess = (XSimpleFileAccess)
                            UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
            String filename = util.utils.getOfficeURL((XMultiServiceFactory)tParam.getMSF())+"/../share/registry/data/org/openoffice/Setup.xcu";
            log.println("Going to parse: "+filename);
            xStream = simpleAccess.openFileRead(filename);
        } catch (com.sun.star.uno.Exception e) {
        }

        log.println("Implementation name: "+ util.utils.getImplName(oObj));

        XActiveDataSink xSink = (XActiveDataSink) UnoRuntime.queryInterface(XActiveDataSink.class, oObj);
        xSink.setInputStream(xStream);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("InputStream", xStream);

        return tEnv;
    }

}