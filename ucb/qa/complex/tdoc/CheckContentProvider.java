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
import complexlib.ComplexTestCase;
import complex.tdoc.interfaces._XChild;
import complex.tdoc.interfaces._XCommandInfoChangeNotifier;
import complex.tdoc.interfaces._XComponent;
import complex.tdoc.interfaces._XServiceInfo;
import complex.tdoc.interfaces._XTypeProvider;
import complex.tdoc.interfaces._XCommandProcessor;
import complex.tdoc.interfaces._XContent;
import complex.tdoc.interfaces._XPropertiesChangeNotifier;
import complex.tdoc.interfaces._XPropertyContainer;
import complex.tdoc.interfaces._XPropertySetInfoChangeNotifier;
import lib.TestEnvironment;
import util.WriterTools;
import util.utils;

/**
 * Check the TransientDocumentsContentProvider (TDOC). Three documents are
 * loaded. Then every possible TDCP content type is instantiated and its
 * interfaces are tested.<br>
 * Important: opened documents are numbered in the order they are opened and
 * numbers are not reused. This test will work only, if you start a new office
 * with an accept parameter (writer is initially opened). Otherwise loaded
 * documents are not found.
 */
public class CheckContentProvider extends ComplexTestCase {
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
    public String[] getTestMethodNames() {
        return new String[]{"checkTDOCRoot",
                            "checkTDOCRootInterfaces",
                            "checkTDOCDocument",
                            "checkTDOCDocumentInterfaces",
                            "checkTDOCFolder",
                            "checkTDOCFolderInterfaces",
                            "checkTDOCStream",
                            "checkTDOCStreamInterfaces",
                            };
    }

    /**
     * Open some documents before the test
     */
    public void before() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        xTextDoc = new XTextDocument[countDocs];
        log.println("Open some new documents.");
        for (int i=0; i<countDocs; i++) {
            String fileName = utils.getFullTestURL(testDocuments[i]);
            System.out.println("Doc " + i + ": " + fileName);
            xTextDoc[i] = WriterTools.loadTextDoc(xMSF, fileName);
        }
    }

    /**
     * Close the documents
     */
    public void after() {
        log.println("Close all documents.");
        for (int i=0; i<countDocs; i++) {
            xTextDoc[i].dispose();
        }
    }

    /**
     * Check the tdcp root.
     */
    public void checkTDOCRoot() {
        try {
            // create a content provider
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            (XContentProvider)UnoRuntime.queryInterface(XContentProvider.class, o);

            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            (XContentIdentifierFactory)UnoRuntime.queryInterface(
                            XContentIdentifierFactory.class, xMSF.createInstance(
                            "com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            log.println("#### Content root: " + content);

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
                    cont = xCont[i].getContentType();
                log.println("Document Content " + i + ": " + cont + "  compare with root: " + returnVal);

                xContentId[i] = xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/" + i + "/content.xml");
                // get content
                xCont[i] = xContentProvider.queryContent(xContentId[i]);
                cont = null;
                if (xCont[i] != null)
                    cont = xCont[i].getContentType();
                log.println("\tContent.xml Content " + i + ": " + cont);
            }

            util.dbg.printInterfaces(xContent);
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            failed("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces of the root.
     */
    public void checkTDOCRootInterfaces() {
        checkInterfaces(false);
    }

    /**
     * Check the tdcp document: document 3 is used.
     */
    public void checkTDOCDocument() {
        try {
            xContent = null;
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            (XContentProvider)UnoRuntime.queryInterface(XContentProvider.class, o);
            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            (XContentIdentifierFactory)UnoRuntime.queryInterface(
                            XContentIdentifierFactory.class, xMSF.createInstance(
                            "com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/3");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            log.println("#### Document root: " + content);
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            failed("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces on the document.
     */
    public void checkTDOCDocumentInterfaces() {
        checkInterfaces(true);
    }

    /**
     * Check a folder on document 2 (document 2 contains an embedded picture and
     * therefore contans a subfolder "Pictures"
     */
    public void checkTDOCFolder() {
        try {
            xContent = null;
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            (XContentProvider)UnoRuntime.queryInterface(XContentProvider.class, o);
            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            (XContentIdentifierFactory)UnoRuntime.queryInterface(
                            XContentIdentifierFactory.class, xMSF.createInstance(
                            "com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/2/Pictures");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            log.println("#### Folder type: " + content);
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            failed("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces on the folder.
     */
    public void checkTDOCFolderInterfaces() {
        checkInterfaces(true);
    }

    /**
     * Open a stream to the embedded picture of document 1.
     */
    public void checkTDOCStream() {
        try {
            xContent = null;
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            (XContentProvider)UnoRuntime.queryInterface(XContentProvider.class, o);

            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            (XContentIdentifierFactory)UnoRuntime.queryInterface(
                            XContentIdentifierFactory.class, xMSF.createInstance(
                            "com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/1/Pictures/10000000000000640000004B9C743800.gif");
            // get content
            xContent = xContentProvider.queryContent(xContentIdentifier);

            String content = xContent.getContentType();
            log.println("#### Folder type: " + content);
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            failed("Unexpected Exception: " + e.getMessage());
        }
    }

    /**
     * Check the interfaces on the stream.
     */
    public void checkTDOCStreamInterfaces() {
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
        xTypeProvider.oObj = (XTypeProvider)UnoRuntime.queryInterface(XTypeProvider.class, xContent);
        xTypeProvider.log = log;
        assure("getImplementationId()", xTypeProvider._getImplementationId());
        assure("getTypes()", xTypeProvider._getTypes());

        // check the XSewrviceInfo interface
        _XServiceInfo xServiceInfo = new _XServiceInfo();
        xServiceInfo.oObj = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, xContent);
        xServiceInfo.log = log;
        assure("getImplementationName()", xServiceInfo._getImplementationName());
        assure("getSupportedServiceNames()", xServiceInfo._getSupportedServiceNames());
        assure("supportsService()", xServiceInfo._supportsService());

        // check the XCommandProcessor interface
        _XCommandProcessor xCommandProcessor = new _XCommandProcessor();
        xCommandProcessor.oObj = (XCommandProcessor)UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
        xCommandProcessor.log = log;
        xCommandProcessor.before((XMultiServiceFactory)param.getMSF());
        assure("createCommandIdentifier()", xCommandProcessor._createCommandIdentifier());
        assure("execute()", xCommandProcessor._execute());
        assure("abort()", xCommandProcessor._abort());

        // check the XChild interface
        _XChild xChild = new _XChild();
        xChild.oObj = (XChild)UnoRuntime.queryInterface(XChild.class, xContent);
        xChild.log = log;
        // hasParent dermines, if this content has a parent
        assure("getParent()", xChild._getParent(hasParent));
        // parameter does dermine, if this funczion is supported: generally not supported with tdcp content
        assure("setParent()", xChild._setParent(false));

        // check the XPropertyChangeNotifier interface
        _XPropertiesChangeNotifier xPropChange = new _XPropertiesChangeNotifier();
        xPropChange.oObj = (XPropertiesChangeNotifier)UnoRuntime.queryInterface(XPropertiesChangeNotifier.class, xContent);
        xPropChange.log = log;
        assure("addPropertiesChangeListener()", xPropChange._addPropertiesChangeListener());
        assure("removePropertiesChangeListener()", xPropChange._removePropertiesChangeListener());

        // check the XPropertySetInfoChangeNotifier interface
        _XPropertySetInfoChangeNotifier xPropSetInfo = new _XPropertySetInfoChangeNotifier();
        xPropSetInfo.oObj = (XPropertySetInfoChangeNotifier)UnoRuntime.queryInterface(XPropertySetInfoChangeNotifier.class, xContent);
        xPropSetInfo.log = log;
        assure("addPropertiesChangeListener()", xPropSetInfo._addPropertiesChangeListener());
        assure("removePropertiesChangeListener()", xPropSetInfo._removePropertiesChangeListener());

        // check the XCommandInfoChangeNotifier interface
        _XCommandInfoChangeNotifier xCommandChange = new _XCommandInfoChangeNotifier();
        xCommandChange.oObj = (XCommandInfoChangeNotifier)UnoRuntime.queryInterface(XCommandInfoChangeNotifier.class, xContent);
        xCommandChange.log = log;
        assure("addCommandInfoChangeListener()", xCommandChange._addCommandInfoChangeListener());
        assure("removeCommandInfoChangeListener()", xCommandChange._removeCommandInfoChangeListener());

        // check the XContent interface
        _XContent xCont = new _XContent();
        xCont.oObj = (XContent)UnoRuntime.queryInterface(XContent.class, xContent);
        xCont.log = log;
        assure("addContentEventListener()", xCont._addContentEventListener());
        assure("getContentType()", xCont._getContentType());
        assure("getIdentifier()", xCont._getIdentifier());
        assure("removeContentEventListener()", xCont._removeContentEventListener());

        // check the XPropertyContainer interface
        _XPropertyContainer xPropCont = new _XPropertyContainer();
        xPropCont.oObj = (XPropertyContainer)UnoRuntime.queryInterface(XPropertyContainer.class, xContent);
        xPropCont.log = log;
        assure("addProperty()", xPropCont._addProperty());
        assure("removeProperty()", xPropCont._removeProperty());

        // check the XComponent interface
        _XComponent xComponent = new _XComponent();
        xComponent.oObj = (XComponent)UnoRuntime.queryInterface(XComponent.class, xContent);
        xComponent.log = log;
        assure("addEventListener()", xComponent._addEventListener());
        assure("removeEventListener()", xComponent._removeEventListener());
//        assure("dispose()", xComponent._dispose());
    }
}
