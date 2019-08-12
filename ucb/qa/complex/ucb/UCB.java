/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package complex.ucb;

import com.sun.star.beans.Property;
import com.sun.star.sdbc.XRow;
import com.sun.star.ucb.*;
import com.sun.star.uno.UnoRuntime;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * This class is used to copy the content of a folder to
 * another folder.
 * There is an inconsistency with argument order.
 * It should be always: dir,filename.
 */
public class UCB  {
    private XUniversalContentBroker ucb;

    public void init() throws Exception {
        ucb = UniversalContentBroker.create(connection.getComponentContext());
    }

    public void delete(String filename) throws Exception {
        executeCommand(getContent(filename), "delete", Boolean.TRUE);
    }

    /**
     * target name can be "", in which case the name stays lige the source name
     */
    public Object executeCommand(
        Object xContent,
        String aCommandName,
        Object aArgument)
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        XCommandProcessor xCmdProcessor =
            UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
        Command aCommand = new Command();
        aCommand.Name = aCommandName;
        aCommand.Handle = -1; // not available
        aCommand.Argument = aArgument;
        return xCmdProcessor.execute(aCommand, 0, null);
    }

    public Object getContentProperty(
        Object content,
        String propName,
        Class type)
        throws Exception {
        Property[] pv = new Property[1];
        pv[0] = new Property();
        pv[0].Name = propName;
        pv[0].Handle = -1;

        Object row = executeCommand(content, "getPropertyValues", pv);
        XRow xrow = UnoRuntime.queryInterface(XRow.class, row);
        if (type.equals(String.class))
                {
                    return xrow.getString(1);
                }
        else if (type.equals(Boolean.class))
                {
                    return xrow.getBoolean(1) ? Boolean.TRUE : Boolean.FALSE;
                }
        else if (type.equals(Integer.class))
                {
                    return Integer.valueOf(xrow.getInt(1));
                }
        else if (type.equals(Short.class))
                {
                    return Short.valueOf(xrow.getShort(1));
                }
        else
                {
                    return null;
                }

    }

    public Object getContent(String path) throws Exception
        {
        XContentIdentifier id = ucb.createContentIdentifier(path);
        return ucb.queryContent(id);
    }

    @Test public void checkWrongFtpConnection() {
        try {
            String acountUrl = "ftp://noname:nopasswd@*nohost.invalid";
            System.out.println(acountUrl);
            init();
            Object content = getContent(acountUrl);

            OpenCommandArgument2 aArg = new OpenCommandArgument2();
            aArg.Mode = OpenMode.ALL; // FOLDER, DOCUMENTS -> simple filter
            aArg.Priority = 32768; // Ignored by most implementations

            System.out.println("now executing open");
            executeCommand(content, "open", aArg);
            fail("Expected 'IllegalArgumentException' was not thrown.");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            //TODO error message;
            System.out.println("Correct exception thrown: " + ex.getClass().toString());
        } catch(com.sun.star.ucb.InteractiveNetworkException ex) {
            System.out.println("This Exception is correctly thrown when no Proxy in StarOffice is used.");
            System.out.println("To reproduce the bug behaviour, use a Proxy and try again.");
        } catch (Exception ex) {
            ex.printStackTrace();
            String exceptionName = ex.toString();
            System.out.println("ExName: '"+exceptionName+"'");
            fail("Wrong exception thrown: " + exceptionName);
        }
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
