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

#ifndef _CODEGEN_HXX
#define _CODEGEN_HXX

class SbiImage;
class SbiParser;
class SbModule;
#include "opcodes.hxx"
#include "buffer.hxx"

class SbiCodeGen {              // Code-Erzeugung:
    SbiParser* pParser;         // fuer Fehlermeldungen, Line, Column etc.
    SbModule& rMod;             // aktuelles Modul
    SbiBuffer aCode;                // Code-Puffer
    short  nLine, nCol;         // Zeile, Spalte fuer Stmnt-Befehl
    short  nForLevel;           // #29955 for-Schleifen-Ebene
    sal_Bool bStmnt;                // sal_True: Statement-Opcode liegt an
public:
    SbiCodeGen( SbModule&, SbiParser*, short );
    SbiParser* GetParser() { return pParser; }
    SbModule& GetModule() { return rMod; }
    sal_uInt32 Gen( SbiOpcode );
    sal_uInt32 Gen( SbiOpcode, sal_uInt32 );
    sal_uInt32 Gen( SbiOpcode, sal_uInt32, sal_uInt32 );
    void Patch( sal_uInt32 o, sal_uInt32 v ){ aCode.Patch( o, v ); }
    void BackChain( sal_uInt32 off )    { aCode.Chain( off );  }
    void Statement();
    void GenStmnt();            // evtl. Statement-Opcode erzeugen
    sal_uInt32 GetPC();
    sal_uInt32 GetOffset()              { return GetPC() + 1; }
    void Save();

    // #29955 for-Schleifen-Ebene pflegen
    void IncForLevel( void ) { nForLevel++; }
    void DecForLevel( void ) { nForLevel--; }

    static sal_uInt32 calcNewOffSet( sal_uInt8* pCode, sal_uInt16 nOffset );
    static sal_uInt16 calcLegacyOffSet( sal_uInt8* pCode, sal_uInt32 nOffset );

};

template < class T, class S >
class PCodeBuffConvertor
{
    T m_nSize; //
    sal_uInt8* m_pStart;
    sal_uInt8* m_pCnvtdBuf;
    S m_nCnvtdSize; //

    //  Disable usual copying symantics and bodgy default ctor
    PCodeBuffConvertor();
    PCodeBuffConvertor(const PCodeBuffConvertor& );
    PCodeBuffConvertor& operator = ( const PCodeBuffConvertor& );
public:
    PCodeBuffConvertor( sal_uInt8* pCode, T nSize ): m_nSize( nSize ),  m_pStart( pCode ), m_pCnvtdBuf( NULL ), m_nCnvtdSize( 0 ){ convert(); }
    S GetSize(){ return m_nCnvtdSize; }
    void convert();
    // Caller owns the buffer returned
    sal_uInt8* GetBuffer() { return m_pCnvtdBuf; }
};

// #111897 PARAM_INFO flags start at 0x00010000 to not
// conflict with DefaultId in SbxParamInfo::nUserData
#define PARAM_INFO_PARAMARRAY   0x0010000

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
