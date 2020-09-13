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

#pragma once

#include <com/sun/star/util/XVeto.hpp>

#include <cppuhelper/implbase.hxx>

namespace dbaccess
{

    // Veto
    typedef ::cppu::WeakImplHelper<   css::util::XVeto
                                  >   Veto_Base;
    /** implements css::util::XVeto
    */
    class Veto : public Veto_Base
    {
    private:
        const css::uno::Any    m_aDetails;

    public:
        Veto( const css::uno::Any& _rDetails );

        virtual OUString SAL_CALL getReason() override;
        virtual css::uno::Any SAL_CALL getDetails() override;

    protected:
        virtual ~Veto() override;

    private:
        Veto( const Veto& ) = delete;
        Veto& operator=( const Veto& ) = delete;
    };

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
