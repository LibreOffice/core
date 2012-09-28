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
import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Set;
import javax.swing.JComponent;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.openide.ErrorManager;
import org.openide.TopManager;
import org.openide.NotifyDescriptor;
import org.openide.WizardDescriptor;
import org.openide.cookies.OpenCookie;
import org.openide.cookies.SourceCookie;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.filesystems.*;

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.netbeans.modules.office.loader.ParcelFolder;
import org.openoffice.netbeans.modules.office.filesystem.OpenOfficeDocFileSystem;
import org.openoffice.netbeans.modules.office.utils.PackageRemover;

/** A template wizard iterator (sequence of panels).
 * Used to fill in the second and subsequent panels in the New wizard.
 * Associate this to a template inside a layer using the
 * Sequence of Panels extra property.
 * Create one or more panels from template as needed too.
 */
public class JavaScriptIterator implements TemplateWizard.Iterator {


    // private static final long serialVersionUID = ...L;

    // You should define what panels you want to use here:

    protected WizardDescriptor.Panel[] createPanels() {
        return new WizardDescriptor.Panel[] {
            // keep the default 2nd panel:
            wiz.targetChooser(),
        };
    }

    // And the list of step names:

    protected String[] createSteps() {
        return new String[] {
            null,
        };
    }

    private DataFolder checkTarget(DataFolder folder) {
        FileObject fo = folder.getPrimaryFile();

        try {
            FileSystem fs = fo.getFileSystem();

            if (fs instanceof OpenOfficeDocFileSystem && fo.isRoot()) {
                FileObject scripts =
                    fo.getFileObject(OpenOfficeDocFileSystem.SCRIPTS_ROOT);
                if (scripts == null)
                    scripts =
                        fo.createFolder(OpenOfficeDocFileSystem.SCRIPTS_ROOT);

                FileObject javafolder = scripts.getFileObject("java");
                if (javafolder == null)
                    javafolder = scripts.createFolder("java");

                DataFolder subfolder = new DataFolder(javafolder);
                return subfolder;
            }
        }
        catch (IOException ioe) {
            /* do nothing, we will just return the folder we were passed in */
        }
        return folder;
    }

    public Set instantiate(TemplateWizard wiz) throws IOException {
        String name = wiz.getTargetName();
        DataFolder targetFolder = wiz.getTargetFolder();
        targetFolder = checkTarget(targetFolder);

        DataObject template = wiz.getTemplate();
        DataObject result;
        if (name == null) {
            // Default name.
            result = template.createFromTemplate(targetFolder);
        } else {
            result = template.createFromTemplate(targetFolder, name);
        }

        FileObject tmp = result.getPrimaryFile();
        if (tmp.getExt().equals("java")) {
            try {
                PackageRemover.removeDeclaration(FileUtil.toFile(tmp));

                // IssueZilla 11986 - rename the FileObject
                // so the JavaNode is resynchronized
                tmp.rename(tmp.lock(), tmp.getName(), tmp.getExt());
            }
            catch (IOException ioe) {
                NotifyDescriptor d = new NotifyDescriptor.Message(
                 "Error removing package declaration from file: " +
                 tmp.getNameExt() +
                 ". You should manually remove this declaration " +
                 "before building the Parcel Recipe");
                TopManager.getDefault().notify(d);
            }
        }

        return Collections.singleton(result);
    }

    // --- The rest probably does not need to be touched. ---

    private transient int index;
    private transient WizardDescriptor.Panel[] panels;
    private transient TemplateWizard wiz;

    // You can keep a reference to the TemplateWizard which can
    // provide various kinds of useful information such as
    // the currently selected target name.
    // Also the panels will receive wiz as their "settings" object.
    public void initialize(TemplateWizard wiz) {
        this.wiz = wiz;
        index = 0;
        panels = createPanels();
        // Make sure list of steps is accurate.
        String[] steps = createSteps();
        for (int i = 0; i < panels.length; i++) {
            Component c = panels[i].getComponent();
            if (steps[i] == null) {
                // Default step name to component name of panel.
                // Mainly useful for getting the name of the target
                // chooser to appear in the list of steps.
                steps[i] = c.getName();
            }
            if (c instanceof JComponent) { // assume Swing components
                JComponent jc = (JComponent)c;
                // Step #.
                jc.putClientProperty("WizardPanel_contentSelectedIndex", new Integer(i)); // NOI18N
                // Step name (actually the whole list for reference).
                jc.putClientProperty("WizardPanel_contentData", steps); // NOI18N
            }
        }
    }
    public void uninitialize(TemplateWizard wiz) {
        this.wiz = null;
        panels = null;
    }

    // --- WizardDescriptor.Iterator METHODS: ---
    // Note that this is very similar to WizardDescriptor.Iterator, but with a
    // few more options for customization. If you e.g. want to make panels appear
    // or disappear dynamically, go ahead.

    public String name() {
        return "";
    }

    public boolean hasNext() {
        return index < panels.length - 1;
    }
    public boolean hasPrevious() {
        return index > 0;
    }
    public void nextPanel() {
        if (!hasNext()) throw new NoSuchElementException();
        index++;
    }
    public void previousPanel() {
        if (!hasPrevious()) throw new NoSuchElementException();
        index--;
    }
    public WizardDescriptor.Panel current() {
        return panels[index];
    }

    // If nothing unusual changes in the middle of the wizard, simply:
    public final void addChangeListener(ChangeListener l) {}
    public final void removeChangeListener(ChangeListener l) {}
    // If something changes dynamically (besides moving between panels),
    // e.g. the number of panels changes in response to user input, then
    // uncomment the following and call when needed:
    // fireChangeEvent();
    /*
    private transient Set listeners = new HashSet(1); // Set<ChangeListener>
    public final void addChangeListener(ChangeListener l) {
        synchronized(listeners) {
            listeners.add(l);
        }
    }
    public final void removeChangeListener(ChangeListener l) {
        synchronized(listeners) {
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
    private void readObject(ObjectInputStream in) throws IOException, ClassNotFoundException {
        in.defaultReadObject();
        listeners = new HashSet(1);
    }
     */

}
