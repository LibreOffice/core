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
package helper;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.uno.UnoRuntime;

/**
 * Read configuration settings.
 */
public class ConfigurationRead {

    private XHierarchicalNameAccess root = null;

    /**
     * Creates new ConfigurationRead
     * @param xMSF An instance of service
     *      "com.sun.star.configuration.ConfigurationProvider"
     * @param rootnode The root of the configuration nodes.
     */
    private ConfigurationRead(XMultiServiceFactory xMSF, String rootnode) {

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

            root = UnoRuntime.queryInterface(
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
     * Get contents of a node by its hierarchical name.
     * @param name The hierarchical name of the node.
     * @return The contents as an object
     */
    public Object getByHierarchicalName(String name) throws NoSuchElementException {
        return root.getByHierarchicalName(name);
    }

}
