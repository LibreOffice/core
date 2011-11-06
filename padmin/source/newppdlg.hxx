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



#ifndef _PAD_NEWPPDLG_HXX_
#define _PAD_NEWPPDLG_HXX_

#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

namespace psp { class PPDParser; }

namespace padmin {

    class PPDImportDialog : public ModalDialog
    {
        OKButton            m_aOKBtn;
        CancelButton        m_aCancelBtn;
        FixedText           m_aPathTxt;
        ComboBox            m_aPathBox;
        PushButton          m_aSearchBtn;
        FixedText           m_aDriverTxt;
        MultiListBox        m_aDriverLB;

        FixedLine           m_aPathGroup;
        FixedLine           m_aDriverGroup;

        String              m_aLoadingPPD;

        DECL_LINK( ClickBtnHdl, PushButton* );
        DECL_LINK( SelectHdl, ComboBox* );
        DECL_LINK( ModifyHdl, ComboBox* );

        void Import();

        std::list< rtl::OUString >  m_aImportedFiles;
    public:
        PPDImportDialog( Window* pParent );
        ~PPDImportDialog();

        const std::list< rtl::OUString >& getImportedFiles() const
        { return m_aImportedFiles; }
    };

} // namespace

#endif // _NEWPPDLG_HXX
