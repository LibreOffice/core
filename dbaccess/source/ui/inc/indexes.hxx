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

#ifndef _DBAUI_INDEXES_HXX_
#define _DBAUI_INDEXES_HXX_

#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>

//......................................................................
namespace dbaui
{
//......................................................................
    //==================================================================
    //= OIndexField
    //==================================================================
    struct OIndexField
    {
        String              sFieldName;
        sal_Bool            bSortAscending;

        OIndexField() : bSortAscending(sal_True) { }
    };

    DECLARE_STL_VECTOR( OIndexField, IndexFields );

    //==================================================================
    //= OIndex
    //==================================================================
    struct GrantIndexAccess
    {
        friend class OIndexCollection;
    private:
        GrantIndexAccess() { }
    };

    //..................................................................
    struct OIndex
    {
    protected:
        ::rtl::OUString     sOriginalName;
        sal_Bool            bModified;

    public:
        ::rtl::OUString     sName;
        ::rtl::OUString     sDescription;
        sal_Bool            bPrimaryKey;
        sal_Bool            bUnique;
        IndexFields         aFields;

    public:
        OIndex(const ::rtl::OUString& _rOriginalName)
            : sOriginalName(_rOriginalName), bModified(sal_False), sName(_rOriginalName), bPrimaryKey(sal_False), bUnique(sal_False)
        {
        }

        const ::rtl::OUString& getOriginalName() const { return sOriginalName; }

        sal_Bool    isModified() const { return bModified; }
        void        setModified(sal_Bool _bModified) { bModified = _bModified; }
        void        clearModified() { setModified(sal_False); }

        sal_Bool    isNew() const { return 0 == getOriginalName().getLength(); }
        void        flagAsNew(const GrantIndexAccess&) { sOriginalName = ::rtl::OUString(); }
        void        flagAsCommitted(const GrantIndexAccess&) { sOriginalName = sName; }


    private:
        OIndex();   // not implemented
    };

    DECLARE_STL_VECTOR( OIndex, Indexes );

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
