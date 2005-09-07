/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aststructinstance.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:09:27 $
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

#include "idlc/aststructinstance.hxx"

#include "idlc/asttype.hxx"
#include "idlc/idlctypes.hxx"

#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"

namespace {

rtl::OString createName(
    AstType const * typeTemplate, DeclList const * typeArguments)
{
    rtl::OStringBuffer buf(typeTemplate->getScopedName());
    if (typeArguments != 0) {
        buf.append('<');
        for (DeclList::const_iterator i(typeArguments->begin());
             i != typeArguments->end(); ++i)
        {
            if (i != typeArguments->begin()) {
                buf.append(',');
            }
            if (*i != 0) {
                buf.append((*i)->getScopedName());
            }
        }
        buf.append('>');
    }
    return buf.makeStringAndClear();
}

}

AstStructInstance::AstStructInstance(
    AstType const * typeTemplate, DeclList const * typeArguments,
    AstScope * scope):
    AstType(
        NT_instantiated_struct, createName(typeTemplate, typeArguments), scope),
    m_typeTemplate(typeTemplate), m_typeArguments(*typeArguments)
{}
