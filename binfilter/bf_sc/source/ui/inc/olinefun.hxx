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

#ifndef SC_OLINEFUN_HXX
#define SC_OLINEFUN_HXX

#include "global.hxx"
namespace binfilter {

class ScDocShell;
class ScRange;


// ---------------------------------------------------------------------------

class ScOutlineDocFunc
{
private:
    ScDocShell&	rDocShell;

public:
                ScOutlineDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                ~ScOutlineDocFunc() {}

    BOOL		MakeOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi );
    BOOL		RemoveOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi );
    BOOL		RemoveAllOutlines( USHORT nTab, BOOL bRecord, BOOL bApi );
    BOOL		AutoOutline( const ScRange& rRange, BOOL bRecord, BOOL bApi );

    BOOL		SelectLevel( USHORT nTab, BOOL bColumns, USHORT nLevel,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL		ShowMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi );
    BOOL		HideMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
