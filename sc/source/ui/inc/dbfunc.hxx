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

#ifndef SC_DBFUNC_HXX
#define SC_DBFUNC_HXX

#include "viewfunc.hxx"

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

struct ScSortParam;
struct ScQueryParam;
class ScDBData;
class ScDBCollection;
class ScDPObject;
class ScDPSaveData;
class ScStrCollection;
struct ScDPNumGroupInfo;

// ---------------------------------------------------------------------------

class ScDBFunc : public ScViewFunc
{
private:
    void            GetSelectedMemberList( ScStrCollection& rEntries, long& rDimension );

public:
                    ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
    virtual         ~ScDBFunc();

                    //  nur UISort wiederholt bei Bedarf die Teilergebnisse

    void            UISort( const ScSortParam& rSortParam,
                          BOOL bRecord = TRUE );

    void            Sort( const ScSortParam& rSortParam,
                          BOOL bRecord = TRUE, BOOL bPaint = TRUE );
    SC_DLLPUBLIC void           Query( const ScQueryParam& rQueryParam,
                           const ScRange* pAdvSource, BOOL bRecord );
    void            DoSubTotals( const ScSubTotalParam& rParam, BOOL bRecord = TRUE,
                            const ScSortParam* pForceNewSort = NULL );

    void            ToggleAutoFilter();
    void            HideAutoFilter();

    void            RepeatDB( BOOL bRecord = TRUE );

    BOOL            ImportData( const ScImportParam& rParam, BOOL bRecord = TRUE );

    void            GotoDBArea( const String& rDBName );

                    // DB-Bereich vom Cursor
    ScDBData*       GetDBData( BOOL bMarkArea = TRUE, ScGetDBMode eMode = SC_DB_MAKE, ScGetDBSelection eSel = SC_DBSEL_KEEP, bool bShrinkToData = false, bool bExpandRows = false );

    void            NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const List& rDelAreaList );

    void            Consolidate( const ScConsolidateParam& rParam, BOOL bRecord = TRUE );

    bool            MakePivotTable( const ScDPSaveData& rData, const ScRange& rDest, BOOL bNewTable,
                                    const ScDPObject& rSource, BOOL bApi = FALSE );
    void            DeletePivotTable();
    void            RecalcPivotTable();
    BOOL            HasSelectionForDateGroup( ScDPNumGroupInfo& rOldInfo, sal_Int32& rParts );
    BOOL            HasSelectionForNumGroup( ScDPNumGroupInfo& rOldInfo );
    void            GroupDataPilot();
    void            DateGroupDataPilot( const ScDPNumGroupInfo& rInfo, sal_Int32 nParts );
    void            NumGroupDataPilot( const ScDPNumGroupInfo& rInfo );
    void            UngroupDataPilot();
    void            DataPilotInput( const ScAddress& rPos, const String& rString );

    bool            DataPilotSort( const ScAddress& rPos, bool bAscending, sal_uInt16* pUserListId = NULL );
    BOOL            DataPilotMove( const ScRange& rSource, const ScAddress& rDest );

    BOOL            HasSelectionForDrillDown( USHORT& rOrientation );
    void            SetDataPilotDetails( BOOL bShow, const String* pNewDimensionName = NULL );

    void            ShowDataPilotSourceData( ScDPObject& rDPObj,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::DataPilotFieldFilter >& rFilters );

    void            MakeOutline( BOOL bColumns, BOOL bRecord = TRUE );
    void            RemoveOutline( BOOL bColumns, BOOL bRecord = TRUE );
    void            RemoveAllOutlines( BOOL bRecord = TRUE );
    void            TestRemoveOutline( BOOL& rCol, BOOL& rRow );

    void            AutoOutline( BOOL bRecord = TRUE );

    void            SelectLevel( BOOL bColumns, USHORT nLevel,
                                    BOOL bRecord = TRUE, BOOL bPaint = TRUE );
    void            ShowOutline( BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord = TRUE, BOOL bPaint = TRUE );
    void            HideOutline( BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord = TRUE, BOOL bPaint = TRUE );

    void            ShowMarkedOutlines( BOOL bRecord = TRUE );
    void            HideMarkedOutlines( BOOL bRecord = TRUE );
    BOOL            OutlinePossible(BOOL bHide);

    void            UpdateCharts(BOOL bAllCharts = FALSE);      // Default: am Cursor

    static USHORT   DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, BOOL bAllCharts );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
