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

#ifndef DBACCESS_MIGRATIONPROGRESS_HXX
#define DBACCESS_MIGRATIONPROGRESS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= IMigrationProgress
    //====================================================================
    class SAL_NO_VTABLE IMigrationProgress
    {
    public:
        virtual void    startObject( const ::rtl::OUString& _rObjectName, const ::rtl::OUString& _rCurrentAction, const sal_uInt32 _nRange ) = 0;
        virtual void    setObjectProgressText( const ::rtl::OUString& _rText ) = 0;
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue ) = 0;
        virtual void    endObject() = 0;
        virtual void    start( const sal_uInt32 _nOverallRange ) = 0;
        virtual void    setOverallProgressText( const ::rtl::OUString& _rText ) = 0;
        virtual void    setOverallProgressValue( const sal_uInt32 _nValue ) = 0;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MIGRATIONPROGRESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
