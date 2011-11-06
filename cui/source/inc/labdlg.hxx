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


#ifndef _SVX_LABDLG_HXX
#define _SVX_LABDLG_HXX

// include ---------------------------------------------------------------


#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/valueset.hxx>
#include <sfx2/tabdlg.hxx>
class SdrView;

// class SvxCaptionTabPage -----------------------------------------------

const sal_uInt16 CAPTYPE_BITMAPS_COUNT = 3;

class SvxCaptionTabPage : public SfxTabPage
{
private:
    ValueSet        aCT_CAPTTYPE;
    FixedText       aFT_ABSTAND;
    MetricField     aMF_ABSTAND;
    FixedText       aFT_WINKEL;
    ListBox         aLB_WINKEL;
    FixedText       aFT_ANSATZ;
    ListBox         aLB_ANSATZ;
    FixedText       aFT_UM;
    MetricField     aMF_ANSATZ;
    FixedText       aFT_ANSATZ_REL;
    ListBox         aLB_ANSATZ_REL;
    FixedText       aFT_LAENGE;
    MetricField     aMF_LAENGE;
    CheckBox        aCB_LAENGE;

    Image*          mpBmpCapTypes[CAPTYPE_BITMAPS_COUNT];
    Image*          mpBmpCapTypesH[CAPTYPE_BITMAPS_COUNT];

    String          aStrHorzList;
    String          aStrVertList;

    short           nCaptionType;
    sal_Bool            bFixedAngle;
    sal_Int32           nFixedAngle;
    sal_Int32           nGap;
    short           nEscDir;
    sal_Bool            bEscRel;
    sal_Int32           nEscAbs;
    sal_Int32           nEscRel;
    sal_Int32           nLineLen;
    sal_Bool            bFitLineLen;

    sal_uInt16          nAnsatzRelPos;
    sal_uInt16          nAnsatzTypePos;
    sal_uInt16          nWinkelTypePos;

#ifdef _SVX_LABDLG_CXX
    void            SetupAnsatz_Impl( sal_uInt16 nType );
    void            SetupType_Impl( sal_uInt16 nType );
    DECL_LINK( AnsatzSelectHdl_Impl, ListBox * );
    DECL_LINK( AnsatzRelSelectHdl_Impl, ListBox * );
    DECL_LINK( LineOptHdl_Impl, Button * );
    DECL_LINK( SelectCaptTypeHdl_Impl, void * );
#endif

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

public:
    SvxCaptionTabPage( Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxCaptionTabPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );
    void                Construct();
    void                SetView( const SdrView* pSdrView )
                            { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void FillValueSet();
};

// class SvxCaptionTabDialog ---------------------------------------------

class SvxCaptionTabDialog : public SfxTabDialog
{
private:
//  const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;
    sal_uInt16              nAnchorCtrls;

    Link                aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:

            SvxCaptionTabDialog(Window* pParent, const SdrView* pView,
                                    sal_uInt16 nAnchorTypes = 0 );

            ~SvxCaptionTabDialog();

            //link for the Writer to validate positions
            void SetValidateFramePosLink( const Link& rLink );
};


#endif //_SVX_LABDLG_HXX

