/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;

public class SwingUnoPropertyNode extends SwingUnoNode implements XUnoPropertyNode{

    private UnoPropertyNode m_oUnoPropertyNode = null;


    public SwingUnoPropertyNode(Property _aProperty, Object _oUnoObject, Object _oUnoReturnObject) {
        super(_oUnoObject);
        m_oUnoPropertyNode = new UnoPropertyNode(_aProperty, _oUnoObject, _oUnoReturnObject);
        super.setUserObject(m_oUnoPropertyNode.getLabel());
        setFoldable(m_oUnoPropertyNode.isFoldable());
    }


    public SwingUnoPropertyNode(Property _aProperty){
        super(null);
        m_oUnoPropertyNode = new UnoPropertyNode(_aProperty);
    }


    public SwingUnoPropertyNode(PropertyValue _aPropertyValue, Object _oUnoObject, Object _oUnoReturnObject) {
        super(_oUnoObject);
        m_oUnoPropertyNode = new UnoPropertyNode(_aPropertyValue, _oUnoObject, _oUnoReturnObject);
    }

    public String getName(){
        return m_oUnoPropertyNode.getName();
    }


    public Object getUnoReturnObject(){
        return m_oUnoPropertyNode.getUnoReturnObject();
    }


    public String getClassName(){
        String sClassName = m_oUnoPropertyNode.getClassName();
        if (sClassName.equals("")){
            sClassName = super.getClassName();
        }
        return sClassName;
    }

    public String getAnchor(){
        return m_oUnoPropertyNode.getAnchor();
    }

    public int getPropertyNodeType(){
        return m_oUnoPropertyNode.getPropertyNodeType();
    }


    public void setPropertyNodeType(int _nPropertyType){
        m_oUnoPropertyNode.setPropertyNodeType(_nPropertyType);
    }

    public Property getProperty(){
        return m_oUnoPropertyNode.getProperty();
    }

}
