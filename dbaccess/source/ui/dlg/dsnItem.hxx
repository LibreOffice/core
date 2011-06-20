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

#ifndef _DBAUI_DSNITEM_HXX_
#define _DBAUI_DSNITEM_HXX_

#include <svl/poolitem.hxx>

//.........................................................................
namespace dbaccess
{
//.........................................................................
    class ODsnTypeCollection;
//.........................................................................
}
//.........................................................................
//.........................................................................
namespace dbaui
{
//.........................................................................
    //=========================================================================
    //= DbuTypeCollectionItem
    //=========================================================================
    /** allows an ODsnTypeCollection to be transported in an SfxItemSet
    */
    class DbuTypeCollectionItem : public SfxPoolItem
    {
        ::dbaccess::ODsnTypeCollection* m_pCollection;

    public:
        TYPEINFO();
        DbuTypeCollectionItem(sal_Int16 nWhich = 0, ::dbaccess::ODsnTypeCollection* _pCollection = NULL);
        DbuTypeCollectionItem(const DbuTypeCollectionItem& _rSource);

        virtual int              operator==(const SfxPoolItem& _rItem) const;
        virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

        ::dbaccess::ODsnTypeCollection* getCollection() const { return m_pCollection; }
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DSNITEM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
