/*************************************************************************
 *
 *  $RCSfile: _XLayerHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:21:07 $
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
package ifc.configuration.backend;

import com.sun.star.configuration.backend.XLayer;
import com.sun.star.configuration.backend.XLayerHandler;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;

import util.XLayerHandlerImpl;


public class _XLayerHandler extends MultiMethodTest {
    public XLayerHandler oObj = null;

    /** this method contains the logic for all test methods
     * the others will be passed OK if this one is
     */
    public void _endLayer() {
        boolean res = false;

        try {
            oObj.startLayer();
            oObj.overrideNode("org.openoffice.Setup", (short) 0, false);
            oObj.overrideNode("Office", (short) 0, false);
            oObj.overrideProperty("ooSetupInstCompleted", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue(new Boolean(false));
            oObj.endProperty();
            oObj.overrideProperty("ooSetupShowIntro", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue(new Boolean(true));
            oObj.endProperty();
            oObj.overrideProperty("ooSetupLocales", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue("en-US");
            oObj.endProperty();
            oObj.overrideNode("Factories", (short) 0, false);
            oObj.addOrReplaceNode("com.sun.star.chart.ChartDocument",
                                  (short) 0);
            oObj.overrideProperty("ooSetupFactoryEmptyDocumentURL", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue("private:factory/schart");
            oObj.endProperty();
            oObj.overrideProperty("ooSetupFactoryIcon", (short) 0,
                                  new Type(java.lang.Long.class), false);
            oObj.setPropertyValue(new Integer(13));
            oObj.endProperty();
            oObj.overrideProperty("ooSetupFactoryShortName", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue("schart");
            oObj.setPropertyValueForLocale("TemplateFile", "en-US");
            oObj.endProperty();
            oObj.overrideProperty("ooSetupFactoryTemplateFile", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue("empty");
            oObj.endProperty();
            oObj.addProperty("ooSetupFactoryTemplateFile", (short) 0,
                             new Type(java.lang.String.class));
            oObj.addPropertyWithValue("ooSetupFactoryTemplateFile", (short) 0,
                                      "TemplateFile");
            oObj.endNode();

            oObj.addOrReplaceNode("dropme", (short) 0);
            oObj.overrideProperty("anyway", (short) 0,
                                  new Type(java.lang.String.class), false);
            oObj.setPropertyValue("nice");
            oObj.endProperty();
            oObj.dropNode("dropme");
            oObj.endNode();

            oObj.addOrReplaceNodeFromTemplate("FromTemplate",
                                              new com.sun.star.configuration.backend.TemplateIdentifier(
                                                      "org.openoffice.Setup",
                                                      "Setup"), (short) 0);
            oObj.endNode();

            oObj.endNode();
            oObj.endNode();
            oObj.endNode();
            oObj.endLayer();

            Object LayerParser = ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                                         "com.sun.star.comp.configuration.backend.xml.LayerParser");

            XActiveDataSink xSink = (XActiveDataSink) UnoRuntime.queryInterface(
                                            XActiveDataSink.class, LayerParser);
            Object fileacc = ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                                     "com.sun.star.comp.ucb.SimpleFileAccess");
            XSimpleFileAccess simpleAccess = (XSimpleFileAccess) UnoRuntime.queryInterface(
                                                     XSimpleFileAccess.class,
                                                     fileacc);

            String filename = util.utils.getOfficeTemp(
                                      (XMultiServiceFactory) tParam.getMSF()) +
                              "LayerWriter.xcu";
            log.println("Going to parse: " + filename);

            XInputStream xStream = simpleAccess.openFileRead(filename);

            xSink.setInputStream(xStream);

            XLayer xLayer = (XLayer) UnoRuntime.queryInterface(XLayer.class,
                                                               LayerParser);

            XLayerHandlerImpl xLayerHandler = new XLayerHandlerImpl();

            xLayer.readData(xLayerHandler);

            res = xLayerHandler.getCalls().equals(getExpected());

            if (!res) {
                log.println("#### Getting: ");
                log.println(xLayerHandler.getCalls());
                log.println("#### Expected: ");
                log.println(getExpected());
            }
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception " + e);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception " + e);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Unexpected Exception " + e);
        }

        tRes.tested("endLayer()", res);
    }

    public void _addOrReplaceNode() {
        requiredMethod("endLayer()");
        tRes.tested("addOrReplaceNode()", true);
    }

    public void _addOrReplaceNodeFromTemplate() {
        requiredMethod("endLayer()");
        tRes.tested("addOrReplaceNodeFromTemplate()", true);
    }

    public void _addProperty() {
        requiredMethod("endLayer()");
        tRes.tested("addProperty()", true);
    }

    public void _addPropertyWithValue() {
        requiredMethod("endLayer()");
        tRes.tested("addPropertyWithValue()", true);
    }

    public void _dropNode() {
        requiredMethod("endLayer()");
        tRes.tested("dropNode()", true);
    }

    public void _endNode() {
        requiredMethod("endLayer()");
        tRes.tested("endNode()", true);
    }

    public void _endProperty() {
        requiredMethod("endLayer()");
        tRes.tested("endProperty()", true);
    }

    public void _overrideNode() {
        requiredMethod("endLayer()");
        tRes.tested("overrideNode()", true);
    }

    public void _overrideProperty() {
        requiredMethod("endLayer()");
        tRes.tested("overrideProperty()", true);
    }

    public void _setPropertyValue() {
        requiredMethod("endLayer()");
        tRes.tested("setPropertyValue()", true);
    }

    public void _setPropertyValueForLocale() {
        requiredMethod("endLayer()");
        tRes.tested("setPropertyValueForLocale()", true);
    }

    public void _startLayer() {
        requiredMethod("endLayer()");
        tRes.tested("startLayer()", true);
    }

    protected String getExpected() {
        String ls = System.getProperty("line.separator");
        String expected = "startLayer();" + ls;
        expected += ("overrideNode(org.openoffice.Setup,0,false);" + ls);
        expected += ("overrideNode(Office,0,false);" + ls);
        expected += ("overrideProperty(ooSetupInstCompleted,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(false);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("overrideProperty(ooSetupShowIntro,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(true);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("overrideProperty(ooSetupLocales,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(en-US);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("overrideNode(Factories,0,false);" + ls);
        expected += ("addOrReplace(com.sun.star.chart.ChartDocument,0);" + ls);
        expected += ("overrideProperty(ooSetupFactoryEmptyDocumentURL,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(private:factory/schart);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("overrideProperty(ooSetupFactoryIcon,0,Type[hyper],false);" + ls);
        expected += ("setPropertyValue(13);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("overrideProperty(ooSetupFactoryShortName,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(schart);" + ls);
        expected += ("setPropertyValueForLocale(TemplateFile,en-US);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("overrideProperty(ooSetupFactoryTemplateFile,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(empty);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("addProperty(ooSetupFactoryTemplateFile,0,Type[string]);" + ls);
        expected += ("addPropertyWithValue(ooSetupFactoryTemplateFile,0,TemplateFile);" + ls);
        expected += ("endNode();" + ls);
        expected += ("addOrReplace(dropme,0);" + ls);
        expected += ("overrideProperty(anyway,0,Type[string],false);" + ls);
        expected += ("setPropertyValue(nice);" + ls);
        expected += ("endProperty();" + ls);
        expected += ("dropNode(dropme);" + ls);
        expected += ("endNode();" + ls);
        expected += ("addOrReplace(FromTemplate,0);" + ls);
        expected += ("endNode();" + ls);
        expected += ("endNode();" + ls);
        expected += ("endNode();" + ls);
        expected += ("endNode();" + ls);
        expected += ("endLayer();" + ls);

        return expected;
    }
}