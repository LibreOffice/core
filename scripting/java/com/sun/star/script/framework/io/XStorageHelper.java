/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XStorageHelper.java,v $
 * $Revision: 1.5 $
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

package com.sun.star.script.framework.io;

import com.sun.star.frame.XModel;

import com.sun.star.container.XNameAccess;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XComponent;

import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;



import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;
import com.sun.star.io.XSeekable;

import com.sun.star.embed.XStorage;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XTransactedObject;

import com.sun.star.document.XDocumentSubStorageSupplier;

import com.sun.star.beans.XPropertySet;

import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.PathUtils;

import java.util.*;
import java.io.*;
import java.util.zip.*;


public class XStorageHelper implements XEventListener
{
    XStorage[] xStorages;
    XStream xStream;
    XInputStream xIs = null;
    XOutputStream xOs = null;
    static Map modelMap = new HashMap();
    XModel xModel = null;
    private static XStorageHelper listener = new XStorageHelper();

    private XStorageHelper() {}
    public XStorageHelper(  String path, int mode, boolean create ) throws IOException
    {
        String modelUrl = null;
        int indexOfScriptsDir = path.lastIndexOf( "Scripts" );
        if ( indexOfScriptsDir > -1 )
        {
            modelUrl = path.substring( 0, indexOfScriptsDir - 1 );
            path = path.substring( indexOfScriptsDir, path.length());
        }

        LogUtils.DEBUG("XStorageHelper ctor, path: " + path);
        this.xModel =  getModelForURL( modelUrl );

        try
        {
            StringTokenizer tokens = new StringTokenizer(path, "/");

            if (tokens.countTokens() == 0)
            {
               throw new IOException("Invalid path");
            }
            XDocumentSubStorageSupplier xDocumentSubStorageSupplier =
                (XDocumentSubStorageSupplier) UnoRuntime.queryInterface(
                    XDocumentSubStorageSupplier.class, xModel);
            xStorages =  new XStorage[tokens.countTokens()  ];
            LogUtils.DEBUG("XStorageHelper ctor, path chunks length: " + xStorages.length );

            for ( int i = 0; i < xStorages.length; i++ )
            {
                LogUtils.DEBUG("XStorageHelper, processing index " + i );
                String name = tokens.nextToken();
                LogUtils.DEBUG("XStorageHelper, getting: " + name);
                XStorage storage = null;
                if ( i == 0 )
                {
                    storage  = xDocumentSubStorageSupplier.getDocumentSubStorage( name, mode );
                    if ( storage == null )
                    {
                        LogUtils.DEBUG("** boo hoo Storage is null " );
                    }
                    XPropertySet xProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,storage );
                    if ( xProps != null )
                    {
                        String mediaType = AnyConverter.toString( xProps.getPropertyValue( "MediaType" ) );
                        LogUtils.DEBUG("***** media type is " + mediaType );
                        if ( !mediaType.equals("scripts") )
                        {
                            xProps.setPropertyValue("MediaType","scripts");
                        }
                    }
                }
                else
                {
                    XNameAccess xNameAccess = (XNameAccess)
                    UnoRuntime.queryInterface(XNameAccess.class, xStorages[i-1]);
                    if (xNameAccess == null )
                    {
                        disposeObject();
                        throw new IOException("No name access " + name);
                    }
                    else if ( !xNameAccess.hasByName(name) || !xStorages[i-1].isStorageElement(name) )
                    {
                        if ( !create )
                        {
                            disposeObject();
                            throw new IOException("No subdir: " + name);
                        }
                        else
                        {
                            // attempt to create new storage
                            LogUtils.DEBUG("Attempt to create new storage for " + name );
                        }
                    }

                    storage = xStorages[i-1].openStorageElement(
                        name, mode );
                }
                if ( storage == null )
                {
                    disposeObject();
                    throw new IOException("storage not found: " + name);
                }
                xStorages[ i ] = storage;

            }
        }
        catch ( com.sun.star.io.IOException ioe)
        {
            disposeObject();
        }
        catch (com.sun.star.uno.Exception e)
        {
            disposeObject();
            throw new IOException(e.getMessage());
        }
    }

    public synchronized static void addNewModel( XModel model )
    {
        // TODO needs to cater for model for untitled document
        modelMap.put( PathUtils.getOidForModel( model ), model );
        XComponent xComp = (XComponent)
             UnoRuntime.queryInterface(XComponent.class, model);

        if ( xComp != null )
        {
            try
            {
                xComp.addEventListener( listener );
            }
            catch ( Exception e )
            {
                // What TODO here ?
                LogUtils.DEBUG( LogUtils.getTrace( e ) );
            }
        }
    }

    public void disposing( EventObject Source )
    {
        XModel model = (XModel)
            UnoRuntime.queryInterface(XModel.class,Source.Source );

        if ( model != null )
        {
            LogUtils.DEBUG(" Disposing doc " + model.getURL() );
            Object result = modelMap.remove( model );
            result = null;
        }
    }
    public XStorage getStorage()
    {
        return xStorages[ xStorages.length - 1 ];
    }
    public XModel getModel()
    {
        return xModel;
    }
    public void disposeObject()
    {
        disposeObject( false );
    }
    public void disposeObject( boolean shouldCommit )
    {
        LogUtils.DEBUG("In disposeObject");

        for ( int i = xStorages.length -1 ; i > -1; i-- )
        {
            LogUtils.DEBUG("In disposeObject disposing storage " + i );
            try
            {
                XStorage xStorage = xStorages[i];
                if ( shouldCommit )
                {
                    commit(xStorage);
                }
                disposeObject(xStorage);
                LogUtils.DEBUG("In disposeObject disposed storage " + i );
            }
            catch( Exception ignore )
            {
                LogUtils.DEBUG("Exception disposing storage " + i );
            }

        }

    }
    static public void disposeObject( XInterface xInterface )
    {
        if (xInterface == null) {
            return;
        }

        XComponent xComponent = (XComponent)
        UnoRuntime.queryInterface(XComponent.class, xInterface);

        if (xComponent == null) {
            return;
        }
        xComponent.dispose();
    }
    static public void commit( XInterface xInterface )
    {
        XTransactedObject xTrans = (XTransactedObject)
        UnoRuntime.queryInterface(XTransactedObject.class, xInterface);
        if ( xTrans != null )
        {
            try
            {
                xTrans.commit();
            }
            catch ( Exception e )
            {
                LogUtils.DEBUG("Something went bellyup exception: " + e );
            }
        }
    }

    public XModel getModelForURL( String url )
    {
       //TODO does not cater for untitled documents
       return (XModel)modelMap.get( url );
    }

}

