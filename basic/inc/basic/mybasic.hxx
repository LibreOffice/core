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



#ifndef _MYBASIC_HXX
#define _MYBASIC_HXX

#include <basic/sbstar.hxx>

class BasicApp;
class AppBasEd;
class ErrorEntry;

class BasicError {
    AppBasEd* pWin;
    sal_uInt16  nLine, nCol1, nCol2;
    String aText;
public:
    BasicError( AppBasEd*, sal_uInt16, const String&, sal_uInt16, sal_uInt16, sal_uInt16 );
    void Show();
};

DECLARE_LIST( ErrorList, BasicError* )

#define SBXID_MYBASIC   0x594D      // MyBasic: MY
#define SBXCR_TEST      0x54534554  // TEST

class MyBasic : public StarBASIC
{
    SbError nError;
    virtual sal_Bool ErrorHdl();
    virtual sal_uInt16 BreakHdl();

protected:
    Link GenLogHdl();
    Link GenWinInfoHdl();
    Link GenModuleWinExistsHdl();
    Link GenWriteStringHdl();

    virtual void StartListeningTT( SfxBroadcaster &rBroadcaster );

    String GenRealString( const String &aResString );

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_TEST,SBXID_MYBASIC,1);
    ErrorList aErrors;
    MyBasic();
    virtual ~MyBasic();
    virtual sal_Bool Compile( SbModule* );
    void Reset();
    SbError GetErrors() { return nError; }

        // Do not use #ifdefs here because this header file is both used for testtool and basic
    SbxObject *pTestObject; // for Testool; otherwise NULL

    virtual void LoadIniFile();

    // Determines the extended symbol type for syntax highlighting
    virtual SbTextType GetSymbolType( const String &Symbol, sal_Bool bWasTTControl );
    virtual const String GetSpechialErrorText();
    virtual void ReportRuntimeError( AppBasEd *pEditWin );
    virtual void DebugFindNoErrors( sal_Bool bDebugFindNoErrors );

    static void SetCompileModule( SbModule *pMod );
    static SbModule *GetCompileModule();
};

SV_DECL_IMPL_REF(MyBasic)

#endif
