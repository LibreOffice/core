/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smplmailmsg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:09:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SMPLMAILMSG_HXX_
#define _SMPLMAILMSG_HXX_

//_______________________________________________________________________________________________________________________
//  includes of other projects
//_______________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_SYS_SHELL_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSimpleMailMessage.hpp>
#endif

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CSmplMailMsg :
    public  cppu::WeakImplHelper1< com::sun::star::system::XSimpleMailMessage >
{
public:
    CSmplMailMsg( );

    //------------------------------------------------
    //
    //------------------------------------------------

    virtual void SAL_CALL setRecipient( const ::rtl::OUString& aRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    //
    //------------------------------------------------

    virtual void SAL_CALL setCcRecipient( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aCcRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCcRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    //
    //------------------------------------------------

    virtual void SAL_CALL setBccRecipient( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aBccRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getBccRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    //
    //------------------------------------------------

    virtual void SAL_CALL setOriginator( const ::rtl::OUString& aOriginator )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getOriginator(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    //
    //------------------------------------------------

    virtual void SAL_CALL setSubject( const ::rtl::OUString& aSubject )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getSubject(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    //
    //------------------------------------------------

    virtual void SAL_CALL setAttachement( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aAttachement )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAttachement(  )
        throw (::com::sun::star::uno::RuntimeException);

private:
    rtl::OUString                                   m_aRecipient;
    rtl::OUString                                   m_aOriginator;
    rtl::OUString                                   m_aSubject;
    com::sun::star::uno::Sequence< rtl::OUString >  m_CcRecipients;
    com::sun::star::uno::Sequence< rtl::OUString >  m_BccRecipients;
    com::sun::star::uno::Sequence< rtl::OUString >  m_Attachements;
};

#endif
