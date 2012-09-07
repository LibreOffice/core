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
import com.sun.star.awt.PushButtonType;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XListBox;
import com.sun.star.beans.MethodConcept;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XIntrospection;
import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class UnoDialogSample2 extends UnoDialogSample {
    XIntrospectionAccess m_xIntrospectionAccess = null;
    Object m_oUnoObject = null;
    // define some constants used to set positions and sizes
    // of controls. For further information see
    // http://ui.openoffice.org/knowledge/DialogSpecificationandGuidelines.odt
    final static int nFixedTextHeight = 8;
    final static int nControlMargin = 6;
    final static int nDialogWidth = 250;
    final static int nDialogHeight = 140;
    // the default roadmap width == 80 MAPs
    final static int nRoadmapWidth = 80;
    final static int nButtonHeight = 14;
    final static int nButtonWidth = 50;


    public UnoDialogSample2(XComponentContext _xContext, XMultiComponentFactory _xMCF, Object _oUnoObject) {
        super(_xContext, _xMCF);
        try {
            m_oUnoObject = _oUnoObject;
            Object o = m_xMCF.createInstanceWithContext("com.sun.star.beans.Introspection", m_xContext);
            XIntrospection m_xIntrospection = ( XIntrospection ) UnoRuntime.queryInterface(XIntrospection.class, o );
            // the variable m_xIntrospectionAccess offers functionality to access all methods and properties
            // of a variable
            m_xIntrospectionAccess = m_xIntrospection.inspect(_oUnoObject);
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
    }

    public static void main(String args[]) {
        UnoDialogSample2 oUnoDialogSample2 = null;
        try {
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null )
                System.out.println("Connected to a running office ...");
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            PropertyValue[] aPropertyValues = new PropertyValue[]{};
            // create an arbitrary Uno-Object - in this case an empty writer document..
            Object oDesktop =xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", xContext);
            XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, oDesktop);
            Object oUnoObject = xComponentLoader.loadComponentFromURL("private:factory/swriter", "_default", 0, aPropertyValues);

            // define some coordinates where to position the controls
            final int nButtonPosX = (int)((nDialogWidth/2) - (nButtonWidth/2));
            final int nButtonPosY = nDialogHeight - nButtonHeight - nControlMargin;
            final int nControlPosX = nRoadmapWidth + 2*nControlMargin;
            final int nControlWidth = nDialogWidth - 3*nControlMargin - nRoadmapWidth;
            final int nListBoxHeight = nDialogHeight - 4*nControlMargin - nButtonHeight;
            oUnoDialogSample2 = new UnoDialogSample2(xContext, xMCF, oUnoObject);
            oUnoDialogSample2.initialize( new String[] {"Height", "Moveable", "Name","PositionX","PositionY", "Step", "TabIndex","Title","Width"},
                                          new Object[] { new Integer(nDialogHeight), Boolean.TRUE, "Dialog1", new Integer(102),new Integer(41), new Integer(1), new Short((short) 0), "Inspect a Uno-Object", new Integer(nDialogWidth)});
            String sIntroLabel = "This Dialog lists information about a given Uno-Object.\nIt offers a view to inspect all suppported servicenames, exported interfaces, methods and properties.";
            oUnoDialogSample2.insertMultiLineFixedText(nControlPosX, 27, nControlWidth, 4, 1, sIntroLabel);
            // get the data from the UNO object...
            String[] sSupportedServiceNames = oUnoDialogSample2.getSupportedServiceNames();
            String[] sInterfaceNames = oUnoDialogSample2.getExportedInterfaceNames();
            String[] sMethodNames = oUnoDialogSample2.getMethodNames();
            String[] sPropertyNames = oUnoDialogSample2.getPropertyNames();
            // add controls to the dialog...
            oUnoDialogSample2.insertListBox(nControlPosX, nControlMargin, nListBoxHeight, nControlWidth, 2, sSupportedServiceNames);
            oUnoDialogSample2.insertListBox(nControlPosX, nControlMargin, nListBoxHeight, nControlWidth, 3, sInterfaceNames);
            oUnoDialogSample2.insertListBox(nControlPosX, nControlMargin, nListBoxHeight, nControlWidth, 4, sMethodNames);
            oUnoDialogSample2.insertListBox(nControlPosX, nControlMargin, nListBoxHeight, nControlWidth, 5, sPropertyNames);
            oUnoDialogSample2.insertButton(oUnoDialogSample2, nButtonPosX, nButtonPosY, nButtonWidth, "~Close", (short) PushButtonType.OK_value);
            oUnoDialogSample2.insertHorizontalFixedLine(0, nButtonPosY - nControlMargin - 4, nDialogWidth, "");
            // create the windowpeer;
            // it must be kept in mind that it must be created after the insertion of the controls
            // (see http://qa.openoffice.org/issues/show_bug.cgi?id=75129)
            oUnoDialogSample2.createWindowPeer();
            // add the roadmap control. Note that the roadmap may not be created before the windowpeer of the dialog exists
            // (see http://qa.openoffice.org/issues/show_bug.cgi?id=67369)
            oUnoDialogSample2.addRoadmap(oUnoDialogSample2.getRoadmapItemStateChangeListener());
            oUnoDialogSample2.insertRoadmapItem(0, true, "Introduction", 1);
            oUnoDialogSample2.insertRoadmapItem(1, true, "Supported Services", 2);
            oUnoDialogSample2.insertRoadmapItem(2, true, "Interfaces", 3);
            oUnoDialogSample2.insertRoadmapItem(3, true, "Methods", 4);
            oUnoDialogSample2.insertRoadmapItem(4, true, "Properties", 5);
            oUnoDialogSample2.m_xRMPSet.setPropertyValue("CurrentItemID", new Short((short) 1));
            oUnoDialogSample2.m_xRMPSet.setPropertyValue("Complete", Boolean.TRUE);
            oUnoDialogSample2.xDialog = (XDialog) UnoRuntime.queryInterface(XDialog.class, oUnoDialogSample2.m_xDialogControl);
            oUnoDialogSample2.xDialog.execute();
        }catch( Exception ex ) {
            ex.printStackTrace(System.err);
        }
        finally{
            //make sure always to dispose the component and free the memory!
            if (oUnoDialogSample2 != null){
                if (oUnoDialogSample2.m_xComponent != null){
                    oUnoDialogSample2.m_xComponent.dispose();
                }
            }
        }

        System.exit( 0 );
    }


    public String[] getMethodNames() {
        String[] sMethodNames = new String[]{};
        try {
            XIdlMethod[] xIdlMethods = m_xIntrospectionAccess.getMethods(MethodConcept.ALL);
            sMethodNames = new String[xIdlMethods.length];
            for (int i = 0; i < xIdlMethods.length; i++){
                sMethodNames[i] = xIdlMethods[i].getName();
            }
        }
        catch( Exception e ) {
            System.err.println( e );
        }
        return sMethodNames;
    }

    // returns the names of all supported servicenames of a UNO object
    public String[] getSupportedServiceNames() {
        String[] sSupportedServiceNames = new String[]{};
        // If the Uno-Object supports "com.sun.star.lang.XServiceInfo"
        // this will give access to all supported servicenames
        XServiceInfo xServiceInfo = ( XServiceInfo ) UnoRuntime.queryInterface( XServiceInfo.class, m_oUnoObject);
        if ( xServiceInfo != null ) {
            sSupportedServiceNames = xServiceInfo.getSupportedServiceNames();
        }
        return sSupportedServiceNames;
    }

    // returns the names of all properties of a UNO object
    protected String[] getPropertyNames() {
        String[] sPropertyNames = new String[]{};
        try {
            Property[] aProperties = m_xIntrospectionAccess.getProperties(com.sun.star.beans.PropertyConcept.ATTRIBUTES + com.sun.star.beans.PropertyConcept.PROPERTYSET);
            sPropertyNames = new String[aProperties.length];
            for (int i = 0; i < aProperties.length; i++){
                sPropertyNames[i] = aProperties[i].Name;
            }
        }
        catch( Exception e ) {
            System.err.println( e );
        }
        return sPropertyNames;
    }


    // returns the names of all exported interfaces of a UNO object
    protected String[] getExportedInterfaceNames(){
        Type[] aTypes = new Type[]{};
        String[] sTypeNames = new String[]{};
        // The XTypeProvider interfaces offers access to all exported interfaces
        XTypeProvider xTypeProvider = ( XTypeProvider ) UnoRuntime.queryInterface( XTypeProvider.class, m_oUnoObject);
        if ( xTypeProvider != null ) {
            aTypes = xTypeProvider.getTypes();
            sTypeNames = new String[aTypes.length];
            for (int i = 0; i < aTypes.length - 1; i++){
                sTypeNames[i] = aTypes[i].getTypeName();
            }
        }
        return sTypeNames;
    }



    public XListBox insertListBox(int _nPosX, int _nPosY, int _nHeight, int _nWidth, int _nStep, String[] _sStringItemList) {
        XListBox xListBox = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ListBox");
            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oListBoxModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlListBoxModel");
            XMultiPropertySet xLBModelMPSet = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oListBoxModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xLBModelMPSet.setPropertyValues(
                new String[]  {"Dropdown", "Height", "Name", "PositionX", "PositionY", "ReadOnly", "Step", "StringItemList", "Width" } ,
                new Object[] {Boolean.FALSE, new Integer(_nHeight), sName, new Integer(_nPosX), new Integer(_nPosY), Boolean.TRUE, new Integer(_nStep), _sStringItemList, new Integer(_nWidth)});
            m_xDlgModelNameContainer.insertByName(sName, xLBModelMPSet);
        }catch (com.sun.star.uno.Exception ex) {
            throw new java.lang.RuntimeException("cannot happen...");
        }
        return xListBox;
    }


    public void insertMultiLineFixedText(int _nPosX, int _nPosY, int _nWidth, int _nLineCount, int _nStep, String _sLabel) {
        try {
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "Label");
            int nHeight = _nLineCount * nFixedTextHeight;
            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oFTModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTModelMPSet = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oFTModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xFTModelMPSet.setPropertyValues(
                new String[] {"Height", "Label", "MultiLine", "Name", "PositionX", "PositionY", "Step", "Width"},
                new Object[] { new Integer(nHeight), _sLabel, Boolean.TRUE, sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nStep), new Integer(_nWidth)});
            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oFTModel);
        }catch (com.sun.star.uno.Exception ex){
            /* perform individual exception handling here.
             * Possible exception types are:
             * com.sun.star.lang.IllegalArgumentException,
             * com.sun.star.lang.WrappedTargetException,
             * com.sun.star.container.ElementExistException,
             * com.sun.star.beans.PropertyVetoException,
             * com.sun.star.beans.UnknownPropertyException,
             * com.sun.star.uno.Exception
             */
            ex.printStackTrace(System.err);
        }
    }

}// end of class
