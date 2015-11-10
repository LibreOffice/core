/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <idlc/astoperation.hxx>
#include <idlc/asttype.hxx>
#include <idlc/astbasetype.hxx>
#include <idlc/astparameter.hxx>
#include <idlc/errorhandler.hxx>

#include <registry/writer.hxx>

void AstOperation::setExceptions(DeclList const * pExceptions)
{
    if (pExceptions != nullptr) {
        m_exceptions = *pExceptions;
    }
}

bool AstOperation::isVariadic() const {
    DeclList::const_iterator i(getIteratorEnd());
    return i != getIteratorBegin()
        && static_cast< AstParameter const * >(*(--i))->isRest();
}

bool AstOperation::dumpBlob(typereg::Writer & rBlob, sal_uInt16 index)
{
    sal_uInt16      nParam = getNodeCount(NT_parameter);
    sal_uInt16      nExcep = nExceptions();
    RTMethodMode    methodMode = RTMethodMode::TWOWAY;

    OUString returnTypeName;
    if (m_pReturnType == nullptr) {
        returnTypeName = "void";
    } else {
        returnTypeName = OStringToOUString(
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
        RTParamMode paramMode;
        sal_uInt16 paramIndex = 0;
        while ( iter != end )
        {
            AstDeclaration* pDecl = *iter;
            if ( pDecl->getNodeType() == NT_parameter )
            {
                AstParameter* pParam = static_cast<AstParameter*>(pDecl);
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

    return true;
}

AstDeclaration* AstOperation::addDeclaration(AstDeclaration* pDecl)
{
    return AstScope::addDeclaration(pDecl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
