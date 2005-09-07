/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astbasetype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:55:03 $
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
#ifndef _IDLC_ASTBASETYPE_HXX_
#define _IDLC_ASTBASETYPE_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif
#ifndef _IDLC_ASTEXPRESSION_HXX_
#include <idlc/astexpression.hxx>
#endif


class AstBaseType : public AstType
{
public:
    AstBaseType(const ExprType type, const ::rtl::OString& name, AstScope* pScope)
        : AstType(NT_predefined, name, pScope)
        , m_exprType(type)
        {}

    virtual ~AstBaseType() {}

    const ExprType getExprType() const
        { return m_exprType; }
private:
    const ExprType m_exprType;
};

#endif // _IDLC_ASTBASETYPE_HXX_

