/*************************************************************************
 *
 *  $RCSfile: CheckTransientDocumentsDocumentContent.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-09 13:48:01 $
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
package complex.tdoc;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XDocumentSubStorageSupplier;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XStorage;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTransientDocumentsDocumentContentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.ContentInfo;
import com.sun.star.ucb.InsertCommandArgument;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentAccess;
import com.sun.star.ucb.XContentCreator;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import util.WriterTools;
import util.utils;


/**
 *
 */
public class CheckTransientDocumentsDocumentContent extends ComplexTestCase {
    private final String testDocuments = "sForm.sxw";
    private final String folderName = "TestFolder";
    private XMultiServiceFactory xMSF = null;
    private XTextDocument xTextDoc = null;

    public String[] getTestMethodNames() {
        return new String[]{"checkTransientDocumentsDocumentContent"};
    }

    public void before() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        log.println("Open a document.");
        String fileName = utils.getFullTestURL(testDocuments);
        xTextDoc = WriterTools.loadTextDoc(xMSF, fileName);
    }
    public void after() {
        log.println("Close all documents.");
        xTextDoc.dispose();
    }

    /**
     * Check the provider of document content: open some documents
     * and look if they are accessible.
     */
    public void checkTransientDocumentsDocumentContent() {
        try {
            // create a content provider
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsDocumentContentFactory");

            XTransientDocumentsDocumentContentFactory xTransientDocumentsDocumentContentFactory =
                            (XTransientDocumentsDocumentContentFactory)UnoRuntime.queryInterface(
                            XTransientDocumentsDocumentContentFactory.class, o);
            // get the model from the opened document
            XModel xModel = xTextDoc.getCurrentController().getModel();

            // a little additional check for 114733
            XDocumentSubStorageSupplier xDocumentSubStorageSupplier = (XDocumentSubStorageSupplier)
                            UnoRuntime.queryInterface(XDocumentSubStorageSupplier.class, xModel);
            String[]names = xDocumentSubStorageSupplier.getDocumentSubStoragesNames();
            for (int i=0; i<names.length; i++) {
                log.println("SubStorage names " + i + ": " +names[i]);
            }
            XStorage xStorage = xDocumentSubStorageSupplier.getDocumentSubStorage(names[0], ElementModes.READWRITE);
            assure("Could not get a storage from the XDocumentStorageSupplier.", xStorage != null);
            // get content
            XContent xContent = xTransientDocumentsDocumentContentFactory.createDocumentContent(xModel);
            // actual test: execute some commands
            XCommandProcessor xCommandProcessor = (XCommandProcessor)UnoRuntime.queryInterface(XCommandProcessor.class, xContent);

            // create the command and arguments
            Command command = new Command();
            OpenCommandArgument2 cmargs2 = new OpenCommandArgument2();
            Property[]props = new Property[1];
            props[0] = new Property();
            props[0].Name = "Title";
            props[0].Handle = -1;
            cmargs2.Mode = OpenMode.ALL;
            cmargs2.Properties = props;

            command.Name = "open";
            command.Argument = cmargs2;

            Object result = xCommandProcessor.execute(command, 0, null);
            XDynamicResultSet xDynamicResultSet = (XDynamicResultSet)UnoRuntime.queryInterface(XDynamicResultSet.class, result);
            XResultSet xResultSet = xDynamicResultSet.getStaticResultSet();
            XRow xRow = (XRow)UnoRuntime.queryInterface(XRow.class, xResultSet);
            // create the new folder 'folderName': first, check if it's already there
            while(xResultSet.next()) {
                String existingFolderName = xRow.getString(1);
                log.println("Found existing folder: '" + existingFolderName + "'");
                if (folderName.equals(existingFolderName)) {
                    failed("Cannot create a new folder: folder already exists: adapt test or choose a different document.");
                }
            }
            // create a folder
            XContent xNewFolder = null;
            log.println("Create new folder "+ folderName);
            ContentInfo contentInfo = new ContentInfo();
            contentInfo.Type = "application/vnd.sun.star.tdoc-folder";
            XContentCreator xContentCreator = (XContentCreator)UnoRuntime.queryInterface(XContentCreator.class, xContent);
            xNewFolder = xContentCreator.createNewContent(contentInfo);
            XCommandProcessor xFolderCommandProcessor = (XCommandProcessor)UnoRuntime.queryInterface(XCommandProcessor.class, xNewFolder);
            log.println("Got the new folder: " + utils.getImplName(xNewFolder));

            // name the new folder
            PropertyValue[] titleProp = new PropertyValue[1];
            titleProp[0] = new PropertyValue();
            titleProp[0].Name = "Title";
            titleProp[0].Handle = -1;
            titleProp[0].Value = folderName;
            Command titleSetCommand = new Command();
            titleSetCommand.Name = "setPropertyValues";
            titleSetCommand.Argument = titleProp;
            xFolderCommandProcessor.execute(titleSetCommand, 0, null);

            // 2do: check all this stuff!
            // commit changes
/*            InsertCommandArgument insertArgs = new InsertCommandArgument();
            insertArgs.Data = null;
            insertArgs.ReplaceExisting = true;
            Command commitCommand = new Command();
            commitCommand.Name = "insert";
            commitCommand.Argument = insertArgs;
            xFolderCommandProcessor.execute(commitCommand, 0, null); */
        }
        catch (com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            failed("Could not create test objects.");
        }

    }

}
