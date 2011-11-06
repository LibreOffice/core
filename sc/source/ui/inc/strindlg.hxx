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



#ifndef SC_STRINDLG_HXX
#define SC_STRINDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

//------------------------------------------------------------------------

class ScStringInputDlg : public ModalDialog
{
public:
            ScStringInputDlg(     Window* pParent,
                            const String& rTitle,
                            const String& rEditTitle,
                            const String& rDefault,
                            const rtl::OString& sHelpId, const rtl::OString& sEditHelpId );
            ~ScStringInputDlg();

    void GetInputString( String& rString ) const;

private:
    FixedText       aFtEditTitle;
    Edit            aEdInput;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
};

#include <layout/layout-post.hxx>

#endif // SC_STRINDLG_HXX


