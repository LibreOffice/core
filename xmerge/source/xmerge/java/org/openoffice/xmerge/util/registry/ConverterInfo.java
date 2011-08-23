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

package org.openoffice.xmerge.util.registry;

import java.util.Vector;
import java.util.Enumeration;
import java.net.URL;
import java.net.URLClassLoader;
import java.lang.reflect.Constructor;
import org.openoffice.xmerge.PluginFactory;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.DocumentMergerFactory;

/**
 *  Class for storing the information about a converter plugin.
 *
 *  @author  Brian Cameron
 */
public class ConverterInfo {

    /** 
     *  Keep track of the valid Office mime types
     */
    private static String[] validOfficeTypes;
    public static String SxwType = "staroffice/sxw";
    public static String SxcType = "staroffice/sxc";


    static {
       // This needs to be updated to reflect all valid office types.
       //
       validOfficeTypes = new String[2];
       validOfficeTypes[0] = SxwType;
       validOfficeTypes[1] = SxcType;
    }

    private String           piJarName;
    private String           piOfficeMime;
    private Vector           piDeviceMime;
    private String           piDisplayName;
    private String           piDescription;
    private String           piVersion;
    private String           piVendor;
    private String           piClassImpl;
    private String           piXsltSerial;
    private String           piXsltDeserial;
    private boolean          piCanSerialize      = false;
    private boolean          piCanDeserialize    = false;
    private boolean          piCanMerge          = false;
    private ClassLoader      piClassLoader       = null;
    private PluginFactory    piPluginFactory;


    /**
     *  The constructor builds a ConverterInfo structure.
     *
     *  @param  jarName      The URL of the jarfile.
     *  @param  officeMime   The office mime-type.
     *  @param  deviceMime   The device mime-type.
     *  @param  displayName  The display name.
     *  @param  description  The description.
     *  @param  version      The version.
     *  @param  vendor       The vendor name.
     *  @param  impl         The implementation class name of
     *                       PluginFactory.
     *  @param  xsltSerial   The url of the serializer xsl stylesheet
     *  @param  xsltDeserial The url of the deserializer xsl stylesheet 
     *
     *  @throws RegistryException   If <code>ci</code> cannot 
     *                              be loaded.
     */
    public ConverterInfo(String jarName, String officeMime,
        Vector deviceMime, String displayName, String description,
    String version, String vendor, String impl,String xsltSerial,
    String xsltDeserial)
        throws RegistryException {

        if (isValidOfficeType(officeMime.trim()) == false) {
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
    piXsltSerial  = xsltSerial.trim();
    piXsltDeserial= xsltDeserial.trim();
        piClassImpl   = impl.trim();
        piClassLoader = this.getClass().getClassLoader();

        // Get instance of the PluginFactory.
        //
        try {
            URL jarURL = new URL(jarName);
            URLClassLoader loader = new URLClassLoader(new URL[] { jarURL },
               piClassLoader);
            Class clas = loader.loadClass(piClassImpl);
            Class[] argumentTypes = { org.openoffice.xmerge.util.registry.ConverterInfo.class };
            Constructor construct = clas.getConstructor(argumentTypes);

            Object[] arguments = { this };
            piPluginFactory = ( PluginFactory ) construct.newInstance(arguments);

            // See which interfaces the plug-in PluginFactory supports.
            //
            Class[] cl = piPluginFactory.getClass().getInterfaces();
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
            RegistryException re = new RegistryException(
                "Class implementation of the plug-in cannot be loaded.");
            throw re;
        }
    }

     /**
     *  The constructor builds a ConverterInfo structure.
     *
     *  @param  jarName      The URL of the jarfile.
     *  @param  officeMime   The office mime-type.
     *  @param  deviceMime   The device mime-type.
     *  @param  displayName  The display name.
     *  @param  description  The description.
     *  @param  version      The version.
     *  @param  vendor       The vendor name.
     *  @param  impl         The implementation class name of
     *                       PluginFactory.
     *
     *  @throws RegistryException   If <code>ci</code> cannot 
     *                              be loaded.
     */


    public ConverterInfo(String jarName, String officeMime,
        Vector deviceMime, String displayName, String description,
        String version, String vendor, String impl)
        throws RegistryException {

        if (isValidOfficeType(officeMime.trim()) == false) {
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
        //
        try {
            URL jarURL = new URL(jarName);
            URLClassLoader loader = new URLClassLoader(new URL[] { jarURL },
               piClassLoader);
            Class clas = loader.loadClass(piClassImpl);
            Class[] argumentTypes = { org.openoffice.xmerge.util.registry.ConverterInfo.class };
            Constructor construct = clas.getConstructor(argumentTypes);

            Object[] arguments = { this };
            piPluginFactory = ( PluginFactory ) construct.newInstance(arguments);

            // See which interfaces the plug-in PluginFactory supports.
            //
            Class[] cl = piPluginFactory.getClass().getInterfaces();
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
            RegistryException re = new RegistryException(
                "Class implementation of the plug-in cannot be loaded.");
            throw re;
        }
    }




    /**
     *  Create a default constructor so we can use isValidOfficeType 
     *  without having to actually have a valid ConverterInfo.
     */
    private ConverterInfo() {
    }
    
    
    /**
     *  Returns an instance of the DocumentDeserializerFactory interface.
     *
     *  @return  instance of the DocumentDeserializer for this ConverterInfo.
     */
    public DocumentSerializerFactory getDocSerializerFactory() {
        return (DocumentSerializerFactory)piPluginFactory;
    }


    /**
     *  Returns an instance of the DocumentSerializerFactory interface.
     *
     *  @return  instance of the DocumentSerializer for this ConverterInfo.
     */
    public DocumentDeserializerFactory getDocDeserializerFactory() {
        return (DocumentDeserializerFactory)piPluginFactory;
    }


    /**
     *  Returns an instance of the DocumentMergerFactory interface.
     *
     *  @return  instance of the DocumentMergerFactory for this ConverterInfo.
     */
    public DocumentMergerFactory getDocMergerFactory() {
        return (DocumentMergerFactory)piPluginFactory;
    }


    /**
     *  Returns the jar file name.
     *
     *  @return  The jar file name, null if none exists.
     */
    public String getJarName() {
        return piJarName;
    }

  
    /**
     *  Returns the office mime-type.
     *
     *  @return  The office mime-type, null if none exists.
     */
    public String getOfficeMime() {
        return piOfficeMime;
    }
  

    /**
     *  Returns an <code>Enumeration</code> of <code>String</code>
     *  objects indicating the device mime-type.
     *
     *  @return  An <code>Enumeration</code> of <code>String</code>
     *           objects indicating the device mime-type.
     */
    public Enumeration getDeviceMime() {
        return(piDeviceMime.elements());
    }
  

    /**
     *  Returns the display name.
     *
     *  @return  The display name, null if none exists.
     */
    public String getDisplayName() {
        return piDisplayName;
    }

  
    /**
     *  Returns the description.
     *
     *  @return  The description, null if none exists.
     */
    public String getDescription() {
        return piDescription;
    }
  

    /**
     *  Returns the version.
     *
     *  @return  The version, null if none exists.
     */
    public String getVersion() {
        return piVersion;
    }
  

    /**
     *  Returns the vendor name.
     *
     *  @return  The vendor name, null if none exists.
     */
    public String getVendor() {
        return piVendor;
    }

  
    /**
     *  Returns the implementation class name of PluginFactory.
     *
     *  @return  The implementation class name of PluginFactory,
     *           null if none exists.
     */
    public String getClassImpl() {
        return piClassImpl;
    }


    /**
     *  Returns the PluginFactory instance for this plug-in.
     *
     *  @return  The PluginFactory instance for this plug-in.
     */
    public PluginFactory getPluginFactory() {
        return piPluginFactory;
    }


    /**
     *  Returns true if this plug-in has a serializier, false otherwise.
     *
     *  @return  true if this plug-in has a serializier, false otherwise.
     */
    public boolean canSerialize() {
        return piCanSerialize;
    }


    /**
     *  Returns true if this plug-in has a deserializier, false otherwise.
     *
     *  @return  true if this plug-in has a deserializier, false otherwise.
     */
    public boolean canDeserialize() {
        return piCanDeserialize;
    }


    /**
     *  Returns true if this plug-in has a merger, false otherwise.
     *
     *  @return  true if this plug-in has a merger, false otherwise.
     */
    public boolean canMerge() {
        return piCanMerge;
    }


    /**
     *  Returns true if the officeMime is a valid Office mime type.
     *
     *  @return  true if the officeMime is a valid Office mime type.
     */
    public static boolean isValidOfficeType(String officeMime) {

        boolean rc = false;
        for (int i=0; i < validOfficeTypes.length; i++) {
            if (officeMime.equals(validOfficeTypes[i])) {
                rc = true;
            }
        }

       return rc;
    }

    /**
     *  Returns a <code>String</code> containing the Xslt stylesheet url that 
     *  is to be used by the Xslt Plugin Serializer.
     *
     *  @return  <code>String</code>
     */

     public String getXsltSerial() {
        return piXsltSerial;
    }

    /**  
     * Returns a <code>String</code> containing the xslt stylesheet url that 
     * is to be used by the Xslt Plugin Deserializer.
     *
     *  @return  <code>String</code>
     */

    public String getXsltDeserial() {
        return piXsltDeserial;
    }
}

