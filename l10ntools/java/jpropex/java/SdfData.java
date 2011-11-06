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
