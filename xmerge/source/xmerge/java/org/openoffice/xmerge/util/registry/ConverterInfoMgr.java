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

import java.util.*;
import java.io.*;

/**
 *  Manages the converter plug-ins that are currently active.
 *  This class allows plug-ins to be added or removed dynamically.
 *  This class is a singleton (static) class, so that only one
 *  manager can exist at a time.  It is final, so it may not be
 *  subclassed.
 *
 */
public final class ConverterInfoMgr {

    private static ArrayList<ConverterInfo> converterInfoList;

   /**
    *  Constructor
    */
    static {
       converterInfoList = new ArrayList<ConverterInfo>();
    }

   /**
    *  Adds a converter plug-in to the registry.  The
    *  <code>ConverterInfo</code> must have a unique DisplayName
    *  and must have non-null values for DisplayName, ClassImpl,
    *  OfficeMime, and DeviceMime.
    *
    *  @param  ci  A <code>ConverterInfo</code> object describing a plug-in.
    *
    *  @throws  RegistryException  If the <code>ConverterInfo</code>
    *                              is not valid.
    */
    public static void addPlugIn(ConverterInfo ci) throws RegistryException {

        ConverterInfo converterInfo;

        // Validate
        //
        if (ci.getDisplayName() == null) {
            RegistryException re = new RegistryException(
                "Converter must have valid name.");
            throw re;
        }
        if (ci.getClassImpl() == null) {
            RegistryException re = new RegistryException(
                "Converter must have valid class implementation specified.");
            throw re;
        }
        if (ci.getOfficeMime() == null) {
            RegistryException re = new RegistryException(
                "Converter must have valid office mime specified.");
            throw re;
        }
        if (! ci.getDeviceMime().hasNext()) {
            RegistryException re = new RegistryException(
                "Converter must have valid device mime specified.");
            throw re;
        }

        // Verify there is no converter with the same Display Name in
        // the registry.
        //
        Iterator<ConverterInfo> ciEnum = converterInfoList.iterator();
        while (ciEnum.hasNext()) {
            converterInfo = ciEnum.next();
            if (ci.getDisplayName().equals(converterInfo.getDisplayName())) {
                RegistryException re = new RegistryException(
                    "Converter with specified display name already exists.");
                throw re;
            }
        }

        // Since this is a adding to a static Vector, make sure this
        // add method call is synchronized.
        //
        synchronized (converterInfoList) {
            converterInfoList.add(ci);
        }
    }

   /**
    *  Adds a list of converter plug-ins to the registry.
    *  Each <code>ConverterInfo</code> in the list must have
    *  a unique DisplayName and must have non-null values for DisplayName,
    *  ClassImpl, OfficeMime, and DeviceMime.
    *
    *  @param  jarEnum  An <code>Enumeration</code> of <code>ConverterInfo</code>
    *                     objects describing one or more plug-in(s).
    *
    *  @throws  RegistryException  If a <code>ConverterInfo</code> in the
    *                              <code>Vector</code> is not valid.
    */
    public static void addPlugIn(Iterator<ConverterInfo> jarEnum) throws RegistryException {

        while (jarEnum.hasNext()) {
            ConverterInfo converterInfo = jarEnum.next();
            addPlugIn(converterInfo);
        }
    }

   /**
    *  Returns an <code>Enumeration</code> of registered
    *  <code>ConverterInfo</code> objects.
    *
    *  @return  An <code>Enumeration</code> containing the currently registered
    *           <code>ConverterInfo</code> objects, an empty
    *           <code>Vector</code> if none exist.
    */
   public static Iterator<ConverterInfo> getConverterInfoEnumeration() {
      return (converterInfoList.iterator());
   }

   /**
    *  Removes any <code>ConverterInfo</code> object from the registry
    *  that have the specified jar name value.
    *
    *  @param  jar  The name of the jarfile.
    *
    *  @return  True if a <code>ConverterInfo</code> object was
    *           removed, false otherwise.
    */
   public static boolean removeByJar(String jar) {

        ConverterInfo converterInfo;
        boolean       rc = false;

        // FIX (HJ): Has to use an iterator, since we are removing items
        /*Enumeration ciEnum = converterInfoList.elements();
        while (ciEnum.hasMoreElements())
        {
            converterInfo = (ConverterInfo)ciEnum.nextElement();
            if (jar.equals(converterInfo.getJarName())) {
               converterInfoList.remove(converterInfo);
               rc = true;
            }
        }*/

        Iterator<ConverterInfo> ciIter = converterInfoList.iterator();
        while (ciIter.hasNext())
        {
            converterInfo = ciIter.next();
            if (jar.equals(converterInfo.getJarName())) {
               ciIter.remove();
               rc = true;
            }
        }
        return rc;
    }

   /**
    *  Removes any <code>ConverterInfo</code> object from the registry
    *  that have the specified display name value.
    *
    *  @param  name  The display name.
    *
    *  @return  True if a <code>ConverterInfo</code> object was
    *           removed, false otherwise.
    */
   public static boolean removeByName(String name) {

        ConverterInfo converterInfo;
        boolean       rc = false;

        Iterator<ConverterInfo> ciEnum = converterInfoList.iterator();
        while (ciEnum.hasNext())
        {
            converterInfo = ciEnum.next();
            if (name.equals(converterInfo.getDisplayName())) {
               converterInfoList.remove(converterInfo);
               rc = true;
            }
        }
        return rc;
    }

   /**
    *  Returns the <code>ConverterInfo</code> object that supports
    *  the specified device/office mime type conversion.  If there
    *  are multiple <code>ConverterInfo</code> objects registered
    *  that support this conversion, only the first is returned.
    *
    *  @param  deviceMime  The device mime.
    *  @param  officeMime  The office mime.
    *
    *  @return  The first plug-in that supports the specified
    *           conversion.
    */
    public static ConverterInfo findConverterInfo(String deviceMime, String officeMime) {

        ConverterInfo converterInfo;

        if (deviceMime == null ||
            ConverterInfo.isValidOfficeType(officeMime) == false) {
            return null;
        }

        // Loop over elements comparing with deviceFromMime
        //
        Iterator<ConverterInfo> ciEnum = converterInfoList.iterator();
        while (ciEnum.hasNext()) {

            converterInfo = ciEnum.next();
            String toDeviceInfo = converterInfo.getOfficeMime();
            Iterator<String> fromEnum = converterInfo.getDeviceMime();

            // Loop over the deviceMime types.
            //
            while (fromEnum.hasNext()) {
                String fromDeviceInfo = fromEnum.next();
                if (deviceMime.trim().equals(fromDeviceInfo) &&
                    officeMime.trim().equals(toDeviceInfo)) {
                   return (converterInfo);
                }
            }
        }
        return null;
    }

   /**
    *  Returns an array of two <code>ConverterInfo</code> objects that
    *  can be chained to perform the specified mime type conversion.
    *  If there are multiple <code>ConverterInfo</code> objects that
    *  support this conversion, only the first is returned.
    *
    *  @param  deviceFromMime  The device from mime.
    *  @param  deviceToMime    The device to mime.
    *
    *  @return  An array of two <code>ConverterInfo</code> objects
    *           that can be chained to perform the specified
    *           conversion.
    */
    public static ConverterInfo[] findConverterInfoChain(String deviceFromMime, String deviceToMime) {

        if (deviceFromMime == null || deviceToMime == null) {
            return null;
        }

        ConverterInfo[] converterInfo = new ConverterInfo[2];

        // Loop over elements comparing with deviceFromMime
        //
        Iterator<ConverterInfo> cifEnum = converterInfoList.iterator();
        while (cifEnum.hasNext()) {

            converterInfo[0] = cifEnum.next();
            String fromOfficeInfo = converterInfo[0].getOfficeMime();
            Iterator<String> fromEnum = converterInfo[0].getDeviceMime();

            // Loop over the deviceMime types looking for a deviceFromMime
            // match.
            //
            while (fromEnum.hasNext()) {
                String fromDeviceInfo = fromEnum.next();

                if (deviceFromMime.trim().equals(fromDeviceInfo)) {

                    // Found a a match for deviceFrom.  Now loop over the
                    // elements comparing with deviceToMime
                    //
                    Iterator<ConverterInfo> citEnum = converterInfoList.iterator();
                    while (citEnum.hasNext()) {

                        converterInfo[1] = citEnum.next();
                        String toOfficeInfo = converterInfo[1].getOfficeMime();
                        Iterator<String> toEnum = converterInfo[1].getDeviceMime();

                        // Loop over deviceMime types looking for a
                        // deviceToMime match.
                        //
                        while (toEnum.hasNext()) {
                            String toDeviceInfo = toEnum.next();
                            if (deviceToMime.trim().equals(toDeviceInfo) &&
                                fromOfficeInfo.equals(toOfficeInfo)) {

                                // Found a match
                                //
                                return (converterInfo);
                            }
                        }
                    }
                }
            }
        }
        return null;
    }

    /**
     *  main to let the user specify what plug-ins to register from
     *  jarfiles and to display the currently registered plug-ins.
     *
     *  @param  args  Not used.
     */
    public static void main(String args[]) {

        ConverterInfoReader cir = null;
    boolean validate = false;
        InputStreamReader   isr = new InputStreamReader(System.in);
        BufferedReader      br  = new BufferedReader(isr);
        char                c   = ' ';

        boolean exitFlag = false;
        while (exitFlag == false) {

            System.out.println("\nMenu:");
            System.out.println("(L)oad plug-ins from a jar file");
            System.out.println("(D)isplay name unload");
            System.out.println("(J)ar name unload");
            System.out.println("(F)ind ConverterInfo");
            System.out.println("(C)ind ConverterInfo chain");
            System.out.println("(V)iew plug-ins");
        System.out.println("(T)oggle Validation");
            System.out.println("(Q)uit\n");

            try {
                c = br.readLine().toUpperCase().trim().charAt(0);
            } catch(Exception e) {
                System.out.println("Invalid entry");
                System.out.println("Error msg: " + e.getMessage());
                continue;
            }

            System.out.println("");

            // Quit
            //
            if (c == 'Q') {
                exitFlag = true;

            // Load by Jarfile
            //
            } else if (c == 'L') {

                System.out.println("Enter path to jarfile: ");
                try {
                    String jarname = br.readLine().trim();
                    cir = new ConverterInfoReader(jarname,validate);
                } catch (RegistryException e) {
                    System.out.println("Cannot load plug-in ConverterFactory implementation.");
                    System.out.println("Error msg: " + e.getMessage());
                } catch (Exception e) {
                    System.out.println("Error adding data to registry");
                    System.out.println("Error msg: " + e.getMessage());
                }

                if (cir != null) {
                    Iterator<ConverterInfo> jarInfoEnum = cir.getConverterInfoEnumeration();
                    try {
                        ConverterInfoMgr.addPlugIn(jarInfoEnum);
                    } catch (Exception e) {
                        System.out.println("Error adding data to registry");
                        System.out.println("Error msg: " + e.getMessage());
                    }
                }

            // Unload by Display Name or Jarfile
            //
        } else if (c == 'T') {
        if (validate== true){
            System.out.println("Validation switched off");
            validate=false;
        }else{
            System.out.println("Validation switched on");
            validate=true;
        }
            } else if (c == 'D' || c == 'J') {

                if (c == 'D') {
                   System.out.println("Enter display name: ");
                } else {
                   System.out.println("Enter path to jarfile: ");
                }

                try
                {
                    String name = br.readLine().trim();
                    boolean rc = false;

                    if (c == 'D') {
                        rc = ConverterInfoMgr.removeByName(name);
                    } else {
                        rc = ConverterInfoMgr.removeByJar(name);
                    }

                    if (rc == true) {
                        System.out.println("Remove successful.");
                    } else {
                        System.out.println("Remove failed.");
                    }

                } catch (Exception e) {
                    System.out.println("Error removing value from registry");
                    System.out.println("Error msg: " + e.getMessage());
                }

            // Find Office Mime
            //
            } else if (c == 'F' || c == 'C') {

                String findMimeOne = null;
                String findMimeTwo = null;

                if (c == 'F') {
                    System.out.println("Enter device mime: ");
                } else {
                    System.out.println("Enter device from mime: ");
                }

                try {
                    findMimeOne = br.readLine().trim();
                } catch (Exception e) {
                    System.out.println("Error adding data to registry");
                    System.out.println("Error msg: " + e.getMessage());
                }

                if (c == 'F') {
                    System.out.println("Enter office mime: ");
                } else {
                    System.out.println("Enter device to mime: ");
                }

                try {
                    findMimeTwo = br.readLine().trim();
                } catch (Exception e) {
                    System.out.println("Error adding data to registry");
                    System.out.println("Error msg: " + e.getMessage());
                }

                if (c == 'F') {
                    ConverterInfo foundInfo = ConverterInfoMgr.findConverterInfo(findMimeOne, findMimeTwo);
                    if (foundInfo != null) {
                        System.out.println("    Found ConverterInfo");
                        System.out.println("    DisplayName  : " + foundInfo.getDisplayName());
                    } else {
                        System.out.println("    Did not find ConverterInfo");
                    }
                } else {
                    ConverterInfo[] foundInfo = ConverterInfoMgr.findConverterInfoChain(findMimeOne,
                        findMimeTwo);
                    if (foundInfo[0] != null && foundInfo[1] != null ) {
                        System.out.println("    Found ConverterInfo Chain");
                        System.out.println("    DisplayName  : " + foundInfo[0].getDisplayName());
                        System.out.println("    DisplayName  : " + foundInfo[1].getDisplayName());
                    } else {
                        System.out.println("    Did not find ConverterInfo");
                    }
                }

            // View
            //
            } else if (c == 'V') {

                Iterator<ConverterInfo> ciEnum = ConverterInfoMgr.getConverterInfoEnumeration();

                int ciCnt = 0;
                while (ciEnum.hasNext())
                {
                    System.out.println("");
                    System.out.println("  Displaying converter number " + ciCnt);
                    ConverterInfo converterInfo = ciEnum.next();
                    System.out.println("    DisplayName  : " + converterInfo.getDisplayName());
                    System.out.println("    JarFile      : " + converterInfo.getJarName());
                    System.out.println("    Description  : " + converterInfo.getDescription());
                    System.out.println("    Version      : " + converterInfo.getVersion());
                    System.out.println("    OfficeMime   : " + converterInfo.getOfficeMime());
                    Iterator<String> fromEnum = converterInfo.getDeviceMime();
                    int feCnt = 1;
                    while (fromEnum.hasNext())
                    {
                        System.out.println("    DeviceMime   : (#" + feCnt + ") : " +
                            fromEnum.next());
                        feCnt++;
                    }
                    if (feCnt == 1) {
                        System.out.println("    DeviceMime   : None specified");
                    }

                    System.out.println("    Vendor       : " + converterInfo.getVendor());
                    System.out.println("    ClassImpl    : " + converterInfo.getClassImpl());
                    System.out.println("    XsltSerial   : " + converterInfo.getXsltSerial());
            System.out.println("    XsltDeserial : " + converterInfo.getXsltDeserial());
            System.out.println("    Serialize    : " + converterInfo.canSerialize());
                    System.out.println("    Deserialize  : " + converterInfo.canDeserialize());
                    System.out.println("    Merge        : " + converterInfo.canMerge());
                    ciCnt++;
                }

                if (ciCnt == 0) {
                    System.out.println("No converters registered");
                }
            } else {
                System.out.println("Invalid input");
            }
        }
    }
}
