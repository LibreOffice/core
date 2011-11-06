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


#ifndef INCLUDED_SFX_MAILMODEL_HXX
#define INCLUDED_SFX_MAILMODEL_HXX

#include <com/sun/star/frame/XFrame.hpp>

// class SfxMailModel_Impl -----------------------------------------------

class AddressList_Impl;

class SfxMailModel_Impl
{
public:
    enum MailPriority
    {
        PRIO_HIGHEST,
        PRIO_HIGH,
        PRIO_NORMAL,
        PRIO_LOW,
        PRIO_LOWEST
    };

    enum AddressRole
    {
        ROLE_TO,
        ROLE_CC,
        ROLE_BCC
    };

    enum MailDocType
    {
        TYPE_SELF,
        TYPE_ASPDF
    };

private:
    enum SaveResult
    {
        SAVE_SUCCESSFULL,
        SAVE_CANCELLED,
        SAVE_ERROR
    };

    AddressList_Impl*   mpToList;
    AddressList_Impl*   mpCcList;
    AddressList_Impl*   mpBccList;
    String              maFromAddress;
    String              maSubject;
    MailPriority        mePriority;

    sal_Bool            mbLoadDone;

    void                ClearList( AddressList_Impl* pList );
    void                MakeValueList( AddressList_Impl* pList, String& rValueList );
    SaveResult          SaveDocumentAsFormat( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const rtl::OUString& rType, rtl::OUString& rFileNamePath );

    DECL_LINK( DoneHdl, void* );

public:
    enum SendMailResult
    {
        SEND_MAIL_OK,
        SEND_MAIL_CANCELLED,
        SEND_MAIL_ERROR
    };

    SfxMailModel_Impl();
    ~SfxMailModel_Impl();

    void                AddAddress( const String& rAddress, AddressRole eRole );
    void                SetFromAddress( const String& rAddress )    { maFromAddress = rAddress; }
    void                SetSubject( const String& rSubject )        { maSubject = rSubject; }
    void                SetPriority( MailPriority ePrio )           { mePriority = ePrio; }

    SendMailResult      Send( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const rtl::OUString& rType );
};

sal_Bool CreateFromAddress_Impl( String& rFrom );

#endif // INCLUDED_SFX_MAILMODEL_HXX
