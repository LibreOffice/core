/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astobserves.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:58:57 $
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
#ifndef _IDLC_ASTOBSERVES_HXX_
#define _IDLC_ASTOBSERVES_HXX_

#ifndef _IDLC_ASTINTERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif

class AstObserves : public AstDeclaration
{
public:
    AstObserves(AstInterface* pRealInterface, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_observes, name, pScope)
        , m_pRealInterface(pRealInterface)
        {}
    virtual ~AstObserves() {}

    AstInterface* getRealInterface()
        { return m_pRealInterface; }
private:
    AstInterface*   m_pRealInterface;
};

#endif // _IDLC_ASTOBSERVES_HXX_

