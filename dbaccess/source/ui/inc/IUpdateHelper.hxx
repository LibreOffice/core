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
#ifndef DBAUI_IUPDATEHELPER_HXX
#define DBAUI_IUPDATEHELPER_HXX

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>

namespace dbaui
{
    class SAL_NO_VTABLE IUpdateHelper
    {
    public:
        virtual void updateString(sal_Int32 _nPos, const ::rtl::OUString& _sValue) = 0;
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue) = 0;
        virtual void updateInt(sal_Int32 _nPos,const sal_Int32& _nValue) = 0;
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32 sqlType) = 0;
        virtual void updateDate(sal_Int32 _nPos,const ::com::sun::star::util::Date& _nValue) = 0;
        virtual void updateTime(sal_Int32 _nPos,const ::com::sun::star::util::Time& _nValue) = 0;
        virtual void updateTimestamp(sal_Int32 _nPos,const ::com::sun::star::util::DateTime& _nValue) = 0;
        virtual void moveToInsertRow() = 0;
        virtual void insertRow() = 0;
    };
}

#endif // DBAUI_IUPDATEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
