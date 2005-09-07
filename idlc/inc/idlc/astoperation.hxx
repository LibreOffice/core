/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astoperation.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:59:21 $
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
#ifndef _IDLC_ASTOPERATION_HXX_
#define _IDLC_ASTOPERATION_HXX_

#ifndef _IDLC_ASTDECLARATION_HXX_
#include <idlc/astdeclaration.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif

namespace typereg { class Writer; }

#define OP_NONE         0x0000
#define OP_ONEWAY       0x0001

class AstType;

class AstOperation : public AstDeclaration
                   , public AstScope
{
public:
    AstOperation(sal_uInt32 flags, AstType* pReturnType, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_operation, name, pScope)
        , AstScope(NT_operation)
        , m_flags(flags)
        , m_pReturnType(pReturnType)
        {}
    virtual ~AstOperation() {}

    sal_Bool isOneway()
        { return ((m_flags & OP_ONEWAY) == OP_ONEWAY); }
    sal_Bool isVoid();

    bool isVariadic() const;

    bool isConstructor() const { return m_pReturnType == 0; }

    void setExceptions(DeclList const * pExceptions);
    const DeclList& getExceptions()
        { return m_exceptions; }
    sal_uInt16 nExceptions()
        { return (sal_uInt16)(m_exceptions.size()); }

    sal_Bool dumpBlob(typereg::Writer & rBlob, sal_uInt16 index);

    // scope management
    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);
private:
    sal_uInt32  m_flags;
    AstType*    m_pReturnType;
    DeclList    m_exceptions;
};

#endif // _IDLC_ASTOPERATION_HXX_

