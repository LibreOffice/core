/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_MAILMERGEHELPER_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_MAILMERGEHELPER_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/mail/XAuthenticator.hpp>
#include <com/sun/star/mail/XConnectionListener.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/mail/XMailMessage.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/compbase.hxx>
#include <utility>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <rtl/ustring.hxx>
#include <swdllapi.h>

class SwMailMergeConfigItem;

namespace com::sun::star::mail {
    class XMailService;
    class XSmtpService;
}

namespace SwMailMergeHelper
{
    SW_DLLPUBLIC OUString CallSaveAsDialog(weld::Window* pParent, OUString& rFilter);
    SW_DLLPUBLIC bool CheckMailAddress(std::u16string_view aMailAddress);
    SW_DLLPUBLIC css::uno::Reference<css::mail::XSmtpService> ConnectToSmtpServer(
                            SwMailMergeConfigItem const & rConfigItem,
                            css::uno::Reference<css::mail::XMailService>& xInMailService,
                            const OUString& rInMailServerPassword,
                            const OUString& rOutMailServerPassword,
                            weld::Window* pDialogParentWindow = nullptr);
}

struct SwAddressPreview_Impl;

// Preview window used to show the possible selection of address blocks
// and also the resulting address filled with database data
class SW_DLLPUBLIC SwAddressPreview final : public weld::CustomWidgetController
{
    std::unique_ptr<SwAddressPreview_Impl> m_pImpl;
    std::unique_ptr<weld::ScrolledWindow> m_xVScrollBar;
    Link<LinkParamNone*,void> m_aSelectHdl;

    void DrawText_Impl(vcl::RenderContext& rRenderContext, std::u16string_view rAddress,
                       const Point& rTopLeft, const Size& rSize, bool bIsSelected);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;
    void UpdateScrollBar();

    DECL_DLLPRIVATE_LINK(ScrollHdl, weld::ScrolledWindow&,void);

public:
    SwAddressPreview(std::unique_ptr<weld::ScrolledWindow> xParent);
    virtual ~SwAddressPreview() override;

    /** The address string is a list of address elements separated by spaces
    and breaks. The addresses fit into the given layout. If more addresses then
    rows/columns should be used a scrollbar will be added.

     AddAddress appends the new address to the already added ones.
     Initially the first added address will be selected
    */
    void AddAddress(const OUString& rAddress);
    //  for preview mode - replaces the currently used address by the given one
    void SetAddress(const OUString& rAddress);
    // removes all addresses
    void Clear();

    // returns the selected address
    sal_uInt16 GetSelectedAddress() const;
    void SelectAddress(sal_uInt16 nSelect);
    void ReplaceSelectedAddress(const OUString&);
    void RemoveSelectedAddress();

    // set the number of rows and columns of addresses
    void SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns);
    void EnableScrollBar();

    // fill the actual data into a string (address block or greeting)
    static OUString FillData(const OUString& rAddress, SwMailMergeConfigItem const & rConfigItem,
                             const css::uno::Sequence<OUString>* pAssignments = nullptr);

    void SetSelectHdl (const Link<LinkParamNone*,void>& rLink) { m_aSelectHdl = rLink; }
};


// iterate over an address block or a greeting line the iterator returns the
// parts either as pure string or as column
struct SwMergeAddressItem
{
    OUString sText;
    bool bIsColumn;
    bool bIsReturn;

    SwMergeAddressItem()
        : bIsColumn(false)
        , bIsReturn(false)
    {}
};

class SW_DLLPUBLIC SwAddressIterator
{
    OUString m_sAddress;
public:
    SwAddressIterator(OUString aAddress) :
        m_sAddress(std::move(aAddress))
    {}

    SwMergeAddressItem  Next();
    bool HasMore() const { return !m_sAddress.isEmpty(); }
};

class SW_DLLPUBLIC SwAuthenticator final :
    public cppu::WeakImplHelper<css::mail::XAuthenticator>
{
    OUString m_aUserName;
    OUString m_aPassword;
    weld::Window* m_pParentWindow;
public:
    SwAuthenticator()
        : m_pParentWindow(nullptr)
    {}
    SwAuthenticator(OUString username, OUString password, weld::Window* pParent)
        : m_aUserName(std::move(username))
        , m_aPassword(std::move(password))
        , m_pParentWindow(pParent)
    {}
    virtual ~SwAuthenticator() override;

    virtual OUString SAL_CALL getUserName() override;
    virtual OUString SAL_CALL getPassword() override;

};

class SW_DLLPUBLIC SwConnectionContext final : public cppu::WeakImplHelper<css::uno::XCurrentContext>
{
    OUString m_sMailServer;
    sal_Int16 m_nPort;
    OUString m_sConnectionType;

public:
    SwConnectionContext(OUString aMailServer, sal_Int16 nPort, OUString aConnectionType);
    virtual ~SwConnectionContext() override;

    virtual css::uno::Any SAL_CALL getValueByName(const OUString& Name) override;
};

class SW_DLLPUBLIC SwConnectionListener final :
        public comphelper::WeakComponentImplHelper<css::mail::XConnectionListener>
{
    using comphelper::WeakComponentImplHelperBase::disposing;

public:
    virtual ~SwConnectionListener() override;

    virtual void SAL_CALL connected(const css::lang::EventObject& aEvent) override;

    virtual void SAL_CALL disconnected(const css::lang::EventObject& aEvent) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;
};

class SW_DLLPUBLIC SwMailTransferable final :
        public comphelper::WeakComponentImplHelper<css::datatransfer::XTransferable, css::beans::XPropertySet>
{
    OUString  m_aMimeType;
    OUString  m_sBody;
    OUString  m_aURL;
    OUString  m_aName;
    bool m_bIsBody;

    public:
    SwMailTransferable(OUString aURL, OUString aName, OUString aMimeType);
    SwMailTransferable(OUString aBody, OUString aMimeType);
    virtual ~SwMailTransferable() override;
    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& aFlavor) override;

    virtual css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& aFlavor) override;

    //XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const css::uno::Any& aValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName,
                                                    const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName,
                                                       const css::uno::Reference<css::beans::XPropertyChangeListener >& aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName,
                                                    const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,
                                                       const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

};

class SAL_DLLPUBLIC_RTTI SwMailMessage final :
        public comphelper::WeakComponentImplHelper<css::mail::XMailMessage>
{
    OUString m_sSenderName;
    OUString m_sSenderAddress;
    OUString m_sReplyToAddress;
    OUString m_sSubject;

    css::uno::Reference<css::datatransfer::XTransferable> m_xBody;
//  css::mail::MailMessageBody m_aBody;

    css::uno::Sequence<OUString> m_aRecipients;
    css::uno::Sequence<OUString> m_aCcRecipients;
    css::uno::Sequence<OUString> m_aBccRecipients;
//  css::uno::Sequence<css::mail::MailAttachmentDescriptor> m_aAttachments;
    css::uno::Sequence<css::mail::MailAttachment> m_aAttachments;
public:
    SW_DLLPUBLIC SwMailMessage();
    virtual ~SwMailMessage() override;

    // attributes
    virtual OUString SAL_CALL getSenderName() override;
    virtual OUString SAL_CALL getSenderAddress() override;
    virtual OUString SAL_CALL getReplyToAddress() override;
    SW_DLLPUBLIC virtual void SAL_CALL setReplyToAddress( const OUString& _replytoaddress ) override;
    virtual OUString SAL_CALL getSubject() override;
    SW_DLLPUBLIC virtual void SAL_CALL setSubject(const OUString& _subject) override;

    virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getBody() override;
    SW_DLLPUBLIC virtual void SAL_CALL setBody(const css::uno::Reference<css::datatransfer::XTransferable>& _body) override;

    // methods
    SW_DLLPUBLIC virtual void SAL_CALL addRecipient( const OUString& sRecipientAddress ) override;
    SW_DLLPUBLIC virtual void SAL_CALL addCcRecipient( const OUString& sRecipientAddress ) override;
    SW_DLLPUBLIC virtual void SAL_CALL addBccRecipient( const OUString& sRecipientAddress ) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getRecipients() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getCcRecipients() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getBccRecipients() override;
    SW_DLLPUBLIC virtual void SAL_CALL addAttachment(const css::mail::MailAttachment& aMailAttachment) override;
    virtual css::uno::Sequence<css::mail::MailAttachment> SAL_CALL getAttachments() override;
    void SetSenderName(const OUString& rSenderName)
    {
        m_sSenderName = rSenderName;
    }
    void SetSenderAddress(const OUString& rSenderAddress)
    {
        m_sSenderAddress = rSenderAddress;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
