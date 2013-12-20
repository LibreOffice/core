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

package org.openoffice.netbeans.modules.office.filesystem;

import java.awt.Image;
import java.io.File;
import java.beans.*;

import org.openide.ErrorManager;
import org.openide.filesystems.FileSystem;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

/**
 * Description of the OpenOffice.org Document filesystem.
 */
public class OpenOfficeDocFileSystemBeanInfo
    extends SimpleBeanInfo
{
    private static String ICONLOCATION =
        "org/openoffice/netbeans/modules/office/resources";
    private static String COLORICON16NAME =
        ICONLOCATION + File.separator + "OpenOfficeDocFileSystemIcon.png";
    private static String COLORICON32NAME =
        ICONLOCATION + File.separator + "OpenOfficeDocFileSystemIcon32.png";

    /**
     * Retrieves an additional bean information.
     */
    public BeanInfo[] getAdditionalBeanInfo()
    {
        try {
            return new BeanInfo[] {
                Introspector.getBeanInfo(FileSystem.class)
            };
        } catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }
/*
    // If you have a visual dialog to customize configuration of the
    // filesystem:
    public BeanDescriptor getBeanDescriptor()
    {
        return new BeanDescriptor(OpenOfficeDocFileSystem.class,
            OpenOfficeDocFileSystemCustomizer.class);
    }
*/
    /**
     * Retrieves bean property descriptors.
     */
    public PropertyDescriptor[] getPropertyDescriptors()
    {
        try {
            // Included only to make it a writable property (it is read-only
            // in FileSystem):
            PropertyDescriptor readOnly = new PropertyDescriptor(
                "readOnly", OpenOfficeDocFileSystem.class);
            readOnly.setDisplayName(NbBundle.getMessage(
                OpenOfficeDocFileSystemBeanInfo.class, "PROP_readOnly"));
            readOnly.setShortDescription(NbBundle.getMessage(
                OpenOfficeDocFileSystemBeanInfo.class, "HINT_readOnly"));

            // This could be whatever properties you use to configure the
            // filesystem:
            PropertyDescriptor document = new PropertyDescriptor(
                "Document", OpenOfficeDocFileSystem.class);
            document.setDisplayName(NbBundle.getMessage(
                OpenOfficeDocFileSystemBeanInfo.class, "PROP_document"));
            document.setShortDescription(NbBundle.getMessage(
                OpenOfficeDocFileSystemBeanInfo.class, "HINT_document"));
            // Request to the property editor that it be permitted only to
            // choose directories:
            document.setValue("directories", Boolean.FALSE);    // NOI18N
            document.setValue("files", Boolean.TRUE);           // NOI18N

            return new PropertyDescriptor[] {readOnly, document};
        } catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }

    /**
     * Retrieves an icon by the icon type.
     */
    public Image getIcon(int type)
    {
        if((type == BeanInfo.ICON_COLOR_16x16) ||
            (type == BeanInfo.ICON_MONO_16x16)) {
            return Utilities.loadImage(COLORICON16NAME);
        } else {
            return Utilities.loadImage(COLORICON32NAME);
        }
    }

}
