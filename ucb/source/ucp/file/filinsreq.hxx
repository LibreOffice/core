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


#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILINSREQ_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILINSREQ_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase.hxx>


namespace fileaccess {


    class shell;


class XInteractionSupplyNameImpl : public cppu::WeakImplHelper<
    css::ucb::XInteractionSupplyName >
    {
    public:

        XInteractionSupplyNameImpl()
            : m_bSelected(false)
        {
        }

        virtual void SAL_CALL select()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            m_bSelected = true;
        }

        void SAL_CALL setName(const OUString& Name)
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            m_aNewName = Name;
        }

        OUString getName() const
        {
            return m_aNewName;
        }

        bool isSelected() const
        {
            return m_bSelected;
        }

    private:

        bool          m_bSelected;
        OUString m_aNewName;
    };



    class XInteractionAbortImpl : public cppu::WeakImplHelper<
        css::task::XInteractionAbort >
    {
    public:

        XInteractionAbortImpl()
            : m_bSelected(false)
        {
        }

        virtual void SAL_CALL select()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
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



    class XInteractionRequestImpl : public cppu::WeakImplHelper<
        css::task::XInteractionRequest >
    {
    public:

        XInteractionRequestImpl(
            const OUString& aClashingName,
            const com::sun::star::uno::Reference<
            com::sun::star::uno::XInterface>& xOrigin,
            shell* pShell,
            sal_Int32 CommandId);

        ::com::sun::star::uno::Any SAL_CALL getRequest(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > SAL_CALL
        getContinuations(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            return m_aSeq;
        }

        bool aborted() const
        {
            return p2->isSelected();
        }

        OUString newName() const
        {
            if( p1->isSelected() )
                return p1->getName();
            else
                return OUString();
        }

    private:

        XInteractionSupplyNameImpl* p1;
        XInteractionAbortImpl* p2;
        sal_Int32 m_nErrorCode,m_nMinorError;

        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > m_aSeq;

        OUString m_aClashingName;
        com::sun::star::uno::Reference<
            com::sun::star::uno::XInterface> m_xOrigin;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
