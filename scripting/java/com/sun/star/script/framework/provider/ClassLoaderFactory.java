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



package com.sun.star.script.framework.provider;

import java.net.*;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.container.ScriptMetaData;

/**
 *  Class Loader Factory
 *
 * @author     Noel Power
 * @created    August 2, 2002
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
        throws NoSuitableClassLoaderException
    {
        ClassLoader loader =
            new URLClassLoader( classpath, parent);

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

}
