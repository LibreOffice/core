/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Resources.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:41:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.util;

import java.util.ResourceBundle;
import java.util.MissingResourceException;

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

