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

#ifndef SC_DBFUNC_HXX
#define SC_DBFUNC_HXX

#include "viewfunc.hxx"
#include "dptypes.hxx"

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

struct ScSortParam;
struct ScQueryParam;
class ScDBData;
class ScDBCollection;
class ScDPObject;
class ScDPSaveData;
struct ScDPNumGroupInfo;
struct ScSubTotalParam;

// ---------------------------------------------------------------------------

class ScDBFunc : public ScViewFunc
{
private:
    void GetSelectedMemberList(ScDPUniqueStringSet& rEntries, long& rDimension);

public:
                    ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
    virtual         ~ScDBFunc();

                    // only UISort repeat the partial results if necessary

    void            UISort( const ScSortParam& rSortParam,
                          sal_Bool bRecord = sal_True );

    void            Sort( const ScSortParam& rSortParam,
                          sal_Bool bRecord = sal_True, sal_Bool bPaint = sal_True );
    SC_DLLPUBLIC void           Query( const ScQueryParam& rQueryParam,
                           const ScRange* pAdvSource, sal_Bool bRecord );
    void            DoSubTotals( const ScSubTotalParam& rParam, sal_Bool bRecord = sal_True,
                            const ScSortParam* pForceNewSort = NULL );

    void            ToggleAutoFilter();
    void            HideAutoFilter();

    void            RepeatDB( sal_Bool bRecord = sal_True );

    sal_Bool            ImportData( const ScImportParam& rParam, sal_Bool bRecord = sal_True );

    void GotoDBArea( const OUString& rDBName );

                    // DB range from Cursor
    ScDBData*       GetDBData( bool bMarkArea = true, ScGetDBMode eMode = SC_DB_MAKE, ScGetDBSelection eSel = SC_DBSEL_KEEP);
    ScDBData*       GetAnonymousDBData();

    void            NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const std::vector<ScRange> &rDelAreaList );

    void            Consolidate( const ScConsolidateParam& rParam, sal_Bool bRecord = sal_True );

    bool MakePivotTable(
        const ScDPSaveData& rData, const ScRange& rDest, bool bNewTable,
        const ScDPObject& rSource, bool bApi = false );

    void            DeletePivotTable();
    void            RecalcPivotTable();
    bool HasSelectionForDateGroup( ScDPNumGroupInfo& rOldInfo, sal_Int32& rParts );
    bool HasSelectionForNumGroup( ScDPNumGroupInfo& rOldInfo );
    void            GroupDataPilot();
    void            DateGroupDataPilot( const ScDPNumGroupInfo& rInfo, sal_Int32 nParts );
    void            NumGroupDataPilot( const ScDPNumGroupInfo& rInfo );
    void            UngroupDataPilot();
    void DataPilotInput( const ScAddress& rPos, const OUString& rString );

    bool            DataPilotSort( const ScAddress& rPos, bool bAscending, sal_uInt16* pUserListId = NULL );
    sal_Bool            DataPilotMove( const ScRange& rSource, const ScAddress& rDest );

    bool HasSelectionForDrillDown( sal_uInt16& rOrientation );
    void SetDataPilotDetails(bool bShow, const OUString* pNewDimensionName = NULL);

    void            ShowDataPilotSourceData( ScDPObject& rDPObj,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::DataPilotFieldFilter >& rFilters );

    void            MakeOutline( sal_Bool bColumns, sal_Bool bRecord = sal_True );
    void            RemoveOutline( sal_Bool bColumns, sal_Bool bRecord = sal_True );
    void            RemoveAllOutlines( sal_Bool bRecord = sal_True );
    void            TestRemoveOutline( sal_Bool& rCol, sal_Bool& rRow );

    void            AutoOutline( sal_Bool bRecord = sal_True );

    void            SelectLevel( sal_Bool bColumns, sal_uInt16 nLevel,
                                    sal_Bool bRecord = sal_True, sal_Bool bPaint = sal_True );
    void            ShowOutline( sal_Bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    sal_Bool bRecord = sal_True, sal_Bool bPaint = sal_True );
    void            HideOutline( sal_Bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    sal_Bool bRecord = sal_True, sal_Bool bPaint = sal_True );

    void            ShowMarkedOutlines( sal_Bool bRecord = sal_True );
    void            HideMarkedOutlines( sal_Bool bRecord = sal_True );
    sal_Bool            OutlinePossible(sal_Bool bHide);

    void            UpdateCharts(sal_Bool bAllCharts = false);      // Default: am Cursor

    static sal_uInt16   DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, sal_Bool bAllCharts );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
