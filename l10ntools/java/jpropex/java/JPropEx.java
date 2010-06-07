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


import java.util.*;
import java.io.*;

public class JPropEx
{
    private String inputFileArg     = "";
    private String outputFileArg    = "";
    private String pathPrefixArg    = "";
    private String pathPostfixArg   = "";
    private String projectArg       = "";
    private String rootArg          = "";
    private Vector forcedLangsArg;
    private Vector langsArg;
    private String inputSdfFileArg  = "";
    private boolean isQuiet             = false;
    private final String resourceType   = "javaproperties";
    private final String sourceLanguage = "en-US";
    //private SdfData data;

    public JPropEx()
    {
        //data = new SdfData();
    }

    public JPropEx( String args[] )
    {
        super();
        parseArguments( args );
        testCL();
        //testArguments();
        if( inputSdfFileArg != null && inputSdfFileArg.length() > 0 )
            merge();
        else
            extract();
    }

    private String getSimpleArg( String[] args , int x )
    {
        if( x < args.length ) x++;
        else
        {
            System.err.println("ERROR: Missing arg for "+args[ x ]+"\n");
            help();
        }
        return args[ x ];
    }
    private Vector getComplexArg( String[] args , int x )
    {
        if( x < args.length ) x++;
        else
        {
            System.err.println("ERROR: Missing arg for "+args[ x ]+"\n");
            help();
        }
        String value = args[ x ];
        Vector values = new Vector( Arrays.asList( value.split(",") ) );
        return values;
    }

    private void testCL()
    {
        if( inputFileArg.length()>0 && ( ( pathPrefixArg.length()>0 && pathPostfixArg.length()>0 ) || outputFileArg.length()>0 ) && projectArg.length()>0 && rootArg.length()>0 && langsArg.size()>0 )
            if( ( inputSdfFileArg.length()>0 && ( outputFileArg.length()>0 ||  ( pathPrefixArg.length()>0 && pathPostfixArg.length()>0 ) ) ) ||  ( inputFileArg.length()>0 && outputFileArg.length()>0 ) )
                    return;
        System.out.println("ERROR: Strange parameters!");
        help();
        System.exit( -1 );
    }
    private void help()
    {
        System.out.println("jpropex -> extract / merge java properties files");
        System.out.println("-p <project> -r <project_root> -i <input>|@<input> -o <output> -x <path1> -y <path2> -m <sdf_file> -l <lang>\n");
        System.out.println("Example:\ncd /data/cws/l10ntooling17/DEV300/ooo/reportbuilder/java/com/sun/star/report/function/metadata");
        System.out.println("Extract:\njpropex -p reportbuilder -r ../../../../../../.. -i Title-Function.properties -o new.sdf -l en-US");
        System.out.println("Merge: use either ( -x path -y more_path ) or ( -o ) and ( -i filename ) or ( -i @filename ). @filename contains a list with files");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -x ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata -y ivo -i @abc -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -x ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata -y ivo -i @abc -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -o ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata/ -i Title-Function.properties -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -x ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata -y ivooo -i Title-Function.properties -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.exit( -1 );
    }

    private void extract()
    {
        SdfData data = new SdfData();
        java.util.Properties prop = loadProp( inputFileArg );

        // Get a prototype that already contains the most common settings
        SdfEntity dolly = prepareSdfObj( inputFileArg );
        String key;
        SdfEntity currentStr;
        String value;
        for( Enumeration e = prop.propertyNames() ; e.hasMoreElements() ; )
        {
            key         = (String)      e.nextElement();
            currentStr  = (SdfEntity)   dolly.clone();
            // Set the new LID and the string text
            currentStr.setLid( key );
            value            = prop.getProperty( key , "" );
            //if( value.equals("") )  System.err.println("Warning: in file "+inputFileArg+" the string with the key "+key+" has a empty string!");
            currentStr.setText( (prop.getProperty( key )).replaceAll("\t" , " " ) );     // TODO: Quoting!!!!
            data.add( currentStr );
        }
        data.write( outputFileArg );
    }

    private SdfEntity prepareSdfObj( String filename )
    {
        String path = makeAbs( filename );
        //String path = makeAbs( inputFileArg );
        path = path.replace( rootArg + "/" , "" );
        path = path.replace("/","\\");
        return new SdfEntity( projectArg , path , "" /* dummy1 */ , resourceType , "", "" , "" , "" , "" /* dummy2 */ ,
                              sourceLanguage , "",  "" , ""  , "" , "2002-02-02 02:02:02" );
    }

    private void merge()
    {
        SdfData data = getSdfData();
        if( inputFileArg.startsWith("@") )
        {
            // Read files
            Vector fileList = readFileList( inputFileArg );
            for( Enumeration e = fileList.elements(); e.hasMoreElements(); )
                mergeFile( (String) e.nextElement() , data , false );
        }
        else
        {
            // Single file
            mergeFile( inputFileArg , data , true );
        }
    }

    private Vector readFileList( String filename )
    {
        Vector lines = new Vector();
        try
        {
            BufferedReader in = new BufferedReader( new FileReader( filename.substring( 1 ) ) );
            while( in.ready() )
                lines.add( in.readLine() );
        }
        catch( IOException e )
        {
            System.out.println("ERROR: Can't open file '"+filename.substring( 1 )+"'");
            System.exit( -1 );
        }
        return lines;
    }

    private void mergeFile( String filename , SdfData data , boolean isSingleFile )
    {
        java.util.Properties sourceProp = loadProp( filename );
        Vector langs = getLanguages( data );
        HashMap props = new HashMap();
        // Create a properties object for every language
        for( Enumeration e = langs.elements(); e.hasMoreElements();)
        {
            props.put( (String)e.nextElement() , new java.util.Properties() );
        }
        // Get a prototype that already contains the most common settings

        SdfEntity dolly = prepareSdfObj( filename );
        String key;
        String sourceString;
        SdfEntity curStr;
        SdfEntity curEntity;
        SdfEntity mergedEntity;
        String curLang;
        for( Enumeration e = sourceProp.propertyNames() ; e.hasMoreElements() ; )     // For all property keys
        {
            key          = (String) e.nextElement();
            sourceString = sourceProp.getProperty( key );
            curStr       = (SdfEntity) dolly.clone();
            curStr.setLid( key );
            for( Enumeration lang = langs.elements(); lang.hasMoreElements(); ) // merge in every language
            {
                curEntity   = (SdfEntity) curStr.clone();
                curLang     = (String)    lang.nextElement();
                curEntity.setLangid( curLang );
                mergedEntity = data.get( curEntity );
                if( mergedEntity == null )
                {
                    // if case there is not translation the fallback to the en-US source string
                    ( (java.util.Properties) props.get( curLang )).setProperty( curEntity.getLid() , sourceString  );
                }
                else
                {
                    // Set the merged text from the sdf file
                    ( (java.util.Properties) props.get( curLang )).setProperty( mergedEntity.getLid() , mergedEntity.getText() );  // TODO: Quoting ???
                }
            }

        }
        // Now write them out
        String lang;
        for( Iterator i = props.keySet().iterator() ; i.hasNext() ; )
        {
            lang = (String) i.next();
            writeSinglePropertiesFile( filename , (java.util.Properties) props.get( lang ) , lang , isSingleFile );
        }
    }
    private void writeSinglePropertiesFile( String filename , java.util.Properties prop , String lang , boolean isSingleFile )
    {
        // Prepare path to file
        int filenameIdx     = filename.lastIndexOf( "/" ) > 0 ? filename.lastIndexOf( "/" )+1 : 0 ;
        String path         = new String();
        String name         = new String();
        String lcLang       = lang.toLowerCase();
        // use of -x <path> -y <more_path>
        // -> <path>/<lang>/<more_path>
        if( pathPrefixArg != null && pathPrefixArg.length()>0 && pathPostfixArg != null && pathPostfixArg.length()>0 )
        {
            path = new StringBuffer().append( pathPrefixArg ).append( "/" ).append( lcLang ).append( "/" ).append( pathPostfixArg ).append( "/" ).toString();
            name = new StringBuffer().append( filename.substring( filenameIdx , filename.lastIndexOf( ".properties" ) ) )
                                        .append( "_" ).append( lcLang.replaceAll("-","_") ).append( ".properties" ).toString();
        }
        //use of -i <one_filename>
        else if( !isSingleFile && outputFileArg != null && outputFileArg.length()>0 )
        {
            name = outputFileArg;
            name += new StringBuffer().append( filename.substring( filenameIdx , filename.lastIndexOf( ".properties" ) ) )
                                     .append( "_" ).append( lcLang.replaceAll("-","_") ).append( ".properties" ).toString();
            //name = outputFileArg;
        }
        //use of -i @<file_containing_many_filenames>
        else if( isSingleFile && outputFileArg != null && outputFileArg.length()>0 )
        {
            name = outputFileArg;
            name += new StringBuffer().append( inputFileArg.substring( filenameIdx , filename.lastIndexOf( ".properties" ) ) )
                                     .append( "_" ).append( lcLang.replaceAll("-","_") ).append( ".properties" ).toString();
            //name = outputFileArg;
        }
        else
        {
            System.err.println("ERROR: No outputfile specified .. either -o or -x -y !");
            System.exit( -1 );
        }

        File dir = new File( path );
        try
        {
            if( !dir.exists() && path.length()>0 )
            {
                if( !dir.mkdirs() )
                {
                    System.out.println("ERROR: Can't create directory '"+path+"' !!!");
                    System.exit( -1 );
                }
            }
        }
        catch( SecurityException e )
        {
            System.out.println("ERROR: Can't create directory '"+path+"'!!!Wrong Permissions?");
            System.exit( -1 );
        }
        path += name;
        // Write the properties file
        System.out.println("DBG: Writing to "+path);
        try{
            BufferedOutputStream out = new BufferedOutputStream( new FileOutputStream( path ) );
            if( prop == null )
                System.out.println("DBG: prop == null!!!");
            prop.store( out , "" );     // Legal headers?
        }
        catch( IOException e )
        {
            System.out.println("ERROR: Can't write file '"+path+"' !!!!");
            System.exit( -1 );
        }
    }

    private SdfData getSdfData()
    {
        SdfData data = new SdfData( inputSdfFileArg );
        data.read();
        return data;
    }
    private Vector getLanguages( SdfData data )
    {
        Vector langs = new Vector();

        if( ((String)langsArg.get( 0 )).equalsIgnoreCase( "all" ) ) // for "-l all" use all languages found in the -m sdf file
            langs.addAll( data.getLanguages() );
        else
            langs.addAll( langsArg );              // use the langs giving by -l

        if( forcedLangsArg != null )
            langs.addAll( forcedLangsArg );

        return removeDupes( langs );
    }
    private Vector removeDupes( Vector vec )
    {
        Collection coll = new LinkedHashSet( vec );
        return new Vector( coll );
    }
    private java.util.Properties loadProp( String filename )
    {
        java.util.Properties prop = new java.util.Properties();
        try
        {
            prop.load( new BufferedInputStream( new NoLocalizeFilter( new FileInputStream( filename ) ) ) );
        }
        catch( IOException e )
        {
            System.err.println("ERROR: Can't read file '"+filename+"'!!!");
        }
        return prop;
    }
    private void parseArguments( String[] args )
    {

        if( args.length == 0 )
        {
            System.out.println("ERROR: No args???");
            help();
            System.exit( -1 );
        }
        for( int x = 0; x < args.length ; x++ )
        {
            if( args[ x ].equalsIgnoreCase("-i") )
            {
                // Input resource file
                inputFileArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-o") )
            {
                // Output sdf file
                outputFileArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-x") )
            {
                // path prefix
                pathPrefixArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-y") )
            {
                // path postfix
                pathPostfixArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-p") )
            {
                // project
                projectArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-r") )
            {
                // root
                rootArg = getSimpleArg( args , x );
                rootArg = makeAbs( rootArg );
            }
            else if( args[ x ].equalsIgnoreCase("-lf") )
            {
                // forced langs
                forcedLangsArg = getComplexArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-l") )
            {
                // langs
                langsArg = getComplexArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-m") )
            {
                // input sdf file
                inputSdfFileArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-qq") )
            {
                isQuiet = true;
            }
        }
    }
    private String makeAbs( String path )
    {
        File file;
        try
        {
            file = new File( path );
            return file.getCanonicalPath();
        }catch( IOException e )
        {
            e.printStackTrace();
            System.exit( -1 );
        }
        return null;
    }
/*    private boolean testArguments()
    {
        // nice merge
        if( inputSdfFileArg != null && inputSdfFileArg.length()>0 )
            // nice merge
            return  projectArg != null  && rootArg != null && inputFileArg != null && pathPrefixArg != null && pathPostfixArg != null && langsArg != null &&
                    projectArg.length()>0 && rootArg.length()>0 && inputFileArg.length()>0 && pathPrefixArg.length()>0 && pathPostfixArg.length()>0 && langsArg.size()>0 ;
        else
            // nice extract
            return  projectArg != null && rootArg != null && inputFileArg != null && outputFileArg != null && langsArg != null &&
                    projectArg.length()>0 && rootArg.length()>0 && inputFileArg.length()>0 && outputFileArg.length()>0 && langsArg.size()>0;
    }
*/
}
