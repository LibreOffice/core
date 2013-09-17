/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_DBDOCFUN_HXX
#define SC_DBDOCFUN_HXX

#include "address.hxx"
#include <tools/solar.h>
#include <com/sun/star/uno/Sequence.hxx>

struct ScImportParam;
struct ScQueryParam;
struct ScSortParam;
struct ScSubTotalParam;

class SfxViewFrame;
class ScDBData;
class ScDocShell;
class ScAddress;
class ScRange;
class ScDPObject;

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
} } }

namespace svx {
    class ODataAccessDescriptor;
}

class ScDBDocFunc
{
friend class ScDBFunc;

private:
    ScDocShell&     rDocShell;

public:
                    ScDBDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDBDocFunc() {}

    void            UpdateImport( const OUString& rTarget, const svx::ODataAccessDescriptor& rDescriptor );

    bool DoImport( SCTAB nTab, const ScImportParam& rParam,
                   const svx::ODataAccessDescriptor* pDescriptor,      // used for selection an existing ResultSet
                   bool bRecord,
                   bool bAddrInsert = false );

    bool DoImportUno( const ScAddress& rPos,
                      const com::sun::star::uno::Sequence<
                          com::sun::star::beans::PropertyValue>& aArgs );

    static void     ShowInBeamer( const ScImportParam& rParam, SfxViewFrame* pFrame );

    sal_Bool            Sort( SCTAB nTab, const ScSortParam& rSortParam,
                            sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    SC_DLLPUBLIC sal_Bool           Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                            const ScRange* pAdvSource, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                                    const ScSortParam* pForceNewSort,
                                    sal_Bool bRecord, sal_Bool bApi );

    bool AddDBRange( const OUString& rName, const ScRange& rRange, sal_Bool bApi );
    bool DeleteDBRange( const OUString& rName );
    bool RenameDBRange( const OUString& rOld, const OUString& rNew );
    bool ModifyDBData( const ScDBData& rNewData );  // Name unveraendert

    bool RepeatDB( const OUString& rDBName, bool bRecord, bool bApi, bool bIsUnnamed=false, SCTAB aTab = 0);

    bool DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                          bool bRecord, bool bApi, bool bAllowMove = false );

    bool RemovePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi);
    bool CreatePivotTable(const ScDPObject& rDPObj, bool bRecord, bool bApi);
    bool UpdatePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi);

    /**
     * Reload the referenced pivot cache, and refresh all pivot tables that
     * reference the cache.
     */
    sal_uLong RefreshPivotTables(ScDPObject* pDPObj, bool bApi);

    /**
     * Refresh the group dimensions of all pivot tables referencing the same
     * cache.
     */
    void RefreshPivotTableGroups(ScDPObject* pDPObj);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
