/*************************************************************************
 *
 *  $RCSfile: ConfigurationRead.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Date: 2003-01-27 16:27:34 $
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
