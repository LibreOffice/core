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



#ifndef SC_DAPITYPE_HXX
#define SC_DAPITYPE_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>

//------------------------------------------------------------------------

class ScDataPilotSourceTypeDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    RadioButton     aBtnSelection;
    RadioButton     aBtnDatabase;
    RadioButton     aBtnExternal;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotSourceTypeDlg( Window* pParent, sal_Bool bEnableExternal );
            ~ScDataPilotSourceTypeDlg();

    sal_Bool    IsDatabase() const;
    sal_Bool    IsExternal() const;
};


class ScDataPilotServiceDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    FixedText       aFtService;
    ListBox         aLbService;
    FixedText       aFtSource;
    Edit            aEdSource;
    FixedText       aFtName;
    Edit            aEdName;
    FixedText       aFtUser;
    Edit            aEdUser;
    FixedText       aFtPasswd;
    Edit            aEdPasswd;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotServiceDlg( Window* pParent,
                    const com::sun::star::uno::Sequence<rtl::OUString>& rServices );
            ~ScDataPilotServiceDlg();

    String  GetServiceName() const;
    String  GetParSource() const;
    String  GetParName() const;
    String  GetParUser() const;
    String  GetParPass() const;
};


#endif


