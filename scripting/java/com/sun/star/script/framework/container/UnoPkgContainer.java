/*************************************************************************
 *
 *  $RCSfile: UnoPkgContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:59:58 $
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

import drafts.com.sun.star.script.provider.XScriptContext;

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
        LogUtils.DEBUG("RegisterPackage ctx = " + containerUrl  );
        LogUtils.DEBUG("RegisterPackage language = " + language  );
        LogUtils.DEBUG("RegisterPackage " + c + " for url " + url );
        registeredPackages.put( url, c );
    }

    public void deRegisterPackageContainer( String url )
    {
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
            db =  getUnoPackagesDB();

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

        String pkgLocationUrl = convertPkgLocation( location );

        LogUtils.DEBUG("** pkg location = " + pkgLocationUrl +
            "\nfunction = " + functionName +
            "\nlocation = " + location +
            "\nparcel = " + parcelName );

        ParcelContainer pc =
              pc = getChildContainer( pkgLocationUrl );


        if (  pc  == null )
        {
            throw new com.sun.star.lang.WrappedTargetException( "Failed to resolve script " , null, new com.sun.star.lang.IllegalArgumentException( "Cannot resolve script location for script = " + functionName ) );
        }

        Parcel p = null;
        p = (Parcel)pc.getByName( parcelName );
        scriptData = (ScriptMetaData)p.getByName( functionName );
        LogUtils.DEBUG("** found script data for  " + functionName + " script is " + scriptData );
        return scriptData;

    }

    private String convertPkgLocation( String str )
    {
        int indexOfUnoPackage = str.indexOf( ":uno_packages/" );
        StringBuffer buf = new StringBuffer( 90 );
        if ( indexOfUnoPackage == -1 )
            return null;

        String key =  str.substring( 0, indexOfUnoPackage );
        if ( key.equals("user") )
        {
            buf.append( "vnd.sun.star.pkg://vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE%2Funo_packages%2F" );
        }
        else if ( key.equals("share") )
        {
            buf.append( "vnd.sun.star.pkg://vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE%2Funo_packages%2F" );
        }
        else
        {
            buf.append("DON'T_KNOW_WHAT_TO_DO_WITH_DOC_URL_YET");
        }
        String restOfPath = str.substring( indexOfUnoPackage + ":uno_packages/".length() );
        buf.append( restOfPath );
        return buf.toString();
    }


    private DeployedUnoPackagesDB getUnoPackagesDB() throws com.sun.star.lang.WrappedTargetException
    {
        InputStream is = null;
        DeployedUnoPackagesDB dp = null;
        try
        {
            LogUtils.DEBUG("getUnoPackagesDB()" );

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
                LogUtils.DEBUG("getUnoPackagesDB() " + packagesUrl + " does not exist ");
                dp = new DeployedUnoPackagesDB();
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
        LogUtils.DEBUG("** processUnoPackage getURL() -> " + uri );
        LogUtils.DEBUG("** processUnoPackage getName() -> " + dPackage.getName() );
        LogUtils.DEBUG("** processUnoPackage getMediaType() -> " + dPackage.getMediaType() );
        LogUtils.DEBUG("** processUnoPackage getDisplayName() -> " + dPackage.getDisplayName() );
        processUnoPackage( uri, language );
        db = getUnoPackagesDB();
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


        if ( uri.indexOf( "%2Funo_packages%2F" ) > -1 )
        {
            //its in a bundle need to determine the uno-package file its in
            LogUtils.DEBUG("processUnoPackage - is part of a uno bundle");
            int index = uri.lastIndexOf("/");
            if ( index > -1 )
            {
                parentUrl = uri.substring( 0, index  );
                LogUtils.DEBUG("processUnoPackage - composition is contained in " + parentUrl);
            }

            ParcelContainer pkgContainer = getChildContainer( parentUrl );
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
