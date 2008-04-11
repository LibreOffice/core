/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astinterface.hxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _IDLC_ASTINTERFACE_HXX_
#define _IDLC_ASTINTERFACE_HXX_

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
