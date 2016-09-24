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
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;



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

    @Override
    public String getName(){
        return m_oUnoMethodNode.getName();
    }

    @Override
    public String getClassName(){
        String sClassName = "";
        sClassName = getXIdlMethod().getDeclaringClass().getName();
        if (sClassName.equals("")){
            sClassName = super.getClassName();
        }
        return sClassName;
    }

    @Override
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

    public ArrayList<Object> getMethodObjects(){
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
