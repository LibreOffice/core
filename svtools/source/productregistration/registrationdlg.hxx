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



#ifndef SVTOOLS_REGISTRATIONDLG_HXX
#define SVTOOLS_REGISTRATIONDLG_HXX

#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= RegistrationDialog
    //====================================================================
    class RegistrationDialog : public ModalDialog
    {
    public:
        enum UserResponse
        {
            urRegisterNow,
            urRegisterLater,
            urRegisterNever,
            urAlreadyRegistered
        };

    private:
        UserResponse    m_eResponse;

        FixedImage      m_aLogo;
        FixedText       m_aIntro;

        RadioButton     m_aNow;
        RadioButton     m_aLater;
        RadioButton     m_aNever;
        RadioButton     m_aAlreadyDone;

        FixedLine       m_aSeparator;

        OKButton        m_aOK;
        HelpButton      m_aHelp;

    public:
        RegistrationDialog( Window* _pWindow, const ResId& _rResId, bool _bEvalVersion );

        virtual short   Execute();
        virtual long    PreNotify( NotifyEvent& rNEvt );

        inline  UserResponse    getResponse() const { return m_eResponse; }
    };



//........................................................................
}// namespace svt
//........................................................................

#endif // SVTOOLS_REGISTRATIONDLG_HXX
