/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imp_op.hxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:59:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _IMP_OP_HXX
#define _IMP_OP_HXX


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif
#ifndef _FLTTYPES_HXX
#include "flttypes.hxx"
#endif
#ifndef _NAMEBUFF_HXX
#include "namebuff.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif
#ifndef _OTLNBUFF_HXX
#include "otlnbuff.hxx"
#endif
#ifndef _COLROWST_HXX
#include "colrowst.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif


class SfxItemSet;
class SvStream;

class ScFormulaCell;
class SdrObject;
class ScDocument;
class ScToken;
class ScToken2;
class _ScRangeListTabs;

class XF_Buffer;
class ValueFormBuffer;
class ExcelToSc;


class ImportTyp
{
protected:
    CharSet             eQuellChar;     // Quell-Zeichensatz
    ScDocument*         pD;             // Dokument

public:
                        ImportTyp( ScDocument*, CharSet eSrc );
    virtual             ~ImportTyp();

    virtual FltError    Read( void );
};




class SvInPlaceObjectRef;




struct ExcelChartData
{
    Rectangle               aRect;              // Ecken
    String                  aTitle, aXTitle, aYTitle, aZTitle;
    String                  aLastLabel;         // letzter SERIESTEXT-Label
    SfxItemSet*             pAttrs;             // Attribute
    ExcelChartData*         pNext;              // wer weiss schon...
    SCROW                   nRow1, nRow2;
    SCCOL                   nCol1, nCol2;
    SCTAB                   nTab1, nTab2;   // Quellbereich
    SCTAB                   nBaseTab;
    UINT32                  nObjNum;

                            ExcelChartData( ScDocument*, const Point&, const Point&,
                                            const SCTAB nBaseTab );
                            ~ExcelChartData();
};

class XclImpOutlineDataBuffer
{
private:
    typedef ::std::auto_ptr< XclImpOutlineBuffer >  XclImpOutlineBufferPtr;
    typedef ::std::auto_ptr< XclImpColRowSettings > XclImpColRowSettingsPtr;

    XclImpOutlineBufferPtr      pColOutlineBuff;
    XclImpOutlineBufferPtr      pRowOutlineBuff;
    XclImpColRowSettingsPtr     pColRowBuff;
    SCTAB nTab;

public:
    XclImpOutlineDataBuffer(RootData& rRootData, SCTAB nScTab);
    ~XclImpOutlineDataBuffer();

    inline XclImpColRowSettings* GetColRowBuff() const { return pColRowBuff.get(); }
    inline XclImpOutlineBuffer* GetColOutline()  const { return pColOutlineBuff.get(); }
    inline XclImpOutlineBuffer* GetRowOutline()  const { return pRowOutlineBuff.get(); }
    void Apply(ScDocument* pD);
};

class ImportExcel : public ImportTyp, protected XclImpRoot
{
private:
    ExcelChartData*         pChart;             // aktuelle Chart-Daten
    ExcelChartData*         pUsedChartFirst;    // benutzte Chart-Daten, erster
    ExcelChartData*         pUsedChartLast;     // benutzte Chart-Daten, letzter

protected:
    static const double     fExcToTwips;        // Umrechnung 1/256 Zeichen -> Twips

    RootData*               pExcRoot;

    XclImpStream            maStrm;             // input stream
    XclImpStream&           aIn;                // input stream

    NameBuffer*             pExtNameBuff;       // ... externe Namen (Ind.-Basis=1)
    ExcelToSc*              pFormConv;          // Formel-Konverter

    XclImpOutlineBuffer*    pColOutlineBuff;
    XclImpOutlineBuffer*    pRowOutlineBuff;
    XclImpColRowSettings*   pColRowBuff;        // Col/Row-Einstellungen 1 Tabelle

    typedef ScfDelList< XclImpOutlineDataBuffer > XclImpOutlineListBuffer;
    XclImpOutlineListBuffer* pOutlineListBuffer;

    UINT16                  nIxfeIndex;         // merkt sich Angabe im IXFE-Record
    UINT16                  nLastXF;            // letzter XF in Formula-Record
    SCTAB                   nBdshtTab;          // Counter fuer Boundsheet
    ScFormulaCell*          pLastFormCell;      // fuer String-Records

    BOOL                    bTabTruncated;      // wenn Bereichsueberschreitung zum
                                                //  Abschneiden von Zellen fuehrt

    // Record-Funktionen
    sal_uInt16              ReadXFIndex( bool bBiff2 );

    void                    ReadDimensions();
    void                    ReadBlank();
    void                    ReadInteger();
    void                    ReadNumber();
    void                    ReadLabel();
    void                    ReadBoolErr();
    void                    ReadRk();

    void                    Window1();
    void                    Formula25( void );              // 0x06     -> excform.cxx
    void                    Row25( void );                  // 0x08
    void                    Bof2( void );                   // 0x09
    void                    Eof( void );                    // 0x0A
    void                    DocProtect( void );             // 0x12
    void                    Protect( void );                // 0x12 Sheet Protection
    BOOL                    Password( void );               // 0x13
    void                    Externsheet( void );            // 0x17
    void                    Note( void );                   // 0x1C
    void                    Columndefault( void );          // 0x20
    void                    Array25( void );                // 0x21
    void                    Rec1904( void );                // 0x22
    void                    Externname25( void );           // 0x23
    void                    Colwidth( void );               // 0x24
    void                    Defrowheight2( void );          // 0x25
//      void                Window1( void );                // 0x3D
    void                    Codepage( void );               // 0x42
    void                    Ixfe( void );                   // 0x44
    void                    DefColWidth( void );            // 0x55
    void                    Builtinfmtcnt( void );          // 0x56
    void                    Obj( void );                    // 0x5D
    void                    Colinfo( void );                // 0x7D
    void                    Wsbool( void );                 // 0x81
    void                    Boundsheet( void );             // 0x85
    void                    Country( void );                // 0x8C
    void                    Hideobj( void );                // 0x8D
    void                    Bundleheader( void );           // 0x8F
    void                    Standardwidth( void );          // 0x99
    void                    Shrfmla( void );                // 0xBC
    void                    Mulrk( void );                  // 0xBD
    void                    Mulblank( void );               // 0xBE
    void                    Rstring( void );                // 0xD6
    void                    Olesize( void );                // 0xDE
    void                    ReadUsesElfs();                 // 0x0160
    void                    Formula3( void );               // 0x0206       -> excform.cxx
                                                            // 0x0207 -> 0x07
    void                    Row34( void );                  // 0x0208
    void                    Bof3( void );                   // 0x0209
    void                    Array34( void );                // 0x0221
    void                    Externname34( void );           // 0x0223
    void                    Defrowheight345( void );        // 0x0225
    void                    TableOp( void );                // 0x0236
    //void                  Rk( void );                     // 0x027E -> 0x7E
    void                    Formula4( void );               // 0x0406       -> excform.cxx
    void                    Bof4( void );                   // 0x0409
    void                    Bof5( void );                   // 0x0809
    // ---------------------------------------------------------------
    void                    SetLineStyle( SfxItemSet&, short, short, short );
    void                    SetFillStyle( SfxItemSet&, short, short, short );
    SdrObject*              LineObj( SfxItemSet&, const Point&, const Point& );
    SdrObject*              RectObj( SfxItemSet&, const Point&, const Point& );
    SdrObject*              BeginChartObj( SfxItemSet&, const Point&, const Point& );
    void                    EndChartObj( void );
    void                    ChartSelection( void );
    void                    ChartSeriesText( void );
    void                    ChartObjectLink( void );
    void                    ChartAi( void );

    // ---------------------------------------------------------------
    void                    Formula( const XclAddress& rXclPos,
                                UINT16 nXF, UINT16 nFormLen, double &rCurVal,
                                BYTE nFlag, BOOL bShrFmla );
                                            //      -> excform.cxx

    virtual void            EndSheet( void );
    void                    NeueTabelle( void );
    const ScTokenArray*     ErrorToFormula( BYTE bErrOrVal, BYTE nError,
                                double& rVal );

    virtual void            EndAllChartObjects( void );     // -> excobj.cxx

    virtual void            AdjustRowHeight();
    virtual void            PostDocLoad( void );

public:
                            ImportExcel( XclImpRootData& rImpData, SvStream& rStrm );

    virtual                 ~ImportExcel( void );

    virtual FltError        Read( void );
};

#endif

