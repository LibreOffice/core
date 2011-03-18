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
#ifndef CONNECTIVITY_ADO_WRAP_INDEX_HXX
#define CONNECTIVITY_ADO_WRAP_INDEX_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOIndex : public WpOLEBase<_ADOIndex>
        {
        public:
            WpADOIndex(_ADOIndex* pInt=NULL)    :   WpOLEBase<_ADOIndex>(pInt){}
            WpADOIndex(const WpADOIndex& rhs){operator=(rhs);}

            inline WpADOIndex& operator=(const WpADOIndex& rhs)
                {WpOLEBase<_ADOIndex>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void            put_Name(const ::rtl::OUString& _rName);
            sal_Bool        get_Clustered() const;
            void            put_Clustered(sal_Bool _b);
            sal_Bool        get_Unique() const;
            void            put_Unique(sal_Bool _b);
            sal_Bool        get_PrimaryKey() const;
            void            put_PrimaryKey(sal_Bool _b);
            WpADOColumns    get_Columns() const;
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_INDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
