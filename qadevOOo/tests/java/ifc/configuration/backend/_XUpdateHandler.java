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

import com.sun.star.configuration.backend.TemplateIdentifier;
import com.sun.star.configuration.backend.XUpdateHandler;
import com.sun.star.uno.Type;

import lib.MultiMethodTest;

import util.XLayerHandlerImpl;
import util.XLayerImpl;

public class _XUpdateHandler extends MultiMethodTest {
    public XUpdateHandler oObj;
    protected XLayerHandlerImpl xLayerHandler = null;

    public void before() {
        xLayerHandler = (XLayerHandlerImpl) tEnv.getObjRelation("LayerHandler");

        try {
            xLayerHandler.startLayer();
            xLayerHandler.overrideNode("org.openoffice.Setup", (short) 0,
                                       false);
            xLayerHandler.overrideNode("Office", (short) 0, false);
            xLayerHandler.overrideProperty("ooSetupInstCompleted", (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue(new Boolean(false));
            xLayerHandler.endProperty();
            xLayerHandler.overrideProperty("ooSetupShowIntro", (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue(new Boolean(true));
            xLayerHandler.endProperty();
            xLayerHandler.overrideProperty("ooSetupLocales", (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue("en-US");
            xLayerHandler.endProperty();
            xLayerHandler.overrideNode("Factories", (short) 0, false);
            xLayerHandler.addOrReplaceNode("com.sun.star.chart.ChartDocument",
                                           (short) 0);
            xLayerHandler.overrideProperty("ooSetupFactoryEmptyDocumentURL",
                                           (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue("private:factory/schart");
            xLayerHandler.endProperty();
            xLayerHandler.overrideProperty("ooSetupFactoryIcon", (short) 0,
                                           new Type(java.lang.Long.class),
                                           false);
            xLayerHandler.setPropertyValue(new Integer(13));
            xLayerHandler.endProperty();
            xLayerHandler.overrideProperty("ooSetupFactoryShortName",
                                           (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue("schart");
            xLayerHandler.setPropertyValueForLocale("TemplateFile", "en-US");
            xLayerHandler.endProperty();
            xLayerHandler.overrideProperty("ooSetupFactoryTemplateFile",
                                           (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue("empty");
            xLayerHandler.endProperty();
            xLayerHandler.addProperty("ooSetupFactoryTemplateFile", (short) 0,
                                      new Type(java.lang.String.class));
            xLayerHandler.addPropertyWithValue("ooSetupFactoryTemplateFile",
                                               (short) 0, "TemplateFile");
            xLayerHandler.endNode();

            xLayerHandler.addOrReplaceNode("dropme", (short) 0);
            xLayerHandler.overrideProperty("anyway", (short) 0,
                                           new Type(java.lang.String.class),
                                           false);
            xLayerHandler.setPropertyValue("nice");
            xLayerHandler.endProperty();
            xLayerHandler.dropNode("dropme");
            xLayerHandler.endNode();

            xLayerHandler.addOrReplaceNodeFromTemplate("FromTemplate",
                                                       new com.sun.star.configuration.backend.TemplateIdentifier(
                                                               "org.openoffice.Setup",
                                                               "Setup"),
                                                       (short) 0);
            xLayerHandler.endNode();

            xLayerHandler.endNode();
            xLayerHandler.endNode();
            xLayerHandler.endNode();
            xLayerHandler.endLayer();
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception " + e);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception " + e);
        }
    }

    public void _addOrReplaceNode() {
        requiredMethod("startUpdate()");
        tRes.tested("addOrReplaceNode()", true);
    }

    public void _addOrReplaceNodeFromTemplate() {
        requiredMethod("startUpdate()");
        tRes.tested("addOrReplaceNodeFromTemplate()", true);
    }

    public void _addOrReplaceProperty() {
        requiredMethod("startUpdate()");
        tRes.tested("addOrReplaceProperty()", true);
    }

    public void _addOrReplacePropertyWithValue() {
        requiredMethod("startUpdate()");
        tRes.tested("addOrReplacePropertyWithValue()", true);
    }

    public void _endNode() {
        requiredMethod("startUpdate()");
        tRes.tested("endNode()", true);
    }

    public void _endProperty() {
        requiredMethod("startUpdate()");
        tRes.tested("endProperty()", true);
    }

    public void _endUpdate() {
        requiredMethod("startUpdate()");
        tRes.tested("endUpdate()", true);
    }

    public void _modifyNode() {
        requiredMethod("startUpdate()");
        tRes.tested("modifyNode()", true);
    }

    public void _modifyProperty() {
        requiredMethod("startUpdate()");
        tRes.tested("modifyProperty()", true);
    }

    public void _removeNode() {
        requiredMethod("startUpdate()");
        tRes.tested("removeNode()", true);
    }

    public void _removeProperty() {
        requiredMethod("startUpdate()");
        tRes.tested("removeProperty()", true);
    }

    public void _resetProperty() {
        requiredMethod("startUpdate()");
        tRes.tested("resetProperty()", true);
    }

    public void _resetPropertyValue() {
        requiredMethod("startUpdate()");
        tRes.tested("resetPropertyValue()", true);
    }

    public void _resetPropertyValueForLocale() {
        requiredMethod("startUpdate()");
        tRes.tested("resetPropertyValueForLocale()", true);
    }

    public void _setPropertyValue() {
        requiredMethod("startUpdate()");
        tRes.tested("setPropertyValue()", true);
    }

    public void _setPropertyValueForLocale() {
        requiredMethod("startUpdate()");
        tRes.tested("setPropertyValueForLocale()", true);
    }

    public void _startUpdate() {
        boolean res = true;

        try {
            XLayerImpl xLayer = (XLayerImpl) tEnv.getObjRelation("Layer");
            log.println("Layer called (before): " + xLayer.hasBeenCalled());
            oObj.startUpdate();
            oObj.addOrReplaceNode("whatever", (short) 0);
            oObj.addOrReplaceNodeFromTemplate("Office", (short) 0,
                                              new TemplateIdentifier());
            oObj.addOrReplaceProperty("prop", (short) 0,
                                      new Type(java.lang.String.class));
            oObj.addOrReplacePropertyWithValue("prop2", (short) 0, "this");
            oObj.modifyProperty("ooSetupFactoryIcon", (short) 0, (short) 0,
                                new Type(java.lang.String.class));
            oObj.resetPropertyValue();
            oObj.resetPropertyValueForLocale("en-US");
            oObj.endProperty();
            oObj.modifyProperty("ooSetupFactoryEmptyDocumentURL", (short) 0,
                                (short) 0, new Type(java.lang.String.class));
            oObj.setPropertyValue("newValue");
            oObj.setPropertyValueForLocale("newValue-US", "de-DE");
            oObj.endProperty();
            oObj.removeProperty("ooSetupShowIntro");
            oObj.modifyNode("org.openoffice.Setup", (short) 0, (short) 0, true);
            oObj.removeNode("whatever");
            oObj.resetProperty("prop");
            oObj.endNode();
            oObj.endNode();
            oObj.endNode();
            oObj.endUpdate();

            log.println("Layer called (after): " + xLayer.hasBeenCalled());

            res = xLayer.hasBeenCalled();
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected exception " + e.getMessage());
            res = false;
        } catch (com.sun.star.lang.IllegalAccessException e) {
            log.println("Unexpected exception " + e.getMessage());
            res = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected exception " + e.getMessage());
            res = false;
        }

        tRes.tested("startUpdate()", res);
    }
}
