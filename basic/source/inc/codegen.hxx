/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#define PARAM_INFO_PARAMARRAY       0x0010000
#define PARAM_INFO_WITHBRACKETS     0x0020000

#endif
