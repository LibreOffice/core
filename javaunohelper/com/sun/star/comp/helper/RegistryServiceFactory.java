/*************************************************************************
 *
 *  $RCSfile: RegistryServiceFactory.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:31:32 $
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


package com.sun.star.comp.helper;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

public class RegistryServiceFactory {
    static {
        System.loadLibrary("juh");
    }

    private static native Object createRegistryServiceFactory(
            String writeRegistryFile,
            String readRegistryFile,
            boolean readOnly );

    /**
      * This bootstraps an initial service factory working on a registry. If the first or both
      * parameters contain a value then the service factory is initialized with a simple registry
      * or a nested registry. Otherwise the service factory must be initialized later with a valid
      * registry.
      *<BR>
      * @param writeRegistryFile    file name of the simple registry or the first registry file of
      *                             the nested registry which will be opened with read/write rights. This
      *                             file will be created if necessary.
      * @param readRegistryFile     file name of the second registry file of the nested registry
      *                             which will be opened with readonly rights.
      *
      * @author Markus Herzog
      */
    public static XMultiServiceFactory create(
            String writeRegistryFile, String readRegistryFile )
        throws com.sun.star.uno.Exception
    {
        return create(writeRegistryFile, readRegistryFile, false);
    }

    /**
      * This bootstraps an initial service factory working on a registry. If the first or both
      * parameters contain a value then the service factory is initialized with a simple registry
      * or a nested registry. Otherwise the service factory must be initialized later with a valid
      * registry.
      *<BR>
     * @param writeRegistryFile     file name of the simple registry or the first registry file of
     *                              the nested registry which will be opened with read/write rights. This
     *                              file will be created if necessary.
     * @param readRegistryFile      file name of the second registry file of the nested registry
     *                              which will be opened with readonly rights.
     * @param readOnly              flag which specify that the first registry file will be opened with
     *                              readonly rights. Default is FALSE. If this flag is used the registry
     *                              will not be created if not exist.
     *
      * @author Markus Herzog
      */
    public static XMultiServiceFactory create(
            String writeRegistryFile, String readRegistryFile, boolean readOnly )
        throws com.sun.star.uno.Exception
    {
        if (writeRegistryFile == null && readRegistryFile == null)
            throw new com.sun.star.uno.Exception("No registry is specified!");

        if (writeRegistryFile != null) {
            java.io.File file = new java.io.File(writeRegistryFile);

            if (file.exists()) {
                if (!file.isFile())
                    throw new com.sun.star.uno.Exception(writeRegistryFile + " is not a file!");
            } else
                throw new com.sun.star.uno.Exception(writeRegistryFile + " doese not exist!");
        }

        if (readRegistryFile != null) {
            java.io.File file = new java.io.File(readRegistryFile);

            if (file.exists()) {
                if (!file.isFile())
                    throw new com.sun.star.uno.Exception(readRegistryFile + " is not a file!");
            } else
                throw new com.sun.star.uno.Exception(readRegistryFile + " doese not exist!");
        }

        Object obj = createRegistryServiceFactory( writeRegistryFile, readRegistryFile, readOnly );
        return (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, obj );
    }

    /**
     * This bootstraps an initial service factory working on a registry file.
     *<BR>
     * @param registryFile          file name of the registry to use/ create; if this is an empty
     *                              string, the default registry is used instead
     *
     * @author Markus Herzog
     */
    public static XMultiServiceFactory create( String registryFile )
        throws com.sun.star.uno.Exception
    {
        return create(registryFile, null, false);
    }

    /**
     * This bootstraps an initial service factory working on a registry file.
     *<BR>
     * @param registryFile          file name of the registry to use/ create; if this is an empty
     *                              string, the default registry is used instead
     * @param readOnly              flag which specify that the registry file will be opened with
     *                              readonly rights. Default is FALSE. If this flag is used the registry
     *                              will not be created if not exist.
     *
     * @author Markus Herzog
     */
    public static XMultiServiceFactory create( String registryFile, boolean readOnly )
        throws com.sun.star.uno.Exception
    {
        return create(registryFile, null, readOnly);
    }

    /**
     * This bootstraps a service factory without initilaize a registry.
     *<BR>
     * @author Markus Herzog
     */
    public static XMultiServiceFactory create()
        throws com.sun.star.uno.Exception
    {
        return create( null, null, false );
    }
}

