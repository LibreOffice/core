/*************************************************************************
 *
 *  $RCSfile: excimp8.hxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: dr $ $Date: 2001-07-24 13:51:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/



#ifndef _EXCIMP8_HXX
#define _EXCIMP8_HXX


#ifndef _IMP_OP_HXX
#include "imp_op.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif
#ifndef _EXCSST_HXX
#include "excsst.hxx"
#endif
#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif
#ifndef _EXCSCEN_HXX
#include "excscen.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif
#ifndef _SC_XCLIMPPIVOTTABLES_HXX
#include "XclImpPivotTables.hxx"
#endif
#ifndef _SC_XCLIMPOBJECTS_HXX
#include "XclImpObjects.hxx"
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#include <string.h>

class FilterProgressBar;
class ExcChart;
class ScBaseCell;
class SvStorage;
class ScRangeList;

class ScConditionalFormat;

class ScDBData;

class XclImpStream;
class XclImpAutoFilterBuffer;
class XclImpWebQueryBuffer;




class ExcCondForm : protected ExcRoot//, List
{
    private:
        UINT16                  nTab;
        UINT16                  nNumOfConds;
        UINT16                  nCondCnt;
        UINT16                  nCol;
        UINT16                  nRow;
        ScRangeList*            pRangeList;

        ScConditionalFormat*    pScCondForm;
    protected:
    public:
                                ExcCondForm( RootData* pRootData );
        virtual                 ~ExcCondForm();

        void                    Read( XclImpStream& rIn );
        void                    ReadCf( XclImpStream& rIn, ExcelToSc& rFormConverter );
        void                    Apply( void );
};




class ExcCondFormList : protected List
{
    private:
    protected:
    public:
        virtual                 ~ExcCondFormList();
        inline void             Append( ExcCondForm* pNew );

        void                    Apply( void );
};






struct DVData;


class DVList : protected List
{
private:
public:
    virtual                     ~DVList();

    void                        Add( DVData* );
    void                        Reset( void );
    void                        Apply( ScDocument&, UINT16 nTabNum );
};




class ImportExcel8 : public ImportExcel
{
    protected:
        static String           aSstErrTxt;

        SharedStringTable       aSharedStringTable;

        XclImpObjectManager     aObjManager;
        ExcChart*               pActChart;
        BOOL                    bObjSection;

        ExcScenarioList         aScenList;

        XclImpPivotTableList    aPivotTabList;
        XclImpPivotTable*       pCurrPivTab;
        XclImpPivotCache*       pCurrPivotCache;

        ExcCondForm*            pActCondForm;
        ExcCondFormList*        pCondFormList;

        DVList*                 pDVList;

        XclImpAutoFilterBuffer* pAutoFilterBuffer;  // ranges for autofilter and advanced filter
        XclImpWebQueryBuffer*   pWebQBuffer;        // data for web queries

        BOOL                    bHasBasic;

        BOOL                    bFirstScl;          // only one Scl-Record has to be read per chart!

        void                    RecString( void );              // 0x07
        void                    Calccount( void );              // 0x0C
        void                    Delta( void );                  // 0x10
        void                    Iteration( void );              // 0x11
        void                    Protect( void );                // 0x12
        void                    Verticalpagebreaks( void );     // 0x1A
        void                    Horizontalpagebreaks( void );   // 0x1B
        void                    Note( void );                   // 0x1C
        void                    Format( void );                 // 0x1E
        void                    Externsheet( void );            // 0x17
        void                    Externname( void );             // 0x23
        void                    Font( void );                   // 0x31
        void                    Cont( void );                   // 0x3C
        void                    Dconref( void );                // 0x51
        void                    Xct( void );                    // 0x59
        void                    Crn( void );                    // 0x5A
        void                    Obj( void );                    // 0x5D
        void                    Boundsheet( void );             // 0x85
        void                    FilterMode( void );             // 0x9B
        void                    AutoFilterInfo( void );         // 0x9D
        void                    AutoFilter( void );             // 0x9E
        void                    Scenman( void );                // 0xAE
        void                    Scenario( void );               // 0xAF
        void                    SXView( void );                 // 0xB0
        void                    SXVd( void );                   // 0xB1
        void                    SXVi( void );                   // 0xB2
        void                    SXIvd( void );                  // 0xB4
        void                    SXLi( void );                   // 0xB5
        void                    SXPi( void );                   // 0xB6
        void                    SXDi( void );                   // 0xC5
        void                    SXString( void );               // 0xCD
        void                    SXIdStm( void );                // 0xD5
        void                    SXExt_ParamQry( void );         // 0xDC
        void                    Xf( void );                     // 0xE0
        void                    SXVs( void );                   // 0xE3
        void                    Cellmerging( void );            // 0xE5     geraten...
        void                    BGPic( void );                  // 0xE9     background picture (guess so, no documentation)
        void                    Msodrawinggroup( void );        // 0xEB
        void                    Msodrawing( void );             // 0xEC
        void                    Msodrawingselection( void );    // 0xED
        void                    SXRule( void );                 // 0xF0
        void                    SXEx( void );                   // 0xF1
        void                    SXFilt( void );                 // 0xF2
        void                    SXSelect( void );               // 0xF7
        void                    Sst( void );                    // 0xFC
                                                                // liefert Pos vom Folgerecord
        ScBaseCell*             CreateCellFromShStrTabEntry( const ShStrTabEntry*,
                                                                const UINT16 nXF );
        void                    Labelsst( void );               // 0xFD
        void                    SXVdex( void );                 // 0x0100
        void                    Label( void );                  // 0x0204

        void                    Tabid( void );                  // 0x013D
        void                    Qsi( void );                    // 0x01AD
        void                    Supbook( void );                // 0x01AE
        void                    Condfmt( void );                // 0x01B0
        void                    Cf( void );                     // 0x01B1
        void                    Dval( void );                   // 0x01B2
        void                    Txo( void );                    // 0x01B6
        void                    Hlink( void );                  // 0x01B8
        void                    Codename( BOOL bWBGlobals );    // 0x01BA
        void                    Dv( void );                     // 0x01BE
        void                    Dimensions( void );             // 0x0200
        void                    Name( void );                   // 0x0218
        void                    Style( void );                  // 0x0293

        void                    WebQrySettings( void );         // 0x0803
        void                    WebQryTables( void );           // 0x0804

        void                    ChartEof( void );               // 0x000A
        void                    ChartScl( void );               // 0x00A0
        void                    ChartChart( void );             // 0x1002
        void                    ChartSeries( void );            // 0x1003       changed from Biff5
        void                    ChartDataformat( void );        // 0x1006
        void                    ChartLineformat( void );        // 0x1007
        void                    ChartMarkerformat( void );      // 0x1009
        void                    ChartAreaformat( void );        // 0x100A
        void                    ChartPieformat( void );         // 0x100B
        void                    ChartAttachedlabel( void );     // 0x100C
        void                    ChartSeriestext( void );        // 0x100D
        void                    ChartChartformat( void );       // 0x1014
        void                    ChartLegend( void );            // 0x1015
        void                    ChartBar( void );               // 0x1017
        void                    ChartLine( void );              // 0x1018
        void                    ChartPie( void );               // 0x1019
        void                    ChartArea( void );              // 0x101A
        void                    ChartScatter( void );           // 0x101B
        void                    ChartChartline( void );         // 0x101C
        void                    ChartAxis( void );              // 0x101D
        void                    ChartTick( void );              // 0x101E
        void                    ChartValuerange( void );        // 0x101F
        void                    ChartCatserrange( void );       // 0x1020
        void                    ChartAxislineformat( void );    // 0x1021
        void                    ChartDefaulttext( void );       // 0x1024
        void                    ChartText( void );              // 0x1025
        void                    ChartFontx( void );             // 0x1026
        void                    ChartObjectlink( void );        // 0x1027
        void                    ChartFrame( void );             // 0x1032
        void                    ChartBegin( void );             // 0x1033
        void                    ChartEnd( void );               // 0x1034
        void                    ChartPlotarea( void );          // 0x1035
        void                    Chart3D( void );                // 0x103A
        void                    ChartPicf( void );              // 0x103C
        void                    ChartDropbar( void );           // 0x103D
        void                    ChartRadar( void );             // 0x103E
        void                    ChartSurface( void );           // 0x103F
        void                    ChartAxisparent( void );        // 0x1041
        void                    ChartShtprops( void );          // 0x1044
        void                    ChartSertocrt( void );          // 0x1045
        void                    ChartAxesused( void );          // 0x1046
        void                    ChartIfmt( void );              // 0x104E
        void                    ChartAi( void );                // 0x1051 (ChartSelection)
        void                    ChartAi_2_Series( void );       // 0x1051 (ChartSelection)
        void                    ChartSerfmt( void );            // 0x105D
        void                    Chart3DDataformat( void );      // 0x105F
        void                    ChartFbi( void );               // 0x1060       new to Biff8
        void                    ChartBoppop( void );            // 0x1061
        void                    ChartAxcext( void );            // 0x1062
        void                    ChartDat( void );               // 0x1063
        void                    ChartPlotgrowth( void );        // 0x1064
        void                    ChartSiindex( void );           // 0x1065
        void                    ChartGelframe( void );          // 0x1066
        void                    ChartBoppcustom( void );        // 0x1067

        virtual void            GetHFString( String& rStr );
        void                    EndSheet( void );
        virtual void            EndAllChartObjects( void );     // -> excobj.cxx
        virtual void            PostDocLoad( void );

        virtual FltError        ReadChart8( FilterProgressBar&, const BOOL bOwnTab );
                                // -> read.cxx
        void                    InsertHyperlink( const UINT16 nCol, const UINT16 nRow,
                                                    const String& rURL );
        void                    CreateTmpCtrlStorage( void );
                                    // if possible generate a SvxMSConvertOCXControls compatibel storage
        inline void             SetActChartData( ExcChart* pNewData )
                                                    { aObjManager.SetCurrChartData( pNewData ); }
        inline ExcChart*        GetActChartData()   { return aObjManager.GetCurrChartData(); }

    public:
                                ImportExcel8(
                                    SvStorage*  pStorage,
                                    SvStream&   aStream,
                                    ScDocument* pDoc,
                                    SvStorage*  pPivotCache );

        virtual                 ~ImportExcel8( void );

        virtual FltError        Read( void );
};



//___________________________________________________________________
// web queries

enum XclImpWebQueryMode         { xiwqUnknown, xiwqDoc, xiwqAllTables, xiwqSpecTables };

class XclImpWebQuery
{
public:
    String                      aFilename;
    String                      aTables;
    ScRange                     aDestRange;
    XclImpWebQueryMode          eMode;
    UINT16                      nRefresh;

    inline                      XclImpWebQuery() : eMode( xiwqUnknown ), nRefresh( 0 ) {}
    BOOL                        IsValid();

    void                        ConvertTableNames();
};

class XclImpWebQueryBuffer : private List
{
public:
    virtual                     ~XclImpWebQueryBuffer();

    inline XclImpWebQuery*      First() { return (XclImpWebQuery*) List::First(); }
    inline XclImpWebQuery*      Next()  { return (XclImpWebQuery*) List::Next(); }
    inline XclImpWebQuery*      Last()  { return (XclImpWebQuery*) List::Last(); }

    inline void                 Append( XclImpWebQuery* pQuery )
                                    { List::Insert( pQuery, LIST_APPEND ); }

    void                        Apply( ScDocument* pDoc );
};



//___________________________________________________________________
// classes AutoFilterData, AutoFilterBuffer

class XclImpAutoFilterData : private ExcRoot
{
private:
    ScDBData*                   pCurrDBData;
    ScQueryParam                aParam;
    UINT16                      nFirstEmpty;
    BOOL                        bActive;
    BOOL                        bHasDropDown;
    BOOL                        bHasConflict;

    void                        CreateFromDouble( String& rStr, double fVal );
    void                        SetCellAttribs();
    void                        InsertQueryParam();

protected:
public:
                                XclImpAutoFilterData(
                                    RootData* pRoot,
                                    const ScRange& rRange,
                                    const String& rName );

    inline UINT16               Tab() const         { return aParam.nTab; }
    inline UINT16               StartCol() const    { return aParam.nCol1; }
    inline UINT16               StartRow() const    { return aParam.nRow1; }
    inline UINT16               EndCol() const      { return aParam.nCol2; }
    inline UINT16               EndRow() const      { return aParam.nRow2; }
    BOOL                        HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab ) const;

    void                        ReadAutoFilter( XclImpStream& rStrm );

    inline void                 Activate()          { bActive = TRUE; }
    void                        SetAdvancedRange( const ScRange* pRange );
    void                        SetExtractPos( const ScAddress& rAddr );
    void                        Apply();
};


class XclImpAutoFilterBuffer : private List
{
private:
    inline XclImpAutoFilterData* _First()   { return (XclImpAutoFilterData*) List::First(); }
    inline XclImpAutoFilterData* _Next()    { return (XclImpAutoFilterData*) List::Next(); }

    inline void                 Append( XclImpAutoFilterData* pData )
                                    { List::Insert( pData, LIST_APPEND ); }
protected:
public:
    virtual                     ~XclImpAutoFilterBuffer();

    void                        Insert( RootData* pRoot, const ScRange& rRange,
                                                const String& rName );
    void                        AddAdvancedRange( const ScRange& rRange );
    void                        AddExtractPos( const ScRange& rRange );
    void                        Apply();

    XclImpAutoFilterData*       GetByTab( UINT16 nTab );
    BOOL                        HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab );
};




//___________________________________________________________________
// inlines

inline void ExcCondFormList::Append( ExcCondForm* p )
{
    List::Insert( p, LIST_APPEND );
}

#endif

