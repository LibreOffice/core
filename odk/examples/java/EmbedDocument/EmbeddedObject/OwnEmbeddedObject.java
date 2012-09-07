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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.io.XStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XTruncate;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.embed.VisualRepresentation;
import com.sun.star.container.XNameAccess;


import com.sun.star.embed.*;

import java.util.Vector;
import java.awt.Dimension;
import java.lang.Integer;

public final class OwnEmbeddedObject extends WeakBase
   implements com.sun.star.embed.XEmbedPersist,
              com.sun.star.embed.XEmbeddedObject
{
    protected final XComponentContext m_xContext;
    protected final byte[] m_aClassID;

    protected boolean m_bDisposed = false;
    protected int m_nObjectState = -1;

    protected com.sun.star.embed.XStorage m_xParentStorage;
    protected com.sun.star.embed.XStorage m_xOwnStorage;
    protected String m_aEntryName;

    protected com.sun.star.embed.XStorage m_xNewParentStorage;
    protected com.sun.star.embed.XStorage m_xNewOwnStorage;
    protected String m_aNewEntryName;
    protected boolean m_bWaitSaveCompleted = false;

    protected EditorFrame m_aEditorFrame;

    protected Vector m_aListeners;

    com.sun.star.embed.VerbDescriptor[] m_pOwnVerbs;

    com.sun.star.embed.XEmbeddedClient m_xClient;

    Dimension m_aObjSize;

    // -------------------------------------------------------------
    protected Vector GetListeners()
    {
        if ( m_aListeners == null )
            m_aListeners = new Vector<Object>( 10, 10 );

        return m_aListeners;
    }

    // -------------------------------------------------------------
    protected Dimension UpdateSizeAndGetFromActive()
    {
        if ( m_nObjectState == com.sun.star.embed.EmbedStates.ACTIVE )
            m_aObjSize = m_aEditorFrame.getAppSize();

        if ( m_aObjSize != null )
            return m_aObjSize;
        else
            return new Dimension();
    }

    // -------------------------------------------------------------
    protected void SwitchOwnPersistence( XStorage xParentStorage, XStorage xOwnStorage, String aEntryName )
    {
        if ( xOwnStorage != m_xOwnStorage )
        {
            if ( m_xOwnStorage != null )
                m_xOwnStorage.dispose();
            m_xParentStorage = xParentStorage;
            m_xOwnStorage = xOwnStorage;
            m_aEntryName = aEntryName;
        }
    }

    // -------------------------------------------------------------
    protected void SwitchOwnPersistence( XStorage xParentStorage, String aEntryName ) throws com.sun.star.io.IOException
    {
        if ( xParentStorage != m_xParentStorage || !aEntryName.equals( m_aEntryName ) )
        {
            try
            {
                XStorage xOwnStorage = xParentStorage.openStorageElement( aEntryName, com.sun.star.embed.ElementModes.READWRITE );
                SwitchOwnPersistence( xParentStorage, xOwnStorage, aEntryName );
            }
            catch( com.sun.star.uno.RuntimeException e )
            {
                throw e;
            }
            catch( com.sun.star.io.IOException e )
            {
                throw e;
            }
            catch( com.sun.star.uno.Exception e )
            {
                throw new com.sun.star.io.IOException( "Error while switching object storage!" );
            }
        }
    }

    // -------------------------------------------------------------
    protected static void SaveDataToStorage( XStorage xStorage, String aString, Dimension aDimension ) throws com.sun.star.io.IOException
    {
        try
        {
            // save the text
            XStream xStream = xStorage.openStreamElement( "content.txt", com.sun.star.embed.ElementModes.READWRITE );
            XComponent xStreamComp = ( XComponent ) UnoRuntime.queryInterface( XComponent.class, xStream );
            if ( xStreamComp == null )
                throw new com.sun.star.uno.RuntimeException();

            XOutputStream xOutStream = xStream.getOutputStream();
            XTruncate xTruncate = ( XTruncate ) UnoRuntime.queryInterface( XTruncate.class, xOutStream );
            if ( xTruncate == null )
                throw new com.sun.star.io.IOException();

            xTruncate.truncate();
            xOutStream.writeBytes( aString.getBytes() );

            // save the size
            xStream = xStorage.openStreamElement( "properties.txt", com.sun.star.embed.ElementModes.READWRITE );
            xStreamComp = ( XComponent ) UnoRuntime.queryInterface( XComponent.class, xStream );
            if ( xStreamComp == null )
                throw new com.sun.star.uno.RuntimeException();

            xOutStream = xStream.getOutputStream();
            xTruncate = ( XTruncate ) UnoRuntime.queryInterface( XTruncate.class, xOutStream );
            if ( xTruncate == null )
                throw new com.sun.star.io.IOException();

            xTruncate.truncate();
            String aProps = Integer.toString( (int)aDimension.getWidth() ) + "|" + Integer.toString( (int)aDimension.getHeight() );
            xOutStream.writeBytes( aProps.getBytes() );

            // set the media type
            XPropertySet xPropSet = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, xStorage );
            if ( xPropSet == null )
                throw new com.sun.star.uno.RuntimeException();
            xPropSet.setPropertyValue( "MediaType", "application/x-openoffice-embedded-69474366-FD6F-4806-8374-8EDD1B6E771D" );

            XTransactedObject xTransact = ( XTransactedObject ) UnoRuntime.queryInterface( XTransactedObject.class, xStorage );
            if ( xTransact != null )
                xTransact.commit();

            xStreamComp.dispose();
        }
        catch( com.sun.star.uno.RuntimeException e )
        {
            throw e;
        }
        catch( com.sun.star.io.IOException e )
        {
            throw e;
        }
        catch( com.sun.star.uno.Exception e )
        {
            throw new com.sun.star.io.IOException( "Error while switching object storage!" );
        }
    }

    // -------------------------------------------------------------
    protected void PostEvent( String aEvEntryName )
    {
        if ( m_aListeners != null )
        {
            com.sun.star.document.EventObject aEventObject = new com.sun.star.document.EventObject( this, aEvEntryName );
            for ( int nInd = 0; nInd < m_aListeners.size(); nInd++ )
            {
                try
                {
                    com.sun.star.document.XEventListener xListener = ( com.sun.star.document.XEventListener )
                        UnoRuntime.queryInterface( com.sun.star.document.XEventListener.class, m_aListeners.get( nInd ) );

                    if ( xListener != null )
                        xListener.notifyEvent( aEventObject );
                }
                catch( java.lang.Exception e )
                {
                    m_aListeners.remove( nInd );
                }
            }
        }
    }

    // -------------------------------------------------------------
    protected void StateChangeNotification( boolean bBeforeChange, int nOldState, int nNewState )
    {
        if ( m_aListeners != null )
        {
            com.sun.star.lang.EventObject aEventObject = new com.sun.star.lang.EventObject( this );
            for ( int nInd = 0; nInd < m_aListeners.size(); nInd++ )
            {
                try
                {
                    com.sun.star.embed.XStateChangeListener xListener = ( com.sun.star.embed.XStateChangeListener )
                        UnoRuntime.queryInterface( com.sun.star.embed.XStateChangeListener.class, m_aListeners.get( nInd ) );

                    if ( xListener != null )
                    {
                        if ( bBeforeChange )
                            xListener.changingState( aEventObject, nOldState, nNewState );
                        else
                            xListener.stateChanged( aEventObject, nOldState, nNewState );
                    }
                }
                catch( java.lang.Exception e )
                {
                    m_aListeners.remove( nInd );
                }
            }
        }
    }

    // -------------------------------------------------------------
    protected String ReadStringFromStream( XStorage xStorage, String aStreamName ) throws com.sun.star.io.IOException
    {
        if ( xStorage == null )
            throw new com.sun.star.uno.RuntimeException();

        try
        {
            XStream xStream = xStorage.openStreamElement( aStreamName, com.sun.star.embed.ElementModes.READWRITE );
            XComponent xStreamComp = ( XComponent ) UnoRuntime.queryInterface( XComponent.class, xStream );
            if ( xStreamComp == null )
                throw new com.sun.star.uno.RuntimeException();

            XInputStream xInStream = xStream.getInputStream();
            byte[][] aData = new byte[1][];
            aData[0] = new byte[0];
            String aResult = new String();

            int nLen = 0;
            do
            {
                nLen = xInStream.readBytes( aData, 10 );
                if ( aData.length == 0 || aData[0] == null )
                    throw new com.sun.star.io.IOException();
                aResult += new String( aData[0] );
            } while( nLen > 0 );

            xStreamComp.dispose();

            return aResult;
        }
        catch( com.sun.star.uno.RuntimeException e )
        {
            throw e;
        }
        catch( com.sun.star.io.IOException e )
        {
            throw e;
        }
        catch( com.sun.star.uno.Exception e )
        {
            throw new com.sun.star.io.IOException( "Error while reading one of object streams!" );
        }
    }

    // -------------------------------------------------------------
    protected void ReadSizeFromOwnStorage() throws com.sun.star.io.IOException
    {
        String aSize = ReadStringFromStream( m_xOwnStorage, "properties.txt" );

        int nSeparator = aSize.indexOf( '|' );
        if ( nSeparator > 0 && nSeparator < aSize.length() - 1 )
        {
            int nWidth = Integer.parseInt( aSize.substring( 0, nSeparator ) );
            int nHeight = Integer.parseInt( aSize.substring( nSeparator + 1, aSize.length() ) );
            m_aObjSize = new Dimension( nWidth, nHeight );
        }
    }

    // -------------------------------------------------------------
    public OwnEmbeddedObject( XComponentContext context, byte[] aClassID )
    {
        m_xContext = context;
        m_aClassID = aClassID;
    };

    // -------------------------------------------------------------
    public void CloseFrameRequest()
    {
        com.sun.star.embed.XEmbeddedClient xClient = m_xClient;
        if ( xClient == null )
            return;

        UpdateSizeAndGetFromActive();
        StateChangeNotification( true, com.sun.star.embed.EmbedStates.ACTIVE, com.sun.star.embed.EmbedStates.RUNNING );

        try{
            xClient.visibilityChanged( false );
        } catch( com.sun.star.uno.Exception e ){}

        try{
            xClient.saveObject();
        } catch( com.sun.star.uno.Exception e ){}

        m_nObjectState = com.sun.star.embed.EmbedStates.RUNNING;
        StateChangeNotification( false, com.sun.star.embed.EmbedStates.ACTIVE, m_nObjectState );

        PostEvent( "OnVisAreaChanged" );
    }

    // com.sun.star.embed.XCommonEmbedPersist:
    // -------------------------------------------------------------
    public void storeOwn() throws com.sun.star.embed.WrongStateException, com.sun.star.io.IOException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 || m_bWaitSaveCompleted )
            throw new com.sun.star.embed.WrongStateException();

        // nothing to do, if the object is in loaded state
        if ( m_nObjectState == com.sun.star.embed.EmbedStates.LOADED )
            return;

        if ( m_xOwnStorage == null )
            throw new com.sun.star.io.IOException();

        PostEvent( "OnSave" );

        if ( m_aEditorFrame == null )
            throw new com.sun.star.uno.RuntimeException();

        SaveDataToStorage( m_xOwnStorage, m_aEditorFrame.getText(), UpdateSizeAndGetFromActive() );

        PostEvent( "OnSaveDone" );
    }

    // -------------------------------------------------------------
    public boolean isReadonly() throws com.sun.star.embed.WrongStateException
    {
        return false;
    }

    // -------------------------------------------------------------
    public void reload(com.sun.star.beans.PropertyValue[] aMediaArgs, com.sun.star.beans.PropertyValue[] aObjectArgs) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.io.IOException, com.sun.star.uno.Exception
    {
        // not implemented currently
        return;
    }

    // com.sun.star.embed.XEmbedPersist:
    // -------------------------------------------------------------
    public void setPersistentEntry(com.sun.star.embed.XStorage xStorage, String aEntryName, int nEntryConnectionMode, com.sun.star.beans.PropertyValue[] aMediaArgs, com.sun.star.beans.PropertyValue[] aObjectArgs) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.io.IOException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( xStorage == null || aEntryName.length() == 0 )
            throw new com.sun.star.lang.IllegalArgumentException();

        if ( ( m_nObjectState != -1 || nEntryConnectionMode == com.sun.star.embed.EntryInitModes.NO_INIT )
          && ( m_nObjectState == -1 || nEntryConnectionMode != com.sun.star.embed.EntryInitModes.NO_INIT ) )
        {
            // if the object is not loaded
            // it can not get persistant representation without initialization

            // if the object is loaded
            // it can switch persistant representation only without initialization

            throw new com.sun.star.embed.WrongStateException();
        }

        if ( m_bWaitSaveCompleted )
        {
            if ( nEntryConnectionMode == com.sun.star.embed.EntryInitModes.NO_INIT )
            {
                if ( m_xParentStorage == xStorage && m_aEntryName.equals( aEntryName ) )
                    saveCompleted( false );
                else if ( m_xNewParentStorage == xStorage && m_aNewEntryName.equals( aEntryName ) )
                    saveCompleted( true );
                else
                    throw new com.sun.star.embed.WrongStateException();
            }
            else
                throw new com.sun.star.embed.WrongStateException();

            return;
        }

        boolean bElExists = xStorage.hasByName( aEntryName );

        if ( nEntryConnectionMode == com.sun.star.embed.EntryInitModes.DEFAULT_INIT )
        {
            SwitchOwnPersistence( xStorage, aEntryName );
            if ( bElExists )
            {
                XPropertySet xPropSet = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, m_xOwnStorage );
                if ( xPropSet == null )
                    throw new com.sun.star.uno.RuntimeException();

                String aMediaType = AnyConverter.toString( xPropSet.getPropertyValue( "MediaType" ) );
                if ( !aMediaType.equals( "application/x-openoffice-embedded-69474366-FD6F-4806-8374-8EDD1B6E771D" ) )
                    throw new com.sun.star.lang.IllegalArgumentException();

                m_nObjectState = com.sun.star.embed.EmbedStates.LOADED;
            }
            else
            {
                m_aEditorFrame = new EditorFrame( m_aEntryName, this, 5, 20 );
                m_nObjectState = com.sun.star.embed.EmbedStates.RUNNING;
                m_aObjSize = m_aEditorFrame.getAppSize();
            }
        }
        else if ( nEntryConnectionMode == com.sun.star.embed.EntryInitModes.TRUNCATE_INIT )
        {
            SwitchOwnPersistence( xStorage, aEntryName );
            m_aEditorFrame = new EditorFrame( m_aEntryName, this, 5, 20 );
            m_nObjectState = com.sun.star.embed.EmbedStates.RUNNING;
            m_aObjSize = m_aEditorFrame.getAppSize();
        }
        else
            throw new com.sun.star.lang.IllegalArgumentException();
    }

    // -------------------------------------------------------------
    public void storeToEntry(com.sun.star.embed.XStorage xStorage, String aEntryName, com.sun.star.beans.PropertyValue[] aMediaArgs, com.sun.star.beans.PropertyValue[] aObjectArgs) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.io.IOException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 || m_bWaitSaveCompleted )
            throw new com.sun.star.embed.WrongStateException();

        if ( m_nObjectState == com.sun.star.embed.EmbedStates.LOADED )
        {
            m_xParentStorage.copyElementTo( m_aEntryName, xStorage, aEntryName );
        }
        else
        {
            com.sun.star.embed.XStorage xSubStorage =
                xStorage.openStorageElement( aEntryName,
                            com.sun.star.embed.ElementModes.READWRITE | com.sun.star.embed.ElementModes.TRUNCATE );

            String aContents = m_aEditorFrame.getText();

            SaveDataToStorage( xSubStorage, m_aEditorFrame.getText(), UpdateSizeAndGetFromActive() );
        }
    }

    // -------------------------------------------------------------
    public void storeAsEntry(com.sun.star.embed.XStorage xStorage, String aEntryName, com.sun.star.beans.PropertyValue[] aMediaArgs, com.sun.star.beans.PropertyValue[] aObjectArgs) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.io.IOException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 || m_bWaitSaveCompleted )
            throw new com.sun.star.embed.WrongStateException();

        com.sun.star.embed.XStorage xSubStorage = null;

        if ( m_nObjectState == com.sun.star.embed.EmbedStates.LOADED )
        {
            xSubStorage =
                xStorage.openStorageElement( aEntryName,
                            com.sun.star.embed.ElementModes.READWRITE | com.sun.star.embed.ElementModes.NOCREATE );

            m_xOwnStorage.copyToStorage( xSubStorage );
        }
        else
        {
            xSubStorage =
                xStorage.openStorageElement( aEntryName,
                            com.sun.star.embed.ElementModes.READWRITE | com.sun.star.embed.ElementModes.TRUNCATE );

            SaveDataToStorage( xSubStorage, m_aEditorFrame.getText(), UpdateSizeAndGetFromActive() );
        }

        m_bWaitSaveCompleted = true;
        m_xNewOwnStorage = xSubStorage;
        m_xNewParentStorage = xStorage;
        m_aNewEntryName = aEntryName;

    }

    // -------------------------------------------------------------
    public void saveCompleted(boolean bUseNew) throws com.sun.star.embed.WrongStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        // it is allowed to call saveCompleted( false ) for nonstored objects
        if ( !m_bWaitSaveCompleted && !bUseNew )
            return;

        if ( !m_bWaitSaveCompleted )
            throw new com.sun.star.io.IOException();

        if ( bUseNew )
        {
            SwitchOwnPersistence( m_xNewParentStorage, m_xNewOwnStorage, m_aNewEntryName );
            PostEvent( "OnSaveAsDone" );
        }
        else
        {
            try
            {
                m_xNewOwnStorage.dispose();
            }
            catch( com.sun.star.uno.RuntimeException e )
            {}
        }

        m_xNewOwnStorage = null;
        m_xNewParentStorage = null;
        m_aNewEntryName = null;
        m_bWaitSaveCompleted = false;
    }

    // -------------------------------------------------------------
    public boolean hasEntry() throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_bWaitSaveCompleted )
            throw new com.sun.star.embed.WrongStateException();

        if ( m_xOwnStorage != null )
            return true;

        return false;
    }

    // -------------------------------------------------------------
    public String getEntryName() throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 || m_bWaitSaveCompleted )
            throw new com.sun.star.embed.WrongStateException();

        return m_aEntryName;
    }

    // com.sun.star.embed.XVisualObject:
    // -------------------------------------------------------------
    public void setVisualAreaSize(long nAspect, com.sun.star.awt.Size aSize) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        if ( nAspect == com.sun.star.embed.Aspects.MSOLE_ICON )
            // the ICON aspect should be handled by the container
            throw new com.sun.star.embed.WrongStateException();

        if ( m_nObjectState == com.sun.star.embed.EmbedStates.LOADED )
            changeState( com.sun.star.embed.EmbedStates.RUNNING );

        if ( m_aEditorFrame == null )
            throw new com.sun.star.uno.RuntimeException();

        m_aObjSize.setSize( aSize.Width, aSize.Height );
        m_aEditorFrame.setAppSize( m_aObjSize );
    }

    // -------------------------------------------------------------
    public com.sun.star.awt.Size getVisualAreaSize(long nAspect) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        if ( nAspect == com.sun.star.embed.Aspects.MSOLE_ICON )
            // the ICON aspect should be handled by the container
            throw new com.sun.star.embed.WrongStateException();

        if ( m_nObjectState == com.sun.star.embed.EmbedStates.LOADED )
            changeState( com.sun.star.embed.EmbedStates.RUNNING );

        UpdateSizeAndGetFromActive();

        return new com.sun.star.awt.Size( (int)m_aObjSize.getWidth(), (int)m_aObjSize.getHeight() );
    }

    // -------------------------------------------------------------
    public VisualRepresentation getPreferredVisualRepresentation(long nAspect) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        if ( nAspect == com.sun.star.embed.Aspects.MSOLE_ICON )
            // the ICON aspect should be handled by the container
            throw new com.sun.star.embed.WrongStateException();

        if ( m_nObjectState == com.sun.star.embed.EmbedStates.LOADED )
            changeState( com.sun.star.embed.EmbedStates.RUNNING );

        byte[] aData = m_aEditorFrame.getReplacementImage();
        VisualRepresentation aVisRep = new VisualRepresentation();
        aVisRep.Data = aData;
        aVisRep.Flavor = new com.sun.star.datatransfer.DataFlavor( "image/png", "png", new com.sun.star.uno.Type( byte[].class ) );
        return aVisRep;
    }

    // -------------------------------------------------------------
    public int getMapUnit(long nAspect) throws com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        if ( nAspect == com.sun.star.embed.Aspects.MSOLE_ICON )
            // the ICON aspect should be handled by the container
            throw new com.sun.star.embed.WrongStateException();

        return com.sun.star.embed.EmbedMapUnits.PIXEL;
    }

    // com.sun.star.embed.XClassifiedObject:
    // -------------------------------------------------------------
    public byte[] getClassID()
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        return m_aClassID;
    }

    // -------------------------------------------------------------
    public String getClassName()
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        return new String();
    }

    // -------------------------------------------------------------
    public void setClassInfo(byte[] aClassID, String sClassName) throws com.sun.star.lang.NoSupportException
    {
        throw new com.sun.star.lang.NoSupportException();
    }

    // com.sun.star.embed.XComponentSupplier:
    // -------------------------------------------------------------
    public com.sun.star.util.XCloseable getComponent()
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        // allows no access to the component, this simple example just has no
        return null;
    }

    // com.sun.star.embed.XStateChangeBroadcaster:
    // -------------------------------------------------------------
    public void addStateChangeListener(com.sun.star.embed.XStateChangeListener xListener)
    {
        if ( m_bDisposed )
            return;

        GetListeners().add( xListener );
    }

    // -------------------------------------------------------------
    public void removeStateChangeListener(com.sun.star.embed.XStateChangeListener xListener)
    {
        if ( m_bDisposed )
            return;

        if ( m_aListeners != null )
            m_aListeners.remove( xListener );
    }

    // com.sun.star.document.XEventBroadcaster:
    // -------------------------------------------------------------
    public void addEventListener(com.sun.star.document.XEventListener xListener)
    {
        if ( m_bDisposed )
            return;

        GetListeners().add( xListener );
    }

    // -------------------------------------------------------------
    public void removeEventListener(com.sun.star.document.XEventListener xListener)
    {
        if ( m_bDisposed )
            return;

        if ( m_aListeners != null )
            m_aListeners.remove( xListener );
    }

    // com.sun.star.util.XCloseBroadcaster:
    // -------------------------------------------------------------
    public void addCloseListener(com.sun.star.util.XCloseListener xListener)
    {
        if ( m_bDisposed )
            return;

        GetListeners().add( xListener );
    }

    // -------------------------------------------------------------
    public void removeCloseListener(com.sun.star.util.XCloseListener xListener)
    {
        if ( m_bDisposed )
            return;

        if ( m_aListeners != null )
            m_aListeners.remove( xListener );
    }

    // com.sun.star.util.XCloseable:
    // -------------------------------------------------------------
    public void close(boolean bDeliverOwnership) throws com.sun.star.util.CloseVetoException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        com.sun.star.lang.EventObject aEventObject = new com.sun.star.lang.EventObject( this );

        if ( m_aListeners != null )
        {
            for ( int nInd = 0; nInd < m_aListeners.size(); nInd++ )
            {
                try
                {
                    com.sun.star.util.XCloseListener xListener = ( com.sun.star.util.XCloseListener )
                        UnoRuntime.queryInterface( com.sun.star.document.XEventListener.class, m_aListeners.get( nInd ) );

                    if ( xListener != null )
                        xListener.queryClosing( aEventObject, bDeliverOwnership );
                }
                catch( com.sun.star.util.CloseVetoException e )
                {
                    throw e;
                }
                catch( java.lang.Exception e )
                {
                    m_aListeners.remove( nInd );
                }
            }

            m_bDisposed = true;

            for ( int nInd = 0; nInd < m_aListeners.size(); nInd++ )
            {
                try
                {
                    com.sun.star.util.XCloseListener xListener = ( com.sun.star.util.XCloseListener )
                        UnoRuntime.queryInterface( com.sun.star.document.XEventListener.class, m_aListeners.get( nInd ) );

                    if ( xListener != null )
                        xListener.notifyClosing( aEventObject );
                }
                catch( java.lang.Exception e )
                {
                    m_aListeners.remove( nInd );
                }
            }

            m_aListeners.clear();
        }

        m_bDisposed = true;

        if ( m_aEditorFrame != null )
        {
            m_aEditorFrame.dispose();
            m_aEditorFrame = null;
        }

        if ( m_xOwnStorage != null )
        {
            try {
                m_xOwnStorage.dispose();
            } catch( java.lang.Exception e ) {}
            m_xOwnStorage = null;
        }
    }

    // com.sun.star.embed.XEmbeddedObject:
    // -------------------------------------------------------------
    public void changeState(int nNewState) throws com.sun.star.embed.UnreachableStateException, com.sun.star.embed.WrongStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        int nOldState = m_nObjectState;

        if ( nOldState == nNewState )
        {
            if ( nOldState == com.sun.star.embed.EmbedStates.ACTIVE )
            {
                if ( m_aEditorFrame == null )
                    throw new com.sun.star.uno.RuntimeException();
                m_aEditorFrame.toFront();
            }

            return;
        }

        if ( nNewState != com.sun.star.embed.EmbedStates.LOADED
          && nNewState != com.sun.star.embed.EmbedStates.RUNNING
          && nNewState != com.sun.star.embed.EmbedStates.ACTIVE )
            throw new com.sun.star.embed.UnreachableStateException();

        StateChangeNotification( true, nOldState, nNewState );

        try
        {
            if ( nOldState == com.sun.star.embed.EmbedStates.LOADED )
            {
                // switch to the RUNNING state first
                String aText = ReadStringFromStream( m_xOwnStorage, "content.txt" );

                EditorFrame aEditorFrame = new EditorFrame( m_aEntryName, this, 5, 20 );
                aEditorFrame.setText( aText );

                ReadSizeFromOwnStorage();

                m_aEditorFrame = aEditorFrame;
                m_nObjectState = com.sun.star.embed.EmbedStates.RUNNING;

                if ( nNewState == com.sun.star.embed.EmbedStates.ACTIVE )
                {
                    if ( m_xClient == null )
                        throw new com.sun.star.embed.WrongStateException();

                    m_aEditorFrame.show();
                    m_aEditorFrame.toFront();

                    if ( m_aObjSize != null )
                        aEditorFrame.setAppSize( m_aObjSize );

                    m_xClient.visibilityChanged( true );
                    m_nObjectState = com.sun.star.embed.EmbedStates.ACTIVE;
                }
            }
            else if ( nOldState == com.sun.star.embed.EmbedStates.RUNNING )
            {
                if ( nNewState == com.sun.star.embed.EmbedStates.LOADED )
                {
                    EditorFrame aFrame = m_aEditorFrame;
                    m_aEditorFrame = null;
                    aFrame.dispose();
                    m_nObjectState = nNewState;
                }
                else // nNewState == ACTIVE
                {
                    if ( m_aEditorFrame == null )
                        throw new com.sun.star.uno.RuntimeException();

                    if ( m_xClient == null )
                        throw new com.sun.star.embed.WrongStateException();

                    m_aEditorFrame.show();
                    m_aEditorFrame.toFront();

                    if ( m_aObjSize != null )
                        m_aEditorFrame.setAppSize( m_aObjSize );

                    m_xClient.visibilityChanged( true );

                    m_nObjectState = nNewState;
                }
            }
            else // nOldState == ACTIVE
            {
                UpdateSizeAndGetFromActive();
                if ( nNewState == com.sun.star.embed.EmbedStates.RUNNING )
                {
                    m_aEditorFrame.hide();
                    m_nObjectState = nNewState;
                }
                else // nNewState == LOADED
                {
                    EditorFrame aFrame = m_aEditorFrame;
                    m_aEditorFrame = null;
                    aFrame.dispose();
                    m_nObjectState = nNewState;
                }
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            if ( nOldState != m_nObjectState )
                StateChangeNotification( false, nOldState, m_nObjectState );
            throw e;
        }

        StateChangeNotification( true, nOldState, nNewState );
    }

    // -------------------------------------------------------------
    public int[] getReachableStates() throws com.sun.star.embed.NeedsRunningStateException, com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        int[] pStates = new int[3];
        pStates[0] = com.sun.star.embed.EmbedStates.LOADED;
        pStates[1] = com.sun.star.embed.EmbedStates.RUNNING;
        pStates[2] = com.sun.star.embed.EmbedStates.ACTIVE;

        return pStates;
    }

    // -------------------------------------------------------------
    public int getCurrentState() throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        return m_nObjectState;
    }

    // -------------------------------------------------------------
    public void doVerb(int nVerbID) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.embed.WrongStateException, com.sun.star.embed.UnreachableStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        if ( nVerbID == com.sun.star.embed.EmbedVerbs.MS_OLEVERB_PRIMARY
          || nVerbID == com.sun.star.embed.EmbedVerbs.MS_OLEVERB_SHOW
          || nVerbID == com.sun.star.embed.EmbedVerbs.MS_OLEVERB_OPEN )
            changeState( com.sun.star.embed.EmbedStates.ACTIVE );
        else if ( nVerbID == com.sun.star.embed.EmbedVerbs.MS_OLEVERB_HIDE )
            changeState( com.sun.star.embed.EmbedStates.RUNNING );
    }

    // -------------------------------------------------------------
    public com.sun.star.embed.VerbDescriptor[] getSupportedVerbs() throws com.sun.star.embed.NeedsRunningStateException, com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        if ( m_pOwnVerbs == null )
        {
            try
            {
                XMultiComponentFactory xFactory = m_xContext.getServiceManager();
                Object obj = xFactory.createInstanceWithContext( "com.sun.star.configuration.ConfigurationProvider", m_xContext );
                XMultiServiceFactory xConfProvider = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, obj );
                if ( xConfProvider == null )
                    throw new com.sun.star.uno.RuntimeException();

                Object[] aArgs = new Object[1];
                aArgs[0] = "/org.openoffice.Office.Embedding/Objects";
                Object oSettings = xConfProvider.createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess", aArgs );
                XNameAccess xObjConfNA = ( XNameAccess ) UnoRuntime.queryInterface( XNameAccess.class, oSettings );
                if ( xObjConfNA == null )
                    throw new com.sun.star.uno.RuntimeException();

                Object oEmbObj = xObjConfNA.getByName( "69474366-FD6F-4806-8374-8EDD1B6E771D" );
                XNameAccess xEmbObjNA = (XNameAccess) UnoRuntime.queryInterface( XNameAccess.class, oEmbObj );
                if ( xEmbObjNA == null )
                    throw new com.sun.star.uno.RuntimeException();

                String[] pVerbShortcuts = (String[]) AnyConverter.toArray( xEmbObjNA.getByName( "ObjectVerbs" ) );
                if ( pVerbShortcuts != null && pVerbShortcuts.length != 0 )
                {
                    com.sun.star.embed.VerbDescriptor[] pVerbs = new com.sun.star.embed.VerbDescriptor[pVerbShortcuts.length];
                       aArgs[0] = "/org.openoffice.Office.Embedding/Verbs";
                       Object oVerbs = xConfProvider.createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess", aArgs );
                       XNameAccess xVerbsConfNA = ( XNameAccess ) UnoRuntime.queryInterface( XNameAccess.class, oVerbs );
                       if ( xVerbsConfNA == null )
                        throw new com.sun.star.uno.RuntimeException();

                    for ( int nInd = 0; nInd < pVerbShortcuts.length; nInd++ )
                    {
                        try
                        {
                            XNameAccess xVerbNA = (XNameAccess) UnoRuntime.queryInterface(
                                                                XNameAccess.class,
                                                                xVerbsConfNA.getByName( pVerbShortcuts[nInd] ) );
                            if ( xVerbNA != null )
                            {
                                com.sun.star.embed.VerbDescriptor aVerb = new com.sun.star.embed.VerbDescriptor();
                                aVerb.VerbID = AnyConverter.toInt( xVerbNA.getByName( "VerbID" ) );
                                aVerb.VerbName = AnyConverter.toString( xVerbNA.getByName( "VerbUIName" ) );
                                aVerb.VerbFlags = AnyConverter.toInt( xVerbNA.getByName( "VerbFlags" ) );
                                aVerb.VerbAttributes = AnyConverter.toInt( xVerbNA.getByName( "VerbAttributes" ) );
                                pVerbs[nInd] = aVerb;
                            }
                        }
                        catch( java.lang.Exception e )
                        {
                        }

                        if ( pVerbs[nInd] == null )
                        {
                            // let the error be visible
                            pVerbs[nInd] = new com.sun.star.embed.VerbDescriptor();
                            pVerbs[nInd].VerbID = com.sun.star.embed.EmbedVerbs.MS_OLEVERB_PRIMARY;
                            pVerbs[nInd].VerbName = "ERROR!";
                            pVerbs[nInd].VerbFlags = 0;
                            pVerbs[nInd].VerbAttributes = com.sun.star.embed.VerbAttributes.MS_VERBATTR_ONCONTAINERMENU;
                        }
                    }

                    m_pOwnVerbs = pVerbs;
                }
            }
            catch( com.sun.star.uno.Exception e )
            {}
        }

        if ( m_pOwnVerbs != null )
            return m_pOwnVerbs;

        return new com.sun.star.embed.VerbDescriptor[0];
    }

    // -------------------------------------------------------------
    public void setClientSite(com.sun.star.embed.XEmbeddedClient xClient) throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        m_xClient = xClient;
    }

    // -------------------------------------------------------------
    public com.sun.star.embed.XEmbeddedClient getClientSite() throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        return m_xClient;
    }

    // -------------------------------------------------------------
    public void update() throws com.sun.star.embed.WrongStateException, com.sun.star.uno.Exception
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        // not implemented
    }

    // -------------------------------------------------------------
    public void setUpdateMode(int nMode) throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        // not implemented
    }

    // -------------------------------------------------------------
    public long getStatus(long nAspect) throws com.sun.star.embed.WrongStateException
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        if ( m_nObjectState == -1 )
            throw new com.sun.star.embed.WrongStateException();

        return 0;
    }

    // -------------------------------------------------------------
    public void setContainerName(String sName)
    {
        if ( m_bDisposed )
            throw new com.sun.star.lang.DisposedException();

        // not implemented
    }
}

