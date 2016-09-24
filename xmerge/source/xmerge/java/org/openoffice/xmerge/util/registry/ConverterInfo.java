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

package org.openoffice.xmerge.util.registry;

import java.lang.reflect.Constructor;
import java.net.URL;
import java.net.URLClassLoader;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.DocumentMergerFactory;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.PluginFactory;

/**
 * Class for storing the information about a converter plug-in.
 */
public class ConverterInfo {

    /** Keep track of the valid Office mime types. */
    private static final String[] validOfficeTypes = new String[] {
        // This needs to be updated to reflect all valid office types.
        "staroffice/sxw",
        "staroffice/sxc"
    };

    private final String             piJarName;
    private final String             piOfficeMime;
    private final ArrayList<String>  piDeviceMime;
    private final String             piDisplayName;
    private final String             piDescription;
    private final String             piVersion;
    private final String             piVendor;
    private final String             piClassImpl;
    private String             piXsltSerial;
    private String             piXsltDeserial;
    private boolean            piCanSerialize    = false;
    private boolean            piCanDeserialize  = false;
    private boolean            piCanMerge        = false;
    private final ClassLoader        piClassLoader;
    private PluginFactory      piPluginFactory;


    /**
     * The constructor builds a ConverterInfo structure.
     *
     * @param   jarName      The URL of the jarfile.
     * @param   officeMime   The office mime-type.
     * @param   deviceMime   The device mime-type.
     * @param   displayName  The display name.
     * @param   description  The description.
     * @param   version      The version.
     * @param   vendor       The vendor name.
     * @param   impl         The implementation class name of
     *                       {@code PluginFactory}.
     * @param   xsltSerial   The URL of the serializer XSL stylesheet
     * @param   xsltDeserial The URL of the deserializer XSL stylesheet
     *
     * @throws  RegistryException  If {@code ci} cannot be loaded.
     */
    public ConverterInfo(String jarName, String officeMime,
                         ArrayList<String> deviceMime, String displayName,
                         String description, String version, String vendor,
                         String impl,String xsltSerial, String xsltDeserial)
            throws RegistryException {

        if (!isValidOfficeType(officeMime.trim())) {
            RegistryException re = new RegistryException( "Invalid office type");
            throw re;
        }

        piJarName     = jarName.trim();
        piOfficeMime  = officeMime.trim();
        piDeviceMime  = deviceMime;
        piDisplayName = displayName.trim();
        piDescription = description.trim();
        piVersion     = version.trim();
        piVendor      = vendor.trim();
        piXsltSerial  = xsltSerial.trim();
        piXsltDeserial= xsltDeserial.trim();
        piClassImpl   = impl.trim();
        piClassLoader = this.getClass().getClassLoader();

        // Get instance of the PluginFactory.

        try {
            final URL jarURL = new URL(jarName);
            final URL[] urls = new URL[] { jarURL };
            URLClassLoader loader = AccessController.doPrivileged(
                new PrivilegedAction<URLClassLoader>() {
                    public URLClassLoader run() {
                        return new URLClassLoader(urls, piClassLoader);
                    }
                });
            Class<?> clas = loader.loadClass(piClassImpl);
            Class<?>[] argumentTypes = { org.openoffice.xmerge.util.registry.ConverterInfo.class };
            Constructor<?> construct = clas.getConstructor(argumentTypes);

            Object[] arguments = { this };
            piPluginFactory = ( PluginFactory ) construct.newInstance(arguments);

            // See which interfaces the plug-in PluginFactory supports.

            Class<?>[] cl = piPluginFactory.getClass().getInterfaces();
            for (int i=0; i < cl.length; i++) {

               if (cl[i].getName().equals("org.openoffice.xmerge.DocumentSerializerFactory")) {
                   piCanSerialize = true;
               }
               if (cl[i].getName().equals("org.openoffice.xmerge.DocumentDeserializerFactory")) {
                   piCanDeserialize = true;
               }
               if (cl[i].getName().equals("org.openoffice.xmerge.DocumentMergerFactory")) {
                   piCanMerge = true;
               }
            }

        } catch (Exception e) {
            throw new RegistryException(
                "Class implementation of the plug-in cannot be loaded.", e);
        }
    }

    /**
     * The constructor builds a ConverterInfo structure.
     *
     * @param   jarName      The URL of the jarfile.
     * @param   officeMime   The office mime-type.
     * @param   deviceMime   The device mime-type.
     * @param   displayName  The display name.
     * @param   description  The description.
     * @param   version      The version.
     * @param   vendor       The vendor name.
     * @param   impl         The implementation class name of
     *                       {@code PluginFactory}.
     *
     * @throws  RegistryException  If {@code ci} cannot be loaded.
     */
    public ConverterInfo(String jarName, String officeMime,
        ArrayList<String> deviceMime, String displayName, String description,
        String version, String vendor, String impl)
        throws RegistryException {

        if (officeMime == null || displayName == null || description == null ||
            version    == null || vendor      == null || impl        == null)
          throw new IllegalArgumentException("arguments unexpected null");

        if (!isValidOfficeType(officeMime.trim())) {
            RegistryException re = new RegistryException(
                "Invalid office type");
            throw re;
        }

        piJarName     = jarName.trim();
        piOfficeMime  = officeMime.trim();
        piDeviceMime  = deviceMime;
        piDisplayName = displayName.trim();
        piDescription = description.trim();
        piVersion     = version.trim();
        piVendor      = vendor.trim();
        piClassImpl   = impl.trim();
        piClassLoader = this.getClass().getClassLoader();

        // Get instance of the PluginFactory.

        try {
            final URL jarURL = new URL(jarName);
            final URL[] urls = new URL[] { jarURL };
            URLClassLoader loader = AccessController.doPrivileged(
                new PrivilegedAction<URLClassLoader>() {
                    public URLClassLoader run() {
                        return new URLClassLoader(urls, piClassLoader);
                    }
                });
            Class<?> clas = loader.loadClass(piClassImpl);
            Class<?>[] argumentTypes = { org.openoffice.xmerge.util.registry.ConverterInfo.class };
            Constructor<?> construct = clas.getConstructor(argumentTypes);

            Object[] arguments = { this };
            piPluginFactory = ( PluginFactory ) construct.newInstance(arguments);

            // See which interfaces the plug-in PluginFactory supports.

            Class<?>[] cl = piPluginFactory.getClass().getInterfaces();
            for (int i=0; i < cl.length; i++) {

               if (cl[i].getName().equals("org.openoffice.xmerge.DocumentSerializerFactory")) {
                   piCanSerialize = true;
               }
               if (cl[i].getName().equals("org.openoffice.xmerge.DocumentDeserializerFactory")) {
                   piCanDeserialize = true;
               }
               if (cl[i].getName().equals("org.openoffice.xmerge.DocumentMergerFactory")) {
                   piCanMerge = true;
               }
            }

        } catch (Exception e) {
            throw new RegistryException(
                "Class implementation of the plug-in cannot be loaded.", e);
        }
    }

    /**
     * Returns an instance of the {@code DocumentDeserializerFactory} interface.
     *
     * @return  instance of the {@code DocumentDeserializer} for this
     *          {@code ConverterInfo}.
     */
    public DocumentSerializerFactory getDocSerializerFactory() {
        return (DocumentSerializerFactory)piPluginFactory;
    }

    /**
     * Returns an instance of the {@code DocumentSerializerFactory} interface.
     *
     * @return  instance of the {@code DocumentSerializer} for this
     *          {@code ConverterInfo}.
     */
    public DocumentDeserializerFactory getDocDeserializerFactory() {
        return (DocumentDeserializerFactory)piPluginFactory;
    }

    /**
     * Returns an instance of the DocumentMergerFactory interface.
     *
     * @return  instance of the {@code DocumentMergerFactory} for this
     *          {@code ConverterInfo}.
     */
    public DocumentMergerFactory getDocMergerFactory() {
        return (DocumentMergerFactory)piPluginFactory;
    }

    /**
     * Returns the jar file name.
     *
     * @return  The jar file name, {@code null} if none exists.
     */
    public String getJarName() {
        return piJarName;
    }

    /**
     * Returns the office mime-type.
     *
     * @return  The office mime-type, {@code null} if none exists.
     */
    public String getOfficeMime() {
        return piOfficeMime;
    }

    /**
     * Returns an {@code Enumeration} of {@code String} objects indicating the
     * device mime-type.
     *
     * @return  An {@code Enumeration} of {@code String} objects indicating the
     *          device mime-type.
     */
    public Iterator<String> getDeviceMime() {
        return piDeviceMime.iterator();
    }

    /**
     * Returns the display name.
     *
     * @return  The display name, {@code null} if none exists.
     */
    public String getDisplayName() {
        return piDisplayName;
    }

    /**
     * Returns the description.
     *
     * @return  The description, {@code null} if none exists.
     */
    public String getDescription() {
        return piDescription;
    }

    /**
     * Returns the version.
     *
     * @return  The version, {@code null} if none exists.
     */
    public String getVersion() {
        return piVersion;
    }

    /**
     * Returns the vendor name.
     *
     * @return  The vendor name, {@code null} if none exists.
     */
    public String getVendor() {
        return piVendor;
    }

    /**
     * Returns the implementation class name of PluginFactory.
     *
     * @return  The implementation class name of {@code PluginFactory},
     *          {@code null} if none exists.
     */
    public String getClassImpl() {
        return piClassImpl;
    }

    /**
     * Returns the {@code PluginFactory} instance for this plug-in.
     *
     * @return  The {@code PluginFactory} instance for this plug-in.
     */
    public PluginFactory getPluginFactory() {
        return piPluginFactory;
    }

    /**
     * Returns {@code true} if this plug-in has a serializer, {@code false}
     * otherwise.
     *
     * @return  {@code true} if this plug-in has a serializer, {@code false}
     *          otherwise.
     */
    public boolean canSerialize() {
        return piCanSerialize;
    }

    /**
     * Returns {@code true} if this plug-in has a deserializer, {@code false}
     * otherwise.
     *
     * @return  {@code true} if this plug-in has a deserializer, {@code false}
     *          otherwise.
     */
    public boolean canDeserialize() {
        return piCanDeserialize;
    }

    /**
     * Returns {@code true} if this plug-in has a merger, {@code false}
     * otherwise.
     *
     * @return  {@code true} if this plug-in has a merger, {@code false}
     *          otherwise.
     */
    public boolean canMerge() {
        return piCanMerge;
    }

    /**
     * Returns {@code true} if the officeMime is a valid Office mime type.
     *
     * @return  {@code true} if the officeMime is a valid Office mime type.
     */
    public static boolean isValidOfficeType(String officeMime) {

        boolean rc = false;
        for (String validOfficeType : validOfficeTypes) {
            if (officeMime.equals(validOfficeType)) {
                rc = true;
            }
        }

        return rc;
    }

    /**
     * Returns a {@code String} containing the Xslt stylesheet URL that is to be
     * used by the Xslt Plug-in Serializer.
     *
     *  @return  {@code String}.
     */
    public String getXsltSerial() {
        return piXsltSerial;
    }

    /**
     * Returns a {@code String} containing the xslt stylesheet URL that is to be
     * used by the Xslt Plug-in Deserializer.
     *
     *  @return  {@code String}.
     */
    public String getXsltDeserial() {
        return piXsltDeserial;
    }
}
