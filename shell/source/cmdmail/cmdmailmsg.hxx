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



#ifndef _CMDMAILMSG_HXX_
#define _CMDMAILMSG_HXX_

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#ifndef _COM_SUN_STAR_SYS_SHELL_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSimpleMailMessage2.hpp>
#endif
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CmdMailMsg :
    public  cppu::WeakImplHelper2<
        ::com::sun::star::system::XSimpleMailMessage2,
        ::com::sun::star::container::XNameAccess >
{
    ::rtl::OUString                                   m_aBody;
    ::rtl::OUString                                   m_aRecipient;
    ::rtl::OUString                                   m_aOriginator;
    ::rtl::OUString                                   m_aSubject;
    ::com::sun::star::uno::Sequence< rtl::OUString >  m_CcRecipients;
    ::com::sun::star::uno::Sequence< rtl::OUString >  m_BccRecipients;
    ::com::sun::star::uno::Sequence< rtl::OUString >  m_Attachments;

    ::osl::Mutex m_aMutex;

public:

    CmdMailMsg() {};

    //------------------------------------------------
    // XSimpleMailMessage
    //------------------------------------------------

    virtual void SAL_CALL setBody( const ::rtl::OUString& aBody )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getBody(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setRecipient( const ::rtl::OUString& aRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setCcRecipient( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aCcRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCcRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setBccRecipient( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aBccRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getBccRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setOriginator( const ::rtl::OUString& aOriginator )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getOriginator(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setSubject( const ::rtl::OUString& aSubject )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getSubject(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setAttachement( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aAttachement )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAttachement(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XNameAccess
    //------------------------------------------------

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException) ;

    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XElementAccess
    //------------------------------------------------

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasElements(  )
        throw (::com::sun::star::uno::RuntimeException);

};

#endif
