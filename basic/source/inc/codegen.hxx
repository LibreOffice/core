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

#ifndef _CODEGEN_HXX
#define _CODEGEN_HXX

class SbiParser;
class SbModule;
#include "opcodes.hxx"
#include "buffer.hxx"
#include <basic/codecompletecache.hxx>

class SbiCodeGen {
    SbiParser* pParser;         // for error messages, line, column etc.
    SbModule& rMod;
    SbiBuffer aCode;
    short  nLine, nCol;         // for stmnt command
    short  nForLevel;           // #29955
    bool bStmnt;            // true: statement-opcode is pending

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
    void GenStmnt();            // create statement-opcode maybe
    sal_uInt32 GetPC();
    sal_uInt32 GetOffset()              { return GetPC() + 1; }
    void Save();

    // #29955 service for-loop-level
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
#define PARAM_INFO_PARAMARRAY       0x0010000
#define PARAM_INFO_WITHBRACKETS     0x0020000

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
