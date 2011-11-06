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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"
#include <idlc/astoperation.hxx>
#include <idlc/asttype.hxx>
#include <idlc/astbasetype.hxx>
#include <idlc/astparameter.hxx>
#include <idlc/errorhandler.hxx>

#include "registry/writer.hxx"

using namespace ::rtl;

void AstOperation::setExceptions(DeclList const * pExceptions)
{
    if (pExceptions != 0) {
        if (isOneway()) {
            idlc()->error()->error1(EIDL_ONEWAY_RAISE_CONFLICT, this);
        }
        m_exceptions = *pExceptions;
    }
}

bool AstOperation::isVariadic() const {
    DeclList::const_iterator i(getIteratorEnd());
    return i != getIteratorBegin()
        && static_cast< AstParameter const * >(*(--i))->isRest();
}

sal_Bool AstOperation::dumpBlob(typereg::Writer & rBlob, sal_uInt16 index)
{
    sal_uInt16      nParam = getNodeCount(NT_parameter);
    sal_uInt16      nExcep = nExceptions();
    RTMethodMode    methodMode = RT_MODE_TWOWAY;

    if ( isOneway() )
        methodMode = RT_MODE_ONEWAY;

    rtl::OUString returnTypeName;
    if (m_pReturnType == 0) {
        returnTypeName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void"));
    } else {
        returnTypeName = rtl::OStringToOUString(
            m_pReturnType->getRelativName(), RTL_TEXTENCODING_UTF8);
    }
    rBlob.setMethodData(
        index, getDocumentation(), methodMode,
        OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8),
        returnTypeName, nParam, nExcep);

    if ( nParam )
    {
        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        RTParamMode paramMode;
        sal_uInt16 paramIndex = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_parameter )
            {
                AstParameter* pParam = (AstParameter*)pDecl;
                switch (pParam->getDirection())
                {
                    case DIR_IN :
                        paramMode = RT_PARAM_IN;
                        break;
                    case DIR_OUT :
                        paramMode = RT_PARAM_OUT;
                        break;
                    case DIR_INOUT :
                        paramMode = RT_PARAM_INOUT;
                        break;
                    default:
                        paramMode = RT_PARAM_INVALID;
                        break;
                }
                if (pParam->isRest()) {
                    paramMode = static_cast< RTParamMode >(
                        paramMode | RT_PARAM_REST);
                }

                rBlob.setMethodParameterData(
                    index, paramIndex++, paramMode,
                    OStringToOUString(
                        pDecl->getLocalName(), RTL_TEXTENCODING_UTF8),
                    OStringToOUString(
                        pParam->getType()->getRelativName(),
                        RTL_TEXTENCODING_UTF8));
            }
            ++iter;
        }
    }

    if ( nExcep )
    {
        DeclList::iterator iter = m_exceptions.begin();
        DeclList::iterator end = m_exceptions.end();
        sal_uInt16 exceptIndex = 0;
        while ( iter != end )
        {
            rBlob.setMethodExceptionTypeName(
                index, exceptIndex++,
                OStringToOUString(
                    (*iter)->getRelativName(), RTL_TEXTENCODING_UTF8));
            ++iter;
        }
    }

    return sal_True;
}

AstDeclaration* AstOperation::addDeclaration(AstDeclaration* pDecl)
{
    if ( pDecl->getNodeType() == NT_parameter )
    {
        AstParameter* pParam = (AstParameter*)pDecl;
        if ( isOneway() &&
             (pParam->getDirection() == DIR_OUT || pParam->getDirection() == DIR_INOUT) )
        {
            idlc()->error()->error2(EIDL_ONEWAY_CONFLICT, pDecl, this);
            return NULL;
        }
    }
    return AstScope::addDeclaration(pDecl);
}
