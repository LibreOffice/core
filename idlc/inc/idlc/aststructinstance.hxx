/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aststructinstance.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:02:04 $
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

#ifndef INCLUDED_idlc_inc_idlc_aststructinstance_hxx
#define INCLUDED_idlc_inc_idlc_aststructinstance_hxx

#include "idlc/asttype.hxx"
#include "idlc/idlctypes.hxx"

class AstDeclaration;
class AstScope;

class AstStructInstance: public AstType {
public:
    AstStructInstance(
        AstType const * typeTemplate, DeclList const * typeArguments,
        AstScope * scope);

    AstType const * getTypeTemplate() const { return m_typeTemplate; }

    DeclList::const_iterator getTypeArgumentsBegin() const
    { return m_typeArguments.begin(); }

    DeclList::const_iterator getTypeArgumentsEnd() const
    { return m_typeArguments.end(); }

private:
    AstType const * m_typeTemplate;
    DeclList m_typeArguments;
};

#endif
