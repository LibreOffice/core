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

#ifndef INCLUDED_UCB_SOURCE_UCP_FTP_FTPINTREQ_HXX
#define INCLUDED_UCB_SOURCE_UCP_FTP_FTPINTREQ_HXX

#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase.hxx>


namespace ftp {

    class XInteractionApproveImpl : public cppu::WeakImplHelper <
        css::task::XInteractionApprove >
    {
    public:

        XInteractionApproveImpl();

        virtual void SAL_CALL select()
            throw (css::uno::RuntimeException,
                   std::exception) override;

        bool isSelected() const { return m_bSelected;}

    private:

        bool          m_bSelected;
    };




    class XInteractionDisapproveImpl : public cppu::WeakImplHelper <
        css::task::XInteractionDisapprove >
    {
    public:

        XInteractionDisapproveImpl();

        virtual void SAL_CALL select()
            throw (com::sun::star::uno::RuntimeException, std::exception) override;

    private:

        bool          m_bSelected;
    };



    class XInteractionRequestImpl
    {
    public:

        XInteractionRequestImpl();

        bool approved() const;

        css::uno::Reference<css::task::XInteractionRequest> const& getRequest() const
        {
            return m_xRequest;
        }

    private:

        XInteractionApproveImpl* p1;
        XInteractionDisapproveImpl* p2;

        css::uno::Reference<css::task::XInteractionRequest> m_xRequest;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
