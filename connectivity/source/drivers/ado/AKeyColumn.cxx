/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AKeyColumn.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:29:40 $
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

#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_
#include "adabas/BKeyColumn.hxx"
#endif

using namespace connectivity::adabas;
namespace starbeans     = ::com::sun::star::beans;
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn() : OColumn()
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn(     const ::rtl::OUString& _ReferencedColumn,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _TypeName,
                            const ::rtl::OUString& _DefaultValue,
                            sal_Int32       _IsNullable,
                            sal_Int32       _Precision,
                            sal_Int32       _Scale,
                            sal_Int32       _Type,
                            sal_Bool        _IsAutoIncrement
                        ) : OColumn(_Name,
                            _TypeName,
                            _DefaultValue,
                            _IsNullable,
                            _Precision,
                            _Scale,
                            _Type,
                            _IsAutoIncrement)
                        ,   m_ReferencedColumn(_ReferencedColumn)
{
    construct();
}

// -------------------------------------------------------------------------
void OKeyColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : starbeans::PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}

