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

import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.ParamMode;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;
import java.util.Vector;

public class UnoMethodNode extends UnoNode{
    XIdlMethod m_xIdlMethod = null;
    Object[] m_oParamObjects = null;
    Object m_oUnoReturnObject = null;
    boolean m_bisInvoked = false;
    XUnoMethodNode m_xUnoMethodNode = null;


    /** Creates a new instance of UnoMethodNode */
    public UnoMethodNode(XIdlMethod _xIdlMethod, Object _oUnoObject, XUnoMethodNode _xUnoMethodNode) {
        super(_oUnoObject);
        m_xIdlMethod = _xIdlMethod;
        m_oParamObjects = new Object[m_xIdlMethod.getParameterInfos().length];
        m_xUnoMethodNode = _xUnoMethodNode;
    }

    protected boolean isFoldable(){
        return ((!this.isPrimitive()) &&  (getTypeClass().getValue() != TypeClass.VOID_value));
    }

    protected boolean isInvokable(){
        boolean bisFoldable = true;
        XIdlClass[] xIdlClasses = m_xIdlMethod.getParameterTypes();
        for (int i = 0; i < xIdlClasses.length; i++){
            bisFoldable = Introspector.isPrimitive(xIdlClasses[i].getTypeClass());
            if (!bisFoldable){
                return false;
            }
        }
        return bisFoldable;
    }

    public XIdlMethod getXIdlMethod(){
        return m_xIdlMethod;
    }


    public String getAnchor(){
        return getXIdlMethod().getName();
    }


    public String getName(){
        return getXIdlMethod().getName();
    }


    public Object invoke(){
        Object oUnoReturnObject = null;
        if (!hasParameters()){
            oUnoReturnObject = invokeParameterlessMethod();
            m_bisInvoked = true;
        }
        else{
            Vector<Object> oUnoMethodObjects = m_xUnoMethodNode.getMethodObjects();
            if (oUnoMethodObjects != null){
                for (int i = 0; i < getXIdlMethod().getParameterInfos().length; i++){
                    this.m_oParamObjects[i] = oUnoMethodObjects.get(i);
                }
                if (oUnoMethodObjects.size() == m_oParamObjects.length + 1){
                    oUnoReturnObject = oUnoMethodObjects.get(oUnoMethodObjects.size()-1);
                }
                m_bisInvoked = (oUnoReturnObject != null);
            }
        }
        m_oUnoReturnObject = oUnoReturnObject;
        return oUnoReturnObject;
    }


    public boolean isInvoked(){
        return m_bisInvoked;
    }


    protected String getNodeDescription(){
        String sNodeDescription = "";
        String sParameters = getParameterDescription();
        if (m_xIdlMethod.getParameterInfos().length > 0){
            sNodeDescription = getStandardMethodDescription();
        }
        else{
            TypeClass typeClass = getTypeClass();
            if (typeClass != TypeClass.VOID){
                sNodeDescription = getStandardMethodDescription();
            }
            else{
                sNodeDescription = getStandardMethodDescription();
            }
        }
        return sNodeDescription;
    }


    public String getStandardMethodDescription(){
        String sNodeDescription = m_xIdlMethod.getReturnType().getName() + " " + m_xIdlMethod.getName() + " (" + getParameterDescription() + " )";
        if (isPrimitive()){
            sNodeDescription += "";
        }
        return sNodeDescription;
    }


    public boolean hasParameters(){
        return (m_xIdlMethod.getParameterInfos().length > 0);
    }


    public Object[] getLastParameterObjects(){
        return m_oParamObjects;
    }


    public Object getLastUnoReturnObject(){
        return m_oUnoReturnObject;
    }


    public String getParameterDescription(){
        ParamInfo[] paramInfo = m_xIdlMethod.getParameterInfos();
        String sParameters = "";
        String sStandardMethodDisplayText = m_xIdlMethod.getReturnType().getName() + " " + m_xIdlMethod.getName() + " (" + sParameters + " )";
        if (Introspector.isValid(paramInfo)) {
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


    private Object invokeParameterlessMethod(){
    try {
        Object[][] aParamInfo = new Object[1][];
        aParamInfo[0] = new Object[] {};
        return getXIdlMethod().invoke(getUnoObject(), aParamInfo);
    } catch (Exception ex) {
        ex.printStackTrace(System.err);
        return null;
    }}


    public boolean isPrimitive(){
        return Introspector.isObjectPrimitive(m_xIdlMethod.getClass(), getTypeClass());
    }


    protected Object invoke(Object _oUnoObject, Object[] oParameters) throws com.sun.star.uno.Exception{
        Object[][] aParams = new Object[1][oParameters.length];
        for ( int i = 0; i < oParameters.length; i++ ) {
            aParams[0][i] = oParameters[i];
        }
        return m_xIdlMethod.invoke(_oUnoObject, aParams);
    }
}
