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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;


/** This handler displays lower level UNO information.  These are the
    supported services, interfaces, and the implementation name.
*/
class AccessibleUNOHandler
    extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        if (xContext == null)
            return null;
        else
            return new AccessibleUNOHandler (xContext);
    }

    public AccessibleUNOHandler()
    {
    }

    public AccessibleUNOHandler (XAccessibleContext xContext)
    {
        maChildList.setSize (3);
    }

    private XServiceInfo GetServiceInfo (AccessibleTreeNode aNode)
    {
        XServiceInfo xServiceInfo = null;
        if (aNode instanceof AccTreeNode)
            xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(
                XServiceInfo.class, ((AccTreeNode)aNode).getContext());
        return xServiceInfo;
    }
    private XTypeProvider GetTypeProvider (AccessibleTreeNode aNode)
    {
        XTypeProvider xTypeProvider = null;
        if (aNode instanceof AccTreeNode)
            xTypeProvider = (XTypeProvider)UnoRuntime.queryInterface(
                XTypeProvider.class, ((AccTreeNode)aNode).getContext());
        return xTypeProvider;
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent,
                                           int nIndex)
    {
        AccessibleTreeNode aChild = null;
        XServiceInfo xServiceInfo;
        switch (nIndex)
        {
            case 0 : // Implemenation name.
                xServiceInfo = GetServiceInfo (aParent);
                aChild = new StringNode ("Implementation name: " +
                    (xServiceInfo!=null ? xServiceInfo.getImplementationName()
                        : "<XServiceInfo not supported>"),
                    aParent);
                break;
            case 1 :
                xServiceInfo = GetServiceInfo (aParent);
                if (xServiceInfo == null)
                    aChild = new StringNode (
                        "Supported services: <XServiceInfo not supported>",
                        aParent);
                else
                    aChild = CreateServiceTree (aParent, xServiceInfo);
                break;
            case 2 :
                XTypeProvider xTypeProvider = GetTypeProvider (aParent);
                if (xTypeProvider == null)
                    aChild = new StringNode (
                        "Supported interfaces: <XTypeProvider not supported>",
                        aParent);
                else
                    aChild = CreateInterfaceTree (aParent, xTypeProvider);
                break;
        }

        return aChild;
    }


    private AccessibleTreeNode CreateServiceTree (AccessibleTreeNode aParent,
        XServiceInfo xServiceInfo)
    {
        String[] aServiceNames = xServiceInfo.getSupportedServiceNames();
        VectorNode aNode = new VectorNode ("Supported Services", aParent);

        int nCount = aServiceNames.length;
        for (int i=0; i<nCount; i++)
            aNode.addChild (new StringNode (aServiceNames[i], aParent));

        return aNode;
    }

    private AccessibleTreeNode CreateInterfaceTree (AccessibleTreeNode aParent,
        XTypeProvider xTypeProvider)
    {
        Type[] aTypes = xTypeProvider.getTypes();
        VectorNode aNode = new VectorNode ("Supported Interfaces", aParent);

        int nCount = aTypes.length;
        for (int i=0; i<nCount; i++)
            aNode.addChild (new StringNode (aTypes[i].getTypeName(), aParent));

        return aNode;
    }
}
