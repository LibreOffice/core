/*************************************************************************
 *
 *  $RCSfile: StartTest.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:29 $
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
package workben.tt;

import java.lang.Object;
import java.io.*;
import workben.odbc.*;

import com.sun.star.comp.servicemanager.ServiceManager;
//  import java.io.IOException;

//import com.sun.star.beans.PropertyValue;
//import com.sun.star.beans.PropertyState;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;
import com.sun.star.bridge.XBridge;

//  import com.sun.star.comp.bootstrap.Bootstrap;

//  import com.sun.star.io.XInputStream;
//  import com.sun.star.io.XOutputStream;

//  import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;

import com.sun.star.uno.IBridge;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;

import com.sun.star.sdbc.*;
//  import com.sun.star.sdbc.XConnection;
//  import com.sun.star.uno.Enum;

//  import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;
//  import com.sun.star.lib.uno.typeinfo.TypeInfo;

public class StartTest
{
  //###########################################################
  //#                   allgemeine Variablen                  #
  //###########################################################
  private static java.util.Hashtable ConfigItems = new java.util.Hashtable();
  //###########################################################
  //#                   ResultSetToStringBuffer               #
  //###########################################################
  static StringBuffer ResultSetToStringBuffer(com.sun.star.sdbc.XResultSet results)
  {
          com.sun.star.sdbc.XRow                           Row;
          com.sun.star.sdbc.XResultSetMetaDataSupplier     rsmds;
          com.sun.star.sdbc.XResultSetMetaData             rsmd;

          StringBuffer buf = new StringBuffer();

          try
          {
            rsmds = (com.sun.star.sdbc.XResultSetMetaDataSupplier)UnoRuntime.queryInterface(com.sun.star.sdbc.XResultSetMetaDataSupplier.class,results);
            rsmd = rsmds.getMetaData();
            int numCols = rsmd.getColumnCount();
            int i;

            // get column header info
            for (i=1; i <= numCols; i++)
            {
                if (i == 1) buf.append("\"");
                if (i > 1) buf.append("\"\t\"");
                buf.append(rsmd.getColumnLabel(i));
            }
            buf.append("\"\r\n");



            while (results.next())
            {
                Row = (com.sun.star.sdbc.XRow)UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class, results);
                for (i=1; i <= numCols; i++)
                {
                    if (i == 1) buf.append("\"");
                    if (i > 1) buf.append("\"\t\"");
                    buf.append(Row.getString(i));
                }
                buf.append("\"\r\n");
            }
            return(buf);
          }
          catch (Exception e)
          {
            System.out.println("Exception at ResultSetToStringBuffer : " + e);
            return(buf);
          }
  }

  //###########################################################
  //#                   printStringBuffer                     #
  //###########################################################

  static void printStringBuffer(StringBuffer buf)
  {
    FileWriter f1;
    try
    {
      f1 = new FileWriter("d:\\data\\sdbc\\test.log",true);
      int maxStrings = buf.length() - 1;
      //System.out.println(maxStrings);
      for(int i=0 ; i<=maxStrings ; i++)
      {
        f1.write(buf.charAt(i));
      }
      f1.close();
    }
    catch( IOException e )
    {
      System.out.println("Error: can't create logfile");
    }
  }

  //###########################################################
  //#                   neededServices                        #
  //###########################################################
  static String neededServices[] = new String[] {
        "com.sun.star.comp.servicemanager.ServiceManager",
        "com.sun.star.comp.loader.JavaLoader",
        "com.sun.star.comp.connections.Connector",
          "com.sun.star.comp.connections.Acceptor"
    };

  //###########################################################
  //#                   readConfigFile                        #
  //###########################################################
  static void readConfigFile(String sFileName)
  {
    BufferedReader confFile;
    try
    {
      confFile = new BufferedReader(new FileReader(sFileName));
      ConfigItems.put("ConnectString",confFile.readLine());
      ConfigItems.put("user",confFile.readLine());
      ConfigItems.put("password",confFile.readLine());
      ConfigItems.put("ToTest",confFile.readLine());
      ConfigItems.put("LogFile",confFile.readLine());
      confFile.close();
    }
    catch(Exception e)
    {
      System.out.println(e);
    }
  }

  //###########################################################
  //#                   MAIN                                  #
  //###########################################################

  public static void main(String argv[]) throws Exception
    {

    //Die Parameter auslesen

//    if(argv.length == 0)
//    {
//      System.out.println("missing parameter");
//      System.exit(0);
//    }

    //readConfigFile(argv[0]);

    ////////////////////////

    com.sun.star.comp.servicemanager.ServiceManager smgr = new com.sun.star.comp.servicemanager.ServiceManager();
        smgr.addFactories(neededServices);

        XConnector  xConnector  = (XConnector)smgr.createInstance("com.sun.star.connection.Connector");
    if(xConnector == null) System.err.println("no connector!");

        XConnection xConn       = xConnector.connect("socket,host=localhost,port=6001");
    if(xConn == null) System.err.println("no XConnection!");

        IBridge iBridge = UnoRuntime.getBridgeByName("java", null, "remote", null, new Object[]{"iiop", xConn, null});

        Object rInitialObject = iBridge.mapInterfaceFrom("classic_uno", XInterface.class);

        if(rInitialObject != null)
        {
            System.err.println("got the remote object");
      System.out.println("before naming service !");
      try
      {
        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(XNamingService.class, rInitialObject );
        try
        {
          if(rName != null)
          {
            System.err.println("got the remote naming service !");
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager");
            XMultiServiceFactory rSmgr = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, rXsmgr);
            if(rSmgr != null)
            {
              System.out.println("got the remote service manager !");
              Object rDriver = rSmgr.createInstance("com.sun.star.sdbc.ADriver");
              if(rDriver != null)
              {
                System.out.println("got a com.sun.star.sdbc.Driver !");
                com.sun.star.sdbc.XDriver  xDriver  = (XDriver)UnoRuntime.queryInterface(com.sun.star.sdbc.XDriver.class,rDriver);
                if(xDriver != null)
                {
                  com.sun.star.sdbc.XConnection xConnection = null;
                  try
                  {
                    com.sun.star.beans.PropertyValue [] ConInfo = new com.sun.star.beans.PropertyValue[]
                    {
                      new com.sun.star.beans.PropertyValue("user",0,"qsuser",com.sun.star.beans.PropertyState.DIRECT_VALUE),
                      new com.sun.star.beans.PropertyValue("password",0,"qsuser",com.sun.star.beans.PropertyState.DIRECT_VALUE)
                    };
                    xConnection = xDriver.connect("sdbc:ado:PROVIDER=SQLOLEDB;DATA SOURCE=sqllab2",ConInfo);

                    if(xConnection != null)
                    {
                      System.out.println("got a connection!");
                      com.sun.star.sdbc.XDatabaseMetaData dmd;
                      com.sun.star.sdbc.XStatement    stmt;
                      com.sun.star.sdbc.XResultSet   result;
                      com.sun.star.sdbc.XRow         row;
                      try
                      {
                        stmt = xConnection.createStatement();
                        stmt.executeUpdate("insert into testtab values(4,'vier')");
                        result = stmt.executeQuery("select * from testtab where int = 4");
                        com.sun.star.sdbc.XResultSetMetaDataSupplier     rsmds;

                        rsmds = (com.sun.star.sdbc.XResultSetMetaDataSupplier)UnoRuntime.queryInterface(com.sun.star.sdbc.XResultSetMetaDataSupplier.class,result);
                        com.sun.star.sdbc.XResultSetMetaData  xMD = rsmds.getMetaData();
                        System.out.println("Anzahl Spalten = " + xMD.getColumnCount());
                        if(result.next())
                        {
                            row = (com.sun.star.sdbc.XRow)UnoRuntime.queryInterface(com.sun.star.sdbc.XRow.class,result);
                            String xx = row.getString(1);
                        }
                      }
                      catch (Exception e)
                      {
                      System.out.println("Exception beim Aufruf der Tests");
                      }
                    }
                  }
                  catch(Exception e)
                  {
                    System.out.println("exception while connecting!");
                    e.printStackTrace();
                  }
                  xConnection.close();
                }
              }
            }
          }
        }
        catch(Exception t)
        {
        }
      }
      catch(Exception e1)
      {
        System.err.println("exception from getRegisteredObject!");
      }
    }
    System.out.println("FERTIG");
    System.exit(0);
  }
  }// die Klammer der ganzen Klasse

