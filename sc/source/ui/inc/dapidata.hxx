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



#ifndef SC_DAPIDATA_HXX
#define SC_DAPIDATA_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

//------------------------------------------------------------------------

struct ScImportSourceDesc;


class ScDataPilotDatabaseDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    FixedText       aFtDatabase;
    ListBox         aLbDatabase;
    FixedText       aFtType;
    ListBox         aLbType;
    FixedText       aFtObject;
    ComboBox        aCbObject;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    void    FillObjects();

    DECL_LINK( SelectHdl, ListBox* );

public:
            ScDataPilotDatabaseDlg( Window* pParent );
            ~ScDataPilotDatabaseDlg();

    void    GetValues( ScImportSourceDesc& rDesc );
};


#endif


