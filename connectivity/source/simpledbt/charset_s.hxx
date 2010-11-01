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

#ifndef CONNECTIVITY_DBTOOLS_CHARSET_S_HXX
#define CONNECTIVITY_DBTOOLS_CHARSET_S_HXX

#include <connectivity/virtualdbtools.hxx>
#include "refbase.hxx"
#include <connectivity/dbcharset.hxx>

//........................................................................
namespace connectivity
{
//........................................................................

    //====================================================================
    //= ODataAccessCharSet
    //====================================================================
    class ODataAccessCharSet
            :public simple::IDataAccessCharSet
            ,public ORefBase
    {
    protected:
        ::dbtools::OCharsetMap      m_aCharsetInfo;

    public:
        ODataAccessCharSet() { }

        // IDataAccessCharSet
        sal_Int32   getSupportedTextEncodings(
            ::std::vector< rtl_TextEncoding >& /* [out] */ _rEncs
        ) const;

        // disambiguate IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_DBTOOLS_CHARSET_S_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
