/*************************************************************************
 *
 *  $RCSfile: astinterface.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-06-21 09:48:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _IDLC_ASTINTERFACE_HXX_
#define _IDLC_ASTINTERFACE_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif
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
