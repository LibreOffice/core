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

#ifndef EPPT_GROUP_TABLE
#define EPPT_GROUP_TABLE

#include <com/sun/star/container/XIndexAccess.hpp>

struct GroupEntry
{
    sal_uInt32                  mnCurrentPos;
    sal_uInt32                  mnCount;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >           mXIndexAccess;

    GroupEntry( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & rIndex )
    {
        mXIndexAccess = rIndex;
        mnCount =mXIndexAccess->getCount();
        mnCurrentPos = 0;
    };

    GroupEntry( sal_uInt32 nCount )
    {
        mnCount = nCount;
        mnCurrentPos = 0;
    };

    ~GroupEntry(){};
};

class GroupTable
{
    protected:

        sal_uInt32              mnIndex;
        sal_uInt32              mnCurrentGroupEntry;
        sal_uInt32              mnMaxGroupEntry;
        sal_uInt32              mnGroupsClosed;
        GroupEntry**            mpGroupEntry;

        void                    ImplResizeGroupTable( sal_uInt32 nEntrys );

    public:

        sal_uInt32              GetCurrentGroupIndex() const { return mnIndex; };
        sal_Int32               GetCurrentGroupLevel() const { return mnCurrentGroupEntry - 1; };
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > &
                                GetCurrentGroupAccess() const { return mpGroupEntry[  mnCurrentGroupEntry - 1 ]->mXIndexAccess; };
        sal_uInt32              GetGroupsClosed();
        void                    SkipCurrentGroup();
        void                    ResetGroupTable( sal_uInt32 nCount );
        void                    ClearGroupTable();
        sal_Bool                EnterGroup( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & rIndex );
        sal_Bool                GetNextGroupEntry();
                                GroupTable();
                                ~GroupTable();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
