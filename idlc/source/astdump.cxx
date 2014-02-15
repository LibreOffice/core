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

#include <idlc/astmodule.hxx>
#include <idlc/asttypedef.hxx>
#include <idlc/astservice.hxx>
#include <idlc/astconstant.hxx>
#include <idlc/astattribute.hxx>
#include <idlc/astinterfacemember.hxx>
#include <idlc/astservicemember.hxx>
#include <idlc/astobserves.hxx>
#include <idlc/astneeds.hxx>
#include <idlc/astsequence.hxx>
#include "idlc/astoperation.hxx"

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

bool AstModule::dump(RegistryKey& rKey)
{
    OUString emptyStr;
    RegistryKey localKey;
    if ( getNodeType() == NT_root )
    {
        localKey = rKey;
    }else
    {
        if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
        {
            fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
            return false;
        }
    }

    sal_uInt16          nConst = getNodeCount(NT_const);

    if ( nConst > 0 )
    {
        RTTypeClass typeClass = RT_TYPE_MODULE;
        if ( getNodeType() == NT_constants )
            typeClass = RT_TYPE_CONSTANTS;

        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(), emptyStr, typeClass,
            m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0,
            nConst, 0, 0);

        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16 index = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_const &&
                 pDecl->isInMainfile() )
            {
                ((AstConstant*)pDecl)->dumpBlob(
                    aBlob, index++,
                    getNodeType() == NT_module && pDecl->isPublished());
            }
            ++iter;
        }

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
            return false;
        }
    } else
    {
        RTTypeClass typeClass = RT_TYPE_MODULE;
        if ( getNodeType() == NT_constants )
            typeClass = RT_TYPE_CONSTANTS;

        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(), emptyStr, typeClass, m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0, 0, 0,
            0);

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if ( getNodeType() != NT_root )
        {
            if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY,
                                    (RegValue)pBlob, aBlobSize))
            {
                fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                        idlc()->getOptions()->getProgramName().getStr(),
                        getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
                return false;
            }
        }
    }
    if ( getNodeType() == NT_root )
    {
        localKey.releaseKey();
    }
    return AstDeclaration::dump(rKey);
}

bool AstTypeDef::dump(RegistryKey& rKey)
{
    OUString emptyStr;
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    typereg::Writer aBlob(
        m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
        getDocumentation(), emptyStr, RT_TYPE_TYPEDEF, m_bPublished,
        OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 1, 0, 0, 0);
    aBlob.setSuperTypeName(
        0,
        OStringToOUString(
            getBaseType()->getRelativName(), RTL_TEXTENCODING_UTF8));

    sal_uInt32 aBlobSize;
    void const * pBlob = aBlob.getBlob(&aBlobSize);

    if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY, (RegValue)pBlob, aBlobSize))
    {
        fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    return true;
}

bool AstService::dump(RegistryKey& rKey)
{
    OUString emptyStr;
    typereg_Version version = m_bPublished
        ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0;
    OString superName;
    sal_uInt16 constructors = 0;
    sal_uInt16 properties = 0;
    sal_uInt16 references = 0;
    for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd();
         ++i)
    {
        switch ((*i)->getNodeType()) {
        case NT_interface:
        case NT_typedef:
            version = TYPEREG_VERSION_1;
            OSL_ASSERT(superName.isEmpty());
            superName = (*i)->getRelativName();
            break;

        case NT_operation:
            OSL_ASSERT(getNodeType() == NT_service);
            ++constructors;
            break;

        case NT_property:
            OSL_ASSERT(getNodeType() == NT_service);
            ++properties;
            break;

        case NT_service_member:
            if (getNodeType() == NT_singleton) {
                OSL_ASSERT(superName.isEmpty());
                superName = ((AstServiceMember *)(*i))->
                    getRealService()->getRelativName();
                break;
            }
        case NT_interface_member:
        case NT_observes:
        case NT_needs:
            OSL_ASSERT(getNodeType() == NT_service);
            ++references;
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }
    OSL_ASSERT(constructors == 0 || !m_defaultConstructor);
    if (m_defaultConstructor) {
        constructors = 1;
    }
    RegistryKey localKey;
    if (rKey.createKey(
            OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8),
            localKey)) {
        fprintf(
            stderr, "%s: warning, could not create key '%s' in '%s'\n",
            idlc()->getOptions()->getProgramName().getStr(),
            getFullName().getStr(),
            OUStringToOString(
                rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    typereg::Writer writer(
        version, getDocumentation(), emptyStr,
        getNodeType() == NT_singleton ? RT_TYPE_SINGLETON : RT_TYPE_SERVICE,
        m_bPublished,
        OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
        superName.isEmpty() ? 0 : 1, properties, constructors,
        references);
    if (!superName.isEmpty()) {
        writer.setSuperTypeName(
            0, OStringToOUString(superName, RTL_TEXTENCODING_UTF8));
    }
    sal_uInt16 constructorIndex = 0;
    sal_uInt16 propertyIndex = 0;
    sal_uInt16 referenceIndex = 0;
    for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd(); ++i)
    {
        switch ((*i)->getNodeType()) {
        case NT_operation:
            ((AstOperation *)(*i))->dumpBlob(writer, constructorIndex++);
            break;

        case NT_property:
            ((AstAttribute *)(*i))->dumpBlob(writer, propertyIndex++, 0);
            break;

        case NT_interface_member:
        {
            AstInterfaceMember * decl = (AstInterfaceMember *)(*i);
            writer.setReferenceData(
                referenceIndex++, decl->getDocumentation(), RT_REF_SUPPORTS,
                (decl->isOptional() ? RT_ACCESS_OPTIONAL : RT_ACCESS_INVALID),
                OStringToOUString( decl->getRealInterface()->getRelativName(),
                                        RTL_TEXTENCODING_UTF8));
            break;
        }

        case NT_service_member:
            if (getNodeType() == NT_service)
            {
                AstServiceMember * decl = (AstServiceMember *)(*i);
                writer.setReferenceData(referenceIndex++, decl->getDocumentation(), RT_REF_EXPORTS,
                    (decl->isOptional() ? RT_ACCESS_OPTIONAL : RT_ACCESS_INVALID),
                    OStringToOUString(decl->getRealService()->getRelativName(),
                                           RTL_TEXTENCODING_UTF8));
            }
            break;

        case NT_observes:
            {
                AstObserves * decl = (AstObserves *)(*i);
                writer.setReferenceData(referenceIndex++, decl->getDocumentation(), RT_REF_OBSERVES,
                    RT_ACCESS_INVALID,
                    OStringToOUString( decl->getRealInterface()->getRelativName(),
                                            RTL_TEXTENCODING_UTF8));
                break;
            }

        case NT_needs:
            {
                AstNeeds * decl = (AstNeeds *)(*i);
                writer.setReferenceData( referenceIndex++, decl->getDocumentation(), RT_REF_NEEDS,
                    RT_ACCESS_INVALID,
                    OStringToOUString( decl->getRealService()->getRelativName(),
                                            RTL_TEXTENCODING_UTF8));
                break;
            }

        default:
            OSL_ASSERT( (*i)->getNodeType() == NT_interface || (*i)->getNodeType() == NT_typedef);
            break;
        }
    }
    if (m_defaultConstructor) {
        writer.setMethodData(
            constructorIndex++, emptyStr, RT_MODE_TWOWAY,
            emptyStr, OUString("void"),
            0, 0);
    }
    sal_uInt32 size;
    void const * blob = writer.getBlob(&size);
    if (localKey.setValue(
            emptyStr, RG_VALUETYPE_BINARY, const_cast< void * >(blob),
            size))
    {
        fprintf(
            stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
            idlc()->getOptions()->getProgramName().getStr(),
            getFullName().getStr(),
            OUStringToOString(
                localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    return true;
}

bool AstAttribute::dumpBlob(
    typereg::Writer & rBlob, sal_uInt16 index, sal_uInt16 * methodIndex)
{
    RTFieldAccess accessMode = RT_ACCESS_INVALID;

    if (isReadonly())
    {
        accessMode |= RT_ACCESS_READONLY;
    } else
    {
        accessMode |= RT_ACCESS_READWRITE;
    }
    if (isOptional())
    {
        accessMode |= RT_ACCESS_OPTIONAL;
    }
    if (isBound())
    {
        accessMode |= RT_ACCESS_BOUND;
    }
    if (isMayBeVoid())
    {
        accessMode |= RT_ACCESS_MAYBEVOID;
    }
    if (isConstrained())
    {
        accessMode |= RT_ACCESS_CONSTRAINED;
    }
    if (isTransient())
    {
        accessMode |= RT_ACCESS_TRANSIENT;
    }
    if (isMayBeAmbiguous())
    {
        accessMode |= RT_ACCESS_MAYBEAMBIGUOUS;
    }
    if (isMayBeDefault())
    {
        accessMode |= RT_ACCESS_MAYBEDEFAULT;
    }
    if (isRemoveable())
    {
        accessMode |= RT_ACCESS_REMOVABLE;
    }

    OUString name(OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8));
    rBlob.setFieldData(
        index, getDocumentation(), OUString(), accessMode, name,
        OStringToOUString(getType()->getRelativName(), RTL_TEXTENCODING_UTF8),
        RTConstValue());
    dumpExceptions(
        rBlob, m_getDocumentation, m_getExceptions, RT_MODE_ATTRIBUTE_GET,
        methodIndex);
    dumpExceptions(
        rBlob, m_setDocumentation, m_setExceptions, RT_MODE_ATTRIBUTE_SET,
        methodIndex);

    return true;
}

void AstAttribute::dumpExceptions(
    typereg::Writer & writer, OUString const & documentation,
    DeclList const & exceptions, RTMethodMode flags, sal_uInt16 * methodIndex)
{
    if (!exceptions.empty()) {
        OSL_ASSERT(methodIndex != 0);
        sal_uInt16 idx = (*methodIndex)++;
        // exceptions.size() <= SAL_MAX_UINT16 already checked in
        // AstInterface::dump:
        writer.setMethodData(
            idx, documentation, flags,
            OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8),
            OUString("void"), 0,
            static_cast< sal_uInt16 >(exceptions.size()));
        sal_uInt16 exceptionIndex = 0;
        for (DeclList::const_iterator i(exceptions.begin());
             i != exceptions.end(); ++i)
        {
            writer.setMethodExceptionTypeName(
                idx, exceptionIndex++,
                OStringToOUString(
                    (*i)->getRelativName(), RTL_TEXTENCODING_UTF8));
        }
    }
}

const sal_Char* AstSequence::getRelativName() const
{
    if ( !m_pRelativName )
    {
        m_pRelativName = new OString("[]");
        AstDeclaration const * pType = resolveTypedefs( m_pMemberType );
        *m_pRelativName += pType->getRelativName();
    }

    return m_pRelativName->getStr();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
