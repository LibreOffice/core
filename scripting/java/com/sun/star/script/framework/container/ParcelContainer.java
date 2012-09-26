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
import com.sun.star.lang.*;
import com.sun.star.io.*;
import java.io.*;
import java.util.*;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.ucb.XSimpleFileAccess2;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.uri.XUriReference;
import com.sun.star.uri.XUriReferenceFactory;
import com.sun.star.uri.XVndSunStarScriptUrl;

/**
 * The <code>ParcelContainer</code> object is used to store the
 * ScripingFramework specific Libraries.
 */

public class ParcelContainer implements XNameAccess
{
    protected String language;
    protected String containerUrl;
    protected Collection<Parcel> parcels = new ArrayList<Parcel>(10);
    static protected XSimpleFileAccess m_xSFA;
    protected XComponentContext m_xCtx;
    private ParcelContainer parent = null;
    private Collection<ParcelContainer> childContainers = new ArrayList<ParcelContainer>(10);
    private boolean isPkgContainer = false;

    /**
     * Tests if this <tt>ParcelContainer</tt> represents an UNO package
     * or sub package within an UNO package
     *
     * @return    <tt>true</tt> if has parent <tt>false</tt> otherwise
     */
    public boolean isUnoPkg() { return isPkgContainer; }
    /**
     * Tests if this <tt>ParcelContainer</tt> has a parent
     * a parent.
     *
     * @return    <tt>true</tt> if has parent <tt>false</tt> otherwise
     */
    public boolean hasParent()
    {
        if ( parent == null )
        {
            return false;
        }
        return true;
    }

    /**
     * Returns this <tt>ParcelContainer</tt>'s  parent
     *
     * @return    <tt>ParcelContainer</tt> if has parent null otherwise
     */
    public ParcelContainer parent()
    {
        return parent;
    }

    /**
     * Returns all child <tt>ParcelContainer</tt>
     * this instance of <tt>ParcelContainer</tt>
     *
     * @return    a new array of ParcelContainers. A zero
     * length array is returned if no child ParcelContainers.
     */
    public ParcelContainer[] getChildContainers()
    {
        if ( childContainers.isEmpty() )
        {
            return new ParcelContainer[0];
        }
        return childContainers.toArray( new ParcelContainer[childContainers.size()] );

    }
    /**
     * Removes a child <tt>ParcelContainer</tt>
     * from this instance.
     * @param   child  <tt>ParcelContainer</tt> to be added.
     *
     * @return    <tt>true</tt> if child successfully removed
     */
    public boolean removeChildContainer( ParcelContainer child )
    {
        return childContainers.remove( child );
    }

    /**
     * Adds a new child <tt>ParcelContainer</tt>
     * to this instance.
     * @param   child  <tt>ParcelContainer</tt> to be added.
     *
     */
    public void addChildContainer( ParcelContainer child )
    {
        childContainers.add( child );
    }

    /**
     * Returns a child <tt>ParcelContainer</tt> whose location
     * matches the <tt>location</tt> argument passed to this method.
     * @param    key the <tt>location</tt> which is to
     *           be matched.
     *
     * @return    child <tt>ParcelContainer</tt> or <null> if none
     * found.
     */

    public ParcelContainer getChildContainer( String key )
    {
        ParcelContainer result = null;
        Iterator<ParcelContainer> iter = childContainers.iterator();
        while ( iter.hasNext() )
        {
            ParcelContainer c = iter.next();
            String location = ScriptMetaData.getLocationPlaceHolder(
                c.containerUrl, c.getName());

            if ( key.equals( location ) )
            {
                result = c;
                break;
            }
        }
        return result;
    }

    /**
     * Returns a child <tt>ParcelContainer</tt> whose member
     * <tt>containerUrl</tt> matches the <tt>containerUrl</tt>
     * argument passed to this method.
     * @param    containerUrl the <tt>containerUrl</tt> which is to
     *           be matched.
     *
     * @return    child <tt>ParcelContainer</tt> or <null> if none
     * found.
     */

    public ParcelContainer getChildContainerForURL( String containerUrl )
    {
        ParcelContainer result = null;
        Iterator<ParcelContainer> iter = childContainers.iterator();
        while ( iter.hasNext() )
        {
            ParcelContainer c = iter.next();
            if ( containerUrl.equals( c.containerUrl ) )
            {
                result = c;
                break;
            }
        }
        return result;
    }

    /**
     * Returns Name of this container. Name of this <tt>ParcelContainer</tt>
     * is determined from the <tt>containerUrl</tt> as the last portion
     * of the URL after the last forward slash.
     * @return    name of <tt>ParcelContainer</tt>
     * found.
     */
    public String getName()
    {
        String name = null;
        // TODO handler package ParcelContainer?
        if (  !containerUrl.startsWith( "vnd.sun.star.tdoc:" ) )
        {
            // return name
            String decodedUrl = java.net.URLDecoder.decode( containerUrl );
            int indexOfSlash = decodedUrl.lastIndexOf( "/" );
            if ( indexOfSlash != -1 )
            {
                name =  decodedUrl.substring( indexOfSlash + 1 );
            }
        }
        else
        {
            name =  "document";
        }
        return name;
    }

    /**
     * Initializes a newly created <code>ParcelContainer</code> object.
     * @param    xCtx UNO component context
     * @param   containerUrl location of this container.
     * @param   language language for which entries are stored
     * @throws    com.sun.star.lang.IllegalArgumentException
     * @throws    WrappedTargetException
     */
    public ParcelContainer( XComponentContext xCtx, String containerUrl, String language ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException

    {
        this( null, xCtx, containerUrl, language, true );
    }

    /**
     * Initializes a newly created <code>ParcelContainer</code> object.
     * @param    xCtx UNO component context
     * @param   containerUrl location of this container.
     * @param   language language for which entries are stored
     * @param   loadParcels set to <tt>true</tt> if parcels are to be loaded
     *          on construction.
     * @throws    com.sun.star.lang.IllegalArgumentException
     * @throws    WrappedTargetException
     */
    public ParcelContainer( XComponentContext xCtx, String containerUrl, String language, boolean loadParcels ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        this( null, xCtx, containerUrl, language, loadParcels );
    }

    /**
     * Initializes a newly created <code>ParcelContainer</code> object.
     * @param   parent parent ParcelContainer
     * @param    xCtx UNO component context
     * @param   containerUrl location of this container.
     * @param   language language for which entries are stored
     * @param   loadParcels set to <tt>true</tt> if parcels are to be loaded
     *          on construction.
     * @throws    com.sun.star.lang.IllegalArgumentException
     */
    public ParcelContainer( ParcelContainer parent, XComponentContext xCtx, String containerUrl, String language, boolean loadParcels ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException

    {
        LogUtils.DEBUG("Creating ParcelContainer for " + containerUrl + " loadParcels = " + loadParcels + " language = " + language );
        this.m_xCtx = xCtx;
        this.language = language;
        this.parent = parent;
        this.containerUrl = containerUrl;

        initSimpleFileAccess();
        boolean parentIsPkgContainer = false;

        if ( parent != null )
        {
            parentIsPkgContainer = parent.isUnoPkg();
        }

        if ( containerUrl.endsWith("uno_packages") || parentIsPkgContainer  )
        {
            isPkgContainer = true;
        }

        if ( loadParcels )
        {
            loadParcels();
        }
    }


    public String getContainerURL() { return this.containerUrl; }

    private synchronized void initSimpleFileAccess()
    {
        if ( m_xSFA != null )
        {
            return;
        }
        try
        {
            m_xSFA = UnoRuntime.queryInterface(
                XSimpleFileAccess.class,
                m_xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.ucb.SimpleFileAccess", m_xCtx ) );
        }
        catch ( Exception e )
        {
            // TODO should throw
            LogUtils.DEBUG("Error instantiating simplefile access ");
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
        }
    }

    public String getParcelContainerDir()
    {
        // If this container does not represent an uno-package
        // then then it is a document, user or share
        // in each case the convention is to have a Scripts/[language]
        // dir where scripts reside
        if (  !isUnoPkg() )
        {
            return PathUtils.make_url( containerUrl  ,  "Scripts/" + language.toLowerCase() );
        }
        return containerUrl;
    }
    public Object getByName( String aName ) throws com.sun.star.container.NoSuchElementException, WrappedTargetException
    {
        Parcel parcel = null;
        try
        {
            if ( hasElements() )
            {
                Iterator<Parcel> iter = parcels.iterator();
                while ( iter.hasNext() )
                {
                    Parcel parcelToCheck = iter.next();

                    if ( parcelToCheck.getName().equals( aName ) )
                    {
                       parcel = parcelToCheck;
                       break;
                    }
                }
            }
        }
        catch ( Exception e)
        {
            throw new WrappedTargetException( e.toString() );
        }
        if ( parcel == null )
        {
            throw new com.sun.star.container.NoSuchElementException( "Macro Library " + aName + " not found" );
        }
        return parcel;
    }
    public String[] getElementNames()
    {
        if ( hasElements() )
        {
            Parcel[] theParcels = parcels.toArray( new Parcel[parcels.size()] );
            String[] names = new String[ theParcels.length ];
            for ( int count = 0; count < names.length; count++ )
            {
                names[count] = theParcels[ count ].getName();
            }
            return names;
        }

        return new String[0];
    }
    public boolean hasByName( String aName )
    {
        boolean isFound = false;
        try
        {
            if ( getByName( aName ) != null )
            {
                isFound = true;
            }

        }
        catch ( Exception e )
        {
            //TODO - handle trace
        }
        return isFound;
    }
    public Type getElementType()
    {
        return new Type();
    }
    public boolean hasElements()
    {
        if ( parcels == null || parcels.isEmpty() )
        {
            return false;
        }
        return true;
    }

    private void loadParcels() throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        try
        {
            LogUtils.DEBUG("About to load parcels from " + containerUrl );
            if ( m_xSFA.isFolder( getParcelContainerDir() ) )
            {
                LogUtils.DEBUG( getParcelContainerDir() + " is a folder " );
                String[] children = m_xSFA.getFolderContents( getParcelContainerDir(), true );
                parcels  = new ArrayList<Parcel>(children.length);
                for ( int  i = 0; i < children.length; i++)
                {
                    LogUtils.DEBUG("Processing " + children[ i ] );
                    try
                    {
                        loadParcel( children[ i ] );
                    }
                    catch (java.lang.Exception e)
                    {
                        // print an error message and move on to
                        // the next parcel
                        LogUtils.DEBUG("ParcelContainer.loadParcels caught " + e.getClass().getName() + " exception loading parcel " + children[i] + ": " + e.getMessage() );
                    }
                }
            }
            else
            {
                LogUtils.DEBUG(" ParcelCOntainer.loadParcels " + getParcelContainerDir()  + " is not a folder ");
            }

        }
        catch ( com.sun.star.ucb.CommandAbortedException e )
        {
            LogUtils.DEBUG("ParcelContainer.loadParcels caught exception processing folders for " + getParcelContainerDir()  );
            LogUtils.DEBUG("TRACE: " + LogUtils.getTrace(e) );
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
        catch ( com.sun.star.uno.Exception e )
        {
            LogUtils.DEBUG("ParcelContainer.loadParcels caught exception processing folders for " + getParcelContainerDir()  );
            LogUtils.DEBUG("TRACE: " + LogUtils.getTrace(e) );
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
    }

    public  XNameContainer createParcel(String name) throws ElementExistException, com.sun.star.lang.WrappedTargetException
    {
        Parcel p = null;
        if ( hasByName( name ) )
        {
             throw new ElementExistException( "Parcel " + name + " already exists" );
        }
        String pathToParcel =  PathUtils.make_url( getParcelContainerDir() ,  name );

        try
        {
            LogUtils.DEBUG("ParcelContainer.createParcel, creating folder " + pathToParcel );
            m_xSFA.createFolder( pathToParcel );

            LogUtils.DEBUG("ParcelContainer.createParcel, folder " + pathToParcel  + " created ");

            ParcelDescriptor pd = new ParcelDescriptor();
            pd.setLanguage( language );
            String parcelDesc = PathUtils.make_url( pathToParcel, ParcelDescriptor.PARCEL_DESCRIPTOR_NAME );
            XSimpleFileAccess2 xSFA2 = UnoRuntime.queryInterface( XSimpleFileAccess2.class, m_xSFA );
            if ( xSFA2 != null )
            {
                LogUtils.DEBUG("createParcel() Using XSIMPLEFILEACCESS2 " + parcelDesc );
                ByteArrayOutputStream bos = new ByteArrayOutputStream( 1024 );
                pd.write( bos );
                bos.close();
                ByteArrayInputStream bis = new ByteArrayInputStream( bos.toByteArray() );
                XInputStreamImpl xis = new XInputStreamImpl( bis );
                xSFA2.writeFile( parcelDesc, xis );
                xis.closeInput();
                p = loadParcel( pathToParcel );
            }
        }
        catch ( Exception e )
        {

            LogUtils.DEBUG("createParcel() Exception while attempting to create = " + name );
            throw  new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
        return p;
    }

    public Parcel loadParcel( String parcelUrl ) throws com.sun.star.lang.WrappedTargetException, com.sun.star.lang.IllegalArgumentException
    {

        String name = null;

        String parcelDescUrl =  PathUtils.make_url( parcelUrl,  ParcelDescriptor.PARCEL_DESCRIPTOR_NAME );
        Parcel parcel = null;

        XInputStream xis = null;
        InputStream is = null;

        try
        {
            if ( m_xSFA.exists( parcelDescUrl ) )
            {
                LogUtils.DEBUG("ParcelContainer.loadParcel opening " + parcelDescUrl );
                xis = m_xSFA.openFileRead( parcelDescUrl );
                is = new XInputStreamWrapper( xis );

                ParcelDescriptor pd = new ParcelDescriptor(is) ;
                try
                {
                    is.close();
                    is = null;
                }
                catch ( Exception e )
                {
                    LogUtils.DEBUG("ParcelContainer.loadParcel Exception when closing stream for  " + parcelDescUrl + " :" + e );
                }
                LogUtils.DEBUG("ParcelContainer.loadParcel closed " + parcelDescUrl );
                if ( !pd.getLanguage().equals( language ) )
                {
                    LogUtils.DEBUG("ParcelContainer.loadParcel Language of Parcel does not match this container ");
                    return null;
                }
                LogUtils.DEBUG("Processing " + parcelDescUrl + " closed " );

                int indexOfSlash = parcelUrl.lastIndexOf("/");
                name = parcelUrl.substring( indexOfSlash + 1 );

                parcel = new Parcel( m_xSFA, this, pd, name );

                LogUtils.DEBUG(" ParcelContainer.loadParcel created parcel for " + parcelDescUrl + " for language " + language );
                parcels.add( parcel );
            }
            else
            {
                throw new java.io.IOException( parcelDescUrl + " does NOT exist!");
            }
        }
        catch ( com.sun.star.ucb.CommandAbortedException e )
        {

            LogUtils.DEBUG("loadParcel() Exception while accessing filesystem url = " + parcelDescUrl + e );
            throw  new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
        catch ( java.io.IOException e )
        {
            LogUtils.DEBUG("ParcelContainer.loadParcel() caught IOException while accessing " + parcelDescUrl + ": " + e );
            throw  new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
        catch (  com.sun.star.uno.Exception e )
        {

            LogUtils.DEBUG("loadParcel() Exception while accessing filesystem url = " + parcelDescUrl + e );
            throw  new com.sun.star.lang.WrappedTargetException( e.toString() );
        }

        finally
        {
            if ( is != null )
            {
                try
                {
                    is.close(); // is will close xis
                }
                catch ( Exception ignore )
                {
                }
            }
            else if ( xis != null )
            {
                try
                {
                    xis.closeInput();
                }
                catch ( Exception ignore )
                {
                }
            }
        }
        return parcel;
    }
    public void renameParcel(String oldName, String newName) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG(" ** ParcelContainer Renaming parcel " + oldName + " to " + newName );
        LogUtils.DEBUG(" ** ParcelContainer is " + this );
        Parcel p = (Parcel)getByName( oldName );
        if ( p == null )
        {
            throw new com.sun.star.container.NoSuchElementException( "No parcel named " + oldName );
        }
        String oldParcelDirUrl = PathUtils.make_url( getParcelContainerDir(),
            oldName );
        String newParcelDirUrl = PathUtils.make_url( getParcelContainerDir(),
            newName );
        try
        {
            if (!m_xSFA.isFolder( oldParcelDirUrl ) )
            {
                Exception e = new com.sun.star.io.IOException("Invalid Parcel directory: " + oldName );
                throw new com.sun.star.lang.WrappedTargetException( e.toString() );
            }
            LogUtils.DEBUG(" ** ParcelContainer Renaming folder " + oldParcelDirUrl + " to " + newParcelDirUrl );
            m_xSFA.move( oldParcelDirUrl, newParcelDirUrl );
        }
        catch ( com.sun.star.ucb.CommandAbortedException ce )
        {
            LogUtils.DEBUG(" ** ParcelContainer Renaming failed with " + ce  );
            throw new com.sun.star.lang.WrappedTargetException( ce.toString() );
        }
        catch ( com.sun.star.uno.Exception e )
        {
            LogUtils.DEBUG(" ** ParcelContainer Renaming failed with " + e  );
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }

        p.rename( newName );
    }
    // removes but doesn't physically delele parcel from container
    public boolean removeParcel(String name) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        boolean result = false;
        Parcel p = (Parcel)getByName( name );
        if ( p == null )
        {
            throw new com.sun.star.container.NoSuchElementException("No parcel named " + name );
        }

        result =  parcels.remove( p );
        return result;
    }
    public boolean deleteParcel(String name) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG( "deleteParcel for containerURL " + containerUrl + " name = " + name  + " Langueg = " + language );
        boolean result = false;

        Parcel p = (Parcel)getByName( name );
        if ( p == null )
        {
            throw new com.sun.star.container.NoSuchElementException("No parcel named " + name );
        }

        try
        {
            String pathToParcel =  PathUtils.make_url( getParcelContainerDir(),   name );
            m_xSFA.kill( pathToParcel );
        }
        catch( Exception e )
        {
            LogUtils.DEBUG("Error deleteing parcel " + name );
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }

        result =  parcels.remove( p );
        return result;
    }

    public String getLanguage() { return language; }

    public ScriptMetaData findScript( ParsedScriptUri  parsedUri ) throws  com.sun.star.container.NoSuchElementException,  com.sun.star.lang.WrappedTargetException
    {
        ScriptMetaData scriptData = null;
        Parcel p = null;
        p = (Parcel)getByName( parsedUri.parcel);
        scriptData = (ScriptMetaData)p.getByName( parsedUri.function);
        LogUtils.DEBUG("** found script data for " +  parsedUri.function + " script is " + scriptData );
        return scriptData;

    }
public  ParsedScriptUri parseScriptUri( String scriptURI ) throws com.sun.star.lang.IllegalArgumentException
{

        XMultiComponentFactory xMcFac = null;
        XUriReferenceFactory xFac = null;

        try
        {
            xMcFac = m_xCtx.getServiceManager();
            xFac = UnoRuntime.queryInterface( XUriReferenceFactory.class,
                xMcFac.createInstanceWithContext(
                    "com.sun.star.uri.UriReferenceFactory", m_xCtx ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            LogUtils.DEBUG("Problems parsing  URL:" + e.toString() );
            throw new  com.sun.star.lang.IllegalArgumentException( "Problems parsing  URL reason: " + e.toString() );
        }
        if ( xFac == null )
        {
            LogUtils.DEBUG("Failed to create UrlReference factory");
            throw new  com.sun.star.lang.IllegalArgumentException( "Failed to create UrlReference factory for url " + scriptURI );
        }

        XUriReference uriRef = xFac.parse( scriptURI );
        XVndSunStarScriptUrl  sfUri = UnoRuntime.queryInterface( XVndSunStarScriptUrl.class, uriRef );

        if ( sfUri == null )
        {
            LogUtils.DEBUG("Failed to parse url");
            throw new  com.sun.star.lang.IllegalArgumentException( "Failed to parse url " + scriptURI );
        }

        ParsedScriptUri parsedUri = new ParsedScriptUri();
        // parse language
        parsedUri.language = sfUri.getParameter("language");
        parsedUri.function= sfUri.getName();
        parsedUri.parcel = "";

        // parse parcel name;
        StringTokenizer tokenizer = new StringTokenizer( parsedUri.function, "." );

        if ( tokenizer.hasMoreElements() )
        {
            parsedUri.parcel = (String)tokenizer.nextElement();
            LogUtils.DEBUG("** parcelName = " + parsedUri.parcel );
        }

        if ( parsedUri.function != null && ( parsedUri.function.length() > 0 ) )
        {
            // strip out parcel name
            parsedUri.function = parsedUri.function.substring( parsedUri.parcel.length() + 1);
        }

        // parse location
        parsedUri.location = sfUri.getParameter("location");

        // TODO basic sanity check on language, location, functioname, parcel
        // should be correct e.g. verified  by MSP and LangProvider by the
        // time its got to here

        LogUtils.DEBUG("** location = " + parsedUri.location +
            "\nfunction = " + parsedUri.function +
            "\nparcel = " + parsedUri.parcel +
            "\nlocation = " + parsedUri.location );
        return parsedUri;
}


}
