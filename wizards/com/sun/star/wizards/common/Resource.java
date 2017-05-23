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

package com.sun.star.wizards.common;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.resource.StringResourceWithLocation;
import com.sun.star.resource.XStringResourceWithLocation;
import com.sun.star.util.XMacroExpander;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

public final class Resource
{
    private XStringResourceWithLocation m_xStrResource;

    /**
     * <p>Load the resource bundle that contains the resource {@code String}
     * values.</p>
     */
    public Resource(XMultiServiceFactory xMSF) {
        XComponentContext xContext = Helper.getComponentContext(xMSF);
        XMacroExpander xExpander = Helper.getMacroExpander(xMSF);
        String sPath = xExpander.expandMacros("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/wizards/");
        Locale locale = Configuration.getUILocale(xMSF);
        m_xStrResource = StringResourceWithLocation.create(xContext, sPath, true, locale, "resources", "", null);
    }

    /**
     * This method returns the corresponding {@code String} given the key.
     *
     * @param   key      Key string for getting the message {@code String}.
     * @return  Message  {@code String} corresponding to the key.
     */
    public String getResText(String key) {
        return m_xStrResource.resolveString(key);
    }

    public static void showCommonResourceError(XMultiServiceFactory xMSF)
    {
        String ProductName = Configuration.getProductName(xMSF);
        String sError = "The files required could not be found.\nPlease start the %PRODUCTNAME Setup and choose 'Repair'.";
        sError = JavaTools.replaceSubString(sError, ProductName, "%PRODUCTNAME");
        SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sError);
    }
}
