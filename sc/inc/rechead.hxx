/*************************************************************************
 *
 *  $RCSfile: rechead.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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

#ifndef SC_RECHEAD_HXX
#define SC_RECHEAD_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

//      ID's fuer Dateien:

#define SCID_SIZES          0x4200

#define SCID_POOLS          0x4210
#define SCID_DOCPOOL        0x4211
#define SCID_STYLEPOOL      0x4212
#define SCID_NEWPOOLS       0x4213      // ID fuer SC 3.0a
#define SCID_EDITPOOL       0x4214

#define SCID_DOCUMENT       0x4220
#define SCID_DOCFLAGS       0x4221
#define SCID_TABLE          0x4222
#define SCID_DRAWING        0x4223
#define SCID_RANGENAME      0x4224
#define SCID_DBAREAS        0x4225
#define SCID_PIVOT          0x4226
#define SCID_CHARTS         0x4227
#define SCID_NUMFORMAT      0x4228
#define SCID_DOCOPTIONS     0x4229
#define SCID_VIEWOPTIONS    0x422a
#define SCID_PRINTSETUP     0x422b
#define SCID_CHARSET        0x422c
#define SCID_NEWDOCUMENT    0x422d      // ID fuer SC 3.0a
#define SCID_DDELINKS       0x422e
#define SCID_AREALINKS      0x422f
#define SCID_CONDFORMATS    0x4230
#define SCID_VALIDATION     0x4231
#define SCID_COLNAMERANGES  0x4232
#define SCID_ROWNAMERANGES  0x4233
#define SCID_DETOPLIST      0x4234
#define SCID_CONSOLIDATA    0x4235
#define SCID_CHANGETRACK    0x4236
#define SCID_CHGVIEWSET     0x4237
#define SCID_LINKUPMODE     0x4238
#define SCID_DATAPILOT      0x4239

#define SCID_COLUMNS        0x4240
#define SCID_COLROWFLAGS    0x4241
#define SCID_TABOPTIONS     0x4242
#define SCID_TABLINK        0x4243

#define SCID_COLDATA        0x4250
#define SCID_COLNOTES       0x4251
#define SCID_COLATTRIB      0x4252

#define SCID_DRAWPOOL       0x4260
#define SCID_DRAWMODEL      0x4261



//  Dateiversion
//  Falls das obere Byte inkrementiert wird, wird das Doc von
//  aelteren SCs nicht mehr geladen!

#define SC_INITIAL_VERSION  0x0001
#define SC_FORMULA_LCLVER   0x0002          // Formalen mit lokaler VerNr
//--------------------------------
#define SC_NEW_TOKEN_ARRAYS 0x0003          // neues TokenArray-Format
#define SC_FORMULA_VALUES   0x0004          // Werte in Formelzellen
#define SC_FORMULA_VALUES2  0x0005          // Werte in Formelzellen
#define SC_DATABYTES        0x0006          // Datenbytes, kleine Tables
#define SC_DATABYTES2       0x0007          // Datenbytes, kleine Tables
#define SC_NUMFMT           0x0008          // Zahlenformat an Formelzelle
#define SC_NEWIF            0x0009          // neue Codierung von ocIf (komp.)
//--------------------------------
#define SC_RELATIVE_REFS    0x0010          // relative Referenzen
#define SC_SUBTOTAL_FLAG    0x0011          // bSubTotal der Formelzelle
#define SC_COLROWNAME_RANGEPAIR 0x0012      // ColRowNameRanges als ScRangePair
//--------------------------------
#define SC_31_EXPORT_VER    0x0012          // Version bei 3.1-Export
//-------------------------------- ab 4.0
#define SC_32K_ROWS         0x0100          // 32000 Zeilen - inkompatibel
#define SC_FONTCHARSET      0x0101          // Font-CharSets muessen stimmen
//--------------------------------
#define SC_40_EXPORT_VER    0x0101          // Version bei 4.0-Export
//-------------------------------- ab 5.0
#define SC_RECALC_MODE_BITS 0x0201          // TokenArray RecalcMode
#define SC_MATRIX_DOUBLEREF 0x0202          // DoubleRef implizite Schnittmenge
#define SC_VERSION_EDITPOOL 0x0203          // EditCells mit EditPool
#define SC_SUBTOTAL_BUGFIX  0x0204          // bSubTotal der Formelzelle wirklich
//-------------------------------- ab 5.2
#define SC_CONVERT_RECALC_ON_LOAD 0x0205    // #73616# CONVERT function recalculated on each load
//--------------------------------
#define SC_CURRENT_VERSION  0x0205


//  alles ueber SC_31_EXPORT_VER muss auch beim Speichern abgefragt werden,
//  weil 3.1-Export diese Versionsnummer schreibt.

// btw: 10 nach 09 ist kein Zaehlfehler sondern eine absichtliche Luecke,
// weil nicht klar war, wie lange die RelRefs Entwicklung dauern wuerde.. :)

class SvStream;

// -----------------------------------------------------------------------

//  "Automatischer" Record-Header mit Groessenangabe

class ScReadHeader
{
private:
    SvStream&   rStream;
    ULONG       nDataEnd;

public:
    ScReadHeader(SvStream& rNewStream);
    ~ScReadHeader();

    ULONG   BytesLeft() const;
};

class ScWriteHeader
{
private:
    SvStream&   rStream;
    ULONG       nDataPos;
    ULONG       nDataSize;

public:
    ScWriteHeader(SvStream& rNewStream, ULONG nDefault = 0);
    ~ScWriteHeader();
};

        //  Header mit Groessenangaben fuer mehrere Objekte

class ScMultipleReadHeader
{
private:
    SvStream&       rStream;
    BYTE*           pBuf;
    SvMemoryStream* pMemStream;
    ULONG           nEndPos;
    ULONG           nEntryEnd;
    ULONG           nTotalEnd;

public:
    ScMultipleReadHeader(SvStream& rNewStream);
    ~ScMultipleReadHeader();

    void    StartEntry();
    void    EndEntry();
    ULONG   BytesLeft() const;
};

class ScMultipleWriteHeader
{
private:
    SvStream&       rStream;
    SvMemoryStream  aMemStream;
    ULONG           nDataPos;
    ULONG           nDataSize;
    ULONG           nEntryStart;

public:
    ScMultipleWriteHeader(SvStream& rNewStream, ULONG nDefault = 0);
    ~ScMultipleWriteHeader();

    void    StartEntry();
    void    EndEntry();
};

#endif


