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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "hsqldb/HTools.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= HTools
    //====================================================================
    //--------------------------------------------------------------------
    void HTools::appendTableFilterCrit( ::rtl::OUStringBuffer& _inout_rBuffer, const ::rtl::OUString& _rCatalog,
        const ::rtl::OUString _rSchema, const ::rtl::OUString _rName, bool _bShortForm )
    {
        _inout_rBuffer.appendAscii( " WHERE " );
        if ( _rCatalog.getLength() )
        {
            _inout_rBuffer.appendAscii( _bShortForm ? "TABLE_CAT" : "TABLE_CATALOG" );
            _inout_rBuffer.appendAscii( " = '" );
            _inout_rBuffer.append     ( _rCatalog );
            _inout_rBuffer.appendAscii( "' AND " );
        }
        if ( _rSchema.getLength() )
        {
            _inout_rBuffer.appendAscii( _bShortForm ? "TABLE_SCHEM" : "TABLE_SCHEMA" );
            _inout_rBuffer.appendAscii( " = '" );
            _inout_rBuffer.append     ( _rSchema );
            _inout_rBuffer.appendAscii( "' AND " );
        }
        _inout_rBuffer.appendAscii( "TABLE_NAME = '" );
        _inout_rBuffer.append     ( _rName );
        _inout_rBuffer.appendAscii( "'" );
    }

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
