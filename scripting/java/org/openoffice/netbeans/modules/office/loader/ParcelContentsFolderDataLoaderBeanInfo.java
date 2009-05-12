/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParcelContentsFolderDataLoaderBeanInfo.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.netbeans.modules.office.loader;

import java.awt.Image;
import java.beans.*;

import org.openide.ErrorManager;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

/** Description of {@link ParcelFolderDataLoader}.
 *
 * @author tomaso
 */
public class ParcelContentsFolderDataLoaderBeanInfo extends SimpleBeanInfo {

    // If you have additional properties:
    /*
    public PropertyDescriptor[] getPropertyDescriptors() {
        try {
            PropertyDescriptor myProp = new PropertyDescriptor("myProp", ParcelFolderDataLoader.class);
            myProp.setDisplayName(NbBundle.getMessage(ParcelFolderDataLoaderBeanInfo.class, "PROP_myProp"));
            myProp.setShortDescription(NbBundle.getMessage(ParcelFolderDataLoaderBeanInfo.class, "HINT_myProp"));
            return new PropertyDescriptor[] {myProp};
        } catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }
     */

    public BeanInfo[] getAdditionalBeanInfo() {
        try {
            // I.e. MultiFileLoader.class or UniFileLoader.class.
            return new BeanInfo[] {Introspector.getBeanInfo(ParcelContentsFolderDataLoader.class.getSuperclass())};
        } catch (IntrospectionException ie) {
            ErrorManager.getDefault().notify(ie);
            return null;
        }
    }

    public Image getIcon(int type) {
        if (type == BeanInfo.ICON_COLOR_16x16 || type == BeanInfo.ICON_MONO_16x16) {
            return Utilities.loadImage("org/openoffice/netbeans/modules/office/loaders/ParcelFolderDataIcon.gif");
        } else {
            return Utilities.loadImage("org/openoffice/netbeans/modules/office/loaders/ParcelFolderDataIcon32.gif");
        }
    }

}
