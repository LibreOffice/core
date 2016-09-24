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
import com.sun.star.uno.TypeClass;

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



    public String getlongTypeDescription();

    public String getunsignedlongTypeDescription();

    public String gethyperTypeDescription();

    public String getunsignedhyperTypeDescription();

    public String getfloatTypeDescription();

    public String getdoubleTypeDescription();

    public String getcharTypeDescription();

    public String getstringTypeDescription(boolean _bAsHeaderSourceCode);



    public String getanyTypeDescription(boolean _bAsHeaderSourceCode);



    public String getVariableDeclaration(String _sTypeString, String _sVariableName, boolean _bIsArray, TypeClass aTypeClass, boolean _bInitialize);

    public String getMethodSeparator();

    public String getStringValue(String _sValue);

    public String getConvertedSourceCodeValueOfObject(String _sReturnVariableName, String _sObjectDescription, TypeClass _aTypeClass, String _sTypeName);

    public String castLiteral(String _sExpression, TypeClass _aTypeClass);

    public String getCommentSign();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
