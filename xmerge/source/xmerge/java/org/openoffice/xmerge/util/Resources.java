/************************************************************************
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

