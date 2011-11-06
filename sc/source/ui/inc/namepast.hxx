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



#ifndef SC_NAMEPAST_HXX
#define SC_NAMEPAST_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include "scui_def.hxx" //CHINA001
//CHINA001 #define BTN_PASTE_NAME   100
//CHINA001 #define BTN_PASTE_LIST   101
//CHINA001
class ScRangeName;

class ScNamePasteDlg : public ModalDialog
{
    DECL_LINK( ButtonHdl, Button * );
    DECL_LINK( ListSelHdl, ListBox * );
    DECL_LINK( ListDblClickHdl, ListBox * );

protected:
    FixedText       aLabelText;
    ListBox         aNameList;
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    HelpButton      aHelpButton;
    PushButton      aInsListButton;
public:
    ScNamePasteDlg( Window * pParent, const ScRangeName* pList, sal_Bool bInsList=sal_True );

    String          GetSelectedName() const;
};


#endif //SC_NAMEPAST_HXX
