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


#ifndef _SD_MASTERLAYOUT_DIALOG_HXX
#define _SD_MASTERLAYOUT_DIALOG_HXX

#include <vcl/dialog.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>

#include "sdpage.hxx"

class SdDrawDocument;

namespace sd
{

class MasterLayoutDialog : public ModalDialog
{
private:
    SdDrawDocument* mpDoc;
    SdPage*         mpCurrentPage;

    FixedLine       maFLPlaceholders;
    CheckBox        maCBDate;
    CheckBox        maCBPageNumber;
    CheckBox        maCBHeader;
    CheckBox        maCBFooter;

    OKButton        maPBOK;
    CancelButton    maPBCancel;

    bool            mbOldHeader;
    bool            mbOldFooter;
    bool            mbOldDate;
    bool            mbOldPageNumber;

    void applyChanges();
    void remove( PresObjKind eKind );
    void create( PresObjKind eKind );

public:
    MasterLayoutDialog( Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~MasterLayoutDialog();

    virtual short Execute();
};

}

#endif // _SD_MASTERLAYOUT_DIALOG_HXX

