/*************************************************************************
 *
 *  $RCSfile: _XUpdateHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:21:56 $
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

import com.sun.star.configuration.backend.TemplateIdentifier;
import com.sun.star.configuration.backend.XLayerHandler;
import com.sun.star.configuration.backend.XUpdateHandler;
import com.sun.star.uno.Type;

import lib.MultiMethodTest;

import util.XLayerHandlerImpl;
import util.XLayerImpl;


/**
 *
 * @author  sw93809
 */
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
        } catch (com.sun.star.uno.Exception e) {
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