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


import com.sun.star.uno.TypeClass;
import java.util.Vector;

public interface XLanguageSourceCodeGenerator {

    public static final int nJAVA = 0;
    public static final int nCPLUSPLUS = 1;
    public static final int nBASIC = 2;

    public String getHeaderSourceCode(Object _oUnoObject, String _sClassName, TypeClass _aTypeClass);

    public String getFinalHeaderStatements();

    public void assignqueryInterfaceHeaderSourceCode();

    public String getMainMethodSignatureSourceCode(XUnoNode _oUnoNode, String _soReturnObjectDescription);

    public String getMethodTerminationSourceCode();

    public String getStatementTerminationCharacter();

    public boolean needsqueryInterface();

    public String getqueryInterfaceSourceCode(String _sReturnValueDescription, String _sClassName, String _sObjectDescription);

    public String getPropertyValueGetterSourceCode(String _sPropertyName, String _sReturnVariableName, String _sIncomingObjectName, TypeClass _aTypeClass, String _sTypeName);

    public String getStructSourceCode(String _sReturnVariableDescription, String _sObjectDescription, String _sMember);

    public String getObjectTypeDescription(String _sClassName, boolean _bAsHeader);

    public String getbooleanTypeDescription();

    public String getbyteTypeDescription();

    public String getshortTypeDescription();

    public String getunsignedshortTypeDescription();

    public String getlongTypeDescription();

    public String getunsignedlongTypeDescription();

    public String gethyperTypeDescription();

    public String getunsignedhyperTypeDescription();

    public String getfloatTypeDescription();

    public String getdoubleTypeDescription();

    public String getcharTypeDescription();

    public String getstringTypeDescription(boolean _bAsHeaderSourceCode);

    public String gettypeTypeDescription(boolean _bAsHeaderSourceCode);

    public String getanyTypeDescription(boolean _bAsHeaderSourceCode);

    public String getArrayDeclaration(String sVariableDeclaration);

    public String getVariableDeclaration(String _sTypeString, String _sVariableName, boolean _bIsArray, TypeClass aTypeClass, boolean _bInitialize);

    public String getMethodSeparator();

    public String getStringValue(String _sValue);

    public String getConvertedSourceCodeValueOfObject(String _sReturnVariableName, String _sObjectDescription, TypeClass _aTypeClass, String _sTypeName);

    public String castLiteral(String _sExpression, TypeClass _aTypeClass);

    public String getCommentSign();
}
