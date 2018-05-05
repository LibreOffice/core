/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
package com.sun.star.comp.sdbc;

import java.lang.ref.WeakReference;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.StringTokenizer;

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.CompHelper;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uri.UriReferenceFactory;
import com.sun.star.uri.XUriReferenceFactory;
import com.sun.star.uri.XVndSunStarExpandUrlReference;
import com.sun.star.util.XMacroExpander;

/**
 * A map from pairs of (classPath, name) to pairs of weak Java
 * references to (ClassLoader, Class) is maintained, so that a class is only
 * loaded once.
 *
 * It may happen that the weak reference to the ClassLoader becomes null while
 * the reference to the Class remains non-null (in case the Class was actually
 * loaded by some parent of the ClassLoader), in which case the ClassLoader is
 * resurrected (which cannot cause any classes to be loaded multiple times, as
 * the ClassLoader is no longer reachable, so no classes it has ever loaded are
 * still reachable).
 *
 * Similarly, it may happen that the weak reference to the Class becomes null
 * while the reference to the ClassLoader remains non-null, in which case the
 * Class is simply re-loaded.
 */
public class ClassMap {
    public static class ClassLoaderAndClass {
        private final ClassLoader classLoader;
        private final Class<?> classObject;

        public ClassLoaderAndClass(ClassLoader classLoader, Class<?> classObject) {
            this.classLoader = classLoader;
            this.classObject = classObject;
        }

        public ClassLoader getClassLoader() {
            return classLoader;
        }

        public Class<?> getClassObject() {
            return classObject;
        }
    }

    private static class ClassMapEntry {
        String classPath;
        String className;
        WeakReference<ClassLoader> classLoader;
        WeakReference<Class<?>> classObject;
    }

    private final LinkedList<ClassMapEntry> map = new LinkedList<>();

    public synchronized ClassLoaderAndClass loadClass(XComponentContext context, String classPath, String className)
            throws MalformedURLException, ClassNotFoundException {

        ClassLoader classLoader = null;
        Class<?> classObject = null;
        // Prune dangling weak references from the list while searching for a match,
        // so that the list cannot grow unbounded:
        ClassMapEntry matchingEntry = null;
        for (Iterator<ClassMapEntry> it = map.iterator(); it.hasNext();) {
            ClassMapEntry entry = it.next();
            classLoader = entry.classLoader.get();
            classObject = entry.classObject.get();
            if (classLoader == null && classObject == null) {
                it.remove();
            } else if (entry.classPath.equals(classPath) && entry.className.equals(className)) {
                matchingEntry = entry;
                break;
            }
        }
        if (classLoader == null || classObject == null) {
            if (matchingEntry == null) {
                // Push a new ClassMapEntry (which can potentially fail) before
                // loading the class, so that it never happens that a class is
                // loaded but not added to the map (which could have effects on the
                // JVM that are not easily undone).  If the pushed ClassMapEntry is
                // not used after all (return false, etc.) it will be pruned on next
                // call because its classLoader/classObject are null:
                matchingEntry = new ClassMapEntry();
                matchingEntry.classPath = classPath;
                matchingEntry.className = className;
                map.addFirst(matchingEntry);
            }
            if (classLoader == null) {
                List<URL> urls = translateToUrls(context, classPath);
                classLoader = new URLClassLoader(urls.toArray(new URL[0]));
            }
            if (classObject == null) {
                classObject = classLoader.loadClass(className);
            }
            matchingEntry.classLoader = new WeakReference<ClassLoader>(classLoader);
            matchingEntry.classObject = new WeakReference<Class<?>>(classObject);
        }
        return new ClassLoaderAndClass(classLoader, classObject);
    }

    private static List<URL> translateToUrls(XComponentContext context, String classPath) throws MalformedURLException {
        StringTokenizer tokenizer = new StringTokenizer(classPath, " ", false);
        ArrayList<URL> urls = new ArrayList<>();
        while (tokenizer.hasMoreTokens()) {
            String url = tokenizer.nextToken();
            XUriReferenceFactory uriReferenceFactory = null;
            XVndSunStarExpandUrlReference expUrl = null;
            XMacroExpander macroExpander = null;
            try {
                uriReferenceFactory = UriReferenceFactory.create(context);
                expUrl = UnoRuntime.queryInterface(
                        XVndSunStarExpandUrlReference.class, uriReferenceFactory.parse(url));
                if (expUrl != null) {
                    macroExpander = UnoRuntime.queryInterface(
                            XMacroExpander.class,
                            context.getValueByName("/singletons/com.sun.star.util.theMacroExpander"));
                    try {
                        url = expUrl.expand(macroExpander);
                    } catch (IllegalArgumentException illegalArgumentException) {
                        throw new RuntimeException(
                                "com.sun.star.lang.IllegalArgumentException: " + illegalArgumentException.getMessage(),
                                illegalArgumentException);
                    }
                }
            } finally {
                CompHelper.disposeComponent(uriReferenceFactory);
                CompHelper.disposeComponent(expUrl);
                CompHelper.disposeComponent(macroExpander);
            }
            URL javaURL = new URL(url);
            urls.add(javaURL);
        }
        return urls;
    }
}
