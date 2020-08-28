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

package com.sun.star.script.framework.provider;

import com.sun.star.beans.NamedValue;
import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.provider.XScriptContext;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

import javax.swing.JOptionPane;
import javax.swing.JDialog;

public abstract class ScriptEditor {
    public XScriptContext context;

    public abstract Object execute() throws Exception;
    public abstract void indicateErrorLine(int lineNum);
    public abstract void edit(XScriptContext context, ScriptMetaData entry);
    public abstract String getTemplate();
    public abstract String getExtension();

    public void setContext(XScriptContext context) {
        this.context = context;
    }

    public boolean isMacroExecutionEnabled() {
        XNameAccess xNameAccess = null;
        try {
            String sAccess = "com.sun.star.configuration.ConfigurationAccess";
            XMultiServiceFactory xMSFCfg = theDefaultProvider.get(context.getComponentContext());
            Object oAccess = xMSFCfg.createInstanceWithArguments(sAccess,
                    new Object[] { new NamedValue("nodepath", "org.openoffice.Office.Common/Security/Scripting") });
            xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, oAccess);
            Object result = xNameAccess.getByName("DisableMacrosExecution");
            boolean bMacrosDisabled = AnyConverter.toBoolean(result);
            if (bMacrosDisabled)
                return false;
        } catch (com.sun.star.uno.Exception e) {
            return false;
        }
        return true;
    }

    // Wraps long error messages
    private static class NarrowOptionPane extends JOptionPane {
        private static final long serialVersionUID = 1L;
        public int getMaxCharactersPerLineCount() {
            return 100;
        }
    }

    public void showErrorMessage(String message) {
        JOptionPane optionPane = new NarrowOptionPane();
        optionPane.setMessage(message);
        optionPane.setMessageType(JOptionPane.ERROR_MESSAGE);
        JDialog dialog = optionPane.createDialog(null, "Error");
        dialog.setVisible(true);
    }
}