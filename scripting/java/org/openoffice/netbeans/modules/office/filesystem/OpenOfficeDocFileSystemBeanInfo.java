/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
     * Retrives an additional bean information.
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
     * Retrives bean property descriptors.
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
     * Retrives an icon by the icon type.
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
