/*************************************************************************
 *
 *  $RCSfile: mailmergehelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-20 13:22:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _MAILMERGEHELPER_HXX
#define _MAILMERGEHELPER_HXX

#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_MAIL_XAUTHENTICATOR_HPP_
#include "com/sun/star/mail/XAuthenticator.hpp"
#endif
#ifndef _COM_SUN_STAR_MAIL_XCONNECTIONLISTENER_HPP_
#include "com/sun/star/mail/XConnectionListener.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include "com/sun/star/uno/XCurrentContext.hpp"
#endif
#ifndef _COM_SUN_STAR_MAIL_XMAILMESSAGE_HPP_
#include "com/sun/star/mail/XMailMessage.hpp"
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include "com/sun/star/datatransfer/XTransferable.hpp"
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

class SwMailMergeConfigItem;
namespace com{ namespace sun{ namespace star{
    namespace uno{
        class XComponentContext;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
}}}
/*-- 14.06.2004 12:27:42---------------------------------------------------

  -----------------------------------------------------------------------*/
namespace SwMailMergeHelper
{
    String  CallSaveAsDialog(String& rFilter);
    bool    CheckMailAddress( const ::rtl::OUString& rMailAddress );
}
/* -----------------06.04.2004 10:29-----------------

 --------------------------------------------------*/
class SwBoldFixedInfo : public FixedInfo
{
public:
    SwBoldFixedInfo(Window* pParent, const ResId& rResId);
    ~SwBoldFixedInfo();
};
struct SwAddressPreview_Impl;
namespace com{namespace sun{namespace star{
    namespace container{
        class XNameAccess;
    }
}}}

/*-- 27.04.2004 13:20:00---------------------------------------------------
    Preview window used to show the possible selection of address blocks
    and also the resulting address filled with database data
  -----------------------------------------------------------------------*/
class SwAddressPreview : public Window
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

    DECL_LINK(ScrollHdl, ScrollBar*);

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

/*-- 11.05.2004 15:39:59---------------------------------------------------
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
class SwAddressIterator
{
    String sAddress;
public:
    SwAddressIterator(const String& rAddress) :
        sAddress(rAddress){}

    SwMergeAddressItem  Next();
    bool                HasMore() const{return sAddress.Len() > 0;}
};

/*-- 21.05.2004 10:31:15---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwAuthenticator :
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
/*-- 25.08.2004 12:48:47---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwConnectionContext :
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
/*-- 21.05.2004 10:39:20---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMutexBase
{
public:
    osl::Mutex m_aMutex;
};
/*-- 21.05.2004 10:39:20---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwConnectionListener :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper1< ::com::sun::star::mail::XConnectionListener >
{
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

/*-- 13.07.2004 09:02:12---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailTransferable :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper1< ::com::sun::star::datatransfer::XTransferable >
{
    String  m_aMimeType;
    String  m_sBody;
    String  m_aURL;
    String  m_aName;
    bool    m_bIsBody;

    public:
    SwMailTransferable(const String& rURL, const String& rName, const String& rMimeType);
    SwMailTransferable(const String& rBody, const String& rMimeType);
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
};

/*-- 22.06.2004 16:38:34---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMessage :
        public SwMutexBase,
        public cppu::WeakComponentImplHelper1< ::com::sun::star::mail::XMailMessage >
{
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

    // Attributes
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

    // Methods
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
    void                                SetSenderAddress(const ::rtl::OUString& rSenderAddress)
                                                {m_sSenderAddress = rSenderAddress;}
};
/*-- 21.05.2004 10:17:22---------------------------------------------------

  -----------------------------------------------------------------------*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>
        getCurrentCmpCtx(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory> rSrvMgr);
#endif

