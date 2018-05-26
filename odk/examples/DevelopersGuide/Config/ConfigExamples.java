/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// Import everything we use

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XHierarchicalPropertySet;
import com.sun.star.beans.XMultiHierarchicalPropertySet;
import com.sun.star.beans.XPropertyState;
import com.sun.star.beans.XMultiPropertyStates;

import com.sun.star.configuration.XTemplateInstance;

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameReplace;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.container.XChild;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XHierarchicalName;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.EventObject;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;

import com.sun.star.util.XChangesBatch;
import com.sun.star.util.XChangesNotifier;
import com.sun.star.util.XChangesListener;
import com.sun.star.util.ChangesEvent;
// Config examples

/* These examples show how to use the following features of the Config API:

    o Accessing data
    o Updating data
    o Updating properties in groups
    o Adding and removing items in sets
    o Resetting data to their defaults

   Each example is in a separate method call.
*/
public class ConfigExamples
{
    // The ComponentContext interface of the remote component context
    private final XComponentContext mxContext;

    // The MultiComponentFactory interface of the ServiceManager
    private final XMultiComponentFactory mxServiceManager;

    // The MultiServiceFactory interface of the ConfigurationProvider
    private XMultiServiceFactory mxProvider = null;

    public static void main( String args[] )
    {
        try {
            // get the remote office component context
            com.sun.star.uno.XComponentContext xContext =
                com.sun.star.comp.helper.Bootstrap.bootstrap();

            if( xContext != null )
                System.out.println("Connected to a running office ...");
            else
                System.out.println( "ERROR: Cannot connect - no remote component context available." );

            // Create an instance of the class and call its run method
            ConfigExamples aExample = new ConfigExamples(xContext);
            aExample.run( );

            // if you own the service manager dispose it here
            // to ensure that the default provider is properly disposed and flushed
            System.exit(0);
        }
        catch( Exception e )
        {
            e.printStackTrace();
            System.exit(-1);
        }
    }

    /** Create a ConfigExamples instance supplying a service factory
    */
    public ConfigExamples(XComponentContext xContext)
    {
        mxContext = xContext;
        mxServiceManager = xContext.getServiceManager();
    }

    /** Run the examples with a default ConfigurationProvider
    */
    public void run()
        throws com.sun.star.uno.Exception
    {
        mxProvider = createProvider();

        runExamples( );

        // we are using the default ConfigurationProvider, so we must not dispose it
        mxProvider = null;
    }

    /** Run the examples with a given ConfigurationProvider
    */
    public void runExamples( )
    {
        if (checkProvider(mxProvider))
        {
            System.out.println("\nStarting examples.");

            readDataExample();

            browseDataExample();

            updateGroupExample();

            resetGroupExample();

            updateSetExample();

            System.out.println("\nAll Examples completed.");
        }
        else
            System.out.println("ERROR: Cannot run examples without ConfigurationProvider.");

    }

    /** Do some simple checks, if there is a valid ConfigurationProvider
    */
    public static boolean checkProvider(XMultiServiceFactory xProvider)
    {
        // check the provider we have
        if (xProvider == null)
        {
            System.out.println("No provider available. Cannot access configuration data.");
            return false;

        }

        try
        {
            // check the provider implementation
            XServiceInfo xProviderServices =
                UnoRuntime.queryInterface( XServiceInfo.class, xProvider );

            if (xProviderServices == null ||
                !xProviderServices.supportsService("com.sun.star.configuration.ConfigurationProvider"))
            {
                System.out.println("WARNING: The provider is not a com.sun.star.configuration.ConfigurationProvider");
            }

            if (xProviderServices != null)
            {
                System.out.println("Using provider implementation: " + xProviderServices.getImplementationName());
            }

            return true;
        }
        catch (com.sun.star.uno.RuntimeException e)
        {
            System.err.println("ERROR: Failure while checking the provider services.");
            e.printStackTrace();
            return false;
        }
    }

    /** Get the provider we have
     */
    public XMultiServiceFactory getProvider( )
    {
        return mxProvider;
    }

    /** Create a default configuration provider
     */
    public XMultiServiceFactory createProvider( )
        throws com.sun.star.uno.Exception
    {
        final String sProviderService = "com.sun.star.configuration.ConfigurationProvider";

        // create the provider and return it as a XMultiServiceFactory
        XMultiServiceFactory xProvider = UnoRuntime.queryInterface(XMultiServiceFactory.class,
            mxServiceManager.createInstanceWithContext(sProviderService,
                                                       mxContext));

        return xProvider;
    }

    /** Create a specified read-only configuration view
     */
    public Object createConfigurationView( String sPath )
        throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xProvider = getProvider();

        // The service name: Need only read access:
        final String sReadOnlyView = "com.sun.star.configuration.ConfigurationAccess";

        // creation arguments: nodepath
        com.sun.star.beans.PropertyValue aPathArgument = new com.sun.star.beans.PropertyValue();
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = sPath;

        Object[] aArguments = new Object[1];
        aArguments[0] = aPathArgument;

        // create the view
        Object xViewRoot = xProvider.createInstanceWithArguments(sReadOnlyView, aArguments);

        return xViewRoot;
    }

    /** Create a specified updatable configuration view
     */
    Object createUpdatableView( String sPath )
        throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xProvider = getProvider();

        // The service name: Need update access:
        final String cUpdatableView = "com.sun.star.configuration.ConfigurationUpdateAccess";

        // creation arguments: nodepath
        com.sun.star.beans.PropertyValue aPathArgument = new com.sun.star.beans.PropertyValue();
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = sPath;

        Object[] aArguments = new Object[1];
        aArguments[0] = aPathArgument;

        // create the view
        Object xViewRoot = xProvider.createInstanceWithArguments(cUpdatableView, aArguments);

        return xViewRoot;
    }

    /** This method demonstrates read access to data
     */
    protected void readDataExample ()
    {
        try
        {
            System.out.println("\n--- starting example: read grid option settings --------------------");
            Object aData = readGridConfiguration( );
            System.out.println("Read grid options: " + aData);

        }
        catch ( Exception e )
        {
            e.printStackTrace();
        }
    }

    /** This method demonstrates browsing access to data
     */
    protected void browseDataExample ()
    {
        try
        {
            System.out.println("\n--- starting example: browse filter configuration ------------------");
            printRegisteredFilters( );
        }
        catch ( Exception e )
        {
            e.printStackTrace();
        }
    }

    /** This method demonstrates update access to group data
     */
    protected void updateGroupExample ()
    {
        try
        {
            System.out.println("\n--- starting example: update group data --------------");
            editGridOptions( );
        }
        catch ( Exception e )
        {
            e.printStackTrace();
        }
    }

    /** This method demonstrates resetting data to its default state
     */
    protected void resetGroupExample ()
    {
        try
        {
            System.out.println("\n--- starting example: reset group data -----------------------------");
            Object aOldData = readGridConfiguration( );
            resetGridConfiguration( );
            Object aNewData = readGridConfiguration( );
            System.out.println("Before reset:   user grid options: " + aOldData);
            System.out.println("After reset: default grid options: " + aNewData);
        }
        catch ( Exception e )
        {
            e.printStackTrace();
        }
    }

    /** This method demonstrates update access to set data
     */
    protected void updateSetExample ()
    {
        try
        {
            System.out.println("\n--- starting example: update set data ---------------");
            storeSampleDataSource( );
        }
        catch ( Exception e )
        {
            e.printStackTrace();
        }
    }

// READ example
    /// class to hold information about grid settings
    private static class GridOptions
    {
        private boolean visible;
        private int resolution_x;
        private int resolution_y;
        private int subdivision_x;
        private int subdivision_y;

        @Override
        public String toString() {
            StringBuffer aBuffer = new StringBuffer();
            aBuffer.append("[ Grid is "); aBuffer.append(visible ? "VISIBLE" : "HIDDEN");
            aBuffer.append("; resolution  = (" + resolution_x  + "," + resolution_y  + ")");
            aBuffer.append("; subdivision = (" + subdivision_x + "," + subdivision_y + ")");
            aBuffer.append(" ]");
            return aBuffer.toString();
        }
    }

    /// This method reads information about grid settings
    protected GridOptions readGridConfiguration()
        throws com.sun.star.uno.Exception
    {
        // The path to the root element
        final String cGridOptionsPath = "/org.openoffice.Office.Calc/Grid";

        // create the view
        Object xViewRoot = createConfigurationView(cGridOptionsPath);

        // the result structure
        GridOptions options = new GridOptions();

      // accessing a single nested value
        XHierarchicalPropertySet xProperties =
            UnoRuntime.queryInterface(XHierarchicalPropertySet.class, xViewRoot);

        Object aVisible = xProperties.getHierarchicalPropertyValue("Option/VisibleGrid");
        options.visible = ((Boolean) aVisible).booleanValue();

      // accessing a nested object and its subproperties
        Object xSubdivision = xProperties.getHierarchicalPropertyValue("Subdivision");

        XMultiPropertySet xSubdivProperties =
            UnoRuntime.queryInterface(XMultiPropertySet.class, xSubdivision);

        // variables for multi-element access
        String[] aElementNames = new String[] { "XAxis", "YAxis" };

        Object[] aElementValues = xSubdivProperties.getPropertyValues(aElementNames);

        options.subdivision_x = ((Integer) aElementValues[0]).intValue();
        options.subdivision_y = ((Integer) aElementValues[1]).intValue();

      // accessing deeply nested subproperties
        Object xResolution = xProperties.getHierarchicalPropertyValue("Resolution");

        XMultiHierarchicalPropertySet xResolutionProperties =
            UnoRuntime.queryInterface(XMultiHierarchicalPropertySet.class, xResolution);

        aElementNames[0] = "XAxis/Metric";
        aElementNames[1] = "YAxis/Metric";

        aElementValues = xResolutionProperties.getHierarchicalPropertyValues(aElementNames);

        options.resolution_x = ((Integer) aElementValues[0]).intValue();
        options.resolution_y = ((Integer) aElementValues[1]).intValue();

      // all options have been retrieved - clean up and return
        // we are done with the view - dispose it

        UnoRuntime.queryInterface(XComponent.class, xViewRoot).dispose();

        return options;
    }

// BROWSE example
    /// Interface to process information when browsing the configuration tree
    public interface IConfigurationProcessor
    {
        /// process a value item
       void processValueElement( String sPath_, Object aValue_ );
        /// process a structural item
       void processStructuralElement( String sPath_, XInterface xElement_);
    }

    /// Internal method to recursively browse a structural element in preorder
    public void browseElementRecursively( XInterface xElement, IConfigurationProcessor aProcessor )
        throws com.sun.star.uno.Exception
    {
        // First process this as an element (preorder traversal)
        XHierarchicalName xElementPath =
            UnoRuntime.queryInterface(XHierarchicalName.class, xElement);

        String sPath = xElementPath.getHierarchicalName();

        aProcessor.processStructuralElement( sPath, xElement);

        // now process this as a container
        XNameAccess xChildAccess =
            UnoRuntime.queryInterface(XNameAccess.class, xElement);

        // get a list of child elements
        String[] aElementNames = xChildAccess.getElementNames();

        // and process them one by one
        for(int i=0; i< aElementNames.length; ++i)
        {
            Object aChild = xChildAccess.getByName( aElementNames[i] );
            // is it a structural element (object) ...
            if ( AnyConverter.isObject(aChild) && !AnyConverter.isArray(aChild) )
            {
                // then get an interface
                XInterface xChildElement = UnoRuntime.queryInterface(XInterface.class, aChild);

                // and continue processing child elements recursively
                browseElementRecursively( xChildElement, aProcessor );
            }
            // ... or is it a simple value
            else
            {
                // Build the path to it from the path of
                // the element and the name of the child
                String sChildPath;
                sChildPath =
                    xElementPath.composeHierarchicalName(aElementNames[i]);

                // and process the value
                aProcessor.processValueElement( sChildPath, aChild );
            }
        }
    }

    /** Method to browse the part rooted at sRootPath
        of the configuration that the Provider provides.

        All nodes will be processed by the IConfigurationProcessor passed.
    */
    public void browseConfiguration( String sRootPath, IConfigurationProcessor aProcessor )
        throws com.sun.star.uno.Exception
    {
        // create the root element
        XInterface xViewRoot = (XInterface)createConfigurationView( sRootPath );

        // now do the processing
        browseElementRecursively( xViewRoot, aProcessor );

        // we are done with the view - dispose it
        //   This assumes that the processor
        //   does not keep a reference to the elements in processStructuralElement

        UnoRuntime.queryInterface(XComponent.class,xViewRoot).dispose();
        xViewRoot = null;
    }

    /** Method to browse the filter configuration.

        Information about installed filters will be printed.
    */
    public void printRegisteredFilters()
        throws com.sun.star.uno.Exception
    {
        final String sFilterKey = "/org.openoffice.TypeDetection.Filter/Filters";

       // browse the configuration, dumping filter information
        browseConfiguration( sFilterKey,
           new IConfigurationProcessor () {
               /// prints Path and Value of properties
               public void processValueElement( String sPath_, Object aValue_ ) {
                   if (AnyConverter.isArray(aValue_))
                   {
                       final Object [] aArray = (Object [])aValue_;

                       System.out.print("\tValue: " + sPath_ + " = { ");
                       for (int i=0; i<aArray.length; ++i)
                       {
                           if (i != 0) System.out.print(", ");
                           System.out.print(aArray[i]);
                       }
                       System.out.println(" }");
                   }
                   else
                        System.out.println("\tValue: " + sPath_ + " = " + aValue_);
               }

               /// prints the Filter entries
               public void processStructuralElement( String sPath_, XInterface xElement_) {
                   // get template information, to detect instances of the 'Filter' template
                   XTemplateInstance xInstance =
                       UnoRuntime.queryInterface( XTemplateInstance .class,xElement_);

                   // only select the Filter entries
                   if (xInstance != null && xInstance.getTemplateName().endsWith("Filter")) {
                        XNamed xNamed = UnoRuntime.queryInterface(XNamed.class,xElement_);
                        System.out.println("Filter " + xNamed.getName() + " (" + sPath_ + ")");
                   }
               }
           } );
    }

// GROUP UPDATE example

    /** This method simulates editing configuration data using a GridEditor dialog class
    */
    public void editGridOptions( )
        throws com.sun.star.uno.Exception
    {
        // The path to the root element
        final String cGridOptionsPath = "/org.openoffice.Office.Calc/Grid";

      // create the view
        Object xViewRoot = createUpdatableView( cGridOptionsPath );

        // the 'editor'
        GridOptionsEditor dialog = new GridOptionsEditor();

      // set up the initial values and register listeners
        // get a data access interface, to supply the view with a model
        XMultiHierarchicalPropertySet xProperties =
            UnoRuntime.queryInterface(XMultiHierarchicalPropertySet.class, xViewRoot);

        dialog.setModel( xProperties );

        // get a listener object (probably an adapter) that notifies
        // the dialog of external changes to its model
        XChangesListener xListener = dialog.createChangesListener( );

        XChangesNotifier xNotifier =
            UnoRuntime.queryInterface(XChangesNotifier.class, xViewRoot);

        xNotifier.addChangesListener( xListener );

        // trigger the listener
        changeSomeData( cGridOptionsPath + "/Subdivision" );

        if (dialog.execute() == GridOptionsEditor.SAVE_SETTINGS)
        {
            // changes have been applied to the view here
            XChangesBatch xUpdateControl =
                UnoRuntime.queryInterface(XChangesBatch.class,xViewRoot);

            try
            {
               xUpdateControl.commitChanges();
            }
            catch (Exception e)
            {
                dialog.informUserOfError( e );
            }
        }

      // all changes have been handled - clean up and return
        // listener is done now
        xNotifier.removeChangesListener( xListener );

        // we are done with the view - dispose it
        UnoRuntime.queryInterface(XComponent.class, xViewRoot).dispose();
    }

    /** A class that changes some grid options settings

        The interface of this class is chose to resemble a possible UI dialog class
    */
    private class GridOptionsEditor {
        /// the data this editor edits
        XMultiHierarchicalPropertySet mxModel;

        public static final int CANCELED = 0;
        public static final int SAVE_SETTINGS = 1;

        // sets a model and updates the display
        public void setModel(XMultiHierarchicalPropertySet xModel) {
            mxModel = xModel;
            updateDisplay();
        }

        // this method 'runs' the 'dialog'
        public int execute() {
            try
            {
                System.out.println("-- GridEditor executing --");
                // simulate a user action changing some data
                toggleVisibility();
                System.out.println("-- GridEditor done      --");
                return SAVE_SETTINGS;
            }
            catch (Exception e)
            {
                informUserOfError(e);
                return CANCELED;
            }
        }

        // this method is called to report an error during dialog execution to the user
        public void informUserOfError(Exception e) {
            System.err.println("ERROR in GridEditor:");
            e.printStackTrace();
        }

        /// this method is called to allow the dialog to get feedback about changes occurring elsewhere
        public XChangesListener createChangesListener() {
            if (mxModel == null) return null;

            return (new XChangesListener () {
                public void changesOccurred( ChangesEvent event ) {
                    System.out.println("GridEditor - Listener received changes event containing " +
                                        event.Changes.length + " change(s).");
                    updateDisplay();
                }

                public void disposing(EventObject event) {
                    System.out.println("GridEditor - Listener received disposed event: releasing model");
                    setModel(null);
                }
            });
        }
        /// this method is called when data has changed to display the updated data
        private void updateDisplay()  {
            if (mxModel != null)
                System.out.println("Grid options editor: data=" + readModel());
            else
                System.out.println("Grid options editor: no model set");
        }

        // this method is used to read all relevant data from the model
        private GridOptions readModel()
        {
            try
            {
                String [] aOptionNames = new String [5];
                aOptionNames[0] = "Option/VisibleGrid";
                aOptionNames[1] = "Subdivision/XAxis";
                aOptionNames[2] = "Subdivision/YAxis";
                aOptionNames[3] = "Resolution/XAxis/Metric";
                aOptionNames[4] = "Resolution/YAxis/Metric";

                Object [] aValues = mxModel.getHierarchicalPropertyValues(aOptionNames);

                GridOptions result = new GridOptions();
                result.visible = ((Boolean)aValues[0]).booleanValue();
                result.subdivision_x = ((Integer)aValues[1]).intValue();
                result.subdivision_y = ((Integer)aValues[2]).intValue();
                result.resolution_x  = ((Integer)aValues[3]).intValue();
                result.resolution_y  = ((Integer)aValues[4]).intValue();

                return result;
            }
            catch (Exception e)
            {
                informUserOfError(e);
                return null;
            }
        }

        // this method executes an edit
        private void toggleVisibility()
        {
            try
            {
                XHierarchicalPropertySet xHPS =
                    UnoRuntime.queryInterface(XHierarchicalPropertySet.class, mxModel);

                final String sSetting = "Option/VisibleGrid";

                System.out.println("GridEditor: toggling Visibility");

                Boolean bOldValue = (Boolean)xHPS.getHierarchicalPropertyValue(sSetting);

                Boolean bNewValue = Boolean.valueOf( ! bOldValue.booleanValue() );

                xHPS.setHierarchicalPropertyValue(sSetting,bNewValue);
            }
            catch (Exception e)
            {
                informUserOfError(e);
            }
        }
    }

    /** This method creates an extra updatable view to change some data
        and trigger the listener of the GridEditor
    */
    void changeSomeData(String xKey)
    {
        try
        {
            Object xOtherViewRoot = createUpdatableView(xKey);

            XNameReplace aReplace = UnoRuntime.queryInterface(XNameReplace.class, xOtherViewRoot);

            String aItemNames [] = aReplace.getElementNames();
            for (int i=0; i < aItemNames.length; ++i) {
                Object aItem = aReplace.getByName( aItemNames [i] );
                // replace integers by a 'complement' value
                if ( AnyConverter.isInt(aItem) )
                {
                    int nOld = AnyConverter.toInt(aItem);
                    int nNew = 9999 - nOld;

                    System.out.println("Replacing integer value: " + aItemNames [i]);
                    aReplace.replaceByName( aItemNames [i], Integer.valueOf( nNew ) );
                }

                // and booleans by their negated value
                else if ( AnyConverter.isBoolean(aItem) )
                {
                    boolean bOld = AnyConverter.toBoolean(aItem);
                    boolean bNew = ! bOld;

                    System.out.println("Replacing boolean value: " + aItemNames [i]);
                    aReplace.replaceByName( aItemNames [i], Boolean.valueOf( bNew ) );
                }
            }

            // commit the changes
            XChangesBatch xUpdateControl =
                UnoRuntime.queryInterface(XChangesBatch.class,xOtherViewRoot);

            xUpdateControl.commitChanges();

            // we are done with the view - dispose it
            UnoRuntime.queryInterface(XComponent.class, xOtherViewRoot).dispose();
        }
        catch (Exception e)
        {
            System.err.println("Could not change some data in a different view. An exception occurred:");
            e.printStackTrace();
        }
    }

// GROUP RESET EXAMPLE
    /// This method resets the grid settings to their default values
    protected void resetGridConfiguration()
        throws com.sun.star.uno.Exception
    {
        // The path to the root element
        final String cGridOptionsPath = "/org.openoffice.Office.Calc/Grid";

        // create the view
        Object xViewRoot = createUpdatableView(cGridOptionsPath);

     // resetting a single nested value
        XHierarchicalNameAccess xHierarchicalAccess =
            UnoRuntime.queryInterface(XHierarchicalNameAccess.class, xViewRoot);

        // get using absolute name
        Object xOptions = xHierarchicalAccess.getByHierarchicalName(cGridOptionsPath + "/Option");

        XPropertyState xOptionState =
            UnoRuntime.queryInterface(XPropertyState.class, xOptions);

        xOptionState.setPropertyToDefault("VisibleGrid");

     // resetting more deeply nested values
        Object xResolutionX = xHierarchicalAccess.getByHierarchicalName("Resolution/XAxis");
        Object xResolutionY = xHierarchicalAccess.getByHierarchicalName("Resolution/YAxis");

        XPropertyState xResolutionStateX =
            UnoRuntime.queryInterface(XPropertyState.class, xResolutionX);
        XPropertyState xResolutionStateY =
            UnoRuntime.queryInterface(XPropertyState.class, xResolutionY);

        xResolutionStateX.setPropertyToDefault("Metric");
        xResolutionStateY.setPropertyToDefault("Metric");

     // resetting multiple sibling values
        Object xSubdivision = xHierarchicalAccess.getByHierarchicalName("Subdivision");

        XMultiPropertyStates xSubdivisionStates =
            UnoRuntime.queryInterface(XMultiPropertyStates.class, xSubdivision);

        xSubdivisionStates.setAllPropertiesToDefault();

        // commit the changes
        XChangesBatch xUpdateControl =
            UnoRuntime.queryInterface(XChangesBatch.class,xViewRoot);

        xUpdateControl.commitChanges();

       // we are done with the view - dispose it
        UnoRuntime.queryInterface(XComponent.class, xViewRoot).dispose();
    }


// SET UPDATE EXAMPLE
    private static boolean SET_EXAMPLE_BROKEN_IN_THIS_RELEASE = true;

    /** This method stores a sample data source given some connection data.

        ATTENTION: This example requires an older version of the
                   org.openoffice.Office.DataAccess schema.
                   It does not work with the current schema.
                   Because of this, the method currently does nothing.
                   You can still use the techniques shown in the example code.
    */
    void storeSampleDataSource()
        throws com.sun.star.uno.Exception
    {
        if (SET_EXAMPLE_BROKEN_IN_THIS_RELEASE)
        {
            System.out.println("-  DISABLED: (the existing example does not work with this version) -");
            return; // this function does not work
        }

        String sSampleDataSourceName = "SampleTextDatabase";

        String sSampleDataSourceURL = "sdbc:flat:$(userurl)/database/SampleTextDatabase";

        com.sun.star.beans.NamedValue [] aSettings = new com.sun.star.beans.NamedValue [2];
        aSettings[0] = new com.sun.star.beans.NamedValue("HeaderLine",Boolean.TRUE);
        aSettings[1] = new com.sun.star.beans.NamedValue("FieldDelimiter",";");

        String [] aTableFilter = new String[] { "table.txt", "othertable.txt" };

        storeDataSource(sSampleDataSourceName,sSampleDataSourceURL,"",false,0,aSettings,aTableFilter);
    }

    /// This method stores a data source given some connection data
    void storeDataSource(
        String sDataSourceName,
        String sDataSourceURL,
        String sUser,
        boolean bNeedsPassword,
        int nTimeout,
        com.sun.star.beans.NamedValue [] aDriverSettings,
        String [] aTableFilter
    )
        throws com.sun.star.uno.Exception
    {
      // create the view and get the data source element
        Object xDataSource = createDataSourceDescription(getProvider(),sDataSourceName);

      // set the values
        XPropertySet xDataSourceProperties =
            UnoRuntime.queryInterface(XPropertySet.class, xDataSource);

        xDataSourceProperties.setPropertyValue("URL",  sDataSourceURL  );
        xDataSourceProperties.setPropertyValue("User", sUser  );
        xDataSourceProperties.setPropertyValue("IsPasswordRequired", Boolean.valueOf( bNeedsPassword ) );
        xDataSourceProperties.setPropertyValue("LoginTimeout", Integer.valueOf( nTimeout ) );

        if ( aTableFilter != null )
            xDataSourceProperties.setPropertyValue("TableFilter",  aTableFilter  );

      // store the driver-specific settings
        if (aDriverSettings != null)
        {
            Object xSettingsSet = xDataSourceProperties.getPropertyValue("DataSourceSettings");
            storeSettings( xSettingsSet, aDriverSettings);
        }

      // save the data and dispose the view
        // recover the view root
        Object xViewRoot = getViewRoot(xDataSource);

        // commit the changes
        XChangesBatch xUpdateControl =
            UnoRuntime.queryInterface(XChangesBatch.class,xViewRoot);

        xUpdateControl.commitChanges();

        // now clean up
        UnoRuntime.queryInterface(XComponent.class, xViewRoot).dispose();
    }

    /** This method gets the DataSourceDescription for a data source.
        It either gets the existing entry or creates a new instance.
    */
    Object createDataSourceDescription(XMultiServiceFactory xProvider, String sDataSourceName )
        throws com.sun.star.uno.Exception
    {
        // The service name: Need an update access:
        final String cUpdatableView = "com.sun.star.configuration.ConfigurationUpdateAccess";

         // The path to the DataSources set node
        final String cDataSourcesPath = "/org.openoffice.Office.DataAccess/DataSources";

       // creation arguments: nodepath
        com.sun.star.beans.PropertyValue aPathArgument = new com.sun.star.beans.PropertyValue();
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = cDataSourcesPath ;

        Object[] aArguments = new Object[1];
        aArguments[0] = aPathArgument;

        // create the view
        Object xViewRoot =
            xProvider.createInstanceWithArguments(cUpdatableView, aArguments);

        XNameAccess xSetOfDataSources =
            UnoRuntime.queryInterface(XNameAccess.class,xViewRoot);

        Object xDataSourceDescriptor = null; // the result
        if ( xSetOfDataSources .hasByName( sDataSourceName ))
        {
            // the element is there
            try
            {
                // the view should point to the element directly, so we need to extend the path
                XHierarchicalName xComposePath = UnoRuntime.queryInterface(XHierarchicalName.class, xSetOfDataSources );

                String sElementPath = xComposePath.composeHierarchicalName( sDataSourceName );

                // use the name of the element now
                aPathArgument.Value = sElementPath;

                // create another view now
                Object[] aDeepArguments = new Object[1];
                aDeepArguments[0] = aPathArgument;

                // create the view
                xDataSourceDescriptor  =
                      xProvider.createInstanceWithArguments(cUpdatableView, aDeepArguments);

                if ( xDataSourceDescriptor != null) // all went fine
                {
                    // dispose the other view
                    UnoRuntime.queryInterface(XComponent.class, xViewRoot).dispose();
                    xViewRoot = null;
                }
            }
            catch (Exception e)
            {
              // something went wrong, we retry with a new element
               System.err.println("WARNING: An exception occurred while creating a view for an existing data source: " + e);
               xDataSourceDescriptor  = null;
            }
        }

        // do we have a result element yet ?
        if ( xDataSourceDescriptor == null)
        {
            // get the container
            XNameContainer xSetUpdate =
                UnoRuntime.queryInterface(XNameContainer.class, xViewRoot);

            // create a new detached set element (instance of DataSourceDescription)
            XSingleServiceFactory xElementFactory =
                UnoRuntime.queryInterface(XSingleServiceFactory.class, xSetUpdate);

            // the new element is the result !
             xDataSourceDescriptor  = xElementFactory.createInstance();

            // insert it - this also names the element
            xSetUpdate.insertByName( sDataSourceName ,  xDataSourceDescriptor  );
        }

        return xDataSourceDescriptor ;
    }

    /// this method stores a number of settings in a set node containing DataSourceSetting objects
    void storeSettings(Object xSettingsSet, com.sun.star.beans.NamedValue [] aSettings )
        throws com.sun.star.uno.Exception
    {
        if (aSettings == null)
            return;

        // get the settings set as a container
        XNameContainer xSettingsContainer =
            UnoRuntime.queryInterface( XNameContainer.class, xSettingsSet);

        // and get a factory interface for creating the entries
        XSingleServiceFactory xSettingsFactory =
            UnoRuntime.queryInterface(XSingleServiceFactory.class, xSettingsSet);

        // now insert the individual settings
        for (int i = 0; i < aSettings.length; ++i) {
            // create a DataSourceSetting object
            XPropertySet xSetting = UnoRuntime.queryInterface( XPropertySet.class, xSettingsFactory.createInstance() );

            // can set the value before inserting
            xSetting.setPropertyValue( "Value", aSettings[i].Value );

            // and now insert or replace as appropriate
            if (xSettingsContainer.hasByName( aSettings[i].Name ))
                xSettingsContainer.replaceByName( aSettings[i].Name, xSetting );
            else
                xSettingsContainer.insertByName( aSettings[i].Name, xSetting );
        }
    }

// HELPER FUNCTIONS

    /// This method get the view root node given an interface to any node in the view
    public static Object getViewRoot(Object xElement)
    {
        Object xResult = xElement;

        // set the result to its parent until that would be null
        Object xParent;
        do
        {
            XChild xParentAccess =
                UnoRuntime.queryInterface(XChild.class,xResult);

            if (xParentAccess != null)
                xParent = xParentAccess.getParent();
            else
                xParent = null;

            if (xParent != null)
                xResult = xParent;
         }
         while (xParent != null);

         return xResult;
    }

// workaround methods for unimplemented functionality

    /// WORKAROUND: does the same as xNamedItem.setName(sNewName) should do
    void renameSetItem(XNamed xNamedItem, String sNewName)
        throws com.sun.star.uno.Exception
    {
        XChild xChildItem = UnoRuntime.queryInterface(XChild.class, xNamedItem);

        XNameContainer xParentSet = UnoRuntime.queryInterface( XNameContainer.class, xChildItem.getParent() );

        String sOldName = xNamedItem.getName();

        // now rename the item
        xParentSet.removeByName(sOldName);
        xParentSet.insertByName(sNewName,xNamedItem);
    }

    /// WORKAROUND: does the same as xChildItem.setParent( xNewParent ) should do
    void moveSetItem(XChild xChildItem, XNameContainer xNewParent)
        throws com.sun.star.uno.Exception
    {
        XNamed xNamedItem = UnoRuntime.queryInterface(XNamed.class, xChildItem);

        XNameContainer xOldParent = UnoRuntime.queryInterface( XNameContainer.class, xChildItem.getParent() );

        String sItemName = xNamedItem.getName();

        // now rename the item
        xOldParent.removeByName(sItemName);
        xNewParent.insertByName(sItemName,xChildItem);
    }


// ------- the end -----------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
