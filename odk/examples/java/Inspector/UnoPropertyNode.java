/*************************************************************************
 *
 *  $RCSfile: UnoPropertyNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:02:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.reflection.TypeDescriptionSearchDepth;
import com.sun.star.reflection.XConstantTypeDescription;
import com.sun.star.reflection.XPropertyTypeDescription;
import com.sun.star.reflection.XServiceTypeDescription;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.reflection.XTypeDescriptionEnumeration;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import javax.swing.tree.DefaultMutableTreeNode;

public class UnoPropertyNode extends UnoNode{

    Property aProperty;
    PropertyValue aPropertyValue;
    String m_sPropertyName;
    Object m_oUnoReturnObject;
    public static int nDEFAULT = 0;
    public static int nPROPERTYSETINFOTYPE = 1;
    public static int nPROPERTYVALUETYPE = 2;
    private int m_nPropertyType = nDEFAULT;

    private static XConstantTypeDescription[] xPropertyAttributesTypeDescriptions = null;


    public static UnoPropertyNode getUnoPropertyNodeWithName(Property _aProperty){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aProperty);
        oUnoPropertyNode.setUserObject("Name: " + _aProperty.Name);
        return oUnoPropertyNode;
    }


    public static UnoPropertyNode getUnoPropertyNodeWithHandle(Property _aProperty){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aProperty);
        oUnoPropertyNode.setUserObject("Handle: " + _aProperty.Handle);
        return oUnoPropertyNode;
    }


    public static UnoPropertyNode getUnoPropertyNodeWithType(Property _aProperty){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aProperty);
        oUnoPropertyNode.setUserObject("Type: " + _aProperty.Type.getTypeName());
        return oUnoPropertyNode;
    }


    public static UnoPropertyNode getUnoPropertyNodeWithAttributesDescription(Property _aProperty){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aProperty);
        XConstantTypeDescription[] xPropertyAttributesTypeDescriptions = Introspector.getIntrospector().getFieldsOfConstantGroup("com.sun.star.beans.PropertyAttribute");
        String sDisplay = Introspector.getIntrospector().getConstantDisplayString((int) _aProperty.Attributes, xPropertyAttributesTypeDescriptions, "Attributes: ");
        oUnoPropertyNode.setUserObject(sDisplay);
        return oUnoPropertyNode;
    }


    public static UnoPropertyNode getUnoPropertyNode(Object _oUnoObject, Property _aProperty){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aProperty, _oUnoObject, null);
        oUnoPropertyNode.setPropertyNodeType(nPROPERTYSETINFOTYPE);
        return oUnoPropertyNode;
    }


    public static UnoPropertyNode getUnoPropertyNode(Object _oUnoObject, Property _aProperty, Object _oUnoReturnObject){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aProperty, _oUnoObject, _oUnoReturnObject);
        return oUnoPropertyNode;
    }


    public static UnoPropertyNode getUnoPropertyNode(String _sNodeDescription, Object _oUnoObject, PropertyValue _aPropertyValue, Object _oReturnObject){
        UnoPropertyNode oUnoPropertyNode = new UnoPropertyNode(_aPropertyValue, _oUnoObject, _oReturnObject);
        oUnoPropertyNode.setUserObject(_sNodeDescription);
        return oUnoPropertyNode;
    }


    /** Creates a new instance of UnoMethodNode */
    public UnoPropertyNode(Property _aProperty, Object _oUnoObject, Object _oUnoReturnObject) {
        super(_oUnoObject);
        aProperty = _aProperty;
        m_sPropertyName = aProperty.Name;
        m_oUnoReturnObject = _oUnoReturnObject;
        assignNodeDescription();
    }


    public UnoPropertyNode(Property _aProperty){
        super(null);
        aProperty = _aProperty;
        m_sPropertyName = aProperty.Name;
        m_oUnoReturnObject = null;
    }


    public int getPropertyNodeType(){
        return m_nPropertyType;
    }


    public void setPropertyNodeType(int _nPropertyType){
        m_nPropertyType = _nPropertyType;
    }


    public UnoPropertyNode(PropertyValue _aPropertyValue, Object _oUnoObject, Object _oUnoReturnObject) {
        super(_oUnoObject);
        m_oUnoReturnObject = _oUnoReturnObject;
        aPropertyValue = _aPropertyValue;
        m_sPropertyName = aPropertyValue.Name;
    }


    public String getPropertyName(){
        return m_sPropertyName;
    }


    public String getClassName(){
        String sClassName = "";
        if (m_oUnoObject != null){
            XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, m_oUnoObject);
            if (xServiceInfo != null){
                String[] sServiceNames = xServiceInfo.getSupportedServiceNames();
                for (int i = 0; i < sServiceNames.length; i++){
                    if (doesServiceSupportProperty(sServiceNames[i], m_sPropertyName)){
                        sClassName = sServiceNames[i];
                        break;
                    }
                }
            }
        }
        else{
            sClassName = "com.sun.star.beans.Property";
        }
        if (sClassName.equals("")){
            sClassName = super.getClassName();
        }
        return sClassName;
    }


    public String getAnchor(){
        return m_sPropertyName;
    }


    public String getName(){
        return m_sPropertyName;
    }


    protected boolean doesServiceSupportProperty(String _sServiceName, String _sPropertyName){
    try {
        XPropertyTypeDescription[] xPropertyTypeDescriptions = Introspector.getIntrospector().getPropertyDescriptionsOfService(_sServiceName);
        for (int i = 0; i < xPropertyTypeDescriptions.length; i++){
            if (xPropertyTypeDescriptions[i].getName().equals(_sServiceName + "." + _sPropertyName)){
                return true;
            }
        }
    } catch ( java.lang.Exception e) {
        System.out.println(System.out);
    }
        return false;
    }


    public Object getUnoReturnObject(){
        return m_oUnoReturnObject;
    }


    private boolean isPrimitive(){
        boolean bIsPrimitive = true;
        if (getUnoReturnObject() != null){
            if (getProperty() != null){
                bIsPrimitive = Introspector.isObjectPrimitive(getUnoReturnObject().getClass(), getProperty().Type.getTypeClass());
            }
            else{
                bIsPrimitive = Introspector.isObjectPrimitive(getUnoReturnObject().getClass());
            }
        }
        else{
            bIsPrimitive = Introspector.isObjectPrimitive(aProperty.Type.getTypeClass());
        }
        return bIsPrimitive;
    }


    private void assignNodeDescription(){
        if (! isPrimitive()){
            String sTypeName = getUnoReturnObject().getClass().getName();
            if ( sTypeName.equals("com.sun.star.uno.Type")){
                String sTreeNodeName = getPropertyTypeDescription(aProperty, getUnoReturnObject());
                setUserObject(sTreeNodeName);
            }
            else{
                String sTreeNodeName = getStandardPropertyDescription(aProperty);
                setUserObject(sTreeNodeName);
                addDummyNode();
            }
        }
        else {
            String sTreeNodeName =  getPrimitivePropertyTypeDescription(aProperty, getUnoReturnObject());
            setUserObject(sTreeNodeName);
        }
     }

    public Property getProperty(){
        return aProperty;
    }

    protected  static String getPropertyTypeDescription(Property _aProperty, Object _oUnoObject){
        return _aProperty.Type.getTypeName() + " " + _aProperty.Name + " = " + _oUnoObject.toString();
    }


    protected String getPrimitivePropertyTypeDescription(Property _aProperty, Object _objectElement){
        return _aProperty.Name + " (" + _aProperty.Type.getTypeName() + ") = " + getDisplayValueOfPrimitiveType(_objectElement);
    }


    protected static String getStandardPropertyDescription(Property _aProperty){
        return _aProperty.Name + " = (" + _aProperty.Type.getTypeName() + ") ";
    }


    protected static String getStandardNonPrimitivePropertyValueDescription(PropertyValue _aPropertyValue){
        return _aPropertyValue.Name;
    }


    protected static String getStandardPrimitivePropertyValueDescription(PropertyValue _aPropertyValue){
        return _aPropertyValue.Name + " : " + UnoNode.getDisplayValueOfPrimitiveType(_aPropertyValue.Value);
    }
}


