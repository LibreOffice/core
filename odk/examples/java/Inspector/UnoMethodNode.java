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



import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.ParamMode;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.TypeClass;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
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
            Vector oUnoMethodObjects = m_xUnoMethodNode.getMethodObjects();
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
        ex.printStackTrace(System.out);
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
