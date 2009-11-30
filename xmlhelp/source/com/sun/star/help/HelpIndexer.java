/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: HelpIndexer.java,v $
 * $Revision: 1.21 $
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

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;
import java.util.zip.CRC32;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.analysis.cjk.CJKAnalyzer;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.IndexWriter;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Date;

public class HelpIndexer extends WeakBase
    implements XServiceInfo, XInvocation
{
    static private final String __serviceName =
        "com.sun.star.help.HelpIndexer";
    static private final String aCreateIndexMethodName = "createIndex";

    public HelpIndexer()
    {
    }

    public HelpIndexer(XComponentContext xCompContext)
    {
    }

    /**
     * @param args the command line arguments
     */
    public static void main( String[] args )
    {
        boolean bExtensionMode = false;
        mainImpl( args, bExtensionMode );
    }

    private static void mainImpl( String[] args, boolean bExtensionMode )
    {
        String aDirToZipStr = "";
        String aSrcDirStr = "";
        String aLanguageStr = "";
        String aModule = "";
        String aTargetZipFileStr = "";

        // Scan arguments
        boolean bLang = false;
        boolean bMod = false;
        boolean bZipDir = false;
        boolean bSrcDir = false;
        boolean bOutput = false;

        int nArgCount = args.length;
        for( int i = 0 ; i < nArgCount ; i++ )
        {
            if( "-lang".equals(args[i]) )
            {
                if( i + 1 < nArgCount )
                {
                    aLanguageStr = args[i + 1];
                    bLang = true;
                }
                i++;
            }
            else if( "-mod".equals(args[i]) )
            {
                if( i + 1 < nArgCount )
                {
                    aModule = args[i + 1];
                    bMod = true;
                }
                i++;
            }
            else if( "-zipdir".equals(args[i]) )
            {
                if( i + 1 < nArgCount )
                {
                    aDirToZipStr = args[i + 1];
                    bZipDir = true;
                }
                i++;
            }
            else if( "-srcdir".equals(args[i]) )
            {
                if( i + 1 < nArgCount )
                {
                    aSrcDirStr = args[i + 1];
                    bSrcDir = true;
                }
                i++;
            }
            else if( "-o".equals(args[i]) )
            {
                if( i + 1 < nArgCount )
                {
                    aTargetZipFileStr = args[i + 1];
                    bOutput = true;
                }
                i++;
            }
        }

        if( !bLang || !bMod || !bZipDir || (!bOutput && !bExtensionMode) )
        {
            if( bExtensionMode )
                return;

            System.out.println("Usage: HelpIndexer -lang ISOLangCode -mod HelpModule -zipdir TempZipDir -o OutputZipFile");
            System.exit( -1 );
        }

        String aIndexDirName = aModule + ".idxl";
        File aIndexDir = new File( aDirToZipStr + File.separator + aIndexDirName );
        if( !bSrcDir )
            aSrcDirStr = aDirToZipStr;
        File aCaptionFilesDir = new File( aSrcDirStr + File.separator + "caption" );
        File aContentFilesDir = new File( aSrcDirStr + File.separator + "content" );

        try
        {
            Date start = new Date();
            Analyzer analyzer = aLanguageStr.equals("ja") ? (Analyzer)new CJKAnalyzer() : (Analyzer)new StandardAnalyzer();
            IndexWriter writer = new IndexWriter( aIndexDir, analyzer, true );
            if( !bExtensionMode )
                System.out.println( "Lucene: Indexing to directory '" + aIndexDir + "'..." );
            int nRet = indexDocs( writer, aModule, bExtensionMode, aCaptionFilesDir, aContentFilesDir );
            if( nRet != -1 )
            {
                if( !bExtensionMode )
                {
                    System.out.println();
                    System.out.println( "Optimizing ..." );
                }
                writer.optimize();
            }
            writer.close();

            if( bExtensionMode )
            {
                if( !bSrcDir )
                {
                    deleteRecursively( aCaptionFilesDir );
                    deleteRecursively( aContentFilesDir );
                }
            }
            else
            {
                if( nRet == -1 )
                    deleteRecursively( aIndexDir );

                if( !bExtensionMode )
                    System.out.println( "Zipping ..." );
                File aDirToZipFile = new File( aDirToZipStr );
                createZipFile( aDirToZipFile, aTargetZipFileStr );
                deleteRecursively( aDirToZipFile );
            }

            Date end = new Date();
            if( !bExtensionMode )
                System.out.println(end.getTime() - start.getTime() + " total milliseconds");
        }
        catch (IOException e)
        {
            if( bExtensionMode )
                return;

            System.out.println(" caught a " + e.getClass() +
                "\n with message: " + e.getMessage());
            System.exit( -1 );
        }
    }

    private static int indexDocs(IndexWriter writer, String aModule, boolean bExtensionMode,
        File aCaptionFilesDir, File aContentFilesDir) throws IOException
    {
        if( !aCaptionFilesDir.canRead() || !aCaptionFilesDir.isDirectory() )
        {
            if( !bExtensionMode )
                System.out.println( "Not found: " + aCaptionFilesDir );
            return -1;
        }
        if( !aContentFilesDir.canRead() || !aContentFilesDir.isDirectory() )
        {
            if( !bExtensionMode )
                System.out.println( "Not found: " + aContentFilesDir );
            return -1;
        }

        String[] aCaptionFiles = aCaptionFilesDir.list();
        List aCaptionFilesList = Arrays.asList( aCaptionFiles );
        HashSet aCaptionFilesHashSet = new HashSet( aCaptionFilesList );

        String[] aContentFiles = aContentFilesDir.list();
        List aContentFilesList = Arrays.asList( aContentFiles );
        HashSet aContentFilesHashSet = new HashSet( aContentFilesList );

        // Loop over caption files and find corresponding content file
        if( !bExtensionMode )
            System.out.println( "Indexing, adding files" );
        int nCaptionFilesLen = aCaptionFiles.length;
        for( int i = 0 ; i < nCaptionFilesLen ; i++ )
        {
            String aCaptionFileStr = aCaptionFiles[i];
            File aCaptionFile = new File( aCaptionFilesDir, aCaptionFileStr );
            File aContentFile = null;
            if( aContentFilesHashSet.contains( aCaptionFileStr ) )
                aContentFile = new File( aContentFilesDir, aCaptionFileStr );

            if( !bExtensionMode )
                System.out.print( "." );
            writer.addDocument( HelpFileDocument.Document( aModule, aCaptionFile, aContentFile ) );
        }

        // Loop over content files to find remaining files not mapped to caption files
        int nContentFilesLen = aContentFiles.length;
        for( int i = 0 ; i < nContentFilesLen ; i++ )
        {
            String aContentFileStr = aContentFiles[i];
            if( !aCaptionFilesHashSet.contains( aContentFileStr ) )
            {
                // Not already handled in caption files loop
                File aCaptionFile = null;
                File aContentFile = new File( aContentFilesDir, aContentFileStr );
                if( !bExtensionMode )
                    System.out.print( "." );
                writer.addDocument( HelpFileDocument.Document( aModule, aCaptionFile, aContentFile ) );
            }
        }
        return 0;
    }

    public static void createZipFile( File aDirToZip, String aTargetZipFileStr )
            throws FileNotFoundException, IOException
    {
        FileOutputStream fos = new FileOutputStream( aTargetZipFileStr );
        ZipOutputStream zos = new ZipOutputStream( fos );

        File[] aChildrenFiles = aDirToZip.listFiles();
        int nFileCount = aChildrenFiles.length;
        for( int i = 0 ; i < nFileCount ; i++ )
            addToZipRecursively( zos, aChildrenFiles[i], null );

        zos.close();
    }

    public static void addToZipRecursively( ZipOutputStream zos, File aFile, String aBasePath )
            throws FileNotFoundException, IOException
    {
        if( aFile.isDirectory() )
        {
            String aDirName = aFile.getName();
            if( aDirName.equalsIgnoreCase( "caption" ) || aDirName.equalsIgnoreCase( "content" ) )
                return;

            File[] aChildrenFiles = aFile.listFiles();
            String aNewBasePath = "";
            if( aBasePath != null )
                aNewBasePath += aBasePath + File.separator;
            aNewBasePath += aDirName;

            int nFileCount = aChildrenFiles.length;
            for( int i = 0 ; i < nFileCount ; i++ )
                addToZipRecursively( zos, aChildrenFiles[i], aNewBasePath );

            return;
        }

        // No directory
        // read contents of file we are going to put in the zip
        int fileLength = (int) aFile.length();
        FileInputStream fis = new FileInputStream( aFile );
        byte[] wholeFile = new byte[fileLength];
        int bytesRead = fis.read( wholeFile, 0, fileLength );
        fis.close();

        String aFileName = aFile.getName();
        String aEntryName = "";
        if( aBasePath != null )
            aEntryName += aBasePath + "/";
        aEntryName += aFileName;
        ZipEntry aZipEntry = new ZipEntry( aEntryName );
        aZipEntry.setTime( aFile.lastModified() );
        aZipEntry.setSize( fileLength );

        int nMethod = ( aFileName.toLowerCase().endsWith( ".jar" ) )
                ? ZipEntry.STORED : ZipEntry.DEFLATED;
        aZipEntry.setMethod( nMethod );

        CRC32 tempCRC = new CRC32();
        tempCRC.update( wholeFile, 0, wholeFile.length );
        aZipEntry.setCrc( tempCRC.getValue() );

        // write the contents into the zip element
        zos.putNextEntry( aZipEntry );
        zos.write( wholeFile, 0, fileLength );
        zos.closeEntry();
    }

    static public boolean deleteRecursively( File aFile )
    {
        if( aFile.isDirectory() )
        {
            File[] aChildrenFiles = aFile.listFiles();
            int nFileCount = aChildrenFiles.length;
            for( int i = 0 ; i < nFileCount ; i++ )
            {
                File aChildrenFile = aChildrenFiles[i];
                boolean bSuccess = deleteRecursively( aChildrenFile );
                if( !bSuccess )
                    return false;
            }
        }

        return aFile.delete();
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
        if( !aFunctionName.equals( aCreateIndexMethodName  ) )
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

