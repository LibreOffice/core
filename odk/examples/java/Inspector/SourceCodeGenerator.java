/*************************************************************************
 *
 *  $RCSfile: SourceCodeGenerator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-30 08:12:16 $
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

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
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
    private final String SUNOOBJECTNAME = "oUnobject";
    private final String SUNOSTRUCTNAME = "aUnoStruct";
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
                XUnoNode oUnoNode = _xTreepathProvider.getPathComponent(i);
                if (i == 0){
                    sVariableName = "<UNOENTRYOBJECT>";
                    sMainMethodSignature = m_xLanguageSourceCodeGenerator.getMainMethodSignatureSourceCode(oUnoNode, sVariableName);
                }
                else{
                    if (oUnoNode instanceof XUnoMethodNode){
                        XUnoMethodNode oUnoMethodNode = (XUnoMethodNode) oUnoNode;
                        if (oUnoMethodNode.isInvoked()){
                            UnoObjectDefinition oUnoReturnObjectDefinition = getUnoObjectDefinition(_xTreepathProvider, oUnoMethodNode, i);
                            if (!isVariableDeclared(oUnoReturnObjectDefinition)){
                                sStatementCode += "\n" + getMethodStatementSourceCode(oUnoMethodNode, sVariableName, oUnoReturnObjectDefinition);
                            }
                            sVariableName = oUnoReturnObjectDefinition.getVariableName();
                        }
                    }
                    else if (oUnoNode instanceof XUnoPropertyNode){
                        XUnoPropertyNode oUnoPropertyNode = (XUnoPropertyNode) oUnoNode;
                        Any oReturnObject = com.sun.star.uno.Any.complete(oUnoPropertyNode.getUnoReturnObject());
                        UnoObjectDefinition oUnoReturnObjectDefinition = new UnoObjectDefinition(oReturnObject);
                        if (!isVariableDeclared(oUnoReturnObjectDefinition)){
                            sStatementCode += "\n" + getPropertyStatementSourceCode(oUnoPropertyNode, sVariableName, oUnoReturnObjectDefinition);
                        }
                        sVariableName = oUnoReturnObjectDefinition.getVariableName();
                    }
                }
            }
        }
        String sCompleteCode = combineCompleteSourceCode();
        return sCompleteCode;
    }


    private UnoObjectDefinition getUnoObjectDefinition(XTreePathProvider _xTreePathProvider, XUnoMethodNode _oUnoMethodNode, int _nindex){
        XUnoNode oUnoNode = null;
        Object oUnoReturnObject = null;
        Object[] oParamObjects = null;
        XIdlClass xIdlClass = _oUnoMethodNode.getXIdlMethod().getReturnType();
        String sTypeName = xIdlClass.getName();
        TypeClass aTypeClass = xIdlClass.getTypeClass();
        if (aTypeClass.getValue() != TypeClass.VOID_value){
            if (_xTreePathProvider.getPathCount() > _nindex + 1){
                oUnoNode = _xTreePathProvider.getPathComponent(_nindex + 1);
                oUnoReturnObject = oUnoNode.getUnoObject();
            }
        }
        if (oUnoReturnObject == null){
            oUnoReturnObject = _oUnoMethodNode.getLastUnoReturnObject();
        }
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(oUnoReturnObject, sTypeName, aTypeClass);
        if (_oUnoMethodNode.hasParameters()){
            if (oUnoNode != null){
                oParamObjects = oUnoNode.getParameterObjects();
            }
            else{
                oParamObjects = _oUnoMethodNode.getLastParameterObjects();
            }
        }
        if (oParamObjects != null){
            oUnoObjectDefinition.addParameterObjects(oParamObjects);
        }
        return oUnoObjectDefinition;
    }


    private String combineCompleteSourceCode(){
        String sCompleteCode = "";
        sCompleteCode  += getHeaderSourceCode();
        sCompleteCode += sMainMethodSignature;
        sCompleteCode += sStatementCode;
        sCompleteCode += m_xLanguageSourceCodeGenerator.getMethodTerminationSourceCode();
        return sCompleteCode;
    }


    public String getPropertyStatementSourceCode(XUnoPropertyNode _oUnoPropertyNode, String _sVariableName, UnoObjectDefinition _oUnoReturnObjectDefinition){
        String sReturnObjectVariableDefinition = "";
        String sStatement = "";
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(_oUnoPropertyNode.getUnoObject(), "com.sun.star.beans.XPropertySet");
        if (!isVariableDeclared(oUnoObjectDefinition)){
            String sObjectVariableDefinition = getVariableDeclaration(oUnoObjectDefinition);
            String sNewVariableName = _oUnoReturnObjectDefinition.getVariableName();
            sStatement += m_xLanguageSourceCodeGenerator.getqueryInterfaceSourceCode("XPropertySet", sObjectVariableDefinition, _sVariableName);
        }
        if (_oUnoReturnObjectDefinition.getTypeClass().getValue() != TypeClass.VOID_value){
            sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition) + " = ";
        }
        sStatement += m_xLanguageSourceCodeGenerator.getPropertyValueGetterSourceCode(_oUnoPropertyNode.getProperty().Name, sReturnObjectVariableDefinition, oUnoObjectDefinition.getVariableName());
        return sStatement;
    }


    public String getMethodStatementSourceCode(XUnoMethodNode _oUnoMethodNode, String _sVariableName, UnoObjectDefinition _oUnoReturnObjectDefinition){
        String sReturnObjectVariableDefinition = "";
        String sStatement = "";
        XIdlMethod xIdlMethod = _oUnoMethodNode.getXIdlMethod();
        TypeClass aReturnTypeClass = xIdlMethod.getReturnType().getTypeClass();
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(_oUnoMethodNode.getUnoObject(), _oUnoMethodNode.getClassName());
        if (!isVariableDeclared(oUnoObjectDefinition)){
            String sObjectVariableDefinition = getVariableDeclaration(oUnoObjectDefinition);
            String sShortClassName = getShortClassName(oUnoObjectDefinition.getTypeName());
            sStatement = m_xLanguageSourceCodeGenerator.getqueryInterfaceSourceCode(sShortClassName, sObjectVariableDefinition, _sVariableName);
        }
        Object[] oParamObjects = _oUnoReturnObjectDefinition.getParameterObjects();
        String sParameterCode = getMethodParameterValueDescription(_oUnoMethodNode, oParamObjects, false);
        if (_oUnoReturnObjectDefinition.getTypeClass().getValue() != TypeClass.VOID_value){
            sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition) + " = ";
        }
        sStatement += "\t" + sReturnObjectVariableDefinition + oUnoObjectDefinition.getVariableName() + "." + xIdlMethod.getName() + "(" + sParameterCode + ");";
        return sStatement;
    }


    private String getRootDescription(XUnoNode _oUnoNode){
        return "_o" + _oUnoNode.toString();
    }


    private String getHeaderSourceCode(){
        Vector sHeaderStatements = new HeaderStatements();
        String sHeaderSourcecode = "";
        Enumeration aEnumeration = aVariables.elements();
        while(aEnumeration.hasMoreElements()){
            UnoObjectDefinition oUnoObjectDefinition = (UnoObjectDefinition) aEnumeration.nextElement();
            String sCurHeaderStatement = this.m_xLanguageSourceCodeGenerator.getHeaderSourceCode(oUnoObjectDefinition.getUnoObject(), oUnoObjectDefinition.getTypeName(), oUnoObjectDefinition.getTypeClass());
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
            Object oUnoObject = _oUnoObjectDefinition.getUnoObject();
            if (m_oIntrospector.isObjectPrimitive(oUnoObject)){
                bisDeclared = false;
            }
            else if (m_oIntrospector.isObjectSequence(oUnoObject)){
                bisDeclared = false;
            }
            else{
                String sCompVariableName = sVariableName;
                String sUnoObjectIdentity = oUnoObject.toString();
                boolean bleaveloop = false;
                int a = 2;
                while (!bleaveloop){
                    if (aVariables.containsKey(sCompVariableName)){
                        Object oUnoCompObject = ((UnoObjectDefinition) aVariables.get(sCompVariableName)).getUnoObject();
                        String sUnoCompObjectIdentity = oUnoCompObject.toString();
                        bleaveloop = sUnoCompObjectIdentity.equals(sUnoObjectIdentity);
                        bisDeclared = bleaveloop;
                        if (!bleaveloop){
                            sCompVariableName = sVariableName + SSUFFIXSEPARATOR + a++;
                        }
                    }
                    else{
                        bleaveloop = true;
                        bisDeclared = false;
                    }
                }
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


    private String getTypeString(String _sTypeName, TypeClass _aTypeClass, boolean _bAsHeaderSourceCode){
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
                sTypeString = m_xLanguageSourceCodeGenerator.getstringTypeDescription(_bAsHeaderSourceCode);
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
                //TODO consider mulitdimensional Arrays
                XTypeDescription xTypeDescription = Introspector.getIntrospector().getReferencedType(_sTypeName);
                sTypeString = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass(), _bAsHeaderSourceCode);
                break;
            case TypeClass.ENUM_value:
                System.out.println("declare Enum Variable!!!");
                break;
            case TypeClass.ANY_value:
            case TypeClass.STRUCT_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getanyTypeDescription(_bAsHeaderSourceCode);
                break;
            case TypeClass.INTERFACE_ATTRIBUTE_value:
            case TypeClass.INTERFACE_METHOD_value:
            case TypeClass.INTERFACE_value:
            case TypeClass.PROPERTY_value:
            case TypeClass.TYPE_value:
                if (_bAsHeaderSourceCode){
                    sTypeString = m_xLanguageSourceCodeGenerator.getObjectTypeDescription(_sTypeName);
                }
                else{
                    sTypeString = m_xLanguageSourceCodeGenerator.getObjectTypeDescription(getShortClassName(_sTypeName));
                }
            default:
        }
        return sTypeString;
    }


    private String getVariableDeclaration(UnoObjectDefinition _oUnoObjectDefinition, TypeClass _aTypeClass){
        boolean bIsArray = false;
        if (_oUnoObjectDefinition.getUnoObject() != null){
            bIsArray = m_oIntrospector.isObjectSequence(_oUnoObjectDefinition.getUnoObject());
        }
        else{
            bIsArray = _oUnoObjectDefinition.getTypeClass().getValue() == TypeClass.SEQUENCE_value;
        }
        String sVariableName = _oUnoObjectDefinition.getVariableName();
        String sTypeName = _oUnoObjectDefinition.getTypeName();
        String sTypeString = getTypeString(sTypeName, _aTypeClass, false);
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


class UnoObjectDefinition{
        Object m_oUnoObject = null;
        Type aType = null;
        String sVariableStemName = "";
        String sVariableName = "";
        String m_sTypeName = "";
        TypeClass m_aTypeClass = null;
        Object[] m_oParameterObjects = null;

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

        /** may return null
         */
        public Object getUnoObject(){
            return m_oUnoObject;
        }


        public void setTypeClass(TypeClass _aTypeClass){
            sVariableStemName = "";
            m_aTypeClass = _aTypeClass;
        }


        public TypeClass getTypeClass(){
            return m_aTypeClass;
        }


        public void setTypeName(String _sTypeName){
            sVariableStemName = "";
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


        public void addParameterObjects(Object[] _oParameterObjects){
            m_oParameterObjects = _oParameterObjects;
        }


        public Object[] getParameterObjects(){
            return m_oParameterObjects;
        }


        public boolean hasParameterObjects(){
            boolean breturn = false;
            if (m_oParameterObjects != null){
                breturn = m_oParameterObjects.length > 0;
            }
            return breturn;
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
                    //TODO consider mulitdimensional Arrays
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
                    sVariableStemName = SUNOOBJECTNAME;
                    break;
                case TypeClass.STRUCT_value:
                    sVariableStemName = SUNOSTRUCTNAME;
                    break;
                case TypeClass.INTERFACE_ATTRIBUTE_value:
                case TypeClass.INTERFACE_METHOD_value:
                case TypeClass.INTERFACE_value:
                case TypeClass.PROPERTY_value:
                    sVariableStemName = getVariableNameforUnoObject(getShortClassName(getTypeName()));
                default:
            }
            return sVariableStemName;
        }


        private void setVariableName(String _sVariableName){
            sVariableName = _sVariableName;
        }


        private String getVariableName() throws NullPointerException{
            if (sVariableName.equals("")){
                int a = 2;
                sVariableName = getVariableStemName();
                boolean bleaveloop = false;
                while (!bleaveloop){
                    if (aVariables.containsKey(sVariableName)){
                        String sUnoObjectIdentity = ((UnoObjectDefinition) aVariables.get(sVariableName)).getUnoObject().toString();
                        if (m_oUnoObject != null){
                            if (sUnoObjectIdentity.equals(m_oUnoObject.toString())){
                                bleaveloop = true;
                            }
                            else{
                                sVariableName = getVariableStemName() + SSUFFIXSEPARATOR + a++;
                            }
                        }
                        else{
                            bleaveloop = true;
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


    public String getMethodParameterValueDescription(XUnoMethodNode _oUnoMethodNode, Object[] _oParamObjects, boolean _bIncludeParameterNames){
        String sParamSourceCode = "";
        ParamInfo[] aParamInfos = _oUnoMethodNode.getXIdlMethod().getParameterInfos();
        if (_oParamObjects != null){
            for (int i = 0; i < _oParamObjects.length; i++){
                TypeClass aTypeClass = aParamInfos[i].aType.getTypeClass();
                if (_bIncludeParameterNames){
                    sParamSourceCode += aParamInfos[i].aName + "=";
                }
                sParamSourceCode += getStringValueOfObject(_oParamObjects[i], aTypeClass);
                if (i < _oParamObjects.length - 1){
                    sParamSourceCode += ", ";
                }
            }
        }
        return sParamSourceCode;
    }


    private class JavaCodeGenerator implements XLanguageSourceCodeGenerator{
        String sStatementsCode = "";

        public JavaCodeGenerator(){
        }

        public String getHeaderSourceCode(Object _oUnoObject, String _sClassName, TypeClass _aTypeClass){
            String sClassName = _sClassName;
            String sHeaderStatement = "";
            if (_oUnoObject != null){
                if (!m_oIntrospector.isObjectPrimitive(_oUnoObject)){
                    if (m_oIntrospector.isObjectSequence(_oUnoObject)){
                        XTypeDescription xTypeDescription = m_oIntrospector.getReferencedType(sClassName);
                        if (!m_oIntrospector.isPrimitive(xTypeDescription.getTypeClass())){
                            sClassName = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass(), true);
                        }
                        // primitive Types are not supposed to turn up in the import section...
                        else{
                            sClassName = "";
                        }
                    }
                    else{
                        sClassName = getTypeString(_sClassName, _aTypeClass, true);
                    }
                    if (!sClassName.equals("")){
                        sHeaderStatement =  "import " + sClassName + ";\n";
                    }
                }
            }
            return sHeaderStatement;
        }


        public String getMainMethodSignatureSourceCode(XUnoNode _oUnoNode, String _soReturnObjectDescription){
            //TODO try to use + _oUnoNode.getClassName() instead of the hack
            return "public void codesnippet(" + getanyTypeDescription(false) + " " +  _soReturnObjectDescription + "){";
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

        public String getstringTypeDescription(boolean _bAsHeaderSourceCode){
            if (_bAsHeaderSourceCode){
                return "";
            }
            else{
                return "String";
            }
        }

        public String gettypeTypeDescription(){
            //TODO com.sun.star.uno.Type has to be added to headerTypeDescriptions
            return "Type";
        }

        public String getanyTypeDescription(boolean _bAsHeaderSourceCode){
            if (_bAsHeaderSourceCode){
                return "";
            }
            else{
                return "Object";
            }
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