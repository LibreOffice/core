//*************************************************************************
//
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
//*************************************************************************
package oooapplet;

import java.lang.reflect.Method;
import java.lang.reflect.Array;
import java.net.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import com.sun.star.comp.beans.*;
import java.applet.Applet;
import java.awt.Graphics;
import java.util.*;

public class OOoViewer extends Applet {

    private OOoBean oBean;

    static private CustomURLClassLoader m_loader;

    Object  m_objBean;

    public void init() {
        try {
            if (m_loader == null) {
                String s = getParameter("office");
                System.out.println("sun.awt.noxembed: " + System.getProperty("sun.awt.noxembed"));
                System.setProperty("sun.awt.xembedserver", "true");

                File f = new File(s);
                URL url = f.toURI().toURL();
                String officeURL = url.toString();
                URL[] arURL = new URL[] {
                    new URL(officeURL + "/program/classes/officebean.jar"),
                    new URL(officeURL + "/program/classes/jurt.jar"),
                    new URL(officeURL + "/program/classes/ridl.jar"),
                    new URL(officeURL + "/program/classes/unoil.jar"),
                    new URL(officeURL + "/program/classes/java_uno.jar"),
                    new URL(officeURL + "/program/classes/juh.jar")
                };
                m_loader = new CustomURLClassLoader(arURL);
                File fileProg = new File(s + "/program");
                m_loader.addResourcePath(fileProg.toURI().toURL());
            }
        } catch (MalformedURLException e) {
            e.printStackTrace();
        }
    }

    public void start() {
        try {
        Class beanClass = m_loader.loadClass("com.sun.star.comp.beans.OOoBean");
        m_objBean = beanClass.newInstance();
        setLayout(new BorderLayout());
        add((java.awt.Container)m_objBean, BorderLayout.CENTER);
        setVisible(true);
        //this does not work here. Why?
//        Class arPropValClass = m_loader.loadClass("[Lcom.sun.star.beans.PropertyValue;");
        Object arProp = Array.newInstance(
            m_loader.loadClass("com.sun.star.beans.PropertyValue"), 1);
        Class clazz = arProp.getClass();

        Method methLoad = beanClass.getMethod(
            "loadFromURL", new Class[] {
                String.class, arProp.getClass() });

        methLoad.invoke(m_objBean, new Object[] {"private:factory/swriter", null});
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (ClassCastException e) {
            e.printStackTrace();
        } catch (java.lang.reflect.InvocationTargetException e) {
            e.printStackTrace();
        } catch (java.lang.NoSuchMethodException e) {
            e.printStackTrace();        }



        validate();
    }

    public void stop() {
        try {
            Method methStop = m_objBean.getClass().getMethod(
                "stopOOoConnection", new Class[0]);
            methStop.invoke(m_objBean, null);
        } catch (java.lang.NoSuchMethodException e) {
            e.printStackTrace();
        } catch (java.lang.IllegalAccessException e) {
            e.printStackTrace();
        }
         catch (java.lang.reflect.InvocationTargetException e) {
            e.printStackTrace();
         }

    }

    public void destroy() {
    }

    public void paint(Graphics g) {
    }
}


final class CustomURLClassLoader extends URLClassLoader {

    private Vector resourcePaths;

    public CustomURLClassLoader( URL[] urls ) {
        super( urls );
    }

    protected Class findClass( String name ) throws ClassNotFoundException {
        // This is only called via this.loadClass -> super.loadClass ->
        // this.findClass, after this.loadClass has already called
        // super.findClass, so no need to call super.findClass again:
        throw new ClassNotFoundException( name );
//        return super.findClass(name);
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

    public void addResourcePath(URL rurl) {
        if (resourcePaths == null) resourcePaths = new Vector();
        resourcePaths.add(rurl);
    }

    public URL getResource(String name) {
        if (resourcePaths == null) return null;

        URL result = super.getResource(name);
        if (result != null) {
            return result;
        }

        URL u = null;
        URI uri = null;
        for (Enumeration e = resourcePaths.elements(); e.hasMoreElements();) {
            u = (URL)e.nextElement();
            if (u.getProtocol().startsWith("file")){
                try {
                    File f1 = new File(u.getPath());
                    File f2 = new File(f1, name);
                    if (f2.exists()) {
                        return new URL(f2.toURI().toASCIIString());
                    }
                } catch (MalformedURLException e1) {
                    System.err.println("malformed url: "+e1.getMessage());
                    continue;
                }
            }
        }
        return null;
    }

}
