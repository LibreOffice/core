/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    SW_DLLPUBLIC bool    CheckMailAddress( const ::rtl::OUString& rMailAddress );
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

    void DrawText_Impl( const ::rtl::OUString& rAddress, const Point& rTopLeft, const Size& rSize, bool bIsSelected);

    virtual void        Paint(const Rectangle&);
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    void                UpdateScrollBar();

    DECL_LINK(ScrollHdl, void*);

public:
    SwAddressPreview(Window* pParent, const ResId rResId);
    ~SwAddressPreview();

    /** The address string is a list of address elements separated by spaces
    and breaks. The addresses fit into the given layout. If more addresses then
    rows/columns should be used a scrollbar will be added.

     AddAddress appends the new address to the already added ones.
     Initially the first added address will be selected
    */
    void                    AddAddress(const ::rtl::OUString& rAddress);
    //  for preview mode - replaces the currently used address by the given one
    void                    SetAddress(const ::rtl::OUString& rAddress);
    // removes all addresses
    void                    Clear();

    // returns the selected address
    sal_uInt16              GetSelectedAddress() const;
    void                    SelectAddress(sal_uInt16 nSelect);
    void                    ReplaceSelectedAddress(const ::rtl::OUString&);
    void                    RemoveSelectedAddress();

    // set the number of rows and columns of addresses
    void                    SetLayout(sal_uInt16 nRows, sal_uInt16 nColumns);
    void                    EnableScrollBar(bool bEnable = true);

    // fill the actual data into a string (address block or greeting)
    static String FillData(
            const ::rtl::OUString& rAddress,
            SwMailMergeConfigItem& rConfigItem,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString>* pAssignments = 0);

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
    ::rtl::OUString m_aUserName;
    ::rtl::OUString m_aPassword;
    Window*         m_pParentWindow;
public:
    SwAuthenticator() : m_pParentWindow(0) {}
    SwAuthenticator(const ::rtl::OUString& username, const ::rtl::OUString& password, Window* pParent) :
        m_aUserName(username),
        m_aPassword(password),
        m_pParentWindow( pParent )
    {}
    ~SwAuthenticator();

    virtual ::rtl::OUString SAL_CALL getUserName( ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPassword(  ) throw (::com::sun::star::uno::RuntimeException);

};

class SW_DLLPUBLIC SwConnectionContext :
    public cppu::WeakImplHelper1< ::com::sun::star::uno::XCurrentContext >
{
    ::rtl::OUString     m_sMailServer;
    sal_Int16           m_nPort;
    ::rtl::OUString     m_sConnectionType;

public:
    SwConnectionContext(const ::rtl::OUString& rMailServer, sal_Int16 nPort, const ::rtl::OUString& rConnectionType);
    ~SwConnectionContext();

    virtual ::com::sun::star::uno::Any SAL_CALL     getValueByName( const ::rtl::OUString& Name )
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
    rtl::OUString  m_aMimeType;
    rtl::OUString  m_sBody;
    rtl::OUString  m_aURL;
    rtl::OUString  m_aName;
    bool           m_bIsBody;

    public:
    SwMailTransferable(const rtl::OUString& rURL, const rtl::OUString& rName, const rtl::OUString& rMimeType);
    SwMailTransferable(const rtl::OUString& rBody, const rtl::OUString& rMimeType);
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
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

};

class SwMailMessage :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper1< ::com::sun::star::mail::XMailMessage >
{
    ::rtl::OUString                                                                         m_sSenderName;
    ::rtl::OUString                                                                         m_sSenderAddress;
    ::rtl::OUString                                                                         m_sReplyToAddress;
    ::rtl::OUString                                                                         m_sSubject;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable>        m_xBody;
//    ::com::sun::star::mail::MailMessageBody                                                 m_aBody;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                      m_aRecipients;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                      m_aCcRecipients;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                      m_aBccRecipients;
//    ::com::sun::star::uno::Sequence< ::com::sun::star::mail::MailAttachmentDescriptor >     m_aAttachments;
    ::com::sun::star::uno::Sequence<  ::com::sun::star::mail::MailAttachment >              m_aAttachments;
public:
    SwMailMessage();
    ~SwMailMessage();

    // attributes
    virtual ::rtl::OUString SAL_CALL    getSenderName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL    getSenderAddress() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL    getReplyToAddress() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               setReplyToAddress( const ::rtl::OUString& _replytoaddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL    getSubject() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               setSubject( const ::rtl::OUString& _subject ) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL
                                        getBody()
                                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               setBody( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _body )
                                                throw (::com::sun::star::uno::RuntimeException);

    // methods
    virtual void SAL_CALL               addRecipient( const ::rtl::OUString& sRecipientAddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               addCcRecipient( const ::rtl::OUString& sRecipientAddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               addBccRecipient( const ::rtl::OUString& sRecipientAddress ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getRecipients(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getCcRecipients(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getBccRecipients(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL               addAttachment( const ::com::sun::star::mail::MailAttachment& aMailAttachment )
                                            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::mail::MailAttachment > SAL_CALL
                                        getAttachments(  ) throw (::com::sun::star::uno::RuntimeException);
    void                                SetSenderName(const ::rtl::OUString& rSenderName)
                                                {m_sSenderName = rSenderName;}
    void                                SetSenderAddress(const ::rtl::OUString& rSenderAddress)
                                                {m_sSenderAddress = rSenderAddress;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
