/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ClassContextImpl.java,v $
 * $Revision: 1.14 $
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

package com.sun.star.lib.sandbox;

import java.awt.Toolkit;
import java.awt.Image;

import java.io.IOException;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;

import java.util.Hashtable;

import java.util.zip.ZipEntry;

final class ClassContextImpl extends ClassLoader implements ClassContext {
    private static int instances;
    private static final boolean DEBUG = false;

    private URL codeBase;
    private ProtectionDomain protectionDomain;
    private boolean _bSecure;
    private java.util.Vector _cargoVector = new java.util.Vector();

    private java.util.Vector m_class_path_jars;

    // HACKHACK!!!: java.lang.Thread fragt bei erzeugen eines Threads den
    // HACKHACK!!!: SECURITYMANAGER nach der THREADGROUP
    private ThreadGroup threadGroup;

    public ClassContextImpl(URL codeBase, ProtectionDomain protectionDomain, ThreadGroup threadGroup, boolean bSecure) {
        this(codeBase, protectionDomain, threadGroup);

        _bSecure = bSecure;
    }

    public ClassContextImpl(URL codeBase, ProtectionDomain protectionDomain, ThreadGroup threadGroup) {
        if(DEBUG)System.err.println("#### ClassContextImpl.<init>:" + codeBase + " " + protectionDomain + " " + threadGroup);

        instances ++;

        this.codeBase = codeBase;
        this.protectionDomain = protectionDomain;
        this.threadGroup = threadGroup;
    }

    public void finalize() {
        instances --;
    }

    public URL getBase() {
        return codeBase;
    }

    public void addCargo( Object cargo ) {
        _cargoVector.addElement( cargo );
    }

    // For OS/2
    public URL getCodeBase()
    {
      return getBase();
    }

    public boolean hasThreadGroup() {
        return threadGroup != null && !threadGroup.isDestroyed();
    }

    public ThreadGroup getThreadGroup() {
        if(threadGroup == null || threadGroup.isDestroyed()) {
            threadGroup = new ThreadGroup(codeBase.toString());
            threadGroup.setDaemon(true);
        }

        return threadGroup;
    }

    boolean checkSecurity() {
        return !_bSecure;
    }

    public InputStream getResourceAsStream(String name) {
          if(DEBUG) System.err.println("#### ClassContext.getResourceAsStream:" + name);

        InputStream inputStream = getSystemResourceAsStream(name);
        if(inputStream == null) {
            try {
                URL url = new URL(codeBase, name);
                if(ResourceProxy.isResource(url)) { // VALID URL?
                    ResourceProxy resourceProxy = ResourceProxy.load(url, protectionDomain);
                    inputStream = resourceProxy.getInputStream();
                }
            }
            catch(MalformedURLException me) {
            }
            catch(IOException ioe) {
            }
        }

        return inputStream;
    }

    public URL getResource(String name) {
          if(DEBUG) System.err.println("#### ClassContext.getResource:" + name);

        URL url = getSystemResource(name);
        if(url == null) {
            try {
                url = new URL(codeBase, name);
                if(!ResourceProxy.isResource(url)) // VALID URL?
                    url = null;
                else
                    url = new URL("appletresource:" + codeBase + "/+/" + name);
            }
            catch(MalformedURLException malformedURLException) {
                if(DEBUG) System.err.println("#### ClassContext.getResource - exception: " + malformedURLException);
            }
        }

        return url;
    }

    public Class findClass(String className) throws ClassNotFoundException {
        Class xClass = findLoadedClass(className);

        // It is a nasty hack to test if want to generate
        // a proxy, but exception throw here sometimes
        // kills the java vm (jdk1.2.2)
        if(xClass == null && !className.startsWith("JSGen"))
            xClass = getClass().forName(className);

        // see above, throwing the exception here
        // kills sometimes the javavm
//          if(xClass == null) {
//              throw new ClassNotFoundException();
//          }

        return xClass;
    }

    public Class loadClass(String className) throws ClassNotFoundException {
          return loadClass(className, true);
    }

    synchronized protected Class loadClass(String className, boolean resolve) throws ClassNotFoundException {
          if(DEBUG) System.err.println("#### ClassContext.loadClass:" + className + " "  + resolve);

        // ???SECURITY????
        SecurityManager security = System.getSecurityManager();
        if(security != null) {
            int i = className.lastIndexOf('.');
            if(i >= 0)
                security.checkPackageAccess(className.substring(0, i));
        }


        Class xClass = null;

        try {
            xClass = findClass(className);
        }
        catch(ClassNotFoundException e) {
        }

        if(xClass == null)
        {
            try
            {
                try
                {
                    ResourceProxy resourceProxy = null;
                    resourceProxy =
                        ResourceProxy.load(
                            new URL( codeBase, className.replace('.', '/') + ".class" ), protectionDomain );
                    byte bytes[] = resourceProxy.getBytes();
                    //                      if(DEBUG) printHeader(bytes);

                    xClass = defineClass(className, bytes, 0, bytes.length);
                    //                      xClass = defineClass(className, bytes, 0, bytes.length, protectionDomain);
                }
                catch (IOException exc) // if not found
                {
                    // try further Class-Path jars
                    if (null == m_class_path_jars)
                    {
                        java.util.Vector class_path_jars = new java.util.Vector();
                        try
                        {
                            java.net.URL manifest_url = new java.net.URL(
                                codeBase, "META-INF/MANIFEST.MF" );
                            // read Class-Path from manifest file
                            ResourceProxy resource= ResourceProxy.load( manifest_url, null );
                            java.io.InputStream inManifest = resource.getInputStream();
                            java.util.jar.Manifest manifest =
                                new java.util.jar.Manifest( inManifest );
                            java.util.jar.Attributes attributes = manifest.getMainAttributes();
                            String class_path = attributes.getValue( "Class-Path" );

                            if (class_path != null)
                            {
                                java.util.Enumeration tokens =
                                    new java.util.StringTokenizer( class_path );
                                while (tokens.hasMoreElements())
                                {
                                    try
                                    {
                                        java.net.URL url;
                                        String str_url = (String)tokens.nextElement();
                                        if (str_url.charAt( 0 ) != '/' &&
                                            str_url.indexOf( ':' ) < 0)
                                        {
                                            // relative path
                                            url = new java.net.URL( codeBase, str_url );
                                        }
                                        else
                                        {
                                            url = new java.net.URL( str_url );
                                        }
                                        ClassContext context =
                                            ClassContextProxy.create(
                                                url, protectionDomain, threadGroup, true );
                                        Resource res = ResourceProxy.load(url, null);
                                        res.loadJar(url);
                                        context.addCargo( resource );
                                        class_path_jars.add( context );
                                    }
                                    catch (MalformedURLException e) // ignoring
                                    {
                                    }
                                }
                            }
                        }
                        catch (IOException e2)
                        {
                        }
                        m_class_path_jars = class_path_jars;
                    }

                    java.util.Enumeration enum_elements = m_class_path_jars.elements();
                    while (enum_elements.hasMoreElements())
                    {
                        ClassContext context = (ClassContext)enum_elements.nextElement();
                        try
                        {
                            xClass = context.loadClass( className );
                        }
                        catch (ClassNotFoundException e) // if not found, try next
                        {
                        }
                    }
                    if (null == xClass)
                    {
                        throw new ClassNotFoundException(
                            "ClassContext.loadClass - class not found: "
                            + className + " " + codeBase );
                    }
                }

//                  Object objects[] = new Object[2];
//                  objects[0] = resourceProxy.getProtectionDomain();
//                  setSigners(xClass, objects);
            }
            catch(ClassFormatError classFormatError) {
                if(DEBUG) System.err.println("#### ClassContext.loadClass - ClassFormat exception:" + classFormatError);
                throw new ClassNotFoundException("ClassContext.loadClass - ClassFormatError:"
                                                 + " " + classFormatError
                                                 + " " + className
                                                 + " " + codeBase);
            }
        }

        if (xClass != null && resolve)
            resolveClass(xClass);

        return xClass;
    }

    static class ThreadGroupKiller implements Runnable {
        ThreadGroup threadGroup = null;

        ThreadGroupKiller(ThreadGroup threadGroup) {
            this.threadGroup = threadGroup;
        }

        public void run() {
            try {
                if (DEBUG) System.err.println("#### ClassContext - killerThread start");
                threadGroup.stop();
                threadGroup.destroy();
                if (DEBUG) System.err.println("#### ClassContext - killerThread succeeded");
            }
            catch(Exception exception) {
                if (DEBUG) System.err.println("ClassContext.dispose:" + exception);
            }
        }
    };

    synchronized public void dispose() {
        if(DEBUG)System.err.println("#### ClassContext.dispose:" + threadGroup);

        if(threadGroup != null) {
            threadGroup.list();

              new Thread(new ThreadGroupKiller(threadGroup), codeBase.toString() + " killer thread").start();

            threadGroup = null;
        }
    }

    private void printHeader(byte bytes[]) {
        System.err.print("#### ClassContext.loadClass - bytes header:");
        for(int x =0 ; x < 4; x++) {
            System.err.print(" " + Integer.toHexString(((int)bytes[x]) & 0xff));
        }
        System.err.println();
    }

    public ClassLoader getClassLoader() {
        return this;
    }
}

