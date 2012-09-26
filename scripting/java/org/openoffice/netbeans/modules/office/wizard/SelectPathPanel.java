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

import java.awt.Component;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.openide.WizardDescriptor;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;

import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.idesupport.OfficeInstallation;
import org.openoffice.idesupport.SVersionRCFile;

/** A single panel descriptor for a wizard.
 * You probably want to make a wizard iterator to hold it.
 */
public class SelectPathPanel implements WizardDescriptor.Panel /* .FinishPanel */ {

    /** The visual component that displays this panel.
     * If you need to access the component from this class,
     * just use getComponent().
     */
    private SelectPathVisualPanel component;
    private OfficeInstallation office;

    /** Create the wizard panel descriptor. */
    public SelectPathPanel() {
        office = OfficeSettings.getDefault().getOfficeDirectory();

        if (office == null) {
            try {
                office = SVersionRCFile.createInstance().getDefaultVersion();
            }
            catch (java.io.IOException ioe) {}
        }
    }

    // Get the visual component for the panel. In this template, the component
    // is kept separate. This can be more efficient: if the wizard is created
    // but never displayed, or not all panels are displayed, it is better to
    // create only those which really need to be visible.
    public Component getComponent() {
        if (component == null) {
            component = new SelectPathVisualPanel(this);
        }
        return component;
    }

    public HelpCtx getHelp() {
        // Show no Help button for this panel:
        return HelpCtx.DEFAULT_HELP;
        // If you have context help:
        // return new HelpCtx(SelectPathPanel.class);
    }

    public boolean isValid() {
        // If it is always OK to press Next or Finish, then:
        return true;
        // If it depends on some condition (form filled out...), then:
        // return someCondition();
        // and when this condition changes (last form field filled in...) then:
        // fireChangeEvent();
        // and uncomment the complicated stuff below.
    }

    // public final void addChangeListener(ChangeListener l) {}
    // public final void removeChangeListener(ChangeListener l) {}

    private final Set listeners = new HashSet(1); // Set<ChangeListener>
    public final void addChangeListener(ChangeListener l) {
        synchronized (listeners) {
            listeners.add(l);
        }
    }
    public final void removeChangeListener(ChangeListener l) {
        synchronized (listeners) {
            listeners.remove(l);
        }
    }
    protected final void fireChangeEvent() {
        Iterator it;
        synchronized (listeners) {
            it = new HashSet(listeners).iterator();
        }
        ChangeEvent ev = new ChangeEvent(this);
        while (it.hasNext()) {
            ((ChangeListener)it.next()).stateChanged(ev);
        }
    }

    public void setSelectedPath(OfficeInstallation oi) {
        this.office = oi;
        fireChangeEvent();
    }

    public OfficeInstallation getSelectedPath() {
        return office;
    }

    // You can use a settings object to keep track of state.
    // Normally the settings object will be the WizardDescriptor,
    // so you can use WizardDescriptor.getProperty & putProperty
    // to store information entered by the user.
    public void readSettings(Object settings) {
    }

    public void storeSettings(Object settings) {
        WizardDescriptor wiz = (WizardDescriptor)settings;
        wiz.putProperty(InstallationPathDescriptor.PROP_INSTALLPATH, office);
    }
}
