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

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.reflection.XConstantTypeDescription;
import com.sun.star.reflection.XPropertyTypeDescription;
import com.sun.star.uno.UnoRuntime;

public class UnoPropertyNode extends UnoNode{

    Property aProperty;
    PropertyValue aPropertyValue;
    String m_sPropertyName;
    Object m_oUnoReturnObject;
    private int m_nPropertyType = XUnoPropertyNode.nDEFAULT;
    private String sLabel = "";

    private static XConstantTypeDescription[] xPropertyAttributesTypeDescriptions = null;


    /** Creates a new instance of UnoMethodNode */
    public UnoPropertyNode(Property _aProperty, Object _oUnoObject, Object _oUnoReturnObject) {
        super(_oUnoObject);
        aProperty = _aProperty;
        m_sPropertyName = aProperty.Name;
        m_oUnoReturnObject = _oUnoReturnObject;
    }


    public UnoPropertyNode(Property _aProperty){
        super(null);
        aProperty = _aProperty;
        m_sPropertyName = aProperty.Name;
        m_oUnoReturnObject = null;
    }

    public UnoPropertyNode(PropertyValue _aPropertyValue, Object _oUnoObject, Object _oUnoReturnObject) {
        super(_oUnoObject);
        m_oUnoReturnObject = _oUnoReturnObject;
        aPropertyValue = _aPropertyValue;
        m_sPropertyName = aPropertyValue.Name;
    }


    public int getPropertyNodeType(){
        return m_nPropertyType;
    }


    public void setPropertyNodeType(int _nPropertyType){
        m_nPropertyType = _nPropertyType;
    }


    public String getPropertyName(){
        return m_sPropertyName;
    }

    public String getName(){
        return this.m_sPropertyName;
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
        return sClassName;
    }


    public String getAnchor(){
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


    protected boolean isFoldable(){
        boolean bIsFoldable = false;
        if (! isPrimitive()){
            String sTypeName = getUnoReturnObject().getClass().getName();
            bIsFoldable = (!sTypeName.equals("com.sun.star.uno.Type"));
        }
        return bIsFoldable;
    }


    protected String getLabel(){
        if (!sLabel.equals("")){
            if (! isPrimitive()){
                if (isFoldable()){
                    sLabel = getPropertyTypeDescription(aProperty, getUnoReturnObject());
                }
                else{
                    sLabel = getStandardPropertyDescription(aProperty, getUnoReturnObject());
                }
            }
            else {
                sLabel =  getStandardPropertyDescription(aProperty, getUnoReturnObject());
            }
        }
        return sLabel;
    }

    public Property getProperty(){
        return aProperty;
    }

    protected  static String getPropertyTypeDescription(Property _aProperty, Object _oUnoObject){
        return _aProperty.Type.getTypeName() + " " + _aProperty.Name + " = " + _oUnoObject.toString();
    }


    protected static String getStandardPropertyDescription(Property _aProperty, Object _objectElement){
        if (!Introspector.isObjectPrimitive(_objectElement)){
            return _aProperty.Name + " = (" + _aProperty.Type.getTypeName() + ") ";
        }
        else{
            return _aProperty.Name + " (" + _aProperty.Type.getTypeName() + ") = " + getDisplayValueOfPrimitiveType(_objectElement);
        }
    }


    protected static String getStandardPropertyValueDescription(PropertyValue _aPropertyValue){
        if (!Introspector.isObjectPrimitive(_aPropertyValue.Value)){
            return _aPropertyValue.Name;
        }
        else{
            return _aPropertyValue.Name + " : " + UnoNode.getDisplayValueOfPrimitiveType(_aPropertyValue.Value);
        }
    }
}


