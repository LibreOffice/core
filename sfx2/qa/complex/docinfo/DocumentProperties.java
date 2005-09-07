/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentProperties.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:26:47 $
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
package complex.docinfo;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.Date;

import complexlib.ComplexTestCase;

import util.DesktopTools;
import util.WriterTools;


public class DocumentProperties extends ComplexTestCase {
    XMultiServiceFactory m_xMSF = null;
    XTextDocument xTextDoc = null;

    public String[] getTestMethodNames() {
        return new String[] {"checkDocInfo", "cleanup"};
    }

    public void checkDocInfo() {
        m_xMSF = (XMultiServiceFactory) param.getMSF();

        log.println(
            "check wether there is a valid MultiServiceFactory");

        if (m_xMSF == null) {
            assure("## Couldn't get MultiServiceFactory make sure your Office is started",
                true);
        }

        log.println("... done");

        log.println("Opening a Writer document");
        xTextDoc = WriterTools.createTextDoc(m_xMSF);
        log.println("... done");

        XDocumentInfoSupplier xDocInfoSup =
            (XDocumentInfoSupplier) UnoRuntime.queryInterface(XDocumentInfoSupplier.class,
                xTextDoc);
        XDocumentInfo xDocInfo = xDocInfoSup.getDocumentInfo();
        XPropertyContainer xPropContainer =
            (XPropertyContainer) UnoRuntime.queryInterface(XPropertyContainer.class,
                xDocInfo);

        log.println("Trying to add a existing property");

        boolean worked =
            addProperty(xPropContainer, "Author", (short) 0, "");
        assure("Could set an existing property", !worked);
        log.println("...done");

        log.println("Trying to add a integer property");
        worked =
            addProperty(xPropContainer, "intValue", com.sun.star.beans.PropertyAttribute.READONLY,
                new Integer(17));
        assure("Couldn't set an integer property", worked);
        log.println("...done");

        log.println("Trying to add a double property");
        worked =
            addProperty(xPropContainer, "doubleValue", com.sun.star.beans.PropertyAttribute.REMOVEABLE ,
                new Double(17.7));
        assure("Couldn't set an double property", worked);
        log.println("...done");

        log.println("Trying to add a boolean property");
        worked =
            addProperty(xPropContainer, "booleanValue", com.sun.star.beans.PropertyAttribute.REMOVEABLE,
                Boolean.TRUE);
        assure("Couldn't set an boolean property", worked);
        log.println("...done");

        log.println("Trying to add a date property");
        worked =
            addProperty(xPropContainer, "dateValue", com.sun.star.beans.PropertyAttribute.REMOVEABLE,
                new Date());
        assure("Couldn't set an date property", worked);
        log.println("...done");

        log.println("trying to remove a read only Property");
        try {
            xPropContainer.removeProperty ("intValue");
            assure("Could remove read only property", false);
        } catch (Exception e) {
            log.println("\tException was thrown "+e);
            log.println("\t...OK");
        }
        log.println("...done");


        String tempdir = System.getProperty("java.io.tmpdir");
        String fs = System.getProperty("file.separator");

        if (!tempdir.endsWith(fs)) {
            tempdir += fs;
        }

        tempdir = util.utils.getFullURL(tempdir);

        log.println("Storing the document");

        try {
            XStorable store =
                (XStorable) UnoRuntime.queryInterface(XStorable.class,
                    xTextDoc);
            store.storeToURL(tempdir + "DocInfo.oot",
                new PropertyValue[] {});
            DesktopTools.closeDoc(xTextDoc);
        } catch (Exception e) {
            assure("Couldn't store document", false);
        }

        log.println("...done");

        log.println("loading the document");

        try {
            XComponentLoader xCL =
                (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class,
                    m_xMSF.createInstance(
                        "com.sun.star.frame.Desktop"));
            XComponent xComp =
                xCL.loadComponentFromURL(tempdir + "DocInfo.oot",
                    "_blank", 0, new PropertyValue[] {});
            xTextDoc =
                (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,
                    xComp);
        } catch (Exception e) {
            assure("Couldn't load document", false);
        }

        log.println("...done");

        xDocInfoSup =
            (XDocumentInfoSupplier) UnoRuntime.queryInterface(XDocumentInfoSupplier.class,
                xTextDoc);
        xDocInfo = xDocInfoSup.getDocumentInfo();

        XPropertySet xProps =
            (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,
                xDocInfo);

        assure("Integer doesn't work",
            checkType(xProps, "intValue", "java.lang.Integer"));
        assure("Double doesn't work",
            checkType(xProps, "doubleValue", "java.lang.Double"));
        assure("Boolean doesn't work",
            checkType(xProps, "booleanValue", "java.lang.Boolean"));
        assure("Date doesn't work",
            checkType(xProps, "dateValue",
                "com.sun.star.util.DateTime"));

                xPropContainer =
            (XPropertyContainer) UnoRuntime.queryInterface(XPropertyContainer.class,
                xDocInfo);

        log.println("trying to remove a not user defined Property");
        try {
            xPropContainer.removeProperty ("Author");
            assure("Could remove non user defined property", false);
        } catch (Exception e) {
            log.println("\tException was thrown "+e);
            log.println("\t...OK");
        }
        log.println("...done");

        log.println("Trying to remove a user defined property");
        try {
            xPropContainer.removeProperty ("dateValue");
            log.println("\t...OK");
        } catch (Exception e) {
            log.println("\tException was thrown "+e);
            log.println("\t...FAILED");
            assure("Could not remove user defined property", false);
        }
        log.println("...done");

    }

    public void cleanup() {
        DesktopTools.closeDoc(xTextDoc);
    }

    private boolean checkType(XPropertySet xProps, String aName,
        String expected) {
        boolean ret = true;
        log.println("Checking " + expected);

        String getting =
            getPropertyByName(xProps, aName).getClass().getName();

        if (!getting.equals(expected)) {
            log.println("\t Expected: " + expected);
            log.println("\t Detting:  " + getting);
            ret = false;
        }

        if (ret) {
            log.println("...OK");
        }

        return ret;
    }

    private Object getPropertyByName(XPropertySet xProps, String aName) {
        Object ret = null;

        try {
            ret = xProps.getPropertyValue(aName);
        } catch (Exception e) {
            log.println("\tCouldn't get Property " + aName);
            log.println("\tMessage " + e);
        }

        return ret;
    }

    private boolean addProperty(XPropertyContainer xPropContainer,
        String aName, short attr, Object defaults) {
        boolean ret = true;

        try {
            xPropContainer.addProperty(aName, attr, defaults);
        } catch (Exception e) {
            ret = false;
            log.println("\tCouldn't get Property " + aName);
            log.println("\tMessage " + e);
        }

        return ret;
    }
}
