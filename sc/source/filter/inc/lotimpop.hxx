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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_LOTIMPOP_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_LOTIMPOP_HXX

#include <rtl/ustring.hxx>

#include "imp_op.hxx"
#include "flttypes.hxx"
#include "ftools.hxx"
#include "lotform.hxx"
#include "lotattr.hxx"

class ScFormulaCell;
class LotusFontBuffer;
struct LotusContext;

class ImportLotus : public ImportTyp
{
private:
    SvStream*           pIn;            // benoetigt wegen multiplem Read()!
    LotusToSc           aConv;
    sal_uInt16              nTab;           // z.Zt. bearbeitete Tabelle
    sal_Int32               nExtTab;

    // in WK?-Datei
    void                Bof();                        // 0x0000   00
    bool                BofFm3();                     // 0x0000   00
    void                Columnwidth( sal_uInt16 nRecLen );      // 0x0007   07
    void                Hiddencolumn( sal_uInt16 nRecLen );     // 0x0008   08
    void                Userrange();                  // 0x0009   09
    void                Errcell();                    // 0x0014   20
    void                Nacell();                     // 0x0015   21
    void                Labelcell();                  // 0x0016   22
    void                Numbercell();                 // 0x0017   23
    void                Smallnumcell();               // 0x0018   24
    ScFormulaCell*      Formulacell( sal_uInt16 nRecLen );      // 0x0019   25
                                                            // 0x001b   27 special
    void                NamedSheet();                 //          14000
    void                RowPresentation( sal_uInt16 nRecLen );  //           2007

    // in FM?-Datei
    void                Font_Face();                  // 174
    void                Font_Type();                  // 176
    void                Font_Ysize();                 // 177
    void                _Row( const sal_uInt16 nRecLen );       // 197 ?

    inline void         Read( ScAddress& );
    inline void         Read( ScRange& );
        // fuer Addresses/Ranges im Format Row(16)/Tab(8)/Col(8)
    inline void         Read( sal_Char& );
    inline void         Read( sal_uInt8& );
    inline void         Read( sal_uInt16& );
    inline void         Read( sal_Int16& );
    inline void         Read( double& );                    // 10-Byte-IEEE lesen
    inline void         Read( LotAttrWK3& );
    void                Read( OUString& );                  // 0-terminierten String einlesen
    inline void         Skip( const sal_uInt16 nNumBytes );

public:
    ImportLotus(LotusContext& rContext, SvStream&, ScDocument*, rtl_TextEncoding eSrc);

    virtual             ~ImportLotus();

    FltError            Read() override;
    FltError            Read( SvStream& );                  // special for *.fm3-Dateien
};

inline void ImportLotus::Read( ScAddress& rAddr )
{
    sal_uInt16 nRow;
    pIn->ReadUInt16( nRow );
    rAddr.SetRow( static_cast<SCROW>(nRow) );
    sal_uInt8 nByte;
    pIn->ReadUChar( nByte );
    rAddr.SetTab( static_cast<SCTAB>(nByte) );
    pIn->ReadUChar( nByte );
    rAddr.SetCol( static_cast<SCCOL>(nByte) );
}

inline void ImportLotus::Read( ScRange& rRange )
{
    Read( rRange.aStart );
    Read( rRange.aEnd );
}

inline void ImportLotus::Read( sal_Char& r )
{
    pIn->ReadChar( r );
}

inline void ImportLotus::Read( sal_uInt8& r )
{
    pIn->ReadUChar( r );
}

inline void ImportLotus::Read( sal_uInt16& r )
{
    pIn->ReadUInt16( r );
}

inline void ImportLotus::Read( sal_Int16& r )
{
    pIn->ReadInt16( r );
}

inline void ImportLotus::Read( double& r )
{
    r = ScfTools::ReadLongDouble( *pIn );
}

inline void ImportLotus::Read( LotAttrWK3& r )
{
    pIn->ReadUChar( r.nFont ).ReadUChar( r.nFontCol ).ReadUChar( r.nBack ).ReadUChar( r.nLineStyle );
}

inline void ImportLotus::Skip( const sal_uInt16 n )
{
    pIn->SeekRel( n );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
