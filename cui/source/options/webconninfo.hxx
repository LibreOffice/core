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


#ifndef _SVX_WEBCONNINFO_HXX
#define _SVX_WEBCONNINFO_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>

//........................................................................
namespace svx
{
//........................................................................

    class PasswordTable : public SvxSimpleTable
    {
    public:
        PasswordTable( Window* pParent, const ResId& rResId );

        void InsertHeaderItem( sal_uInt16 nColumn, const String& rText, HeaderBarItemBits nBits );
        void ResetTabs();
        void Resort( bool bForced );
    };

    //====================================================================
    //= class WebConnectionIfoDialog
    //====================================================================
    class WebConnectionInfoDialog : public ModalDialog
    {
    private:
        FixedInfo           m_aNeverShownFI;
        PasswordTable       m_aPasswordsLB;
        PushButton          m_aRemoveBtn;
        PushButton          m_aRemoveAllBtn;
        PushButton          m_aChangeBtn;
        FixedLine           m_aButtonsFL;
        CancelButton        m_aCloseBtn;
        HelpButton          m_aHelpBtn;
        sal_Int32           m_nPos;

    DECL_LINK( HeaderBarClickedHdl, SvxSimpleTable* );
    DECL_LINK( RemovePasswordHdl, PushButton* );
    DECL_LINK( RemoveAllPasswordsHdl, PushButton* );
    DECL_LINK( ChangePasswordHdl, PushButton* );
    DECL_LINK( EntrySelectedHdl, void* );

    void FillPasswordList();

    public:
        WebConnectionInfoDialog( Window* pParent );
        ~WebConnectionInfoDialog();
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // #ifndef _SVX_WEBCONNINFO_HXX

