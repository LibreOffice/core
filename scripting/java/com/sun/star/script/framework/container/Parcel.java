/*************************************************************************
 *
 *  $RCSfile: Parcel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:58:15 $
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
package com.sun.star.script.framework.container;

import  com.sun.star.script.framework.log.*;
import  com.sun.star.script.framework.io.*;
import  com.sun.star.script.framework.browse.*;
import  com.sun.star.script.framework.provider.PathUtils;

import com.sun.star.container.*;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;
import com.sun.star.io.*;
import com.sun.star.frame.*;

import com.sun.star.embed.*;

import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.ucb.XSimpleFileAccess2;
import java.io.*;
import java.util.*;
import java.net.*;

public class Parcel implements XNameContainer
{
    protected ParcelDescriptor parcel;
    protected String name;
    protected ParcelContainer parent;
    protected XSimpleFileAccess m_xSFA;
    public Parcel( XSimpleFileAccess xSFA, ParcelContainer parent, ParcelDescriptor desc, String parcelName )
    {
       this( parent, desc, parcelName );
       this.m_xSFA = xSFA;
    }

    public Parcel( ParcelContainer parent, ParcelDescriptor desc, String parcelName )
    {
        this.parent = parent;
        this.parcel = desc;
        this.name = parcelName;
    }

    /**
     * Tests if this <tt>Parcel</tt> is in an UNO package
     * or within a sub package within an UNO package
     *
     * @return    <tt>true</tt> if has parent <tt>false</tt> otherwise
     */
    public boolean isUnoPkg() { return parent.isUnoPkg(); }

    public String getName()
    {
        return name;
    }
    public java.lang.Object getByName( String aName ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG("** Parcel.getByName for " + aName  );
        ScriptEntry script = null;
        try
        {
            if ( parcel != null && hasElements() )
            {
                ScriptEntry[] scripts = parcel.getScriptEntries();
                if ( scripts.length != 0 )
                {
                    for ( int index = 0; index < scripts.length; index++ )
                    {
                        if ( scripts[ index ].getLanguageName().equals( aName ) )
                        {
                            script = scripts[ index ];
                            break;
                        }
                    }
                }
            }
        }
        // catch unknown or un-checked exceptions
        catch ( Exception e )
        {
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
        if ( script == null )
        {
            LogUtils.DEBUG("No script for " + aName );
            throw new com.sun.star.container.NoSuchElementException("No script named " + aName );
        }
        ScriptMetaData data = new ScriptMetaData( this, script, null );

        LogUtils.DEBUG("returning date  for " + aName );
        return data;
    }
    public String[] getElementNames()
    {
        String[] results = new String[0];
        if ( parcel != null )
        {
            ScriptEntry[] scripts = parcel.getScriptEntries();
            results = new String[ scripts.length ];
            for ( int index = 0; index < scripts.length; index++ )
            {
                results[ index ] = scripts[ index ].getLanguageName();
            }
        }
        return results;
    }
    public boolean hasByName( String aName )
    {

        boolean result = true;
        Object containee  = null;
        try
        {
            containee = getByName( aName );
            if ( containee != null )
            {
                result = true;
            }
        }
        catch( Exception e )
        {
            result = false;
        }
        return result;
    }

    public com.sun.star.uno.Type getElementType() {
        // TODO at the moment this returns void indicating
        // type is unknown ( from UNO point of view this is correct )
        // but, maybe we want to have a private UNO interface
        //
        return new Type();
    }

    public boolean hasElements()
    {
        if ( parcel != null && parcel.getScriptEntries().length > 0 )
        {
            return true;
        }
        return false;
    }



    public ParcelDescriptor getParcelDescriptor()
    {
       return parcel;
    }

    public void replaceByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
   {
       // TODO check type of aElement
       // if not ok, throw IllegalArgument
       if ( parcel != null )
       {
           try
           {
               ScriptEntry script = (ScriptEntry)getByName( aName );
               if ( script != null )
               {
                   //parcel.removeScriptEntry( script );
                   // TODO needs to create source file ( if there is one )
                   //parcel.write();
               }
               else
               {
                   throw new com.sun.star.container.NoSuchElementException("No script named " + aName );
               }


           }
           // TO DO should catch specified exceptions
           catch ( Exception e )
           {
               throw new com.sun.star.lang.WrappedTargetException();
           }

       }
   }
    // create
    public void insertByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, ElementExistException, com.sun.star.lang.WrappedTargetException
    {
        String pathToParcel = getPathToParcel();
        String pathToParcelUrl = PathUtils.make_url( pathToParcel , "parcel-descriptor.xml" );
        // TODO check the type of aElement and throw#
        // if not appropriate
        try
        {
            if ( hasByName( aName ) )
            {
                throw new ElementExistException( aName );
            }
            ScriptMetaData script = (ScriptMetaData)aElement;

            if (  script.hasSource() )
            {
                if ( !script.writeSourceFile() )
                {
                    throw new com.sun.star.lang.WrappedTargetException( "Failed to create source file " + script.getLanguageName() );
                }
            }
            parcel.addScriptEntry( script );

            if ( m_xSFA.exists( pathToParcelUrl ) )
            {
                LogUtils.DEBUG("Parcel.insertByName() opening stream " + pathToParcelUrl );
                writeParcelDesc( m_xSFA, pathToParcelUrl, parcel );
            }
            else
            {
                String error = new String( pathToParcelUrl + " does not exist " );
                throw new  com.sun.star.lang.WrappedTargetException( error );
            }
        }
        catch ( Exception e )
        {
            LogUtils.DEBUG("Failed to write entry " + aName + " in " + pathToParcelUrl + " : " + e);
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
    }


    private void writeParcelDesc( XSimpleFileAccess m_xSFA, String pathToParcelDesc, ParcelDescriptor pd ) throws com.sun.star.ucb.CommandAbortedException, com.sun.star.io.IOException, com.sun.star.uno.Exception, java.io.IOException
    {
        XSimpleFileAccess2 xSFA2 = ( XSimpleFileAccess2 )
        UnoRuntime.queryInterface( XSimpleFileAccess2.class, m_xSFA );
        if ( xSFA2 != null )
        {
            LogUtils.DEBUG("writeParcelDesc2() Using XSIMPLEFILEACCESS2 " + pathToParcelDesc );
            ByteArrayOutputStream bos = new ByteArrayOutputStream( 1024 );
            pd.write( bos );
            ByteArrayInputStream bis = new ByteArrayInputStream( bos.toByteArray() );
            XInputStreamImpl xis = new XInputStreamImpl( bis );
            xSFA2.writeFile( pathToParcelDesc, xis );
            bos.close();
            bis.close();
        }

    }

    // delete
    public void removeByName( String Name ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        try
        {
            ScriptMetaData script = null;
            if ( ( script = (ScriptMetaData)getByName( Name ) ) != null )
            {
//                if ( script.hasSource() )
                {
                   if ( !script.removeSourceFile() )
                   {
                       LogUtils.DEBUG("** Parcel.removeByName Failed to remove script " + Name  );
                       throw new com.sun.star.lang.WrappedTargetException("Failed to remove script " + Name );
                   }
                   LogUtils.DEBUG("** Parcel.removeByName have removed script source file " + Name );
                }
                parcel.removeScriptEntry( script );
                String pathToParcelDesc = PathUtils.make_url( getPathToParcel(),  "parcel-descriptor.xml" );
                writeParcelDesc(  m_xSFA, pathToParcelDesc, parcel );

            }
            else
            {
                throw new com.sun.star.container.NoSuchElementException( "No script named " + Name );
            }

        }
        catch ( Exception e )
        {
            LogUtils.DEBUG("** Parcel.removeByName Exception: " + e );
            throw new  com.sun.star.lang.WrappedTargetException( e.toString() );
        }

    }
    // rename parcel
    public void rename( String name ) throws com.sun.star.lang.WrappedTargetException
    {
        this.name = name;
    }
    public ParcelContainer getParent() { return parent; }
    /**
     * Returns the path of this  <tt>Parcel</tt>
     *
     * @return    <tt>String</tt> path to parcel
    */
    public String getPathToParcel()
    {
        String path = parent.getParcelContainerDir() + "/" + name;
        return path;
    }

}

