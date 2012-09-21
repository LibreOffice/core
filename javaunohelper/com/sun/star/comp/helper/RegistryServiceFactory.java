// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
        if (System.getProperty("java.vendor") == "The Android Project") {
            // See corresponding code in
            // javaunohelper/com/sun/star/comp/helper/Bootstrap.java for more
            // comments.

            boolean disable_dynloading = false;
            try {
                System.loadLibrary("lo-bootstrap");
            } catch (UnsatisfiedLinkError e) {
                disable_dynloading = true;
            }

            if (!disable_dynloading)
                System.loadLibrary("juh");
        } else
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
        return UnoRuntime.queryInterface(
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

// vim:set shiftwidth=4 softtabstop=4 expandtab:
