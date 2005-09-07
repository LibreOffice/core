/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegistryServiceFactory.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:36:25 $
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


package com.sun.star.comp.helper;


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.RuntimeException;

/** The class provides a set of methods which create instances of the
    com.sun.star.lang.RegistryServiceManager service.

    @deprecated use class Bootstrap instead
*/
public class RegistryServiceFactory {
    static {
        System.loadLibrary("juh");
    }

    private static native Object createRegistryServiceFactory(
            String writeRegistryFile,
            String readRegistryFile,
            boolean readOnly,
            ClassLoader loader);

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
     * @return a new RegistryServiceFactory.
      */
    public static XMultiServiceFactory create(String writeRegistryFile, String readRegistryFile)
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
      * @return a new RegistryServiceFactory
      */
    public static XMultiServiceFactory create(String writeRegistryFile, String readRegistryFile, boolean readOnly)
        throws com.sun.star.uno.Exception
    {
        // Ensure that we are on a native threads vm
        // (binary UNO does use native threads).
        String vm_info = System.getProperty("java.vm.info");
        if(vm_info != null && vm_info.indexOf("green") != -1)
            throw new RuntimeException(RegistryServiceFactory.class.toString() + ".create - can't use binary UNO with green threads");


        if (writeRegistryFile == null && readRegistryFile == null)
            throw new com.sun.star.uno.Exception("No registry is specified!");

//          if (writeRegistryFile != null) {
//              java.io.File file = new java.io.File(writeRegistryFile);

//              if (file.exists()) {
//                  if (!file.isFile())
//                      throw new com.sun.star.uno.Exception(writeRegistryFile + " is not a file!");
//              } else
//                  throw new com.sun.star.uno.Exception(writeRegistryFile + " doese not exist!");
//          }

//          if (readRegistryFile != null) {
//              java.io.File file = new java.io.File(readRegistryFile);

//              if (file.exists()) {
//                  if (!file.isFile())
//                      throw new com.sun.star.uno.Exception(readRegistryFile + " is not a file!");
//              } else
//                  throw new com.sun.star.uno.Exception(readRegistryFile + " doese not exist!");
//          }

        Object obj = createRegistryServiceFactory(
            writeRegistryFile, readRegistryFile, readOnly,
            RegistryServiceFactory.class.getClassLoader() );
        return (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, obj );
    }

    /**
     * This bootstraps an initial service factory working on a registry file.
     *<BR>
     * @param registryFile          file name of the registry to use/ create; if this is an empty
     *                              string, the default registry is used instead
     *
     * @return a new RegistryServiceFactory.
     */
    public static XMultiServiceFactory create(String registryFile)
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
     * @return a new RegistryServiceFactory.
     */
    public static XMultiServiceFactory create(String registryFile, boolean readOnly)
        throws com.sun.star.uno.Exception
    {
        return create(registryFile, null, readOnly);
    }

    /**
     * This bootstraps a service factory without initialize a registry.
     *<BR>
     * @return a new RegistryServiceFactory.
     */
    public static XMultiServiceFactory create() throws com.sun.star.uno.Exception {
        return create( null, null, false );
    }
}

