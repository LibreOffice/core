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
package complex.tdoc;

import com.sun.star.beans.XPropertiesChangeNotifier;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySetInfoChangeNotifier;
import com.sun.star.container.XChild;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.XCommandInfoChangeNotifier;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.uno.UnoRuntime;
import util.WriterTools;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * Check the TransientDocumentsContentProvider (TDOC). Three documents are
 * loaded. Then every possible TDCP content type is instantiated and its
 * interfaces are tested.<br>
 * Important: opened documents are numbered in the order they are opened and
 * numbers are not reused. This test will work only, if you start a new office
 * with an accept parameter (writer is initially opened). Otherwise loaded
 * documents are not found.
 */
public class CheckContentProvider {
    private final String testDocuments[] = new String[]{"filter.sxw", "chinese.sxw", "Iterator.sxw"};
    private final int countDocs = testDocuments.length;
    private XMultiServiceFactory xMSF = null;
    private XTextDocument[] xTextDoc = null;
    private XContent xContent = null;

    /**
     * The test methods: the test methods have to be executed in a specified
     * order. This order is:
     * <ol>
     * <li>"checkTDOCRoot"</li>
     * <li>"checkTDOCRootInterfaces"</li>
     * <li>"checkTDOCDocument"</li>
     * <li>"checkTDOCDocumentInterfaces"</li>
     * <li>"checkTDOCFolder"</li>
     * <li>"checkTDOCFolderInterfaces"</li>
     * <li>"checkTDOCStream"</li>
     * <li>"checkTDOCStreamInterfaces"</li>
     * </ol>
     * Important is, that the test of the element comes first, then the test of
     * its interfaces.
     **/
//    public String[] getTestMethodNames() {
//        return new String[]{"checkTDOCRoot",
//                            "checkTDOCRootInterfaces",
//                            "checkTDOCDocument",
//                            "checkTDOCDocumentInterfaces",
//                            "checkTDOCFolder",
//                            "checkTDOCFolderInterfaces",
//                            "checkTDOCStream",
//                            "checkTDOCStreamInterfaces",
//                            };
//    }

    /**
     * Open some documents before the test
     */
    @Before public void before() {
        xMSF = getMSF();
        xTextDoc = new XTextDocument[countDocs];
        System.out.println("Open some new documents.");
        for (int i=0; i<countDocs; i++) {
            String fileName = TestDocument.getUrl(testDocuments[i]);
            System.out.println("Doc " + i + ": " + fileName);
            xTextDoc[i] = WriterTools.loadTextDoc(xMSF, fileName);
            assertNotNull("Can't load document " + fileName, xTextDoc[i]);
        }
    }

    /**
     * Close the documents
     */
    @After public void after() {
        System.out.println("Close all documents.");
        for (int i=0; i<countDocs; i++) {
            xTextDoc[i].dispose();
        }
    }

    /**
     * Check the tdcp root.
     */
    @Test public void checkTDOCRoot() {
        try {
            // create a content provider
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            UnoRuntime.queryInterface(XContentProvider.class, o);

            // create unconfigured ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            UnoRuntime.queryInterface(XContentIdentifierFactory.class, xMSF.createInstance("com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            System.out.println("#### Content root: " + content);

            // try to get some documents: should be "countDocs" at least.
            XContentIdentifier[] xContentId = new XContentIdentifier[countDocs+5];
            XContent[] xCont = new XContent[countDocs+5];

            for (int i=0; i<countDocs+5; i++) {
                xContentId[i] = xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/" + i);
                // get content
                xCont[i] = xContentProvider.queryContent(xContentId[i]);
                int returnVal = xContentProvider.compareContentIds(xContentId[i], xContentIdentifier);
                String cont = null;
                if (xCont[i] != null)
                {
                    cont = xCont[i].getContentType();
                }
                System.out.println("Document Content " + i + ": " + cont + "  compare with root: " + returnVal);

                xContentId[i] = xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/" + i + "/content.xml");
                // get content
                xCont[i] = xContentProvider.queryContent(xContentId[i]);
                cont = null;
                if (xCont[i] != null)
                {
                    cont = xCont[i].getContentType();
                }
                System.out.println("\tContent.xml Content " + i + ": " + cont);
            }

            util.dbg.printInterfaces(xContent);
        }
        catch(Exception e) {
            e.printStackTrace();
            fail("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces of the root.
     */
    @Test public void checkTDOCRootInterfaces() {
        checkInterfaces(false);
    }

    /**
     * Check the tdcp document: document 3 is used.
     */
    @Test public void checkTDOCDocument() {
        try {
            xContent = null;
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            UnoRuntime.queryInterface(XContentProvider.class, o);
            // create unconfigured ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            UnoRuntime.queryInterface(XContentIdentifierFactory.class, xMSF.createInstance("com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/3");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);
            // assertNotNull(xContent);
            String content = xContent.getContentType();
            System.out.println("#### Document root: " + content);
        }
        catch(Exception e) {
            e.printStackTrace();
            fail("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces on the document.
     */
    @Test public void checkTDOCDocumentInterfaces() {
        checkInterfaces(true);
    }

    /**
     * Check a folder on document 2 (document 2 contains an embedded picture and
     * therefore contans a subfolder "Pictures"
     */
    @Test public void checkTDOCFolder() {
        try {
            xContent = null;
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            UnoRuntime.queryInterface(XContentProvider.class, o);
            // create unconfigured ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            UnoRuntime.queryInterface(XContentIdentifierFactory.class, xMSF.createInstance("com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/2/Pictures");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            System.out.println("#### Folder type: " + content);
        }
        catch(Exception e) {
            e.printStackTrace();
            fail("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces on the folder.
     */
    @Test public void checkTDOCFolderInterfaces() {
        checkInterfaces(true);
    }

    /**
     * Open a stream to the embedded picture of document 1.
     */
    @Test public void checkTDOCStream() {
        try {
            xContent = null;
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            UnoRuntime.queryInterface(XContentProvider.class, o);

            // create unconfigured ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            UnoRuntime.queryInterface(XContentIdentifierFactory.class, xMSF.createInstance("com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/1/Pictures/10000000000000640000004B9C743800.gif");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            System.out.println("#### Folder type: " + content);
        }
        catch(Exception e) {
            e.printStackTrace();
            fail("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces on the stream.
     */
    @Test public void checkTDOCStreamInterfaces() {
        checkInterfaces(true);
    }

    /**
     * Since all tdcp content types implement (nearly) the same interfaces, they
     * are called here.
     * Executed interface tests are (in this order):
     * <ol>
     * <li>XTypeProvider</li>
     * <li>XServiceInfo</li>
     * <li>XCommandProcessor</li>
     * <li>XChild</li>
     * <li>XPropertiesChangeNotifier</li>
     * <li>XPropertySetInfoChangeNotifier</li>
     * <li>XCommandInfoChangeNotifier</li>
     * <li>XContent</li>
     * <li>XPropertyContainer</li>
     * <li>XComponent</li>
     * </ol>
     * @param hasParent True, if the tested content type does have a parent:
     * only the root has not. Used in the XChild interface test.
     */
    private void checkInterfaces(boolean hasParent) {
        // check the XTypeProvider interface
        _XTypeProvider xTypeProvider = new _XTypeProvider();
        xTypeProvider.oObj = UnoRuntime.queryInterface(XTypeProvider.class, xContent);
        // xTypeProvider.log = log;
        assertNotNull("getImplementationId()", xTypeProvider._getImplementationId());
        assertNotNull("getTypes()", xTypeProvider._getTypes());

        // check the XSewrviceInfo interface
        _XServiceInfo xServiceInfo = new _XServiceInfo();
        xServiceInfo.oObj = UnoRuntime.queryInterface(XServiceInfo.class, xContent);
        // xServiceInfo.log = log;
        assertNotNull("getImplementationName()", xServiceInfo._getImplementationName());
        assertNotNull("getSupportedServiceNames()", xServiceInfo._getSupportedServiceNames());
        assertNotNull("supportsService()", xServiceInfo._supportsService());

        // check the XCommandProcessor interface
        _XCommandProcessor xCommandProcessor = new _XCommandProcessor();
        xCommandProcessor.oObj = UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
        // xCommandProcessor.log = log;
        xCommandProcessor.before(getMSF());
        assertNotNull("createCommandIdentifier()", xCommandProcessor._createCommandIdentifier());
        assertNotNull("execute()", xCommandProcessor._execute());
        assertNotNull("abort()", xCommandProcessor._abort());

        // check the XChild interface
        _XChild xChild = new _XChild();
        xChild.oObj = UnoRuntime.queryInterface(XChild.class, xContent);
        // xChild.log = log;
        // hasParent dermines, if this content has a parent
        assertNotNull("getParent()", xChild._getParent(hasParent));
        // parameter does dermine, if this funczion is supported: generally not supported with tdcp content
        assertNotNull("setParent()", xChild._setParent(false));

        // check the XPropertyChangeNotifier interface
        _XPropertiesChangeNotifier xPropChange = new _XPropertiesChangeNotifier();
        xPropChange.oObj = UnoRuntime.queryInterface(XPropertiesChangeNotifier.class, xContent);
        // xPropChange.log = log;
        assertNotNull("addPropertiesChangeListener()", xPropChange._addPropertiesChangeListener());
    assertNotNull("removePropertiesChangeListener()", xPropChange._removePropertiesChangeListener());

        // check the XPropertySetInfoChangeNotifier interface
        _XPropertySetInfoChangeNotifier xPropSetInfo = new _XPropertySetInfoChangeNotifier();
        xPropSetInfo.oObj = UnoRuntime.queryInterface(XPropertySetInfoChangeNotifier.class, xContent);
        // xPropSetInfo.log = log;
        assertNotNull("addPropertiesChangeListener()", xPropSetInfo._addPropertiesChangeListener());
    assertNotNull("removePropertiesChangeListener()", xPropSetInfo._removePropertiesChangeListener());

        // check the XCommandInfoChangeNotifier interface
        _XCommandInfoChangeNotifier xCommandChange = new _XCommandInfoChangeNotifier();
        xCommandChange.oObj = UnoRuntime.queryInterface(XCommandInfoChangeNotifier.class, xContent);
        // xCommandChange.log = log;
        assertNotNull("addCommandInfoChangeListener()", xCommandChange._addCommandInfoChangeListener());
        assertNotNull("removeCommandInfoChangeListener()", xCommandChange._removeCommandInfoChangeListener());

        // check the XContent interface
        _XContent xCont = new _XContent();
        xCont.oObj = UnoRuntime.queryInterface(XContent.class, xContent);
        // xCont.log = log;
        assertNotNull("addContentEventListener()", xCont._addContentEventListener());
        assertNotNull("getContentType()", xCont._getContentType());
        assertNotNull("getIdentifier()", xCont._getIdentifier());
        assertNotNull("removeContentEventListener()", xCont._removeContentEventListener());

        // check the XPropertyContainer interface
        _XPropertyContainer xPropCont = new _XPropertyContainer();
        xPropCont.oObj = UnoRuntime.queryInterface(XPropertyContainer.class, xContent);
        // xPropCont.log = log;
        assertNotNull("addProperty()", xPropCont._addProperty());
        assertNotNull("removeProperty()", xPropCont._removeProperty());

        // check the XComponent interface
        _XComponent xComponent = new _XComponent();
        xComponent.oObj = UnoRuntime.queryInterface(XComponent.class, xContent);
        // xComponent.log = log;
        assertNotNull("addEventListener()", xComponent._addEventListener());
        assertNotNull("removeEventListener()", xComponent._removeEventListener());
//        assure("dispose()", xComponent._dispose());
    }




    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
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
