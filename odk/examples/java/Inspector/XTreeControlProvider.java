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

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.Type;

public interface XTreeControlProvider {



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
