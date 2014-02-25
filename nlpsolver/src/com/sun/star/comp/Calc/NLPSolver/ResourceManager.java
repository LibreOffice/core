/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
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

package com.sun.star.comp.Calc.NLPSolver;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.deployment.PackageInformationProvider;
import com.sun.star.deployment.XPackageInformationProvider;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.resource.StringResourceWithLocation;
import com.sun.star.resource.XStringResourceWithLocation;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class ResourceManager {

    private final XComponentContext m_context;
    private final String m_oxtRoot;
    private final String m_resourceBaseUrl;
    private final String m_resourceBasename;
    private XStringResourceWithLocation m_xStrResource;
    private Locale m_locale;

    public ResourceManager(XComponentContext xContext, String oxtId, String relativeResourceBaseUrl, String resourceBasename) {
        m_context = xContext;
        m_resourceBasename = resourceBasename;

        XPackageInformationProvider xPkgInfo = PackageInformationProvider.get(m_context);
        m_oxtRoot = xPkgInfo.getPackageLocation(oxtId);
        m_resourceBaseUrl = m_oxtRoot + relativeResourceBaseUrl;

        try {
            XMultiServiceFactory xConfig = theDefaultProvider.get(m_context);

            Object[] args = new Object[1];
            args[0] = new PropertyValue("nodepath", 0, "/org.openoffice.Setup/L10N", PropertyState.DIRECT_VALUE);
            XPropertySet xConfigProps = UnoRuntime.queryInterface(XPropertySet.class,
                xConfig.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", args));
            String[] locale = AnyConverter.toString(xConfigProps.getPropertyValue("ooLocale")).split("-");
            String lang = locale[0];
            String country = (locale.length >= 2 ? locale[1] : "");
            String variant = (locale.length >= 3 ? locale[2] : "");
            m_locale = new Locale(lang, country, variant);

            m_xStrResource = StringResourceWithLocation.create(m_context, m_resourceBaseUrl, true, m_locale, m_resourceBasename, "", null);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /* This implementation is used if the presence of the key will be handled
     * "outside" (i.e. by catching the appropriate MissingResourceException). */
    public String getLocalizedString(String key) throws com.sun.star.resource.MissingResourceException {
        return m_xStrResource.resolveString(m_resourceBasename + "." + key);
    }

    /* This implementation on the other hand handles the exception by itself
     * and returns a (predefined) default value if necessary. */
    public String getLocalizedString(String key, String defaultValue) {
        try {
            return m_xStrResource.resolveString(m_resourceBasename + "." + key);
        } catch (com.sun.star.resource.MissingResourceException ex) {
            return defaultValue;
        }
    }

}
