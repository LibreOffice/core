/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScDocumentConfiguration.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package mod._qadevOOo;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;

import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;

public class SelfTest extends TestCase
{
    private XTextDocument xTextDoc = null;

    protected void initialize(TestParameters tParam, PrintWriter log)
    {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory) tParam.getMSF());

        log.println("initialize the selftest");
        try
        {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
     * Disposes text document.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log)
    {
        log.println("    cleanup selftest");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log)
    {
        XInterface oObj = null;
        log.println("creating a test environment");

        // get the bodytext of textdocument here
        log.println("getting the TextCursor");

        final XSimpleText aText = xTextDoc.getText();
        final XTextCursor textCursor = aText.createTextCursor();
        oObj = textCursor;

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }
}
