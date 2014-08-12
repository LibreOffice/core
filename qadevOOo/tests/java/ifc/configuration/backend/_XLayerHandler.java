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
package ifc.configuration.backend;

import com.sun.star.configuration.backend.XLayer;
import com.sun.star.configuration.backend.XLayerHandler;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
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
                                  new Type(String.class), false);
            oObj.setPropertyValue(Boolean.FALSE);
            oObj.endProperty();
            oObj.overrideNode("Factories", (short) 0, false);
            oObj.addOrReplaceNode("com.sun.star.chart.ChartDocument",
                                  (short) 0);
            oObj.overrideProperty("ooSetupFactoryEmptyDocumentURL", (short) 0,
                                  new Type(String.class), false);
            oObj.setPropertyValue("private:factory/schart");
            oObj.endProperty();
            oObj.overrideProperty("ooSetupFactoryIcon", (short) 0,
                                  new Type(java.lang.Long.class), false);
            oObj.setPropertyValue(Integer.valueOf(13));
            oObj.endProperty();
            oObj.overrideProperty("ooSetupFactoryShortName", (short) 0,
                                  new Type(String.class), false);
            oObj.setPropertyValue("schart");
            oObj.setPropertyValueForLocale("TemplateFile", "en-US");
            oObj.endProperty();
            oObj.overrideProperty("ooSetupFactoryTemplateFile", (short) 0,
                                  new Type(String.class), false);
            oObj.setPropertyValue("empty");
            oObj.endProperty();
            oObj.addProperty("ooSetupFactoryTemplateFile", (short) 0,
                             new Type(String.class));
            oObj.addPropertyWithValue("ooSetupFactoryTemplateFile", (short) 0,
                                      "TemplateFile");
            oObj.endNode();

            oObj.addOrReplaceNode("dropme", (short) 0);
            oObj.overrideProperty("anyway", (short) 0,
                                  new Type(String.class), false);
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

            Object LayerParser = tParam.getMSF().createInstance(
                                         "com.sun.star.comp.configuration.backend.xml.LayerParser");

            XActiveDataSink xSink = UnoRuntime.queryInterface(
                                            XActiveDataSink.class, LayerParser);
            Object fileacc = tParam.getMSF().createInstance(
                                     "com.sun.star.comp.ucb.SimpleFileAccess");
            XSimpleFileAccess simpleAccess = UnoRuntime.queryInterface(
                                                     XSimpleFileAccess.class,
                                                     fileacc);

            String filename = util.utils.getOfficeTemp(
                                      tParam.getMSF()) +
                              "LayerWriter.xcu";
            log.println("Going to parse: " + filename);

            XInputStream xStream = simpleAccess.openFileRead(filename);

            xSink.setInputStream(xStream);

            XLayer xLayer = UnoRuntime.queryInterface(XLayer.class,
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
