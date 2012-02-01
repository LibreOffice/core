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
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.Type;

public interface XTreeControlProvider {

    public static String sTitle = "User defined";

    public String enableFilterElements(XUnoNode _oUnoNode);

    public void setSourceCode(String _sSourceCode);

    public Object inspect(java.lang.Object _oUserDefinedObject, String _sTitle) throws com.sun.star.uno.RuntimeException;

    public void nodeInserted(XUnoNode _oParentNode, XUnoNode _oChildNode, int index);

    public void nodeChanged(XUnoNode _oNode);

    public boolean setNodeVisible(Object node, boolean v);

    public XUnoNode getSelectedNode();

    public XTreePathProvider getSelectedPath();

    public void expandPath(XTreePathProvider xTreePathProvider) throws java.lang.ClassCastException;

    public void addTreeExpandListener();

    public void addInspectorPane(InspectorPane _oInspectorPane);

    public boolean isPropertyNode(XUnoNode _oUnoNode);

    public boolean isMethodNode(XUnoNode _oUnoNode);

    public boolean isFacetteNode(XUnoNode _oUnoNode);

    public XUnoNode addUnoNode(Object _oUnoObject);

    public XUnoNode addUnoNode(Object _oUnoObject, Type _aType);

    public XUnoFacetteNode addUnoFacetteNode(XUnoNode _oParentNode, String _sNodeDescription, Object _oUnoObject);

    public XUnoMethodNode addMethodNode(Object _objectElement, XIdlMethod _xIdlMethod);

    public XUnoPropertyNode addUnoPropertyNodeWithName(Property _aProperty);

    public XUnoPropertyNode addUnoPropertyNodeWithHandle(Property _aProperty);

    public XUnoPropertyNode addUnoPropertyNodeWithType(Property _aProperty);

    public XUnoPropertyNode addUnoPropertyNodeWithAttributesDescription(Property _aProperty);

    public XUnoPropertyNode addUnoPropertyNode(Object _oUnoObject, Property _aProperty);

    public XUnoPropertyNode addUnoPropertyNode(Object _oUnoObject, PropertyValue _aPropertyValue, Object _oReturnObject);

    public XUnoPropertyNode addUnoPropertyNode(Object _oUnoObject, Property _aProperty, Object _oUnoReturnObject);
}
