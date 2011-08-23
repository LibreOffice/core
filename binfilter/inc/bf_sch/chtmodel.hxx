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


#ifndef _CHTMODEL_HXX
#define _CHTMODEL_HXX

//------------------------------------------------------------------
//
// dieses Define dient nur zum Testen und darf auf keinen Fall aktiv bleiben
// Bei Umstellungen alle Files nach SIG_CHARTMODELDEFSCHDOC durchsuchen
// #define ChartModel SchChartDocument
//------------------------------------------------------------------
//STRIP008 class SdrObjGroup;
//STRIP008 class SdrRectObj;
//STRIP008 
//STRIP008 class SchModelData;

#ifndef _E3D_POLYGON3D_HXX //autogen
#include <bf_svx/polygn3d.hxx>
#endif
#ifndef _E3D_EXTRUD3D_HXX //autogen
#include <bf_svx/extrud3d.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX
#include "bf_svx/obj3d.hxx"
#endif

#ifndef _POLY3D_HXX
#include "bf_svx/poly3d.hxx"
#endif

#ifndef _VIEWPT3D_HXX //autogen
#include <bf_svx/viewpt3d.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SVDTRANS_HXX //autogen
#include <bf_svx/svdtrans.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <bf_svx/chrtitem.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <bf_svx/svdoutl.hxx>
#endif
#ifndef _SCH_ADJUST_HXX //autogen
#include "adjust.hxx"
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <bf_svx/svdotext.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <bf_svx/svdmodel.hxx>
#endif
#ifndef _PERSIST_HXX //autogen
#include <bf_so3/persist.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <bf_sfx2/objsh.hxx>
#endif
#ifndef _DEFINES_HXX
#include "defines.hxx"
#endif
#include "chartbar.hxx"

#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
class Vector3D;

namespace binfilter {

class SvNumberFormatter;
class SvNumberFormatterIndexTable;
class SdrObjGroup;
class SdrRectObj;

class SchModelData;
class SchObjectId;
class ChartAxis;
class SchItemPool;
class SchMemChart;
class ChartScene;
class E3dPolyObj;
class Polygon3D;
class E3dCompoundObject;
class E3dObject;
class SfxPrinter;
class E3dLabelObj;
class SchDataLogBook;
class E3dLabelObj;
struct DataDescription;
class SchRectObj;

DECLARE_LIST(ItemSetList, SfxItemSet*)//STRIP008 DECLARE_LIST(ItemSetList, SfxItemSet*);
DECLARE_LIST(DescrList, E3dLabelObj*)//STRIP008 DECLARE_LIST(DescrList, E3dLabelObj*);

#define VERTICAL_LINE aLine[0].Y() = rRect.Bottom(); aLine[1].Y() = rRect.Top();
#define HORIZONTAL_LINE aLine[0].X() = rRect.Left(); aLine[1].X() = rRect.Right();
#define LONGMARKS ((nMarkLen * 3) / 2)
#define TEXTHEIGHT_OFS 2
#define DEFAULT_COLCNT 3
#define DEFAULT_ROWCNT 4

// base diagram types
#define CHTYPE_INVALID			0

#define CHTYPE_LINE				1
#define CHTYPE_LINESYMB			2	// this one has to be removed !! (why?)
#define CHTYPE_AREA				3
#define CHTYPE_COLUMN			4	// is also a bar-type
#define CHTYPE_BAR				5
#define CHTYPE_CIRCLE			6
#define CHTYPE_XY				7
#define CHTYPE_NET				8
#define CHTYPE_DONUT			9
#define CHTYPE_STOCK		   10
#define CHTYPE_ADDIN		   11

#define SETLINES_BLACK			1
#define SETLINES_FILLCOLOR		2
#define SETLINES_COMPAT			3
#define SETLINES_REVERSE		4

#define SYMBOLMODE_LEGEND       1   // symbol for leged (default)
#define SYMBOLMODE_DESCRIPTION  2   // symbol for series data description
#define SYMBOLMODE_LINE         3   // unused (line as symbol)
#define SYMBOLMODE_ROW          4   // unused (symbol for series)

#define   SETFLAG( status, flag )   (status)|= (flag)
#define RESETFLAG( status, flag )   (status) = ((status) | (flag)) - (flag)
#define ISFLAGSET( status, flag )	(((status) & (flag)) != 0)

#define CHS_USER_QUERY			0	// ask for sorting data for xy charts
#define CHS_USER_NOQUERY		1	// do not ask for sorting
#define CHS_KEEP_ADDIN			2	// if this flag is set a change of chart
                                    // type keeps the reference to the AddIn
#define CHS_NO_ADDIN_REFRESH	4	// used to avoid recursion in BuildChart

#define CHART_NUM_SYMBOLS		8	// number of different polygons as symbols

#define CHART_AXIS_PRIMARY_X	1	// never change these defines (persistent) !!!
#define CHART_AXIS_PRIMARY_Y	2
#define CHART_AXIS_PRIMARY_Z	3
#define CHART_AXIS_SECONDARY_Y	4
#define CHART_AXIS_SECONDARY_X	5

#define CHART_TRACE(  w )			DBG_TRACE1( "SCH(%lx):" #w, (long)this )
#define CHART_TRACE1( w,a )			DBG_TRACE2( "SCH(%lx):" #w, (long)this, a )
#define CHART_TRACE2( w,a,b )		DBG_TRACE3( "SCH(%lx):" #w, (long)this, a,b )
#define CHART_TRACE3( w,a,b,c )		DBG_TRACE4( "SCH(%lx):" #w, (long)this, a,b,c )
#define CHART_TRACE4( w,a,b,c,d )	DBG_TRACE5( "SCH(%lx):" #w, (long)this, a,b,c,d )

class ChartModel : public SdrModel
{
public:
    // -------
    // methods
    // -------

                        ChartModel( const String& rPalettePath, SfxObjectShell* pDocSh );
    virtual            ~ChartModel();

    // SdrModel
    virtual void        SetChanged( FASTBOOL bFlag = TRUE );
    virtual SdrModel *  AllocModel() const;
    virtual SdrPage  *  AllocPage( FASTBOOL bMasterPage );

    void                NewOrLoadCompleted( USHORT eMode );
    void                AdjustPrinter();

    BOOL                IsInitialized() const;
    void                Initialize();
    void                InitChartData( BOOL bNewTitles = TRUE );

    void                BuildChart( BOOL bCheckRange,
                                    long whatTitle = 0 );
    SdrObjGroup *       CreateChart( const Rectangle &  rRect );

    BOOL                ChangeChart( SvxChartStyle eStyle, bool bSetDefaultAttr = true );
    BOOL                ChangeChartData( SchMemChart &   rData,
                                         BOOL            bNewTitles = TRUE,
                                         BOOL            bDontBuild = FALSE );

    void                InitDataAttrs();
    void                SetChartData( SchMemChart &  rData,
                                      BOOL           bNewTitles = TRUE );
    SchMemChart *       GetChartData()  const;
    /// @descr avoid BuildChart invocation during edit. remember changes in data for later change (SP2) #61907#
    void                SetChartDataBuffered( SchMemChart &  rData,
                                              BOOL           bNewTitles = TRUE );

    double              GetData( long   nCol,
                                 long   nRow,
                                 BOOL   bPercent = FALSE,
                                 BOOL   bRowData = TRUE )   const;
    long                GetColCount() const;
    long                GetRowCount() const;

    BOOL                ChangeSwitchData(BOOL bSwitch);
    inline BOOL         IsSwitchData() const;
    /// dependent of chart type (donut => toggle result)
    inline BOOL         IsDataSwitched() const;

    void                SetTextFromObject( SdrTextObj* pObj, OutlinerParaObject* pTextObject );
    void                PrepareOld3DStorage();
    void                CleanupOld3DStorage();



    BOOL                IsReal3D()const
                            { return (Is3DChart() /*&& !IsPieChart()*/); }
    BOOL                HasBarConnectors( long nRow ) const
                            { return (BOOL)(m_nDefaultColorSet&CHSPECIAL_TRACELINES); }
    inline void         SetBarConnectors( BOOL bOn, long nRow );

    BOOL                IsCol( long nRow ) const;
    BOOL                IsArea( long nRow );

    /// return TRUE if the current chart type supports a given axis type
    BOOL                CanAxis( long nAxisId ) const;

    SdrObject*          CreateSymbol( Point      aPoint,
                                      int        nRow,
                                      int        nColumn,
                                      SfxItemSet &aDataAttr,
                                      long       nSymbolSize,
                                      BOOL       bInsert = TRUE );

    void                GenerateSymbolAttr( SfxItemSet&  rSymbolAttr,             // #63904#
                                            const long   nRow,
                                            const long   nMode = SYMBOLMODE_LEGEND );
    ChartAxis*          GetAxisByUID( long nUId );
    void                PrepareAxisStorage();
    ChartAxis*          GetAxis( long nId );
    BOOL                HasSecondYAxis() const;
    /// is the x-axis vertical, ie we have bars not columns
    BOOL                IsXVertikal() const { return IsBar(); }

    void                DrawStockBars( SdrObjList* pList, const Rectangle& rRect );
    BOOL                HasStockBars( SvxChartStyle* pStyle = NULL );
    void                DrawStockLines(SdrObjList* pList,const Rectangle& rRect);
    BOOL                HasStockLines( SvxChartStyle* pStyle=NULL );
    BOOL                HasStockRects( SvxChartStyle* pStyle=NULL );

    /// core methods for chart::XChartDocument / frame::XModel
    void                LockBuild();
    void                UnlockBuild();
    BOOL                IsLockedBuild();

    long                GetChartShapeType();

    void                SetAttributes( const long         nId,
                                       const SfxItemSet&  rAttr,
                                       BOOL               bMerge = TRUE );
    void                SetAutoPilot( Window *pWindow )    { pAutoPilot = pWindow; }            // #46895#

    void                Create2DXYTitles( Rectangle& rRect, BOOL bSwitchColRow );

    // status flag methods
    void                SetChartStatus( const long  nStatus )	{ nChartStatus = nStatus; }
    void                SetChartStatusFlag( const long flag )	{ SETFLAG( nChartStatus, flag ); }
    void                ResetChartStatusFlag( const long flag )	{ RESETFLAG( nChartStatus, flag ); }
    long                GetChartStatus()						{ return nChartStatus; }
    BOOL                GetChartStatusFlag( const long flag )	{ return ISFLAGSET( nChartStatus, flag ); }

    void                SetupLineColors( const long nMode, long nStartIndex = 0 );    //#54870#

    //Einige (bald alle) Objekte setzen durch Implementation der NbcSetAttributes-Methode
    //die ChartModel-Attributsspeicher automatisch. Dies ist nicht immer notwendig und
    //sollte dann abgeschaltet werden, z.B. während des BuildChart

    /** Most chart objects have overloaded NbcSetAttributes methods for setting
        draw items at the object specific item sets. This is not always
        necessary and can be disabled with this method.
        @returns value that was valid before
    */
    inline BOOL         SetAttrAutoStorage( const BOOL b );

    void                SetTitle( const long nId, const String& rTitle );
    String              GetTitle( const long nId );	// BM
    void                SetHasBeenMoved( const long nId, BOOL bMoved = TRUE );
    SdrObject*          GetObjectWithId( const long nId, const long nCol = 0, const long nRow = 0 );

    void                StoreObjectsAttributes( SdrObject*         pObj,
                                                const SfxItemSet&  rAttr,
                                                FASTBOOL           bReplaceAll );      // #52277#

    void			    GetAttr( const long nObjId, SfxItemSet& rAttr, const long nIndex1 = -1 );
    SfxItemSet&		    GetAttr( const long nObjId, const long nIndex1 = -1) const;
    BOOL                ChangeAttr( const SfxItemSet& rAttr, const long nId, const long nIndex1 = -1 );

    /// after binary load restore non-persistent 3d items from scene (is stored completely)
    void                Rescue3DObjAttr( SdrObjList* pList );      // #52277#

    /** set number of lines in a bar/line combination chart. This value
        determines the number of series that should be represented as line
        beginning from the last series, eg when set to 2 the last two series
        are lines. This is not availably via GUI, but only via API
    */
    void                SetNumLinesColChart( const long nSet, BOOL bForceStyleChange = FALSE );
    long                GetNumLinesColChart() const    { return nNumLinesInColChart; } // #50212#

    inline void         SetBarPercentWidth( const long nWidth );
    long                GetBarPercentWidth() const     { return nBarPercentWidth; }    // #50116#

    long                GetDefaultColorSet() const     { return m_nDefaultColorSet; }  // #50037#

    /** set an item to the given item set
        this method is necessary, because for some items (gradients, hatches etc.)
        special handling is needed (names have to be generated for later XML export)

        used by PutItemSetWithNameCreation() */
    void                SetItemWithNameCreation( SfxItemSet& rDestItemSet, const SfxPoolItem* pNewItem );

    /** Put the new item set into the destination set and generate names for attributes
        that need this (gradients, hatches etc.)
     */
    void                PutItemSetWithNameCreation( SfxItemSet& rDestItemSet, const SfxItemSet& rNewItemSet );

    /** @descr Set the attributes that are used by CalcTextSizeOfOneText.
               Because the setting of the attributes is rather expensive,
               better set them once, outside of loops and pass
               bSetTextAttributes=FALSE to CalcTextSizeOfOneText.
        @param rTextAttributes  These attributes are set to the outliner object pOutliner.
        @see   CalcTextSizeOfOneText */
    void                SetTextAttributes( SfxItemSet & rTextAttributes );

    SdrRectObj*         CreateTextObj( UINT16           nId,
                                       const Point      &rPos,
                                       const String     &rText,
                                       const SfxItemSet &rAttr,
                                       BOOL             bIsTitle,
                                       ChartAdjust      eAdjust = CHADJUST_TOP_LEFT,
                                       const long       nMaximumWidth = -1);

    Size                CalcMaxDescrSize( BOOL					 bRowDescr,
                                          SvxChartTextOrient	 eOrient,
                                          const UINT32			 nNumberFormat,
                                          long					 nAxisUId,
                                          const long			 MaximumWidth  = -1,
                                          Pair*                  pFirstAndLast = NULL );


    double              GetVariantY( long nRow );
    double              GetSigmaY( long nRow );
    double              GetBigErrorY( long nRow, double fError );


    /// returns TRUE, if change requires BuildChart - currently always TRUE !
    BOOL                IsAttrChangeNeedsBuildChart( const SfxItemSet& rAttr );


    BOOL                SetBaseType( long nBaseType );

    // chart features (see chtmode7.cxx)
    long                GetBaseType()                     const;

    BOOL                IsPercent()                       const;
    BOOL                IsStacked()                       const;
    BOOL                IsBar()                           const;
    BOOL                IsPieChart()                      const;
    BOOL                Is3DChart()                       const;
    BOOL                IsNetChart()                      const;
    BOOL                IsStackedChart()                  const;
    BOOL                IsPercentChart()                  const;
    BOOL                IsAxisChart()                     const;
    BOOL                IsSplineChart()                   const;
    inline BOOL         IsDonutChart()                    const;

    // features for series
    BOOL                HasSymbols( const long nRow = 0 ) const;
    BOOL                IsLine( const long nRow = 0 )     const;

    // chart features for current chart (default) or the given type
    BOOL                IsNegativeChart( SvxChartStyle* pStyle = NULL ) const;
    BOOL                IsSignedChart( SvxChartStyle* pStyle = NULL )   const;
    BOOL                IsXYChart( SvxChartStyle* pStyle = NULL )       const;




    SfxItemPool&        GetPool()                 { return *pItemPool; }
    SfxObjectShell*     GetObjectShell()          { return pDocShell;  }

    ProjectionType      GetProjection()           { return eProjection; }

        SdrObject*          GetDataPointObj( long nCol, long nRow );


        BOOL                HasAxis( long nObjectId = CHOBJID_ANY ) const;

        const SfxItemSet &  GetLegendAttr() const;

    SfxItemSet          GetFullLegendAttr() const;

       BOOL                ChangeAxisAttr( const SfxItemSet &  rAttr,
                                       SdrObjGroup       *  pAxisObj,
                                        BOOL                bMerge = TRUE );

    SfxItemSet          GetFullAxisAttr( const SdrObjGroup * pAxisObj, bool bOnlyInserted = false ) const;

        void                PutDataRowAttrAll( const SfxItemSet &  rAttr,
                                            BOOL                bMerge = TRUE,
                                            BOOL                bClearPoints = TRUE );
        void                PutDataRowAttr( long                nRow,
                                         const SfxItemSet &  rAttr,
                                         BOOL                bMerge = TRUE,
                                         BOOL                bClearPoints = TRUE );

    const SfxItemSet &  GetDataRowAttr( long nRow ) const;

        void                PutDataPointAttr( long                nCol,
                                           long                nRow,
                                           const SfxItemSet &  rAttr,
                                           BOOL                bMerge = TRUE );

    const SfxItemSet &  GetDataPointAttr( long nCol,long nRow ) const;
    void                ClearDataPointAttr( long nCol, long nRow, const SfxItemSet & rAttr );
    SfxItemSet          GetFullDataPointAttr( long nCol, long nRow ) const;
    SfxItemSet &        MergeDataPointAttr( SfxItemSet & rAttr, long nCol, long nRow) const;

    /**	@descr	Return the raw attributes for the specified data point.  The returned
            pointer is NULL if the attributes have not been explicitely set (that is
            the normal case).  The attributes of the data row are not merged in.
        @param	nCol	Column of the data point.
        @param	nRow	Row of the data point.
        @return	The returned value is either a pointer to an item set or NULL if no
            attributes for this specific data point have been set or the given
            coordinates are not valid.
    */
    const SfxItemSet *  GetRawDataPointAttr	(long nCol,long nRow) const;

    /**	@descr	Attributes for single data points are only stored explicitely if they
            differ from those of their data rows.  This method returns a flag that
            indicates wether there exists an item set for the specified data point.
        @param	nCol	Column of the data point.
        @param	nRow	Row of the data point.
        @return	When an item set for the specified data point exists then TRUE is
            returned.
    */
    BOOL                IsDataPointAttrSet( long nCol, long nRow )  const;


    void                ChangeDataDescr( SvxChartDataDescr eDescr,
                                         BOOL              bSym,
                                         long              nRowToChange = -1,
                                         BOOL              bBuildChart = TRUE );

    BOOL                ResizePage( const Size & rNewSize );

    SvNumberFormatter * GetNumFormatter() const;

    SdrOutliner *       GetOutliner() const;


    void                GetAttr( SfxItemSet & rAttr );
    void                PutAttr( const SfxItemSet & rAttr );

    ChartScene *        GetScene();



    BOOL                ChangeStatistics( const SfxItemSet &  rNewAttr );

    void                SetShowLegend( BOOL bNewShow );

    // ToDo: Remove duplicate method declaration where possible
    inline BOOL &       TextScalability();
    inline BOOL         TextScalability() const;

    inline BOOL &       ShowAverage();
    inline BOOL         ShowAverage() const;

    inline BOOL &       IsCopied ();
    inline BOOL         IsCopied() const;








    int &               Granularity();

    SvxChartStyle &     ChartStyle();
    SvxChartStyle       ChartStyle() const;


    long                PieSegOfs( long nCol ) const;

    BOOL &              ShowMainTitle();

    String &            MainTitle ();

    BOOL &              ShowSubTitle();

    String &            SubTitle();

    BOOL &              ShowXAxisTitle();

    String &            XAxisTitle();

    BOOL &              ShowYAxisTitle();

    String &            YAxisTitle();

    BOOL &              ShowZAxisTitle();

    String &            ZAxisTitle();

    BOOL &              ShowXGridMain();

    BOOL &              ShowXGridHelp();

    BOOL &              ShowYGridMain();

    BOOL &              ShowYGridHelp();

    BOOL &              ShowZGridMain();

    BOOL &              ShowZGridHelp();



    BOOL &              ReadError ();


    Size &              InitialSize();




    int &               SplineDepth()                   { return nSplineDepth; }

    String &            ColText( long nCol );

    String &            RowText( long nRow );


    long                GetAxisUID( long nRow );

    void                SetShowDataDescr( const BOOL b ) { bShowDataDescr = b; }

    BOOL                ShowXDescr() const;
    void                ShowXDescr( BOOL b );
    BOOL                ShowYDescr() const;
    void                ShowYDescr( BOOL b );
    BOOL                ShowZAxis() const;
    void                ShowZAxis( BOOL b );
    BOOL                ShowXAxis() const;
    void                ShowXAxis( BOOL b );
    BOOL                ShowYAxis() const;
    void                ShowYAxis( BOOL b );
    BOOL                ShowZDescr() const;
    void                ShowZDescr( BOOL b );

    UINT32              GetNumFmt( long nObjId, BOOL bPercent );
    void                SetNumFmt( long nObjId, UINT32 nFmt, BOOL bPercent );

    BOOL                CheckForNewAxisNumFormat();
    BOOL                UsesOwnNumberFormatter()             { return ( pNumFormatter == pOwnNumFormatter ); }

    void                SetSpotColor( const Color & rCol )   { aSpotColor = rCol; }

    void                SetAmbientIntensity( const double fIntensity )  { fAmbientIntensity = fIntensity; }
    void                SetAmbientColor( const Color& rCol )            { aAmbientColor = rCol; }

    /// @descr FG: Is called from ChartScene::FitInSnapRect only. bSwitch3DRowCol has to be TRUE (?)
    void                Position3DAxisTitles( const Rectangle & rXDescrOutRect );

    void                SetPieSegOfs( long  nCol,
                                      long  nOfs );

    void                ClearItemSetLists();


    // FG: Diese Abfrage soll ermoeglichen, dass man den linken und den rechten Rand
    //     Notfalls nachregeln kann, wenn die Beschriftung unter den Datenpunkten zentriert ist,
    //     da der erste Datenpunkt auf der Y-Achse liegt und der letzte Datenpunkt meist genau
    //     da, wo das Diagramm aufhoert.
    BOOL                IsDescriptionCenteredUnderDataPoint();

    inline BOOL         GetFormatXAxisTextInMultipleLinesIfNecessary();
    inline void         SetFormatXAxisTextInMultipleLinesIfNecessary( BOOL value );

    inline BOOL         GetFormatYAxisTextInMultipleLinesIfNecessary();
    inline void         SetFormatYAxisTextInMultipleLinesIfNecessary( BOOL value );

    inline BOOL         GetFormatZAxisTextInMultipleLinesIfNecessary();
    inline void         SetFormatZAxisTextInMultipleLinesIfNecessary( BOOL value );

    inline BOOL         GetFormatLegendTextInMultipleLinesIfNecessary();
    inline void         SetFormatLegendTextInMultipleLinesIfNecessary( BOOL value );

    inline BOOL         GetUseRelativePositions()                       { return  bUseRelativePositionsForChartGroups; }
    void                SetUseRelativePositions( BOOL value );

    void                SetAdjustMarginsForLegend( BOOL value )         { bAdjustMarginsForLegend = value; }
    BOOL                GetAdjustMarginsForLegend()                     { return bAdjustMarginsForLegend; }

    void                SetAdjustMarginsForMainTitle( BOOL value )      { bAdjustMarginsForMainTitle = value; }
    BOOL                GetAdjustMarginsForMainTitle()                  { return bAdjustMarginsForMainTitle; }

    void                SetAdjustMarginsForSubTitle( BOOL value )       { bAdjustMarginsForSubTitle = value; }
    BOOL                GetAdjustMarginsForSubTitle()                   { return bAdjustMarginsForSubTitle; }

    void                SetAdjustMarginsForXAxisTitle( BOOL value )     { bAdjustMarginsForXAxisTitle = value; }
    BOOL                GetAdjustMarginsForXAxisTitle()                 { return bAdjustMarginsForXAxisTitle; }

    void                SetAdjustMarginsForYAxisTitle( BOOL value )     { bAdjustMarginsForYAxisTitle = value; }
    BOOL                GetAdjustMarginsForYAxisTitle()                 { return bAdjustMarginsForYAxisTitle; }

    void                SetAdjustMarginsForZAxisTitle( BOOL value )     { bAdjustMarginsForZAxisTitle = value; }
    BOOL                GetAdjustMarginsForZAxisTitle()                 { return bAdjustMarginsForZAxisTitle; }

    BOOL                GetDiagramHasBeenMovedOrResized()               { return bDiagramHasBeenMovedOrResized; }
    void                SetDiagramHasBeenMovedOrResized( BOOL value )   { bDiagramHasBeenMovedOrResized=value; }

    BOOL                GetMainTitleHasBeenMoved()                      { return bMainTitleHasBeenMoved; }
    void                SetMainTitleHasBeenMoved( BOOL value )          { bMainTitleHasBeenMoved = value; }

    BOOL                GetSubTitleHasBeenMoved()                       { return bSubTitleHasBeenMoved; }
    void                SetSubTitleHasBeenMoved( BOOL value )           { bSubTitleHasBeenMoved = value; }

    BOOL                GetLegendHasBeenMoved()                         { return bLegendHasBeenMoved; }
    void                SetLegendHasBeenMoved( BOOL value )             { bLegendHasBeenMoved = value; }

    BOOL                GetXAxisTitleHasBeenMoved()                     { return bXAxisTitleHasBeenMoved; }
    void                SetXAxisTitleHasBeenMoved( BOOL value )         { bXAxisTitleHasBeenMoved = value; }

    BOOL                GetYAxisTitleHasBeenMoved()                     { return bYAxisTitleHasBeenMoved; }
    void                SetYAxisTitleHasBeenMoved( BOOL value )         { bYAxisTitleHasBeenMoved = value; }

    BOOL                GetZAxisTitleHasBeenMoved()                     { return bZAxisTitleHasBeenMoved; }
    void                SetZAxisTitleHasBeenMoved( BOOL value )         { bZAxisTitleHasBeenMoved = value; }

    inline void         SetDiagramRectangle( const Rectangle &rNewRect, bool bStoreLast = true );
    void                SetChartRect( Rectangle &rValue )               { aChartRect = rValue; }
    Rectangle &         GetChartRect()                                  { return aChartRect; }
    void                SetLegendPos( const Point& rPos )               { aLegendTopLeft = rPos; }

    long                GetPieRadius() const				{ return nPieRadius; }
    BOOL                HasDefaultGrayArea( SvxChartStyle* pStyle = NULL ) const;
    BOOL                HasDefaultGrayWall( SvxChartStyle* pStyle = NULL ) const;

    void                SetNumberFormatter( SvNumberFormatter* );
    void                DataRangeChanged( long _nOldRowCnt = 0, long _nOldColCnt = 0 );
    void                TranslateAllNumFormatIds( SvNumberFormatterIndexTable* );

    /** set new doc shell if there was no one before
        returns true if new shell was set */
    BOOL                SetObjectShell( SfxObjectShell* pDocSh );

    void                SetChartAddIn( ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable >& xChartAddin );
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable > GetChartAddIn() const { return mxChartAddIn; }

    /// fills the itemset with an intersection of all data row attributes
    void                GetDataRowAttrAll( SfxItemSet & rOutAttributes );

    LanguageType        GetLanguage( const USHORT nId ) const;
    void                SetLanguage( const LanguageType eLang, const USHORT nId );

    virtual void        HandsOff();

    /** The outer sequence contains a sequence for each series.  The inner sequence may be empty
        (default) or contains a list of data-point indices that have attributes (items) differing
        from the series attributes.
    */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > > GetSetDataPointList();

    /** Whenever the page color changes, the outliner has to be
        informed about that for auto-colors of text-objects.

        Note: The page of a chart is always completely covered be the
        chart area, thus this method should always be called when the
        background of the chart area changes.
     */
    void PageColorChanged( const SfxItemSet& rItems );

    /** introduced for #101318#:

        The chart-internal number formatter is necessary to render values for
        the UI with a higher standard precision than the one you might get
        externally (from Calc).  As you should not modify the precision of an
        external number formatter, this one comes in handy.

        Note that for the dialog only the standard format is used, i.e., any
        user-defined formats that are only available in the external formatter
        are not required here.

        This method is used by SchAttribTabDlg::PageCreated (attrib.cxx)
    
        @returns the chart internal number formatter
     */


    Rectangle	GetDiagramRectangle() { return aDiagramRectangle; }

    DECL_LINK( NotifyUndoActionHdl, SfxUndoAction* );
    SfxUndoAction* GetAndReleaseUndoActionFromDraw() { SfxUndoAction* pRet = m_pUndoActionFromDraw; m_pUndoActionFromDraw = NULL; return pRet;}
    void SetDeleteUndoActionNotificationFromDraw(BOOL bDelete) { m_bDeleteUndoActionNotificationFromDraw=bDelete;}

    // friend declarations
    friend SvStream &   operator << ( SvStream & rOut, const ChartModel & rDoc );
    friend SvStream &   operator >> ( SvStream & rIn, ChartModel & rDoc );

    friend class SchModelData;

// protected:
    // remark: currently there are no classes that are derived from ChartModel
    // and also the distinction between private and protected was not very
    // obvious. So there is no protected section any more

private:

    // -------
    // members
    // -------

    ChartBarDescriptor  aBarY1;
    ChartBarDescriptor  aBarY2;

    DescrList           aXDescrList;
    DescrList           aYDescrList;
    DescrList           aZDescrList;

    BOOL                bClearDepth;
    BOOL                bNewOrLoadCompleted;
    SfxObjectShell*     pDocShell;
    BOOL                bAttrAutoStorage;

    SchMemChart*        pChartDataBuffered; 	// save data in buffer while editing chart #61907#

    OutputDevice*       pChartRefOutDev;
    long                nChartStatus;

    Window*             pAutoPilot;             // #46895#

    SdrObjList*         pSdrObjList;
    Matrix4D            aSceneMatrix;
    BOOL                bResizePie;
    long                nPieRadius;

    SvNumberFormatter*  pOwnNumFormatter;
    SvNumberFormatter*  pNumFormatter;		// points either to pOwnNumFormatter or calc's number formatter

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable > mxChartAddIn;

    long                nBarPercentWidth;       // #50116#
    long                nNumLinesInColChart;
    long                m_nDefaultColorSet;     // #50037#

    SchDataLogBook*     pLogBook;
    Rectangle           aChartRect;
    Size                aInitialSize;

    SchItemPool*        pChItemPool;
    ChartScene*         pScene;

    Vector3D*           aLightVec;

    SchMemChart*        pChartData;
    double              fMinData;
    double              fMaxData;
    double              fAmbientIntensity;
    Color               aAmbientColor;
    double              fSpotIntensity;
    Color               aSpotColor;
    SvxChartStyle       eChartStyle;
    SvxChartStyle       eOldChartStyle;
    int                 eChartLinePoints[ LINE_POINT_COUNT ];

    List*               pDefaultColors;

    BOOL                bTextScalable;
    BOOL                bIsCopied;
    BOOL                bLegendVisible;
    BOOL                bShowAverage;
    SvxChartKindError   eErrorKind;
    SvxChartIndicate    eIndicate;
    SvxChartRegress     eRegression;
    double              fIndicatePercent;
    double              fIndicateBigError;
    double              fIndicatePlus;
    double              fIndicateMinus;
    int                 nSplineDepth;
    int                 nGranularity;

    /** This member is a temporary storage. I don't understand the comment, so
        I can't translate it into english, so I leave it in german, maybe an important hint someday:

        FG: Reiner Zwischenspeicher, wird in chtmod3d.cxx gesetzt und im selben
        File in Position3DAxisTitles im selben File abgefragt, weil die
        ChartScene::FitInSnapRect das nicht als Parameter bekommen kann, da
        diese von der Polyscene gerufen wird.  */
    BOOL                bSwitch3DColRow;

    long                nMarkLen;
    long                nPieHeight;
    long*               pPieSegOfs;
    short               nPieSegCount;
    short               nXAngle;
    short               nYAngle;
    short               nZAngle;

    BOOL                bCanRebuild;

    BOOL                bShowMainTitle;
    BOOL                bShowSubTitle;
    BOOL                bShowXAxisTitle;
    BOOL                bShowYAxisTitle;
    BOOL                bShowZAxisTitle;

    String              aMainTitle;
    String              aSubTitle;
    String              aXAxisTitle;
    String              aYAxisTitle;
    String              aZAxisTitle;

    BOOL                bShowXGridMain;
    BOOL                bShowXGridHelp;
    BOOL                bShowYGridMain;
    BOOL                bShowYGridHelp;
    BOOL                bShowZGridMain;
    BOOL                bShowZGridHelp;

    BOOL                bShowDataDescr; // show description for all series (esp. important for auto pilot)

    SfxItemSet*         pTitleAttr;
    SfxItemSet*         pMainTitleAttr;
    SfxItemSet*         pSubTitleAttr;
    SfxItemSet*         pXAxisTitleAttr;
    SfxItemSet*         pYAxisTitleAttr;
    SfxItemSet*         pZAxisTitleAttr;
    SfxItemSet*         pAxisAttr;

    ChartAxis*          pChartXAxis;
    ChartAxis*          pChartYAxis;
    ChartAxis*          pChartZAxis;
    ChartAxis*          pChartAAxis;    // secondary x axis
    ChartAxis*          pChartBAxis;    // secondary y axis

    SfxItemSet*         pGridAttr;
    SfxItemSet*         pXGridMainAttr;
    SfxItemSet*         pYGridMainAttr;
    SfxItemSet*         pZGridMainAttr;
    SfxItemSet*         pXGridHelpAttr;
    SfxItemSet*         pYGridHelpAttr;
    SfxItemSet*         pZGridHelpAttr;
    SfxItemSet*         pDiagramAreaAttr;
    SfxItemSet*         pDiagramWallAttr;
    SfxItemSet*         pDiagramFloorAttr;
    SfxItemSet*         pLegendAttr;
    SfxItemSet*         pDummyAttr;

    SfxItemSet*         pStockLineAttr;
    SfxItemSet*         pStockLossAttr;
    SfxItemSet*         pStockPlusAttr;


    SfxItemSet*         pTmpXItems;
    SfxItemSet*         pTmpYItems;
    SfxItemSet*         pTmpZItems;

    SfxItemSet*         pChartAttr;

    ItemSetList         aDataRowAttrList;
    ItemSetList         aDataPointAttrList;
    ItemSetList         aSwitchDataPointAttrList;
    ItemSetList         aRegressAttrList;
    ItemSetList         aAverageAttrList;
    ItemSetList         aErrorAttrList;

    // this is for 'old 3d storage' (whatever that is ;-)
    ItemSetList         aTmpDataRowAttrList;
    ItemSetList         aTmpDataPointAttrList;
    ItemSetList         aTmpSwitchDataPointAttrList;


    SvxChartDataDescr   eDataDescr;
    BOOL                bShowSym;
    BOOL                bSwitchData;

    BOOL                bNoBuildChart;          // BuildChart does nothing if this is true
    BOOL                bShouldBuildChart;      // This is set when BuildChart was called and bNoBuildChart was TRUE
    BOOL                bReadError;
    BOOL                mbIsInitialized;

    SdrOutliner*        pOutliner;

    /// short: textbreak enabled
    BOOL                bFormatXAxisTextInMultipleLinesIfNecessary;
    BOOL                bFormatYAxisTextInMultipleLinesIfNecessary;
    BOOL                bFormatZAxisTextInMultipleLinesIfNecessary;
    BOOL                bFormatLegendTextInMultipleLinesIfNecessary;

    /// maximum number of lines for textbreak
    int                 nXAxisTextMaximumNumberOfLines;
    int                 nYAxisTextMaximumNumberOfLines;
    int                 nZAxisTextMaximumNumberOfLines;

    long                nWidthOfFirstXAxisText;
    long                nWidthOfLastXAxisText;

    // positions of chart objects
    Point               aTitleTopCenter;
    Point               aSubTitleTopCenter;
    Rectangle           aDiagramRectangle;
    Rectangle           aLastDiagramRectangle;  // ?
    Point               aLegendTopLeft;

    Point               aTitleXAxisPosition;
    Point               aTitleYAxisPosition;
    Point               aTitleZAxisPosition;

    ChartAdjust         eAdjustXAxesTitle;
    ChartAdjust         eAdjustYAxesTitle;
    ChartAdjust         eAdjustZAxesTitle;

    // FG: Falls dieser gesetzt ist so wird bei einem Resize (eigentlich bei jedem Buildchart)
    //     die relative Position der Gruppenobjekte zur Seite beachtet.
    //     Je nach Objekt bleibt die linke obere Ecke oder das Zentrum an der gleichen Stelle.

    /** On resize objects remember their position relatively to the page if
        this member is TRUE.  For some objects the upper left corner is
        preserved for others the center position */
    BOOL                bUseRelativePositionsForChartGroups;

    /// on manual move of chart objects the calculation of space left has to be changed
    BOOL                bAdjustMarginsForLegend;
    BOOL                bAdjustMarginsForMainTitle;
    BOOL                bAdjustMarginsForSubTitle;
    BOOL                bAdjustMarginsForXAxisTitle;
    BOOL                bAdjustMarginsForYAxisTitle;
    BOOL                bAdjustMarginsForZAxisTitle;

    /// keep track if objects have manually been repositioned
    BOOL                bDiagramHasBeenMovedOrResized;
    BOOL                bMainTitleHasBeenMoved;
    BOOL                bSubTitleHasBeenMoved;
    BOOL                bLegendHasBeenMoved;
    BOOL                bXAxisTitleHasBeenMoved;
    BOOL                bYAxisTitleHasBeenMoved;
    BOOL                bZAxisTitleHasBeenMoved;

    Size                aInitialSizefor3d;

    /** test object for calculating the height of two rows
        @see GetHeightOfnRows */
    SdrRectObj*         pTestTextObj;

    INT32               nXLastNumFmt;
    INT32               nYLastNumFmt;
    INT32               nBLastNumFmt;

    /// document languages
    LanguageType	    eLanguage;
    LanguageType	    eLanguageCJK;
    LanguageType	    eLanguageCTL;

    ProjectionType      eProjection;

    /// for late loading of graphics
    mutable SotStorage*        	 mpDocStor;
    mutable SotStorageRef        mxPictureStorage;
    mutable SotStorageStreamRef  mxDocStream;


    SfxUndoAction*	m_pUndoActionFromDraw;
    BOOL	m_bDeleteUndoActionNotificationFromDraw;

    // -------
    // methods
    // -------

    bool                UsesSourceFormat( long nAxisUID, SfxItemSet** pItemSetPointer = NULL );

    USHORT              GetRegressStrId( long nRow );
    void                ResizeText( SfxItemSet *pTextAttr, Size aPageSize, BOOL bResizePage=TRUE );

    void                DeleteObject( SdrObject* pObj );


    void                               SetTextAttr( SdrTextObj       &rTextObj,
                                                    const SfxItemSet &rAttr,
                                                    const long       nMaximumWidth = -1 );

    /**	@descr	This method calculates the size of the bounding box of the text that is already
            set at the outliner pOutliner.  If MaximumWidth > 0 then the text will be split
            (umgebrochen) exactly once at this width.
            [Falls der Schalter ChangeBorder auf TRUE steht wird der untere
            Rand nachgeregelt, dass muss uber den Schalter passieren, da sonst nicht
            zwischen reinen Abfragen und einer Abfrage um eine Aenderung zu erzielen
            unterschieden werden kann.]
        @param	eOrient	Distinguishes between horizontal and stacked vertical text orientation.
        @param	rTextAttr	Text attributes determining the text appearence.
        @param	pOutliner	This object does the actual calculation of the bbox size.
        @param	MaximumWidth	Width of predefined text split (if >0).
        @param	bGetRotated	If TRUE then the rotated text (according to the current text rotation)
            is used for calculating the bounding box.
        @param	bSetTextAttributes	Only if this flag is TRUE then the attributes rTextAttr are
            set to the given outliner.  Otherwise they are ignored.  This is used to move the
            expensive setting of attributes out out loops while at the same time keep the method's
            signiture (almost) unchanged.
        @see	SetTextAttributes
    */
    Size                CalcTextSizeOfOneText( SvxChartTextOrient eOrient,
                                               SfxItemSet &       rTextAttr,
                                               SdrOutliner *      pOutliner,
                                               long               MaximumWidth,
                                               BOOL               bGetRotated = FALSE,
                                               BOOL               bUseTextAttributes = TRUE );

    SdrObjGroup*        CreateLegend( const Rectangle &aRect );

    void                StoreAttributes( SvStream& rOut ) const;
    void                LoadAttributes(  SvStream& rIn );

    void                Create2DBackplane( Rectangle &  rRect,
                                           SdrObjList & rObjList,
                                           BOOL         bPartDescr,
                                           USHORT       eStackMode );

    SdrObjGroup*        Create2DColChart     (Rectangle aRect);
    SdrObjGroup*        Create2DRowLineChart (Rectangle aRect);
    SdrObjGroup*        Create2DPieChart     (Rectangle aRect);
    SdrObjGroup*        Create2DDonutChart   (Rectangle aRect);
    SdrObjGroup*        Create2DNetChart     (Rectangle aRect);
    SdrObjGroup*        Create2DXYChart      (Rectangle aRect);

    void                Create3DBackplanes( Rectangle &    rRect,
                                            Vector3D       aPos,
                                            Vector3D       aSizeVec,
                                            ChartScene     &rScene,
                                            BOOL           bPartDescr,
                                            BOOL           bXLogarithm,
                                            BOOL           bYLogarithm,
                                            BOOL           bZLogarithm,
                                            USHORT         eStackMode,
                                            BOOL           bPercent,
                                            BOOL           bFlatChart,
                                            BOOL           bSwitchColRow );

    SdrObjGroup*        Create3DDeepChart   (Rectangle &rRect);
    SdrObjGroup*        Create3DFlatChart   (Rectangle &rRect);
    SdrObjGroup*        Create3DNewPieChart (Rectangle &rRect);

    SdrTextObj*         CreateTitle( SfxItemSet *    pTitleAttr,
                                     short           nID,
                                     BOOL            bSwitchColRow,
                                     const String &  rText,
                                     BOOL            bVert,
                                     ChartAdjust *   pTextDirection = NULL );

    void                CreateAndInsert3DAxesTitles( Rectangle &rRect, BOOL bSwitchColRow );

    E3dCompoundObject*  Create3DBar( Vector3D         aPos,
                                     Vector3D         aSizeVec,
                                     long             nCol,
                                     long             nRow,
                                     SfxItemSet &     rAttr,
                                     BOOL             bIsSimple,
                                     double           nMinPos = 0.0,
                                     double           nOriPos = 0.0,
                                     double           nMaxPos = 0.0 );

    E3dObject *         Create3DObject  ( UINT16 ID );
    E3dObject *         Create3DAxisObj ( UINT16 nId );
    E3dScene  *         Create3DScene   ( UINT16 nId );

    void                Create3DPolyObject( const SfxItemSet *  pAttr,
                                            E3dPolygonObj *     pMyObject,
                                            UINT16              nID,
                                            E3dObject *         pParent );

    void                Create3DExtrudePolyObj( const SfxItemSet * pAttr,
                                                E3dExtrudeObj *    pMyObject,
                                                UINT16             nID,
                                                E3dObject *        pParent );

    ChartScene*         CreateScene( const Rectangle &  rRect,
                                     const Vector3D &   aLightVec,
                                     double             fSpotIntensity,
                                     Color&             aSpotColor,
                                     double             fAmbientIntensity,
                                     Color &            aAmbientColor );

    void                SetAxisAttributes( const SfxItemSet *  pAttr,
                                           const SdrObjGroup * pAxisObj );




    void                Dirty2D( long               nRowCnt,
                                 long               nCol,
                                 SdrObjList **      pDescrLists,
                                 BOOL               bRowDescr,
                                 DataDescription *  pDescription );


    /// this method shouldn't be used because it is not axis-oriented (why does it exist then?)

    /// this one is axis-oriented (whatever that means)

    void                CreateDefaultColors();
    void                DestroyDefaultColors();




    // ChangeDataRowAttr sub methods

    // ChangeDataPointAttr sub methods


    // BuildChart sub methods
    void                CreateRectsAndTitles( long whatTitle );
    void                CalculateUpperBorder();
    void                ResizeChart( const Size& rPageSize );
    BOOL                CheckRanges( BOOL bCheckAlways );
    void                DeleteChartObjects();
    void                ScaleText( long nTitle, const Size& rPageSize );

    void                DoShowMainTitle( USHORT & rIndex, const long nYOfs);
    void                DoShowSubTitle(  USHORT & rIndex, const long nYOfs );
    void                DoShowLegend( const Rectangle & rWholeRect,
                                      const long        nXOfs,
                                      const long        nYOfs,
                                      USHORT &          rIndex );


    void                LogBookAttrData();
    void                SetDefAttrRow( SfxItemSet* pDataRowAttr, const long i );

    SdrRectObj *        CreateRect( Rectangle &  rRect,
                                    long         nCol,
                                    long         nRow,
                                    SfxItemSet & rAttr );

    SdrObject* CreatePieSegment(   SfxItemSet &  rAttr,
                                   Rectangle  &  rRect,
                                   long          nCol,
                                   long          nRow,
                                   long          nStartAngle,
                                   long          nEndAngle,
                                   long          nColCnt );

    SdrObject* CreateDonutSegment( SfxItemSet &  aAttr,
                                   Rectangle  &  aRect,
                                   ULONG         nWidth,
                                   long          nCol,
                                   long          nRow,
                                   long          nStartAngle,
                                   long          nEndAngle,
                                   long          nColCnt );

    void Position2DAxisTitles( const Rectangle &  rRect,
                               BOOL               bSwitchColRow,
                               long               nTitleLeft,
                               long               nTitleBottom );
};



// ==============
// inline methods
// ==============

inline void         ChartModel::SetBarConnectors( BOOL bOn, long nRow )
{
    m_nDefaultColorSet = bOn
        ? m_nDefaultColorSet|CHSPECIAL_TRACELINES
        :(m_nDefaultColorSet|CHSPECIAL_TRACELINES) - CHSPECIAL_TRACELINES;
}

inline BOOL         ChartModel::SetAttrAutoStorage( const BOOL b )
{
    BOOL ret = bAttrAutoStorage;
    bAttrAutoStorage = b;
    return ret;
}

// #50116#
inline void         ChartModel::SetBarPercentWidth( const long nWidth )
{
    if( nWidth <= 100 &&
        nWidth > 0 )
        nBarPercentWidth = nWidth;
}

inline BOOL & ChartModel::IsCopied()
{
    return bIsCopied;
}

inline BOOL   ChartModel::IsCopied() const
{
    return bIsCopied;
}

inline BOOL & ChartModel::TextScalability()
{
    return bTextScalable;
}

inline BOOL   ChartModel::TextScalability() const
{
    return bTextScalable;
}

inline BOOL & ChartModel::ShowAverage()
{
    return bShowAverage;
}

inline BOOL   ChartModel::ShowAverage() const
{
    return bShowAverage;
}

inline BOOL ChartModel::IsSwitchData() const
{
    return bSwitchData;
};

inline BOOL ChartModel::IsDataSwitched() const
{
    if( IsDonutChart())
        return ! bSwitchData;
    else
        return bSwitchData;
}

inline BOOL ChartModel::IsDonutChart() const
{
    return ( eChartStyle == CHSTYLE_2D_DONUT1 ||
             eChartStyle == CHSTYLE_2D_DONUT2 );
};

inline BOOL ChartModel::GetFormatXAxisTextInMultipleLinesIfNecessary()
{
    return bFormatXAxisTextInMultipleLinesIfNecessary;
}
inline void ChartModel::SetFormatXAxisTextInMultipleLinesIfNecessary( BOOL value )
{
    bFormatXAxisTextInMultipleLinesIfNecessary = value;
}

inline BOOL ChartModel::GetFormatYAxisTextInMultipleLinesIfNecessary()
{
    return bFormatYAxisTextInMultipleLinesIfNecessary;
}
inline void ChartModel::SetFormatYAxisTextInMultipleLinesIfNecessary( BOOL value )
{
    bFormatYAxisTextInMultipleLinesIfNecessary = value;
}

inline BOOL ChartModel::GetFormatZAxisTextInMultipleLinesIfNecessary()
{
    return bFormatZAxisTextInMultipleLinesIfNecessary;
}
inline void ChartModel::SetFormatZAxisTextInMultipleLinesIfNecessary( BOOL value )
{
    bFormatZAxisTextInMultipleLinesIfNecessary = value;
}

inline BOOL ChartModel::GetFormatLegendTextInMultipleLinesIfNecessary()
{
    return bFormatLegendTextInMultipleLinesIfNecessary;
}
inline void ChartModel::SetFormatLegendTextInMultipleLinesIfNecessary( BOOL value )
{
    bFormatLegendTextInMultipleLinesIfNecessary = value;
}

inline void ChartModel::SetDiagramRectangle( const Rectangle &rNewRect, bool bStoreLast )
{
    if( bStoreLast )
        aLastDiagramRectangle = aDiagramRectangle;
    aDiagramRectangle = rNewRect;
}

} //namespace binfilter
#endif // _CHTMODEL_HXX

