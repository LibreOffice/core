/*************************************************************************
*
*  $RCSfile: ClassLoaderFactory.java,v $
*
*  $Revision: 1.1 $
*
*  last change: $Author: toconnor $ $Date: 2003-09-10 10:44:26 $
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

package com.sun.star.script.framework.provider;

import java.util.ArrayList;
import java.util.Vector;
import java.io.File;
import java.io.IOException;
import java.net.*;

import com.sun.star.uno.XComponentContext;

import com.sun.star.script.framework.log.LogUtils;

/**
 *  Class Loader Factory
 *
 * @author     Noel Power
 * @created    August 2, 2002
 */
public class ClassLoaderFactory
{
    private ClassLoaderFactory() {}

    public static ClassLoader getClassLoader(XComponentContext ctxt,
        ClassLoader parent, Vector classpath)
        throws NoSuitableClassLoaderException
    {
        ClassLoader loader =
            getURLClassLoader(ctxt, parent, classpath);

        if (loader != null)
        {
            return loader;
        }
        else
        {
            throw new NoSuitableClassLoaderException(
                "Unable to create URLClassLoader");
        }
    }

    private static ClassLoader getURLClassLoader(XComponentContext ctxt,
        ClassLoader parent, Vector classpath)
    {
        LogUtils.DEBUG("ClassLoaderFactory creating classloader with classpath list vector length " + classpath.size() );
        URLStreamHandler handler = new UCBStreamHandler(ctxt);

        int len = classpath.size();
        ArrayList urls = new ArrayList(len);

        for (int i = 0; i < len; i++) {
            try {
                URL url = PathUtils.createScriptURL( ( String )classpath.elementAt( i ), ctxt );
                urls.add(url);
            }
            catch (MalformedURLException mue) {
                System.err.println("Got a malformed URL: " + mue.getMessage());
                mue.printStackTrace();
            }
        }

        if (urls.size() == 0)
            return null;

        return new URLClassLoader((URL[])urls.toArray(new URL[0]), parent);
    }
}
