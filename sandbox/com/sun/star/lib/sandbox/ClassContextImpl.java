/*************************************************************************
 *
 *  $RCSfile: ClassContextImpl.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
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

import sun.tools.jar.JarVerifierStream;

final class ClassContextImpl extends ClassLoader implements ClassContext {
    private static int instances;
    private static final boolean DEBUG = false;

    private URL codeBase;
    private ProtectionDomain protectionDomain;
    private boolean _bSecure;

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

    public boolean hasThreadGroup() {
        return threadGroup != null && !threadGroup.isDestroyed();
    }

    public synchronized ThreadGroup getThreadGroup() {
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
        if(xClass == null)
            xClass = findSystemClass(className);

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
            try {
                ResourceProxy resourceProxy =
                    ResourceProxy.load(new URL(codeBase, className.replace('.', '/') + ".class"), protectionDomain);

                byte bytes[] = resourceProxy.getBytes();
                //                      if(DEBUG) printHeader(bytes);

                xClass = defineClass(className, bytes, 0, bytes.length);
                //                      xClass = defineClass(className, bytes, 0, bytes.length, protectionDomain);

                Object objects[] = new Object[2];
                objects[0] = resourceProxy.getProtectionDomain();
                setSigners(xClass, objects);
            }
            catch(MalformedURLException malformedURLException) {
                if(DEBUG) System.err.println("#### ClassContext.loadClasss - URL exception:" + malformedURLException);
                throw new ClassNotFoundException("ClassContext.loadClass - MalformedURLException:"
                                                 + " " + malformedURLException
                                                 + " " + className
                                                 + " " + codeBase);
            }
            catch(IOException iOException) {
                if(DEBUG) System.err.println("#### ClassContext.loadClasss - IO exception:" + iOException);
                throw new ClassNotFoundException("ClassContext.loadClass - IOException:"
                                                 + " " + iOException
                                                 + " " + className
                                                 + " " + codeBase);
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
                System.err.println("#### ClassContext - killerThread start");
                threadGroup.stop();
                threadGroup.destroy();
                System.err.println("#### ClassContext - killerThread succeeded");
            }
            catch(Exception exception) {
                System.err.println("ClassContext.dispose:" + exception);
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

