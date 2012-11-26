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




#ifndef _SD_DLGSNAP_HXX
#define _SD_DLGSNAP_HXX


#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <tools/fract.hxx>
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include "sdenumdef.hxx"
/************************************************************************/

class SfxItemSet;
namespace sd {
    class View;
}

/*************************************************************************
|*
|* Dialog zum Einstellen von Fanglinien und -punkten
|*
\************************************************************************/

class SdSnapLineDlg : public ModalDialog
{
private:
    FixedLine           aFlPos;
    FixedText           aFtX;
    MetricField         aMtrFldX;
    FixedText           aFtY;
    MetricField         aMtrFldY;
    FixedLine           aFlDir;
    ImageRadioButton    aRbPoint;
    ImageRadioButton    aRbVert;
    ImageRadioButton    aRbHorz;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    PushButton          aBtnDelete;
    long                nXValue;
    long                nYValue;
    FieldUnit           eUIUnit;
    Fraction            aUIScale;

    DECL_LINK( ClickHdl, Button * );

public:
    SdSnapLineDlg(Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView);

    void GetAttr(SfxItemSet& rOutAttrs);

    void HideRadioGroup();
    void HideDeleteBtn() { aBtnDelete.Hide(); }
    void SetInputFields(bool bEnableX, bool bEnableY);
};



#endif      // _SD_DLGSNAP_HXX
