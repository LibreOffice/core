/*************************************************************************
 *
 *  $RCSfile: ParcelContentsIterator.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-09-10 10:46:30 $
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

package org.openoffice.netbeans.modules.office.wizard;

import java.awt.Component;
import java.io.File;
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

import org.openide.TopManager;
import org.openide.NotifyDescriptor;
import org.openide.WizardDescriptor;
import org.openide.cookies.OpenCookie;
import org.openide.cookies.SourceCookie;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.filesystems.*;

import com.sun.star.script.framework.browse.ParcelDescriptor;
import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.netbeans.modules.office.loader.ParcelFolder;
import org.openoffice.netbeans.modules.office.loader.ParcelContentsFolder;
import org.openoffice.netbeans.modules.office.filesystem.OpenOfficeDocFileSystem;

/** A template wizard iterator (sequence of panels).
 * Used to fill in the second and subsequent panels in the New wizard.
 * Associate this to a template inside a layer using the
 * Sequence of Panels extra property.
 * Create one or more panels from template as needed too.
 *
 * @author tomaso
 */
public class ParcelContentsIterator implements TemplateWizard.Iterator {


    // private static final long serialVersionUID = ...L;

    // You should define what panels you want to use here:

    public static final String PROP_LANGUAGE =
        ParcelFolder.LANGUAGE_ATTRIBUTE;

    protected WizardDescriptor.Panel[] createPanels() {
        return new WizardDescriptor.Panel[] {
            // keep the default 2nd panel:
            // wiz.targetChooser(),
            new ParcelPropertiesPanel(),
        };
    }

    // And the list of step names:

    protected String[] createSteps() {
        return new String[] {
            // null,
            "Parcel Properties",
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

        String language = (String)wiz.getProperty(PROP_LANGUAGE);

        DataObject template = wiz.getTemplate();
        DataObject result;
        if (name == null) {
            // Default name.
            result = template.createFromTemplate(targetFolder);
        } else {
            result = template.createFromTemplate(targetFolder, name);
        }

        FileObject recipe = result.getPrimaryFile();

        FileObject contents =
            recipe.getFileObject(ParcelZipper.CONTENTS_DIRNAME);

        if (contents != null) {
            File f = FileUtil.toFile(contents);
            ParcelDescriptor pd = ParcelDescriptor.createParcelDescriptor(f);
            pd.setLanguage(language);
            pd.write();

            DataFolder parent = DataFolder.findFolder(contents);
            ParcelContentsFolder.createEmptyScript(parent, language);
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
