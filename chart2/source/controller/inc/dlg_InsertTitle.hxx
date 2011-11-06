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


#ifndef _CHART2_DLG_INSERT_TITLE_GRID_HXX
#define _CHART2_DLG_INSERT_TITLE_GRID_HXX

#include "TitleDialogData.hxx"
#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
//for auto_ptr
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
class TitleResources;
class SchTitleDlg : public ModalDialog
{
private:
    ::std::auto_ptr< TitleResources >  m_apTitleResources;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

public:
    SchTitleDlg( Window* pParent, const TitleDialogData& rInput );
    virtual ~SchTitleDlg();

    void getResult( TitleDialogData& rOutput );
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

