/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ListSelection.java,v $
 * $Revision: 1.7 $
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
package integration.forms;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.container.XNamed;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XIndexContainer;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XListBox;
import com.sun.star.script.XLibraryContainer;
import com.sun.star.script.XEventAttacherManager;
import com.sun.star.script.ScriptEventDescriptor;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.frame.XStorable;

import integration.forms.DocumentHelper;

public class ListSelection extends integration.forms.TestCase implements com.sun.star.awt.XItemListener
{
    private final static boolean m_useJavaCallbacks = false;

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
                clickTheListBox();
                synchronized( this ) { this.wait( 1000 ); }
                closeDocument();
            }
        }
    }

    /* ------------------------------------------------------------------ */
    final protected void clickTheListBox()
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

                // open the popup of the list box (not really necessary, but to better
                // simlate user action ...)
/*                XAccessibleAction listBoxActions = (XAccessibleAction)UnoRuntime.queryInterface(
                    XAccessibleAction.class, context );
                listBoxActions.doAccessibleAction( 0 );
*/

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
            System.err.println( e );
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    final protected void createListenerScript()
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
        }
    }

    /* ------------------------------------------------------------------ */
    final protected void assignCallbackScript( XPropertySet controlModel, String interfaceName, String interfaceMethod, String scriptCode )
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
        }
    }

    /* ------------------------------------------------------------------ */
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();
        if ( !m_useJavaCallbacks )
            createListenerScript();

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
            assignCallbackScript( buttonModel, "XActionListener", "actionPerformed", "document:default.callbacks.onButtonClicked" );
        }

        // and a list box
        XPropertySet listBox = m_formLayer.createControlAndShape( "ListBox", 50, 10, 40, 6 );
        listBox.setPropertyValue( "Dropdown", new Boolean( true ) );
        listBox.setPropertyValue( "StringItemList", newSheetNames );
        listBox.setPropertyValue( "Name", "ListBox" );

        if ( !m_useJavaCallbacks )
            assignCallbackScript( listBox, "XItemListener", "itemStateChanged", "document:default.callbacks.onListBoxSelected" );

        // clone this sheet
        for ( short i = 1; i < newSheetNames.length; ++i )
            sheets.copyByName( newSheetNames[0], newSheetNames[i], (short)i );

        // switch the thing to alive mode
        m_document.getCurrentView().toggleFormDesignMode();

        // add to the list box control as item listener
        if ( m_useJavaCallbacks )
        {
            XControl control = m_document.getCurrentView().getControl( listBox );
            XListBox listBoxControl = (XListBox)UnoRuntime.queryInterface(
                XListBox.class, control );
            listBoxControl.addItemListener( this );
        }

        try
        {
            XStorable storable = (XStorable)m_document.query( XStorable.class );
            storable.storeAsURL( java.io.File.createTempFile( getTestObjectName(),".oxs").getAbsoluteFile().toURL().toString(), new com.sun.star.beans.PropertyValue[]{} );
        }
        catch( java.lang.Throwable e )
        {
            e.printStackTrace();
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

    /* ------------------------------------------------------------------ */
    public void itemStateChanged( com.sun.star.awt.ItemEvent event ) throws com.sun.star.uno.RuntimeException
    {
        try
        {
            // get the selected string
            XControl control = (XControl)UnoRuntime.queryInterface( XControl.class,
                event.Source );
            XPropertySet model = dbfTools.queryPropertySet( control.getModel() );
            String[] entries = (String[])model.getPropertyValue( "StringItemList" );
            String selectedEntry = entries[ event.Selected ];

            // activate this sheet
            SpreadsheetDocument document = (SpreadsheetDocument)m_document;
            XSpreadsheet sheet = (XSpreadsheet)UnoRuntime.queryInterface(
                XSpreadsheet.class, document.getSheets().getByName( selectedEntry ) );
            XSpreadsheetView view = (XSpreadsheetView)m_document.getCurrentView().query( XSpreadsheetView.class );
            view.setActiveSheet( sheet );

            if ( m_useJavaCallbacks )
            {
                // after we switched to another sheet, we need to register at its list box
                // control, again. The reason is that controls exist as long as their sheet is active.
                XListBox listBoxControl = getListBoxControl( sheet );
                listBoxControl.addItemListener( this );

                // in the list box of this new sheet, select its name. Everything else
                // is way too confusing
                listBoxControl.selectItem( selectedEntry, true );
                    // don't do it. It deadlocks :(
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    public void disposing( com.sun.star.lang.EventObject rEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in by now
    }
 }
