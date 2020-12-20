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

#include <aststruct.hxx>
#include <astmember.hxx>

#include <registry/version.h>
#include <registry/writer.hxx>

AstStruct::AstStruct(
    const OString& name, std::vector< OString > const & typeParameters,
    AstStruct const* pBaseType, AstScope* pScope)
    : AstType(NT_struct, name, pScope)
    , AstScope(NT_struct)
    , m_pBaseType(pBaseType)
{
    for (auto const& elem : typeParameters)
    {
        m_typeParameters.emplace_back(
            new AstType(NT_type_parameter, elem, nullptr));
    }
}

AstStruct::AstStruct(const NodeType type,
                        const OString& name,
                       AstStruct const* pBaseType,
                     AstScope* pScope)
    : AstType(type, name, pScope)
    , AstScope(type)
    , m_pBaseType(pBaseType)
{
}

AstStruct::~AstStruct()
{
}

AstDeclaration const * AstStruct::findTypeParameter(std::string_view name)
    const
{
    for (auto const& elem : m_typeParameters)
    {
        if (elem->getLocalName() == name) {
            return elem.get();
        }
    }
    return nullptr;
}

bool AstStruct::isType() const {
    return getNodeType() == NT_struct
        ? getTypeParameterCount() == 0 : AstDeclaration::isType();
}

bool AstStruct::dump(RegistryKey& rKey)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey) != RegError::NO_ERROR)
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    if (m_typeParameters.size() > SAL_MAX_UINT16) {
        fprintf(
            stderr,
            ("%s: polymorphic struct type template %s has too many type"
             " parameters\n"),
            idlc()->getOptions()->getProgramName().getStr(),
            getScopedName().getStr());
        return false;
    }

    sal_uInt16 nMember = getNodeCount(NT_member);

    RTTypeClass typeClass = RT_TYPE_STRUCT;
    if ( getNodeType() == NT_exception )
        typeClass = RT_TYPE_EXCEPTION;

    typereg::Writer aBlob(
        (m_typeParameters.empty() && !m_bPublished
         ? TYPEREG_VERSION_0 : TYPEREG_VERSION_1),
        getDocumentation(), "", typeClass, m_bPublished,
        OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
        m_pBaseType == nullptr ? 0 : 1, nMember, 0,
        static_cast< sal_uInt16 >(m_typeParameters.size()));
    if (m_pBaseType != nullptr) {
        aBlob.setSuperTypeName(
            0,
            OStringToOUString(
                m_pBaseType->getRelativName(), RTL_TEXTENCODING_UTF8));
    }

    if ( nMember > 0 )
    {
        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstMember*  pMember = nullptr;
        sal_uInt16  index = 0;
        while ( iter != end )
        {
            AstDeclaration* pDecl = *iter;
            if ( pDecl->getNodeType() == NT_member )
            {
                pMember = static_cast<AstMember*>(pDecl);
                RTFieldAccess flags = RTFieldAccess::READWRITE;
                OString typeName;
                if (pMember->getType()->getNodeType() == NT_type_parameter) {
                    flags |= RTFieldAccess::PARAMETERIZED_TYPE;
                    typeName = pMember->getType()->getLocalName();
                } else {
                    typeName = pMember->getType()->getRelativName();
                }
                aBlob.setFieldData(
                    index++, pMember->getDocumentation(), "", flags,
                    OStringToOUString(
                        pMember->getLocalName(), RTL_TEXTENCODING_UTF8),
                    OStringToOUString(typeName, RTL_TEXTENCODING_UTF8),
                    RTConstValue());
            }
            ++iter;
        }
    }

    sal_uInt16 index = 0;
    for (auto const& elem : m_typeParameters)
    {
        aBlob.setReferenceData(
            index++, "", RTReferenceType::TYPE_PARAMETER, RTFieldAccess::INVALID,
            OStringToOUString(
                elem->getLocalName(), RTL_TEXTENCODING_UTF8));
    }

    sal_uInt32 aBlobSize;
    void const * pBlob = aBlob.getBlob(&aBlobSize);

    if (localKey.setValue("", RegValueType::BINARY,
                            const_cast<RegValue>(pBlob), aBlobSize) != RegError::NO_ERROR)
    {
        fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
