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

#ifndef SC_ADDINHELPID_HXX
#define SC_ADDINHELPID_HXX

#include <rtl/ustring.hxx>
namespace binfilter {


// ============================================================================

struct ScUnoAddInHelpId;

/** Generates help IDs for standard Calc AddIns. */
class ScUnoAddInHelpIdGenerator
{
private:
    const ScUnoAddInHelpId*     pCurrHelpIds;       /// Array of function names and help IDs.
    sal_uInt32                  nArrayCount;        /// Count of array entries.

public:
                                ScUnoAddInHelpIdGenerator( const ::rtl::OUString& rServiceName );

    /** Sets service name of the AddIn. Has to be done before requesting help IDs. */
    void                        SetServiceName( const ::rtl::OUString& rServiceName );

    /** @return  The help ID of the function with given built-in name or 0 if not found. */
    sal_uInt16                  GetHelpId( const ::rtl::OUString& rFuncName ) const;
};


// ============================================================================

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
