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
#ifndef INCLUDED_IDLC_INC_ASTINTERFACE_HXX
#define INCLUDED_IDLC_INC_ASTINTERFACE_HXX

#include "asttype.hxx"
#include "astscope.hxx"
#include "inheritedinterface.hxx"

#include <map>
#include <vector>

class AstInterface : public AstType
                   , public AstScope
{
public:
    typedef std::vector< InheritedInterface > InheritedInterfaces;

    struct DoubleMemberDeclaration {
        AstDeclaration const * first;
        AstDeclaration const * second;
    };

    typedef std::vector< DoubleMemberDeclaration > DoubleMemberDeclarations;

    struct DoubleDeclarations {
        std::vector< AstInterface const * > interfaces;
        DoubleMemberDeclarations members;
    };

    AstInterface(
        const OString& name, AstInterface const * pInherits,
        AstScope* pScope);
    virtual ~AstInterface() override;

    AstInterface(AstInterface const &) = default;
    AstInterface(AstInterface &&) = default;
    AstInterface & operator =(AstInterface const &) = default;
    AstInterface & operator =(AstInterface &&) = default;

    InheritedInterfaces const & getAllInheritedInterfaces() const
    { return m_inheritedInterfaces; }

    bool hasMandatoryInheritedInterfaces() const
    { return m_mandatoryInterfaces > 0; }

    void setDefined() { m_bIsDefined = true; }
    bool isDefined() const
        { return m_bIsDefined; }

    bool usesSingleInheritance() const { return m_bSingleInheritance; }

    DoubleDeclarations checkInheritedInterfaceClashes(
        AstInterface const * ifc, bool optional) const;

    void addInheritedInterface(
        AstType const * ifc, bool optional,
        OUString const & documentation);

    DoubleMemberDeclarations checkMemberClashes(
        AstDeclaration const * member) const;

    void addMember(AstDeclaration /*TODO: const*/ * member);

    void forwardDefined(AstInterface const & def);

    virtual bool dump(RegistryKey& rKey) override;

private:
    enum InterfaceKind {
        INTERFACE_INDIRECT_OPTIONAL, INTERFACE_DIRECT_OPTIONAL,
        INTERFACE_INDIRECT_MANDATORY, INTERFACE_DIRECT_MANDATORY };

    struct VisibleMember {
        explicit VisibleMember(AstDeclaration const * theMandatory = nullptr):
            mandatory(theMandatory) {}

        typedef std::map< OString, AstDeclaration const * > Optionals;

        AstDeclaration const * mandatory;
        Optionals optionals;
    };

    typedef std::map< OString, InterfaceKind > VisibleInterfaces;
    typedef std::map< OString, VisibleMember > VisibleMembers;

    void checkInheritedInterfaceClashes(
        DoubleDeclarations & doubleDeclarations,
        std::set< OString > & seenInterfaces, AstInterface const * ifc,
        bool direct, bool optional, bool mainOptional) const;

    void checkMemberClashes(
        DoubleMemberDeclarations & doubleMembers, AstDeclaration const * member,
        bool checkOptional) const;

    void addVisibleInterface(
        AstInterface const * ifc, bool direct, bool optional);

    void addOptionalVisibleMembers(AstInterface const * ifc);

    bool increment(sal_uInt16 * counter, char const * sort) const;

    InheritedInterfaces m_inheritedInterfaces;
    InheritedInterfaces::size_type m_mandatoryInterfaces;
    bool    m_bIsDefined;
    bool m_bSingleInheritance;
    VisibleInterfaces m_visibleInterfaces;
    VisibleMembers m_visibleMembers;
};

#endif // INCLUDED_IDLC_INC_ASTINTERFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
