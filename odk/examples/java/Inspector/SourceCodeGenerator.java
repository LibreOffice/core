/*************************************************************************
 *
 *  $RCSfile: SourceCodeGenerator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:00:18 $
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
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;



public class SourceCodeGenerator {
    private Vector sHeaderDescriptionVector = new Vector();
    private XLanguageSourceCodeGenerator m_xLanguageSourceCodeGenerator;
    private String sHeaderCode = "";
    private String sStatementCode = "";
    private String sMainMethodSignature = "";
    private Hashtable aVariables = new Hashtable();
    private final String SSUFFIXSEPARATOR = "_";
    private final String SVARIABLENAME = "VariableName";
    private final String SARRAYVARIABLENAME = "VariableNameList";
    private Introspector m_oIntrospector;


    /** Creates a new instance of SourceCodeGenerator */
    public SourceCodeGenerator() {
        m_xLanguageSourceCodeGenerator = new JavaCodeGenerator();
        m_oIntrospector = Introspector.getIntrospector();
    }


    public String addSourceCodeOfUnoObject(XTreePathProvider _xTreepathProvider){
        String sSourceCode = "";
        String sVariableName = "";
        if (_xTreepathProvider != null) {
            for (int i = 0; i < _xTreepathProvider.getPathCount(); i++){
                UnoNode oUnoNode = _xTreepathProvider.getPathComponent(i);
                if (i == 0){
                    sVariableName = "<UNOENTRYOBJECT>";
                    sMainMethodSignature = m_xLanguageSourceCodeGenerator.getMainMethodSignatureSourceCode(oUnoNode, sVariableName);
                }
                else{
                    if (oUnoNode instanceof UnoMethodNode){
                        UnoMethodNode oUnoMethodNode = (UnoMethodNode) oUnoNode;
                        XIdlClass xIdlClass = oUnoMethodNode.getXIdlMethod().getReturnType();
                        String sTypeName = xIdlClass.getName();
                        TypeClass aTypeClass = xIdlClass.getTypeClass();
                        UnoObjectDefinition oUnoReturnObjectDefinition = new UnoObjectDefinition(oUnoMethodNode.getUnoReturnObject(), sTypeName, aTypeClass);
                        String sNewVariableName = oUnoReturnObjectDefinition.getVariableName();
                        if (!isVariableDeclared(oUnoReturnObjectDefinition)){
                            sStatementCode += "\n" + getMethodStatementSourceCode(oUnoMethodNode, sVariableName, oUnoReturnObjectDefinition);
                        }
                        sVariableName = sNewVariableName;
                    }
                    else if (oUnoNode instanceof UnoPropertyNode){
                        UnoPropertyNode oUnoPropertyNode = (UnoPropertyNode) oUnoNode;
                        Any oReturnObject = com.sun.star.uno.Any.complete(oUnoPropertyNode.getUnoReturnObject());
                        UnoObjectDefinition oUnoReturnObjectDefinition = new UnoObjectDefinition(oReturnObject);
                        String sNewVariableName = oUnoReturnObjectDefinition.getVariableName();
                        if (!isVariableDeclared(oUnoReturnObjectDefinition)){
                            sStatementCode += "\n" + getPropertyStatementSourceCode(oUnoPropertyNode, sVariableName, oUnoReturnObjectDefinition);
                        }
                        sVariableName = sNewVariableName;
                    }
                }
            }
        }
        String sCompleteCode = combineCompleteSourceCode();
        return sCompleteCode;
    }


    private String combineCompleteSourceCode(){
        String sCompleteCode = "";
        sCompleteCode  += getHeaderSourceCode();
        sCompleteCode += sMainMethodSignature;
        sCompleteCode += sStatementCode;
        sCompleteCode += m_xLanguageSourceCodeGenerator.getMethodTerminationSourceCode();
        return sCompleteCode;
    }


    public String getPropertyStatementSourceCode(UnoPropertyNode _oUnoPropertyNode, String _sObjectDescription, UnoObjectDefinition _oUnoReturnObjectDefinition){
        String sReturnObjectVariableDefinition = "";
        String sStatement = "";
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(_oUnoPropertyNode.getUnoObject(), "com.sun.star.beans.XPropertySet");
        if (!isVariableDeclared(oUnoObjectDefinition)){
            String sObjectVariableDefinition = getVariableDeclaration(oUnoObjectDefinition);
            sStatement += m_xLanguageSourceCodeGenerator.getqueryInterfaceSourceCode("XPropertySet", sObjectVariableDefinition, _sObjectDescription);
        }
        if (_oUnoReturnObjectDefinition.getTypeClass().getValue() != TypeClass.VOID_value){
            sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition) + " = ";
        }
        sStatement += m_xLanguageSourceCodeGenerator.getPropertyValueGetterSourceCode(_oUnoPropertyNode.getPropertyName(), _oUnoReturnObjectDefinition.getVariableName(), oUnoObjectDefinition.getVariableName());
        return sStatement;
    }


    public String getMethodStatementSourceCode(UnoMethodNode _oUnoMethodNode, String _sObjectDescription, UnoObjectDefinition _oUnoReturnObjectDefinition){
        String sReturnObjectVariableDefinition = "";
        XIdlMethod xIdlMethod = _oUnoMethodNode.getXIdlMethod();
        String sStatement = "";
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(_oUnoMethodNode.getUnoObject(), _oUnoMethodNode.getClassName());
        if (!isVariableDeclared(oUnoObjectDefinition)){
            String sObjectVariableDefinition = getVariableDeclaration(oUnoObjectDefinition);
            String sShortClassName = getShortClassName(oUnoObjectDefinition.getTypeName());
            sStatement = m_xLanguageSourceCodeGenerator.getqueryInterfaceSourceCode(sShortClassName, sObjectVariableDefinition, _sObjectDescription);
        }
        _oUnoReturnObjectDefinition.setTypeName(xIdlMethod.getReturnType().getName());
        String sParameterCode = getMethodParameterValueDescription(_oUnoMethodNode, false);
        if (_oUnoReturnObjectDefinition.getTypeClass().getValue() != TypeClass.VOID_value){
            sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition) + " = ";
        }
        sStatement += "\t" + sReturnObjectVariableDefinition + oUnoObjectDefinition.getVariableName() + "." + xIdlMethod.getName() + "(" + sParameterCode + ");";
        return sStatement;
    }


    private String getRootDescription(UnoNode _oUnoNode){
        return "_o" + _oUnoNode.toString();
    }


    private String getHeaderSourceCode(){
        Vector sHeaderStatements = new HeaderStatements();
        String sHeaderSourcecode = "";
        Enumeration aEnumeration = aVariables.elements();
        while(aEnumeration.hasMoreElements()){
            UnoObjectDefinition oUnoObjectDefinition = (UnoObjectDefinition) aEnumeration.nextElement();
            String sCurHeaderStatement = this.m_xLanguageSourceCodeGenerator.getHeaderSourceCode(oUnoObjectDefinition.getUnoObject(), oUnoObjectDefinition.getTypeName());
            if (!sHeaderStatements.contains(sCurHeaderStatement)){
                sHeaderSourcecode += sCurHeaderStatement;
                sHeaderStatements.add(sCurHeaderStatement);
            }
        }
        return sHeaderSourcecode +"\n";
    }

    private class HeaderStatements extends Vector{

        public boolean contains(Object _oElement){
           String sCompName = (String) _oElement;
           for (int i = 0; i < this.size(); i++){
               String sElement = (String) this.get(i);
               if (sElement.equals(sCompName)){
                   return true;
               }
           }
           return false;
       }

    }


    private boolean isVariableDeclared(UnoObjectDefinition _oUnoObjectDefinition){
        boolean bisDeclared = false;
        String sVariableName = _oUnoObjectDefinition.getVariableStemName();
        bisDeclared = aVariables.containsKey(sVariableName);
        if (bisDeclared){
            Object oUnoObject = ((UnoObjectDefinition) aVariables.get(sVariableName)).getUnoObject();
            String sUnoObjectIdentity = oUnoObject.toString();
            if (m_oIntrospector.isObjectPrimitive(oUnoObject)){
                bisDeclared = false;
            }
            else{
                bisDeclared = (sUnoObjectIdentity.equals(oUnoObject.toString()));
            }
        }
        return bisDeclared;
    }


    private String addUniqueVariableName(String _sStemVariableName, UnoObjectDefinition _oUnoObjectDefinition){
        boolean bElementexists = true;
        int a = 2;
        String sCompName = _sStemVariableName;
        while (bElementexists){
            if (! aVariables.containsKey(sCompName)){
                aVariables.put(sCompName, _oUnoObjectDefinition);
                break;
            }
            sCompName = _sStemVariableName + SSUFFIXSEPARATOR + a++;
        }
        return sCompName;
    }


    private String getVariableDeclaration(UnoObjectDefinition _oUnoObjectDefinition){
        TypeClass aTypeClass = _oUnoObjectDefinition.getTypeClass();
        return getVariableDeclaration(_oUnoObjectDefinition, aTypeClass);
    }



    private String getTypeString(String _sTypeName, TypeClass _aTypeClass){
        String sTypeString = "";
        switch (_aTypeClass.getValue()){
            case TypeClass.BOOLEAN_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getbooleanTypeDescription();
                break;
            case TypeClass.BYTE_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getbyteTypeDescription();
                break;
            case TypeClass.CHAR_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getcharTypeDescription();
                break;
            case TypeClass.DOUBLE_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getdoubleTypeDescription();
                break;
            case TypeClass.FLOAT_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getfloatTypeDescription();
                break;
            case TypeClass.HYPER_value:
                sTypeString = m_xLanguageSourceCodeGenerator.gethyperTypeDescription();
                break;
            case TypeClass.LONG_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getlongTypeDescription();
                break;
            case TypeClass.SHORT_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getshortTypeDescription();
                break;
            case TypeClass.STRING_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getstringTypeDescription();
                break;
            case TypeClass.UNSIGNED_HYPER_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getunsignedhyperTypeDescription();
                break;
            case TypeClass.UNSIGNED_LONG_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getunsignedlongTypeDescription();
                break;
            case TypeClass.UNSIGNED_SHORT_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getdoubleTypeDescription();
                break;
            case TypeClass.SEQUENCE_value:
                //TODO mehrdimensionale Arrays berücksichtigen!!!
                XTypeDescription xTypeDescription = Introspector.getIntrospector().getReferencedType(_sTypeName);
                sTypeString = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass());
                break;
            case TypeClass.ENUM_value:
                System.out.println("declare Enum Variable!!!");
                break;
            case TypeClass.ANY_value:
            case TypeClass.INTERFACE_ATTRIBUTE_value:
            case TypeClass.INTERFACE_METHOD_value:
            case TypeClass.INTERFACE_value:
            case TypeClass.STRUCT_value:
            case TypeClass.PROPERTY_value:
            case TypeClass.TYPE_value:
                String sShortClassName = getShortClassName(_sTypeName);
                sTypeString = m_xLanguageSourceCodeGenerator.getObjectTypeDescription(sShortClassName);
            default:
        }
        return sTypeString;
    }


    private String getVariableDeclaration(UnoObjectDefinition _oUnoObjectDefinition, TypeClass _aTypeClass){
        boolean bIsArray = m_oIntrospector.isObjectSequence(_oUnoObjectDefinition.getUnoObject());
        String sVariableName = _oUnoObjectDefinition.getVariableName();
        String sTypeName = _oUnoObjectDefinition.getTypeName();
        String sTypeString = getTypeString(sTypeName, _aTypeClass);
        String sVariableDeclaration = m_xLanguageSourceCodeGenerator.getVariableDeclaration(sTypeString, sVariableName, bIsArray);
        addUniqueVariableName(sVariableName, _oUnoObjectDefinition);
        return sVariableDeclaration;
    }


    public String getVariableInitialization(UnoObjectDefinition _oUnoObjectDefinition){
        String sObjectVariableDeclaration = "";
        String sVariableStemName = _oUnoObjectDefinition.getVariableStemName();
        if (isVariableDeclared(_oUnoObjectDefinition)){
            sObjectVariableDeclaration = sVariableStemName;
        }
        else{
            sObjectVariableDeclaration =  getVariableDeclaration(_oUnoObjectDefinition);
        }
        return sObjectVariableDeclaration;
    }


    public static String getShortClassName(String _sClassName){
        String sShortClassName = _sClassName;
        int nindex = _sClassName.lastIndexOf(".");
        if ((nindex < _sClassName.length()) && nindex > -1){
            sShortClassName = _sClassName.substring(nindex + 1);
        }
        return sShortClassName;
    }


    public String getVariableNameforUnoObject(String _sShortClassName){
        if (_sShortClassName.startsWith("X")){
            return  "x" + _sShortClassName.substring(1);
        }
        else{
            return _sShortClassName;
        }
    }


    public class UnoObjectDefinition{
        Object m_oUnoObject = null;
        Type aType = null;
        String sVariableStemName = "";
        String sVariableName = "";
        String m_sTypeName = "";
        TypeClass m_aTypeClass = null;

        public UnoObjectDefinition(Any _oUnoObject){
            m_sTypeName = _oUnoObject.getType().getTypeName();
            m_aTypeClass = _oUnoObject.getType().getTypeClass();
            m_oUnoObject = _oUnoObject;
        }

        public UnoObjectDefinition(Object _oUnoObject, String _sTypeName, TypeClass _aTypeClass){
            m_oUnoObject = _oUnoObject;
            m_sTypeName = _sTypeName;
            m_aTypeClass = _aTypeClass;
        }

        public UnoObjectDefinition(Object _oUnoObject, String _sTypeName){
            m_oUnoObject = _oUnoObject;
            m_sTypeName = _sTypeName;
            m_aTypeClass = AnyConverter.getType(_oUnoObject).getTypeClass();
        }


        public Object getUnoObject(){
            return m_oUnoObject;
        }


        public TypeClass getTypeClass(){
            return m_aTypeClass;
        }


        public void setTypeName(String _sTypeName){
            m_sTypeName = _sTypeName;
        }

        public String getTypeName(){
            return m_sTypeName;
        }

        public String getVariableStemName(){
            if (sVariableStemName.equals("")){
                sVariableStemName = getVariableStemName(m_aTypeClass);
            }
            return sVariableStemName;
        }


        public String getVariableStemName(TypeClass _aTypeClass){
            int nTypeClass = _aTypeClass.getValue();
            switch(nTypeClass){
                case TypeClass.BOOLEAN_value:
                    sVariableStemName = "b" + SVARIABLENAME;
                    break;
                case TypeClass.DOUBLE_value:
                case TypeClass.FLOAT_value:
                    sVariableStemName = "f" + SVARIABLENAME;
                    break;
                case TypeClass.BYTE_value:
                case TypeClass.HYPER_value:
                case TypeClass.LONG_value:
                case TypeClass.UNSIGNED_HYPER_value:
                case TypeClass.UNSIGNED_LONG_value:
                case TypeClass.UNSIGNED_SHORT_value:
                case TypeClass.SHORT_value:
                    sVariableStemName = "n" + SVARIABLENAME;
                    break;
                case TypeClass.CHAR_value:
                case TypeClass.STRING_value:
                    sVariableStemName = "s" + SVARIABLENAME;
                    break;
                case TypeClass.SEQUENCE_value:
                    //TODO mehrdimensionale Arrays berücksichtigen!!!
                    XTypeDescription xTypeDescription = Introspector.getIntrospector().getReferencedType(getTypeName());
                    sVariableStemName = getVariableStemName(xTypeDescription.getTypeClass());
                    break;
                case TypeClass.ENUM_value:
                    System.out.println("declare Enum Variable!!!");
                    break;
                case TypeClass.TYPE_value:
                    sVariableStemName = "a" + SVARIABLENAME;
                    break;
                case TypeClass.ANY_value:
                case TypeClass.INTERFACE_ATTRIBUTE_value:
                case TypeClass.INTERFACE_METHOD_value:
                case TypeClass.INTERFACE_value:
                case TypeClass.STRUCT_value:
                case TypeClass.PROPERTY_value:
                    sVariableStemName = getVariableNameforUnoObject(getShortClassName(getTypeName()));

                default:
            }
            return sVariableStemName;
        }


        private String getVariableName() throws NullPointerException{
            if (sVariableName.equals("")){
                boolean bleaveloop = false;
                int a = 2;
                sVariableName = getVariableStemName();
                while (!bleaveloop){
                    if (aVariables.containsKey(sVariableName)){
                        String sUnoObjectIdentity = ((UnoObjectDefinition) aVariables.get(sVariableName)).getUnoObject().toString();
                        if (sUnoObjectIdentity.equals(m_oUnoObject.toString())){
                            bleaveloop = true;
                        }
                        else{
                            sVariableName = getVariableStemName() + SSUFFIXSEPARATOR + a++;
                        }
                    }
                    else{
                        bleaveloop = true;
        //                throw new NullPointerException("SourceCode Variable " + _sStemVariableName + " not defined");
                    }
                }
            }
            return sVariableName;
        }
    }


    public String getStringValueOfObject(Object _oUnoObject, TypeClass _aTypeClass){
        String sReturn = "";
        switch (_aTypeClass.getValue()){
            case TypeClass.BOOLEAN_value:
                boolean bbooleanValue = ((Boolean) _oUnoObject).booleanValue();
                sReturn += Boolean.toString(bbooleanValue);
            case TypeClass.CHAR_value:
                break;
            case TypeClass.DOUBLE_value:
                double fdoubleValue = ((Double) _oUnoObject).doubleValue();
                sReturn += Double.toString(fdoubleValue);
                break;
            case TypeClass.ENUM_value:
                break;
            case TypeClass.FLOAT_value:
                float floatValue = ((Float) _oUnoObject).floatValue();
                sReturn += Float.toString(floatValue);
                break;
            case TypeClass.HYPER_value:
                long nlongValue = ((Long) _oUnoObject).longValue();
                sReturn += Long.toString(nlongValue);
                break;
            case TypeClass.LONG_value:
                int  nintValue = ((Integer) _oUnoObject).intValue();
                sReturn += Integer.toString(nintValue);
                break;
            case TypeClass.SHORT_value:
                short  nshortValue = ((Short) _oUnoObject).shortValue();
                sReturn += Short.toString(nshortValue);
                break;
            case TypeClass.STRING_value:
                sReturn += "\"" + (String) _oUnoObject + "\"";
                break;
            case TypeClass.UNSIGNED_HYPER_value:
                nlongValue = ((Long) _oUnoObject).longValue();
                sReturn += Long.toString(nlongValue);
                break;
            case TypeClass.UNSIGNED_LONG_value:
                nintValue = ((Integer) _oUnoObject).intValue();
                sReturn += Integer.toString(nintValue);
                break;
            case TypeClass.UNSIGNED_SHORT_value:
                nshortValue = ((Short) _oUnoObject).shortValue();
                sReturn += Short.toString(nshortValue);
                break;
            default:
                System.out.println("Type not yet defined!!!");
        }
        return sReturn;
    }


    public String getMethodParameterValueDescription(UnoMethodNode _oUnoMethodNode, boolean _bIncludeParameterNames){
        String sParamSourceCode = "";
        Object[] oParameterObjects = _oUnoMethodNode.getParameterValues();
        ParamInfo[] aParamInfos = _oUnoMethodNode.getXIdlMethod().getParameterInfos();
        for (int i = 0; i < oParameterObjects.length; i++){
            TypeClass aTypeClass = aParamInfos[i].aType.getTypeClass();
            if (_bIncludeParameterNames){
                sParamSourceCode += aParamInfos[i].aName + "=";
            }
            sParamSourceCode += getStringValueOfObject(oParameterObjects[i], aTypeClass);
            if (i < oParameterObjects.length - 1){
                sParamSourceCode += ", ";
            }
        }
        return sParamSourceCode;
    }


    private class JavaCodeGenerator implements XLanguageSourceCodeGenerator{
        String sStatementsCode = "";

        public JavaCodeGenerator(){
        }

        public String getHeaderSourceCode(Object _oUnoObject, String _sClassName){
            String sClassName = _sClassName;
            String sHeaderStatement = "";
            if (!m_oIntrospector.isObjectPrimitive(_oUnoObject)){
                if (m_oIntrospector.isObjectSequence(_oUnoObject)){
                    XTypeDescription xTypeDescription = m_oIntrospector.getReferencedType(sClassName);
                    if (m_oIntrospector.isPrimitive(xTypeDescription.getTypeClass())){
                        return "";
                    }
                    else{
                        sClassName = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass());
                    }
                }
                sHeaderStatement =  "import " + sClassName + ";\n";
            }
            return sHeaderStatement;
        }


        public String getMainMethodSignatureSourceCode(UnoNode _oUnoNode, String _soReturnObjectDescription){
            //TODO try to use + _oUnoNode.getClassName() instead of the hack
            return "public void codesnippet(" + getanyTypeDescription() + " " +  _soReturnObjectDescription + "){";
        }


        public String getqueryInterfaceSourceCode(String _sShortClassName, String _sReturnVariableName, String _sIncomingObjectName){
           return "\t" + _sReturnVariableName + " =  (" + _sShortClassName + ") UnoRuntime.queryInterface(" + _sShortClassName + ".class, " + _sIncomingObjectName + ");\n";
        }


        public String getPropertyValueGetterSourceCode(String _sPropertyName, String _sReturnVariableName, String _sIncomingObjectName){
           return "\t" + _sReturnVariableName + " =  " + _sIncomingObjectName + ".getPropertyValue(\"" + _sPropertyName + "\");";
        }


        public String getObjectTypeDescription(String sShortClassName){
            return sShortClassName;
        }


        public String getMethodTerminationSourceCode(){
            return "\n}";
        }

        public String getbooleanTypeDescription(){
            return  "boolean";
        }

        public String getbyteTypeDescription(){
            return "byte";
        }

        public String getshortTypeDescription(){
            return "short";
        }

        public String getunsignedshortTypeDescription(){
            return "short";
        }

        public String getlongTypeDescription(){
            return "int";
        }

        public String getunsignedlongTypeDescription(){
            return "int";
        }

        public String gethyperTypeDescription(){
            return "long";
        }

        public String getunsignedhyperTypeDescription(){
            return "long";
        }

        public String getfloatTypeDescription(){
            return "float";
        }

        public String getdoubleTypeDescription(){
            return "double";
        }

        public String getcharTypeDescription(){
            return "char";
        }

        public String getstringTypeDescription(){
            return "String";
        }

        public String gettypeTypeDescription(){
            //TODO com.sun.star.uno.Type has to be added to headerTypeDescriptions
            return "Type";
        }

        public String getanyTypeDescription(){
            return "Object";
        }


        public String getVariableDeclaration(String _sTypeString, String _sVariableName, boolean bIsArray){
            String sReturn = "";
            if (bIsArray){
                sReturn = _sTypeString + "[] " + _sVariableName;
            }
            else{
                sReturn = _sTypeString + " " + _sVariableName;
            }
            return sReturn;
        }


        public String getArrayDeclaration(String _sVariableDeclaration){
            String sReturn = "";
            String[] sDeclarations = _sVariableDeclaration.split(" ");
            for (int i = 0; i< sDeclarations.length;i++){
                sReturn += sDeclarations[i];
                if (i == 0){
                    sReturn += "[]";
                }
                if (i < (sDeclarations.length -1)){
                    sReturn += " ";
                }
            }
            return sReturn;
        }
    }
}