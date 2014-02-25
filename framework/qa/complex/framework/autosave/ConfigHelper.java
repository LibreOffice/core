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

package complex.framework.autosave;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.util.*;

class ConfigHelper
{
    private XHierarchicalNameAccess m_xConfig = null;


    public ConfigHelper(XComponentContext context,
                        String               sConfigPath ,
                        boolean              bReadOnly   )
        throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xConfigRoot = theDefaultProvider.get(context);

        PropertyValue[] lParams = new PropertyValue[1];
        lParams[0] = new PropertyValue();
        lParams[0].Name  = "nodepath";
        lParams[0].Value = sConfigPath;

        Object aConfig;
        if (bReadOnly)
        {
            aConfig = xConfigRoot.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", lParams);
        }
        else
        {
            aConfig = xConfigRoot.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", lParams);
        }

        m_xConfig = UnoRuntime.queryInterface(XHierarchicalNameAccess.class, aConfig);

        if (m_xConfig == null)
        {
            throw new com.sun.star.uno.Exception("Could not open configuration \"" + sConfigPath + "\"");
        }
    }


    public Object readRelativeKey(String sRelPath,
                                  String sKey    )
        throws com.sun.star.container.NoSuchElementException
    {
        try
        {
            XPropertySet xPath = UnoRuntime.queryInterface(XPropertySet.class, m_xConfig.getByHierarchicalName(sRelPath));
            return xPath.getPropertyValue(sKey);
        }
        catch(com.sun.star.uno.Exception ex)
        {
            throw new com.sun.star.container.NoSuchElementException(ex.getMessage());
        }
    }


    public void writeRelativeKey(String sRelPath,
                                 String sKey    ,
                                 Object aValue  )
        throws com.sun.star.container.NoSuchElementException
    {
        try
        {
            XPropertySet xPath = UnoRuntime.queryInterface(XPropertySet.class, m_xConfig.getByHierarchicalName(sRelPath));
            xPath.setPropertyValue(sKey, aValue);
        }
        catch(com.sun.star.uno.Exception ex)
        {
            throw new com.sun.star.container.NoSuchElementException(ex.getMessage());
        }
    }


    public void flush()
    {
        try
        {
            XChangesBatch xBatch = UnoRuntime.queryInterface(XChangesBatch.class, m_xConfig);
            xBatch.commitChanges();
        }
        catch(com.sun.star.uno.Exception ex)
        {}
    }
}
