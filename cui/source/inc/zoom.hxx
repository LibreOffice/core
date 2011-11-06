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


#ifndef _SVX_ZOOM_HXX
#define _SVX_ZOOM_HXX

// include ---------------------------------------------------------------

#include <layout/layout.hxx>
#include <sfx2/basedlgs.hxx>
#ifndef _SV_BUTTON_HXX //autogen wg. Radio-/OK-/Cancel-/HelpButton
#include <vcl/button.hxx>
#endif
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

// define ----------------------------------------------------------------
/* CHINA001
#define ZOOMBTN_OPTIMAL     ((sal_uInt16)0x0001)
#define ZOOMBTN_PAGEWIDTH   ((sal_uInt16)0x0002)
#define ZOOMBTN_WHOLEPAGE   ((sal_uInt16)0x0004)
*/
#ifndef _SVX_ZOOM_HXX
#include "zoom_def.hxx"
#endif
// class SvxZoomDialog ---------------------------------------------------
/*
    {k:\svx\prototyp\dialog\zoom.bmp}

    [Beschreibung]
    Mit diesem Dialog wird ein Zoom-Faktor eingestellt.

    [Items]
    SvxZoomItem <SID_ATTR_ZOOM>
*/

#include <layout/layout-pre.hxx>

class SvxZoomDialog : public SfxModalDialog
{
private:
    FixedLine           aZoomFl;
    RadioButton         aOptimalBtn;
    RadioButton         aWholePageBtn;
    RadioButton         aPageWidthBtn;
    RadioButton         a100Btn;
    RadioButton         aUserBtn;
    MetricField         aUserEdit;

    FixedLine           aViewLayoutFl;
    RadioButton         aAutomaticBtn;
    RadioButton         aSingleBtn;
    RadioButton         aColumnsBtn;
    MetricField         aColumnsEdit;
    CheckBox            aBookModeChk;

    FixedLine           aBottomFl;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    const SfxItemSet&   rSet;
    SfxItemSet*         pOutSet;
    sal_Bool                bModified;

#ifdef _SVX_ZOOM_CXX
    DECL_LINK( UserHdl, RadioButton* );
    DECL_LINK( SpinHdl, MetricField* );
    DECL_LINK( ViewLayoutUserHdl, RadioButton* );
    DECL_LINK( ViewLayoutSpinHdl, MetricField* );
    DECL_LINK( ViewLayoutCheckHdl, CheckBox* );
    DECL_LINK( OKHdl, Button* );
#endif

public:
    SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet );
    ~SvxZoomDialog();

    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    sal_uInt16              GetFactor() const;
    void                SetFactor( sal_uInt16 nNewFactor, sal_uInt16 nBtnId = 0 );

    void                HideButton( sal_uInt16 nBtnId );
    void                SetLimits( sal_uInt16 nMin, sal_uInt16 nMax );
};

#include <layout/layout-post.hxx>

#endif

