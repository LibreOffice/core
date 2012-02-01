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
        m_oUnoNode.openIdlDescription(_SDKPath, getClassName(), getAnchor());
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
