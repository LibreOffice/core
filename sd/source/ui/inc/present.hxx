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




#ifndef _SD_PRESENT_HXX_
#define _SD_PRESENT_HXX_

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <vcl/fixed.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class SfxItemSet;
class List;

/*************************************************************************
|* Dialog zum Festlegen von Optionen und Starten der Praesentation
\************************************************************************/
class SdStartPresentationDlg : public ModalDialog
{
private:

    FixedLine           aGrpRange;
    RadioButton         aRbtAll;
    RadioButton         aRbtAtDia;
    RadioButton         aRbtCustomshow;
    ListBox             aLbDias;
    ListBox             aLbCustomshow;

    FixedLine           aGrpKind;
    RadioButton         aRbtStandard;
    RadioButton         aRbtWindow;
    RadioButton         aRbtAuto;
    TimeField           aTmfPause;
    CheckBox            aCbxAutoLogo;

    FixedLine           aGrpOptions;
    CheckBox            aCbxManuel;
    CheckBox            aCbxMousepointer;
    CheckBox            aCbxPen;
    CheckBox            aCbxNavigator;
    CheckBox            aCbxAnimationAllowed;
    CheckBox            aCbxChangePage;
    CheckBox            aCbxAlwaysOnTop;

    FixedLine           maGrpMonitor;
    FixedText           maFtMonitor;
    ListBox             maLBMonitor;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    List*               pCustomShowList;
    const SfxItemSet&   rOutAttrs;
    sal_Int32           mnMonitors;

    String              msPrimaryMonitor;
    String              msMonitor;
    String              msAllMonitors;

                        DECL_LINK( ChangeRangeHdl, void * );
                        DECL_LINK( ClickWindowPresentationHdl, void * );
                        DECL_LINK( ChangePauseHdl, void * );

    void                InitMonitorSettings();

public:
                        SdStartPresentationDlg( Window* pWindow,
                                const SfxItemSet& rInAttrs,
                                List& rPageNames,
                                List* pCSList );

    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // _SD_PRESENT_HXX_

