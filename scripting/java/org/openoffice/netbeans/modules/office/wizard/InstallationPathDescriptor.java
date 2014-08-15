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

package org.openoffice.netbeans.modules.office.wizard;

import java.awt.Dimension;
import java.awt.Toolkit;
import java.net.URL;
import java.net.MalformedURLException;

import org.openide.TopManager;
import org.openide.WizardDescriptor;
import org.openide.util.NbBundle;

/** A wizard descriptor.
 */
public class InstallationPathDescriptor extends WizardDescriptor {

    private final InstallationPathIterator iterator;
    public static final String PROP_INSTALLPATH = "INSTALLPATH";

    /** Make a descriptor suited to use InstallationPathIterator.
     * Sets up various wizard properties to follow recommended
     * style guidelines.
     */
    public InstallationPathDescriptor() {
        this(new InstallationPathIterator());
    }
    private InstallationPathDescriptor(InstallationPathIterator iterator) {
        super(iterator);
        this.iterator = iterator;
        // Set title for the dialog:
        setTitle(NbBundle.getMessage(InstallationPathDescriptor.class, "TITLE_wizard"));
        // Make the left pane appear:
        putProperty("WizardPanel_autoWizardStyle", Boolean.TRUE); // NOI18N
        // Make the left pane show list of steps etc.:
        putProperty("WizardPanel_contentDisplayed", Boolean.TRUE); // NOI18N
    }

    // Called when user moves forward or backward etc.:
    protected void updateState() {
        super.updateState();
        putProperty("WizardPanel_contentData", iterator.getSteps()); // NOI18N
        putProperty("WizardPanel_contentSelectedIndex", Integer.valueOf(iterator.getIndex())); // NOI18N
    }

}
