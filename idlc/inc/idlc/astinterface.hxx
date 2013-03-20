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
#pragma once
#if 1

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>
#include "idlc/inheritedinterface.hxx"

#include <map>
#include <vector>

class AstInterface : public AstType
                   , public AstScope
{
public:
    typedef std::vector< InheritedInterface > InheritedInterfaces;

    typedef std::vector< AstInterface const * > DoubleInterfaceDeclarations;

    struct DoubleMemberDeclaration {
        AstDeclaration const * first;
        AstDeclaration const * second;
    };

    typedef std::vector< DoubleMemberDeclaration > DoubleMemberDeclarations;

    struct DoubleDeclarations {
        DoubleInterfaceDeclarations interfaces;
        DoubleMemberDeclarations members;
    };

    AstInterface(
        const ::rtl::OString& name, AstInterface const * pInherits,
        AstScope* pScope);
    virtual ~AstInterface();

    InheritedInterfaces const & getAllInheritedInterfaces() const
    { return m_inheritedInterfaces; }

    bool hasMandatoryInheritedInterfaces() const
    { return m_mandatoryInterfaces > 0; }

    void setForwarded(sal_Bool bForwarded)
        { m_bForwarded = bForwarded; }
    sal_Bool isForwarded()
        { return m_bForwarded; }
    void setForwardedInSameFile(sal_Bool bForwarded)
        { m_bForwardedInSameFile = bForwarded; }
    sal_Bool isForwardedInSameFile()
        { return m_bForwardedInSameFile; }

    void setDefined() { m_bIsDefined = true; }
    sal_Bool isDefined() const
        { return m_bIsDefined; }

    bool usesSingleInheritance() const { return m_bSingleInheritance; }

    DoubleDeclarations checkInheritedInterfaceClashes(
        AstInterface const * ifc, bool optional) const;

    void addInheritedInterface(
        AstType const * ifc, bool optional,
        rtl::OUString const & documentation);

    DoubleMemberDeclarations checkMemberClashes(
        AstDeclaration const * member) const;

    void addMember(AstDeclaration /*TODO: const*/ * member);

    void forwardDefined(AstInterface const & def);

    virtual sal_Bool dump(RegistryKey& rKey);

private:
    enum InterfaceKind {
        INTERFACE_INDIRECT_OPTIONAL, INTERFACE_DIRECT_OPTIONAL,
        INTERFACE_INDIRECT_MANDATORY, INTERFACE_DIRECT_MANDATORY };

    struct VisibleMember {
        explicit VisibleMember(AstDeclaration const * theMandatory = 0):
            mandatory(theMandatory) {}

        typedef std::map< rtl::OString, AstDeclaration const * > Optionals;

        AstDeclaration const * mandatory;
        Optionals optionals;
    };

    typedef std::map< rtl::OString, InterfaceKind > VisibleInterfaces;
    typedef std::map< rtl::OString, VisibleMember > VisibleMembers;

    void checkInheritedInterfaceClashes(
        DoubleDeclarations & doubleDeclarations,
        std::set< rtl::OString > & seenInterfaces, AstInterface const * ifc,
        bool optional, bool direct, bool mainOptional) const;

    void checkMemberClashes(
        DoubleMemberDeclarations & doubleMembers, AstDeclaration const * member,
        bool checkOptional) const;

    void addVisibleInterface(
        AstInterface const * ifc, bool direct, bool optional);

    void addOptionalVisibleMembers(AstInterface const * ifc);

    bool increment(sal_uInt16 * counter, char const * sort) const;

    InheritedInterfaces m_inheritedInterfaces;
    InheritedInterfaces::size_type m_mandatoryInterfaces;
    sal_Bool    m_bIsDefined;
    sal_Bool    m_bForwarded;
    sal_Bool    m_bForwardedInSameFile;
    bool m_bSingleInheritance;
    VisibleInterfaces m_visibleInterfaces;
    VisibleMembers m_visibleMembers;
};

#endif // _IDLC_ASTINTERFACE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
