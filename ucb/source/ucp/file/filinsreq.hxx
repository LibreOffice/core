/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filinsreq.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:25:40 $
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


#ifndef _FILINSREQ_HXX_
#define _FILINSREQ_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYNAME_HPP_
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif


namespace fileaccess {


    class shell;


    class XInteractionSupplyNameImpl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::ucb::XInteractionSupplyName
    {
    public:

        XInteractionSupplyNameImpl()
            : m_bSelected(false)
        {
        }

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        virtual void SAL_CALL select()
            throw (::com::sun::star::uno::RuntimeException)
        {
            m_bSelected = true;
        }

        void SAL_CALL setName(const ::rtl::OUString& Name)
            throw(::com::sun::star::uno::RuntimeException)
        {
            m_aNewName = Name;
        }

        rtl::OUString getName() const
        {
            return m_aNewName;
        }

        bool isSelected() const
        {
            return m_bSelected;
        }

    private:

        bool          m_bSelected;
        rtl::OUString m_aNewName;
    };



    class XInteractionAbortImpl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::task::XInteractionAbort
    {
    public:

        XInteractionAbortImpl()
            : m_bSelected(false)
        {
        }

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        virtual void SAL_CALL select()
            throw (::com::sun::star::uno::RuntimeException)
        {
            m_bSelected = true;
        }


        bool isSelected() const
        {
            return m_bSelected;
        }

    private:

        bool          m_bSelected;
    };



    class XInteractionRequestImpl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::task::XInteractionRequest
    {
    public:

        XInteractionRequestImpl(
            const rtl::OUString& aClashingName,
            const com::sun::star::uno::Reference<
            com::sun::star::uno::XInterface>& xOrigin,
            shell* pShell,
            sal_Int32 CommandId);

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()

        ::com::sun::star::uno::Any SAL_CALL getRequest(  )
            throw (::com::sun::star::uno::RuntimeException);

        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > SAL_CALL
        getContinuations(  )
            throw (::com::sun::star::uno::RuntimeException)
        {
            return m_aSeq;
        }

        bool aborted() const
        {
            return p2->isSelected();
        }

        rtl::OUString newName() const
        {
            if( p1->isSelected() )
                return p1->getName();
            else
                return rtl::OUString();
        }

    private:

        XInteractionSupplyNameImpl* p1;
        XInteractionAbortImpl* p2;
        sal_Int32 m_nErrorCode,m_nMinorError;

        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > m_aSeq;

        rtl::OUString m_aClashingName;
        com::sun::star::uno::Reference<
            com::sun::star::uno::XInterface> m_xOrigin;
    };

}


#endif
