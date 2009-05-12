/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ClassContextProxy.java,v $
 * $Revision: 1.5 $
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

public final class ClassContextProxy implements ClassContext, Cachable {
    private static int instances;
    private static final boolean DEBUG = false;

    /*
    ** interface cachable methods
    */
    private ClassContext classContext;
    private WeakRef weakRef;

    public ClassContextProxy() {
        instances ++;
    }

    public Object getHardObject() {
        return classContext;
    }

    public void setWeakRef(WeakRef weakRef) {
        classContext = (ClassContext)weakRef.getRef();
        base = classContext.getBase();

        this.weakRef = weakRef;
        weakRef.incRefCnt();
    }

    public void finalize() {
        weakRef.decRefCnt();

        instances --;
    }

    /*
    ** ClassContext methods
    */
    private URL base;


    public static ClassContext create(URL base, ProtectionDomain protectionDomain, ThreadGroup threadGroup) {
        return create(base, protectionDomain, threadGroup, false);
    }

    public static ClassContext create(URL base, ProtectionDomain protectionDomain, ThreadGroup threadGroup, boolean bSecure) {
        Object object = WeakTable.get("ClassContxt: " + base);
        if(DEBUG) System.err.println("#### ClassContextProxy.create:" + object);
        ClassContextProxy classContextProxy = (ClassContextProxy)object;

        if(classContextProxy == null) {
            classContextProxy = new ClassContextProxy(base, protectionDomain, threadGroup, bSecure);
            WeakTable.put("ClassContxt: " + base, classContextProxy);
        }

        return classContextProxy;
    }

    private ClassContextProxy(URL base, ProtectionDomain protectionDomain, ThreadGroup threadGroup, boolean bSecure) {
        this();

        this.base = base;

        classContext = new ClassContextImpl(base, protectionDomain, threadGroup, bSecure);
    }

    public ClassLoader getClassLoader() {
        return classContext.getClassLoader();
    }

    public boolean hasThreadGroup() {
        return classContext.hasThreadGroup();
    }

    public ThreadGroup getThreadGroup() {
        return classContext.getThreadGroup();
    }

    public URL getBase() {
        return classContext.getBase();
    }

    public InputStream getResourceAsStream(String name) {
        return classContext.getResourceAsStream(name);
    }

    public URL getResource(String name) {
        return classContext.getResource(name);
    }

    public void addCargo( Object cargo ) {
        classContext.addCargo( cargo );
    }

    public void dispose() { // HIER SOLLTE VIELLEICHT NOCH MEHR GETAN WERDEN?
        classContext.dispose();
    }

    public synchronized Class findClass(String className) throws ClassNotFoundException {
        return classContext.findClass(className);
    }

    public synchronized Class loadClass(String className) throws ClassNotFoundException {
        return classContext.loadClass(className);
    }
}

