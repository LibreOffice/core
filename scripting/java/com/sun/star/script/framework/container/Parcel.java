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
package com.sun.star.script.framework.container;
import  com.sun.star.script.framework.log.*;
import  com.sun.star.script.framework.io.*;
import  com.sun.star.script.framework.provider.PathUtils;

import com.sun.star.container.*;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.ucb.XSimpleFileAccess2;
import java.io.*;

public class Parcel implements XNameContainer
{
    protected ParcelDescriptor m_descriptor;
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
        this.m_descriptor = desc;
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
            if ( m_descriptor != null && hasElements() )
            {
                ScriptEntry[] scripts = m_descriptor.getScriptEntries();
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
        if ( m_descriptor != null )
        {
            ScriptEntry[] scripts = m_descriptor.getScriptEntries();
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
        if ( m_descriptor != null && m_descriptor.getScriptEntries().length > 0 )
        {
            return true;
        }
        return false;
    }

    public void replaceByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
   {
       // TODO check type of aElement
       // if not ok, throw IllegalArgument
       if ( m_descriptor != null )
       {
           try
           {
               ScriptEntry script = (ScriptEntry)getByName( aName );
               if ( script != null )
               {
                   //m_descriptor.removeScriptEntry( script );
                   // TODO needs to create source file ( if there is one )
                   //m_descriptor.write();
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
                LogUtils.DEBUG("Inserting source: " + script.getSource());
                if ( !script.writeSourceFile() )
                {
                    throw new com.sun.star.lang.WrappedTargetException( "Failed to create source file " + script.getLanguageName() );
                }
            }
            m_descriptor.addScriptEntry( script );
            writeParcelDescriptor();
        }
        catch ( Exception e )
        {
            LogUtils.DEBUG("Failed to insert entry " + aName + ": " + e.getMessage());
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
    }


    private void writeParcelDescriptor()
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.io.IOException,
               com.sun.star.uno.Exception, java.io.IOException
    {
        String pathToDescriptor = PathUtils.make_url(
            getPathToParcel(),  ParcelDescriptor.PARCEL_DESCRIPTOR_NAME );

        XSimpleFileAccess2 xSFA2 = UnoRuntime.queryInterface( XSimpleFileAccess2.class, m_xSFA );

        if ( xSFA2 != null )
        {
            ByteArrayOutputStream bos = null;
            ByteArrayInputStream bis = null;
            XInputStreamImpl xis = null;
            try
            {
                bos = new ByteArrayOutputStream( 1024 );
                m_descriptor.write( bos );
                bis = new ByteArrayInputStream( bos.toByteArray() );

                xis = new XInputStreamImpl( bis );
                xSFA2.writeFile( pathToDescriptor, xis );
            }
            finally
            {
                if (bos != null) bos.close();
                if (bis != null) bis.close();
                if (xis != null) xis.closeInput();
            }
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
                m_descriptor.removeScriptEntry( script );
                writeParcelDescriptor();

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

