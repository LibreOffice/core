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

package org.openoffice.xmerge.util;

import java.util.ResourceBundle;

/**
 *  <p>Provides a singleton resource class for converter messages.</p>
 *
 *  <p>By default, this class will search for a
 *  <code>ResourceBundle</code> class file or properties file based
 *  on the default locale.</p>
 *
 *  <p>A properties file resources.properties will be provided.</p>
 *
 *  <p>Note that if the resource bundle object is not loaded, the
 *  construction of the singleton object will throw a
 *  <code>MissingResourceException</code>, which is a
 *  <code>RuntimeException</code>, thus I opted to not explicitly
 *  declare it.  If it does throw <code>MissingResourceException</code>,
 *  it may be due to a packaging problem.</p>
 *
 *  @author      Herbie Ong
 */
public final class Resources
{
    private ResourceBundle rb = null;

    private static Resources instance = null;


    /**
     *  This method returns the singleton instance
     *  of this class.
     *
     *  @return  The singleton <code>Resources</code>
     *           instance.
     */
    public synchronized static Resources getInstance()
    {
        if (instance == null)
        {
            instance = new Resources();
        }

        return instance;
    }


    /**
     *  Default constructor is only accessible within this class.
     *  Load the resource bundle that contains the resource
     *  <code>String</code> values.
     */
    private Resources()
    {
        rb = ResourceBundle.getBundle("org.openoffice.xmerge.util.resources");
    }


    /**
     *  This method returns the corresponding <code>String</code> given
     *  the key.
     *
     *  @param   key     Key string for getting the message
     *                   <code>String</code>.
     *  @return  Message <code>String</code> corresponding to the key.
     */
    public String getString(String key)
    {
        return rb.getString(key);
    }
}

