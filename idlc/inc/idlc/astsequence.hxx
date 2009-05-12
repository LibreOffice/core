/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astsequence.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _IDLC_ASTSEQUENCE_HXX_
#define _IDLC_ASTSEQUENCE_HXX_

#include <idlc/asttype.hxx>

class AstSequence : public AstType
{
public:
    AstSequence(AstType* pMemberType, AstScope* pScope)
        : AstType(NT_sequence, ::rtl::OString("[]")+pMemberType->getScopedName(), pScope)
        , m_pMemberType(pMemberType)
        , m_pRelativName(NULL)
    {}
    virtual ~AstSequence()
    {
        if ( m_pRelativName )
            delete m_pRelativName;
    }

    AstType* getMemberType() const
        { return m_pMemberType; }

    virtual bool isUnsigned() const
    { return m_pMemberType != 0 && m_pMemberType->isUnsigned(); }

    virtual const sal_Char* getRelativName() const;
private:
    AstType*        m_pMemberType;
    mutable ::rtl::OString* m_pRelativName;
};

#endif // _IDLC_ASTSEQUENCE_HXX_

