/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AddField.cxx,v $
 * $Revision: 1.5 $
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
#ifndef RPTUI_COLUMNINFO_HXX
#define RPTUI_COLUMNINFO_HXX

#include <rtl/ustring.hxx>

namespace rptui
{
    struct ColumnInfo
    {
        ::rtl::OUString sColumnName;
        ::rtl::OUString sLabel;
        bool bColumn;
        ColumnInfo(const ::rtl::OUString& i_sColumnName,const ::rtl::OUString& i_sLabel)
            : sColumnName(i_sColumnName)
            , sLabel(i_sLabel)
            , bColumn(true)
        {
        }
        ColumnInfo(const ::rtl::OUString& i_sColumnName)
            : sColumnName(i_sColumnName)
            , bColumn(false)
        {
        }
    };
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_COLUMNINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
