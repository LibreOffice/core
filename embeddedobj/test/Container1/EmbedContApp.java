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

package embeddedobj.test;

import java.awt.*;
import java.applet.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.Vector;

import javax.swing.JOptionPane;
import javax.swing.Timer;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;

import com.sun.star.lang.XComponent;

import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;
import com.sun.star.util.URL;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.NamedValue;

import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.XTransferable;

import com.sun.star.container.XNameAccess;

import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;

import com.sun.star.awt.XWindow;
import com.sun.star.awt.XBitmap;

import com.sun.star.task.XJob;

import com.sun.star.embed.*;


class ActionObject
{
    public byte m_nID;
    public String m_sParam;

    public ActionObject()
    {
        m_nID = 0;
        m_sParam = null;
    }

    public ActionObject( byte nID )
    {
        m_nID = nID;
        m_sParam = null;
    }

    public ActionObject( byte nID, String sParam )
    {
        m_nID = nID;
        m_sParam = sParam;
    }

    public ActionObject( ActionObject aObject )
    {
        m_nID = aObject.m_nID;
        m_sParam = aObject.m_sParam;
    }
};

public class EmbedContApp extends Applet
    implements MouseListener, XEmbeddedClient, ActionListener, XJob, XInplaceClient, XWindowSupplier
{
    private XMultiServiceFactory m_xServiceFactory;

    private final boolean m_bStoreVisRepl = false;

    private XJob m_xMainThreadExecutor;
    private NamedValue[] m_pValuesForExecutor;

    private XEmbeddedObject m_xEmbedObj;
    private XStorage m_xStorage;
    private float    m_nXScaling;
    private float    m_nYScaling;
    private float    m_nXPixelSize;
    private float    m_nYPixelSize;

    private Frame m_aFrame;
    private Menu m_aFileMenu;
    private Menu m_aObjectMenu;
    private Toolkit m_aToolkit;

    private Image m_aImage;
    private Object m_oImageLock;

    private boolean m_bOwnFile = false;

    private boolean m_bLinkObj = false;
    private String m_aLinkURI;

    private Object m_oActionsNumberLock;
    private Vector m_aActionsList;

    private Timer m_aTimer;
    private boolean m_bDestroyed = false;

    private Object m_oInHandlerLock;
    private boolean m_bInHandler = false;

    private XURLTransformer m_xTransformer;

    private NativeView m_aNativeView;
    private XWindow m_xVCLWindow;

    private XBitmap m_xBitmap;
    private BitmapPainter m_aBitmapPainter;

// Constants
    private final byte DESTROY                  =  1;
    private final byte ACTIVATE_OUTPLACE        =  2;
    private final byte NEW_DOCUMENT             =  3;
    private final byte SAVE_AS                  =  4;
    private final byte OPEN_FILE                =  5;
    private final byte SAVE                     =  6;
    private final byte NEW_OBJECT               =  7;
    private final byte OBJECT_FROM_FILE         =  8;
    private final byte LINK_FROM_FILE           =  9;
    private final byte CONVERT_LINK_TO_OBJECT   = 10;
    private final byte ACTIVATE_INPLACE         = 11;
    private final byte DEACTIVATE               = 12;

// Methods
    public EmbedContApp( Frame aFrame, XMultiServiceFactory xServiceFactory )
    {
        m_aFrame = aFrame;
        m_xServiceFactory = xServiceFactory;
    }

    public void init()
    {
        resize( 800, 600 );
        setBackground( Color.gray );

        m_aToolkit = Toolkit.getDefaultToolkit();

        try {
            Object oTransformer = m_xServiceFactory.createInstance( "com.sun.star.util.URLTransformer" );
            m_xTransformer = (XURLTransformer)UnoRuntime.queryInterface( XURLTransformer.class, oTransformer );
        } catch( Exception e ) { System.exit( 0 ); }

        m_oActionsNumberLock = new Object();
        m_aActionsList = new Vector();

        m_oInHandlerLock = new Object();
        m_oImageLock = new Object();

        try {
            Object oJob = m_xServiceFactory.createInstance( "com.sun.star.comp.thread.MainThreadExecutor" );
            m_xMainThreadExecutor = (XJob)UnoRuntime.queryInterface( XJob.class, oJob );
        } catch( Exception e ) {}

        if ( m_xMainThreadExecutor == null )
        {
            System.out.println( "Can't create MainThreadExecutor! The application is unusable!" );
            System.exit( 0 );
        }

        m_nXScaling = 1;
        m_nYScaling = 1;
        m_nXPixelSize = 1;
        m_nYPixelSize = 1;

        m_pValuesForExecutor = new NamedValue[1];
        m_pValuesForExecutor[0] = new NamedValue( "JobToExecute", (Object)this );

        m_aTimer = new Timer( 100, this );
        m_aTimer.start();

        // Get a menu bar.
        MenuBar aMenuBar = m_aFrame.getMenuBar();
        if( aMenuBar == null )
        {
            aMenuBar = new MenuBar();
            m_aFrame.setMenuBar( aMenuBar );
        }

        // Create menus for the menu bar.

        // File menu
        m_aFileMenu = new Menu( "File", true );
        aMenuBar.add( m_aFileMenu );

        MenuItem aItem = new NewMenuItem();
        m_aFileMenu.add( aItem );

        aItem = new OpenFileMenuItem();
        m_aFileMenu.add( aItem );

        aItem = new SaveMenuItem();
        m_aFileMenu.add( aItem );

        aItem = new SaveAsMenuItem();
        m_aFileMenu.add( aItem );

        // Object menu
        m_aObjectMenu = new Menu( "Object", true );
        aMenuBar.add( m_aObjectMenu );

        aItem = new NewObjectMenuItem();
        m_aObjectMenu.add( aItem );

        aItem = new LoadObjectMenuItem();
        m_aObjectMenu.add( aItem );

        aItem = new LinkObjectMenuItem();
        m_aObjectMenu.add( aItem );

        aItem = new ConvertLinkToEmbedMenuItem();
        m_aObjectMenu.add( aItem );

        // Activation menu
        m_aObjectMenu = new Menu( "Activation", true );
        aMenuBar.add( m_aObjectMenu );

        aItem = new ActivateOutplaceMenuItem();
        m_aObjectMenu.add( aItem );

        aItem = new ActivateInplaceMenuItem();
        m_aObjectMenu.add( aItem );

        aItem = new DeactivateMenuItem();
        m_aObjectMenu.add( aItem );

        m_aNativeView = new NativeView();
        m_aNativeView.resize( 800, 600 );
        this.add( m_aNativeView );

        // Handle mouse clicks in our window.
//      addMouseListener( this );
    }

    public void actionPerformed( ActionEvent evt )
    {
        synchronized( m_oInHandlerLock )
        {
            if ( m_bInHandler )
                return;
            m_bInHandler = true;
        }

        synchronized( m_oActionsNumberLock )
        {
            if ( m_aActionsList.size() > 0 )
            {
                try {
                    m_xMainThreadExecutor.execute( m_pValuesForExecutor );
                }
                catch( Exception e )
                {
                    System.out.println( "Exception in actionPerformed() : " + e );
                }
            }
            else
            {
                synchronized( m_oInHandlerLock )
                {
                    m_bInHandler = false;
                }
            }
        }
    }

    // XWindowSupplier
    public XWindow getWindow()
    {
        return m_xVCLWindow;
    }

    // XEmbeddedClient
    public void saveObject()
        throws com.sun.star.uno.Exception
    {
        if ( m_xEmbedObj != null )
        {
            try {
                XEmbedPersist xPersist = (XEmbedPersist)UnoRuntime.queryInterface( XEmbedPersist.class, m_xEmbedObj );
                if ( xPersist != null )
                {
                    xPersist.storeOwn();
                    generateNewImage();
                }
                else
                    JOptionPane.showMessageDialog( m_aFrame, "No XEmbedPersist!", "Error:", JOptionPane.ERROR_MESSAGE );
            }
            catch( Exception e )
            {
                JOptionPane.showMessageDialog( m_aFrame, e, "Exception in saveObject:", JOptionPane.ERROR_MESSAGE );
            }
        }

        generateNewImage();
        repaint();
    }

    public void onShowWindow( boolean bVisible )
    {
        // for now nothing to do
    }

    // XInplaceClient
    public boolean canInplaceActivate()
    {
        return true;
    }

    public void onInplaceActivate()
    {
        // TODO
        // prepare for inplace activation

        // REMOVE
        // workaround for CLIPCHILDREN problem
        if ( m_aBitmapPainter != null )
            m_aBitmapPainter.stopPainting();
    }

    public void onUIActivate()
    {
        // TODO
        // prepare for UI activate
    }

    public void onInplaceDeactivate()
    {
        // TODO
        // inplace deactivation is done

        // REMOVE
        // workaround for CLIPCHILDREN problem
        if ( m_aBitmapPainter != null )
            m_aBitmapPainter.startPainting();
    }

    public void onUIDeactivate()
    {
        // TODO
        // prepare for UI deactivate
    }

    public XIPMainContainerWindow getTopmostWindow()
    {
        // TODO
        // return an implementation of XIPMainContainerWindow
        // mainly required for ui activation
        // dummy implementation is enough for inplace activation

        return null;
    }

    public XInplaceUIWindow getDocumentWindow()
    {
        // TODO
        // return implementation of XInplaceUIWindow
        // mainly required for ui activation
        // dummy implementation is enough for inplace activation

        return null;
    }

    public com.sun.star.awt.Rectangle getPosRect()
    {
        // provide position rectangle to the object
        try {
            // here object bitmap and scaling factor hold the size
            com.sun.star.awt.Size aBitmapSize = m_xBitmap.getSize();
            com.sun.star.awt.Size aVisSize = new com.sun.star.awt.Size(
                                            (int)( aBitmapSize.Width * m_nXScaling ),
                                            (int)( aBitmapSize.Height * m_nYScaling ) );
            return new com.sun.star.awt.Rectangle( 10, 10, aVisSize.Width, aVisSize.Height );
        }
        catch( Exception e )
        {
            System.out.println( "Position rectangle generation failed!" );
        }

        return new com.sun.star.awt.Rectangle( 10, 10, 110, 110 );
    }

    public com.sun.star.awt.Rectangle getClipRect()
    {
        // provide clip rectangle to the object
        // in this application position and clip rectangles are the same

        try {
            // here object bitmap and scaling factor hold the size
            com.sun.star.awt.Size aBitmapSize = m_xBitmap.getSize();
            com.sun.star.awt.Size aVisSize = new com.sun.star.awt.Size(
                                            (int)( aBitmapSize.Width * m_nXScaling ),
                                            (int)( aBitmapSize.Height * m_nYScaling ) );
            return new com.sun.star.awt.Rectangle( 10, 10, aVisSize.Width, aVisSize.Height );
        }
        catch( Exception e )
        {
            System.out.println( "Clip rectangle generation failed!" );
        }

        return new com.sun.star.awt.Rectangle( 10, 10, 110, 110 );
    }

    public void translateAccelerators( com.sun.star.awt.KeyEvent[] aKeys )
    {
        // TODO
        // an accelerator table for object
        // ui activation related
    }

    public void scrollObj( com.sun.star.awt.Size aOffset )
    {
        // TODO
        // scrolls the object to a specified offset
        // not mandatory for the testing application :)
    }

    public void onPosRectChange( com.sun.star.awt.Rectangle aPosRect )
    {
        // object asks to change the position
        if ( m_xEmbedObj != null )
        {
            try {
                int nState = m_xEmbedObj.getCurrentState();
                // such a position change make sence only when object is
                // either inplace or ui active
                if ( nState == EmbedStates.EMBED_INPLACE_ACTIVE
                  || nState == EmbedStates.EMBED_UI_ACTIVE )
                 {
                    XInplaceObject xInplObj = (XInplaceObject)UnoRuntime.queryInterface( XInplaceObject.class, m_xEmbedObj );
                     if ( xInplObj != null )
                    {
                        xInplObj.setObjectRects( aPosRect, aPosRect ); // show the whole object
                        if ( m_aBitmapPainter != null )
                            m_aBitmapPainter.setRect( aPosRect );
                    }
                    else
                         System.out.println( "Why object that does not support inplace activation behave like inplace object?!" );
                 }
                 else
                     System.out.println( "The object is not active but asks to change visual area!" );
            } catch( Exception e )
            {
                 System.out.println( "Exception is thrown in onPosRectChange: " + e );
            }
        }
        else
             System.out.println( "Who asks to change visual area?!!" );
    }

    // XJob
    public Object execute( NamedValue[] pValues )
    {
        for( int nInd = 0; nInd < m_aActionsList.size(); nInd++ )
        {
            ActionObject aAction = ( ActionObject ) m_aActionsList.get( nInd );
            if ( aAction != null )
            {
                if ( aAction.m_nID == DESTROY )
                {
                    // free all resources
                    clearObjectAndStorage();
                    m_bDestroyed = true;
                }
                else if ( aAction.m_nID == ACTIVATE_OUTPLACE )
                {
                    // activate object if exists and not active
                    if ( m_xEmbedObj != null )
                    {
                        try {
                            m_xEmbedObj.changeState( EmbedStates.EMBED_ACTIVE );
                        }
                        catch( Exception ex )
                        {
                            System.out.println( "Exception on mouse click" + ex );
                        }
                    }
                }
                else if ( aAction.m_nID == NEW_DOCUMENT )
                {
                    // clear everything
                    clearObjectAndStorage();

                    repaint();
                }
                else if ( aAction.m_nID == SAVE_AS )
                {
                    // open SaveAs dialog and store

                    if ( m_xStorage != null && m_xEmbedObj != null )
                    {
                        try {
                            /*
                                if ( m_bLinkObj )
                                    storeLinkAsFileURI( aFileURI );
                                else
                            */
                            saveObjectAsFileURI( aAction.m_sParam );
                        }
                        catch( Exception ex )
                        {
                            System.out.println( "Exception in SaveAsMenuItem: " + ex );
                        }
                    }
                }
                else if ( aAction.m_nID == OPEN_FILE )
                {
                    // clear everything
                    clearObjectAndStorage();

                    // load from specified file
                    loadFileURI( aAction.m_sParam );

                    if ( m_xEmbedObj != null )
                    {
                        try {
                            m_xEmbedObj.setClientSite( this );
                        }
                        catch( Exception ex )
                        {
                            System.out.println( "Exception in OpenFileMenuItem: " + ex );
                        }
                    }

                    generateNewImage();
                    repaint();
                }
                else if ( aAction.m_nID == SAVE )
                {
                    if ( m_xStorage != null && m_xEmbedObj != null )
                    {
                        // if has persistance store there
                        // if not it is and error, SaveAs had to be used

                        if ( m_bOwnFile )
                        {
                            if ( m_xStorage != null )
                            {
                                try {
                                    saveObject();

                                    if ( m_bLinkObj )
                                        storeLinkToStorage();

                                    XTransactedObject xTransact = (XTransactedObject)UnoRuntime.queryInterface( XTransactedObject.class,
                                                                                                                m_xStorage );
                                    if ( xTransact != null )
                                        xTransact.commit();
                                }
                                catch( Exception ex )
                                {
                                    System.out.println( "Exception during save operation in SaveMenuItem:" + ex );
                                }
                            }
                            else
                            {
                                System.out.println( "No storage for owned file!" );
                            }
                        }
                        else
                        {
                            System.out.println( "No owned file!" );
                        }
                    }
                }
                else if ( aAction.m_nID == NEW_OBJECT )
                {
                    // remove current object an init a new one
                    clearObjectAndStorage();

                    if ( aAction.m_sParam != null )
                    {
                        m_xStorage = createTempStorage();

                        if ( m_xStorage != null )
                            m_xEmbedObj = createEmbedObject( aAction.m_sParam );
                        else
                            System.out.println( "Can't create temporary storage!" );

                        if ( m_xEmbedObj != null )
                        {
                            try {
                                m_xEmbedObj.setClientSite( this );
                            }
                            catch( Exception ex )
                            {
                                System.out.println( "Exception in NewObjectMenuItem:" + ex );
                            }
                        }
                    }

                    generateNewImage();
                    repaint();
                }
                else if ( aAction.m_nID == OBJECT_FROM_FILE )
                {
                    // first remove current object
                    clearObjectAndStorage();

                    // create object from specified file
                    m_xStorage = createTempStorage();

                    if ( m_xStorage != null )
                        m_xEmbedObj = loadEmbedObject( aAction.m_sParam );

                    if ( m_xEmbedObj != null )
                    {
                        try {
                            m_xEmbedObj.setClientSite( this );
                        }
                        catch( Exception ex )
                        {
                            System.out.println( "Exception in LoadObjectMenuItem: " + ex );
                        }
                    }

                    generateNewImage();
                    repaint();
                }
                else if ( aAction.m_nID == LINK_FROM_FILE )
                {
                    // first remove current object
                    clearObjectAndStorage();

                    m_xStorage = createTempStorage();

                    // create object from specified file
                    m_xEmbedObj = createLinkObject( aAction.m_sParam );

                    if ( m_xEmbedObj != null )
                    {
                        m_aLinkURI = aAction.m_sParam;
                        m_bLinkObj = true;

                        try {
                            m_xEmbedObj.setClientSite( this );
                        }
                        catch( Exception ex )
                        {
                            System.out.println( "Exception in LinkObjectMenuItem:" + ex );
                        }
                    }

                    generateNewImage();
                    repaint();
                }
                else if ( aAction.m_nID == CONVERT_LINK_TO_OBJECT )
                {
                    if ( !m_bLinkObj )
                    {
                        System.out.println( "The object is not a link!" );
                        continue;
                    }

                    if ( m_xEmbedObj != null )
                    {
                        if ( m_xStorage != null )
                        {
                            try {
                                XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
                                                                                                m_xStorage );
                                if ( xNameAccess != null && xNameAccess.hasByName( "LinkName" ) )
                                    m_xStorage.removeElement( "LinkName" );

                                XLinkageSupport xLinkage = (XLinkageSupport)UnoRuntime.queryInterface( XLinkageSupport.class,
                                                                                                        m_xEmbedObj );
                                if ( xLinkage != null )
                                {
                                    xLinkage.breakLink( m_xStorage, "EmbedSub" );
                                    m_bLinkObj = false;
                                    m_aLinkURI = null;
                                }
                                else
                                    System.out.println( "No XLinkageSupport in ConvertLink... !" );
                            }
                            catch( Exception e1 )
                            {
                                System.out.println( "Exception in ConvertLinkToEmbed:try 1 :" + e1 );
                            }
                        }
                    }
                }
                else if ( aAction.m_nID == ACTIVATE_INPLACE )
                {
                    // activate object
                    if ( m_xEmbedObj != null )
                    {
                        // in general it is better to check acceptable states
                        try {
                            m_xEmbedObj.changeState( EmbedStates.EMBED_INPLACE_ACTIVE );
                        }
                        catch( Exception ex )
                        {
                            System.out.println( "Exception on inplace activation " + ex );
                        }
                    }
                }
                else if ( aAction.m_nID == DEACTIVATE )
                {
                    // activate object

                    if ( m_xEmbedObj != null )
                    {
                        int nOldState = -1;
                        try {
                            nOldState = m_xEmbedObj.getCurrentState();
                        } catch( Exception e )
                        {}

                        if ( nOldState == EmbedStates.EMBED_ACTIVE
                          || nOldState == EmbedStates.EMBED_INPLACE_ACTIVE
                          || nOldState == EmbedStates.EMBED_UI_ACTIVE )
                        {
                            try {
                                m_xEmbedObj.changeState( EmbedStates.EMBED_RUNNING );
                            }
                            catch( Exception ex )
                            {
                                System.out.println( "Exception on inplace activation " + ex );
                            }
                        }
                        else
                        {
                            System.out.println( "Deactivation of nonactive object!" );
                        }
                    }
                }
                else
                {
                    System.out.println( "Unknoun action is requested: " + aAction.m_nID + "\n" );
                }
            }
        }

        m_aActionsList.clear();

        synchronized( m_oInHandlerLock )
        {
            m_bInHandler = false;
        }

        return Any.VOID;
    }

    public void actionRegister( byte nActionID, String sParam )
    {
        synchronized( m_oActionsNumberLock )
        {
            int nSize = m_aActionsList.size();
            if ( nSize < 199 )
            {
                if ( nSize == 0 )
                    m_aActionsList.add( new ActionObject( nActionID, sParam ) );
                else
                {
                    ActionObject aAction = ( ActionObject ) m_aActionsList.get( nSize - 1 );
                    if ( aAction != null && aAction.m_nID != DESTROY )
                        m_aActionsList.add( new ActionObject( nActionID, sParam ) );
                }
            }
        }
    }

    public void SaveAsOperation()
    {
        if ( m_xStorage != null && m_xEmbedObj != null )
        {
            FileDialog aFileDialog = new FileDialog( m_aFrame, "SaveAs", FileDialog.SAVE );
            aFileDialog.show();
            if ( aFileDialog.getFile() != null )
            {
                String aFileName = aFileDialog.getDirectory() + aFileDialog.getFile();
                File aFile = new File( aFileName );
                if ( aFile != null )
                {
                    // create object from specified file
                    String aFileURI = getValidURL( aFile.toURI().toASCIIString() );
                    actionRegister( SAVE_AS, aFileURI );
                }
            }
        }
        else
            JOptionPane.showMessageDialog( m_aFrame, "No document is embedded!", "Error:", JOptionPane.ERROR_MESSAGE );

    }

    public void destroy()
    {
        // redirect the call through the timer and call super.destroy();
        actionRegister( DESTROY, null );

        for ( int i = 0; i < 3 && !m_bDestroyed; i++ )
        {
            try {
                Thread.sleep( 200 );
            } catch ( Exception e )
            {}
        }

        if ( !m_bDestroyed )
            System.out.println( "The object application can not exit correctly!" );

        m_aTimer.stop();

        super.destroy();
    }

    public void update( Graphics g )
    {
        paint( g );
    }

    public void paint( Graphics g )
    {
        super.paint( g );

        // m_aNativeView.paint( g );

        createVclWindow();
    }

    public void createVclWindow()
    {
        synchronized( m_oImageLock )
        {
            if ( m_xVCLWindow == null && m_xServiceFactory != null && m_xEmbedObj != null && m_xBitmap != null )
            {
                java.awt.Rectangle aBounds = getBounds();
                m_xVCLWindow = WindowHelper.createWindow( m_xServiceFactory, m_aNativeView, aBounds );
                m_xVCLWindow.setVisible( true );

                com.sun.star.awt.Size aBitmapSize = new com.sun.star.awt.Size( 200, 100 );

                XVisualObject xVisObj = (XVisualObject)UnoRuntime.queryInterface( XVisualObject.class, m_xEmbedObj );
                try {
                    com.sun.star.awt.Size aVisSize = xVisObj.getVisAreaSize( Aspects.MSASPECT_CONTENT );
                    m_nXPixelSize = aVisSize.Width / aBitmapSize.Width;
                    m_nYPixelSize = aVisSize.Height / aBitmapSize.Height;
                }
                catch( Exception e )
                {
                }

                if ( m_xBitmap != null )
                     aBitmapSize = m_xBitmap.getSize();

                System.out.println( "The visual area is Width = " + aBitmapSize.Width + "; Height = " + aBitmapSize.Height );

                com.sun.star.awt.Rectangle aRect = new com.sun.star.awt.Rectangle(
                                                        10,
                                                        10,
                                                        Math.min( (int)aBounds.getWidth() - 20, aBitmapSize.Width ),
                                                        Math.min( (int)aBounds.getHeight() - 20, aBitmapSize.Height ) );

                m_aBitmapPainter = new BitmapPainter( m_xMainThreadExecutor, m_xVCLWindow, m_xBitmap, aRect );
            }
        }
    }

    public void generateNewImage()
    {
        if ( m_xEmbedObj != null )
        {
            try {
                int nOldState = m_xEmbedObj.getCurrentState();
                int nState = nOldState;
                if ( nOldState == EmbedStates.EMBED_LOADED )
                {
                    m_xEmbedObj.changeState( EmbedStates.EMBED_RUNNING );
                    nState = EmbedStates.EMBED_RUNNING;
                }

                if ( nState == EmbedStates.EMBED_UI_ACTIVE || nState == EmbedStates.EMBED_INPLACE_ACTIVE
                  || nState == EmbedStates.EMBED_ACTIVE || nState == EmbedStates.EMBED_RUNNING )
                {
                    XComponentSupplier xCompProv = (XComponentSupplier)UnoRuntime.queryInterface(
                                                                                    XComponentSupplier.class,
                                                                                    m_xEmbedObj );
                    if ( xCompProv != null )
                    {
                        XCloseable xCloseable = xCompProv.getComponent();
                        XTransferable xTransfer = (XTransferable)UnoRuntime.queryInterface(
                                                                                    XTransferable.class,
                                                                                    xCloseable );
                        if ( xTransfer != null )
                        {
                            DataFlavor aFlavor = new DataFlavor();
                            aFlavor.MimeType = "application/x-openoffice;windows_formatname=\"Bitmap\"";
                            aFlavor.HumanPresentableName = "Bitmap";
                            aFlavor.DataType = new Type( byte[].class );

                            Object aAny = xTransfer.getTransferData( aFlavor );
                            if ( aAny != null && AnyConverter.isArray( aAny ) )
                            {
                                synchronized( m_oImageLock )
                                {
                                    m_xBitmap = WindowHelper.getVCLBitmapFromBytes( m_xServiceFactory, aAny );
                                    if ( m_aBitmapPainter != null )
                                    {
                                        m_aBitmapPainter.setBitmap( m_xBitmap );

                                        if ( m_xBitmap != null )
                                        {
                                            try {
                                                com.sun.star.awt.Size aBitmapSize = m_xBitmap.getSize();
                                                com.sun.star.awt.Size aVisSize = new com.sun.star.awt.Size(
                                                                                (int)( aBitmapSize.Width * m_nXScaling ),
                                                                                (int)( aBitmapSize.Height * m_nYScaling ) );
                                                m_aBitmapPainter.setSize( aVisSize );
                                            }
                                            catch( Exception e )
                                            {
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                            System.out.println( "paint() : can not get XTransferable for the component!\n" );
                    }
                    else
                        System.out.println( "paint() : XComponentSupplier is not implemented!\n" );
                }
            }
            catch( com.sun.star.uno.Exception e )
            {
                // dialogs should not be used in paint()
                System.out.println( "Exception in paint(): " + e );
            }
        }
    }

    public void mouseClicked( MouseEvent e )
    {
        if( e.getModifiers() == InputEvent.BUTTON1_MASK )
        {
            actionRegister( ACTIVATE_OUTPLACE, null );
        }
    }

    public void mousePressed( MouseEvent e ){};
    public void mouseEntered( MouseEvent e ){};
    public void mouseExited( MouseEvent e ){};
    public void mouseReleased( MouseEvent e ){};

    class NewMenuItem extends MenuItem implements ActionListener // Menu New
    {
        public NewMenuItem()
        {
            super( "New", new MenuShortcut( KeyEvent.VK_A ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            actionRegister( NEW_DOCUMENT, null );
        }
    }

    class SaveAsMenuItem extends MenuItem implements ActionListener // Menu SaveAs...
    {
        public SaveAsMenuItem()
        {
            super( "SaveAs..." );
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            // open SaveAs dialog and store

            SaveAsOperation();
        }
    }

    class OpenFileMenuItem extends MenuItem implements ActionListener // Menu Open
    {
        public OpenFileMenuItem()
        {
            super( "Open", new MenuShortcut( KeyEvent.VK_C ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            // open OpenFile dialog and load doc
            FileDialog aFileDialog = new FileDialog( m_aFrame, "Open" );
            aFileDialog.show();
            if ( aFileDialog.getFile() != null )
            {
                String aFileName = aFileDialog.getDirectory() + aFileDialog.getFile();
                File aFile = new File( aFileName );
                if ( aFile != null )
                {
                    // create object from specified file
                    String aFileURI = getValidURL( aFile.toURI().toASCIIString() );
                    actionRegister( OPEN_FILE, aFileURI );
                }
            }
        }
    }

    class SaveMenuItem extends MenuItem implements ActionListener // Menu Save
    {
        public SaveMenuItem()
        {
            super( "Save", new MenuShortcut( KeyEvent.VK_D ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            // if has persistance store there
            // if not open SaveAs dialog and store
            if ( m_xStorage != null && m_xEmbedObj != null )
            {
                if ( m_bOwnFile )
                {
                    if ( m_xStorage == null )
                    {
                        JOptionPane.showMessageDialog( m_aFrame,
                                                        "No storage for oned file!",
                                                        "Error:",
                                                        JOptionPane.ERROR_MESSAGE );

                        return;
                    }

                    actionRegister( SAVE, null );
                }
                else
                {
                    SaveAsOperation();
                }
            }
            else
                JOptionPane.showMessageDialog( m_aFrame, "No document is embedded!", "Error:", JOptionPane.ERROR_MESSAGE );
        }
    }

    class NewObjectMenuItem extends MenuItem implements ActionListener // Menu NewObject
    {
        public NewObjectMenuItem()
        {
            super( "Create", new MenuShortcut( KeyEvent.VK_N ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            Object[] possibleValues = { "com.sun.star.comp.Writer.TextDocument",
                                        "com.sun.star.comp.Writer.GlobalDocument",
                                        "com.sun.star.comp.Writer.WebDocument",
                                        "com.sun.star.comp.Calc.SpreadsheetDocument",
                                        "com.sun.star.comp.Draw.PresentationDocument",
                                        "com.sun.star.comp.Draw.DrawingDocument",
                                        "com.sun.star.comp.Math.FormulaDocument",
                                        "BitmapImage" };

            String selectedValue = (String)JOptionPane.showInputDialog( null, "DocumentType", "Select",
                                                                           JOptionPane.INFORMATION_MESSAGE, null,
                                                                           possibleValues, possibleValues[0] );

            actionRegister( NEW_OBJECT, selectedValue );
        }
    }

    class LoadObjectMenuItem extends MenuItem implements ActionListener // Menu LoadObject
    {
        public LoadObjectMenuItem()
        {
            super( "Load from file", new MenuShortcut( KeyEvent.VK_L ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            // open OpenFile dialog and load doc
            FileDialog aFileDialog = new FileDialog( m_aFrame, "Select sources to use for object init" );
            aFileDialog.show();
            if ( aFileDialog.getFile() != null )
            {
                String aFileName = aFileDialog.getDirectory() + aFileDialog.getFile();
                File aFile = new File( aFileName );
                if ( aFile != null )
                {
                    // create object from specified file
                    String aFileURI = getValidURL( aFile.toURI().toASCIIString() );
                    actionRegister( OBJECT_FROM_FILE, aFileURI );
                }
            }
        }
    }

    class LinkObjectMenuItem extends MenuItem implements ActionListener // Menu LinkObject
    {
        public LinkObjectMenuItem()
        {
            super( "Create link", new MenuShortcut( KeyEvent.VK_M ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            // open OpenFile dialog and load doc
            FileDialog aFileDialog = new FileDialog( m_aFrame, "Select sources to use for object init" );
            aFileDialog.show();
            if ( aFileDialog.getFile() != null )
            {
                String aFileName = aFileDialog.getDirectory() + aFileDialog.getFile();
                File aFile = new File( aFileName );
                if ( aFile != null )
                {
                    // create object from specified file
                    String aFileURI = getValidURL( aFile.toURI().toASCIIString() );
                    actionRegister( LINK_FROM_FILE, aFileURI );
                }
            }
        }
    }

    class ConvertLinkToEmbedMenuItem extends MenuItem implements ActionListener // Menu LinkObject
    {
        public ConvertLinkToEmbedMenuItem()
        {
            super( "Convert link to embed", new MenuShortcut( KeyEvent.VK_M ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            actionRegister( CONVERT_LINK_TO_OBJECT, null );
        }
    }

    class ActivateOutplaceMenuItem extends MenuItem implements ActionListener // Menu ActiveteOutplace
    {
        public ActivateOutplaceMenuItem()
        {
            super( "Activate outplace", new MenuShortcut( KeyEvent.VK_A ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            actionRegister( ACTIVATE_OUTPLACE, null );
        }
    }

    class ActivateInplaceMenuItem extends MenuItem implements ActionListener // Menu ActivateInplace
    {
        public ActivateInplaceMenuItem()
        {
            super( "Activate inplace", new MenuShortcut( KeyEvent.VK_I ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            actionRegister( ACTIVATE_INPLACE, null );
        }
    }

    class DeactivateMenuItem extends MenuItem implements ActionListener // Menu Deactivate
    {
        public DeactivateMenuItem()
        {
            super( "Deactivate", new MenuShortcut( KeyEvent.VK_D ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            actionRegister( DEACTIVATE, null );
        }
    }

    // Helper methods
    public XEmbeddedObject createEmbedObject( String aServiceName )
    {
        XEmbeddedObject xEmbObj = null;
        byte[] pClassID = new byte[16];

        if ( aServiceName.equals( "com.sun.star.comp.Writer.TextDocument" ) )
        {
            int[] pTempClassID = { 0x8B, 0xC6, 0xB1, 0x65, 0xB1, 0xB2, 0x4E, 0xDD,
                                    0xAA, 0x47, 0xDA, 0xE2, 0xEE, 0x68, 0x9D, 0xD6 };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "com.sun.star.comp.Writer.GlobalDocument" ) )
        {
            int[] pTempClassID = { 0xB2, 0x1A, 0x0A, 0x7C, 0xE4, 0x03, 0x41, 0xFE,
                                    0x95, 0x62, 0xBD, 0x13, 0xEA, 0x6F, 0x15, 0xA0 };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "com.sun.star.comp.Writer.WebDocument" ) )
        {
            int[] pTempClassID = { 0xA8, 0xBB, 0xA6, 0x0C, 0x7C, 0x60, 0x45, 0x50,
                                    0x91, 0xCE, 0x39, 0xC3, 0x90, 0x3F, 0xAC, 0x5E };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "com.sun.star.comp.Calc.SpreadsheetDocument" ) )
        {
            int[] pTempClassID = { 0x47, 0xBB, 0xB4, 0xCB, 0xCE, 0x4C, 0x4E, 0x80,
                                    0xA5, 0x91, 0x42, 0xD9, 0xAE, 0x74, 0x95, 0x0F };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "com.sun.star.comp.Draw.PresentationDocument" ) )
        {
            int[] pTempClassID = { 0x91, 0x76, 0xE4, 0x8A, 0x63, 0x7A, 0x4D, 0x1F,
                                    0x80, 0x3B, 0x99, 0xD9, 0xBF, 0xAC, 0x10, 0x47 };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "com.sun.star.comp.Draw.DrawingDocument" ) )
        {
            int[] pTempClassID = { 0x4B, 0xAB, 0x89, 0x70, 0x8A, 0x3B, 0x45, 0xB3,
                                    0x99, 0x1C, 0xCB, 0xEE, 0xAC, 0x6B, 0xD5, 0xE3 };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "com.sun.star.comp.Math.FormulaDocument" ) )
        {
            int[] pTempClassID = { 0x07, 0x8B, 0x7A, 0xBA, 0x54, 0xFC, 0x45, 0x7F,
                                    0x85, 0x51, 0x61, 0x47, 0xE7, 0x76, 0xA9, 0x97 };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }
        else if ( aServiceName.equals( "BitmapImage" ) )
        {
            int[] pTempClassID = { 0xD3, 0xE3, 0x4B, 0x21, 0x9D, 0x75, 0x10, 0x1A,
                                    0x8C, 0x3D, 0x00, 0xAA, 0x00, 0x1A, 0x16, 0x52 };
            for ( int ind = 0; ind < 16; ind++ )
                pClassID[ind] = (byte)pTempClassID[ind];
        }

        if ( pClassID != null )
        {
            // create embedded object based on the class ID
            try {
                Object oEmbedCreator = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectCreator" );
                XEmbedObjectCreator xEmbedCreator = (XEmbedObjectCreator)UnoRuntime.queryInterface(
                                                                                        XEmbedObjectCreator.class,
                                                                                        oEmbedCreator );
                if ( xEmbedCreator != null )
                {
                    Object oEmbObj = xEmbedCreator.createInstanceInitNew( pClassID,
                                                                        "Dummy name",
                                                                        m_xStorage,
                                                                        "EmbedSub",
                                                                        new PropertyValue[0] );
                    xEmbObj = (XEmbeddedObject)UnoRuntime.queryInterface( XEmbeddedObject.class, oEmbObj );
                }
                else
                    JOptionPane.showMessageDialog( m_aFrame,
                                                   "Can't create EmbedCreator!",
                                                   "Error:",
                                                   JOptionPane.ERROR_MESSAGE );
            }
            catch( Exception e )
            {
                JOptionPane.showMessageDialog( m_aFrame, e, "Exception in createInstanceInitNew():", JOptionPane.ERROR_MESSAGE );
            }
        }
        else
            JOptionPane.showMessageDialog( m_aFrame, "Can't retrieve class ID!", "Error:", JOptionPane.ERROR_MESSAGE );

        return xEmbObj;
    }

    public XEmbeddedObject createLinkObject( String aLinkURL )
    {
        XEmbeddedObject xEmbObj = null;

        try {
            Object oLinkCreator = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectCreator" );
            XLinkCreator xLinkCreator = (XLinkCreator)UnoRuntime.queryInterface(
                                                                                    XLinkCreator.class,
                                                                                    oLinkCreator );
            if ( xLinkCreator != null )
            {
                PropertyValue[] aMedDescr = { new PropertyValue(), new PropertyValue() };
                aMedDescr[0].Name = "URL";
                aMedDescr[0].Value = (Object) aLinkURL;
                aMedDescr[1].Name = "ReadOnly";
                aMedDescr[1].Value = (Object) new Boolean( false );
                Object oEmbObj = xLinkCreator.createInstanceLink( m_xStorage, "EmbedSub", aMedDescr, new PropertyValue[0] );
                xEmbObj = (XEmbeddedObject)UnoRuntime.queryInterface( XEmbeddedObject.class, oEmbObj );
            }
            else
                JOptionPane.showMessageDialog( m_aFrame,
                                               "Can't create LinkCreator!",
                                               "Error:",
                                               JOptionPane.ERROR_MESSAGE );
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in createLinkObject():", JOptionPane.ERROR_MESSAGE );
        }


        return xEmbObj;
    }


    public XEmbeddedObject loadEmbedObject( String aFileURI )
    {
        XEmbeddedObject xEmbObj = null;
        try {
            Object oEmbedCreator = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectCreator" );
            XEmbedObjectCreator xEmbedCreator = (XEmbedObjectCreator)UnoRuntime.queryInterface(
                                                                                    XEmbedObjectCreator.class,
                                                                                    oEmbedCreator );
            if ( xEmbedCreator != null )
            {
                PropertyValue[] aMedDescr = { new PropertyValue(), new PropertyValue() };
                aMedDescr[0].Name = "URL";
                aMedDescr[0].Value = (Object) aFileURI;
                aMedDescr[1].Name = "ReadOnly";
                aMedDescr[1].Value = (Object) new Boolean( false );
                Object oEmbObj = xEmbedCreator.createInstanceInitFromMediaDescriptor( m_xStorage,
                                                                                    "EmbedSub",
                                                                                    aMedDescr,
                                                                                    new PropertyValue[0] );
                xEmbObj = (XEmbeddedObject)UnoRuntime.queryInterface( XEmbeddedObject.class, oEmbObj );
            }
            else
                JOptionPane.showMessageDialog( m_aFrame,
                                               "Can't create EmbedFactory!",
                                               "Error:",
                                               JOptionPane.ERROR_MESSAGE );
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in loadEmbedObject():", JOptionPane.ERROR_MESSAGE );
        }

        return xEmbObj;
    }

    public void clearObjectAndStorage()
    {
        synchronized( m_oImageLock )
        {
            m_aImage = null;
        }

        m_nXScaling = 1;
        m_nYScaling = 1;
        m_nXPixelSize = 1;
        m_nYPixelSize = 1;

        m_bOwnFile = false;

        m_aLinkURI = null;
        m_bLinkObj = false;

        if ( m_xEmbedObj != null )
        {
            try {
                XCloseable xClose = (XCloseable)UnoRuntime.queryInterface( XCloseable.class, m_xEmbedObj );
                if ( xClose != null )
                    xClose.close( true );
            }
            catch ( Exception ex )
            {}
            m_xEmbedObj = null;
        }

        if ( m_xStorage != null )
        {
            try {
                XComponent xComponent = (XComponent)UnoRuntime.queryInterface( XComponent.class, m_xStorage );
                if ( xComponent != null )
                    xComponent.dispose();
            }
            catch ( Exception ex )
            {}
            m_xStorage = null;
        }
    }

    public XStorage createTempStorage()
    {
        XStorage xTempStorage = null;

        try {
            Object oStorageFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.StorageFactory" );
            XSingleServiceFactory xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
                                                                                        XSingleServiceFactory.class,
                                                                                        oStorageFactory );
            if ( xStorageFactory != null )
            {
                Object oStorage = xStorageFactory.createInstance();
                xTempStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oStorage );
            }
            else
                JOptionPane.showMessageDialog( m_aFrame,
                                                "Can't create StorageFactory!",
                                                "Error:",
                                                JOptionPane.ERROR_MESSAGE );
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in createTempStorage():", JOptionPane.ERROR_MESSAGE );
        }

        return xTempStorage;
    }

    public void saveObjectAsFileURI( String aFileURI )
    {
        try {
            Object oStorageFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.StorageFactory" );
            XSingleServiceFactory xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
                                                                                        XSingleServiceFactory.class,
                                                                                        oStorageFactory );
            if ( xStorageFactory != null )
            {
                XEmbedPersist xPersist = (XEmbedPersist)UnoRuntime.queryInterface( XEmbedPersist.class, m_xEmbedObj );
                if ( xPersist != null )
                {
                    Object aArgs[] = new Object[2];
                    aArgs[0] = aFileURI;
                    aArgs[1] = new Integer( ElementModes.ELEMENT_READWRITE );

                    Object oStorage = xStorageFactory.createInstanceWithArguments( aArgs );
                    XStorage xTargetStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oStorage );

                    PropertyValue aProps[] = { new PropertyValue() };
                    aProps[0].Name = "StoreVisualReplacement";
                    aProps[0].Value = new Boolean( m_bStoreVisRepl );

                    xPersist.storeAsEntry( xTargetStorage, "EmbedSub", new PropertyValue[0], aProps );
                    xPersist.saveCompleted( true );

                    // the object must be already based on new storage
                    XComponent xComponent = (XComponent)UnoRuntime.queryInterface( XComponent.class, m_xStorage );
                    xComponent.dispose();

                    m_xStorage = xTargetStorage;
                    m_bOwnFile = true;

                    XTransactedObject xTransact = (XTransactedObject)UnoRuntime.queryInterface( XTransactedObject.class,
                                                                                            m_xStorage );
                    if ( xTransact != null )
                        xTransact.commit();
                }
                else
                    JOptionPane.showMessageDialog( m_aFrame, "No XEmbedPersist!", "Error:", JOptionPane.ERROR_MESSAGE );
            }
            else
                JOptionPane.showMessageDialog( m_aFrame,
                                                "Can't create StorageFactory!",
                                                "Error:",
                                                JOptionPane.ERROR_MESSAGE );
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in saveStorageToFileURI():", JOptionPane.ERROR_MESSAGE );
        }

    }

    public void loadFileURI( String aFileURI )
    {
        try
        {
            Object oStorageFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.StorageFactory" );
            XSingleServiceFactory xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
                                                                                        XSingleServiceFactory.class,
                                                                                        oStorageFactory );
            Object aArgs[] = new Object[2];
            aArgs[0] = aFileURI;
            aArgs[1] = new Integer( ElementModes.ELEMENT_READWRITE );

            Object oStorage = xStorageFactory.createInstanceWithArguments( aArgs );
            XStorage xTargetStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oStorage );

            Object oEmbedCreator = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectCreator" );
            XEmbedObjectCreator xEmbedCreator = (XEmbedObjectCreator)UnoRuntime.queryInterface(
                                                                                    XEmbedObjectCreator.class,
                                                                                    oEmbedCreator );

            XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
                                                                            xTargetStorage );
            if ( xNameAccess == null )
            {
                JOptionPane.showMessageDialog( m_aFrame, "No XNameAccess!", "Error:", JOptionPane.ERROR_MESSAGE );
                return;
            }

            Object oEmbObj = null;
            if ( xNameAccess.hasByName( "LinkName" ) && xTargetStorage.isStreamElement( "LinkName" ) )
            {
            /*
                // OOo links will not be tested until they have correct persistence
                XStream xLinkStream = xTargetStorage.openStreamElement( "LinkName", ElementModes.ELEMENT_READ );
                if ( xLinkStream != null )
                {
                    XInputStream xInStream = xLinkStream.getInputStream();
                    if ( xInStream != null )
                    {
                        byte[][] pBuff = new byte[1][0];
                        int nRead = xInStream.readBytes( pBuff, 1000 );
                        m_aLinkURI = new String( pBuff[0] );
                        xInStream.closeInput();
                        oEmbObj = xEmbedCreator.createInstanceLink( m_aLinkURI );
                        m_bLinkObj = true;
                    }
                }
            */
            }
            else
                oEmbObj = xEmbedCreator.createInstanceInitFromEntry( xTargetStorage,
                                                                    "EmbedSub",
                                                                    false,
                                                                    new PropertyValue[0] );

            m_xEmbedObj = (XEmbeddedObject)UnoRuntime.queryInterface( XEmbeddedObject.class, oEmbObj );

            if ( m_xEmbedObj != null )
            {
                m_xStorage = xTargetStorage;
                m_bOwnFile = true;
            }
            else
                JOptionPane.showMessageDialog( m_aFrame,
                                               "Can't create EmbedObject from storage!",
                                               "Error:",
                                               JOptionPane.ERROR_MESSAGE );
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in loadFileURI():", JOptionPane.ERROR_MESSAGE );
        }
    }

    public void storeLinkToStorage()
    {
        if ( m_xStorage != null && m_bLinkObj )
        {
            try {
                XStream xLinkStream = m_xStorage.openStreamElement( "LinkName", ElementModes.ELEMENT_WRITE );

                if ( xLinkStream != null )
                {
                    XOutputStream xLinkOutStream = xLinkStream.getOutputStream();
                    XTruncate xTruncate = (XTruncate) UnoRuntime.queryInterface( XTruncate.class,
                                                                                 xLinkOutStream );
                    if ( xLinkOutStream != null && xTruncate != null )
                    {
                        xTruncate.truncate();

                        char[] aLinkChar = m_aLinkURI.toCharArray();
                        byte[] aLinkBytes = new byte[ aLinkChar.length ];
                        for ( int ind = 0; ind < aLinkChar.length; ind++ )
                            aLinkBytes[ind] = (byte)aLinkChar[ind];

                        xLinkOutStream.writeBytes( aLinkBytes );
                        xLinkOutStream.closeOutput();

                        XComponent xComponent = (XComponent) UnoRuntime.queryInterface( XComponent.class,
                                                                                        xLinkStream );
                        if ( xComponent != null )
                            xComponent.dispose();
                    }
                    else
                        JOptionPane.showMessageDialog( m_aFrame,
                                                        "The substream can not be truncated or written!",
                                                        "Error:",
                                                        JOptionPane.ERROR_MESSAGE );

                }
                else
                    JOptionPane.showMessageDialog( m_aFrame,
                                                    "Can't create/open substream!",
                                                    "Error:",
                                                    JOptionPane.ERROR_MESSAGE );
            }
            catch( Exception e )
            {
                JOptionPane.showMessageDialog( m_aFrame,
                                            e,
                                            "Exception in storeLinkToStorage:",
                                            JOptionPane.ERROR_MESSAGE );

            }
        }
    }

    public void storeLinkAsFileURI( String aFileURI )
    {
        try {
            Object oStorageFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.StorageFactory" );
            XSingleServiceFactory xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
                                                                                        XSingleServiceFactory.class,
                                                                                        oStorageFactory );
            if ( xStorageFactory != null )
            {
                Object aArgs[] = new Object[2];
                aArgs[0] = aFileURI;
                aArgs[1] = new Integer( ElementModes.ELEMENT_READWRITE );

                Object oStorage = xStorageFactory.createInstanceWithArguments( aArgs );
                XStorage xTargetStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oStorage );

                XComponent xComponent = (XComponent)UnoRuntime.queryInterface( XComponent.class, m_xStorage );
                xComponent.dispose();

                m_xStorage = xTargetStorage;
                m_bOwnFile = true;

                storeLinkToStorage();

                XTransactedObject xTransact = (XTransactedObject)UnoRuntime.queryInterface( XTransactedObject.class,
                                                                                            m_xStorage );
                if ( xTransact != null )
                    xTransact.commit();
            }
            else
                JOptionPane.showMessageDialog( m_aFrame,
                                                "Can't create StorageFactory!",
                                                "Error:",
                                                JOptionPane.ERROR_MESSAGE );
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in saveStorageToFileURI():", JOptionPane.ERROR_MESSAGE );
        }
    }

    public String getValidURL( String sFileURL )
    {
        // m_xTransformer must be set!
        URL[] aURLs = { new URL() };
        aURLs[0].Complete = sFileURL;

        try {
            if ( !m_xTransformer.parseSmart( aURLs, "" ) )
                throw new Exception();
        }
        catch( Exception e )
        {
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in getValidURL():", JOptionPane.ERROR_MESSAGE );
        }

        return aURLs[0].Complete;
    }

    public void disposeObject()
    {
        // TODO:
        // usage of object, storage and bitmap painter should be locked
        // but since possibility of rasecondition is very low
        // it is not really required for testing application

        clearObjectAndStorage();

        if ( m_aBitmapPainter != null )
        {
            m_aBitmapPainter.disconnectListener();
            m_aBitmapPainter = null;
        }
    }
}

