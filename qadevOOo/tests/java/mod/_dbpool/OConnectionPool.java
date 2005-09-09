/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OConnectionPool.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:04:11 $
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

package mod._dbpool;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdbc.ConnectionPool</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sdbc::XDriverManager</code></li>
* </ul>
* @see com.sun.star.sdbc.XDriverManager
* @see ifc.sdbc.XDriverManager
*/
public class OConnectionPool extends TestCase {
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)
                xMSF.createInstance("com.sun.star.sdbc.ConnectionPool");
        } catch(com.sun.star.uno.Exception e) {
            throw new StatusException(
                Status.failed("Couldn't create instance"));
        }

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //adding relations for XDriverManager
        String dbaseURL = (String) Param.get("dbase.url");
        if (dbaseURL == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'dbase.url'"));
        }

        tEnv.addObjRelation("SDBC.URL", "sdbc:dbase:" + dbaseURL);

        String jdbcURL = (String) Param.get("jdbc.url");
        if (jdbcURL == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'jdbc.url'"));
        }

        tEnv.addObjRelation("JDBC.URL", "jdbc:" + jdbcURL);

        String jdbcUser = (String) Param.get("jdbc.user");
        if (jdbcUser == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'jdbc.user'"));
        }

        String jdbcPassword = (String) Param.get("jdbc.password");
        if (jdbcPassword == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'jdbc.password'"));
        }

        PropertyValue[] jdbcInfo = new PropertyValue[3];
        jdbcInfo[0] = new PropertyValue();
        jdbcInfo[0].Name = "user";
        jdbcInfo[0].Value = jdbcUser;
        jdbcInfo[1] = new PropertyValue();
        jdbcInfo[1].Name = "password";
        jdbcInfo[1].Value = jdbcPassword;
        jdbcInfo[2] = new PropertyValue();
        jdbcInfo[2].Name = "JavaDriverClass";
        jdbcInfo[2].Value = DBTools.TST_JDBC_DRIVER;

        tEnv.addObjRelation("JDBC.INFO", jdbcInfo);

        return tEnv;
    }
}
