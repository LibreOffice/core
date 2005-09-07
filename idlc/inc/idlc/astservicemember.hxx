/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astservicemember.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:01:07 $
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
#ifndef _IDLC_ASTSERVICEMEMBER_HXX_
#define _IDLC_ASTSERVICEMEMBER_HXX_

#ifndef _IDLC_ASTSERVICE_HXX_
#include <idlc/astservice.hxx>
#endif

class AstServiceMember : public AstDeclaration
{
public:
    AstServiceMember(const sal_uInt32 flags, AstService* pRealService,
                     const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_service_member, name, pScope)
        , m_pRealService(pRealService)
        , m_flags(flags)
        {}
    virtual ~AstServiceMember() {}

    AstService* getRealService()
        { return m_pRealService; }
    sal_Bool isOptional()
        { return ((m_flags & AF_OPTIONAL) == AF_OPTIONAL); }
private:
    const sal_uInt32    m_flags;
    AstService*         m_pRealService;
};

#endif // _IDLC_ASTSERVICEMEMBER_HXX_

