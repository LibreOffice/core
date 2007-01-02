/*************************************************************************
 *
 *  $RCSfile: UnoMethodNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:02:06 $
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

import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.ParamMode;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

public class UnoMethodNode extends UnoNode implements ActionListener{
    XIdlMethod m_xIdlMethod = null;
    Object[] m_oParameterObjects = null;
    boolean m_bisInvoked = false;
    Object m_oUnoReturnObject;



    /** Creates a new instance of UnoMethodNode */
    public UnoMethodNode(XIdlMethod _xIdlMethod, Object _oUnoObject) {
        super(_oUnoObject);
        m_xIdlMethod = _xIdlMethod;
        m_oParameterObjects = new Object[m_xIdlMethod.getParameterInfos().length];
        assignNodeDescription();
        if ((!this.isPrimitive()) &&  (getTypeClass().getValue() != TypeClass.VOID_value)){
            addDummyNode();
        }
    }


    public XIdlMethod getXIdlMethod(){
        return m_xIdlMethod;
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
        return getXIdlMethod().getName();
    }


    public String getName(){
        return getXIdlMethod().getName();
    }


    public Object getUnoReturnObject(){
        if (!hasParameters()){
            m_oUnoReturnObject = invokeParameterlessMethod();
            m_bisInvoked = true;
        }
        else{
            MethodParametersDialog oMethodParametersDialog = new MethodParametersDialog(getXIdlMethod(), getUnoObject());
            oMethodParametersDialog.addActionListener(this);
            Vector oUnoMethodObjects = oMethodParametersDialog.getMethodObjects();
            for (int i = 0; i < getXIdlMethod().getParameterInfos().length; i++){
                this.m_oParameterObjects[i] = oUnoMethodObjects.get(i);
            }
            if (oUnoMethodObjects.size() == m_oParameterObjects.length + 1){
                m_oUnoReturnObject = oUnoMethodObjects.get(oUnoMethodObjects.size()-1);
            }
            m_bisInvoked = (m_oUnoReturnObject != null);
        }
        return m_oUnoReturnObject;
    }


    public boolean isInvoked(){
        return m_bisInvoked;
    }

    public Object[] getParameterValues(){
        return this.m_oParameterObjects;
    }


    private void assignNodeDescription(){
        String sParameters = getParameterDescription();
        if (m_xIdlMethod.getParameterInfos().length > 0){
            setUserObject(getStandardMethodDescription());
        }
        else{
            TypeClass typeClass = getTypeClass();
            if (typeClass != TypeClass.VOID){
                setUserObject(getStandardMethodDescription());
            }
            else{
                setUserObject(getStandardMethodDescription());
            }
        }
    }


    protected String getStandardMethodDescription(){
        String sNodeDescription = m_xIdlMethod.getReturnType().getName() + " " + m_xIdlMethod.getName() + " (" + getParameterDescription() + " )";
        if (isPrimitive()){
            sNodeDescription += "";
        }
        return sNodeDescription;
    }



    public boolean hasParameters(){
        return (m_xIdlMethod.getParameterInfos().length > 0);
    }


    public String getParameterDescription(){
        ParamInfo[] paramInfo = m_xIdlMethod.getParameterInfos();
        String sParameters = "";
        String sStandardMethodDisplayText = m_xIdlMethod.getReturnType().getName() + " " + m_xIdlMethod.getName() + " (" + sParameters + " )";
        if (isValid(paramInfo)) {
            //  get all parameters with type and mode
            for ( int i = 0; i < paramInfo.length; i++ ) {
                XIdlClass xIdlClass = paramInfo[ i ].aType;
                if ( i == 0 ) {
                    //  the first parameter has no leading comma
                    sParameters += "[" + getParamMode(paramInfo[ i ].aMode ) + "] " + xIdlClass.getName();
                }
                else {
                    //  all other parameters are separated with comma
                    sParameters += ", [" + getParamMode(paramInfo[ i ].aMode ) + "] " + xIdlClass.getName();
                }
            }
        }
        return sParameters;
    }


    //  return the parameter mode (IN, OUT, INOUT)
    private static String getParamMode(ParamMode paramMode) {
        String toReturn = "";
        if ( paramMode == ParamMode.IN ) {
            toReturn = "IN";
        }
        if ( paramMode == ParamMode.OUT ) {
            toReturn = "OUT";
        }
        if ( paramMode == ParamMode.INOUT ) {
            toReturn = "INOUT";
        }
        return( toReturn );
    }

    public TypeClass getTypeClass(){
        XIdlClass xIdlClass = m_xIdlMethod.getReturnType();
        return xIdlClass.getTypeClass();
    }


    public Object invokeParameterlessMethod(){
    try {
        Object[][] aParamInfo = new Object[1][];
        aParamInfo[0] = new Object[] {};
        return getXIdlMethod().invoke(getUnoObject(), aParamInfo);
    } catch (Exception ex) {
        ex.printStackTrace(System.out);
        return null;
    }}


    public boolean areAllMethodParametersPrimitive(){
        boolean breturn = true;
        ParamInfo[] aParamInfos = m_xIdlMethod.getParameterInfos();
        for (int i = 0; i < aParamInfos.length; i++){
            ParamInfo aParamInfo = aParamInfos[i];
            if (!aParamInfo.aType.getClass().isPrimitive()){
                breturn = false;
                break;
            }
        }
        return breturn;
    }


    public String getParameterName(ParamInfo _aParamInfo){
        return _aParamInfo.aName;
    }

    public boolean isPrimitive(){
        return Introspector.isObjectPrimitive(m_xIdlMethod.getClass(), getTypeClass());
    }


    public void actionPerformed(ActionEvent e){
        openIdlDescription();
    }


    public String[] getVariableDeclarations(int _nLanguageType){
        return new String[]{""};
    }
}
