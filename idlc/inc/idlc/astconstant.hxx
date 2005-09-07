/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astconstant.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:55:21 $
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
#ifndef _IDLC_ASTCONSTANT_HXX_
#define _IDLC_ASTCONSTANT_HXX_

#ifndef _IDLC_ASTDECLARATION_HXX_
#include <idlc/astdeclaration.hxx>
#endif
#ifndef _IDLC_ASTEXPRESSION_HXX_
#include <idlc/astexpression.hxx>
#endif

namespace typereg { class Writer; }

class AstConstant : public AstDeclaration
{
public:
    AstConstant(const ExprType type, const NodeType nodeType,
                AstExpression* pExpr, const ::rtl::OString& name, AstScope* pScope);
    AstConstant(const ExprType type, AstExpression* pExpr,
                const ::rtl::OString& name, AstScope* pScope);
    virtual ~AstConstant();

    AstExpression* getConstValue()
        { return m_pConstValue; }
    const ExprType getConstValueType()
        { return m_constValueType; }

    sal_Bool dumpBlob(
        typereg::Writer & rBlob, sal_uInt16 index, bool published);
private:
    AstExpression*                  m_pConstValue;
    const ExprType  m_constValueType;
};

#endif // _IDLC_ASTCONSTANT_HXX_

