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



#ifndef EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX
#define EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

#include "abspage.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= AdminDialogInvokationPage
    //=====================================================================
    class AdminDialogInvokationPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aExplanation;
        PushButton      m_aInvokeAdminDialog;
        FixedText       m_aErrorMessage;

        sal_Bool        m_bSuccessfullyExecutedDialog;

    public:
        AdminDialogInvokationPage( OAddessBookSourcePilot* _pParent );

    protected:
        // TabDialog overridables
        virtual void        ActivatePage();
        virtual void        initializePage();

        // OImportPage overridables
        virtual bool        canAdvance() const;

    private:
        DECL_LINK( OnInvokeAdminDialog, void* );

        void implTryConnect();
        void implUpdateErrorMessage();
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX

