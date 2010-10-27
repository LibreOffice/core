/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _DBAUI_STRINGLISTITEM_HXX_
#define _DBAUI_STRINGLISTITEM_HXX_

#include <svl/poolitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OStringListItem
//=========================================================================
/** <type>SfxPoolItem</type> which transports a sequence of <type scope="rtl">OUString</type>'s
*/
class OStringListItem : public SfxPoolItem
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >      m_aList;

public:
    TYPEINFO();
    OStringListItem(sal_Int16 nWhich, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rList);
    OStringListItem(const OStringListItem& _rSource);

    virtual int              operator==(const SfxPoolItem& _rItem) const;
    virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  getList() const { return m_aList; }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_STRINGLISTITEM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
