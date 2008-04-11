/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoPkgContainer.java,v $
 * $Revision: 1.9 $
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
package com.sun.star.script.framework.container;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.PathUtils;
import com.sun.star.script.framework.io.XOutputStreamWrapper;
import com.sun.star.script.framework.io.XInputStreamWrapper;

import java.util.Map;
import java.util.HashMap;
import java.util.StringTokenizer;
import java.io.OutputStream;
import java.io.InputStream;

import com.sun.star.uno.XComponentContext;

import com.sun.star.uno.UnoRuntime;

import com.sun.star.frame.XModel;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;

import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;


import com.sun.star.uno.Type;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Any;

import com.sun.star.ucb.Command;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XSimpleFileAccess;

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.Property;

import com.sun.star.sdbc.XRow;

import com.sun.star.deployment.XPackage;
import com.sun.star.deployment.XPackageManager;
import com.sun.star.deployment.XPackageManagerFactory;

import com.sun.star.uno.XComponentContext;

import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.script.provider.XScriptContext;

public class UnoPkgContainer extends ParcelContainer
{

    private Map registeredPackages = new HashMap();

    public UnoPkgContainer(  XComponentContext xCtx, String locationURL, String language ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        super( xCtx, locationURL, language, false );
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
        ParcelContainer result = (ParcelContainer)registeredPackages.get( url );
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
                        ParcelContainer container =
                            ( ParcelContainer ) registeredPackages.get( url );
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
            String path = containerUrl.substring( 0, containerUrl.lastIndexOf("/") );
            String packagesUrl = PathUtils.make_url( path, "Scripts/unopkg-desc.xml" );
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
            String path = containerUrl.substring( 0, containerUrl.lastIndexOf("/") );
            String packagesUrl = PathUtils.make_url( path, "Scripts/unopkg-desc.xml" );
            xos =  m_xSFA.openFileWrite( packagesUrl );
            XTruncate xTrc = (XTruncate) UnoRuntime.queryInterface( XTruncate.class, xos );
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
        LogUtils.DEBUG("** processUnoPackage getDisplayName() -> " + dPackage.getDisplayName() );
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
             uri.indexOf( "/uno_packages/" ) > -1 )
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
