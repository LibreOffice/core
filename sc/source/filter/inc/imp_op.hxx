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

#ifndef SC_IMP_OP_HXX
#define SC_IMP_OP_HXX

#include <sal/types.h>
#include "xiroot.hxx"
#include "xistream.hxx"
#include "xistyle.hxx"
#include "flttypes.hxx"
#include "namebuff.hxx"
#include "root.hxx"
#include "otlnbuff.hxx"
#include "colrowst.hxx"
#include "excdefs.hxx"
#include "rtl/ref.hxx"
#include "clkernelthread.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

class SvStream;

class ScFormulaCell;
class ScDocument;

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

class XclImpOutlineDataBuffer : protected XclImpRoot
{
public:
    explicit            XclImpOutlineDataBuffer( const XclImpRoot& rRoot, SCTAB nScTab );
    virtual             ~XclImpOutlineDataBuffer();

    inline XclImpColRowSettings* GetColRowBuff() const { return mxColRowBuff.get(); }
    inline XclImpOutlineBuffer* GetColOutline()  const { return mxColOutlineBuff.get(); }
    inline XclImpOutlineBuffer* GetRowOutline()  const { return mxRowOutlineBuff.get(); }
    void                Convert();

private:
    typedef boost::shared_ptr< XclImpOutlineBuffer >  XclImpOutlineBfrRef;
    typedef boost::shared_ptr< XclImpColRowSettings > XclImpColRowSettRef;

    XclImpOutlineBfrRef mxColOutlineBuff;
    XclImpOutlineBfrRef mxRowOutlineBuff;
    XclImpColRowSettRef mxColRowBuff;
    SCTAB               mnScTab;
};

class ImportExcel : public ImportTyp, protected XclImpRoot
{
protected:
    struct LastFormula
    {
        SCCOL mnCol;
        SCROW mnRow;
        double mfValue;
        sal_uInt16 mnXF;
        ScFormulaCell* mpCell;
    };
    typedef boost::unordered_map<SCCOL, LastFormula> LastFormulaMapType;

    rtl::Reference<sc::CLBuildKernelThread> mxCLKernelThread;

    static const double     fExcToTwips;        // Umrechnung 1/256 Zeichen -> Twips

    RootData*               pExcRoot;

    XclImpStream            maStrm;             // input stream
    XclImpStream&           aIn;                // input stream

    ScfUInt32Vec            maSheetOffsets;
    ScRange                 maScOleSize;        /// Visible range if embedded.

    NameBuffer*             pExtNameBuff;       // ... externe Namen (Ind.-Basis=1)
    ExcelToSc*              pFormConv;          // Formel-Konverter

    XclImpOutlineBuffer*    pColOutlineBuff;
    XclImpOutlineBuffer*    pRowOutlineBuff;
    XclImpColRowSettings*   pColRowBuff;        // Col/Row-Einstellungen 1 Tabelle

    typedef boost::ptr_vector< XclImpOutlineDataBuffer > XclImpOutlineListBuffer;
    XclImpOutlineListBuffer* pOutlineListBuffer;

    LastFormulaMapType maLastFormulaCells; // Keep track of last formula cells in each column.
    LastFormula* mpLastFormula;

    sal_Int16               mnLastRefIdx;
    sal_uInt16              mnIxfeIndex;        /// Current XF identifier from IXFE record.
    sal_uInt16 mnLastRecId;

    SCTAB                   nBdshtTab;          // Counter fuer Boundsheet

    sal_Bool                    bTabTruncated;      // wenn Bereichsueberschreitung zum
                                                //  Abschneiden von Zellen fuehrt

    bool mbBiff2HasXfs:1;      /// Select XF formatting or direct formatting in BIFF2.
    bool mbBiff2HasXfsValid:1; /// False = mbBiff2HasXfs is undetermined yet.
    bool mbRunCLKernelThread:1;

    void SetLastFormula( SCCOL nCol, SCROW nRow, double fVal, sal_uInt16 nXF, ScFormulaCell* pCell );
    LastFormula* GetLastFormula( SCCOL nCol );

    // Record-Funktionen
    void                    ReadFileSharing();

    sal_uInt16              ReadXFIndex( const ScAddress& rScPos, bool bBiff2 );

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
    void                    SheetProtect( void );           // 0x12 Sheet Protection
    void                    DocPasssword( void );           // 0x13 document password
    void                    SheetPassword( void );               // 0x13 sheet password
    void                    Externsheet( void );            // 0x17
    void                    WinProtection( void );          // 0x19
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
    void                    Cellmerging( void );            // 0xE5
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
    void Formula(
        const XclAddress& rXclPos, sal_uInt16 nXF, sal_uInt16 nFormLen, double fCurVal, bool bShrFmla);
                                            //      -> excform.cxx

    virtual void            EndSheet( void );
    void                    NeueTabelle( void );
    const ScTokenArray*     ErrorToFormula( sal_uInt8 bErrOrVal, sal_uInt8 nError,
                                double& rVal );

    virtual void            AdjustRowHeight();
    virtual void            PostDocLoad( void );

public:
                            ImportExcel( XclImpRootData& rImpData, SvStream& rStrm );

    virtual                 ~ImportExcel( void );

    virtual FltError        Read( void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
