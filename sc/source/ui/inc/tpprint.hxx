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



#ifndef SC_TPPRINT_HXX
#define SC_TPPRINT_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

//===================================================================

class ScTpPrintOptions : public SfxTabPage
{
    FixedLine       aPagesFL;
    CheckBox        aSkipEmptyPagesCB;
    FixedLine       aSheetsFL;
    CheckBox        aSelectedSheetsCB;

            ScTpPrintOptions( Window* pParent, const SfxItemSet& rCoreSet );
            ~ScTpPrintOptions();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static sal_uInt16*      GetRanges();
    virtual sal_Bool        FillItemSet( SfxItemSet& rCoreSet );
    virtual void        Reset( const SfxItemSet& rCoreSet );
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );
};

#endif

