/*************************************************************************
 *
 *  $RCSfile: lotimpop.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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



#ifndef _LOTIMPOP_HXX
#define _LOTIMPOP_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include "imp_op.hxx"
#include "flttypes.hxx"
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
    rAddr.SetRow( nRow );
    BYTE nByte;
    *pIn >> nByte;
    rAddr.SetTab( nByte );
    *pIn >> nByte;
    rAddr.SetCol( nByte );
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


extern double ReadLongDouble( SvStream& rStr ); // -> exctools.cxx


inline void ImportLotus::Read( double& r )
{
    r = ReadLongDouble( *pIn );
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
