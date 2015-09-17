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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_IMP_OP_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_IMP_OP_HXX

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
#include <rtl/ref.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <unordered_map>

class SvStream;

class ScFormulaCell;
class ScDocument;

class ExcelToSc;

class ImportTyp
{
protected:
    rtl_TextEncoding    eQuellChar;     // source (Quelle) character set
    ScDocument*         pD;             // document

public:
                        ImportTyp( ScDocument*, rtl_TextEncoding eSrc );
    virtual             ~ImportTyp();

    virtual FltError    Read();
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
    typedef std::unordered_map<SCCOL, LastFormula> LastFormulaMapType;

    static const double     fExcToTwips;        // translate 1/256 chars -> Twips

    RootData*               pExcRoot;

    XclImpStream            maStrm;             // input stream
    XclImpStream&           aIn;                // input stream

    ScfUInt32Vec            maSheetOffsets;
    ScRange                 maScOleSize;        /// Visible range if embedded.

    NameBuffer*             pExtNameBuff;       // ... external names (Ind.-Basis=1)
    ExcelToSc*              pFormConv;          // formula-converter

    XclImpOutlineBuffer*    pColOutlineBuff;
    XclImpOutlineBuffer*    pRowOutlineBuff;
    XclImpColRowSettings*   pColRowBuff;        // Col/Row settings 1 table

    typedef boost::ptr_vector< XclImpOutlineDataBuffer > XclImpOutlineListBuffer;
    XclImpOutlineListBuffer* pOutlineListBuffer;

    LastFormulaMapType maLastFormulaCells; // Keep track of last formula cells in each column.
    LastFormula* mpLastFormula;

    sal_Int16               mnLastRefIdx;
    sal_uInt16              mnIxfeIndex;        /// Current XF identifier from IXFE record.
    sal_uInt16 mnLastRecId;

    SCTAB                   nBdshtTab;          // Counter for Boundsheet

    bool                    bTabTruncated;      // if extended range leads to
                                                // truncation of cells

    bool mbBiff2HasXfs:1;      /// Select XF formatting or direct formatting in BIFF2.
    bool mbBiff2HasXfsValid:1; /// False = mbBiff2HasXfs is undetermined yet.

    void SetLastFormula( SCCOL nCol, SCROW nRow, double fVal, sal_uInt16 nXF, ScFormulaCell* pCell );

    // Record functions
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
    void                    Formula25();              // 0x06     -> excform.cxx
    void                    Row25();                  // 0x08
    void                    Bof2();                   // 0x09
    void                    Eof();                    // 0x0A
    void                    DocProtect();             // 0x12
    void                    SheetProtect();           // 0x12 Sheet Protection
    void                    DocPasssword();           // 0x13 document password
    void                    SheetPassword();               // 0x13 sheet password
    void                    Externsheet();            // 0x17
    void                    WinProtection();          // 0x19
    void                    Columndefault();          // 0x20
    void                    Array25();                // 0x21
    void                    Rec1904();                // 0x22
    void                    Externname25();           // 0x23
    void                    Colwidth();               // 0x24
    void                    Defrowheight2();          // 0x25
//      void                Window1();                // 0x3D
    void                    Codepage();               // 0x42
    void                    Ixfe();                   // 0x44
    void                    DefColWidth();            // 0x55
    void                    Colinfo();                // 0x7D
    void                    Wsbool();                 // 0x81
    void                    Boundsheet();             // 0x85
    void                    Country();                // 0x8C
    void                    Hideobj();                // 0x8D
    void                    Standardwidth();          // 0x99
    void                    Shrfmla();                // 0xBC
    void                    Mulrk();                  // 0xBD
    void                    Mulblank();               // 0xBE
    void                    Rstring();                // 0xD6
    void                    Cellmerging();            // 0xE5
    void                    Olesize();                // 0xDE
    void                    ReadUsesElfs();                 // 0x0160
    void                    Formula3();               // 0x0206       -> excform.cxx
                                                            // 0x0207 -> 0x07
    void                    Row34();                  // 0x0208
    void                    Bof3();                   // 0x0209
    void                    Array34();                // 0x0221
    void                    Defrowheight345();        // 0x0225
    void                    TableOp();                // 0x0236
    //void                  Rk();                     // 0x027E -> 0x7E
    void                    Formula4();               // 0x0406       -> excform.cxx
    void                    Bof4();                   // 0x0409
    void                    Bof5();                   // 0x0809

    void Formula(
        const XclAddress& rXclPos, sal_uInt16 nXF, sal_uInt16 nFormLen, double fCurVal, bool bShrFmla);
                                            //      -> excform.cxx

    virtual void            EndSheet();
    void                    NewTable();
    const ScTokenArray*     ErrorToFormula( bool bErrOrVal, sal_uInt8 nError,
                                double& rVal );

    void            AdjustRowHeight();
    virtual void            PostDocLoad();

public:
                            ImportExcel( XclImpRootData& rImpData, SvStream& rStrm );

    virtual                 ~ImportExcel();

    virtual FltError        Read() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
