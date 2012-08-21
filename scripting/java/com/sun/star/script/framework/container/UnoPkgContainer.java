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

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.PathUtils;
import com.sun.star.script.framework.io.XOutputStreamWrapper;
import com.sun.star.script.framework.io.XInputStreamWrapper;

import java.util.Map;
import java.util.HashMap;
import java.io.OutputStream;
import java.io.InputStream;

import com.sun.star.uno.XComponentContext;

import com.sun.star.uno.UnoRuntime;

import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;

import com.sun.star.deployment.XPackage;

public class UnoPkgContainer extends ParcelContainer
{

    private Map<String,ParcelContainer> registeredPackages = new HashMap<String,ParcelContainer>();
    protected String extensionDb;
    protected String extensionRepository;

    public UnoPkgContainer(  XComponentContext xCtx, String locationURL,
            String _extensionDb, String _extensionRepository, String language ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        super( xCtx, locationURL, language, false );
        extensionDb = _extensionDb;
        extensionRepository = _extensionRepository;
        init();
    }

    // gets the ParcelContainer for persisted uno packages
    public ParcelContainer getRegisteredUnoPkgContainer( String url )
    {
        if (!url.endsWith("/"))
        {
            url += "/";
        }

        LogUtils.DEBUG("** getRegisterPackage ctx = " + containerUrl  );
        LogUtils.DEBUG("** getRegisterPackage  for uri " + url );
        LogUtils.DEBUG("** getRegisterPackage  for langugage " + language );
        ParcelContainer result = registeredPackages.get( url );
        LogUtils.DEBUG("getRegisterPackage result is  " + result );
        return result;
    }

    public boolean hasRegisteredUnoPkgContainer( String url )
    {
        boolean result = false;
        if ( getRegisteredUnoPkgContainer( url ) != null )
        {
            result = true;
        }
        return result;
    }

    private void registerPackageContainer( String url, ParcelContainer c )
    {
        if (!url.endsWith("/"))
        {
            url += "/";
        }

        LogUtils.DEBUG("RegisterPackage ctx = " + containerUrl  );
        LogUtils.DEBUG("RegisterPackage language = " + language  );
        LogUtils.DEBUG("RegisterPackage " + c + " for url " + url );
        registeredPackages.put( url, c );
    }

    public void deRegisterPackageContainer( String url )
    {
        if (!url.endsWith("/"))
        {
            url += "/";
        }

        LogUtils.DEBUG("In deRegisterPackageContainer for " + url );
        if ( hasRegisteredUnoPkgContainer( url ) )
        {
            try
            {
                DeployedUnoPackagesDB db = getUnoPackagesDB();
                if ( db != null )
                {
                    if ( db.removePackage( language, url ) )
                    {
                        writeUnoPackageDB( db );
                        ParcelContainer container = registeredPackages.get( url );
                        if ( !container.hasElements() )
                        {
                           // When all libraries within a package bundle
                           // ( for this language ) are removed also
                           // remove the container from its parent
                           // Otherwise, a container ( with no containees )
                           // representing the uno package bundle will
                           // still exist and so will get displayed
                           if ( container.parent() != null )
                           {
                               container.parent().removeChildContainer( container );
                           }
                       }
                       registeredPackages.remove( url );
                   }
               }
           }
           catch (Exception e)
           {
               //TODO revisit exception handling and exception here
               //means something very wrong
               LogUtils.DEBUG("***** deRegisterPackageContainer() got exception " + e );
           }
        }
        LogUtils.DEBUG("Leaving deRegisterPackageContainer for " + url );
    }

    private void init() throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG("getting container for  " + containerUrl );
        DeployedUnoPackagesDB db =  null;
        try
        {
            db = getUnoPackagesDB();
            if ( db != null )
            {
                String[] packages = db.getDeployedPackages( language );

                for ( int i=0; i<packages.length;i++)
                {
                    try
                    {
                        processUnoPackage( packages[i], language );
                    }
                    catch ( com.sun.star.lang.IllegalArgumentException ila)
                    {
                        LogUtils.DEBUG("Failed to process " +  packages[i] + " for " + language);
                        LogUtils.DEBUG("   Reason: " + ila );
                    }
                    catch( Exception e )
                    {
                        // TODO proper exception or do we wish
                        // to ignore errors here
                        LogUtils.DEBUG("Something very wrong!!!!!");
                        LogUtils.DEBUG("Failed to process " +  packages[i] + " for " + language);
                        LogUtils.DEBUG("   Reason: " + e );
                    }
                }
            }
        }
        catch ( com.sun.star.lang.WrappedTargetException e )
        {
            // no deployed packages
           LogUtils.DEBUG("No deployed uno-packages for " + containerUrl );
        }
    }


    public ScriptMetaData findScript( ParsedScriptUri psu ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException

    {
        ScriptMetaData scriptData = null;

        String language = psu.language;
        String functionName = psu.function;
        String parcelName = psu.parcel;
        String location = psu.location;

        LogUtils.DEBUG("*** UnoPkgContainer.findScript() ***" +
            "\ncontainerUrl = " + containerUrl +
            "\nfunction = " + functionName +
            "\nlocation = " + location +
            "\nparcel = " + parcelName );

        ParcelContainer pc = getChildContainer( location );

        if (  pc  == null )
        {
            throw new com.sun.star.lang.WrappedTargetException( "Failed to resolve script " , null, new com.sun.star.lang.IllegalArgumentException( "Cannot resolve script location for script = " + functionName ) );
        }

        scriptData = pc.findScript( psu );
        return scriptData;

    }

    private DeployedUnoPackagesDB getUnoPackagesDB() throws com.sun.star.lang.WrappedTargetException
    {
        InputStream is = null;
        DeployedUnoPackagesDB dp = null;
        try
        {
//            String path = containerUrl.substring( 0, containerUrl.lastIndexOf("/") );
            String packagesUrl = PathUtils.make_url( extensionDb, "/Scripts/" + extensionRepository + "-extension-desc.xml" );
            LogUtils.DEBUG("getUnoPackagesDB() looking for existing db in " + packagesUrl );
            if ( m_xSFA.exists( packagesUrl ) )
            {
                if ( packagesUrl.startsWith( "vnd.sun.star.tdoc" ) )
                {
                    // handles using XStorage directly
                    throw new com.sun.star.lang.WrappedTargetException("Can't handle documents yet");
                }

                is = new XInputStreamWrapper( m_xSFA.openFileRead( packagesUrl ) );
                dp = new DeployedUnoPackagesDB( is );
                try
                {
                    is.close();
                    is = null;
                }
                catch ( Exception ignore )
                {
                }
            }
            else
            {
                LogUtils.DEBUG("getUnoPackagesDB() " + packagesUrl + " does not exist");
            }
        }
        catch( Exception e )
        {
            LogUtils.DEBUG("getUnoPackagesDB() caught Exception: " + e  );
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
            throw new com.sun.star.lang.WrappedTargetException( e.toString());
        }
        finally
        {
            if ( is != null )
            {
                try
                {
                    is.close();
                    is = null;
                }
                catch ( Exception ignore )
                {
                }
            }
        }

        return dp;
    }

    private  void writeUnoPackageDB( DeployedUnoPackagesDB dp ) throws com.sun.star.lang.IllegalArgumentException,  com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG("In writeUnoPackageDB() ");

        XOutputStream xos = null;
        OutputStream os = null;
        try
        {
//            String path = containerUrl.substring( 0, containerUrl.lastIndexOf("/") );
            String packagesUrl = PathUtils.make_url( extensionDb, "/Scripts/" + extensionRepository + "-extension-desc.xml" );
            xos =  m_xSFA.openFileWrite( packagesUrl );
            XTruncate xTrc = UnoRuntime.queryInterface( XTruncate.class, xos );
            if (  xTrc != null )
            {
                LogUtils.DEBUG("In writeUnoPackageDB() Truncating...." );
                xTrc.truncate();
            }
            else
            {
                LogUtils.DEBUG("In writeUnoPackageDB() CANT Truncate...." );
            }
            os = new XOutputStreamWrapper( xos );
            dp.write( os );
            try
            {
                os.close(); // will close xos
                os = null;
            }
            catch( Exception ignore )
            {
            }
        }
        catch( Exception e )
        {
            LogUtils.DEBUG("In writeUnoPackageDB() Exception: " + e  );
            throw new com.sun.star.lang.WrappedTargetException( e.toString());
        }
        finally
        {
            if ( os != null )
            {
                try
                {
                    os.close(); // will close xos
                    os = null;
                }
                catch ( Exception ignore )
                {
                }
            }
        }
    }

    public  void processUnoPackage( XPackage dPackage, String language ) throws com.sun.star.lang.IllegalArgumentException,  com.sun.star.lang.WrappedTargetException, com.sun.star.container.ElementExistException
    {
        LogUtils.DEBUG("** in processUnoPackage " );
        String uri = null;
        DeployedUnoPackagesDB db = null;
        uri =  dPackage.getURL();

        if ( !uri.endsWith( "/" ) )
        {
            uri += "/";
        }

        LogUtils.DEBUG("** processUnoPackage getURL() -> " + uri );
        LogUtils.DEBUG("** processUnoPackage getName() -> " + dPackage.getName() );
        LogUtils.DEBUG("** processUnoPackage getMediaType() -> " + dPackage.getPackageType().getMediaType() );
        try
        {
            LogUtils.DEBUG("** processUnoPackage getDisplayName() -> " + dPackage.getDisplayName() );
        }
        catch (com.sun.star.deployment.ExtensionRemovedException e)
        {
            throw new com.sun.star.lang.WrappedTargetException(e.toString(), this, e);
        }

        processUnoPackage( uri, language );

        db = getUnoPackagesDB();
        if ( db == null )
        {
            try
            {
                db = new DeployedUnoPackagesDB();
            }
            catch ( java.io.IOException ioe )
            {
                throw new com.sun.star.lang.WrappedTargetException( ioe.toString());
            }
        }
        db.addPackage( language, uri );
        writeUnoPackageDB( db );
    }

    private  void processUnoPackage( String uri, String language ) throws com.sun.star.lang.IllegalArgumentException,  com.sun.star.lang.WrappedTargetException, com.sun.star.container.ElementExistException
    {
        if ( hasRegisteredUnoPkgContainer( uri ) )
        {
            throw new com.sun.star.container.ElementExistException( "Already a registered uno package " + uri + " for language " + language );
        }
        LogUtils.DEBUG("processUnoPackage - URL = " + uri );
        LogUtils.DEBUG("processUnoPackage - script library package");
        String parentUrl = uri;

        if ( uri.indexOf( "%2Funo_packages%2F" ) > -1 ||
             uri.indexOf( "/uno_packages/" ) > -1 ||
             uri.indexOf("$UNO_USER_PACKAGES_CACHE/") > -1 ||
             uri.indexOf("$UNO_SHARED_PACKAGES_CACHE/") > -1 ||
             uri.indexOf("$BUNDLED_EXTENSIONS/") > -1 )
        {
            //its in a bundle need to determine the uno-package file its in
            LogUtils.DEBUG("processUnoPackage - is part of a uno bundle");

            int index = uri.lastIndexOf("/");
            if ( uri.endsWith("/") )
            {
                uri = uri.substring( 0, index );
                index = uri.lastIndexOf("/");
            }

            if ( index > -1 )
            {
                parentUrl = uri.substring( 0, index  );
                LogUtils.DEBUG("processUnoPackage - composition is contained in " + parentUrl);
            }

            ParcelContainer pkgContainer = getChildContainerForURL( parentUrl );
            if ( pkgContainer == null )
            {
                pkgContainer = new ParcelContainer( this, m_xCtx, parentUrl, language, false );
                if ( pkgContainer.loadParcel( uri ) == null )
                {
                    throw new com.sun.star.lang.IllegalArgumentException( "Couldn't load script library from composition package " + uri + " for language " + language );

                }
                addChildContainer( pkgContainer );
            }
            else
            {
                if ( pkgContainer.loadParcel( uri ) == null )
                {
                    throw new com.sun.star.lang.IllegalArgumentException( "Couldn't load script library from composition package " + uri + " for language " + language );
                }

            }
            registerPackageContainer( uri,  pkgContainer );
        }
        else
        {
            // stand-alone library package, e.g. not contained in
            // an uno package
            if ( loadParcel( uri ) == null )
            {
                throw new com.sun.star.lang.IllegalArgumentException( "Couldn't load script library package " + uri + " for language " + language );
            }
            registerPackageContainer( uri, this );
        }

    }

}
