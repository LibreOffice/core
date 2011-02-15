/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package complex.sfx2;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;



import util.DesktopTools;
import util.WriterTools;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

public class DocumentInfo
{
    XMultiServiceFactory m_xMSF = null;
    XTextDocument xTextDoc = null;
    XTextDocument xTextDocSecond = null;

    @Test public void checkDocInfo()
    {
        m_xMSF = getMSF();

        System.out.println("check wether there is a valid MultiServiceFactory");

        assertNotNull("## Couldn't get MultiServiceFactory make sure your Office is started", m_xMSF);

        // TODO: need other temp directory!
        String tempdir = System.getProperty("java.io.tmpdir");
        String fs = System.getProperty("file.separator");

        if (!tempdir.endsWith(fs))
        {
            tempdir += fs;
        }
        tempdir = util.utils.getFullURL(tempdir);
        final String sTempDocument = tempdir + "DocInfo.oot";

        if (true)
        {
            System.out.println("... done");


            System.out.println("Opening a Writer document");
            xTextDoc = WriterTools.createTextDoc(m_xMSF);
            System.out.println("... done");

            XDocumentInfoSupplier xDocInfoSup = UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDoc);
            XDocumentInfo xDocInfo = xDocInfoSup.getDocumentInfo();
            XPropertyContainer xPropContainer = UnoRuntime.queryInterface(XPropertyContainer.class, xDocInfo);

            System.out.println("Trying to add a existing property");

            boolean worked = addProperty(xPropContainer, "Author", (short) 0, "");
            assertTrue("Could set an existing property", !worked);
            System.out.println("...done");

            System.out.println("Trying to add a integer property");
            worked = addProperty(xPropContainer, "intValue", com.sun.star.beans.PropertyAttribute.READONLY, new Integer(17));
            assertTrue("Couldn't set an integer property", worked);
            System.out.println("...done");

            System.out.println("Trying to add a double property");
            worked = addProperty(xPropContainer, "doubleValue", com.sun.star.beans.PropertyAttribute.REMOVEABLE, new Double(17.7));
            assertTrue("Couldn't set an double property", worked);
            System.out.println("...done");

            System.out.println("Trying to add a boolean property");
            worked = addProperty(xPropContainer, "booleanValue", com.sun.star.beans.PropertyAttribute.REMOVEABLE, Boolean.TRUE);
            assertTrue("Couldn't set an boolean property", worked);
            System.out.println("...done");

            System.out.println("Trying to add a date property");
            worked = addProperty(xPropContainer, "dateValue", com.sun.star.beans.PropertyAttribute.REMOVEABLE, new Date());
            assertTrue("Couldn't set an date property", worked);
            System.out.println("...done");

            System.out.println("trying to remove a read only Property");
            try
            {
                xPropContainer.removeProperty("intValue");
                fail("Could remove read only property");
            }
            catch (Exception e)
            {
                System.out.println("\tException was thrown " + e);
                System.out.println("\t...OK");
            }
            System.out.println("...done");

            XPropertySet xProps2 = UnoRuntime.queryInterface(XPropertySet.class, xPropContainer);
            showPropertySet(xProps2);


            System.out.println("Storing the document");
            try
            {
                XStorable store = UnoRuntime.queryInterface(XStorable.class, xTextDoc);
                store.storeToURL(sTempDocument, new PropertyValue[] {});
                DesktopTools.closeDoc(xTextDoc);
            }
            catch (Exception e)
            {
                fail("Couldn't store document");
            }

            System.out.println("...done");
        }


        if (true)
        {
            System.out.println("loading the document");

            try
            {
                XComponentLoader xCL = UnoRuntime.queryInterface(XComponentLoader.class, m_xMSF.createInstance("com.sun.star.frame.Desktop"));
                XComponent xComp = xCL.loadComponentFromURL(sTempDocument, "_blank", 0, new PropertyValue[] {});
                xTextDocSecond = UnoRuntime.queryInterface(XTextDocument.class, xComp);
            }
            catch (Exception e)
            {
                fail("Couldn't load document");
            }

            System.out.println("...done");

            XDocumentInfoSupplier xDocInfoSup = UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDocSecond);
            XDocumentInfo xDocInfo = xDocInfoSup.getDocumentInfo();
            XPropertyContainer xPropContainer = UnoRuntime.queryInterface(XPropertyContainer.class, xDocInfo);

            XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class, xDocInfo);
            showPropertySet(xProps);

            assertTrue("Double doesn't work", checkType(xProps, "doubleValue", "java.lang.Double"));
            assertTrue("Boolean doesn't work", checkType(xProps, "booleanValue", "java.lang.Boolean"));

            // TODO: dateValue does not exist.
            // assertTrue("Date doesn't work",     checkType(xProps, "dateValue", "com.sun.star.util.DateTime"));

            // TODO: is java.lang.Double
            // assertTrue("Integer doesn't work",  checkType(xProps, "intValue", "java.lang.Integer"));

            xPropContainer = UnoRuntime.queryInterface(XPropertyContainer.class, xDocInfo);

            System.out.println("trying to remove a not user defined Property");
            try
            {
                xPropContainer.removeProperty("Author");
                fail("Could remove non user defined property");
            }
            catch (Exception e)
            {
                System.out.println("\tException was thrown " + e);
                System.out.println("\t...OK");
            }
            System.out.println("...done");


            System.out.println("Trying to remove a user defined property");
            try
            {
                xPropContainer.removeProperty("booleanValue");
                System.out.println("\t...OK");
            }
            catch (Exception e)
            {
                System.out.println("\tException was thrown " + e);
                System.out.println("\t...FAILED");
                fail("Could not remove user defined property");
            }
            showPropertySet(xProps);
            System.out.println("...done");
        }
    }

    @After public void cleanup()
    {
        DesktopTools.closeDoc(xTextDocSecond);
        DesktopTools.closeDoc(xTextDoc);
    }

    private void showPropertySet(XPropertySet xProps)
        {
            try
            {
                // get an XPropertySet, here the one of a text cursor
                // XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, mxDocCursor);

                // get the property info interface of this XPropertySet
                XPropertySetInfo xPropsInfo = xProps.getPropertySetInfo();

                // get all properties (NOT the values) from XPropertySetInfo
                Property[] aProps = xPropsInfo.getProperties();
                int i;
                for (i = 0; i < aProps.length; ++i) {
                    // number of property within this info object
                    System.out.print("Property #" + i);

                    // name of property
                    System.out.print(": Name<" + aProps[i].Name);

                    // handle of property (only for XFastPropertySet)
                    System.out.print("> Handle<" + aProps[i].Handle);

                    // type of property
                    System.out.print("> " + aProps[i].Type.toString());

                    // attributes (flags)
                    System.out.print(" Attributes<");
                    short nAttribs = aProps[i].Attributes;
                    if ((nAttribs & PropertyAttribute.MAYBEVOID) != 0)
                        System.out.print("MAYBEVOID|");
                    if ((nAttribs & PropertyAttribute.BOUND) != 0)
                        System.out.print("BOUND|");
                    if ((nAttribs & PropertyAttribute.CONSTRAINED) != 0)
                        System.out.print("CONSTRAINED|");
                    if ((nAttribs & PropertyAttribute.READONLY) != 0)
                        System.out.print("READONLY|");
                    if ((nAttribs & PropertyAttribute.TRANSIENT) != 0)
                        System.out.print("TRANSIENT|");
                    if ((nAttribs & PropertyAttribute.MAYBEAMBIGUOUS ) != 0)
                        System.out.print("MAYBEAMBIGUOUS|");
                    if ((nAttribs & PropertyAttribute.MAYBEDEFAULT) != 0)
                        System.out.print("MAYBEDEFAULT|");
                    if ((nAttribs & PropertyAttribute.REMOVEABLE) != 0)
                        System.out.print("REMOVEABLE|");
                    System.out.println("0>");
                }
            } catch (Exception e) {
                // If anything goes wrong, give the user a stack trace
                e.printStackTrace(System.out);
            }
        }

    private boolean checkType(XPropertySet xProps, String aName,
            String expected)
    {
        boolean ret = true;
        System.out.println("Checking " + expected);

        String getting =
                getPropertyByName(xProps, aName).getClass().getName();

        if (!getting.equals(expected))
        {
            System.out.println("\t Expected: " + expected);
            System.out.println("\t Getting:  " + getting);
            ret = false;
        }

        if (ret)
        {
            System.out.println("...OK");
        }
        return ret;
    }

    private Object getPropertyByName(XPropertySet xProps, String aName)
    {
        Object ret = null;

        try
        {
            ret = xProps.getPropertyValue(aName);
        }
        catch (Exception e)
        {
            System.out.println("\tCouldn't get Property " + aName);
            System.out.println("\tMessage " + e);
        }

        return ret;
    }

    private boolean addProperty(XPropertyContainer xPropContainer,
            String aName, short attr, Object defaults)
    {
        boolean ret = true;

        try
        {
            xPropContainer.addProperty(aName, attr, defaults);
        }
        catch (Exception e)
        {
            ret = false;
            System.out.println("\tCouldn't get Property " + aName);
            System.out.println("\tMessage " + e);
        }

        return ret;
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception
    {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "starting class: " + DocumentInfo.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "finishing class: " + DocumentInfo.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
