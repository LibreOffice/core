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

#ifndef SC_DBDOCFUN_HXX
#define SC_DBDOCFUN_HXX

#include <tools/solar.h>

#include <com/sun/star/uno/Sequence.hxx>

class String;
namespace binfilter {

struct ScImportParam;
struct ScQueryParam;
struct ScSortParam;
struct ScSubTotalParam;

class SfxViewFrame;
class SbaSelectionList;
class ScDBData;
class ScDocShell;
class ScAddress;
class ScRange;
class ScDPObject;
} //namespace binfilter
namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
    namespace sdbc {
        class XResultSet;
    }
} } }
namespace binfilter {//STRIP009
// ---------------------------------------------------------------------------

class ScDBDocFunc
{
friend class ScDBFunc;

private:
    ScDocShell&		rDocShell;

public:
                    ScDBDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDBDocFunc() {}

    BOOL			DoImport( USHORT nTab, const ScImportParam& rParam,
                        const ::com::sun::star::uno::Reference<
                        ::com::sun::star::sdbc::XResultSet >& xResultSet,
                        const SbaSelectionList* pSelection, BOOL bRecord,
                        BOOL bAddrInsert = FALSE );

    BOOL			Sort( USHORT nTab, const ScSortParam& rSortParam,
                            BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL			Query( USHORT nTab, const ScQueryParam& rQueryParam,
                            const ScRange* pAdvSource, BOOL bRecord, BOOL bApi );

    BOOL			DoSubTotals( USHORT nTab, const ScSubTotalParam& rParam,
                                    const ScSortParam* pForceNewSort,
                                    BOOL bRecord, BOOL bApi );

    BOOL			AddDBRange( const String& rName, const ScRange& rRange, BOOL bApi );
    BOOL			DeleteDBRange( const String& rName, BOOL bApi );
    BOOL			RenameDBRange( const String& rOld, const String& rNew, BOOL bApi );
    BOOL			ModifyDBData( const ScDBData& rNewData, BOOL bApi );	// Name unveraendert

    BOOL			RepeatDB( const String& rDBName, BOOL bRecord, BOOL bApi );

    BOOL			DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj, 
                                        BOOL bRecord, BOOL bApi );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
