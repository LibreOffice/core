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

import java.awt.*;
import java.applet.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;

import javax.swing.JOptionPane;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

import com.sun.star.lang.XComponent;

import com.sun.star.beans.PropertyValue;

import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.XTransferable;

import com.sun.star.container.XNameAccess;

import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;

import com.sun.star.util.XCloseable;

import com.sun.star.embed.*;

public class EmbedContApp extends Applet implements MouseListener, XEmbeddedClient
{
    private XMultiServiceFactory m_xServiceFactory;

    private XEmbeddedObject m_xEmbedObj;
    private XStorage m_xStorage;

    private Frame m_aFrame;
    private Menu m_aFileMenu;
    private Menu m_aObjectMenu;
    private Toolkit m_aToolkit;
    private Image m_aImage;

    private boolean m_bOwnFile = false;

    private boolean m_bLinkObj = false;
    private String m_aLinkURI;

    public EmbedContApp( Frame aFrame, XMultiServiceFactory xServiceFactory )
    {
        m_aFrame = aFrame;
        m_xServiceFactory = xServiceFactory;
    }

    public void init()
    {
        resize( 640, 480 );
        setBackground( Color.gray );

        m_aToolkit = Toolkit.getDefaultToolkit();

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

        // Handle mouse clicks in our window.
//      addMouseListener( new MouseWatcher() );
        addMouseListener( this );
    }

    public void update( Graphics g )
    {
        paint( g );
    }

    public void paint( Graphics g )
    {
        super.paint( g );

        if ( m_xEmbedObj != null )
        {
            synchronized( this )
            {
                if ( m_aImage != null )
                    g.drawImage( m_aImage, 0, 0, EmbedContApp.this );
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
                if ( nOldState == EmbedStates.LOADED )
                {
                    m_xEmbedObj.changeState( EmbedStates.RUNNING );
                    nState = EmbedStates.RUNNING;
                }

                if ( nState == EmbedStates.ACTIVE || nState == EmbedStates.RUNNING )
                {
                    XComponentSupplier xCompProv = (XComponentSupplier)UnoRuntime.queryInterface(
                                                                                    XComponentSupplier.class,
                                                                                    m_xEmbedObj );
                    if ( xCompProv != null )
                    {
                        XCloseable xComp = xCompProv.getComponent();
                        XTransferable xTransfer = (XTransferable)UnoRuntime.queryInterface(
                                                                                    XTransferable.class,
                                                                                    xComp );
                        if ( xTransfer != null )
                        {
                            DataFlavor aFlavor = new DataFlavor();
                            aFlavor.MimeType = "image/png";
                            aFlavor.HumanPresentableName = "Portable Network Graphics";
                            aFlavor.DataType = new Type( byte[].class );

                            byte[] aPNGData = (byte[])AnyConverter.toArray( xTransfer.getTransferData( aFlavor ) );
                            if ( aPNGData != null && aPNGData.length != 0 )
                            {
                                synchronized( this )
                                {
                                    m_aImage = m_aToolkit.createImage( aPNGData );
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
            // activate object if exists and not active
            if ( m_xEmbedObj != null )
            {
                try {
                    m_xEmbedObj.changeState( EmbedStates.ACTIVE );
                }
                catch( Exception ex )
                {
                    JOptionPane.showMessageDialog( m_aFrame, ex, "Exception on mouse click", JOptionPane.ERROR_MESSAGE );
                }
            }
        }
    }

    public void mousePressed( MouseEvent e ){};
    public void mouseEntered( MouseEvent e ){};
    public void mouseExited( MouseEvent e ){};
    public void mouseReleased( MouseEvent e ){};

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

    class NewMenuItem extends MenuItem implements ActionListener // Menu New
    {
        public NewMenuItem()
        {
            super( "New", new MenuShortcut( KeyEvent.VK_A ));
            addActionListener( this );
        }

        public void actionPerformed( ActionEvent e )
        {
            // clear everything
            clearObjectAndStorage();

            repaint();
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
                        String aFileURI = aFile.toURI().toASCIIString();
                        try {
                            saveObject();

                            if ( m_bLinkObj )
                                storeLinkToStorage();

                            saveStorageAsFileURI( aFileURI );
                        }
                        catch( Exception ex )
                        {
                            JOptionPane.showMessageDialog( m_aFrame,
                                                            ex,
                                                            "Exception in SaveAsMenuItem:",
                                                            JOptionPane.ERROR_MESSAGE );
                        }
                    }
                }
            }
            else
                JOptionPane.showMessageDialog( m_aFrame, "No document is embedded!", "Error:", JOptionPane.ERROR_MESSAGE );
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
            // clear everything
            clearObjectAndStorage();

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
                    String aFileURI = aFile.toURI().toASCIIString();

                    // load from specified file
                    loadFileURI( aFileURI );

                    if ( m_xEmbedObj != null )
                    {
                        try {
                            m_xEmbedObj.setClientSite( EmbedContApp.this );
                        }
                        catch( Exception ex )
                        {
                            JOptionPane.showMessageDialog( m_aFrame,
                                                            ex,
                                                            "Exception in OpenFileMenuItem:",
                                                            JOptionPane.ERROR_MESSAGE );
                        }
                    }
                }
            }

            generateNewImage();
            repaint();
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
                        JOptionPane.showMessageDialog( m_aFrame,
                                                        ex,
                                                        "Exception during save operation in SaveMenuItem:",
                                                        JOptionPane.ERROR_MESSAGE );
                    }
                }
                else
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
                            String aFileURI = aFile.toURI().toASCIIString();
                            try {
                                saveObject();

                                if ( m_bLinkObj )
                                    storeLinkToStorage();

                                saveStorageAsFileURI( aFileURI );
                            }
                            catch( Exception ex )
                            {
                                JOptionPane.showMessageDialog( m_aFrame,
                                                                ex,
                                                                "Exception during 'save as' operation in SaveMenuItem:",
                                                                JOptionPane.ERROR_MESSAGE );
                            }
                        }
                    }
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
            // remove current object an init a new one
            clearObjectAndStorage();

            Object[] possibleValues = { "com.sun.star.comp.Writer.TextDocument",
                                        "com.sun.star.comp.Writer.GlobalDocument",
                                        "com.sun.star.comp.Writer.WebDocument",
                                        "com.sun.star.comp.Calc.SpreadsheetDocument",
                                        "com.sun.star.comp.Draw.PresentationDocument",
                                        "com.sun.star.comp.Draw.DrawingDocument",
                                        "com.sun.star.comp.Math.FormulaDocument" };

            String selectedValue = (String)JOptionPane.showInputDialog( null, "DocumentType", "Select",
                                                                        JOptionPane.INFORMATION_MESSAGE, null,
                                                                        possibleValues, possibleValues[0] );

            if ( selectedValue != null )
            {
                m_xStorage = createTempStorage();

                if ( m_xStorage != null )
                    m_xEmbedObj = createEmbedObject( selectedValue );
                else
                    JOptionPane.showMessageDialog( m_aFrame,
                                                    "Can't create temporary storage!",
                                                    "Error:",
                                                    JOptionPane.ERROR_MESSAGE );


                if ( m_xEmbedObj != null )
                {
                    try {
                        m_xEmbedObj.setClientSite( EmbedContApp.this );
                    }
                    catch( Exception ex )
                    {
                        JOptionPane.showMessageDialog( m_aFrame,
                                                        ex,
                                                        "Exception in NewObjectMenuItem:",
                                                        JOptionPane.ERROR_MESSAGE );
                    }
                }
            }

            generateNewImage();
            repaint();
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
            // first remove current object
            clearObjectAndStorage();

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
                    String aFileURI = aFile.toURI().toASCIIString();
                    m_xStorage = createTempStorage();

                    if ( m_xStorage != null )
                        m_xEmbedObj = loadEmbedObject( aFileURI );

                    if ( m_xEmbedObj != null )
                    {
                        try {
                            m_xEmbedObj.setClientSite( EmbedContApp.this );
                        }
                        catch( Exception ex )
                        {
                            JOptionPane.showMessageDialog( m_aFrame,
                                                            ex,
                                                            "Exception in LoadObjectMenuItem:",
                                                            JOptionPane.ERROR_MESSAGE );
                        }
                    }
                }
            }

            generateNewImage();
            repaint();
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
            // first remove current object
            clearObjectAndStorage();

            // open OpenFile dialog and load doc
            FileDialog aFileDialog = new FileDialog( m_aFrame, "Select sources to use for object init" );
            aFileDialog.show();
            if ( aFileDialog.getFile() != null )
            {
                m_xStorage = createTempStorage();

                String aFileName = aFileDialog.getDirectory() + aFileDialog.getFile();
                File aFile = new File( aFileName );
                if ( aFile != null )
                {
                    // create object from specified file
                    String aFileURI = aFile.toURI().toASCIIString();

                    m_xEmbedObj = createLinkObject( aFileURI );

                    if ( m_xEmbedObj != null )
                    {
                        m_aLinkURI = aFileURI;
                        m_bLinkObj = true;

                        try {
                            m_xEmbedObj.setClientSite( EmbedContApp.this );
                        }
                        catch( Exception ex )
                        {
                            JOptionPane.showMessageDialog( m_aFrame,
                                                            ex,
                                                            "Exception in LinkObjectMenuItem:",
                                                            JOptionPane.ERROR_MESSAGE );
                        }
                    }
                }
            }

            generateNewImage();
            repaint();
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
            if ( !m_bLinkObj )
            {
                JOptionPane.showMessageDialog( m_aFrame, "The object is not a link!", "Error:", JOptionPane.ERROR_MESSAGE );
                return;
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

                        XEmbedPersist xPersist = (XEmbedPersist)UnoRuntime.queryInterface( XEmbedPersist.class,
                                                                                        m_xEmbedObj );
                        if ( xPersist != null )
                        {
                            PropertyValue[] pEmp = new PropertyValue[0];
                            xPersist.setPersistentEntry( m_xStorage, "EmbedSub", EntryInitModes.NO_INIT, pEmp );
                            m_bLinkObj = false;
                            m_aLinkURI = null;
                        }
                        else
                            JOptionPane.showMessageDialog( m_aFrame,
                                                            "No XEmbedPersist in ConvertLink... !",
                                                            "Error:",
                                                            JOptionPane.ERROR_MESSAGE );
                    }
                    catch( Exception e1 )
                    {
                        JOptionPane.showMessageDialog( m_aFrame,
                                                        e1,
                                                        "Exception in ConvertLinkToEmbed:try 1 :",
                                                        JOptionPane.ERROR_MESSAGE );
                    }
                }
            }
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

        if ( pClassID != null )
        {
            // create embedded object based on the class ID
            try {
                Object oEmbedFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectFactory" );
                XEmbedObjectFactory xEmbedFactory = (XEmbedObjectFactory)UnoRuntime.queryInterface(
                                                                                        XEmbedObjectFactory.class,
                                                                                        oEmbedFactory );
                if ( xEmbedFactory != null )
                {
                    Object oEmbObj = xEmbedFactory.createInstanceInitNew( pClassID,
                                                                        "Dummy name",
                                                                        m_xStorage,
                                                                        "EmbedSub" );
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
            Object oEmbedFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectFactory" );
            XEmbedObjectFactory xEmbedFactory = (XEmbedObjectFactory)UnoRuntime.queryInterface(
                                                                                    XEmbedObjectFactory.class,
                                                                                    oEmbedFactory );
            if ( xEmbedFactory != null )
            {
                Object oEmbObj = xEmbedFactory.createInstanceLink( aLinkURL );
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
            JOptionPane.showMessageDialog( m_aFrame, e, "Exception in createLinkObject():", JOptionPane.ERROR_MESSAGE );
        }


        return xEmbObj;
    }


    public XEmbeddedObject loadEmbedObject( String aFileURI )
    {
        XEmbeddedObject xEmbObj = null;
        try {
            Object oEmbedFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectFactory" );
            XEmbedObjectFactory xEmbedFactory = (XEmbedObjectFactory)UnoRuntime.queryInterface(
                                                                                    XEmbedObjectFactory.class,
                                                                                    oEmbedFactory );
            if ( xEmbedFactory != null )
            {
                PropertyValue[] aMedDescr = { new PropertyValue(), new PropertyValue() };
                aMedDescr[0].Name = "URL";
                aMedDescr[0].Value = (Object) aFileURI;
                aMedDescr[1].Name = "ReadOnly";
                aMedDescr[1].Value = (Object) new Boolean( false );
                Object oEmbObj = xEmbedFactory.createInstanceInitFromMediaDescriptor( m_xStorage,
                                                                                    "EmbedSub",
                                                                                    aMedDescr );
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
        synchronized( this )
        {
            m_aImage = null;
        }

        m_bOwnFile = false;

        m_aLinkURI = null;
        m_bLinkObj = false;

        if ( m_xEmbedObj != null )
        {
            try {
                XComponent xComponent = (XComponent)UnoRuntime.queryInterface( XComponent.class, m_xEmbedObj );
                if ( xComponent != null )
                    xComponent.dispose();
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

    public void saveStorageAsFileURI( String aFileURI )
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
                aArgs[1] = new Integer( ElementModes.READWRITE );

                Object oStorage = xStorageFactory.createInstanceWithArguments( aArgs );
                XStorage xTargetStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oStorage );
                m_xStorage.copyToStorage( xTargetStorage );

                XComponent xComponent = (XComponent)UnoRuntime.queryInterface( XComponent.class, m_xStorage );
                xComponent.dispose();

                m_xStorage = xTargetStorage;
                m_bOwnFile = true;
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
            aArgs[1] = new Integer( ElementModes.READWRITE );

            Object oStorage = xStorageFactory.createInstanceWithArguments( aArgs );
            XStorage xTargetStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oStorage );

            Object oEmbedFactory = m_xServiceFactory.createInstance( "com.sun.star.embed.EmbeddedObjectFactory" );
            XEmbedObjectFactory xEmbedFactory = (XEmbedObjectFactory)UnoRuntime.queryInterface(
                                                                                    XEmbedObjectFactory.class,
                                                                                    oEmbedFactory );

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
                XStream xLinkStream = xTargetStorage.openStreamElement( "LinkName", ElementModes.READ );
                if ( xLinkStream != null )
                {
                    XInputStream xInStream = xLinkStream.getInputStream();
                    if ( xInStream != null )
                    {
                        byte[][] pBuff = new byte[1][0];
                        int nRead = xInStream.readBytes( pBuff, 1000 );
                        m_aLinkURI = new String( pBuff[0] );
                        xInStream.closeInput();
                        oEmbObj = xEmbedFactory.createInstanceLink( m_aLinkURI );
                        m_bLinkObj = true;
                    }
                }
            }
            else
                oEmbObj = xEmbedFactory.createInstanceInitFromEntry( xTargetStorage,
                                                                    "EmbedSub",
                                                                    false );

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
                XStream xLinkStream = m_xStorage.openStreamElement( "LinkName", ElementModes.WRITE );

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
}

