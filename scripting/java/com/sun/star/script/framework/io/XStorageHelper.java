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

package com.sun.star.script.framework.io;

import com.sun.star.frame.XModel;

import com.sun.star.container.XNameAccess;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XComponent;

import com.sun.star.uno.AnyConverter;

import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;

import com.sun.star.embed.XStorage;
import com.sun.star.embed.XTransactedObject;

import com.sun.star.document.XDocumentSubStorageSupplier;

import com.sun.star.beans.XPropertySet;

import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.PathUtils;

import java.util.*;
import java.io.*;


public class XStorageHelper implements XEventListener
{
    XStorage[] xStorages;
    XStream xStream;
    XInputStream xIs = null;
    XOutputStream xOs = null;
    static Map<String,XModel> modelMap = new HashMap<String,XModel>();
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
                UnoRuntime.queryInterface(
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
                    XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class,storage );
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
                    XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, xStorages[i-1]);
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
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class, model);

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
        XModel model = UnoRuntime.queryInterface(XModel.class,Source.Source );

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

        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, xInterface);

        if (xComponent == null) {
            return;
        }
        xComponent.dispose();
    }
    static public void commit( XInterface xInterface )
    {
        XTransactedObject xTrans = UnoRuntime.queryInterface(XTransactedObject.class, xInterface);
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
       return modelMap.get( url );
    }

}

