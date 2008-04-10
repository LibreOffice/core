/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigurationRead.java,v $
 * $Revision: 1.3 $
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
package helper;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XHierarchicalName;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;

/**
 * Read configuration settings.
 */
public class ConfigurationRead {

    XHierarchicalNameAccess root = null;

    /**
     * Creates new ConfigurationRead
     * @param xMSF An instance of service
     *      "com.sun.star.configuration.ConfigurationProvider"
     * @param rootnode The root of the configuration nodes.
     */
    public ConfigurationRead(XMultiServiceFactory xMSF, String rootnode) {

        PropertyValue [] nodeArgs = new PropertyValue [1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = rootnode;
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0]=nodepath;

        try {
            Object rootObject = xMSF.createInstanceWithArguments(
                            "com.sun.star.configuration.ConfigurationAccess",
                            nodeArgs);

            root = (XHierarchicalNameAccess)
                            UnoRuntime.queryInterface(
                            XHierarchicalNameAccess.class, rootObject);
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Creates new ConfigurationRead. This uses "org.openoffice.Setup"
     * as default root name.
     * @param xMSF An instance of service
     *      "com.sun.star.configuration.ConfigurationProvider"
     */
    public ConfigurationRead(XMultiServiceFactory xMSF) {
        this(xMSF, "org.openoffice.Setup");
    }

    /**
     * Does the node with this hierarchical name exist?
     * @param name The hierarchical name of a subnode.
     * @return True, if the node exists.
     */
    public boolean hasByHieracrhicalName(String name) throws NoSuchElementException,
                                    com.sun.star.lang.WrappedTargetException {

        return root.hasByHierarchicalName(name);

    }


    /**
     * Get the elements of the root node.
     * @return All elements of the root node.
     */
    public String[] getRootNodeNames() {

        XNameAccess xName = (XNameAccess)
                    UnoRuntime.queryInterface(XNameAccess.class, root);
        String[]names = xName.getElementNames();
        return names;
    }

    /**
     * Get all elements of this node
     * @param name The name of the node
     * @return All elements of this node (as hierarchical names).
     */
    public String[] getSubNodeNames(String name) {
        String[]names = null;
        try {

            Object next = root.getByHierarchicalName(name);
            XNameAccess x = (XNameAccess)UnoRuntime.queryInterface(
                                                XNameAccess.class, next);
            names = x.getElementNames();
            for (int i=0; i< names.length; i++) {
                names[i] = name + "/" + names[i];
            }
        }
        catch(Exception e) {
            //just return null, if there are no further nodes
        }
        return names;
    }

    /**
     * Get contents of a node by its hierarchical name.
     * @param The hierarchical name of the node.
     * @return The contents as an object
     */
    public Object getByHierarchicalName(String name) throws NoSuchElementException {
        return root.getByHierarchicalName(name);
    }

}
