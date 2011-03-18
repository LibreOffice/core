/*************************************************************************
 *
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
 ************************************************************************/

package com.sun.star.help;

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.script.XInvocation;
import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Date;
import java.util.zip.ZipOutputStream;

/**
   When this tool is used with long path names on Windows, that is paths which start
   with \\?\, then the caller must make sure that the path is unique. This is achieved
   by removing '.' and '..' from the path. Paths which are created by
   osl_getSystemPathFromFileURL fulfill this requirement. This is necessary because
   lucene is patched to not use File.getCanonicalPath. See long_path.patch in the lucene
   module.
 */
public class HelpIndexer extends WeakBase
    implements XServiceInfo, XInvocation
{
    static private final String __serviceName =
        "com.sun.star.help.HelpIndexer";
    static private final String aCreateIndexMethodName = "createIndex";

    static private com.sun.star.help.HelpIndexerTool helpindexer = new com.sun.star.help.HelpIndexerTool();

    public HelpIndexer()
    {
    }

    public HelpIndexer(XComponentContext xCompContext)
    {
    }

    public static void mainImpl( String[] args, boolean bExtensionMode )
    {
        helpindexer.mainImpl( args , bExtensionMode );
    }

    public static void createZipFile( File aDirToZip, String aTargetZipFileStr )
            throws FileNotFoundException, IOException
    {
        helpindexer.createZipFile( aDirToZip , aTargetZipFileStr );
    }

    public static void addToZipRecursively( ZipOutputStream zos, File aFile, String aBasePath )
            throws FileNotFoundException, IOException
    {
        helpindexer.addToZipRecursively( zos , aFile , aBasePath );
    }

    static public boolean deleteRecursively( File aFile )
    {
        return helpindexer.deleteRecursively( aFile );
    }

    //===================================================
    // XInvocation
    public XIntrospectionAccess getIntrospection()
    {
        return  null;
    }

    public Object invoke( String aFunctionName, java.lang.Object[] aParams,
        short[][] aOutParamIndex, java.lang.Object[][] aOutParam )
            throws  com.sun.star.lang.IllegalArgumentException,
                    com.sun.star.script.CannotConvertException,
                    com.sun.star.reflection.InvocationTargetException
    {
        if(
              !aFunctionName.equals( aCreateIndexMethodName  ) )
            throw new com.sun.star.lang.IllegalArgumentException();

        aOutParamIndex[0] = new short[0];
        aOutParam[0] = new Object[0];

        int nParamCount = aParams.length;
        String aStrs[] = new String[nParamCount];
        for( int i = 0 ; i < nParamCount ; i++ )
        {
            try
            {
                aStrs[i] = AnyConverter.toString( aParams[i] );
            }
            catch( IllegalArgumentException e )
            {
                aStrs[i] = "";
            }
        }

        boolean bExtensionMode = true;
        mainImpl( aStrs, bExtensionMode );

        return null;
    }

    public void setValue( String aPropertyName, java.lang.Object aValue )
        throws  com.sun.star.beans.UnknownPropertyException,
                com.sun.star.script.CannotConvertException,
                com.sun.star.reflection.InvocationTargetException
    {
        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public Object getValue( String aPropertyName )
        throws com.sun.star.beans.UnknownPropertyException
    {
        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public boolean hasMethod( String aMethodName )
    {
        boolean bRet = (aMethodName.equals( aCreateIndexMethodName ) );
        return bRet;
    }
    public boolean hasProperty( String aName ) {
        return false;
    }



    /** This method returns an array of all supported service names.
     * @return Array of supported service names.
     */
    public String[] getSupportedServiceNames()
    {
        return getServiceNames();
    }

    /** This method is a simple helper function to used in the
     * static component initialisation functions as well as in
     * getSupportedServiceNames.
     */
    public static String[] getServiceNames()
    {
        String[] sSupportedServiceNames = { __serviceName };
        return sSupportedServiceNames;
    }


    /** This method returns true, if the given service will be
     * supported by the component.
     * @param sServiceName Service name.
     * @return True, if the given service name will be supported.
     */
    public boolean supportsService( String sServiceName )
    {
        return sServiceName.equals( __serviceName );
    }


    /** Return the class name of the component.
     * @return Class name of the component.
     */
    public String getImplementationName()
    {
        return  HelpIndexer.class.getName();
    }

}

