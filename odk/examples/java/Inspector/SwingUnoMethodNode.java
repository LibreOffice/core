/*************************************************************************
 *
 *  $RCSfile: SwingUnoMethodNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-30 08:13:33 $
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
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;



public class SwingUnoMethodNode extends SwingUnoNode implements ActionListener, XUnoMethodNode{

    private UnoMethodNode m_oUnoMethodNode;

    public SwingUnoMethodNode(XIdlMethod _xIdlMethod, Object _oUnoObject) {
        super(_oUnoObject);
        m_oUnoMethodNode = new UnoMethodNode(_xIdlMethod, _oUnoObject, this);
        setUserObject(m_oUnoMethodNode.getNodeDescription());
        if (m_oUnoMethodNode.isFoldable()){
            setFoldable(true);
        }
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

    public boolean isPrimitive() {
        return m_oUnoMethodNode.isPrimitive();
    }

    public void actionPerformed(ActionEvent e){
        openIdlDescription();
    }

    public Vector getMethodObjects(){
        MethodParametersDialog oMethodParametersDialog = new MethodParametersDialog(this);
        oMethodParametersDialog.addActionListener(this);
        return oMethodParametersDialog.getMethodObjects();
    }


    public String getStandardMethodDescription(){
        return m_oUnoMethodNode.getStandardMethodDescription();
    }
}