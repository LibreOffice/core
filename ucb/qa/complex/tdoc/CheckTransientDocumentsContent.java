/*************************************************************************
 *
 *  $RCSfile: CheckTransientDocumentsContent.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-09 13:47:38 $
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
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentAccess;
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
public class CheckTransientDocumentsContent extends ComplexTestCase{
    private final String testDocuments[] = new String[]{"sForm.sxw"};//, "chinese.sxw", "Iterator.sxw"};
    private final int countDocs = testDocuments.length;
    private XMultiServiceFactory xMSF = null;
    private XTextDocument[] xTextDoc = null;

    public String[] getTestMethodNames() {
        return new String[] {"checkTransientDocumentsContent"};
    }

    public void before() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        xTextDoc = new XTextDocument[countDocs];
        log.println("Open some documents.");
        for (int i=0; i<countDocs; i++) {
            String fileName = utils.getFullTestURL(testDocuments[i]);
            xTextDoc[i] = WriterTools.loadTextDoc(xMSF, fileName);
        }
    }
    public void after() {
        log.println("Close all documents.");
        for (int i=0; i<countDocs; i++) {
            xTextDoc[i].dispose();
        }
    }

    /**
     * Check the content of one document
     */
    public void checkTransientDocumentsContent() {
        try {
            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            (XContentIdentifierFactory)UnoRuntime.queryInterface(
                            XContentIdentifierFactory.class, xMSF.createInstance(
                            "com.sun.star.ucb.UniversalContentBroker"));
            XContentProvider xContentProvider =
                            (XContentProvider)UnoRuntime.queryInterface(XContentProvider.class, xContentIdentifierFactory);
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                               xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/1");
            // get content
            XContent xContent = xContentProvider.queryContent(xContentIdentifier);

            // actual test: commands to get some properties
            XCommandProcessor xCommandProcessor = (XCommandProcessor)UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
            // build up the command
            Command command = new Command();
            command.Name = "getPropertySetInfo";
            command.Handle = -1;

            // execute the command
            Object result = xCommandProcessor.execute(command, 0, null);

            // check the result
            log.println("Result: "+ result.getClass().toString());
            XPropertySetInfo xPropertySetInfo = (XPropertySetInfo)UnoRuntime.queryInterface(XPropertySetInfo.class, result);
            Property[] props = xPropertySetInfo.getProperties();
            boolean res = false;
            for(int i=0; i<props.length; i++) {
                String propName = props[i].Name;
                res |= propName.equals("DocumentModel");
                log.println("Found property: " + propName + "   type: " + props[i].Type.getTypeName());
            }
            assure("Did not find property 'DocumentModel' in the Property array.", res);

            // get on property
            command.Name = "getPropertyValues";
            command.Handle = -1;
            Property[] prop = new Property[1];
            prop[0] = new Property();
            prop[0].Name = "DocumentModel";
            prop[0].Handle = -1;
            command.Argument = prop;

            // execute the command
            result = xCommandProcessor.execute(command, 0, null);

            // check the result
            log.println("Result: "+ result.getClass().toString());

            XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, result);
            assure("Did not get property 'DocumentModel'.", xModel == null);
        }
        catch (com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            failed("Could not create test objects.");
        }

    }

}
