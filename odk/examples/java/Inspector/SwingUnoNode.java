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
import com.sun.star.uno.Type;
import javax.swing.tree.TreeNode;


public class SwingUnoNode extends HideableMutableTreeNode implements XUnoNode{
    private UnoNode m_oUnoNode;

    /** Creates a new instance of SwingUnoNode */
    public SwingUnoNode(Object _oUnoObject) {
        super();
        m_oUnoNode = new UnoNode(_oUnoObject);
    }


    public SwingUnoNode(Object _oUnoObject, Type _aType) {
        super();
        m_oUnoNode = new UnoNode(_oUnoObject, _aType);
        if (_aType != null){
            this.setLabel(_aType.getTypeName());
        }
    }


    public Object getUnoObject(){
        return m_oUnoNode.getUnoObject();
    }

    public void setVisible(String _sFilter){
        boolean bisVisible = isFilterApplicable(_sFilter);
        super.setVisible(bisVisible);
    }

    public boolean isFilterApplicable(String _sFilter) {
        return m_oUnoNode.isFilterApplicable(_sFilter, getName());
    }

    public void setParameterObjects(Object[] _oParamObjects) {
        m_oUnoNode.setParameterObjects(_oParamObjects);
    }

    public void openIdlDescription(String _SDKPath) {
        m_oUnoNode.openIdlDescription(_SDKPath, getClassName());
    }

    public Object[] getParameterObjects() {
        return m_oUnoNode.getParameterObjects();
    }

    public String getClassName(){
        String sClassName = m_oUnoNode.getClassName();
        if (sClassName.equals("")){
            TreeNode oTreeNode = getParent();
            if (oTreeNode != null){
               if (oTreeNode instanceof XUnoNode){
                   SwingUnoNode oUnoNode = (SwingUnoNode) oTreeNode;
                   sClassName = oUnoNode.getClassName();
               }
            }
        }
        return sClassName;
    }

    public String getAnchor() {
        return m_oUnoNode.getAnchor();
    }


    public void setFoldable(boolean _bIsFoldable){
        if (_bIsFoldable){
            addDummyNode();
        }
        else{
            removeDummyNode();
        }
    }


    public XUnoNode getParentNode(){
        return (SwingUnoNode) super.getParent();
    }


    public void addChildNode(XUnoNode _xUnoNode) {
        super.add((SwingUnoNode) _xUnoNode);
    }

    public void  setLabel(String _sLabel){
        super.setUserObject(_sLabel);
        this.m_oUnoNode.setLabel(_sLabel);
    }

    public String getLabel(){
        return (String) super.getUserObject();
    }


    @Override
    public int getChildCount(){
        return super.getChildCount();
    }


    public XUnoNode getChild(int _i){
        return (SwingUnoNode) super.getChildAt(_i);
    }

    public int getNodeType(){
        return m_oUnoNode.getNodeType();
    }

    public void setNodeType(int _nNodeType){
        m_oUnoNode.setNodeType(_nNodeType);
    }


    public String getName(){
        return getClassName();
    }


    public Type getUnoType(){
        return m_oUnoNode.getUnoType();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
