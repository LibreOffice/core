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

#ifndef SC_LOTIMPOP_HXX
#define SC_LOTIMPOP_HXX

#include <tools/string.hxx>

#include "imp_op.hxx"
#include "flttypes.hxx"
#include "ftools.hxx"
#include "lotform.hxx"
#include "lotattr.hxx"

class ScFormulaCell;
class LotusFontBuffer;
class SvxBorderLine;


class ImportLotus : public ImportTyp
{
private:
    SvStream*           pIn;            // benoetigt wegen multiplem Read()!
    LotusFontBuffer*    pFontBuff;
    LotusToSc           aConv;
    UINT16              nTab;           // z.Zt. bearbeitete Tabelle
    INT32               nExtTab;
    // -------------------------------------------------------------------
    // in WK?-Datei
    void                Bof( void );                        // 0x0000   00
    BOOL                BofFm3( void );                     // 0x0000   00
    void                Columnwidth( UINT16 nRecLen );      // 0x0007   07
    void                Hiddencolumn( UINT16 nRecLen );     // 0x0008   08
    void                Userrange( void );                  // 0x0009   09
    void                Errcell( void );                    // 0x0014   20
    void                Nacell( void );                     // 0x0015   21
    void                Labelcell( void );                  // 0x0016   22
    void                Numbercell( void );                 // 0x0017   23
    void                Smallnumcell( void );               // 0x0018   24
    ScFormulaCell*      Formulacell( UINT16 nRecLen );      // 0x0019   25
    void                Formulastring( ScFormulaCell& );    // 0x001a   26
                                                            // 0x001b   27 special
    void                NamedSheet( void );                 //          14000
    void                RowPresentation( UINT16 nRecLen );  //           2007

    // -------------------------------------------------------------------
    // in FM?-Datei
    void                Font_Face( void );                  // 174
    void                Font_Type( void );                  // 176
    void                Font_Ysize( void );                 // 177
    void                _Row( const UINT16 nRecLen );       // 197 ?
    // -------------------------------------------------------------------
    inline void         Read( ScAddress& );
    inline void         Read( ScRange& );
        // fuer Addresses/Ranges im Format Row(16)/Tab(8)/Col(8)
    inline void         Read( sal_Char& );
    inline void         Read( BYTE& );
    inline void         Read( UINT16& );
    inline void         Read( INT16& );
    inline void         Read( UINT32& );
    inline void         Read( double& );                    // 10-Byte-IEEE lesen
    inline void         Read( LotAttrWK3& );
    void                Read( String& );                    // 0-terminierten String einlesen
    inline void         Skip( const UINT16 nNumBytes );
    // -------------------------------------------------------------------
public:
                        ImportLotus( SvStream&, ScDocument*, CharSet eSrc );

    virtual             ~ImportLotus();

    FltError            Read();
    FltError            Read( SvStream& );                  // special for *.fm3-Dateien
};


inline void ImportLotus::Read( ScAddress& rAddr )
{
    UINT16 nRow;
    *pIn >> nRow;
    rAddr.SetRow( static_cast<SCROW>(nRow) );
    BYTE nByte;
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


inline void ImportLotus::Read( BYTE& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( UINT16& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( INT16& r )
{
    *pIn >> r;
}


inline void ImportLotus::Read( UINT32& r )
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


inline void ImportLotus::Skip( const UINT16 n )
{
    pIn->SeekRel( n );
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
