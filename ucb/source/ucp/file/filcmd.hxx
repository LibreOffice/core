 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filcmd.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:19:31 $
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
#ifndef _FILCMD_HXX_
#define _FILCMD_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif


namespace fileaccess {


    // forward
    class shell;


    class XCommandInfo_impl
        : public cppu::OWeakObject,
          public com::sun::star::ucb::XCommandInfo
    {
    public:

        XCommandInfo_impl( shell* pMyShell );

        virtual ~XCommandInfo_impl();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();

        // XCommandInfo

        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo > SAL_CALL
        getCommands(
            void )
            throw( com::sun::star::uno::RuntimeException);

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByName(
            const rtl::OUString& Name )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasCommandByName(
            const rtl::OUString& Name )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasCommandByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::uno::RuntimeException );


    private:

        shell*                                                                  m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
    };

}

#endif
