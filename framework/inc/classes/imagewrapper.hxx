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

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/image.hxx>

namespace framework
{

class ImageWrapper final : public ::cppu::WeakImplHelper< css::awt::XBitmap,
                                                    css::lang::XUnoTunnel >
{
    public:
        ImageWrapper( const Image& aImage );
        virtual ~ImageWrapper() override;

        const Image&    GetImage() const
        {
            return m_aImage;
        }

        static css::uno::Sequence< sal_Int8 > const & GetUnoTunnelId();

        // XBitmap
        virtual css::awt::Size SAL_CALL getSize() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getDIB() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getMaskDIB() override;

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    private:
        Image   m_aImage;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
