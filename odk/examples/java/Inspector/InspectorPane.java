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


import org.openoffice.XInstanceInspector;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

    public class InspectorPane extends WeakBase implements XInstanceInspector{  //, XServiceInfo
        private XIdlMethod[] xIdlMethods;
        private Property[] aProperties;
        private boolean bIncludeContent = false;
        private XComponentContext m_xComponentContext;
        private XDialogProvider m_xDialogProvider;
        private Introspector m_oIntrospector = null;
        private SourceCodeGenerator m_oSourceCodeGenerator;
        private XTreeControlProvider m_xTreeControlProvider;
        private String sTitle = "";


        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xMultiServiceFactory XMultiServiceFactory
         */
        public InspectorPane(XComponentContext _xComponentContext, XDialogProvider _xDialogProvider, XTreeControlProvider _xTreeControlProvider, int _nLanguage) {
            m_xComponentContext = _xComponentContext;
            m_xTreeControlProvider = _xTreeControlProvider;
            m_xDialogProvider = _xDialogProvider;
            m_oIntrospector = Introspector.getIntrospector(m_xComponentContext);
            m_oSourceCodeGenerator = new SourceCodeGenerator(_nLanguage);
            _xTreeControlProvider.addInspectorPane(this);
        }


        private void setMaximumOfProgressBar(Object _oUnoObject){
            int nMaxValue = 0;
            xIdlMethods = m_oIntrospector.getMethods(_oUnoObject);
            aProperties = m_oIntrospector.getProperties(_oUnoObject);
        }


        /** Inspect the given object for methods, properties, interfaces, and
         * services.
         * @param a The object to inspect
         * @throws RuntimeException If
         */
        public void inspect(java.lang.Object _oUserDefinedObject, String _sTitle) throws com.sun.star.uno.RuntimeException {
        try {
            Object oContainer = m_xTreeControlProvider.inspect(_oUserDefinedObject, _sTitle);
            m_xDialogProvider.addInspectorPage(getTitle(), oContainer);
        }
        catch( Exception exception ) {
            exception.printStackTrace(System.err);
        }}


        public void setTitle(String _sTitle){
            if (_sTitle != null){
                if (_sTitle.length() > 0){
                    sTitle = _sTitle;
                }
            }
        }


        public String getTitle(){
            return sTitle;
        }



        private Type[] getTypes(Object _oUnoObject){
            Type[] aTypes = null;
            if (AnyConverter.isArray(_oUnoObject)){
                aTypes = (Type[])_oUnoObject;
            }
            else{
                aTypes = m_oIntrospector.getInterfaces(_oUnoObject);
            }
            return aTypes;
        }


        private Object invoke(XUnoMethodNode _oUnoMethodNode){
        try{
            String sParamValueDescription = "";
            Object oUnoReturnObject = _oUnoMethodNode.invoke();
            boolean bHasParameters = _oUnoMethodNode.hasParameters();
            boolean bIsPrimitive = _oUnoMethodNode.isPrimitive();
                if (bHasParameters){
                    sParamValueDescription = " (" + m_oSourceCodeGenerator.getMethodParameterValueDescription(_oUnoMethodNode, _oUnoMethodNode.getLastParameterObjects(), true) + ")";
                }
            if (oUnoReturnObject != null ){
                String sNodeDescription = "";
                XUnoNode oUnoNode = null;
                if (_oUnoMethodNode.getXIdlMethod().getReturnType().getTypeClass().getValue() == TypeClass.VOID_value){
                    sNodeDescription += _oUnoMethodNode.getXIdlMethod().getReturnType().getName() + " " + _oUnoMethodNode.getXIdlMethod().getName() + sParamValueDescription;
                    _oUnoMethodNode.setLabel(sNodeDescription);
                    m_xTreeControlProvider.nodeChanged(_oUnoMethodNode);
                }
                else if (bHasParameters || !bIsPrimitive){
                    if (bIsPrimitive){
                        sNodeDescription += m_oSourceCodeGenerator.getStringValueOfObject(oUnoReturnObject, _oUnoMethodNode.getTypeClass());
                    }
                    else{
                        Any aReturnObject = Any.complete(oUnoReturnObject);
                        String sShortClassName = Introspector.getShortClassName(aReturnObject.getType().getTypeName());
                        sNodeDescription += m_oSourceCodeGenerator.getVariableNameforUnoObject(sShortClassName);
                    }
                    if (Introspector.isArray(oUnoReturnObject)){
                        if (Introspector.isUnoTypeObject(oUnoReturnObject)){
                            oUnoNode = addUnoFacetteNode(_oUnoMethodNode, XUnoFacetteNode.SINTERFACEDESCRIPTION, _oUnoMethodNode.getUnoObject());
                        }
                        else if(Introspector.isUnoPropertyTypeObject(oUnoReturnObject)){
                            oUnoNode = addUnoFacetteNode(_oUnoMethodNode, XUnoFacetteNode.SPROPERTYINFODESCRIPTION, oUnoReturnObject);
                        }
                        else if(Introspector.isUnoPropertyValueTypeObject(oUnoReturnObject)){
                            oUnoNode = addUnoFacetteNode(_oUnoMethodNode, XUnoFacetteNode.SPROPERTYVALUEDESCRIPTION, oUnoReturnObject);
                        }
                    }
                    if (oUnoNode == null){
                        if (bHasParameters){
                            sNodeDescription += sParamValueDescription;
                        }
                        oUnoNode = addUnoNode(null, oUnoReturnObject, sNodeDescription);
                        if (bHasParameters){
                            oUnoNode.setParameterObjects(_oUnoMethodNode.getLastParameterObjects());
                        }
                    }
                    if (oUnoNode != null){
                        oUnoNode.setFoldable(!bIsPrimitive);
                        _oUnoMethodNode.setFoldable(false);
                        _oUnoMethodNode.addChildNode(oUnoNode);
                        m_xTreeControlProvider.nodeInserted(_oUnoMethodNode, oUnoNode, _oUnoMethodNode.getChildCount()-1);
                    }
                }
                else{
                    if (bIsPrimitive){
                        sNodeDescription  = _oUnoMethodNode.getStandardMethodDescription() + "=" + UnoMethodNode.getDisplayValueOfPrimitiveType(oUnoReturnObject);
                        _oUnoMethodNode.setLabel(sNodeDescription);
                        m_xTreeControlProvider.nodeChanged(_oUnoMethodNode);
                    }
                }
            }
            // ReturnObject of method node == null..
            else{
                if (!bHasParameters){
                    _oUnoMethodNode.setLabel(_oUnoMethodNode.getLabel() + " = null");
                }
                else{
                    _oUnoMethodNode.setLabel(_oUnoMethodNode.getXIdlMethod().getName() + sParamValueDescription + " = null");
                }
                m_xTreeControlProvider.nodeChanged(_oUnoMethodNode);
            }
            return oUnoReturnObject;
        }catch(Exception exception ) {
            return null;
        }}


        public void invokeSelectedMethodNode(){
            XTreePathProvider xTreePathProvider = m_xTreeControlProvider.getSelectedPath();
            XUnoNode oUnoNode = xTreePathProvider.getLastPathComponent();
            if (oUnoNode instanceof XUnoMethodNode){
                invoke((XUnoMethodNode) oUnoNode);
                m_xTreeControlProvider.expandPath(xTreePathProvider);
            }
        }


        public void addSourceCodeOfSelectedNode(){
            XTreePathProvider oTreePathProvider = m_xTreeControlProvider.getSelectedPath();
            XUnoNode oUnoNode = oTreePathProvider.getLastPathComponent();
            if (oUnoNode instanceof XUnoMethodNode){
                XUnoMethodNode oUnoMethodNode = (XUnoMethodNode) oUnoNode;
                if (!oUnoMethodNode.isInvoked() && oUnoMethodNode.isInvokable()){
                    invoke(oUnoMethodNode);
                }
            }
            String sSourceCode = m_oSourceCodeGenerator.addSourceCodeOfUnoObject(oTreePathProvider, true, true, true);
            m_xTreeControlProvider.setSourceCode(sSourceCode);
        }


        public void convertCompleteSourceCode(int _nLanguage){
            String sSourceCode = m_oSourceCodeGenerator.convertAllUnoObjects(_nLanguage);
            m_xTreeControlProvider.setSourceCode(sSourceCode);
        }

        protected XUnoNode getSelectedNode(){
            return m_xTreeControlProvider.getSelectedNode();
        }


        //  add the object to the hashtable for a possible access in the tree
        private  XUnoFacetteNode addUnoFacetteNode(XUnoNode _oParentNode, String _sNodeDescription, Object _oUnoObject){
            return m_xTreeControlProvider.addUnoFacetteNode( _oParentNode, _sNodeDescription, _oUnoObject);
        }


        private XUnoMethodNode addMethodNode(Object _objectElement, XIdlMethod _xIdlMethod){
            return m_xTreeControlProvider.addMethodNode(_objectElement, _xIdlMethod);
        }



        public void addMethodsToTreeNode(XUnoNode _oGrandParentNode, Object _oUnoParentObject, XIdlMethod[] _xIdlMethods){
            if (Introspector.isValid(_xIdlMethods)){
                for ( int n = 0; n < _xIdlMethods.length; n++ ) {
                    XIdlMethod xIdlMethod = _xIdlMethods[n];
                    if (!xIdlMethod.getDeclaringClass().getName().equals("com.sun.star.uno.XInterface")){
                        XUnoMethodNode oChildNode = addMethodNode(_oUnoParentObject, xIdlMethod);
                        if (oChildNode != null){
                            _oGrandParentNode.addChildNode(oChildNode);
                        }
                    }
                }
            }
        }


        private void addFacetteNodesToTreeNode(XUnoNode _oParentNode, Object _oUnoObject){
            if (m_oIntrospector.hasMethods(_oUnoObject)){
                XUnoFacetteNode oUnoFacetteNode = addUnoFacetteNode(_oParentNode, XUnoFacetteNode.SMETHODDESCRIPTION, _oUnoObject);
            }
            if (m_oIntrospector.hasProperties(_oUnoObject)){
                XUnoFacetteNode oUnoFacetteNode = addUnoFacetteNode(_oParentNode, XUnoFacetteNode.SPROPERTYDESCRIPTION, _oUnoObject);
            }
            if (m_oIntrospector.hasInterfaces(_oUnoObject)){
                XUnoFacetteNode oUnoFacetteNode = addUnoFacetteNode(_oParentNode, XUnoFacetteNode.SINTERFACEDESCRIPTION, _oUnoObject);
            }
            if (m_oIntrospector.isContainer(_oUnoObject)){
                XUnoFacetteNode oUnoFacetteNode = addUnoFacetteNode(_oParentNode, XUnoFacetteNode.SCONTAINERDESCRIPTION, _oUnoObject);
            }
            if (m_oIntrospector.hasSupportedServices(_oUnoObject)){
                XUnoFacetteNode oUnoFacetteNode = addUnoFacetteNode(_oParentNode, XUnoFacetteNode.SSERVICEDESCRIPTION, _oUnoObject);
            }
        }


        private void addInterfacesToTreeNode(XUnoNode _oGrandParentNode, Object _oUnoParentObject, Type[] _aTypes) {
        try {
            if (_oUnoParentObject != null){
                for ( int m = 0; m < _aTypes.length; m++ ) {
                    addUnoNode(_oGrandParentNode, _oUnoParentObject, _aTypes[m]);
                }
            }
        }
        catch( Exception exception ) {
            exception.printStackTrace(System.err);
        }}


        //  add all services for the given object to the tree under the node parent
        private void addServicesToTreeNode(XUnoNode _oGrandParentNode, Object _oUnoObject) {
        try{
            XServiceInfo xServiceInfo = UnoRuntime.queryInterface( XServiceInfo.class, _oUnoObject );
            if ( xServiceInfo != null ){
                String[] sSupportedServiceNames = xServiceInfo.getSupportedServiceNames();
                for ( int m = 0; m < sSupportedServiceNames.length; m++ ) {
                    String sServiceName = sSupportedServiceNames[m];
                    if (sServiceName.length() > 0){
                        XUnoNode oUnoNode = addUnoNode(_oGrandParentNode, _oUnoObject, sSupportedServiceNames[m]);
                        oUnoNode.setNodeType(XUnoNode.nSERVICE);
                    }
                }
            }
        }
        catch(Exception exception) {
            exception.printStackTrace(System.err);
        }}


        private void addPropertiesAndInterfacesOfService(XUnoNode _oUnoServiceNode){
            String sServiceName  = _oUnoServiceNode.getClassName();
            Object oUnoObject = _oUnoServiceNode.getUnoObject();
            addInterfacesToTreeNode(_oUnoServiceNode, oUnoObject, m_oIntrospector.getInterfaces(oUnoObject, sServiceName));
            addPropertiesToTreeNode(_oUnoServiceNode, oUnoObject, m_oIntrospector.getProperties(oUnoObject, sServiceName));
        }


    private void addPropertiesToTreeNode(XUnoNode _oParentNode, Object _oUnoParentObject,  Property[] _aProperties) {
    try {
        if (_aProperties.length > 0){
            for ( int n = 0; n < _aProperties.length; n++ ){
                Property aProperty = _aProperties[n];
                XIntrospectionAccess xIntrospectionAccess = m_oIntrospector.getXIntrospectionAccess(_oUnoParentObject);
                XPropertySet xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, xIntrospectionAccess.queryAdapter(new Type( XPropertySet.class)));
                if (xPropertySet != null) {
                    if (xPropertySet.getPropertySetInfo().hasPropertyByName(aProperty.Name)){
                        Object objectElement = xPropertySet.getPropertyValue(aProperty.Name);
                        if (objectElement != null) {
                            XUnoNode oChildNode = m_xTreeControlProvider.addUnoPropertyNode(_oUnoParentObject, aProperty, objectElement);
                            if (oChildNode != null){
                                _oParentNode.addChildNode(oChildNode);
                            }
                        }
                    }
                }
            }
        }
    }
    catch(Exception exception) {
        exception.printStackTrace(System.err);
    }}


    public void addContainerElementsToTreeNode(XUnoNode _oParentNode, Object _oUnoParentObject){
        Object[] oUnoContainerElements = m_oIntrospector.getUnoObjectsOfContainer(_oUnoParentObject);
        if (Introspector.isValid(oUnoContainerElements)){
            if (oUnoContainerElements.length > 0){
                for (int i=0; i< oUnoContainerElements.length; i++){
                    XUnoNode oChildNode = addUnoNode(_oParentNode, oUnoContainerElements[i], UnoNode.getNodeDescription(oUnoContainerElements[i]));
                    m_xTreeControlProvider.nodeInserted(_oParentNode, oChildNode, _oParentNode.getChildCount()-1);
                }
            }
        }
    }


    private XUnoNode addUnoNode(XUnoNode _oParentNode, Object _oUnoObject, String _sLabel ){
        XUnoNode oUnoNode = this.m_xTreeControlProvider.addUnoNode(_oUnoObject);
        oUnoNode.setLabel(_sLabel);
        if (_oParentNode != null){
            _oParentNode.addChildNode(oUnoNode);
        }
        setNodeFoldable(oUnoNode, _oUnoObject);
        return oUnoNode;
    }


    private void setNodeFoldable(XUnoNode _oUnoNode, Object _oUnoObject){
        if (_oUnoObject != null){
            if (!Introspector.isObjectPrimitive(_oUnoObject)){
                _oUnoNode.setFoldable(true);
            }
        }
    }


    private XUnoNode addUnoNode(XUnoNode _oParentNode, Object _oUnoObject, Type _aType){
        XUnoNode oUnoNode = this.m_xTreeControlProvider.addUnoNode(_oUnoObject, _aType);
        if (_oParentNode != null){
            _oParentNode.addChildNode(oUnoNode);
        }
        setNodeFoldable(oUnoNode, _oUnoObject);
        return oUnoNode;
    }


    private void addPropertySetInfoNodesToTreeNode(XUnoFacetteNode _oUnoFacetteNode, Object _oUnoObject){
        if (_oUnoObject.getClass().isArray()){
            Object[] object = ( Object[] ) _oUnoObject;
            for ( int i = 0; i < object.length; i++ ) {
                Property aProperty = (Property) object[i];
                XUnoPropertyNode oUnoPropertyNode = m_xTreeControlProvider.addUnoPropertyNode(_oUnoObject, aProperty);
                oUnoPropertyNode.setLabel(XUnoFacetteNode.SPROPERTYINFODESCRIPTION + " (" + aProperty.Name + ")");
                oUnoPropertyNode.setFoldable(true);
                _oUnoFacetteNode.addChildNode(oUnoPropertyNode);
            }
        }
    }

    private void addPrimitiveValuesToTreeNode(XUnoNode _oUnoNode, Object _oUnoObject){
        if (_oUnoObject.getClass().isArray()){
            if (!_oUnoObject.getClass().getComponentType().isPrimitive()){
                Object[] object = ( Object[] ) _oUnoObject;
                for ( int i = 0; i < object.length; i++ ) {
                    if (Introspector.isObjectPrimitive(object[i])){
                        XUnoNode oChildNode = addUnoNode(_oUnoNode, null, UnoNode.getNodeDescription(object[i]));
                    }
                }
            }
            else{
                String[] sDisplayValues = UnoNode.getDisplayValuesofPrimitiveArray(_oUnoObject);
                for ( int i = 0; i < sDisplayValues.length; i++ ) {
                    XUnoNode oUnoNode = addUnoNode(_oUnoNode, null, sDisplayValues[i]);
                }
            }
        }
    }


    private void addPropertySetInfoSubNodes(XUnoPropertyNode _oUnoPropertyNode){
        Property aProperty = _oUnoPropertyNode.getProperty();
        _oUnoPropertyNode.addChildNode(m_xTreeControlProvider.addUnoPropertyNodeWithName(aProperty));
        _oUnoPropertyNode.addChildNode(m_xTreeControlProvider.addUnoPropertyNodeWithType(aProperty));
        _oUnoPropertyNode.addChildNode(m_xTreeControlProvider.addUnoPropertyNodeWithHandle(aProperty));
        _oUnoPropertyNode.addChildNode(m_xTreeControlProvider.addUnoPropertyNodeWithAttributesDescription(aProperty));
    }


    private void addPropertyValueSubNodes(XUnoFacetteNode _oUnoFacetteNode, Object _oUnoObject){
        if (Introspector.isUnoPropertyValueTypeObject(_oUnoObject)){
            Object[] object = ( Object[] ) _oUnoObject;
            for ( int i = 0; i < object.length; i++ ) {
                String sObjectClassName = object[i].getClass().getName();
                if (sObjectClassName.equals("com.sun.star.beans.PropertyValue")){
                    XUnoNode oChildNode = null;
                    PropertyValue aPropertyValue = (PropertyValue) object[i];
                    if (! Introspector.isObjectPrimitive(aPropertyValue.Value)){
                        oChildNode = m_xTreeControlProvider.addUnoPropertyNode(_oUnoObject, aPropertyValue, _oUnoObject);
                    }
                    else{
                        oChildNode = addUnoNode(null, aPropertyValue.Value, UnoPropertyNode.getStandardPropertyValueDescription(aPropertyValue));
                    }
                    if (oChildNode != null){
                        _oUnoFacetteNode.addChildNode(oChildNode);
                    }
                }
            }
        }
    }


    public boolean expandNode(XUnoNode _oUnoNode, XTreePathProvider _xTreePathProvider){
        if ( _oUnoNode != null ) {
            _oUnoNode.setFoldable(false);
            if (m_xTreeControlProvider.isMethodNode(_oUnoNode)){
                XUnoMethodNode oUnoMethodNode = (XUnoMethodNode) _oUnoNode;
                if (!oUnoMethodNode.isInvoked()){
                    Object oUnoReturnObject = invoke((XUnoMethodNode) _oUnoNode);
                    if (oUnoReturnObject == null){
                        _oUnoNode.setFoldable(true);
                        return false;
                    }
                }
            }
            else if (m_xTreeControlProvider.isPropertyNode(_oUnoNode)){
                XUnoPropertyNode oUnoPropertyNode = (XUnoPropertyNode) _oUnoNode;
                Object oUnoObject = oUnoPropertyNode.getUnoReturnObject();
                if (oUnoPropertyNode.getPropertyNodeType() == XUnoPropertyNode.nPROPERTYSETINFOTYPE){
                    addPropertySetInfoSubNodes(oUnoPropertyNode);
                }
                else{
                    //TODO this code is redundant!!!
                    if ( oUnoObject.getClass().isArray()){
                        // TODO probably we should provid a possibility to insert also non-primitive nodes
                        addPrimitiveValuesToTreeNode(oUnoPropertyNode, oUnoObject);
                    }
                    else{
                        addFacetteNodesToTreeNode(oUnoPropertyNode, oUnoObject);
                    }
                }
            }
            else if (m_xTreeControlProvider.isFacetteNode(_oUnoNode)){
                XUnoFacetteNode oUnoFacetteNode = (XUnoFacetteNode) _oUnoNode;
                Object oUnoObject = _oUnoNode.getUnoObject();
                if (oUnoFacetteNode.isMethodNode()){
                    addMethodsToTreeNode(oUnoFacetteNode, oUnoObject, m_oIntrospector.getMethods(oUnoObject));
                }
                if (oUnoFacetteNode.isPropertyNode()){
                    String sNodeDescription = oUnoFacetteNode.getLabel();
                    // TODO: it's very dangerous to distinguish the different UnoFacetteNodes only by the node description
                    if (sNodeDescription.startsWith(XUnoFacetteNode.SPROPERTYINFODESCRIPTION)){
                        addPropertySetInfoNodesToTreeNode(oUnoFacetteNode, oUnoObject);
                    }
                    else if (sNodeDescription.startsWith(XUnoFacetteNode.SPROPERTYVALUEDESCRIPTION)){
                        addPropertyValueSubNodes(oUnoFacetteNode, oUnoObject);
                    }
                    else{
                        addPropertiesToTreeNode(oUnoFacetteNode, oUnoObject, m_oIntrospector.getProperties(oUnoObject));
                    }
                }
                if (oUnoFacetteNode.isServiceNode()){
                    addServicesToTreeNode(oUnoFacetteNode, oUnoObject);
                }
                if (oUnoFacetteNode.isInterfaceNode()){
                    addInterfacesToTreeNode(oUnoFacetteNode, oUnoObject, getTypes(oUnoObject));
                }
                if (oUnoFacetteNode.isContainerNode()){
                    addContainerElementsToTreeNode(oUnoFacetteNode, oUnoObject);
                }
            }
            else if (_oUnoNode.getNodeType() == XUnoNode.nSERVICE){
                addPropertiesAndInterfacesOfService(_oUnoNode);
            }
            else if (_oUnoNode.getNodeType() == XUnoNode.nINTERFACE){
                Type aType = _oUnoNode.getUnoType();
                if (aType != null){
                    addMethodsToTreeNode(_oUnoNode, _oUnoNode.getUnoObject(), m_oIntrospector.getMethodsOfInterface(aType));
                }
            }
            else if (_oUnoNode.getNodeType() == XUnoNode.nOTHERS){
                Object oUnoObject = _oUnoNode.getUnoObject();
                if ( oUnoObject.getClass().isArray()){
                    // TODO probably we should provid a possibility to insert also non-primitive nodes
                     addPrimitiveValuesToTreeNode(_oUnoNode, oUnoObject);
                }
                else{
                    addFacetteNodesToTreeNode(_oUnoNode, oUnoObject);
                }
            }
        }
        return true;
    }



        public void applyFilter(XUnoFacetteNode _oUnoFacetteNode, String _sFilter){
            for (int i = 0; i < _oUnoFacetteNode.getChildCount(); i++){
                XUnoNode oUnoNode = _oUnoFacetteNode.getChild(i);
                boolean bIsVisible = oUnoNode.isFilterApplicable(_sFilter);
                m_xTreeControlProvider.setNodeVisible(oUnoNode, bIsVisible);
            }
            _oUnoFacetteNode.setFilter(_sFilter);
        }


        /** In opposition to 'getUnoObjectOfTreeNode' this method inspects the passed node if it represents a Uno object
         *  If not it returns null
         *
         */
        private Object getUnoObjectOfExplicitTreeNode(XTreePathProvider _xTreePathProvider){
            XUnoNode oUnoNode = _xTreePathProvider.getLastPathComponent();
            if (oUnoNode != null){
                return oUnoNode.getUnoObject();
            }
            return null;
        }


        public String getFilter(XUnoNode _oUnoNode){
            String sFilter = "";
            if (_oUnoNode != null){
                XUnoNode oUnoNode = _oUnoNode;
                boolean bleaveLoop = false;
                do{
                    if (m_xTreeControlProvider.isFacetteNode(oUnoNode)){
                        sFilter = ((XUnoFacetteNode) oUnoNode).getFilter();
                        bleaveLoop = true;
                    }
                    else{
                        if (oUnoNode.getParentNode() != null){
                            oUnoNode = oUnoNode.getParentNode();
                        }
                        else{
                            bleaveLoop = true;
                        }
                    }
                } while(!bleaveLoop);
            }
            return sFilter;
        }


        /** In opposition to 'getUnoObjectOfexplictTreeNode' this method inspects the passed node if it represents a Uno object
         *  if not it keeps checking all ancestors until it finds a Uno representation
         */
        private Object getUnoObjectOfTreeNode(XTreePathProvider _xTreePathProvider){
            XTreePathProvider xTreePathProvider = _xTreePathProvider;
            HideableMutableTreeNode oNode = null;
            Object oUnoObject = null;
            while (xTreePathProvider != null){
                oUnoObject = getUnoObjectOfExplicitTreeNode(xTreePathProvider);
                if (oUnoObject != null){
                    if (oUnoObject instanceof String){
                    }
                    else{
                        if (!Introspector.isUnoTypeObject(oUnoObject)){
                            return oUnoObject;
                        }
                    }
                }
                xTreePathProvider = xTreePathProvider.getParentPath();
            }
            return null;
        }


        public void showPopUpMenu(Object _invoker, int x, int y) throws ClassCastException{
            XUnoNode oUnoNode = getSelectedNode();
            boolean bdoEnableInvoke = oUnoNode instanceof XUnoMethodNode;
//            boolean bdoEnableSourceCodeGeneration = true;
            if (bdoEnableInvoke){
                XUnoMethodNode oUnoMethodNode = (XUnoMethodNode) oUnoNode;
                bdoEnableInvoke = oUnoMethodNode.isInvokable();
//                bdoEnableSourceCodeGeneration = bdoEnableInvoke;
            }
            m_xDialogProvider.enablePopupMenuItem(XDialogProvider.SINVOKE, bdoEnableInvoke);
//            m_xDialogProvider.enablePopupMenuItem(XDialogProvider.SADDTOSOURCECODE, bdoEnableSourceCodeGeneration);
            m_xDialogProvider.showPopUpMenu(_invoker, x, y);
        }
}
