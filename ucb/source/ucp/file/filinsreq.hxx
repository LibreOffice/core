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
            throw (css::uno::RuntimeException, std::exception) override
        {
            m_bSelected = true;
        }

        void SAL_CALL setName(const OUString& Name)
            throw(css::uno::RuntimeException, std::exception) override
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
            throw (css::uno::RuntimeException, std::exception) override
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
    {
    public:

        XInteractionRequestImpl(
            const OUString& aClashingName,
            const css::uno::Reference< css::uno::XInterface>& xOrigin,
            shell* pShell,
            sal_Int32 CommandId);

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

        css::uno::Reference<css::task::XInteractionRequest> const& getRequest() const
        {
            return m_xRequest;
        }

    private:

        XInteractionSupplyNameImpl* p1;
        XInteractionAbortImpl* p2;
        sal_Int32 m_nErrorCode,m_nMinorError;

        css::uno::Reference<css::task::XInteractionRequest> m_xRequest;

        OUString m_aClashingName;
        css::uno::Reference< css::uno::XInterface> m_xOrigin;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
