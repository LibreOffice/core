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

#include <idlc/astinterface.hxx>
#include <idlc/astattribute.hxx>
#include <idlc/astoperation.hxx>
#include "idlc/idlc.hxx"

#include <registry/version.h>
#include <registry/writer.hxx>

#include <osl/diagnose.h>

AstInterface::AstInterface(const OString& name,
                           AstInterface const * pInherits,
                           AstScope* pScope)
    : AstType(NT_interface, name, pScope)
    , AstScope(NT_interface)
    , m_mandatoryInterfaces(0)
    , m_bIsDefined(false)
    , m_bSingleInheritance(pInherits != nullptr)
{
    if (pInherits != nullptr) {
        addInheritedInterface(pInherits, false, OUString());
    }
}

AstInterface::~AstInterface()
{
}

AstInterface::DoubleDeclarations AstInterface::checkInheritedInterfaceClashes(
    AstInterface const * ifc, bool optional) const
{
    DoubleDeclarations doubleDecls;
    std::set< OString > seen;
    checkInheritedInterfaceClashes(
        doubleDecls, seen, ifc, true, optional, optional);
    return doubleDecls;
}

void AstInterface::addInheritedInterface(
    AstType const * ifc, bool optional, OUString const & documentation)
{
    m_inheritedInterfaces.push_back(
        InheritedInterface(ifc, optional, documentation));
    if (!optional) {
        ++m_mandatoryInterfaces;
    }
    AstInterface const * resolved = resolveInterfaceTypedefs(ifc);
    addVisibleInterface(resolved, true, optional);
    if (optional) {
        addOptionalVisibleMembers(resolved);
    }
}

AstInterface::DoubleMemberDeclarations AstInterface::checkMemberClashes(
    AstDeclaration const * member) const
{
    DoubleMemberDeclarations doubleMembers;
    checkMemberClashes(doubleMembers, member, true);
    return doubleMembers;
}

void AstInterface::addMember(AstDeclaration /*TODO: const*/ * member) {
    addDeclaration(member);
    m_visibleMembers.insert(
        VisibleMembers::value_type(
            member->getLocalName(), VisibleMember(member)));
}

void AstInterface::forwardDefined(AstInterface const & def)
{
    setImported(def.isImported());
    setInMainfile(def.isInMainfile());
    setLineNumber(def.getLineNumber());
    setFileName(def.getFileName());
    setDocumentation(def.getDocumentation());
    m_inheritedInterfaces = def.m_inheritedInterfaces;
    m_mandatoryInterfaces = def.m_mandatoryInterfaces;
    m_bIsDefined = true;
}

bool AstInterface::dump(RegistryKey& rKey)
{
    if ( !isDefined() )
        return true;

    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey) != RegError::NO_ERROR)
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    if (m_mandatoryInterfaces > SAL_MAX_UINT16
        || m_inheritedInterfaces.size() - m_mandatoryInterfaces
            > SAL_MAX_UINT16)
    {
        fprintf(
            stderr, "%s: interface %s has too many direct base interfaces\n",
            idlc()->getOptions()->getProgramName().getStr(),
            getScopedName().getStr());
        return false;
    }
    sal_uInt16 nBaseTypes = static_cast< sal_uInt16 >(m_mandatoryInterfaces);
    sal_uInt16 nAttributes = 0;
    sal_uInt16 nMethods = 0;
    sal_uInt16 nReferences = static_cast< sal_uInt16 >(
        m_inheritedInterfaces.size() - m_mandatoryInterfaces);
    typereg_Version version
        = (nBaseTypes <= 1 && nReferences == 0 && !m_bPublished
           ? TYPEREG_VERSION_0 : TYPEREG_VERSION_1);
    for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd();
          ++i)
    {
        switch ((*i)->getNodeType()) {
        case NT_attribute:
            {
                if (!increment(&nAttributes, "attributes")) {
                    return false;
                }
                AstAttribute * attr = static_cast<AstAttribute *>(*i);
                if (attr->isBound()) {
                    version = TYPEREG_VERSION_1;
                }
                DeclList::size_type getCount = attr->getGetExceptionCount();
                if (getCount > SAL_MAX_UINT16) {
                    fprintf(
                        stderr,
                        ("%s: raises clause of getter for attribute %s of"
                         " interface %s is too long\n"),
                        idlc()->getOptions()->getProgramName().getStr(),
                        (*i)->getLocalName().getStr(),
                        getScopedName().getStr());
                    return false;
                }
                if (getCount > 0) {
                    version = TYPEREG_VERSION_1;
                    if (!increment(&nMethods, "attributes")) {
                        return false;
                    }
                }
                DeclList::size_type setCount = attr->getSetExceptionCount();
                if (setCount > SAL_MAX_UINT16) {
                    fprintf(
                        stderr,
                        ("%s: raises clause of setter for attribute %s of"
                         " interface %s is too long\n"),
                        idlc()->getOptions()->getProgramName().getStr(),
                        (*i)->getLocalName().getStr(),
                        getScopedName().getStr());
                    return false;
                }
                if (setCount > 0) {
                    version = TYPEREG_VERSION_1;
                    if (!increment(&nMethods, "attributes")) {
                        return false;
                    }
                }
                break;
            }

        case NT_operation:
            if (!increment(&nMethods, "methods")) {
                return false;
            }
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }

    OUString emptyStr;
    typereg::Writer aBlob(
        version, getDocumentation(), emptyStr, RT_TYPE_INTERFACE, m_bPublished,
        OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), nBaseTypes,
        nAttributes, nMethods, nReferences);

    sal_uInt16 superTypeIndex = 0;
    sal_uInt16 referenceIndex = 0;
    for (InheritedInterfaces::iterator i = m_inheritedInterfaces.begin();
          i != m_inheritedInterfaces.end(); ++i)
    {
        if (i->isOptional()) {
            aBlob.setReferenceData(
                referenceIndex++, i->getDocumentation(), RTReferenceType::SUPPORTS,
                RTFieldAccess::OPTIONAL,
                OStringToOUString(
                    i->getInterface()->getRelativName(),
                    RTL_TEXTENCODING_UTF8));
        } else {
            aBlob.setSuperTypeName(
                superTypeIndex++,
                OStringToOUString(
                    i->getInterface()->getRelativName(),
                    RTL_TEXTENCODING_UTF8));
        }
    }

    sal_uInt16 attributeIndex = 0;
    sal_uInt16 methodIndex = 0;
    for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd();
          ++i)
    {
        switch ((*i)->getNodeType()) {
        case NT_attribute:
            static_cast<AstAttribute *>(*i)->dumpBlob(
                aBlob, attributeIndex++, &methodIndex);
            break;

        case NT_operation:
            static_cast<AstOperation *>(*i)->dumpBlob(aBlob, methodIndex++);
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }

    sal_uInt32 aBlobSize;
    void const * pBlob = aBlob.getBlob(&aBlobSize);

    if (localKey.setValue(emptyStr, RegValueType::BINARY, const_cast<RegValue>(pBlob), aBlobSize) != RegError::NO_ERROR)
    {
        fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }

    return true;
}

void AstInterface::checkInheritedInterfaceClashes(
    DoubleDeclarations & doubleDeclarations,
    std::set< OString > & seenInterfaces, AstInterface const * ifc,
    bool direct, bool optional, bool mainOptional) const
{
    if (direct || optional
        || seenInterfaces.insert(ifc->getScopedName()).second)
    {
        VisibleInterfaces::const_iterator visible(
            m_visibleInterfaces.find(ifc->getScopedName()));
        if (visible != m_visibleInterfaces.end()) {
            switch (visible->second) {
            case INTERFACE_INDIRECT_OPTIONAL:
                if (direct && optional) {
                    doubleDeclarations.interfaces.push_back(ifc);
                    return;
                }
                break;

            case INTERFACE_DIRECT_OPTIONAL:
                if (direct || !mainOptional) {
                    doubleDeclarations.interfaces.push_back(ifc);
                }
                return;

            case INTERFACE_INDIRECT_MANDATORY:
                if (direct) {
                    doubleDeclarations.interfaces.push_back(ifc);
                }
                return;

            case INTERFACE_DIRECT_MANDATORY:
                if (direct || (!optional && !mainOptional)) {
                    doubleDeclarations.interfaces.push_back(ifc);
                }
                return;
            }
        }
        if (direct || !optional) {
            for (DeclList::const_iterator i(ifc->getIteratorBegin());
                  i != ifc->getIteratorEnd(); ++i)
            {
                checkMemberClashes(
                    doubleDeclarations.members, *i, !mainOptional);
            }
            for (InheritedInterfaces::const_iterator i(
                      ifc->m_inheritedInterfaces.begin());
                  i != ifc->m_inheritedInterfaces.end(); ++i)
            {
                checkInheritedInterfaceClashes(
                    doubleDeclarations, seenInterfaces, i->getResolved(),
                    false, i->isOptional(), mainOptional);
            }
        }
    }
}

void AstInterface::checkMemberClashes(
    DoubleMemberDeclarations & doubleMembers, AstDeclaration const * member,
    bool checkOptional) const
{
    VisibleMembers::const_iterator i(
        m_visibleMembers.find(member->getLocalName()));
    if (i != m_visibleMembers.end()) {
        if (i->second.mandatory != nullptr) {
            if (i->second.mandatory->getScopedName() != member->getScopedName())
            {
                DoubleMemberDeclaration d;
                d.first = i->second.mandatory;
                d.second = member;
                doubleMembers.push_back(d);
            }
        } else if (checkOptional) {
            for (VisibleMember::Optionals::const_iterator j(
                     i->second.optionals.begin());
                 j != i->second.optionals.end(); ++j)
            {
                if (j->second->getScopedName() != member->getScopedName()) {
                    DoubleMemberDeclaration d;
                    d.first = j->second;
                    d.second = member;
                    doubleMembers.push_back(d);
                }
            }
        }
    }
}

void AstInterface::addVisibleInterface(
    AstInterface const * ifc, bool direct, bool optional)
{
    InterfaceKind kind = optional
        ? direct ? INTERFACE_DIRECT_OPTIONAL : INTERFACE_INDIRECT_OPTIONAL
        : direct ? INTERFACE_DIRECT_MANDATORY : INTERFACE_INDIRECT_MANDATORY;
    std::pair< VisibleInterfaces::iterator, bool > result(
        m_visibleInterfaces.insert(
            VisibleInterfaces::value_type(ifc->getScopedName(), kind)));
    bool seen = !result.second
        && result.first->second >= INTERFACE_INDIRECT_MANDATORY;
    if (!result.second && kind > result.first->second) {
        result.first->second = kind;
    }
    if (!optional && !seen) {
        for (DeclList::const_iterator i(ifc->getIteratorBegin());
              i != ifc->getIteratorEnd(); ++i)
        {
            m_visibleMembers.insert(
                VisibleMembers::value_type(
                    (*i)->getLocalName(), VisibleMember(*i)));
        }
        for (InheritedInterfaces::const_iterator i(
                  ifc->m_inheritedInterfaces.begin());
              i != ifc->m_inheritedInterfaces.end(); ++i)
        {
            addVisibleInterface(i->getResolved(), false, i->isOptional());
        }
    }
}

void AstInterface::addOptionalVisibleMembers(AstInterface const * ifc) {
    for (DeclList::const_iterator i(ifc->getIteratorBegin());
          i != ifc->getIteratorEnd(); ++i)
    {
        VisibleMembers::iterator visible(
            m_visibleMembers.find((*i)->getLocalName()));
        if (visible == m_visibleMembers.end()) {
            visible = m_visibleMembers.insert(
                VisibleMembers::value_type(
                    (*i)->getLocalName(), VisibleMember())).first;
        }
        if (visible->second.mandatory == nullptr) {
            visible->second.optionals.insert(
                VisibleMember::Optionals::value_type(ifc->getScopedName(), *i));
        }
    }
    for (InheritedInterfaces::const_iterator i(
              ifc->m_inheritedInterfaces.begin());
          i != ifc->m_inheritedInterfaces.end(); ++i)
    {
        if (!i->isOptional()) {
            addOptionalVisibleMembers(i->getResolved());
        }
    }
}

bool AstInterface::increment(sal_uInt16 * counter, char const * sort) const {
    if (*counter == SAL_MAX_UINT16) {
        fprintf(
            stderr, "%s: interface %s has too many direct %s\n",
            idlc()->getOptions()->getProgramName().getStr(),
            getScopedName().getStr(), sort);
        return false;
    }
    ++*counter;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
