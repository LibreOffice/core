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



#ifndef SD_COPY_DLG_HXX
#define SD_COPY_DLG_HXX

#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svx/dlgctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

class XColorList;

namespace sd {

class View;

/*************************************************************************
|*
|* Dialog zum Einstellen des Bildschirms
|*
\************************************************************************/
class CopyDlg
    : public SfxModalDialog
{
public:
    CopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
        XColorList* pColTab, ::sd::View* pView );
    ~CopyDlg();

    void    GetAttr( SfxItemSet& rOutAttrs );
    DECL_LINK( Reset, void* );

private:
    FixedText           maFtCopies;
    NumericField        maNumFldCopies;
    ImageButton         maBtnSetViewData;

    FixedLine           maGrpMovement;
    FixedText           maFtMoveX;
    MetricField         maMtrFldMoveX;
    FixedText           maFtMoveY;
    MetricField         maMtrFldMoveY;
    FixedText           maFtAngle;
    MetricField         maMtrFldAngle;
    FixedLine           maGrpEnlargement;

    FixedText           maFtWidth;
    MetricField         maMtrFldWidth;
    FixedText           maFtHeight;
    MetricField         maMtrFldHeight;

    FixedLine           maGrpColor;
    FixedText           maFtStartColor;
    ColorLB             maLbStartColor;
    FixedText           maFtEndColor;
    ColorLB             maLbEndColor;

    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    PushButton          maBtnSetDefault;

    const SfxItemSet&   mrOutAttrs;
    XColorList*     mpColorTab;
    Fraction            maUIScale;
    ::sd::View*         mpView;

    DECL_LINK( SelectColorHdl, void * );
    DECL_LINK( SetViewData, void * );
    DECL_LINK( SetDefault, void * );
};

} // end of namespace sd

#endif

