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
#ifndef _MAILMERGEHELPER_HXX
#define _MAILMERGEHELPER_HXX

#include <svtools/stdctrl.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.h>
#include "com/sun/star/mail/XAuthenticator.hpp"
#include "com/sun/star/mail/XConnectionListener.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "com/sun/star/mail/XMailMessage.hpp"
#include "com/sun/star/datatransfer/XTransferable.hpp"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase2.hxx>
#include <vcl/scrbar.hxx>
#include "swdllapi.h"

class SwMailMergeConfigItem;

namespace com { namespace sun { namespace star { namespace mail {
    class XMailService;
    class XSmtpService;
} } } }

namespace SwMailMergeHelper
{
    SW_DLLPUBLIC String  CallSaveAsDialog(String& rFilter);
    SW_DLLPUBLIC bool    CheckMailAddress( const OUString& rMailAddress );
    SW_DLLPUBLIC com::sun::star::uno::Reference< com::sun::star::mail::XSmtpService >
                         ConnectToSmtpServer( SwMailMergeConfigItem& rConfigItem,
                            com::sun::star::uno::Reference< com::sun::star::mail::XMailService >&  xInMailService,
                            const String& rInMailServerPassword,
                            const String& rOutMailServerPassword,
                            Window* pDialogParentWindow = 0 );
}

class SW_DLLPUBLIC SwBoldFixedInfo : public FixedInfo
{
public:
    SwBoldFixedInfo(Window* pParent, const ResId& rResId);
    ~SwBoldFixedInfo();
};
struct SwAddressPreview_Impl;

/*-------------------------------------------------------------------------
    Preview window used to show the possible selection of address blocks
    and also the resulting address filled with database data
  -----------------------------------------------------------------------*/
class SW_DLLPUBLIC SwAddressPreview : public Window
{
    ScrollBar               aVScrollBar;
    SwAddressPreview_Impl*  pImpl;
    Link                    m_aSelectHdl;

    void DrawText_Impl( const OUString& rAddress, const Point& rTopLeft, const Size& rSize, bool bIsSelected);

    virtual void        Paint(const Rectangle&);
    virtual void        Resize();
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    void                UpdateScrollBar();

    DECL_LINK(ScrollHdl, void*);

public:
    SwAddressPreview(Window* pParent, const ResId& rResId);

    SwAddressPreview(Window* pParent, WinBits nStyle=WB_BORDER);

    void positionScrollBar();

    /** The address string is a list of address elements separated by spaces
    and breaks. The addresses fit into the given layout. If more addresses then
    rows/columns should be used a scrollbar will be added.

     AddAddress appends the new address to the already added ones.
     Initially the first added address will be selected
    */
    void                    AddAddress(const OUString& rAddress);
    //  for preview mode - replaces the currently used address by the given one
    void                    SetAddress(const OUString& rAddress);
    // removes all addresses
    void                    Clear();

    // returns the selected address
    sal_uInt16              GetSelectedAddress() const;
    void                    SelectAddress(sal_uInt16 nSelect);
    void                    ReplaceSelectedAddress(const OUString&);
    void                    RemoveSelectedAddress();

    // set the number of rows and columns of addresses
    void                    SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns);
    void                    EnableScrollBar(bool bEnable = true);

    // fill the actual data into a string (address block or greeting)
    static String FillData(
            const OUString& rAddress,
            SwMailMergeConfigItem& rConfigItem,
            const ::com::sun::star::uno::Sequence< OUString>* pAssignments = 0);

    void    SetSelectHdl (const Link& rLink) {m_aSelectHdl = rLink;}
};

/*-------------------------------------------------------------------------
    iterate over an address block or a greeting line
    the iterator returns the parts either as pure string
    or as column
  -----------------------------------------------------------------------*/
struct SwMergeAddressItem
{
    String  sText;
    bool    bIsColumn;
    bool    bIsReturn;
    SwMergeAddressItem() :
        bIsColumn(false),
        bIsReturn(false) {}
};

class SW_DLLPUBLIC   SwAddressIterator
{
    String sAddress;
public:
    SwAddressIterator(const String& rAddress) :
        sAddress(rAddress){}

    SwMergeAddressItem  Next();
    bool                HasMore() const{return sAddress.Len() > 0;}
};

class SW_DLLPUBLIC SwAuthenticator :
    public cppu::WeakImplHelper1< ::com::sun::star::mail::XAuthenticator>
{
    OUString m_aUserName;
    OUString m_aPassword;
    Window*         m_pParentWindow;
public:
    SwAuthenticator() : m_pParentWindow(0) {}
    SwAuthenticator(const OUString& username, const OUString& password, Window* pParent) :
        m_aUserName(username),
        m_aPassword(password),
        m_pParentWindow( pParent )
    {}
    ~SwAuthenticator();

    virtual OUString SAL_CALL getUserName( ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getPassword(  ) throw (::com::sun::star::uno::RuntimeException);

};

class SW_DLLPUBLIC SwConnectionContext :
    public cppu::WeakImplHelper1< ::com::sun::star::uno::XCurrentContext >
{
    OUString     m_sMailServer;
    sal_Int16           m_nPort;
    OUString     m_sConnectionType;

public:
    SwConnectionContext(const OUString& rMailServer, sal_Int16 nPort, const OUString& rConnectionType);
    ~SwConnectionContext();

    virtual ::com::sun::star::uno::Any SAL_CALL     getValueByName( const OUString& Name )
                                                            throw (::com::sun::star::uno::RuntimeException);
};

class SwMutexBase
{
public:
    osl::Mutex m_aMutex;
};

class SW_DLLPUBLIC SwConnectionListener :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper1< ::com::sun::star::mail::XConnectionListener >
{
    using cppu::WeakComponentImplHelperBase::disposing;

public:
    SwConnectionListener() :
        cppu::WeakComponentImplHelper1< ::com::sun::star::mail::XConnectionListener>(m_aMutex)
    {}
    ~SwConnectionListener();

    virtual void SAL_CALL connected(const ::com::sun::star::lang::EventObject& aEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disconnected(const ::com::sun::star::lang::EventObject& aEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing(const com::sun::star::lang::EventObject& aEvent)
        throw(com::sun::star::uno::RuntimeException);
};

class SwMailTransferable :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper2
        <
            ::com::sun::star::datatransfer::XTransferable,
            ::com::sun::star::beans::XPropertySet
        >
{
    OUString  m_aMimeType;
    OUString  m_sBody;
    OUString  m_aURL;
    OUString  m_aName;
    bool           m_bIsBody;

    public:
    SwMailTransferable(const OUString& rURL, const OUString& rName, const OUString& rMimeType);
    SwMailTransferable(const OUString& rBody, const OUString& rMimeType);
    ~SwMailTransferable();
    virtual ::com::sun::star::uno::Any SAL_CALL
                        getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
                            throw (::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL
                        getTransferDataFlavors(  )
                            throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::sal_Bool SAL_CALL
                        isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
                            throw (::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

};

class SwMailMessage :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper1< ::com::sun::star::mail::XMailMessage >
{
    OUString                                                                         m_sSenderName;
    OUString                                                                         m_sSenderAddress;
    OUString                                                                         m_sReplyToAddress;
    OUString                                                                         m_sSubject;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable>        m_xBody;
//    ::com::sun::star::mail::MailMessageBody                                                 m_aBody;

    ::com::sun::star::uno::Sequence< OUString >                                      m_aRecipients;
    ::com::sun::star::uno::Sequence< OUString >                                      m_aCcRecipients;
    ::com::sun::star::uno::Sequence< OUString >                                      m_aBccRecipients;
//    ::com::sun::star::uno::Sequence< ::com::sun::star::mail::MailAttachmentDescriptor >     m_aAttachments;
    ::com::sun::star::uno::Sequence<  ::com::sun::star::mail::MailAttachment >              m_aAttachments;
public:
    SwMailMessage();
    ~SwMailMessage();

    // attributes
    virtual OUString SAL_CALL    getSenderName() throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL    getSenderAddress() throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL    getReplyToAddress() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               setReplyToAddress( const OUString& _replytoaddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL    getSubject() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               setSubject( const OUString& _subject ) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL
                                        getBody()
                                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               setBody( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _body )
                                                throw (::com::sun::star::uno::RuntimeException);

    // methods
    virtual void SAL_CALL               addRecipient( const OUString& sRecipientAddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               addCcRecipient( const OUString& sRecipientAddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               addBccRecipient( const OUString& sRecipientAddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                                        getRecipients(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                                        getCcRecipients(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                                        getBccRecipients(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               addAttachment( const ::com::sun::star::mail::MailAttachment& aMailAttachment )
                                            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::mail::MailAttachment > SAL_CALL
                                        getAttachments(  ) throw (::com::sun::star::uno::RuntimeException);
    void                                SetSenderName(const OUString& rSenderName)
                                                {m_sSenderName = rSenderName;}
    void                                SetSenderAddress(const OUString& rSenderAddress)
                                                {m_sSenderAddress = rSenderAddress;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
