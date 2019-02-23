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

#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/mail/XAuthenticator.hpp>
#include <com/sun/star/mail/XConnectionListener.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/mail/XMailMessage.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <rtl/ustring.hxx>
#include <swdllapi.h>

class SwMailMergeConfigItem;

namespace com { namespace sun { namespace star { namespace mail {
    class XMailService;
    class XSmtpService;
} } } }

namespace SwMailMergeHelper
{
    SW_DLLPUBLIC OUString CallSaveAsDialog(weld::Window* pParent, OUString& rFilter);
    SW_DLLPUBLIC bool CheckMailAddress(const OUString& rMailAddress);
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
class SW_DLLPUBLIC SwAddressPreview : public vcl::Window
{
    VclPtr<ScrollBar> aVScrollBar;
    SwAddressPreview_Impl* pImpl;
    Link<LinkParamNone*,void> m_aSelectHdl;

    void DrawText_Impl(vcl::RenderContext& rRenderContext, const OUString& rAddress,
                       const Point& rTopLeft, const Size& rSize, bool bIsSelected);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void Resize() override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void StateChanged( StateChangedType nStateChange ) override;
    void UpdateScrollBar();

    DECL_LINK(ScrollHdl, ScrollBar*,void);

public:
    SwAddressPreview(vcl::Window* pParent, WinBits nStyle);
    virtual ~SwAddressPreview() override;
    virtual void dispose() override;

    void positionScrollBar();

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

    // set the number of rows and columns of addresses
    void SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns);

    // fill the actual data into a string (address block or greeting)
    static OUString FillData(const OUString& rAddress, SwMailMergeConfigItem const & rConfigItem,
                             const css::uno::Sequence<OUString>* pAssignments = nullptr);

    void SetSelectHdl (const Link<LinkParamNone*,void>& rLink) { m_aSelectHdl = rLink; }
};

class SW_DLLPUBLIC AddressPreview : public weld::CustomWidgetController
{
    std::unique_ptr<SwAddressPreview_Impl> pImpl;
    std::unique_ptr<weld::ScrolledWindow> m_xVScrollBar;

    void DrawText_Impl(vcl::RenderContext& rRenderContext, const OUString& rAddress,
                       const Point& rTopLeft, const Size& rSize, bool bIsSelected);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;
    void UpdateScrollBar();

    DECL_LINK(ScrollHdl, weld::ScrolledWindow&,void);

public:
    AddressPreview(std::unique_ptr<weld::ScrolledWindow> xParent);
    virtual ~AddressPreview() override;

    /** The address string is a list of address elements separated by spaces
    and breaks. The addresses fit into the given layout. If more addresses then
    rows/columns should be used a scrollbar will be added.

     AddAddress appends the new address to the already added ones.
     Initially the first added address will be selected
    */
    void AddAddress(const OUString& rAddress);
    //  for preview mode - replaces the currently used address by the given one
    void SetAddress(const OUString& rAddress);

    // returns the selected address
    sal_uInt16 GetSelectedAddress() const;
    void SelectAddress(sal_uInt16 nSelect);
    void ReplaceSelectedAddress(const OUString&);
    void RemoveSelectedAddress();

    // set the number of rows and columns of addresses
    void SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns);
    void EnableScrollBar();
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
    OUString sAddress;
public:
    SwAddressIterator(const OUString& rAddress) :
        sAddress(rAddress)
    {}

    SwMergeAddressItem  Next();
    bool HasMore() const { return !sAddress.isEmpty(); }
};

class SW_DLLPUBLIC SwAuthenticator :
    public cppu::WeakImplHelper<css::mail::XAuthenticator>
{
    OUString const m_aUserName;
    OUString m_aPassword;
    weld::Window* m_pParentWindow;
public:
    SwAuthenticator()
        : m_pParentWindow(nullptr)
    {}
    SwAuthenticator(const OUString& username, const OUString& password, weld::Window* pParent)
        : m_aUserName(username)
        , m_aPassword(password)
        , m_pParentWindow(pParent)
    {}
    virtual ~SwAuthenticator() override;

    virtual OUString SAL_CALL getUserName() override;
    virtual OUString SAL_CALL getPassword() override;

};

class SW_DLLPUBLIC SwConnectionContext : public cppu::WeakImplHelper<css::uno::XCurrentContext>
{
    OUString const m_sMailServer;
    sal_Int16 const m_nPort;
    OUString const m_sConnectionType;

public:
    SwConnectionContext(const OUString& rMailServer, sal_Int16 nPort, const OUString& rConnectionType);
    virtual ~SwConnectionContext() override;

    virtual css::uno::Any SAL_CALL getValueByName(const OUString& Name) override;
};

class SwMutexBase
{
public:
    osl::Mutex m_aMutex;
};

class SW_DLLPUBLIC SwConnectionListener :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper<css::mail::XConnectionListener>
{
    using cppu::WeakComponentImplHelperBase::disposing;

public:
    SwConnectionListener() :
        cppu::WeakComponentImplHelper<css::mail::XConnectionListener>(m_aMutex)
    {}
    virtual ~SwConnectionListener() override;

    virtual void SAL_CALL connected(const css::lang::EventObject& aEvent) override;

    virtual void SAL_CALL disconnected(const css::lang::EventObject& aEvent) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;
};

class SW_DLLPUBLIC SwMailTransferable :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper<css::datatransfer::XTransferable, css::beans::XPropertySet>
{
    OUString const  m_aMimeType;
    OUString const  m_sBody;
    OUString const  m_aURL;
    OUString const  m_aName;
    bool const m_bIsBody;

    public:
    SwMailTransferable(const OUString& rURL, const OUString& rName, const OUString& rMimeType);
    SwMailTransferable(const OUString& rBody, const OUString& rMimeType);
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

class SW_DLLPUBLIC SwMailMessage :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper<css::mail::XMailMessage>
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
    SwMailMessage();
    virtual ~SwMailMessage() override;

    // attributes
    virtual OUString SAL_CALL getSenderName() override;
    virtual OUString SAL_CALL getSenderAddress() override;
    virtual OUString SAL_CALL getReplyToAddress() override;
    virtual void SAL_CALL setReplyToAddress( const OUString& _replytoaddress ) override;
    virtual OUString SAL_CALL getSubject() override;
    virtual void SAL_CALL setSubject(const OUString& _subject) override;

    virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getBody() override;
    virtual void SAL_CALL setBody(const css::uno::Reference<css::datatransfer::XTransferable>& _body) override;

    // methods
    virtual void SAL_CALL addRecipient( const OUString& sRecipientAddress ) override;
    virtual void SAL_CALL addCcRecipient( const OUString& sRecipientAddress ) override;
    virtual void SAL_CALL addBccRecipient( const OUString& sRecipientAddress ) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getRecipients() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getCcRecipients() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getBccRecipients() override;
    virtual void SAL_CALL addAttachment(const css::mail::MailAttachment& aMailAttachment) override;
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
