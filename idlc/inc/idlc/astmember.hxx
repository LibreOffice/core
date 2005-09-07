/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astmember.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:57:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _IDLC_ASTMEMBER_HXX_
#define _IDLC_ASTMEMBER_HXX_

#ifndef _IDLC_ASTDECLARATION_HXX_
#include "idlc/astdeclaration.hxx"
#endif

namespace rtl { class OString; }
class AstScope;
class AstType;

class AstMember: public AstDeclaration {
public:
    AstMember(
        AstType const * pType, rtl::OString const & name, AstScope * pScope):
        AstDeclaration(NT_member, name, pScope), m_pType(pType) {}

    virtual ~AstMember() {}

    AstType const * getType() const { return m_pType; }

protected:
    AstMember(
        NodeType type, AstType const * pType, rtl::OString const & name,
        AstScope * pScope):
        AstDeclaration(type, name, pScope), m_pType(pType) {}

private:
    AstType const * m_pType;
};

#endif
