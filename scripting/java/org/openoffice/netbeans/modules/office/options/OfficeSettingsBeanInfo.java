/*************************************************************************
 *
 *  $RCSfile: OfficeSettingsBeanInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-24 12:53:43 $
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

package org.openoffice.netbeans.modules.office.options;

import java.awt.Image;
import java.awt.Component;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import java.beans.*;

import org.openide.ErrorManager;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

import org.openoffice.idesupport.OfficeInstallation;
import org.openoffice.netbeans.modules.office.wizard.SelectPathPanel;

/** Description of {@link OfficeSettings}.
 *
 * @author tomaso
 */
public class OfficeSettingsBeanInfo extends SimpleBeanInfo {

    public PropertyDescriptor[] getPropertyDescriptors() {
        try {
            PropertyDescriptor[] props = new PropertyDescriptor[] {
                new PropertyDescriptor(OfficeSettings.OFFICE_DIRECTORY,
                               OfficeSettings.class,
                               "get" + OfficeSettings.OFFICE_DIRECTORY,
                               "set" + OfficeSettings.OFFICE_DIRECTORY),
                new PropertyDescriptor(OfficeSettings.WARN_BEFORE_DOC_DEPLOY,
                               OfficeSettings.class,
                               "get" + OfficeSettings.WARN_BEFORE_DOC_DEPLOY,
                               "set" + OfficeSettings.WARN_BEFORE_DOC_DEPLOY),
                new PropertyDescriptor(OfficeSettings.WARN_AFTER_DIR_DEPLOY,
                               OfficeSettings.class,
                               "get" + OfficeSettings.WARN_AFTER_DIR_DEPLOY,
                               "set" + OfficeSettings.WARN_AFTER_DIR_DEPLOY),
                new PropertyDescriptor(OfficeSettings.WARN_BEFORE_MOUNT,
                               OfficeSettings.class,
                               "get" + OfficeSettings.WARN_BEFORE_MOUNT,
                               "set" + OfficeSettings.WARN_BEFORE_MOUNT)
            };

            props[0].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_OfficeDirectory"));
            props[0].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_OfficeDirectory"));
            props[0].setPropertyEditorClass(OfficeDirectoryEditor.class);

            props[1].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_WarnBeforeDocDeploy"));
            props[1].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_WarnBeforeDocDeploy"));
            props[1].setHidden(true);

            props[2].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_WarnAfterDirDeploy"));
            props[2].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_WarnAfterDirDeploy"));
            props[2].setHidden(true);

            props[3].setDisplayName(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "PROP_WarnBeforeMount"));
            props[3].setShortDescription(NbBundle.getMessage(
                OfficeSettingsBeanInfo.class, "HINT_WarnBeforeMount"));
            props[3].setHidden(true);

            return props;
        }
        catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }

    public Image getIcon(int type) {
        if (type == BeanInfo.ICON_COLOR_16x16 || type == BeanInfo.ICON_MONO_16x16) {
            return Utilities.loadImage("/org/openoffice/netbeans/modules/office/options/OfficeSettingsIcon.gif");
        } else {
            return Utilities.loadImage("/org/openoffice/netbeans/modules/office/options/OfficeSettingsIcon32.gif");
        }
    }

    public static class OfficeDirectoryEditor extends PropertyEditorSupport
        implements ChangeListener {

        private SelectPathPanel panel;

        public String getAsText () {
            return ((OfficeInstallation)getValue()).getPath();
        }

        public void setAsText (String path) {
            OfficeInstallation oi = new OfficeInstallation(path);

            if (!oi.supportsFramework())
                throw new IllegalArgumentException(path +
                    " is not a valid Office install");
            else
                setValue (oi);
        }

        public Component getCustomEditor() {
            panel = new SelectPathPanel();
            panel.addChangeListener(this);
            return panel.getComponent();
        }

        public boolean supportsCustomEditor() {
            return true;
        }

        public void stateChanged(ChangeEvent evt) {
            setValue(panel.getSelectedPath());
        }
    }
}
