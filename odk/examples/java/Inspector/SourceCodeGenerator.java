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

import java.util.ArrayList;
import java.util.HashMap;

import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.TypeClass;



public class SourceCodeGenerator {
    private ArrayList<String> sExceptions = new ArrayList<String>();
    private ArrayList<String> sHeaderStatements = new HeaderStatements();
    private XLanguageSourceCodeGenerator m_xLanguageSourceCodeGenerator;
    private String sStatementCode = "";
    private String sMainMethodSignature = "";

    private HashMap<String, UnoObjectDefinition> aVariables = new HashMap<String, UnoObjectDefinition>();
    private final String SSUFFIXSEPARATOR = "_";
    private final String SVARIABLENAME = "VariableName";
    private Introspector m_oIntrospector;
    private ArrayList<XTreePathProvider> aTreepathProviders = new ArrayList<XTreePathProvider>();
    private boolean baddExceptionHandling = false;
    private boolean bXPropertySetExceptionsAreAdded = false;
    private XUnoNode oInitialUnoNode = null;
    private final String sINITIALVARIABLENAME = "_oUnoEntryObject";


    /** Creates a new instance of SourceCodeGenerator */
    public SourceCodeGenerator(int _nLanguage) {
        this.setLanguage(_nLanguage);
        m_oIntrospector = Introspector.getIntrospector();
    }


    public String addSourceCodeOfUnoObject(XTreePathProvider _xTreepathProvider, boolean _brememberPath, boolean _baddHeader){
        String sVariableName = "";
        if (_xTreepathProvider != null) {
            for (int i = 0; i < _xTreepathProvider.getPathCount(); i++){
                XUnoNode oUnoNode = _xTreepathProvider.getPathComponent(i);
                if (i == 0){
                    sVariableName = sINITIALVARIABLENAME;
                    oInitialUnoNode = oUnoNode;
                }
                else{
                    if (oUnoNode instanceof XUnoMethodNode){
                        XUnoMethodNode oUnoMethodNode = (XUnoMethodNode) oUnoNode;
                        if (oUnoMethodNode.isInvoked()){
                            UnoObjectDefinition oUnoReturnObjectDefinition = getUnoObjectDefinition(_xTreepathProvider, oUnoMethodNode, i);
                            if (!isVariableDeclared(oUnoReturnObjectDefinition, this.generateVariableNameFromMethod(oUnoMethodNode.getXIdlMethod()))){
                                sStatementCode += "\n" + getMethodStatementSourceCode(oUnoMethodNode, sVariableName, oUnoReturnObjectDefinition);
                            }
                            sVariableName = oUnoReturnObjectDefinition.getVariableName();
                        }
                    }
                    else if (oUnoNode instanceof XUnoPropertyNode){
                        XUnoPropertyNode oUnoPropertyNode = (XUnoPropertyNode) oUnoNode;
                        Any oReturnObject = com.sun.star.uno.Any.complete(oUnoPropertyNode.getUnoReturnObject());
                        UnoObjectDefinition oUnoReturnObjectDefinition = new UnoObjectDefinition(oReturnObject);
                        if (!isVariableDeclared(oUnoReturnObjectDefinition, oUnoPropertyNode.getProperty().Name)){
                            sStatementCode += "\n" + getPropertyStatementSourceCode(oUnoPropertyNode, sVariableName, oUnoReturnObjectDefinition);
                        }
                        sVariableName = oUnoReturnObjectDefinition.getVariableName();
                    }
                }
            }
        }
        String sCompleteCode = combineCompleteSourceCode(_baddHeader);
        if (_brememberPath){
            aTreepathProviders.add(_xTreepathProvider);
        }
        return sCompleteCode;
    }


    private void setLanguage(int _nLanguage){
        XLanguageSourceCodeGenerator xLanguageSourceCodeGenerator = null;
        switch(_nLanguage){
            case XLanguageSourceCodeGenerator.nJAVA:
                xLanguageSourceCodeGenerator = new JavaCodeGenerator();
                break;
            case XLanguageSourceCodeGenerator.nCPLUSPLUS:
                xLanguageSourceCodeGenerator = new CPlusPlusCodeGenerator();
                break;
            case XLanguageSourceCodeGenerator.nBASIC:
                xLanguageSourceCodeGenerator = new BasicCodeGenerator();
                break;
            default:
                System.out.println("Unknown Sourcecode Language. Check Menus!");
        }
        if (xLanguageSourceCodeGenerator != null){
            m_xLanguageSourceCodeGenerator = xLanguageSourceCodeGenerator;
        }
    }

    private void resetSourceCodeGeneration(int _nLanguage){
        aVariables.clear();
        this.sHeaderStatements.clear();
        setLanguage(_nLanguage);
        sStatementCode = "";
    }

    private String generateVariableNameFromMethod(String _sMethodName, String _sPrefix, boolean _bConsiderAll){
        String sReturn = "";
        if (_sMethodName.startsWith(_sPrefix)){
            int nPrefixLength = _sPrefix.length();
            if (_sMethodName.length() > nPrefixLength){
                String sChar = _sMethodName.substring(nPrefixLength, nPrefixLength + 1);
                String sUpperChar = sChar.toUpperCase();
                if (sUpperChar.equals(sChar)){
                    if (_bConsiderAll){
                        sReturn = _sMethodName;
                    }
                    else{
                        sReturn = _sMethodName.substring(nPrefixLength, _sMethodName.length());
                    }
                }
            }
        }
        return sReturn;
    }


    private String generateVariableNameFromMethod(XIdlMethod _xIdlMethod){
        // todo: refactor this!!!
        String sMethodName = _xIdlMethod.getName();
        String sReturn = "";
        sReturn = generateVariableNameFromMethod(sMethodName, "getBy", false);
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "get", false);
        }
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "attach", false);
        }
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "assign", false);
        }
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "attach", false);
        }
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "create", false);
        }
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "is", true);
        }
        if (sReturn.equals("")){
            sReturn = generateVariableNameFromMethod(sMethodName, "has", true);
        }
        if (sReturn.equals("")){
            sReturn = sMethodName;
        }
        return sReturn;
    }


    public String convertAllUnoObjects(int _nLanguage){
        String sSourceCode = "";
        resetSourceCodeGeneration(_nLanguage);
        int ncount = aTreepathProviders.size();
        for (int i=0; i< ncount; i++){
            sSourceCode = addSourceCodeOfUnoObject(aTreepathProviders.get(i), false, (i == (ncount-1)));
    }
        return sSourceCode;
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


    private String combineCompleteSourceCode(boolean _bAddHeader){
        String sCompleteCode = "";
        if (_bAddHeader){
            sMainMethodSignature = m_xLanguageSourceCodeGenerator.getMainMethodSignatureSourceCode(oInitialUnoNode, sINITIALVARIABLENAME);
            m_xLanguageSourceCodeGenerator.assignqueryInterfaceHeaderSourceCode();
            sCompleteCode += getHeaderSourceCode();
        }
        sCompleteCode += sMainMethodSignature;
        sCompleteCode += sStatementCode;
        if (_bAddHeader){
            sCompleteCode += m_xLanguageSourceCodeGenerator.getMethodTerminationSourceCode();
            sCompleteCode += "\n" + m_xLanguageSourceCodeGenerator.getCommentSign() + "...";
        }
        return sCompleteCode;
    }


    private String getPropertyStatementSourceCode(XUnoPropertyNode _oUnoPropertyNode, String _sVariableName, UnoObjectDefinition _oUnoReturnObjectDefinition){
        String sReturnObjectVariableDefinition = "";
        String sStatement = "";
        String sPropertyName = _oUnoPropertyNode.getProperty().Name;
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(_oUnoPropertyNode.getUnoObject(), "com.sun.star.beans.XPropertySet");
            if (!m_xLanguageSourceCodeGenerator.needsqueryInterface() || (oUnoObjectDefinition.getTypeClass().getValue() == TypeClass.STRUCT_value)){
                oUnoObjectDefinition.setVariableName(_sVariableName);
                if (oUnoObjectDefinition.getTypeClass().getValue() == TypeClass.STRUCT_value){
                    sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition, false);
                    sStatement += m_xLanguageSourceCodeGenerator.getStructSourceCode(sReturnObjectVariableDefinition, oUnoObjectDefinition.getVariableName(), sPropertyName);
                    return sStatement;
                }
            }
            sStatement += addQueryInterfaceSourceCode(oUnoObjectDefinition, _sVariableName, "com.sun.star.beans.XPropertySet");
            if (_oUnoReturnObjectDefinition.getTypeClass().getValue() != TypeClass.VOID_value){
                sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition, true);
            }
            sStatement += m_xLanguageSourceCodeGenerator.getPropertyValueGetterSourceCode(sPropertyName, sReturnObjectVariableDefinition, oUnoObjectDefinition.getVariableName(), _oUnoReturnObjectDefinition.getTypeClass(), _oUnoReturnObjectDefinition.getTypeName());
            addXPropertySetRelatedExceptions();
            return sStatement;
    }


    private String getMethodStatementSourceCode(XUnoMethodNode _oUnoMethodNode, String _sVariableName, UnoObjectDefinition _oUnoReturnObjectDefinition){
        String sReturnObjectVariableDefinition = "";
        String sStatement = "";
        XIdlMethod xIdlMethod = _oUnoMethodNode.getXIdlMethod();
        TypeClass aReturnTypeClass = xIdlMethod.getReturnType().getTypeClass();
        UnoObjectDefinition oUnoObjectDefinition = new UnoObjectDefinition(_oUnoMethodNode.getUnoObject(), _oUnoMethodNode.getClassName());
        String sVariableStemName = this.generateVariableNameFromMethod(xIdlMethod);
        sStatement += addQueryInterfaceSourceCode(oUnoObjectDefinition, _sVariableName, oUnoObjectDefinition.getTypeName());
        if (_oUnoReturnObjectDefinition.getTypeClass().getValue() != TypeClass.VOID_value){
            sReturnObjectVariableDefinition = getVariableInitialization(_oUnoReturnObjectDefinition, false) + " = ";
        }
        Object[] oParamObjects = _oUnoReturnObjectDefinition.getParameterObjects();
        String sParameterCode = getMethodParameterValueDescription(_oUnoMethodNode, oParamObjects, false);
        String sSeparator = m_xLanguageSourceCodeGenerator.getMethodSeparator();
        sStatement += "\t" + sReturnObjectVariableDefinition + oUnoObjectDefinition.getVariableName() + sSeparator + xIdlMethod.getName() + "(" + sParameterCode + ")";
        sStatement += m_xLanguageSourceCodeGenerator.getStatementTerminationCharacter();
        addExceptions(xIdlMethod);
        return sStatement;
    }


    private String addQueryInterfaceSourceCode(UnoObjectDefinition _oUnoObjectDefinition, String _sVariableName,  String _sTypeName){
        String sLocStatement = "";
        if (m_xLanguageSourceCodeGenerator.needsqueryInterface()){
            if (!isVariableDeclared(_oUnoObjectDefinition, "")){
                String sObjectVariableDefinition = getVariableDeclaration(_oUnoObjectDefinition, false, "");
                sLocStatement += m_xLanguageSourceCodeGenerator.getqueryInterfaceSourceCode(_sTypeName, sObjectVariableDefinition, _sVariableName);
            }
        }
        else{
            _oUnoObjectDefinition.setVariableName(_sVariableName);
        }
        return sLocStatement;
    }


    private void addXPropertySetRelatedExceptions(){
        if (!bXPropertySetExceptionsAreAdded){
            sExceptions.add("com.sun.star.beans.UnknownPropertyException");
            sExceptions.add("com.sun.star.lang.WrappedTargetException");
            sExceptions.add("com.sun.star.lang.IllegalArgumentException");
            bXPropertySetExceptionsAreAdded = true;
            baddExceptionHandling = true;
        }
    }


    private void addExceptions(XIdlMethod _xIdlMethod){
        XIdlClass[] xIdlClasses = _xIdlMethod.getExceptionTypes();
        for (int i = 0; i > xIdlClasses.length; i++){
            sExceptions.add(xIdlClasses[0].getName());
            baddExceptionHandling = true;
        }
    }

    private String getHeaderSourceCode(){
        for(UnoObjectDefinition oUnoObjectDefinition : aVariables.values()){
            String sCurHeaderStatement = m_xLanguageSourceCodeGenerator.getHeaderSourceCode(oUnoObjectDefinition.getUnoObject(), oUnoObjectDefinition.getTypeName(), oUnoObjectDefinition.getTypeClass());
            sHeaderStatements.add(sCurHeaderStatement);
        }
        String sHeaderSourcecode = "";
        String[] sHeaderStatementArray = new String[sHeaderStatements.size()];
        sHeaderStatements.toArray(sHeaderStatementArray);
        java.util.Arrays.sort(sHeaderStatementArray);
        for (int i = 0; i < sHeaderStatementArray.length; i++){
            sHeaderSourcecode += sHeaderStatementArray[i];
        }
        sHeaderSourcecode += m_xLanguageSourceCodeGenerator.getFinalHeaderStatements();
        return sHeaderSourcecode +"\n" + m_xLanguageSourceCodeGenerator.getCommentSign() + "...\n";
    }


    private class HeaderStatements extends ArrayList<String> {

        public boolean contains(String _oElement){
           String sCompName = _oElement;
           for (int i = 0; i < this.size(); i++){
               String sElement = this.get(i);
               if (sElement.equals(sCompName)){
                   return true;
               }
           }
           return false;
       }


       @Override
    public boolean add(String _oElement){
            if (!contains(_oElement)){
                super.add(_oElement);
                return true;
            }
           return false;
       }
    }


    private boolean isVariableDeclared(UnoObjectDefinition _oUnoObjectDefinition, String _sDefaultStemName){
        boolean bisDeclared = false;
        if (!_sDefaultStemName.equals("")){
            _oUnoObjectDefinition.setCentralVariableStemName(_sDefaultStemName);
        }
        String sVariableStemName = _oUnoObjectDefinition.getVariableStemName();
        bisDeclared = aVariables.containsKey(sVariableStemName);
        if (bisDeclared){
            Object oUnoObject = _oUnoObjectDefinition.getUnoObject();
            if (Introspector.isObjectPrimitive(oUnoObject)){
                bisDeclared = false;
            }
            else if (Introspector.isObjectSequence(oUnoObject)){
                bisDeclared = false;
            }
            else{
                String sCompVariableName = sVariableStemName;
                String sUnoObjectIdentity = oUnoObject.toString();
                boolean bleaveloop = false;
                int a = 2;
                while (!bleaveloop){
                    if (aVariables.containsKey(sCompVariableName)){
                        Object oUnoCompObject = aVariables.get(sCompVariableName).getUnoObject();
                        String sUnoCompObjectIdentity = oUnoCompObject.toString();
                        bleaveloop = sUnoCompObjectIdentity.equals(sUnoObjectIdentity);
                        bisDeclared = bleaveloop;
                        if (!bleaveloop){
                            sCompVariableName = sVariableStemName + SSUFFIXSEPARATOR + a++;
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
                if (xTypeDescription != null){
                    sTypeString = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass(), _bAsHeaderSourceCode);
                }
                break;
            case TypeClass.ANY_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getanyTypeDescription(_bAsHeaderSourceCode);
                break;
            case TypeClass.TYPE_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getObjectTypeDescription("com.sun.star.uno.Type", _bAsHeaderSourceCode);
                break;
            case TypeClass.ENUM_value:
            case TypeClass.STRUCT_value:
            case TypeClass.INTERFACE_ATTRIBUTE_value:
            case TypeClass.INTERFACE_METHOD_value:
            case TypeClass.INTERFACE_value:
            case TypeClass.PROPERTY_value:
                sTypeString = m_xLanguageSourceCodeGenerator.getObjectTypeDescription(_sTypeName, _bAsHeaderSourceCode);
                break;
            default:
        }
        return sTypeString;
    }


    private String getVariableDeclaration(UnoObjectDefinition _oUnoObjectDefinition, boolean _bInitialize, String _sVariableDefaultName){
        TypeClass aTypeClass = _oUnoObjectDefinition.getTypeClass();
        TypeClass aLocTypeClass = aTypeClass;
        boolean bIsArray = false;
        if (_oUnoObjectDefinition.getUnoObject() != null){
            bIsArray = Introspector.isObjectSequence(_oUnoObjectDefinition.getUnoObject());
        }
        else{
            bIsArray = _oUnoObjectDefinition.getTypeClass().getValue() == TypeClass.SEQUENCE_value;
        }
        String sVariableName = _oUnoObjectDefinition.getVariableName(_sVariableDefaultName);
        String sTypeName = _oUnoObjectDefinition.getTypeName();
        String sTypeString = getTypeString(sTypeName, aLocTypeClass, false);
        if (bIsArray){
            XTypeDescription xTypeDescription = Introspector.getIntrospector().getReferencedType(sTypeName);
            if (xTypeDescription != null){
                aLocTypeClass = xTypeDescription.getTypeClass();
            }
        }
        String sVariableDeclaration = m_xLanguageSourceCodeGenerator.getVariableDeclaration(sTypeString, sVariableName, bIsArray, aLocTypeClass, _bInitialize);
        addUniqueVariableName(sVariableName, _oUnoObjectDefinition);
        return sVariableDeclaration;
    }


    private String getVariableInitialization(UnoObjectDefinition _oUnoObjectDefinition, boolean _bInitialize){
        String sObjectVariableDeclaration = "";
        String sVariableName = _oUnoObjectDefinition.getVariableName();
        if (isVariableDeclared(_oUnoObjectDefinition, "")){
            sObjectVariableDeclaration = sVariableName;
        }
        else{
            sObjectVariableDeclaration =  getVariableDeclaration(_oUnoObjectDefinition, _bInitialize, "");
        }
        return sObjectVariableDeclaration;
    }



    public String getVariableNameforUnoObject(String _sShortClassName){
        if (_sShortClassName.startsWith("X")){
            return  "x" + _sShortClassName.substring(1);
        }
        else{
            return _sShortClassName;
        }
    }


private class UnoObjectDefinition{
        private Object m_oUnoObject = null;

        private String sVariableStemName = "";
        private String m_sCentralVariableStemName = "";
        private String sVariableName = "";
        private String m_sTypeName = "";
        private TypeClass m_aTypeClass = null;
        private Object[] m_oParameterObjects = null;


        private UnoObjectDefinition(Any _oUnoObject){
            m_sTypeName = _oUnoObject.getType().getTypeName();
            m_aTypeClass = _oUnoObject.getType().getTypeClass();
            m_oUnoObject = _oUnoObject;
            m_sCentralVariableStemName = getCentralVariableStemName(m_aTypeClass);
        }


        private UnoObjectDefinition(Object _oUnoObject, String _sTypeName, TypeClass _aTypeClass){
            m_oUnoObject = _oUnoObject;
            m_sTypeName = _sTypeName;
            m_aTypeClass = _aTypeClass;
            m_sCentralVariableStemName = getCentralVariableStemName(m_aTypeClass);
        }


        private UnoObjectDefinition(Object _oUnoObject, String _sTypeName){
            m_oUnoObject = _oUnoObject;
            m_sTypeName = _sTypeName;
            m_aTypeClass = AnyConverter.getType(_oUnoObject).getTypeClass();
            m_sCentralVariableStemName = getCentralVariableStemName(m_aTypeClass);
        }


        private String getCentralVariableStemName(TypeClass _aTypeClass){
            String sCentralVariableStemName = "";
            int nTypeClass = _aTypeClass.getValue();
            switch(nTypeClass){
                case TypeClass.SEQUENCE_value:
                    //TODO consider mulitdimensional Arrays
                    XTypeDescription xTypeDescription = Introspector.getIntrospector().getReferencedType(getTypeName());
                    if (xTypeDescription != null){
                        sCentralVariableStemName = getCentralVariableStemName(xTypeDescription.getTypeClass());
                    }
                    break;
                case TypeClass.TYPE_value:
                    sCentralVariableStemName = SVARIABLENAME;
                    break;
                case TypeClass.STRUCT_value:
                    sCentralVariableStemName = Introspector.getShortClassName(getTypeName());
                    break;
                case TypeClass.INTERFACE_ATTRIBUTE_value:
                case TypeClass.INTERFACE_METHOD_value:
                case TypeClass.INTERFACE_value:
            case TypeClass.PROPERTY_value:
                    String sShortClassName = Introspector.getShortClassName(getTypeName());
                    sCentralVariableStemName = getVariableNameforUnoObject(sShortClassName);
                default:
                    sCentralVariableStemName = SVARIABLENAME;
            }
            return sCentralVariableStemName;
        }

        /** may return null
         */
        public Object getUnoObject(){
            return m_oUnoObject;
        }


        public TypeClass getTypeClass(){
            return m_aTypeClass;
        }


        public String getTypeName(){
            return m_sTypeName;
        }


        public void setCentralVariableStemName(String _sCentralVariableStemName){
            m_sCentralVariableStemName = _sCentralVariableStemName;
        }


        public String getVariableStemName(){
            if (sVariableStemName.equals("")){
                sVariableStemName = getVariableStemName(m_aTypeClass);
            }
            return sVariableStemName;
        }


        private void addParameterObjects(Object[] _oParameterObjects){
            m_oParameterObjects = _oParameterObjects;
        }


        public Object[] getParameterObjects(){
            return m_oParameterObjects;
        }





        private String getVariableStemName(TypeClass _aTypeClass){
            int nTypeClass = _aTypeClass.getValue();
            switch(nTypeClass){
                case TypeClass.BOOLEAN_value:
                    sVariableStemName = "b" + m_sCentralVariableStemName;
                    break;
                case TypeClass.DOUBLE_value:
                case TypeClass.FLOAT_value:
                    sVariableStemName = "f" + m_sCentralVariableStemName;
                    break;
                case TypeClass.BYTE_value:
                case TypeClass.HYPER_value:
                case TypeClass.LONG_value:
                case TypeClass.UNSIGNED_HYPER_value:
                case TypeClass.UNSIGNED_LONG_value:
                case TypeClass.UNSIGNED_SHORT_value:
                case TypeClass.SHORT_value:
                    sVariableStemName = "n" + m_sCentralVariableStemName;
                    break;
                case TypeClass.CHAR_value:
                case TypeClass.STRING_value:
                    sVariableStemName = "s" + m_sCentralVariableStemName;
                    break;
                case TypeClass.SEQUENCE_value:
                    //TODO consider mulitdimensional Arrays
                    XTypeDescription xTypeDescription = Introspector.getIntrospector().getReferencedType(getTypeName());
                    if (xTypeDescription != null){
                        sVariableStemName = getVariableStemName(xTypeDescription.getTypeClass());
                    }
                    break;
                case TypeClass.TYPE_value:
                    sVariableStemName = "a" + m_sCentralVariableStemName;
                    break;
                case TypeClass.ANY_value:
                    sVariableStemName = "o" + m_sCentralVariableStemName;
                    break;
                case TypeClass.STRUCT_value:
                case TypeClass.ENUM_value:
                    sVariableStemName = "a" + m_sCentralVariableStemName;
                    break;
                case TypeClass.INTERFACE_ATTRIBUTE_value:
                case TypeClass.INTERFACE_METHOD_value:
                case TypeClass.INTERFACE_value:
                case TypeClass.PROPERTY_value:
                    String sShortClassName = Introspector.getShortClassName(getTypeName());
                    sVariableStemName = getVariableNameforUnoObject(sShortClassName);
                default:
            }
            return sVariableStemName;
        }


        private void setVariableName(String _sVariableName){
            sVariableName = _sVariableName;
        }


        private String getVariableName(String _sCentralVariableStemName){
            if (!_sCentralVariableStemName.equals("")){
                this.m_sCentralVariableStemName = _sCentralVariableStemName;
            }
            return getVariableName();
        }


        private String getVariableName() throws NullPointerException{
            if (sVariableName.equals("")){
                int a = 2;
                sVariableName = getVariableStemName();
                boolean bleaveloop = false;
                while (!bleaveloop){
                    if (aVariables.containsKey(sVariableName)){
                        String sUnoObjectIdentity = aVariables.get(sVariableName).getUnoObject().toString();
                        if (m_oUnoObject != null){
                            if ((sUnoObjectIdentity.equals(m_oUnoObject.toString()) && (!Introspector.isPrimitive(this.getTypeClass())) &&
                                (! Introspector.isObjectSequence(m_oUnoObject)))){
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
                sReturn +=  (String) _oUnoObject;
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
                System.out.println("Type " + _aTypeClass.getValue() + " not yet defined in 'getStringValueOfObject()'");
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
                String sParamDescription = getStringValueOfObject(_oParamObjects[i], aTypeClass);
                sParamDescription = this.m_xLanguageSourceCodeGenerator.castLiteral(sParamDescription, aTypeClass);
                sParamSourceCode += sParamDescription;

                if (i < _oParamObjects.length - 1){
                    sParamSourceCode += ", ";
                }
            }
        }
        return sParamSourceCode;
    }


    private class JavaCodeGenerator implements XLanguageSourceCodeGenerator{

        private boolean bAddAnyConverter = false;
        private boolean bIsPropertyUnoObjectDefined = false;

        public JavaCodeGenerator(){
        }


        public String getStatementTerminationCharacter(){
            return ";";
        }


        public String getHeaderSourceCode(Object _oUnoObject, String _sClassName, TypeClass _aTypeClass){
            String sClassName = _sClassName;
            String sHeaderStatement = "";
            if (_oUnoObject != null){
                if (!Introspector.isObjectPrimitive(_oUnoObject)){
                    if (Introspector.isObjectSequence(_oUnoObject)){
                        XTypeDescription xTypeDescription = m_oIntrospector.getReferencedType(sClassName);
                        if (xTypeDescription != null){
                            if (!Introspector.isPrimitive(xTypeDescription.getTypeClass())){
                                sClassName = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass(), true);
                            }
                            // primitive Types are not supposed to turn up in the import section...
                            else{
                                sClassName = "";
                            }
                        }
                    }
                    else{
                        sClassName = getTypeString(_sClassName, _aTypeClass, true);
                    }
                }
                else if (_aTypeClass.getValue() == TypeClass.ENUM_value){
                    sClassName = _sClassName;
                }
                else{
                    sClassName = "";
                }
                if (!sClassName.equals("")){
                    sHeaderStatement =  "import " + sClassName + ";\n";
                }
            }
            return sHeaderStatement;
        }


        public String getFinalHeaderStatements(){
            return "";
        }


        public void assignqueryInterfaceHeaderSourceCode(){
            sHeaderStatements.add("import com.sun.star.uno.UnoRuntime;\n");
            sHeaderStatements.add("import com.sun.star.uno.XInterface;\n");
            if (bAddAnyConverter){
                sHeaderStatements.add("import com.sun.star.uno.AnyConverter;\n");
            }
        }



    public String getConvertedSourceCodeValueOfObject(String _sReturnVariableName, String _sObjectDescription, TypeClass _aTypeClass, String _sTypeName){
        boolean bLocAddAnyConverter = true;
        String sReturn = "";
        switch (_aTypeClass.getValue()){
            case TypeClass.BOOLEAN_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toBoolean(" + _sObjectDescription + ")";
                break;
            case TypeClass.CHAR_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toChar(" + _sObjectDescription + ")";
                break;
            case TypeClass.BYTE_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toByte(" + _sObjectDescription + ")";
                break;
            case TypeClass.DOUBLE_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toDouble(" + _sObjectDescription + ")";
                break;
            case TypeClass.FLOAT_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toFloat(" + _sObjectDescription + ")";
                break;
            case TypeClass.UNSIGNED_HYPER_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toUnsignedLong(" + _sObjectDescription + ")";
                break;
            case TypeClass.HYPER_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toLong(" + _sObjectDescription + ")";
                break;
            case TypeClass.UNSIGNED_LONG_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toUnsignedInt(" + _sObjectDescription + ")";
                break;
            case TypeClass.LONG_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toInt(" + _sObjectDescription + ")";
                break;
            case TypeClass.SHORT_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toShort(" + _sObjectDescription + ")";
                break;
            case TypeClass.UNSIGNED_SHORT_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toUnsignedShort(" + _sObjectDescription + ")";
                break;
            case TypeClass.STRING_value:
                sReturn = _sReturnVariableName + " = AnyConverter.toString(" + _sObjectDescription + ")";
                break;
            default:
                String sShortTypeName = Introspector.getShortClassName(_sTypeName);
                if (bIsPropertyUnoObjectDefined){
                    sReturn = "oUnoObject = " + _sObjectDescription + ";\n\t";
                }
                else{
                    sReturn = "Object oUnoObject = " + _sObjectDescription + ";\n\t";
                    bIsPropertyUnoObjectDefined = true;
                }
                sReturn += _sReturnVariableName + " = (" + sShortTypeName + ") AnyConverter.toObject(" + sShortTypeName + ".class, oUnoObject);";
                break;
        }
        if (!bAddAnyConverter){
            bAddAnyConverter = bLocAddAnyConverter;
        }
        return sReturn;
    }


        public String getStructSourceCode(String _sReturnVariableDescription, String _sObjectDescription, String _sMember){
            return  "\t" + _sReturnVariableDescription + " = " + _sObjectDescription + "." + _sMember + ";";
        }

        public String getMainMethodSignatureSourceCode(XUnoNode _oUnoNode, String _soReturnObjectDescription){
            //TODO try to use + _oUnoNode.getClassName() instead of the hack
            String sReturn = "public void codesnippet(XInterface " +  _soReturnObjectDescription + "){";
            if (baddExceptionHandling){
                sReturn += "\ntry{";
            }
            return sReturn;
        }

        public String getMethodSeparator(){
            return ".";
        }

        public boolean needsqueryInterface(){
            return true;
        }

        public String getqueryInterfaceSourceCode(String _sClassName, String _sReturnVariableName, String _sIncomingObjectName){
            String sShortClassName = Introspector.getShortClassName(_sClassName);
            return "\t" + _sReturnVariableName + " =  (" + sShortClassName + ") UnoRuntime.queryInterface(" + sShortClassName + ".class, " + _sIncomingObjectName + ");\n";
        }


        public String getPropertyValueGetterSourceCode(String _sPropertyName, String _sReturnVariableName, String _sIncomingObjectName, TypeClass _aTypeClass, String _sTypeName){
            String sObjectDescription = _sIncomingObjectName + ".getPropertyValue(\"" + _sPropertyName + "\")";
            String sReturn = getConvertedSourceCodeValueOfObject(_sReturnVariableName, sObjectDescription,  _aTypeClass, _sTypeName);
            sReturn += ";";
            sReturn = "\t" + sReturn;
            return sReturn;
        }


        public String getObjectTypeDescription(String _sClassName, boolean _bAsHeader){
            String sReturn = "";
            if (_bAsHeader){
                sReturn = _sClassName;
            }
            else{
                sReturn = Introspector.getShortClassName(_sClassName);
            }
            return sReturn;
        }


        public String getMethodTerminationSourceCode(){
            String sReturn = "";
            int nIndex = 1;
            String sExceptionName = "e";
            if (baddExceptionHandling){
                for (int i = 0; i < sExceptions.size(); i++){
                    String sCurException = sExceptions.get(i);
                    if (sReturn.indexOf(sCurException) == -1){
                        if (nIndex > 1){
                            sExceptionName = "e"+ nIndex;
                        }
                        else{
                            sReturn +="\n}";
                        }
                        sReturn += "catch (" + sCurException + " " + sExceptionName + "){\n";
                        sReturn += "\t" + sExceptionName + ".printStackTrace(System.err);\n";
                        sReturn += "\t" + getCommentSign() + "Enter your Code here...\n}";
                        nIndex++;
                    }
                }
            }
            sReturn += "\n}";
            return sReturn;
        }

        public String castLiteral(String _sExpression, TypeClass _aTypeClass){
        String sReturn = "";
        switch (_aTypeClass.getValue()){
            case TypeClass.BOOLEAN_value:
                sReturn = _sExpression;
                break;
            case TypeClass.BYTE_value:
                sReturn = "(byte) " + _sExpression;
                break;
            case TypeClass.CHAR_value:
                sReturn = "'" + _sExpression + "'";
                break;
            case TypeClass.DOUBLE_value:
                sReturn = "(double) " + _sExpression;
                break;
            case TypeClass.FLOAT_value:
                sReturn = "(float) " + _sExpression;
                break;
            case TypeClass.UNSIGNED_SHORT_value:
            case TypeClass.SHORT_value:
                sReturn = "(short) " + _sExpression;
                break;
            case TypeClass.STRING_value:
                sReturn = "\"" + _sExpression + "\"";
                break;
            case TypeClass.HYPER_value:
            case TypeClass.UNSIGNED_HYPER_value:
                sReturn = "(long) " + _sExpression;
                break;
            case TypeClass.LONG_value:
                sReturn = _sExpression;
                break;
            case TypeClass.ENUM_value:
            default:
                sReturn = _sExpression;
                System.out.println("Type " + _aTypeClass.getValue() + " not yet defined in 'castliteral()'");
        }
        return sReturn;
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

        public String getanyTypeDescription(boolean _bAsHeaderSourceCode){
            if (_bAsHeaderSourceCode){
                return "";
            }
            else{
                return "Object";
            }
        }


        public String getStringValue(String _sValue){
            return _sValue;
        }


        public String getVariableDeclaration(String _sTypeString, String _sVariableName, boolean _bIsArray, TypeClass _aTypeClass, boolean _bInitialize){
            String sReturn = "";
            if (_bIsArray){
                sReturn = _sTypeString + "[] " + _sVariableName;
            }
            else{
                sReturn = _sTypeString + " " + _sVariableName;
            }
            return sReturn;
        }


        public String getCommentSign(){
            return "//";
        }
    }


    private class BasicCodeGenerator implements XLanguageSourceCodeGenerator{


        public BasicCodeGenerator(){
        }

        public String getHeaderSourceCode(Object _oUnoObject, String _sClassName, TypeClass _aTypeClass){
            String sHeaderStatement = "";
            return sHeaderStatement;
        }

        public String getFinalHeaderStatements(){
            return "";
        }

        public String getMainMethodSignatureSourceCode(XUnoNode _oUnoNode, String _soReturnObjectDescription){
            //TODO try to use + _oUnoNode.getClassName() instead of the hack
            return "Sub Main(" +  _soReturnObjectDescription +  " as " +  getanyTypeDescription(false) + ")";
        }

        public boolean needsqueryInterface(){
            return false;
        }

        public void assignqueryInterfaceHeaderSourceCode(){
        }

        public String getMethodSeparator(){
            return ".";
        }


        public String getqueryInterfaceSourceCode(String _sClassName, String _sReturnVariableName, String _sIncomingObjectName){
            return _sIncomingObjectName;
        }


        public String getPropertyValueGetterSourceCode(String _sPropertyName, String _sReturnVariableName, String _sIncomingObjectName, TypeClass _aTypeClass, String _sTypeName){
           return "\t" + _sReturnVariableName + " =  " + _sIncomingObjectName + "."  + _sPropertyName;
        }


        public String getStructSourceCode(String _sReturnVariableDescription, String _sObjectDescription, String _sMember){
            return getPropertyValueGetterSourceCode(_sMember, _sReturnVariableDescription, _sObjectDescription, null, "" );
        }

        public String getConvertedSourceCodeValueOfObject(String _sReturnVariableName, String _sObjectDescription, TypeClass _aTypeClass, String _sTypeName){
            return _sReturnVariableName + " = " + _sObjectDescription;
        }


        public String getObjectTypeDescription(String _sClassName, boolean _bAsHeader){
            return "Object";
        }


        public String getMethodTerminationSourceCode(){
            return "\nEnd Sub\n";
        }


        public String castLiteral(String _sExpression, TypeClass _aTypeClass){
        String sReturn = "";
        switch (_aTypeClass.getValue()){
            case TypeClass.BOOLEAN_value:
            case TypeClass.BYTE_value:
            case TypeClass.DOUBLE_value:
            case TypeClass.FLOAT_value:
            case TypeClass.UNSIGNED_SHORT_value:
            case TypeClass.SHORT_value:
            case TypeClass.LONG_value:
            case TypeClass.UNSIGNED_LONG_value:
            case TypeClass.HYPER_value:
            case TypeClass.UNSIGNED_HYPER_value:
                sReturn = _sExpression;
                break;
            case TypeClass.CHAR_value:
            case TypeClass.STRING_value:
                sReturn =  "\"" +_sExpression + "\"";
                break;
            case TypeClass.ENUM_value:
            default:
                sReturn = _sExpression;
                System.out.println("Type " + _aTypeClass.getValue() + " not yet defined in 'castliteral()'");
        }
        return sReturn;
    }



        public String getbooleanTypeDescription(){
            return  "Boolean";
        }

        public String getbyteTypeDescription(){
            return "Integer";
        }

        public String getshortTypeDescription(){
            return "Integer";
        }

        public String getlongTypeDescription(){
            return "Integer";
        }

        public String getunsignedlongTypeDescription(){
            return "Long";
        }

        public String gethyperTypeDescription(){
            return "Long";
        }

        public String getunsignedhyperTypeDescription(){
            return "Long";
        }

        public String getfloatTypeDescription(){
            return "Double";
        }

        public String getdoubleTypeDescription(){
            return "Double";
        }

        public String getcharTypeDescription(){
            return "String";
        }

        public String getstringTypeDescription(boolean _bAsHeaderSourceCode){
            if (_bAsHeaderSourceCode){
                return "";
            }
            else{
                return "String";
            }
        }

        public String getanyTypeDescription(boolean _bAsHeaderSourceCode){
            if (_bAsHeaderSourceCode){
                return "";
            }
            else{
                return "Object";
            }
        }

        public String getStatementTerminationCharacter(){
            return "";
        }


        public String getVariableDeclaration(String _sTypeString, String _sVariableName, boolean bIsArray, TypeClass _aTypeClass, boolean _bInitialize){
            String sReturn = "";
            if (bIsArray){
                sReturn = "Dim " + _sVariableName + "() as " + _sTypeString + "\n\t" + _sVariableName;
            }
            else{
                sReturn = "Dim " + _sVariableName + " as " + _sTypeString + "\n\t" + _sVariableName;
            }
            return sReturn;
        }


        public String getStringValue(String _sValue){
            return _sValue;
        }


        public String getCommentSign(){
            return "'";
        }

    }

    private class CPlusPlusCodeGenerator implements XLanguageSourceCodeGenerator{

        private boolean bIncludeStringHeader = false;
        private boolean bIncludeAny = false;
        private boolean bIncludeSequenceHeader = false;

        public CPlusPlusCodeGenerator(){
        }

        private String getCSSNameSpaceString(){
            return "css";
        }

        public String getStatementTerminationCharacter(){
            return ";";
        }


        public String getHeaderSourceCode(Object _oUnoObject, String _sClassName, TypeClass _aTypeClass){
            String sClassName = _sClassName;
            String sHeaderStatement = "";
            if (_oUnoObject != null){
                if (!Introspector.isObjectPrimitive(_oUnoObject)){
                    if (Introspector.isObjectSequence(_oUnoObject)){
                        XTypeDescription xTypeDescription = m_oIntrospector.getReferencedType(sClassName);
                        if (xTypeDescription != null){
                            if (!Introspector.isPrimitive(xTypeDescription.getTypeClass())){
                                sClassName = getTypeString(xTypeDescription.getName(), xTypeDescription.getTypeClass(), true);
                            }
                            // primitive Types are not supposed to turn up in the import section...
                            else{
                                sClassName = "";
                            }
                        }
                    }
                    else{
                        sClassName = getTypeString(_sClassName, _aTypeClass, true);
                    }
                    if (!sClassName.equals("")){
                        sHeaderStatement =  getHeaderOfClass(sClassName);
                    }
                }
            }
            return sHeaderStatement;
        }



        public String getFinalHeaderStatements(){
            String sReturn = "";
            sReturn += "\nnamespace " + getCSSNameSpaceString() + " = com::sun::star;\n";
            sReturn += "using ::rtl::OUString;\n";
            return sReturn;
        }


        private String getHeaderOfClass(String _sClassName){
            return "#include \"" + _sClassName.replace('.', '/') + ".hpp\"\n";
        }



        public void assignqueryInterfaceHeaderSourceCode(){
            sHeaderStatements.add("#include \"sal/config.h\"\n");
            sHeaderStatements.add("#include \"sal/types.h\"\n");
            if (bIncludeStringHeader){
                sHeaderStatements.add("#include \"rtl/ustring.hxx\"\n");
            }
            sHeaderStatements.add("#include \"com/sun/star/uno/Reference.hxx\"\n");
            if (bIncludeSequenceHeader){
                sHeaderStatements.add("#include \"com/sun/star/uno/Sequence.hxx\"\n");
            }
            sHeaderStatements.add(getHeaderOfClass("com.sun.star.uno.XInterface"));
            if (bIncludeAny){
                sHeaderStatements.add(getHeaderOfClass("com.sun.star.uno.Any"));
            }
        }


        public String getMainMethodSignatureSourceCode(XUnoNode _oUnoNode, String _soReturnObjectDescription){
            String sReturn = "";
            sReturn = "void codesnippet(const " + getCSSNameSpaceString() + "::uno::Reference<" + getCSSNameSpaceString() + "::uno::XInterface>& "  +  _soReturnObjectDescription + " ){";
            int a = 0;
            if (!sExceptions.contains("com.sun.star.uno.RuntimeException")){
                sExceptions.add("com.sun.star.uno.RuntimeException");
            }
            if (baddExceptionHandling){
                sReturn += "\n//throw ";
                for (int i = 0; i < sExceptions.size(); i++){
                    String sCurException = sExceptions.get(i);
                    if (sReturn.indexOf(sCurException) == -1){
                        if (a++ > 0){
                            sReturn += ", ";
                        }
                        sReturn += getObjectTypeDescription(sCurException, false);

                    }
                }

            }
            sReturn += "{";
            return sReturn;
        }


        public boolean needsqueryInterface(){
            return true;
        }


        public String getqueryInterfaceSourceCode(String _sClassName, String _sReturnVariableName, String _sIncomingObjectName){
            return "\t" + _sReturnVariableName + "( " + _sIncomingObjectName + ", " + getCSSNameSpaceString() + "::uno::UNO_QUERY_THROW);\n";
        }


        public String getPropertyValueGetterSourceCode(String _sPropertyName, String _sReturnVariableName, String _sIncomingObjectName, TypeClass _aTypeClass, String _sTypeName){
            String sFirstLine = "\t";
            String sReturnVariableName = _sReturnVariableName;
            // e.g. uno::Any a = xPropSet->getPropertyValue( rtl::OUString( "DefaultContext" ) );
            String[] sVarDefinition = _sReturnVariableName.split("=");
            if (sVarDefinition.length > 0){
                String sVariable = sVarDefinition[0];
                String[] sVarDeclaration = sVariable.split(" ");
                if (sVarDeclaration.length > 0){
                    sFirstLine += sReturnVariableName + ";\n";
                    sReturnVariableName = sVarDeclaration[sVarDeclaration.length-1];
                }
            }
            String sObjectDescription = _sIncomingObjectName + "->getPropertyValue(" + getStringValue(_sPropertyName) + ")";
            String sSecondLine = "\t" + getConvertedSourceCodeValueOfObject(sReturnVariableName, sObjectDescription,  _aTypeClass, _sTypeName) + ";";
            return sFirstLine + sSecondLine;
        }


        public String getStructSourceCode(String _sReturnVariableDescription, String _sObjectDescription, String _sMember){
            return "\t" + _sReturnVariableDescription + " = " + _sObjectDescription + "->" + _sMember + ";";
        }


        public String getConvertedSourceCodeValueOfObject(String _sReturnVariableName, String _sObjectDescription, TypeClass _aTypeClass, String _sTypeName){
            return _sObjectDescription + " >>= " + _sReturnVariableName;
        }


        public String getStringValue(String _sValue){
            bIncludeStringHeader = true;
            return "OUString(\"" + _sValue + "\")";
        }


        public String getObjectTypeDescription(String _sClassName, boolean _bAsHeader){
            String sReturn = "";
            if (_bAsHeader){
                sReturn = _sClassName.replace('.', '/');
            }
            else{
                String sModuleName = Introspector.getModuleName(_sClassName);
                sModuleName = Introspector.getShortClassName(sModuleName);
                sReturn = getCSSNameSpaceString() + "::" + sModuleName + "::" + Introspector.getShortClassName(_sClassName);
            }
            return sReturn;
        }


        public String getMethodTerminationSourceCode(){
            return "\n}";
        }

        public String getMethodSeparator(){
            return "->";
        }


        public String castLiteral(String _sExpression, TypeClass _aTypeClass){
        String sReturn = "";
        switch (_aTypeClass.getValue()){
            case TypeClass.BOOLEAN_value:
            case TypeClass.BYTE_value:
            case TypeClass.DOUBLE_value:
            case TypeClass.FLOAT_value:
            case TypeClass.UNSIGNED_SHORT_value:
            case TypeClass.SHORT_value:
            case TypeClass.LONG_value:
            case TypeClass.UNSIGNED_LONG_value:
            case TypeClass.HYPER_value:
            case TypeClass.UNSIGNED_HYPER_value:
                sReturn = _sExpression;
                break;
            case TypeClass.CHAR_value:
                sReturn = "'" + _sExpression + "'";
                break;
            case TypeClass.STRING_value:
                sReturn = getStringValue(_sExpression);
                break;
            case TypeClass.ENUM_value:
            default:
                sReturn = _sExpression;
                System.out.println("Type " + _aTypeClass.getValue() + " not yet defined in 'castliteral()'");
        }
        return sReturn;
    }

        public String getbooleanTypeDescription(){
            return  "sal_Bool";
        }

        public String getbyteTypeDescription(){
            return "sal_Int8";
        }

        public String getshortTypeDescription(){
            return "sal_Int16";
        }

        public String getlongTypeDescription(){
            return "sal_Int32";
        }

        public String getunsignedlongTypeDescription(){
            return "sal_uInt32";
        }

        public String gethyperTypeDescription(){
            return "sal_Int64";
        }

        public String getunsignedhyperTypeDescription(){
            return "sal_uInt64";
        }

        public String getfloatTypeDescription(){
            return "float";
        }

        public String getdoubleTypeDescription(){
            return "double";
        }

        public String getcharTypeDescription(){
            return "sal_Unicode";
        }

        public String getstringTypeDescription(boolean _bAsHeaderSourceCode){
            bIncludeStringHeader = true;
            if (_bAsHeaderSourceCode){
                return "";
            }
            else{
                return "OUString";
            }
        }

        public String getanyTypeDescription(boolean _bAsHeaderSourceCode){
            if (_bAsHeaderSourceCode){
                    return "com/sun/star/uno/XInterface";
            }
            else{
                return "XInterface";
            }
        }


        public String getVariableDeclaration(String _sTypeString, String _sVariableName, boolean bIsArray, TypeClass _aTypeClass, boolean _bInitialize){
            boolean bIsPrimitive = Introspector.isPrimitive(_aTypeClass);

            String sReturn = "";
            if (bIsArray){
                bIncludeSequenceHeader = true;
                sReturn = getCSSNameSpaceString() + "::uno::Sequence<" + _sTypeString + "> " + _sVariableName;
            }
            else{
                if (bIsPrimitive){
                    sReturn = _sTypeString + " " + _sVariableName;
                    if (_bInitialize){
                        switch (_aTypeClass.getValue()){
                            case TypeClass.BOOLEAN_value:
                                sReturn = sReturn + " = false";
                                break;
                            case TypeClass.BYTE_value:
                            case TypeClass.UNSIGNED_SHORT_value:
                            case TypeClass.SHORT_value:
                            case TypeClass.LONG_value:
                            case TypeClass.UNSIGNED_LONG_value:
                            case TypeClass.HYPER_value:
                            case TypeClass.UNSIGNED_HYPER_value:
                                sReturn = sReturn + " = 0";
                                break;
                            case TypeClass.DOUBLE_value:
                            case TypeClass.FLOAT_value:
                                sReturn = sReturn + " = 0.0";
                                break;
                            case TypeClass.CHAR_value:
                                sReturn = sReturn + "'0'";
                                break;
                            case TypeClass.STRING_value:
                                sReturn = _sTypeString + " " + _sVariableName;
                                break;
                            default:
                                sReturn = _sTypeString + " " + _sVariableName;
                                System.out.println("Type " + _aTypeClass.getValue() + " not yet defined in 'getVariableDeclaration()'");
                        }
                    }
                }
                else{
                    sReturn = getCSSNameSpaceString() + "::uno::Reference<" + _sTypeString + "> "  +_sVariableName;
                }
            }
            return sReturn;
        }

        public String getCommentSign(){
            return "//";
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
