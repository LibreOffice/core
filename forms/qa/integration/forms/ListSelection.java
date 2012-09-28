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
package integration.forms;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.container.XNamed;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XIndexContainer;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XListBox;
import com.sun.star.script.XLibraryContainer;
import com.sun.star.script.XEventAttacherManager;
import com.sun.star.script.ScriptEventDescriptor;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.frame.XStorable;

public class ListSelection extends integration.forms.TestCase
{
    /** Creates a new instance of ListSelection */
    public ListSelection()
    {
        super( DocumentType.CALC );
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkUserListSelection"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Form Control List Selection Test";
    }

    /* ------------------------------------------------------------------ */
    public void checkUserListSelection() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        boolean interactiveTest = param.getBool( "Interactive" );

        if ( interactiveTest )
        {
            prepareDocument();
            waitForUserInput();
            closeDocumentByUI();
        }
        else
        {
            int runs = param.getInt( "Runs" );
            if ( runs == 0 )
                runs = 10;

            for ( int i = 0; i < runs; ++i )
            {
                log.println( "Round " + ( i + 1 ) + " of " + runs );
                prepareDocument();
                impl_clickListBox();
                synchronized( this ) { this.wait( 1000 ); }
                closeDocument();
            }
        }
    }

    /* ------------------------------------------------------------------ */
    final private void impl_clickListBox()
    {
        try
        {
            final int runs = 10;
            java.util.Random generator = new java.util.Random();
            for ( int i = 0; i < runs; ++i )
            {
                // obtain the active sheet
                XSpreadsheetView view = (XSpreadsheetView)m_document.getCurrentView().query( XSpreadsheetView.class );
                XSpreadsheet activeSheet = view.getActiveSheet();

                // Accessibility access to the list box control in this sheet
                XAccessible accessibleListBox = (XAccessible)UnoRuntime.queryInterface(
                    XAccessible.class, getListBoxControl( activeSheet ) );
                XAccessibleContext context = accessibleListBox.getAccessibleContext();

                // the first "accessible child" of a list box is its list
                XAccessibleSelection accessibleList = (XAccessibleSelection)UnoRuntime.queryInterface(
                    XAccessibleSelection.class, context.getAccessibleChild( 1 ) );

                int selectPosition = generator.nextInt( 5 );
                String selectSheetName = getListBoxControl( activeSheet ).getItem( (short)selectPosition );
                accessibleList.selectAccessibleChild( selectPosition );
                try
                {
                    synchronized( this )
                    {
                        this.wait( 500 );
                    }
                }
                catch( java.lang.InterruptedException e ) { }

                XNamed sheetName = (XNamed)UnoRuntime.queryInterface( XNamed.class, view.getActiveSheet() );
                assure( "sheet was not selected as expected!", sheetName.getName().equals( selectSheetName ) );
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
            failed( "caught an exception: " + e.toString() );
        }
    }

    /* ------------------------------------------------------------------ */
    final private void impl_setupListenerScript()
    {
        try
        {
            XPropertySet docProps = dbfTools.queryPropertySet( m_document.getDocument() );
            XLibraryContainer basicLibs = (XLibraryContainer)UnoRuntime.queryInterface(
                XLibraryContainer.class, docProps.getPropertyValue( "BasicLibraries" ) );
            XNameContainer basicLib = basicLibs.createLibrary( "default" );

            String sListSelectionScript =
                "Option Explicit\n" +
                "\n" +
                "Sub onListBoxSelected( oEvent as Object )\n" +
                "    Dim oView as Object\n" +
                "    Dim oSheet as Object\n" +
                "    Dim oSheets as Object\n" +
                "\n" +
                "    Dim oControlModel as Object\n" +
                "    Dim sSheet as String\n" +
                "\n" +
                "    if ( oEvent.Selected <> 65535 ) Then\n" +
                "        oControlModel = oEvent.Source.Model\n" +
                "        sSheet = oControlModel.StringItemList( oEvent.Selected )\n" +
                "\n" +
                "        oSheets = thisComponent.Sheets\n" +
                "        oSheet = oSheets.getByName(sSheet)\n" +
                "\n" +
                "       oView = thisComponent.CurrentController\n" +
                "       oView.setActiveSheet( oSheet )\n" +
                "    End If\n" +
                "End Sub\n" +
                "\n" +
                "Sub onButtonClicked\n" +
                "    MsgBox \"clicked\"\n" +
                "End Sub\n";

            basicLib.insertByName( "callbacks", sListSelectionScript );
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
            failed( "caught an exception: " + e.toString() );
        }
    }

    /* ------------------------------------------------------------------ */
    final private void impl_assignStarBasicScript( XPropertySet controlModel, String interfaceName, String interfaceMethod, String scriptCode )
    {
        try
        {
            XIndexContainer parentForm = (XIndexContainer)dbfTools.getParent( controlModel, XIndexContainer.class );

            XEventAttacherManager manager = (XEventAttacherManager)UnoRuntime.queryInterface(
                XEventAttacherManager.class, parentForm );

            int containerPosition = -1;
            for ( int i = 0; i < parentForm.getCount(); ++i )
            {
                XPropertySet child = dbfTools.queryPropertySet( parentForm.getByIndex( i ) );
                if ( child.equals( controlModel ) )
                {
                    containerPosition = i;
                    break;
                }
            }
            manager.registerScriptEvent( containerPosition, new ScriptEventDescriptor(
                interfaceName,
                interfaceMethod,
                "",
                "StarBasic",
                scriptCode
            ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
            failed( "caught an exception: " + e.toString() );
        }
    }

    /* ------------------------------------------------------------------ */
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();
        impl_setupListenerScript();

        SpreadsheetDocument document = (SpreadsheetDocument)m_document;
        XSpreadsheets sheets = document.getSheets();

        // delete all sheets except one
        String[] sheetNames = sheets.getElementNames();
        for ( short i = 1; i < sheetNames.length; ++i )
            sheets.removeByName( sheetNames[ i ] );

        // need 5 sheets
        String[] newSheetNames = new String[] { "first", "second", "third", "forth", "fifth" };

        // give the first one the right name
        XNamed sheet = (XNamed)UnoRuntime.queryInterface( XNamed.class,
            sheets.getByName( sheetNames[ 0 ] )
        );
        sheet.setName( newSheetNames[ 0 ] );

        // add some dummy buttons
        for ( int i = 0; i < 4; ++i )
        {
            XPropertySet buttonModel = m_formLayer.createControlAndShape( "CommandButton", 10, 10 + i * 10, 30, 8 );
            impl_assignStarBasicScript( buttonModel, "XActionListener", "actionPerformed", "document:default.callbacks.onButtonClicked" );
        }

        // and a list box
        XPropertySet listBox = m_formLayer.createControlAndShape( "ListBox", 50, 10, 40, 6 );
        listBox.setPropertyValue( "Dropdown", new Boolean( true ) );
        listBox.setPropertyValue( "StringItemList", newSheetNames );
        listBox.setPropertyValue( "Name", "ListBox" );

        impl_assignStarBasicScript( listBox, "XItemListener", "itemStateChanged", "document:default.callbacks.onListBoxSelected" );

        // clone this sheet
        for ( short i = 1; i < newSheetNames.length; ++i )
            sheets.copyByName( newSheetNames[0], newSheetNames[i], i );

        // switch the thing to alive mode
        m_document.getCurrentView().toggleFormDesignMode();

        try
        {
            XStorable storable = (XStorable)m_document.query( XStorable.class );
            java.io.File testFile = java.io.File.createTempFile( getTestObjectName(),".ods");
            storable.storeAsURL( testFile.getAbsoluteFile().toURI().toURL().toString(), new com.sun.star.beans.PropertyValue[]{} );
            testFile.deleteOnExit();
        }
        catch( java.lang.Throwable e )
        {
            e.printStackTrace();
            failed( "caught an exception: " + e.toString() );
        }
    }

    /* ------------------------------------------------------------------ */
    protected XControlModel getListBoxModel( XSpreadsheet sheet )
    {
        XDrawPageSupplier suppPage = (XDrawPageSupplier)UnoRuntime.queryInterface(
            XDrawPageSupplier.class, sheet );
        FormComponent formsRoot = new FormComponent( suppPage.getDrawPage() );
        XControlModel listBoxModel = (XControlModel)formsRoot.getByIndex( 0 ).
            getByName( "ListBox" ).query( XControlModel.class );
        return listBoxModel;
    }

    /* ------------------------------------------------------------------ */
    protected XListBox getListBoxControl( XSpreadsheet sheet ) throws com.sun.star.uno.Exception
    {
        return (XListBox)UnoRuntime.queryInterface(
            XListBox.class, m_document.getCurrentView().getControl( getListBoxModel( sheet ) ) );
    }
 }
