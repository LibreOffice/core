/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astparameter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:59:47 $
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

#ifndef _IDLC_ASTPARAMETER_HXX_
#define _IDLC_ASTPARAMETER_HXX_

#ifndef _IDLC_ASTMEMBER_HXX_
#include <idlc/astmember.hxx>
#endif

enum Direction { DIR_IN, DIR_OUT, DIR_INOUT };

class AstParameter: public AstMember {
public:
    AstParameter(
        Direction direction, bool rest, AstType const * type,
        rtl::OString const & name, AstScope * scope):
        AstMember(NT_parameter, type, name, scope), m_direction(direction),
        m_rest(rest) {}

    virtual ~AstParameter() {}

    Direction getDirection() const { return m_direction; }

    bool isRest() const { return m_rest; }

private:
    Direction m_direction;
    bool m_rest;
};

#endif
