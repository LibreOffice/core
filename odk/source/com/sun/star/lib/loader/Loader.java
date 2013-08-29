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

package com.sun.star.lib.loader;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Method;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Enumeration;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.StringTokenizer;
import java.util.ArrayList;

/**
 * This class can be used as a loader for application classes which use UNO.
 *
 * <p>The Loader class detects a UNO installation on the system and adds the
 * UNO jar files to the search path of a customized class loader, which is used
 * for loading the application classes.</p>
 */
public final class Loader {

    private static ClassLoader m_Loader = null;

    /**
     * do not instantiate
     */
    private Loader() {}

    /**
     * The main method instantiates a customized class loader with the
     * UNO jar files added to the search path and loads the application class,
     * which is specified in the Main-Class attribute of the
     * com/sun/star/lib/Loader.class entry of the manifest file or
     * as first parameter in the argument list.
     */
    public static void main( String[] arguments ) throws Exception {

        // get the name of the class to be loaded from the manifest
        String className = null;
        Class clazz = Loader.class;
        ClassLoader loader = clazz.getClassLoader();
        ArrayList<URL> res = new ArrayList<URL>();
        try {
            Enumeration<URL> en = loader.getResources( "META-INF/MANIFEST.MF" );
            while ( en.hasMoreElements() ) {
                res.add( en.nextElement() );
            }
            // the jarfile with the com/sun/star/lib/loader/Loader.class
            // per-entry attribute is most probably the last resource in the
            // list, therefore search backwards
            for ( int i = res.size() - 1; i >= 0; i-- ) {
                URL jarurl = res.get( i );
                try {
                    JarURLConnection jarConnection =
                        (JarURLConnection) jarurl.openConnection();
                    Manifest mf = jarConnection.getManifest();
                    Attributes attrs = mf.getAttributes(
                        "com/sun/star/lib/loader/Loader.class" );
                    if ( attrs != null ) {
                        className = attrs.getValue( "Application-Class" );
                        if ( className != null )
                            break;
                    }
                } catch ( IOException e ) {
                    // if an I/O error occurs when opening a new
                    // JarURLConnection, ignore this manifest file
                    System.err.println( "com.sun.star.lib.loader.Loader::" +
                                        "main: bad manifest file: " + e );
                }
            }
        } catch ( IOException e ) {
            // if an I/O error occurs when getting the manifest resources,
            // try to get the name of the class to be loaded from the argument
            // list
            System.err.println( "com.sun.star.lib.loader.Loader::" +
                                "main: cannot get manifest resources: " + e );
        }

        // if no manifest entry was found, get the name of the class
        // to be loaded from the argument list
        String[] args;
        if ( className == null ) {
            if ( arguments.length > 0 ) {
                className = arguments[0];
                args = new String[arguments.length - 1];
                System.arraycopy( arguments, 1, args, 0, args.length );
            } else {
                throw new IllegalArgumentException(
                    "The name of the class to be loaded must be either " +
                    "specified in the Main-Class attribute of the " +
                    "com/sun/star/lib/loader/Loader.class entry " +
                    "of the manifest file or as a command line argument." );
            }
        } else {
            args = arguments;
        }

        // load the class with the customized class loader and
        // invoke the main method
        if ( className != null ) {
            ClassLoader cl = getCustomLoader();
            Thread.currentThread().setContextClassLoader(cl);
            Class c = cl.loadClass( className );
            Method m = c.getMethod( "main", new Class[] { String[].class } );
            m.invoke( null, new Object[] { args } );
        }
    }

    /**
     * Gets the customized class loader with the UNO jar files added to the
     * search path.
     *
     * @return the customized class loader
     */
    public static synchronized ClassLoader getCustomLoader() {
        if ( m_Loader == null ) {

            // get the urls from which to load classes and resources
            // from the class path
            ArrayList<URL> vec = new ArrayList<URL>();
            String classpath = null;
            try {
                classpath = System.getProperty( "java.class.path" );
            } catch ( SecurityException e ) {
                // don't add the class path entries to the list of class
                // loader URLs
                System.err.println( "com.sun.star.lib.loader.Loader::" +
                    "getCustomLoader: cannot get system property " +
                    "java.class.path: " + e );
            }
            if ( classpath != null ) {
                addUrls(vec, classpath, File.pathSeparator);
            }

            // get the urls from which to load classes and resources
            // from the UNO installation
            String path = InstallationFinder.getPath();
            if ( path != null ) {
                callUnoinfo(path, vec);
            } else {
                System.err.println( "com.sun.star.lib.loader.Loader::" +
                    "getCustomLoader: no UNO installation found!" );
            }

            // copy urls to array
            URL[] urls = new URL[vec.size()];
            vec.toArray( urls );

            // instantiate class loader
            m_Loader = new CustomURLClassLoader( urls );
        }

        return m_Loader;
    }

    private static void addUrls(ArrayList<URL> urls, String data, String delimiter) {
        StringTokenizer tokens = new StringTokenizer( data, delimiter );
        while ( tokens.hasMoreTokens() ) {
            try {
                urls.add( new File( tokens.nextToken() ).toURI().toURL() );
            } catch ( MalformedURLException e ) {
                // don't add this class path entry to the list of class loader
                // URLs
                System.err.println( "com.sun.star.lib.loader.Loader::" +
                    "getCustomLoader: bad pathname: " + e );
            }
        }
    }

    private static void callUnoinfo(String path, ArrayList<URL> urls) {
        Process p;
        try {
            p = Runtime.getRuntime().exec(
                new String[] { new File(path, "unoinfo").getPath(), "java" });
        } catch (IOException e) {
            System.err.println(
                "com.sun.star.lib.loader.Loader::getCustomLoader: exec" +
                " unoinfo: " + e);
            return;
        }
        new Drain(p.getErrorStream()).start();
        int code;
        byte[] buf = new byte[1000];
        int n = 0;
        try {
            InputStream s = p.getInputStream();
            code = s.read();
            for (;;) {
                if (n == buf.length) {
                    if (n > Integer.MAX_VALUE / 2) {
                        System.err.println(
                            "com.sun.star.lib.loader.Loader::getCustomLoader:" +
                            " too much unoinfo output");
                        return;
                    }
                    byte[] buf2 = new byte[2 * n];
                    for (int i = 0; i < n; ++i) {
                        buf2[i] = buf[i];
                    }
                    buf = buf2;
                }
                int k = s.read(buf, n, buf.length - n);
                if (k == -1) {
                    break;
                }
                n += k;
            }
        } catch (IOException e) {
            System.err.println(
                "com.sun.star.lib.loader.Loader::getCustomLoader: reading" +
                " unoinfo output: " + e);
            return;
        }
        int ev;
        try {
            ev = p.waitFor();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.err.println(
                "com.sun.star.lib.loader.Loader::getCustomLoader: waiting for" +
                " unoinfo: " + e);
            return;
        }
        if (ev != 0) {
            System.err.println(
                "com.sun.star.lib.loader.Loader::getCustomLoader: unoinfo"
                + " exit value " + n);
            return;
        }
        String s;
        if (code == '0') {
            s = new String(buf);
        } else if (code == '1') {
            try {
                s = new String(buf, "UTF-16LE");
            } catch (UnsupportedEncodingException e) {
                System.err.println(
                    "com.sun.star.lib.loader.Loader::getCustomLoader:" +
                    " transforming unoinfo output: " + e);
                return;
            }
        } else {
            System.err.println(
                "com.sun.star.lib.loader.Loader::getCustomLoader: bad unoinfo"
                + " output");
            return;
        }
        addUrls(urls, s, "\0");
    }

    private static final class Drain extends Thread {
        public Drain(InputStream stream) {
            super("unoinfo stderr drain");
            this.stream = stream;
        }

        public void run() {
            try {
                while (stream.read() != -1) {}
            } catch (IOException e) { /* ignored */ }
        }

        private final InputStream stream;
    }

    /**
     * A customized class loader which is used to load classes and resources
     * from a search path of user-defined URLs.
     */
    private static final class CustomURLClassLoader extends URLClassLoader {

        public CustomURLClassLoader( URL[] urls ) {
            super( urls );
        }

        protected Class findClass( String name ) throws ClassNotFoundException {
            // This is only called via this.loadClass -> super.loadClass ->
            // this.findClass, after this.loadClass has already called
            // super.findClass, so no need to call super.findClass again:
            throw new ClassNotFoundException( name );
        }

        protected Class loadClass( String name, boolean resolve )
            throws ClassNotFoundException
        {
            Class c = findLoadedClass( name );
            if ( c == null ) {
                try {
                    c = super.findClass( name );
                } catch ( ClassNotFoundException e ) {
                    return super.loadClass( name, resolve );
                } catch ( SecurityException e ) {
                    // A SecurityException "Prohibited package name: java.lang"
                    // may occur when the user added the JVM's rt.jar to the
                    // java.class.path:
                    return super.loadClass( name, resolve );
                }
            }
            if ( resolve ) {
                resolveClass( c );
            }
            return c;
        }
    }
}
