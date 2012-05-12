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

