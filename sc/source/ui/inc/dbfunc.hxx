/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
//UNUSED2008-05     ScDBFunc( Window* pParent, const ScDBFunc& rDBFunc, ScTabViewShell* pViewShell );
    virtual         ~ScDBFunc();

                    //  nur UISort wiederholt bei Bedarf die Teilergebnisse

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

    void            GotoDBArea( const String& rDBName );

                    // DB-Bereich vom Cursor
    ScDBData*       GetDBData( sal_Bool bMarkArea = sal_True, ScGetDBMode eMode = SC_DB_MAKE, ScGetDBSelection eSel = SC_DBSEL_KEEP );

    void            NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const List& rDelAreaList );

    void            Consolidate( const ScConsolidateParam& rParam, sal_Bool bRecord = sal_True );

    bool            MakePivotTable( const ScDPSaveData& rData, const ScRange& rDest, sal_Bool bNewTable,
                                    const ScDPObject& rSource, sal_Bool bApi = sal_False );
    void            DeletePivotTable();
    // Wang Xu Ming -- 2009-6-17
    // DataPilot Migration
    sal_uLong   RecalcPivotTable();
    // End Comments
    sal_Bool            HasSelectionForDateGroup( ScDPNumGroupInfo& rOldInfo, sal_Int32& rParts );
    sal_Bool            HasSelectionForNumGroup( ScDPNumGroupInfo& rOldInfo );
    void            GroupDataPilot();
    void            DateGroupDataPilot( const ScDPNumGroupInfo& rInfo, sal_Int32 nParts );
    void            NumGroupDataPilot( const ScDPNumGroupInfo& rInfo );
    void            UngroupDataPilot();
    void            DataPilotInput( const ScAddress& rPos, const String& rString );

    bool            DataPilotSort( const ScAddress& rPos, bool bAscending, sal_uInt16* pUserListId = NULL );
    sal_Bool            DataPilotMove( const ScRange& rSource, const ScAddress& rDest );

    sal_Bool            HasSelectionForDrillDown( sal_uInt16& rOrientation );
    void            SetDataPilotDetails( sal_Bool bShow, const String* pNewDimensionName = NULL );

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

    void            UpdateCharts(sal_Bool bAllCharts = sal_False);      // Default: am Cursor

    static sal_uInt16   DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, sal_Bool bAllCharts );
};



#endif

