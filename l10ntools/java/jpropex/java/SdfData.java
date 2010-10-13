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

public class SdfData
{
    private String          filename;
    private OrderedHashMap  ohm;
    private LinkedHashSet   languagesFound;

    public SdfData()
    {
        languagesFound  = new LinkedHashSet();
        ohm             = new OrderedHashMap();
        languagesFound  = new LinkedHashSet();
    }
    public SdfData( String filename )
    {
        this();
        this.filename = filename;
    }

    public LinkedHashSet getLanguages()
    {
        return languagesFound;
    }
    public SdfEntity get( SdfEntity obj )
    {
        return (SdfEntity) ohm.get( (String)obj.getId() );
    }
    public SdfEntity get( String key ){
        return (SdfEntity) ohm.get( key );
    }
    public void add( SdfEntity obj )
    {
        ohm.add( obj.getId() , obj );
    }

    public void read()
    {
        BufferedReader in;
        try
        {
            in = new BufferedReader( new InputStreamReader( new FileInputStream( filename ), "UTF-8" ) );
            SdfEntity entity;
            while( in.ready() )
            {
                String line = in.readLine();
                if( line.length() > 0 )
                {
                    entity = new SdfEntity( line );
                    ohm.add( entity.getId() , entity );        // test if is valid
                    languagesFound.add( entity.getLangid() );
                }
            }
            in.close();
        }
        catch( IOException e )
        {
            System.out.println("Warning: can not read file " + filename);
        }
    }
    public void write( String filename )
    {
        FileWriter out;
        try
        {
            out = new FileWriter( filename , true );  // Always append
            for( Enumeration e = ohm.elements(); e.hasMoreElements(); )
            {
                out.write( ( (SdfEntity) e.nextElement() ).toString() + "\n" );
            }
            out.close();
        }
        catch( IOException e )
        {
            System.out.println("Error: Can't write to file " + filename);
            System.exit( -1 );
        }
    }
}
