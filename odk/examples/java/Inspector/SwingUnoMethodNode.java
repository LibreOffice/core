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


import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;



public class SwingUnoMethodNode extends SwingUnoNode implements ActionListener, XUnoMethodNode{

    private UnoMethodNode m_oUnoMethodNode;
    private XDialogProvider m_xDialogProvider = null;

    public SwingUnoMethodNode(XIdlMethod _xIdlMethod, Object _oUnoObject, XDialogProvider _xDialogProvider) {
        super(_oUnoObject);
        m_oUnoMethodNode = new UnoMethodNode(_xIdlMethod, _oUnoObject, this);
        m_xDialogProvider = _xDialogProvider;
        setUserObject(m_oUnoMethodNode.getNodeDescription());
        if (m_oUnoMethodNode.isFoldable()){
            setFoldable(true);
        }
    }

    public boolean isFoldable(){
        return m_oUnoMethodNode.isFoldable();
    }

    public String getName(){
        return m_oUnoMethodNode.getName();
    }

    public String getClassName(){
        String sClassName = "";
        sClassName = getXIdlMethod().getDeclaringClass().getName();
        if (sClassName.equals("")){
            sClassName = super.getClassName();
        }
        return sClassName;
    }

    public String getAnchor(){
        return m_oUnoMethodNode.getAnchor();
    }

    public Object invoke(Object _oUnoObject, Object[] _oParameters) throws com.sun.star.uno.Exception{
        return m_oUnoMethodNode.invoke(_oUnoObject, _oParameters);
    }


    public Object[] getLastParameterObjects(){
        return m_oUnoMethodNode.getLastParameterObjects();
    }

    public Object getLastUnoReturnObject(){
        return m_oUnoMethodNode.getLastUnoReturnObject();
    }

    public TypeClass getTypeClass() {
        return m_oUnoMethodNode.getTypeClass();
    }

    public XIdlMethod getXIdlMethod() {
        return m_oUnoMethodNode.getXIdlMethod();
    }

    public boolean hasParameters() {
        return m_oUnoMethodNode.hasParameters();
    }

    public Object invoke()  throws com.sun.star.uno.Exception{
        return m_oUnoMethodNode.invoke();
    }

    public boolean isInvoked() {
        return m_oUnoMethodNode.isInvoked();
    }

    public boolean isInvokable(){
        return m_oUnoMethodNode.isInvokable();
    }

    public boolean isPrimitive() {
        return m_oUnoMethodNode.isPrimitive();
    }

    public void actionPerformed(ActionEvent e){
        openIdlDescription(m_xDialogProvider.getIDLPath());
    }

    public Vector getMethodObjects(){
        MethodParametersDialog oMethodParametersDialog = new MethodParametersDialog(this);
        oMethodParametersDialog.addActionListener(this);
        return oMethodParametersDialog.getMethodObjects();
    }


    public String getParameterDescription(){
        return m_oUnoMethodNode.getParameterDescription();
    }


    public String getStandardMethodDescription(){
        return m_oUnoMethodNode.getStandardMethodDescription();
    }
}