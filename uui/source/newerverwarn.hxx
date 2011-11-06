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


#ifndef _NEWERVERSIONWARNING_HXX
#define _NEWERVERSIONWARNING_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>

#define RET_ASK_LATER   short( 100 )

namespace uui
{
    class NewerVersionWarningDialog : public ModalDialog
    {
    private:
        FixedImage      m_aImage;
        FixedText       m_aInfoText;
        FixedLine       m_aButtonLine;
        PushButton      m_aUpdateBtn;
        CancelButton    m_aLaterBtn;

        ::rtl::OUString m_sVersion;

        DECL_LINK(      UpdateHdl, PushButton* );
        DECL_LINK(      LaterHdl, CancelButton* );

        void            InitButtonWidth();

    public:
        NewerVersionWarningDialog( Window* pParent, const ::rtl::OUString& rVersion, ResMgr& rResMgr );
        ~NewerVersionWarningDialog();
    };
} // namespace uui

#endif // #ifndef _NEWERVERSIONWARNING_HXX

