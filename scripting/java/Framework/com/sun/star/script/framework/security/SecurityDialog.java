/*************************************************************************
 *
 *  $RCSfile: SecurityDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-02-06 17:21:23 $
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

package com.sun.star.script.framework.security;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;

import com.sun.star.beans.XPropertySet;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XInitialization;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.task.XJobExecutor;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class SecurityDialog extends WeakBase implements XServiceInfo, XDialog,
XInitialization {

    static final String __serviceName = "com.sun.star.scripting.framework.security.SecurityDialog";

    // DOUBLE CHECK ALL STRINGS AGAINST STARBASIC!!!!!!
    private static final String _label1Name = "Label1";
    private static final String _label1String = "This document contains macros. Do you want to allow these macros to be run?";

    private static final String _label2Name = "Label2";
    private static final String _label2String = "This document contains macros. According to the security settings, the macros in this document should not be run. Do you want to run them anyway?";

    private static final String _label3Name = "Label3";
    private static final String _label3String = "Add this directory to the list of secure paths: ";
    private static final String _checkBoxName = "CheckBox";

    private static final String _title = "Run Macro";
    private static final String _runMacro = "Run";
    private static final String _runButtonName = "Run";
    private static final String _doNotRunMacro = "Do Not Run";
    private static final String _doNotRunButtonName = "DoNotRun";

    private static final int dialogX = 100;
    private static final int dialogY = 100;
    private static final int dialogW = 200;
    private static final int dialogH = 40;

    private static int cbIncrX = 0;
    private static int cbIncrY = 0;
    private static int cbIncrW = 0;
    private static int cbIncrH = 0;

    private static final int runButtonW = 30;
    private static final int runButtonH = 14;
    private static final int doNotRunButtonW = 30;
    private static final int doNotRunButtonH = 14;
    private static final int label1X = 0;
    private static final int label1Y = 0;
    private static final int label1W = 0;
    private static final int label1H = 0;
    private static final int label2X = 0;
    private static final int label2Y = 0;
    private static final int label2W = 0;
    private static final int label2H = 0;
    private static final int label3X = 0;
    private static final int label3Y = 0;
    private static final int label3W = 0;
    private static final int label3H = 0;
    private static final int checkBoxX = 0;
    private static final int checkBoxY = 0;
    private static final int checkBoxW = 0;
    private static final int checkBoxH = 0;

    private boolean checkBoxDialog;
    private String checkBoxPath;
    private String _pushed = _doNotRunButtonName;

    private XComponentContext _xComponentContext;
    private XDialog _xDialog;

    public SecurityDialog( XComponentContext xComponentContext ) {

        _xComponentContext = xComponentContext;
    }

    public void initialize( Object[] args )
    {
        // figure out if we need a checkbox
        if ( args.length != 1)
        {
            //check args is a path
            // set checkBoxPath with the arg
            checkBoxPath = (String) args[0];
            checkBoxDialog=true;

        }
        else
        {
            checkBoxDialog=false;
        }

        // now try and create the dialog
        try
        {
            _xDialog = createDialog();
        }
        catch ( Exception e )
        {
            System.out.println("Couldn't create dialog");
        }

    }

    // static component operations
    public static XSingleServiceFactory __getServiceFactory( String implName,
                                                             XMultiServiceFactory multiFactory,
                                                             XRegistryKey regKey ) {
        XSingleServiceFactory xSingleServiceFactory = null;
        if ( implName.equals( SecurityDialog.class.getName() ) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                SecurityDialog.class, SecurityDialog.__serviceName, multiFactory, regKey );
        }
        return xSingleServiceFactory;
    }

    public static boolean __writeRegistryServiceInfo( XRegistryKey regKey ) {
        return FactoryHelper.writeRegistryServiceInfo(
            SecurityDialog.class.getName(), SecurityDialog.__serviceName, regKey );
    }

    // XServiceInfo
    public String getImplementationName(  ) {
        return getClass().getName();
    }

    // XServiceInfo
    public boolean supportsService( /*IN*/String serviceName ) {
        if ( serviceName.equals( __serviceName))
            return true;
        return false;
    }

    // XServiceInfo
    public String[] getSupportedServiceNames(  ) {
        String[] retValue= new String[0];
        retValue[0] = __serviceName;
        return retValue;
    }


    /** method for creating a dialog at runtime
     */
    private XDialog createDialog() throws com.sun.star.uno.Exception {

        // get the service manager from the component context
        XMultiComponentFactory xMultiComponentFactory = _xComponentContext.getServiceManager();

        // create the dialog model and set the properties
        Object dialogModel = xMultiComponentFactory.createInstanceWithContext(
            "com.sun.star.awt.UnoControlDialogModel", _xComponentContext );
        XPropertySet xPSetDialog = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, dialogModel );
        xPSetDialog.setPropertyValue( "PositionX", new Integer( dialogX+cbIncrX  ) );
        xPSetDialog.setPropertyValue( "PositionY", new Integer( dialogY+cbIncrY ));
        xPSetDialog.setPropertyValue( "Width", new Integer( dialogW+cbIncrW ));
        xPSetDialog.setPropertyValue( "Height", new Integer( dialogH+cbIncrH ));
        xPSetDialog.setPropertyValue( "Title", _title );

        // get the service manager from the dialog model
        XMultiServiceFactory xMultiServiceFactory = ( XMultiServiceFactory )UnoRuntime.queryInterface(
            XMultiServiceFactory.class, dialogModel );

        // create the Run Macro button model and set the properties
        Object runButtonModel = xMultiServiceFactory.createInstance(
            "com.sun.star.awt.UnoControlButtonModel" );
        XPropertySet xPSetButton = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, runButtonModel );
        xPSetButton.setPropertyValue( "PositionX", new Integer( (((dialogX+cbIncrX)/2)-runButtonW -1) ));
        xPSetButton.setPropertyValue( "PositionY", new Integer( ((dialogY+cbIncrY-runButtonH-1)) ));
        xPSetButton.setPropertyValue( "Width", new Integer( runButtonW ));
        xPSetButton.setPropertyValue( "Height", new Integer( runButtonH ));
        xPSetButton.setPropertyValue( "Name", _runButtonName );
        xPSetButton.setPropertyValue( "TabIndex", new Short( (short)0 ) );
        xPSetButton.setPropertyValue( "Label", _runMacro );

        // create the Dont Run Macro button model and set the properties
        Object doNotRunButtonModel = xMultiServiceFactory.createInstance(
            "com.sun.star.awt.UnoControlButtonModel" );
        xPSetButton = ( XPropertySet )UnoRuntime.queryInterface(
            XPropertySet.class, doNotRunButtonModel );
        xPSetButton.setPropertyValue( "PositionX", new Integer( ( ((dialogX+cbIncrX)/2) + 1) ));
        xPSetButton.setPropertyValue( "PositionY", new Integer( ( dialogY+cbIncrY-doNotRunButtonH-1 ) ));
        xPSetButton.setPropertyValue( "Width", new Integer( doNotRunButtonW ));
        xPSetButton.setPropertyValue( "Height", new Integer( doNotRunButtonH ));
        xPSetButton.setPropertyValue( "Name", _doNotRunButtonName );
        xPSetButton.setPropertyValue( "TabIndex", new Short( (short)0 ) );
        xPSetButton.setPropertyValue( "Label", _doNotRunMacro );

        // insert the control models into the dialog model
        XNameContainer xNameCont = ( XNameContainer )UnoRuntime.queryInterface(
            XNameContainer.class, dialogModel );
        xNameCont.insertByName( _runButtonName, runButtonModel );
        xNameCont.insertByName( _doNotRunButtonName, doNotRunButtonModel );

        if ( checkBoxDialog )
        {
            // create the label model and set the properties
            Object label2Model = xMultiServiceFactory.createInstance(
                "com.sun.star.awt.UnoControlFixedTextModel" );
            XPropertySet xPSetLabel = ( XPropertySet )UnoRuntime.queryInterface(
                XPropertySet.class, label2Model );
            xPSetLabel.setPropertyValue( "PositionX", new Integer( label2X ));
            xPSetLabel.setPropertyValue( "PositionY", new Integer( label2Y ));
            xPSetLabel.setPropertyValue( "Width", new Integer( label2W ));
            xPSetLabel.setPropertyValue( "Height", new Integer( label2H ));
            xPSetLabel.setPropertyValue( "Name", _label2Name );
            xPSetLabel.setPropertyValue( "TabIndex", new Short( (short)1 ) );
            xPSetLabel.setPropertyValue( "Label", _label2String );

            // create the label model and set the properties
            Object checkBoxModel = xMultiServiceFactory.createInstance(
                "com.sun.star.awt.UnoControlCheckBoxModel" );
            XPropertySet xPSetCheckBox = ( XPropertySet )UnoRuntime.queryInterface(
                XPropertySet.class, checkBoxModel );
            xPSetCheckBox.setPropertyValue( "PositionX", new Integer( checkBoxX ));
            xPSetCheckBox.setPropertyValue( "PositionY", new Integer( checkBoxY ));
            xPSetCheckBox.setPropertyValue( "State", new Integer (0) );
            xPSetCheckBox.setPropertyValue( "Name", _checkBoxName );
            xPSetCheckBox.setPropertyValue( "TabIndex", new Short( (short)1 ) );

            // create the label model and set the properties
            Object label3Model = xMultiServiceFactory.createInstance(
                "com.sun.star.awt.UnoControlFixedTextModel" );
            XPropertySet xPSetLabel2 = ( XPropertySet )UnoRuntime.queryInterface(
                XPropertySet.class, label3Model );
            xPSetLabel2.setPropertyValue( "PositionX", new Integer( label3X ));
            xPSetLabel2.setPropertyValue( "PositionY", new Integer( label3Y ));
            xPSetLabel2.setPropertyValue( "Width", new Integer( label3W ));
            xPSetLabel2.setPropertyValue( "Height", new Integer( label3H ));
            xPSetLabel2.setPropertyValue( "Name", _label3Name );
            xPSetLabel2.setPropertyValue( "TabIndex", new Short( (short)1 ) );
            xPSetLabel2.setPropertyValue( "Label", _label3String );
            // insert the control models into the dialog model
            xNameCont.insertByName( _label2Name, label2Model );
            xNameCont.insertByName( _checkBoxName, checkBoxModel );
            xNameCont.insertByName( _label2Name, label3Model );
        }
        else
        {
            // create the label model and set the properties
            Object labelModel = xMultiServiceFactory.createInstance(
                "com.sun.star.awt.UnoControlFixedTextModel" );
            XPropertySet xPSetLabel = ( XPropertySet )UnoRuntime.queryInterface(
                XPropertySet.class, labelModel );
            xPSetLabel.setPropertyValue( "PositionX", new Integer( label1X ));
            xPSetLabel.setPropertyValue( "PositionY", new Integer( label1Y ));
            xPSetLabel.setPropertyValue( "Width", new Integer( label1W ));
            xPSetLabel.setPropertyValue( "Height", new Integer( label1H ));
            xPSetLabel.setPropertyValue( "Name", _label1Name );
            xPSetLabel.setPropertyValue( "TabIndex", new Short( (short)1 ) );
            xPSetLabel.setPropertyValue( "Label", _label1String );

            // insert the control models into the dialog model
            xNameCont.insertByName( _label1Name, labelModel );
        }

        // create the dialog control and set the model
        Object dialog = xMultiComponentFactory.createInstanceWithContext(
            "com.sun.star.awt.UnoControlDialog", _xComponentContext );
        XControl xControl = ( XControl )UnoRuntime.queryInterface(
            XControl.class, dialog );
        XControlModel xControlModel = ( XControlModel )UnoRuntime.queryInterface(
            XControlModel.class, dialogModel );
        xControl.setModel( xControlModel );

        // add an action listener to the button control
        XControlContainer xControlCont = ( XControlContainer )UnoRuntime.queryInterface(
            XControlContainer.class, dialog );

        // Add to yes button
        Object objectButton = xControlCont.getControl( _runButtonName );
        XButton xButton = ( XButton )UnoRuntime.queryInterface(
            XButton.class, objectButton );
        xButton.addActionListener( new ActionListenerImpl( xControlCont, _runButtonName ) );

        // add to no button
        objectButton = xControlCont.getControl( _doNotRunButtonName );
        xButton = ( XButton )UnoRuntime.queryInterface(
            XButton.class, objectButton );
        xButton.addActionListener( new ActionListenerImpl( xControlCont, _doNotRunButtonName ) );

        // create a peer
        Object toolkit = xMultiComponentFactory.createInstanceWithContext(
            "com.sun.star.awt.ExtToolkit", _xComponentContext );
        XToolkit xToolkit = ( XToolkit )UnoRuntime.queryInterface(
            XToolkit.class, toolkit );
        XWindow xWindow = ( XWindow )UnoRuntime.queryInterface(
            XWindow.class, xControl );
        xWindow.setVisible( false );
        xControl.createPeer( xToolkit, null );

        // execute the dialog
        XDialog xDialog = ( XDialog )UnoRuntime.queryInterface(
            XDialog.class, dialog );
        return xDialog;
    }
    public short execute()
    {

        _pushed = _doNotRunButtonName;
        System.out.println("**** Before execute " );
        _xDialog.execute();
        System.out.println("**** After execute " );

        /*// dispose the dialog
        XComponent xComponent = ( XComponent )UnoRuntime.queryInterface(
            XComponent.class, dialog );
        xComponent.dispose();*/
        if ( _pushed.equals( _runButtonName ) )
        {
            return 1;
        }
        return 0;
    }

    public void endExecute()
    {
        _xDialog.endExecute();
    }

   public String getTitle()
   {
        return _xDialog.getTitle();
   }

   public void setTitle( String Title )
   {
        _xDialog.setTitle( Title );
   }

   public void dispose ()
   {
        XComponent xComponent = ( XComponent )UnoRuntime.queryInterface(
            XComponent.class, _xDialog );
       xComponent.dispose();
   }

   public void addEventListener ( com.sun.star.lang.XEventListener xListener )
   {
        XComponent xComponent = ( XComponent )UnoRuntime.queryInterface(
            XComponent.class, _xDialog );
       xComponent.addEventListener( xListener );
   }


   public void removeEventListener ( com.sun.star.lang.XEventListener aListener )
   {
        XComponent xComponent = ( XComponent )UnoRuntime.queryInterface(
            XComponent.class, _xDialog );
       xComponent.removeEventListener( aListener );
   }

    public class ActionListenerImpl implements com.sun.star.awt.XActionListener {
        private XControlContainer _xControlCont;
        private String _buttonName;
        public ActionListenerImpl( XControlContainer xControlCont, String buttonName ) {
            _xControlCont = xControlCont;
            _buttonName = buttonName;
        }

        // XEventListener
        public void disposing( EventObject eventObject ) {
            _xControlCont = null;
        }

        // XActionListener
        public void actionPerformed( ActionEvent actionEvent ) {
            /*
            Object buttonModel = actionEvent.Source;
            try
            {
                String name = com.sun.star.uno.AnyConverter.toType( buttonModel ).getTypeName();
                System.out.println( "Type is " + name );
            }
            catch ( Exception e )
            {
                e.printStackTrace();
            }
            System.out.println( " source is " + buttonModel);
            System.out.println( " desc is " + actionEvent.ActionCommand );
            XPropertySet xPSetLabel = ( XPropertySet )UnoRuntime.queryInterface(
                XPropertySet.class, buttonModel );
            String lable = "";
            if ( xPSetLabel != null )
            {
                try
                {
                    lable = (String) xPSetLabel.getPropertyValue( "Name" );
                }
                catch ( Exception e)
                {
                    e.printStackTrace();
                }
            }
            else
            {
                System.out.println(" No XPropertySet found ");
            }
            // set label text
            Object label = _xControlCont.getControl( _label1Name );
            XFixedText xLabel = ( XFixedText )UnoRuntime.queryInterface(
                XFixedText.class, label );
            xLabel.setText( _label1String + _nCounts );
          */
          _pushed = _buttonName;
          System.out.println("** Button pushed ->" + _pushed );

          _xDialog.endExecute();
        }
    }
}
