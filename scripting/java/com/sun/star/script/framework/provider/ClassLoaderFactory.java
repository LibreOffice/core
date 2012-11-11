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

package com.sun.star.script.framework.provider;

import java.net.*;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.container.ScriptMetaData;

/**
 *  Class Loader Factory
 */
public class ClassLoaderFactory
{
    private ClassLoaderFactory() {}

    public static ClassLoader getURLClassLoader( ScriptMetaData scriptData )
        throws NoSuitableClassLoaderException, MalformedURLException
    {
        ClassLoader parent = scriptData.getClass().getClassLoader();
        URL[] classPath = scriptData.getClassPath();
        LogUtils.DEBUG("Classpath has length " + classPath.length );
        for ( int i=0; i < classPath.length; i++ )
        {
            LogUtils.DEBUG("ClassPath " + i + "} is " + classPath[ i ].toString() );
        }
        return getURLClassLoader( parent, classPath );
    }
    public static ClassLoader getURLClassLoader( ClassLoader parent, URL[] classpath)
    {
        return new URLClassLoader( classpath, parent);
    }

}
