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


#ifndef _SVX_OPTSAVE_HXX
#define _SVX_OPTSAVE_HXX

// include ---------------------------------------------------------------

#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/uno/Reference.h>
#include <readonlyimage.hxx>

namespace com { namespace sun { namespace star {
  namespace beans {
    struct PropertyValue;
}}}}

// define ----------------------------------------------------------------

#define SfxSaveTabPage SvxSaveTabPage

// class SvxSaveTabPage --------------------------------------------------

struct SvxSaveTabPage_Impl;

class SvxSaveTabPage : public SfxTabPage
{
private:
    FixedLine               aLoadFL;
    CheckBox                aLoadUserSettingsCB;
    CheckBox                aLoadDocPrinterCB;

    FixedLine               aSaveFL;
    CheckBox                aDocInfoCB;
    ReadOnlyImage           aBackupFI;
    CheckBox                aBackupCB;
    CheckBox                aAutoSaveCB;
    NumericField            aAutoSaveEdit;
    FixedText               aMinuteFT;
    CheckBox                aRelativeFsysCB;
    CheckBox                aRelativeInetCB;

    FixedLine               aDefaultFormatFL;
    FixedText               aODFVersionFT;
    ListBox                 aODFVersionLB;
    CheckBox                aODFEncryptionCB;
    CheckBox                aSizeOptimizationCB;
    CheckBox                aWarnAlienFormatCB;
    FixedText               aDocTypeFT;
    ListBox                 aDocTypeLB;
    FixedText               aSaveAsFT;
    ReadOnlyImage           aSaveAsFI;
    ListBox                 aSaveAsLB;
    FixedImage              aODFWarningFI;
    FixedText               aODFWarningFT;

    SvxSaveTabPage_Impl*    pImpl;

#ifdef _SVX_OPTSAVE_CXX
    DECL_LINK( AutoClickHdl_Impl, CheckBox * );
    DECL_LINK( FilterHdl_Impl, ListBox * );
    DECL_LINK( ODFVersionHdl_Impl, ListBox * );

    bool    AcceptFilter( sal_uInt16 nPos );
    void    DetectHiddenControls();
#endif

public:
    SvxSaveTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxSaveTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif  // #ifndef _SVX_OPTSAVE_HXX

