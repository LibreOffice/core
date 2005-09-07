/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astsequence.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:00:19 $
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
#ifndef _IDLC_ASTSEQUENCE_HXX_
#define _IDLC_ASTSEQUENCE_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif

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

    AstType* getMemberType()
        { return m_pMemberType; }

    virtual const sal_Char* getRelativName() const;
private:
    AstType*        m_pMemberType;
    mutable ::rtl::OString* m_pRelativName;
};

#endif // _IDLC_ASTSEQUENCE_HXX_

