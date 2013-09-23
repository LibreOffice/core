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

#ifndef SC_LOTIMPOP_HXX
#define SC_LOTIMPOP_HXX

#include <rtl/ustring.hxx>

#include "imp_op.hxx"
#include "flttypes.hxx"
#include "ftools.hxx"
#include "lotform.hxx"
#include "lotattr.hxx"

class ScFormulaCell;
class LotusFontBuffer;


class ImportLotus : public ImportTyp
{
private:
    SvStream*           pIn;            // benoetigt wegen multiplem Read()!
    LotusToSc           aConv;
    sal_uInt16              nTab;           // z.Zt. bearbeitete Tabelle
    sal_Int32               nExtTab;
    // -------------------------------------------------------------------
    // in WK?-Datei
    void                Bof( void );                        // 0x0000   00
    sal_Bool                BofFm3( void );                     // 0x0000   00
    void                Columnwidth( sal_uInt16 nRecLen );      // 0x0007   07
    void                Hiddencolumn( sal_uInt16 nRecLen );     // 0x0008   08
    void                Userrange( void );                  // 0x0009   09
    void                Errcell( void );                    // 0x0014   20
    void                Nacell( void );                     // 0x0015   21
    void                Labelcell( void );                  // 0x0016   22
    void                Numbercell( void );                 // 0x0017   23
    void                Smallnumcell( void );               // 0x0018   24
    ScFormulaCell*      Formulacell( sal_uInt16 nRecLen );      // 0x0019   25
    void                Formulastring( ScFormulaCell& );    // 0x001a   26
                                                            // 0x001b   27 special
    void                NamedSheet( void );                 //          14000
    void                RowPresentation( sal_uInt16 nRecLen );  //           2007

    // -------------------------------------------------------------------
    // in FM?-Datei
    void                Font_Face( void );                  // 174
    void                Font_Type( void );                  // 176
    void                Font_Ysize( void );                 // 177
    void                _Row( const sal_uInt16 nRecLen );       // 197 ?
    // -------------------------------------------------------------------
    inline void         Read( ScAddress& );
    inline void         Read( ScRange& );
        // fuer Addresses/Ranges im Format Row(16)/Tab(8)/Col(8)
    inline void         Read( sal_Char& );
    inline void         Read( sal_uInt8& );
    inline void         Read( sal_uInt16& );
    inline void         Read( sal_Int16& );
    inline void         Read( sal_uInt32& );
    inline void         Read( double& );                    // 10-Byte-IEEE lesen
    inline void         Read( LotAttrWK3& );
    void                Read( String& );                    // 0-terminierten String einlesen
    inline void         Skip( const sal_uInt16 nNumBytes );
    // -------------------------------------------------------------------
public:
                        ImportLotus( SvStream&, ScDocument*, CharSet eSrc );

    virtual             ~ImportLotus();

    FltError            Read();
    FltError            Read( SvStream& );                  // special for *.fm3-Dateien
};


inline void ImportLotus::Read( ScAddress& rAddr )
{
    sal_uInt16 nRow;
    *pIn >> nRow;
    rAddr.SetRow( static_cast<SCROW>(nRow) );
    sal_uInt8 nByte;
    *pIn >> nByte;
    rAddr.SetTab( static_cast<SCTAB>(nByte) );
    *pIn >> nByte;
    rAddr.SetCol( static_cast<SCCOL>(nByte) );
}


inline void ImportLotus::Read( ScRange& rRange )
{
    Read( rRange.aStart );
    Read( rRange.aEnd );
}


inline void ImportLotus::Read( sal_Char& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( sal_uInt8& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( sal_uInt16& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( sal_Int16& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( sal_uInt32& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( double& r )
{
    r = ScfTools::ReadLongDouble( *pIn );
}


inline void ImportLotus::Read( LotAttrWK3& r )
{
    *pIn >> r.nFont >> r.nFontCol >> r.nBack >> r.nLineStyle;
}


inline void ImportLotus::Skip( const sal_uInt16 n )
{
    pIn->SeekRel( n );
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
