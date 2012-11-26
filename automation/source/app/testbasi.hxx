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



#ifndef _TTBASIC_HXX
#define _TTBASIC_HXX

#include <basic/sbstar.hxx>
#include <basic/mybasic.hxx>

class ErrorEntry;

#define SBXID_TTBASIC   0x5454      // TTBasic: TT

#define SBXCR_TEST2   0x54534554L   // TEST

class TTBasic : public MyBasic
{
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_TEST2,SBXID_TTBASIC,1);
    TTBasic();
   ~TTBasic();
    sal_Bool Compile( SbModule* );
    static MyBasic* CreateMyBasic();

    // nicht mit #ifdefs klammern, da diese Headerdatei für testtool und basic
    // gleichermaßen verwendet wird.
    DECL_LINK( CErrorImpl, ErrorEntry* );
//  SbxObject *pTestObject;         // für das Testtool; ansonsten NULL

    void LoadIniFile();
    SbTextType GetSymbolType( const String &Symbol, sal_Bool bWasTTControl );   // Besimmt den erweiterten Symboltyp für das Syntaxhighlighting
    virtual const String GetSpechialErrorText();
    virtual void ReportRuntimeError( AppBasEd *pEditWin );
    virtual void DebugFindNoErrors( sal_Bool bDebugFindNoErrors );
};

SV_DECL_IMPL_REF(TTBasic)

#endif
