/*************************************************************************
 *
 *  $RCSfile: excimp8.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-15 09:03:46 $
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
#ifndef _SC_XCLIMPPIVOTTABLES_HXX
#include "XclImpPivotTables.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif
#include <string.h>

class OfaFilterOptions;

struct ClientAnchorData;
class FilterProgressBar;
class PosBuffer;
class ExcEscherObjList;
class ExcChart;
class ScBaseCell;
class SvStorage;
class DffRecordHeader;
struct DffObjData;
class ScRangeList;

class ScConditionalFormat;
class ExcEscherTxo;

class ScDBData;

class XclImpStream;


class Biff8MSDffManager : public SvxMSDffManager, protected ExcRoot
{
private:
    PosBuffer&              rPosBuff;
    ExcEscherObjList&       rEscherObjList;
protected:
    virtual void            ProcessClientAnchor2( SvStream&, DffRecordHeader&, void*, DffObjData& );
    virtual SdrObject*      ProcessObj( SvStream&, DffObjData&, void* pData, Rectangle& rTextRect,
                                        SdrObject* pObj = NULL );
    virtual ULONG           Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const;
    virtual FASTBOOL        GetColorFromPalette( USHORT nNum, Color& ) const;
    virtual BOOL            ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const;
    static UINT32           GetImportOpts( OfaFilterOptions* );
public:
                            Biff8MSDffManager(
                                            RootData*           pRootData,
                                            PosBuffer&          rPosBuffer,
                                            ExcEscherObjList&   rEscherObjList,
                                            SvStream&           rStCtrl,
                                            long                nOffsDgg,
                                            SvStream*           pStData,
                                            SdrModel*           pSdrModel_          = 0,
                                            long                nApplicationScale   = 0,
                                            ColorData           mnDefaultColor_     = COL_DEFAULT,
                                            ULONG               nDefaultFontHeight_ = 24,
                                            SvStream*           pStData2_           = 0);
    virtual                 ~Biff8MSDffManager();
};




class TxoCont
{
private:
    friend class ImportExcel8;
    friend class ExcEscherTxo;

    String*                         pText;
    EditTextObject*                 pFormText;
    UINT16                          nTextLen;
    UINT16                          nFormCnt;
    UINT16                          nStepCount;

    void                            ReadTxo( XclImpStream& );
    void                            ReadCont( XclImpStream&, RootData&, ScEditEngineDefaulter& );
public:
    inline                          TxoCont( void );
    virtual                         ~TxoCont();

    inline BOOL                     HasText( void ) const;
    inline const String*            GetText( void ) const;

    inline BOOL                     HasFormText( void ) const;
    inline const EditTextObject*    GetTextObject( void ) const;

    inline BOOL                     IsComplete( void ) const;

    void                            Clear( void );
};




class PosBuffer : protected List
{
private:
protected:
public:
    virtual                     ~PosBuffer();
    void                        Append( const UINT32 nStartRange, const UINT32 nEndRange,
                                        const UINT32 nObjNum, const UINT16 nTabNum );
    BOOL                        GetObjNum( const UINT32 nPos, UINT32& rObjNum );

    BOOL                        SetAnchorData( const UINT32 nPos, ClientAnchorData* );
    const ClientAnchorData*     GetAnchorData( const UINT32 nObjNum ) const;

    List::Count;
};




struct ClientAnchorData
{
    UINT32                      nObjNum;

    UINT16                      nTab;
    UINT16                      nCol;
    UINT16                      nX;
    UINT16                      nRow;
    UINT16                      nY;
    UINT16                      nDCol;
    UINT16                      nDX;
    UINT16                      nDRow;
    UINT16                      nDY;

    inline                      ClientAnchorData( void );
    inline                      ClientAnchorData( const ClientAnchorData& );
};




enum OBJECTTYPE
{
    OT_UNKNOWN = 0,
    OT_CHART,
    OT_PICTURE,
    OT_TXO,
    OT_NOTE,
    OT_OLE,
    OT_CTRL
};



class ExcEscherObj : public ExcRoot
{
private:
    friend class ExcEscherObjList;
protected:
    UINT32                      nStrPosStart;
    UINT32                      nStrPosEnd;
    UINT16                      nTab;
    UINT16                      nId;
    Rectangle*                  pAnchor;
    SdrObject*                  pSdrObj;
public:
                                ExcEscherObj( const UINT32 nStrPosStart, const UINT32 nStrPosEnd,
                                                const UINT16 nT, RootData* );
                                ExcEscherObj( ExcEscherObj& rCopy );
                                // !ACHTUNG: pSdrObj von rCopy wandert nach this!
                                //  rCopy.pSdrObj ist danach NULL!!!
    virtual                     ~ExcEscherObj();

    inline void                 SetId( UINT16 );
    inline UINT16               GetId( void ) const;

    virtual OBJECTTYPE          GetObjType( void ) const;

    void                        SetAnchor( const Rectangle& );

    inline const Rectangle*     GetAnchor( void ) const;

    virtual void                SetObj( SdrObject* );
    inline const SdrObject*     GetObj( void ) const;

    virtual void                Apply( void );

    void                        MorpheFrom( ExcEscherObj*& rp );

    virtual ExcChart*           GetChartData( void );       // Special for ExcEscherChart
    virtual void                SetChartData( ExcChart* );
};




class ExcEscherChart : public ExcEscherObj
{
private:
protected:
    ExcChart*                   pChrtData;
public:
                                ExcEscherChart( ExcEscherObj*& rp );
    virtual                     ~ExcEscherChart();

    virtual OBJECTTYPE          GetObjType( void ) const;

    virtual ExcChart*           GetChartData( void );
    virtual void                SetChartData( ExcChart* );

    virtual void                Apply( void );
};




class ExcEscherOle : public ExcEscherObj
{
private:
    String                      aStorageName;
    UINT32                      nBlipId;
    BOOL                        bAsSymbol   : 1;
    BOOL                        bLinked     : 1;

    OBJECTTYPE                  eType;      // OLE or CTRL
protected:
public:
                                ExcEscherOle( ExcEscherObj*& rpCopyAndDel );
    virtual                     ~ExcEscherOle();

    virtual OBJECTTYPE          GetObjType( void ) const;
    virtual void                Apply( void );

    inline  void                SetAsSymbol( BOOL bVal )    { bAsSymbol = (bVal != 0); }
    inline  void                SetLinked( BOOL bVal )      { bLinked = (bVal != 0); }
    inline  void                SetBlipId( UINT32 nVal )    { nBlipId = nVal; }

    void                        ReadPictFmla( XclImpStream&, UINT16 nLen );
    void                        CreateSdrOle( Biff8MSDffManager&, UINT32 nOLEImpFlags = 0 );
};




class ExcEscherDrwObj : public ExcEscherObj
{
private:
protected:
    SdrObject*                  pSdrObj;
public:
                                ExcEscherDrwObj( ExcEscherObj*& rp );
    virtual                     ~ExcEscherDrwObj();

    virtual OBJECTTYPE          GetObjType( void ) const;
};




class ExcEscherTxo : public ExcEscherObj
{
private:
protected:
    String*                     pText;
    EditTextObject*             pFormText;
    BOOL                        bIsApplied;

    void                        ApplyTextOnObject( SdrObject* pSdrObj = NULL );
public:
                                ExcEscherTxo( ExcEscherObj*& rp );
    virtual                     ~ExcEscherTxo();

    virtual OBJECTTYPE          GetObjType( void ) const;

    virtual void                SetObj( SdrObject* );

    void                        SetText( const String& );
    inline const String*        GetText( void ) const;

    void                        TakeTxo( TxoCont& );

    virtual void                Apply( void );
    void                        Apply( SdrObject* );
                                // also set bIsApplied, but doesn't put on drawing layer!
};




class ExcEscherNote : public ExcEscherTxo
{
private:
protected:
public:
                                ExcEscherNote( ExcEscherObj*& rp );
    virtual OBJECTTYPE          GetObjType( void ) const;
    virtual void                Apply( void );
};




class ExcEscherObjList : protected List, protected ExcRoot
{
private:
    UINT32                      nLastReqTabStart;
    UINT16                      nLastReqTab;

    BOOL                        SetTabStart( const UINT16 nNewTab );
protected:
    PosBuffer&                  rPosBuffer;
public:
                                ExcEscherObjList( PosBuffer&, RootData* );
    virtual                     ~ExcEscherObjList();

    void                        Append( ExcEscherObj* );
    void                        MorpheLastObj( ExcEscherObj* );

    inline const ExcEscherObj*  First( void );
    inline const ExcEscherObj*  Next( void );
    const ExcEscherObj*         Get( const UINT32 nObjNum, const UINT16 nTab = 0xFFFF ) const;
    ExcEscherTxo*               GetTxo( const UINT32 nObjNum, const UINT16 nTab = 0xFFFF ) const;

    void                        SetData( const UINT32 nObjNum, SdrObject* pDrawObj );
    void                        SetData( const UINT32 nObjNum, const Rectangle& rAnchor );
    void                        SetData( const UINT32 nObjNum, const Rectangle& rAnchor,
                                        SdrObject* pDrawObj );

    List::Count;

    void                        Apply( void );
};




class ExcCondForm : protected ExcRoot//, List
{
    private:
        UINT16                  nTab;
        UINT16                  nNumOfConds;
        UINT16                  nCondCnt;
        UINT16                  nCol;
        UINT16                  nRow;
//      UINT16                  nRow2;
//      UINT16                  nCol2;
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

struct ExcStreamNode
{
    sal_uInt32      nPos;
    sal_uInt32      nSize;
    ExcStreamNode*  pPrev;
};

class ExcStreamConsumer
{
        sal_uInt32              nBytesLeft;
        SvStream*               pStrm;
        ExcStreamNode*          pNode;
        DffRecordHeader         aHd;

        void                    UpdateNode( const DffRecordHeader& rHd );
        void                    RemoveNode();

    public :

        const DffRecordHeader*  Consume( SvStream* pStrm, sal_uInt32 nLen );    // owns stream if nLen == 0
        sal_Bool                AppendData( sal_Char* pBuf, sal_uInt32 nLen );
        SvStream*               GetStream() const { return pStrm; };

        ExcStreamConsumer();
        ~ExcStreamConsumer();
};


class XclImpTabIdBuffer : protected UINT16List
{
private:
    void                        Append( UINT16 nTabId );
public:
    void                        Fill( XclImpStream& rStrm, UINT16 nCount );
    UINT16                      GetIndex( UINT16 nTabId, UINT16 nMaxTabId = 0xFFFF ) const;
};


class ImportExcel8 : public ImportExcel
{
    protected:
        static String           aSstErrTxt;

        SharedStringTable       aSharedStringTable;

        ExcStreamConsumer       aExcStreamConsumer;
        PosBuffer               aPosBuffer;

        ExcChart*               pActChart;

        ExcEscherObj*           pActEscherObj;
        BOOL                    bLeadingTxo;
        BOOL                    bMaybeTxo;
        BOOL                    bTabStartDummy;
        BOOL                    bCond4EscherCont;
        BOOL                    bLeadingObjRec;
        ExcEscherObjList        aEscherObjList;

        TxoCont*                pActTxo;

        ExcScenarioList         aScenList;

        XclImpPivotTableList    aPivotTabList;
        XclImpPivotTable*       pCurrPivTab;
        XclImpPivotCache*       pCurrPivotCache;

        ExcCondForm*            pActCondForm;
        ExcCondFormList*        pCondFormList;

        BOOL                    bHasBasic;

        void                    Formula( void );                // 0x06
        void                    RecString( void );              // 0x07
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
        void                    SXIdStm( void );                // 0xD5
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

        // Subrecords fuer OBJ (nach Obj( void )
        ExcEscherObj*           ObjFtCmo( void );               // 0x15
        void                    ObjFtPioGrbit( ExcEscherObj* pObj );    // 0x08
        void                    ObjFtPictFmla( ExcEscherObj* pObj, UINT16 nLen );   // 0x09

        void                    Formula( UINT16 nCol, UINT16 nRow, UINT16 nTab,
                                    UINT16 nXF, UINT16 nFormLen, double &rCurVal,
                                    BYTE nFlag, BOOL bShrFmla );
                                                        //      -> excform8.cxx
        virtual void            GetHFString( String& rStr );
        void                    EndSheet( void );
        virtual void            EndAllChartObjects( void );     // -> excobj.cxx
        void                    EndChartObj( void );
        virtual void            PostDocLoad( void );

        virtual FltError        ReadChart8( FilterProgressBar&, const BOOL bOwnTab );
                                // -> read.cxx
        void                    InsertHyperlink( const UINT16 nCol, const UINT16 nRow,
                                                    const String& rURL );
        void                    CreateTmpCtrlStorage( void );
                                    // if possible generate a SvxMSConvertOCXControls compatibel storage
        inline ExcChart*        GetActChartData( void );
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
// internal and external 3D reference handling

struct XclImpSupbookTab
{
    String                      aName;
    UINT16                      nScNum;

    inline                      XclImpSupbookTab() : nScNum( EXC_TAB_INVALID ) {}
};

class XclImpSupbook : protected List
{
private:
    String                      aFileName;
    UINT16                      nCurrScTab;
    BOOL                        bSelf;

    inline XclImpSupbookTab*    Get( ULONG nIndex ) const
                                    { return (XclImpSupbookTab*) List::GetObject( nIndex ); }

public:
                                XclImpSupbook( XclImpStream& rIn, RootData& rExcRoot );
    virtual                     ~XclImpSupbook();

    static void                 ReadDocName( XclImpStream& rStrm, String& rDocName, BOOL& rSelf );
    static void                 ReadTabName( XclImpStream& rStrm, RootData& rExcRoot, String& rTabName );

    inline const String&        GetName() const         { return aFileName; }
    UINT16                      GetScTabNum( UINT16 nExcTabNum ) const;
    UINT16                      GetScTabNum( const String& rTabName ) const;

    inline void                 SetCurrScTab( UINT16 nExcTabNum )
                                    { nCurrScTab = GetScTabNum( nExcTabNum ); }
    inline UINT16               GetCurrScTab() const    { return nCurrScTab; }
    inline BOOL                 HasValidScTab() const   { return nCurrScTab != EXC_TAB_INVALID; }

    void                        CreateTables( RootData& rRootData, UINT16 nFirst, UINT16 nLast ) const;
};

class XclImpSupbookBuffer : protected List
{
private:
    inline XclImpSupbook*       _First()    { return (XclImpSupbook*) List::First(); }
    inline XclImpSupbook*       _Next()     { return (XclImpSupbook*) List::Next(); }
    inline XclImpSupbook*       _Last()     { return (XclImpSupbook*) List::Last(); }

public:
    virtual                     ~XclImpSupbookBuffer();

    inline void                 Append( XclImpSupbook* pNewSupbook )
                                    { List::Insert( pNewSupbook, LIST_APPEND ); }
    void                        CreateTables( RootData& rRootData, UINT16 nFirst, UINT16 nLast );

                                List::Count;
    inline const XclImpSupbook* Get( ULONG nIndex ) const
                                    { return (XclImpSupbook*) List::GetObject( nIndex ); }
    const XclImpSupbook*        Get( const String& rDocName ) const;
    inline XclImpSupbook*       GetCurrSupbook() { return _Last(); }
};

struct XclImpXti
{
    UINT16                      nSupbook;       // index to supbook
    UINT16                      nFirst;         // first sheet tab
    UINT16                      nLast;          // last sheet tab
};

class XclImpExternsheetBuffer : protected List
{
private:
    XclImpSupbookBuffer         aSupbookBuffer;
    BOOL                        bCreated;

    inline XclImpXti*           _First()    { return (XclImpXti*) List::First(); }
    inline XclImpXti*           _Next()     { return (XclImpXti*) List::Next(); }

    BOOL                        FindNextTabRange( UINT16 nSupb, UINT16 nStart, UINT16& rnFirst, UINT16& rnLast );

public:
    inline                      XclImpExternsheetBuffer() : bCreated( FALSE ) {}
    virtual                     ~XclImpExternsheetBuffer();

    void                        Read( XclImpStream& rIn );
    inline void                 AppendSupbook( XclImpSupbook* pSupbook )
                                    { aSupbookBuffer.Append( pSupbook ); }
    void                        CreateTables( RootData& rRootData );

    inline const XclImpXti*     GetXti( ULONG nXtiIndex ) const
                                    { return (const XclImpXti*) List::GetObject( nXtiIndex ); }

    const XclImpSupbook*        GetSupbook( ULONG nXtiIndex ) const;
    inline const XclImpSupbook* GetSupbook( const String& rDocName ) const
                                    { return aSupbookBuffer.Get( rDocName ); }
    inline XclImpSupbook*       GetCurrSupbook()
                                    { return aSupbookBuffer.GetCurrSupbook(); }
};



//___________________________________________________________________
// classes AutoFilterData, AutoFilterBuffer

class AutoFilterData : private ExcRoot
{
private:
    ScDBData*                   pCurrDBData;
    ScQueryParam                aParam;
    UINT16                      nFirstEmpty;
    BOOL                        bHasDropDown;
    BOOL                        bHasConflict;

    void                        CreateFromDouble( String& rStr, double fVal );
    void                        SetCellAttribs();
    void                        InsertQueryParam();

protected:
public:
                                AutoFilterData( RootData* pRoot, const ScRange& rRange,
                                                const String& rName );

    inline UINT16               Tab() const         { return aParam.nTab; }
    inline UINT16               StartCol() const    { return aParam.nCol1; }
    inline UINT16               StartRow() const    { return aParam.nRow1; }
    inline UINT16               EndCol() const      { return aParam.nCol2; }
    inline UINT16               EndRow() const      { return aParam.nRow2; }
    BOOL                        HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab ) const;

    void                        ReadAutoFilter( XclImpStream& rStrm );

    void                        SetAdvancedRange( const ScRange* pRange );
    void                        SetExtractPos( const ScAddress& rAddr );
    void                        Apply();
};


class AutoFilterBuffer : private List
{
private:
    inline AutoFilterData*      _First()    { return (AutoFilterData*) List::First(); }
    inline AutoFilterData*      _Next()     { return (AutoFilterData*) List::Next(); }

    inline void                 Append( AutoFilterData* pData )
                                    { List::Insert( pData, LIST_APPEND ); }
protected:
public:
    virtual                     ~AutoFilterBuffer();

    void                        Insert( RootData* pRoot, const ScRange& rRange,
                                                const String& rName );
    void                        AddAdvancedRange( const ScRange& rRange );
    void                        AddExtractPos( const ScRange& rRange );
    void                        Apply();

    AutoFilterData*             GetByTab( UINT16 nTab );
    BOOL                        HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab );
};




//___________________________________________________________________
// inlines

inline ClientAnchorData::ClientAnchorData( void )
{
    memset( this, 0x00, sizeof( ClientAnchorData ) );
}


inline ClientAnchorData::ClientAnchorData( const ClientAnchorData& r )
{
    memcpy( this, &r, sizeof( ClientAnchorData ) );
}




inline void ExcEscherObj::SetId( UINT16 n )
{
        nId = n;
}

inline UINT16 ExcEscherObj::GetId( void ) const
{
        return nId;
}

inline const Rectangle* ExcEscherObj::GetAnchor( void ) const
{
    return pAnchor;
}


inline const SdrObject* ExcEscherObj::GetObj( void ) const
{
    return pSdrObj;
}




inline const String* ExcEscherTxo::GetText( void ) const
{
    return pText;
}




inline const ExcEscherObj* ExcEscherObjList::First( void )
{
    return ( const ExcEscherObj* ) List::First();
}


inline const ExcEscherObj* ExcEscherObjList::Next( void )
{
    return ( const ExcEscherObj* ) List::Next();
}




inline TxoCont::TxoCont( void )
{
    pText = NULL;
    pFormText = NULL;
    Clear();
}


inline BOOL TxoCont::HasText( void ) const
{
    return pText != NULL;
}


inline const String* TxoCont::GetText( void ) const
{
    return pText;
}


inline BOOL TxoCont::HasFormText( void ) const
{
    return pFormText != NULL;
}


inline const EditTextObject* TxoCont::GetTextObject( void ) const
{
    return pFormText;
}


inline BOOL TxoCont::IsComplete( void ) const
{
    return nStepCount > 2;
}




inline void ExcCondFormList::Append( ExcCondForm* p )
{
    List::Insert( p, LIST_APPEND );
}


inline ExcChart* ImportExcel8::GetActChartData( void )
{
    if( pActEscherObj )
        return pActEscherObj->GetChartData();
    else
        return NULL;
}


#endif

