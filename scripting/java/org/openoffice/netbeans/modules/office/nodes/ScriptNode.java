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

package org.openoffice.netbeans.modules.office.nodes;

import org.w3c.dom.*;

import org.openide.actions.*;
import org.openide.nodes.*;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

/** A simple node with no children.
 */
public class ScriptNode extends AbstractNode {
    private Element element;
    private static final String LOGICAL_NAME = "logicalname";
    private static final String LANGUAGE_NAME = "languagename";

    public ScriptNode(Element element) {
        super(Children.LEAF);
        this.element = element;
        init();
    }

    private void init() {
        setIconBase("/org/openoffice/netbeans/modules/office/resources/OfficeIcon");
        setDefaultAction(SystemAction.get(PropertiesAction.class));

        NodeList nl = element.getElementsByTagName(LOGICAL_NAME);
        Element nameElement = (Element)nl.item(0);

        String name = nameElement.getAttribute("value");
        setName(name);
        setDisplayName(name.substring(name.lastIndexOf(".") + 1));
        setShortDescription(name);
    }

    protected SystemAction[] createActions() {
        return new SystemAction[] {
            SystemAction.get(ToolsAction.class),
            null,
            SystemAction.get(PropertiesAction.class),
        };
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    // RECOMMENDED - handle cloning specially (so as not to invoke the overhead of FilterNode):
    /*
    public Node cloneNode() {
        // Try to pass in similar constructor params to what you originally got:
        return new ScriptNode();
    }
     */

    protected Sheet createSheet() {
        Sheet sheet = super.createSheet();
        Sheet.Set props = sheet.get(Sheet.PROPERTIES);
        if (props == null) {
            props = Sheet.createPropertiesSet();
            sheet.put(props);
        }

        org.openide.nodes.Node.Property prop = null;
        if ((prop = getStringProperty(LOGICAL_NAME)) != null)
            props.put(prop);

        if ((prop = getStringProperty(LANGUAGE_NAME)) != null)
            props.put(prop);

        return sheet;
    }

    private org.openide.nodes.Node.Property getStringProperty(String name) {
        NodeList nl = element.getElementsByTagName(name);
        if(nl.getLength() != 1)
            return null;

        Element nameElement = (Element)nl.item(0);
        String value = nameElement.getAttribute("value");

        return new StringProperty(this, name, value);
    }

    private class StringProperty extends PropertySupport.ReadOnly {
        private String name, value;
        private ScriptNode sn;

        public StringProperty(ScriptNode sn, String name, String value) {
            super(value, String.class, name,
                "The name of the java language method for this script");
            this.sn = sn;
            this.name = name;
            this.value = value;
        }

        public Object getValue() {
            return value;
        }

        /* public void setValue(Object obj) {
            System.out.println("Setting value to: " + obj.toString());

            if ((value instanceof String) != true)
                throw new IllegalArgumentException(name +
                    " property must be of type String");

            value = obj.toString();
            if (name.equals(LOGICAL_NAME)) {
                sn.setName(value);
                sn.setDisplayName(value.substring(value.lastIndexOf(".") + 1));
                sn.setShortDescription(value);
            }
        } */
    }

    /* public boolean canRename() {
        return true;
    }

    public void setName(String nue) {
        // Update visible name, fire property changes:
        super.setName(nue);
    } */

    /*
     public boolean canDestroy() {
        return true;
    }
    public void destroy() throws IOException {
        // Actually remove the node itself and fire property changes:
        super.destroy();
        // perform additional actions, i.e. delete underlying object
    } */

    // Handle copying and cutting specially:
    /*
    public boolean canCopy() {
        return true;
    }
    public boolean canCut() {
        return true;
    }
    public Transferable clipboardCopy() {
        // Add to, do not replace, the default node copy flavor:
        ExTransferable et = ExTransferable.create(super.clipboardCopy());
        et.put(new ExTransferable.Single(DataFlavor.stringFlavor) {
                protected Object getData() {
                    return ScriptNode.this.getDisplayName();
                }
            });
        return et;
    }
    public Transferable clipboardCut() {
        // Add to, do not replace, the default node cut flavor:
        ExTransferable et = ExTransferable.create(super.clipboardCut());
        // This is not so useful because this node will not be destroyed afterwards
        // (it is up to the paste type to decide whether to remove the "original",
        // and it is not safe to assume that getData will only be called once):
        et.put(new ExTransferable.Single(DataFlavor.stringFlavor) {
                protected Object getData() {
                    return ScriptNode.this.getDisplayName();
                }
            });
        return et;
    }
     */

    // Permit user to customize whole node at once (instead of per-property):
    /*
    public boolean hasCustomizer() {
        return true;
    }
    public Component getCustomizer() {
        return new MyCustomizingPanel(this);
    }
     */

}
